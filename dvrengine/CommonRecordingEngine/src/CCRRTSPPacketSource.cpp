/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:    RTSP Client impl.*
*/




// INCLUDE FILES
#include "CCRRtspPacketSource.h"
#include "CCRPunchPacketSender.h"
#include "CCRRtpTcpStreamer.h"
#include "CCRRtspCommand.h"
#include "CCRPacketBuffer.h"
#include <ipvideo/CDvrSdpParser.h>
#include "CCRTimer.h"
#include <Uri16.h>
#include <e32msgqueue.h>
#include <centralrepository.h>
#include <WebUtilsInternalCRKeys.h>
#include <mmf/common/mmferrors.h>  // ROP error codes

// DATA TYPES
// ######################################################
// WARNING: JUMBOJET-SIZED KLUDGE AHEAD:
// ######################################################
#define private public
// Explanation: timestamp getter in rtcp sender report
// class is broken beyond repair. It may be fixed but the
// broken version is already shipped to millions of phones
// around the world. The broken getter method can't
// be overridden as it requires access to private part
// of sender reports instance variables. The item we
// need (ntp timestamp) is there intact in private instance
// variables but there is useless getter for that.
#include <rtcp.h>

/* sender report (SR) */
class TRtcpSRPart
    {
public:
    TUint32 ssrc;     /**< sender generating this report */
    TUint32 ntp_sec;  /**< NTP timestamp */
    TUint32 ntp_frac; /**< Fractal seconds */
    TUint32 rtp_ts;   /**< RTP timestamp */
    TUint32 psent;    /**< packets sent */
    TUint32 osent;    /**< octets sent */
    };
#undef private
// ######################################################
// Major kludge ends here.
// ######################################################

// CONSTANTS
const TInt KCRPortNumberBase( 16670 );
const TInt KCSeqForRtspNegoation( 42 );
const TInt KRtspPortNumber( 554 );
const TInt KRtpPacketVersion( 2 ); 
const TUint KSenderReportPacketType( 0xC8 ); // 200 decimal
const TInt KDVR10Seconds( 10000000 );

// The number of sequential packets that must be received
// before a stream is considered good. 1 means no delay, start
// from very first packet
const TInt KDVRMinSequential( 1 ); 
// The maximum number of dropped packets to be considered a
// dropout, as opposed to an ended and restarted stream.    
const TInt KDVRMaxMisorder( 50 ); 
// The maximum number of packets by which a packet can be delayed 
// before it is considered dropped. 
const TInt KDVRMaxDropOut( 3000 ); 
_LIT( KRtspPortString, "554" );
_LIT8( KCRCName, "N++ " );
// Timeout for RTP/UDP reception before switching to TCP mode
const TTimeIntervalMicroSeconds32 KCRRtspRtpUdpTimeout( 10 * 1e6 );
// Timeout for waiting for server response to any RTSP command
const TTimeIntervalMicroSeconds32 KCRRtspResponseTimeout( 15 * 1e6 );
// Timeout for waiting for server response to TIERDOWN command
const TTimeIntervalMicroSeconds32 KCRRtspTierdownTimeout( 3 * 1e6 );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCRRtspPacketSource* CCRRtspPacketSource::NewL(
    const SCRRtspParams& aParams,
    CCRConnection& aConnection,
    RSocketServ& aSockServer,
    MCRStreamObserver& aSessionObs,
    CCRStreamingSession& aOwningSession )
    {
    CCRRtspPacketSource* self = new( ELeave )
        CCRRtspPacketSource( aConnection, aSockServer, aSessionObs, aOwningSession );
    CleanupStack::PushL( self );
    self->ConstructL( aParams );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::CCRRtspPacketSource
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCRRtspPacketSource::CCRRtspPacketSource(
    CCRConnection& aConnection,
    RSocketServ& aSockServer,
    MCRStreamObserver& aSessionObs,
    CCRStreamingSession& aOwningSession )
  : CCRPacketSourceBase( aOwningSession, CCRStreamingSession::ECRRtspSourceId ),
    iSockServer( aSockServer ),
    iConnection( aConnection ),
    iStage( ERTSPInit ),
    iCSeq( KCSeqForRtspNegoation ),
    iClientPort( KCRPortNumberBase ),
    iSessionId(NULL, 0 ),
    iReadyToPlay(EFalse),
    iSessionObs( aSessionObs ),
    iStartPos( KRealZero ),
    iEndPos( KRealMinusOne ),
    iUdpFound( EFalse ),
    iTrafficFound( EFalse )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::ConstructL( const SCRRtspParams& aParams )
    {
    LOG( "CCRRtspPacketSource::ConstructL() in" );
    if ( aParams.iUrl.Length() == 0 ) 
        {
        User::Leave ( KErrArgument ); 
        }

    iSentData = HBufC8::NewL ( KCROptionsReply().Length() + KMaxInfoName );
    iRtpTcpStreamer = CCRRtpTcpStreamer::NewL( *this );
    iRtspUri = aParams.iUrl.AllocL();
    iRtspUri8 = HBufC8::NewL( aParams.iUrl.Length());
    iRtspUri8->Des().Copy( aParams.iUrl );
    iUserName = aParams.iUserName.AllocL();
    iPassword = aParams.iPassword.AllocL();
    User::LeaveIfError( iConnection.RegisterObserver( this ) );
    iUdpReceptionTimer = CCRTimer::NewL( EPriorityLow, *this );
    iProxyServerAddr = aParams.iProxyServerAddr; 
    iProxyServerPort = aParams.iProxyServerPort; 
    DoConnectL(); // Makes no sense to construct without immediately connecting

    LOG( "CCRRtspPacketSource::ConstructL() out" );
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::~CCRRtspPacketSource
// Destructor.
// -----------------------------------------------------------------------------
//
CCRRtspPacketSource::~CCRRtspPacketSource()
    {
    LOG( "CCRRtspPacketSource::~CCRRtspPacketSource() in" );
    // Deletes everything related to session
    CleanUp();
    delete iRtspTimeout;
    delete iSentData;
    delete iAuthType;
    delete iRtspUri8;
    delete iUserName;
    delete iPassword;
    delete iNonce;
    delete iOpaque;
    delete iRealm;
    delete iRtpTcpStreamer;
    delete iUdpReceptionTimer;
    iReceiveStreams.Reset();
    iObserver = NULL;
    iConnection.UnregisterObserver( this );
    LOG( "CCRRtspPacketSource::~CCRRtspPacketSource() out" );
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::CleanUp
// Callback method called from cleanup-cidle that just calls the actual
// cleanup method.
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::CleanUp()
    {
    LOG( "CCRRtspPacketSource::CleanUp() in" );
    if ( iUdpReceptionTimer )
        {
        iUdpReceptionTimer->Cancel();
        }
    delete iRtspPingTimer;
    iRtspPingTimer = NULL;

    iRtpRecvSrcAudio.Close();
    iRtpRecvSrcVideo.Close();
    iAudioSession.Close();
    iVideoSession.Close();
    iReadyToPlay = EFalse;
    delete iSdpParser; iSdpParser = NULL;
    delete iRtspUri; iRtspUri = NULL;
    delete iRtspSock; iRtspSock = NULL;

    for ( TInt i( 0 ); i < ERTPMaxSockets; i++ )
        {
        delete iRTPSockArr[i];
        iRTPSockArr[i] = NULL;
        }
    for ( TInt i( 0 ); i < ERTSPLastStage; i++ )
        {
        delete iPrevCommands[i];
        iPrevCommands[i] = NULL;
        delete iResponses[i];
        iResponses[i] = NULL;
        }
    
    iSessionId.Set( NULL, 0 );
    delete iPunchPacketSenderAudio; iPunchPacketSenderAudio = NULL;
    delete iPunchPacketSenderVideo; iPunchPacketSenderVideo = NULL;
    delete iUserAgent; iUserAgent = NULL;
    delete iWapProfile; iWapProfile = NULL;
    iStartPos = KRealZero;
    iEndPos = KRealMinusOne;
    
    LOG( "CCRRtspPacketSource::CleanUp() out" );
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::DoConnectL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::DoConnectL( void )
    {
    if ( !iRtspUri )
        {
        User::Leave( KErrNotReady );
        }
    if ( iRtspSock )
        {
        delete iRtspSock; iRtspSock = NULL;
        }

    iRtspSock = CCRSock::NewL( *this, ERTPControl, iConnection.Connection(),
                               iSockServer, ETrue, ETrue );
    TUriParser uriParser;
    User::LeaveIfError( uriParser.Parse( iRtspUri->Des() ) );
    iRtspUriHost.Set( uriParser.Extract( EUriHost ) );
    TPtrC portString( KRtspPortString );
    if ( uriParser.IsPresent( EUriPort ) )
        {
        portString.Set( uriParser.Extract( EUriPort ) );
        }
    
    TLex portLex( portString );
    TInt port( KRtspPortNumber );
    if ( portLex.Val( port ) != KErrNone )
        {
        User::Leave( KErrMMInvalidURL );
        }
    if ( iProxyServerAddr.Length() && iProxyServerPort )
        {
        LOG2( "CCRRtspPacketSource::DoConnectL(), Proxy: %S port: %d",
            &iProxyServerAddr, iProxyServerPort );
        User::LeaveIfError( iRtspSock->ConnectSock( iProxyServerAddr, iProxyServerPort ) );      
        }
    else
        {
        User::LeaveIfError(iRtspSock->ConnectSock( iRtspUriHost, port ) );
        }
    iCSeq = KCSeqForRtspNegoation;

    TTime now;
    now.UniversalTime();
    iClientPort = 
        KCRPortNumberBase + ( ( now.DateTime().MicroSecond() / 1000 ) * 2 );

    // Get transport method from connection heuristics
    iTransport = ( iConnection.GetHeuristic(
        CCRConnection::EUdpStreamingBlocked ) )? ERTPOverTCP: ERTPOverUDP;
    LOG1( "CCRRtspPacketSource::DoConnectL(), RTP transport: %d (0=UDP, 1=TCP)", iTransport );

    // Get user agent, bandwidth and wap profile based on connection bearer (3G or not)
    TConnMonBearerType bearer = iConnection.BearerType();
    TBool is3g( iConnection.IsBearerWLANor3G( bearer ) );

    // Fetch wap profile from WebUtils repository
    if ( !iWapProfile )
        {
        CRepository* repository = CRepository::NewLC( KCRUidWebUtils );
        TUint32 profilekey = ( is3g )? KWebUtilsUaProf3G: KWebUtilsUaProf;
        TFileName profilebuf( KNullDesC );
        if ( !repository->Get( profilekey, profilebuf ) )
            {
            iWapProfile = HBufC8::NewL( profilebuf.Length() );
            iWapProfile->Des().Copy( profilebuf );
            }
        
        CleanupStack::PopAndDestroy( repository );
        LOG1( "CCRRtspPacketSource::DoConnectL(), iWapProfile: %S", &profilebuf );
        }

    // Fetch user agent
    // Should we add version information to user agent string?
    delete iUserAgent; iUserAgent = NULL;
    iUserAgent = KCRRTSPDefaultUserAgent().AllocL();

    // Get bandwidth from connection
    iBandwidth = iConnection.MaximumBandwidth();

    LOG( "CCRRtspPacketSource::DoConnectL out" );
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::URI
// -----------------------------------------------------------------------------
//
TPtr CCRRtspPacketSource::URI(void)
    {
    __ASSERT_DEBUG( iRtspUri != NULL , User::Panic( _L( "RTSP source" ), KErrBadHandle ) );
    TPtr retval ( NULL , 0 );
    if ( iRtspUri ) 
        {
        retval.Set( iRtspUri->Des() );
        }
    else
        {
        LOG( "CCRRtspPacketSource::URI iRtspUri was NULL !!!!!!!!!! " );
        }
    return retval; 
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::GetSdp
// -----------------------------------------------------------------------------
//
TInt CCRRtspPacketSource::GetSdp( TPtrC8& aSdp )
    {
    TInt retval( KErrNotReady );
    if ( iSdpParser )
        {
        return iSdpParser->GetSdp( aSdp );
        }
    
    return retval;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SeqAndTS
// -----------------------------------------------------------------------------
//
TInt CCRRtspPacketSource::SeqAndTS(
    TUint& aAudioSeq,
    TUint& aAudioTS,
    TUint& aVideoSeq,
    TUint& aVideoTS )
    {
    TInt retval( KErrNotReady );
    if ( iSeqFromRtpInfoForVideo != 0 || iSeqFromRtpInfoForAudio != 0  )
        {
        aAudioSeq = iSeqFromRtpInfoForAudio;
        aAudioTS = iRTPTimeStampAudio;
        aVideoSeq = iSeqFromRtpInfoForVideo;
        aVideoTS = iRTPTimeStampVideo;
        retval = KErrNone;
        }
    
    return retval;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::PostActionL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::PostActionL()
    {
    LOG1( "CCRRtspPacketSource::PostActionL(), SDP will be handled, iSdpParser: %d",
                                                                    iSdpParser );
    User::LeaveIfNull( iSdpParser );
    iSessionObs.StatusChanged( MCRPacketSource::ERtpStateSdpAvailable );
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::Play
// -----------------------------------------------------------------------------
//
TInt CCRRtspPacketSource::Play( const TReal& aStartPos, const TReal& aEndPos )
    {
    LOG2( "CCRRtspPacketSource::Play(), aStartPos: %f, aEndPos: %f",
                                        aStartPos, aEndPos );
    LOG2( "CCRRtspPacketSource::Play(), sent seq: %d, rec: %d", 
                                        iCSeq, iLastReceivedSeq );
    iReadyToPlay = ETrue;
    iStartPos = aStartPos;
    iEndPos = aEndPos;
    ResetStreamFlags();

    // In xps case we never get startpos with this method. 
    // instead setposition will be called 
    if ( iBuffer )
        {
        iBuffer->ResetBuffer();
        }    

    // If both audio and video sessions are closed, we
    // need to open at least one of them:
    TInt err( KErrNone );
    if ( iStage == ERTSPReadyToPlay || iStage == ERTSPPauseSent )
        {
        if ( iStage == ERTSPReadyToPlay || iCSeq == ( iLastReceivedSeq + 1 ) )
            {
            TRAP( err, SendPlayCommandL() );
            }
        else
            { 
            // We have a fast-fingered user in charge; play has been issued
            // but the previous pause has not been completed yet: postpone this
            // operation
            iPostPonedPlay = ETrue;
            }
        }
    
    return err;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::Pause
// -----------------------------------------------------------------------------
//
TInt CCRRtspPacketSource::Pause()
    {
    LOG1( "CCRRTSPPacketSource::Pause() stage %d", iStage );
    TInt err( KErrNotReady );
    if ( iStage == ERTSPPlaying )
        {
        if ( iResponses[ERTSPPlaySent]->IsLiveStream() || iSdpParser->IsLiveStream() )
            {
            err = KErrNotSupported;
            }
        else
            {
            TRAP( err, SendPauseCommandL() );
            }
        }
    if ( iStage == ERTSPPauseSent )
        {
        err = KErrNone;
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::Stop
// -----------------------------------------------------------------------------
//
TInt CCRRtspPacketSource::Stop()
    {
    LOG( "CCRRtspPacketSource::Stop()" );
    
    iReadyToPlay = EFalse;
    iPostPonedPlay = EFalse;
    iStartPos = KRealZero;
    TInt err( KErrDisconnected );
    
    if ( iStage == ERTSPPlaySent || iStage == ERTSPPlaying ||
         iStage == ERTSPPauseSent || iStage == ERTSPSetupAudioSent ||
         iStage == ERTSPSetupVideoSent )
        {
        err = KErrNone;
        if ( iRtspSock )
            {
            iRtspSock->Cancel();
            }
       
        TRAP_IGNORE( SendTearDownCommandL() ); // if this fails, we don't care
        iStage = ERTSPTearDownSent;
        StartRtspTimeout( KCRRtspTierdownTimeout );
        }

    return err;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SetPosition
// -----------------------------------------------------------------------------

TInt CCRRtspPacketSource::SetPosition( const TInt64 aPosition ) 
    {
    LOG1( "CCRRtspPacketSource::SetPosition(), iStartPos: %f", iStartPos );

    if ( aPosition == -2 )
        {
        if ( iStage != ERTSPPlaySent && iObserver )
            {
            iObserver->ConnectionStatusChange(
                iOwningSession.SourceChecksum(),
                ECRReadyToSeek, KErrNone );
            }
        }
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::GetRange
// -----------------------------------------------------------------------------
void CCRRtspPacketSource::GetRange( TReal& aLower, TReal& aUpper )
    {
    aLower = KRealZero;
    aUpper = KRealMinusOne;

    if ( ( iStage == ERTSPPlaySent || iStage == ERTSPPlaying ) &&
         iResponses[ERTSPPlaySent] )
         {
         iResponses[ERTSPPlaySent]->GetRange(aLower,aUpper);
         }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::DataReceived
// This is called when data is received from socket.
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::DataReceived( TInt /*aSockId*/, const TDesC8& aData )
    {
    // Find out RTCP message or RTP packet from IP packet
    iRtpTcpStreamer->DataAvailable( aData, ( iTransport == ERTPOverTCP ) );
    }
    
// -----------------------------------------------------------------------------
// CCRRtspPacketSource::RtspMsgAvailable
// This is called when data is received from socket.
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::RtspMsgAvailable( const TDesC8& aData )
    {
#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE )
    if ( aData.Length() > 0 )
        {
        LOG1( "CCRRtspPacketSource::RtspMsgAvailable(), aData len: %d", aData.Length() );
        TName d( KNullDesC );
        for( TInt i( 0 );  i < aData.Length(); i++ )
            {
            TChar c = aData[i];
            d.Append( c );
            if ( ( i > 0 ) && ( i % 80 ) == 0 )
                {
                LOG1( ">%S<", &d );
                d.Zero();
                }
            }
        
        LOG1( ">%S<", &d );
        }
#endif // LIVE_TV_FILE_TRACE || LIVE_TV_RDEBUG_TRACE

    TRAPD( err, ProcessRtspResponseL( aData ) );
    if ( err )
        {
        LOG1( "CCRRtspPacketSource::RtspMsgAvailable(), ProcessRtspResponseL Leaved, err: %d", err );
        if ( err == KErrNotSupported ) 
            {
            // The response did not look like rtsp response at all. 
            // some servers decide to send rtsp commands to us so lets
            // try interpreting it as a command
            err = KErrNone; 
            TRAP( err, ProcessRTSPCommandL( aData ) )
            if ( err )
                {
                iOwningSession.SourceStop();                
                }
            }
        else
            {
            iOwningSession.SourceStop();
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SockStatusChange
// This is called when socket status changes.
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::SockStatusChange(
    TInt aSockId, 
    CCRSock::TCRSockStatus aStatus,
    TInt aError )
    {
#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE )
    if ( aStatus == CCRSock::EFailed )
        {
        LOG3( "CCRRtspPacketSource::SockStatusChange(), aSockId: %d,  aStatus: %d, aError: %d",
                                                        aSockId, aStatus, aError );
        }
#else // LIVE_TV_FILE_TRACE || LIVE_TV_RDEBUG_TRACE
    ( void )aSockId;
    ( void )aError;
#endif // LIVE_TV_FILE_TRACE || LIVE_TV_RDEBUG_TRACE

    if ( aStatus == CCRSock::EFailed )
        {
        // Ask session to perform cleanup
        iOwningSession.SourceStop();

        if ( iStage == ERTSPInit && aSockId == ERTPControl && aError == KErrCouldNotConnect )
            {
            // map error to different error id, so we can know that showing reconnect query is pointless.
            aError = KErrEof;
            }
        
        // Inform the observer that there is a problem. Exclude case where we're closing
        // and the error is KErrEof
        if ( ! ( iStage == ERTSPTearDownSent && aError == KErrEof ) ) 
            {
            if ( iObserver )
                {
                iObserver->ConnectionStatusChange(
                    iOwningSession.SourceChecksum(), ECRConnectionError, aError );
                }
            }
        else
            {
            LOG( "CCRRtspPacketSource::SockStatusChange(), eof in closing: normal" );
            }
        }
    else if ( aSockId == ERTPControl && aStatus == CCRSock::EIdle && 
              iStage == ERTSPInit )
        {
        // Called once from here for lifetime of this object
        TRAPD( err, SendRtspCommandL() ); 
        if ( err )
            {
            LOG1( "CCRRtspPacketSource::SockStatusChange(), SendRtspCommandL Leaved: %d", err );
    
            // Ask session to perform cleanup
            iOwningSession.SourceStop();
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::RtpTcpPacketAvailable
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::RtpTcpPacketAvailable(
    TInt aChannel,
    const TDesC8& aPacket )
    {
    // Map embedded TCP channel to streamid:
    // video: channel=(0,1) --> id=(2,3)
    // audio: channel=(2,3) --> id=(0,1)  when video present
    // audio: channel=(0,1) --> id=(0,1)  when audio only
    TInt mappedChannel( ( iSdpParser->VideoControlAddr().Length() )? 
                        ( aChannel + 2 ) % 4: aChannel );
    MCRPacketSource::TCRPacketStreamId streamid( 
        ( MCRPacketSource::TCRPacketStreamId )( mappedChannel ) );

    iBuffer->AddPacket( streamid, aPacket );
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::ForwardRtpTcpChunk
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::ForwardRtpTcpChunck( const TDesC8& aChunk )
    {
    if ( iRtspSock )
        {
        iRtspSock->SendData( aChunk );
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::TimerExpired
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::TimerExpired( CCRTimer* )
    {
    LOG( "CCRRtspPacketSource::TimerExpired: RTP/UDP timer expired, switching to RTP/TCP" );

    if ( !iUdpFound )
        {            
        // Signal heuristic for TCP streaming
        LOG( "CCRRtspPacketSource::TimerExpired - Switch to TCP" );
        iConnection.SetHeuristic( CCRConnection::EUdpStreamingBlocked, ETrue );
        }
    else
        {
        // We had UDP before in this session but now it is lost for some reason.
        // Try UDP again.
        
        // Flag UDP found away
        iUdpFound = EFalse;
        iTrafficFound = EFalse;
        
        // Clear stream followup
        iReceiveStreams.Reset();
        
        LOG( "CCRRtspPacketSource::TimerExpired - Trying UDP again" );
        }
    
    // Notify client to close us and start a new session
    if ( iObserver )
        {
        // Notify client
        iObserver->ConnectionStatusChange(
            iOwningSession.SourceChecksum(), ECRSwitchingToTcp, KErrNone );
        }
    else
        {
        // If no client observer, teardown and cleanup ourselves
        iPostPonedPlay = EFalse; 
        TRAPD( err, SendTearDownCommandL() );
        if ( err != KErrNone )
            {
            LOG1( "CCRRtspPacketSource::TimerExpired() Send TEARDOWN failed: %d", err );
            }
    
        CleanUp();
        iSessionObs.StatusChanged( MCRPacketSource::ERtpStateClosing );
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::ProcessRTSPCommandL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::ProcessRTSPCommandL( const TDesC8& aData )
    {
    LOG1( "CCRRtspPacketSource::ProcessRTSPCommandL(), iStage: %d", ( int )iStage );

    CCRRtspCommand* cmd = CCRRtspCommand::NewL();
    CleanupStack::PushL( cmd );
    cmd->TryParseL( aData );
    
    switch ( cmd->Command() )
        {
        case CCRRtspCommand::ERTSPCommandOPTIONS:
            iSentData->Des().Format( KCROptionsReply, cmd->CSeq() );
            iRtspSock->SendData( iSentData->Des() );
            break;
            
        default:
            // Server sent us a command and it is not options. 
            // for sure they want us to stop ; is there 
            iOwningSession.SourceStop();
            break;
        }
    
    CleanupStack::PopAndDestroy( cmd );
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::ProcessRtspResponseL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::ProcessRtspResponseL( const TDesC8& aData )
    {
    LOG1( "CCRRtspPacketSource::ProcessRtspResponseL(), iStage: %d", iStage );

    // Cancel timeout timer
    if ( iRtspTimeout )
        {
        iRtspTimeout->Cancel();
        }

    // The server responded to our TEARDOWN command. No need to parse the response
    // since we don't care what the server said. Ask session to clean us up.
    if ( iStage == ERTSPTearDownSent )
        {
        iOwningSession.SourceStop();
        return;
        }

    // First parse response
    CCRRtspResponse* resp = CCRRtspResponse::NewL();
    CleanupStack::PushL( resp );
    resp->TryParseL( aData );
    
    // Then find the command that this resp is associated with:
    iLastReceivedSeq = resp->CSeq(); 
    TBool commandFound( EFalse );
    for ( TInt i( 0 ); i < ERTSPLastStage && !commandFound; i++ )
        {
        LOG2( "CCRRtspPacketSource:: prevcommand stage: %d cseq: %d", 
            i, ( iPrevCommands[i] )? iPrevCommands[i]->CSeq(): KErrNotFound );
        
        if ( iPrevCommands[i] && ( iPrevCommands[i]->CSeq() == resp->CSeq() ) )
            {
            LOG1( "CCRRtspPacketSource::ProcessRtspResponseL(), matching command: %d", i );
            LOG1( "CCRRtspPacketSource::ProcessRtspResponseL(), cseq was: %d", resp->CSeq() );
            delete iResponses[i];
            CleanupStack::Pop( resp );
            iResponses[i] = resp;
            commandFound = ETrue;
            if ( i == ERTSPOptSent )
                {
                // Process options no further, used only for ping here
                return; 
                }
            }
        }
    
    // Delete response if sequency not match
    if ( !commandFound )
        {
        CleanupStack::PopAndDestroy( resp );
        LOG1( "CCRRtspPacketSource::ProcessRtspResponseL(), Command not found, cseq: %d", resp->CSeq() );
        }
    else
        {
        if ( iResponses[iStage]->StatusCode() ==  CCRRtspResponse::ERTSPRespOK ||      // 200
             iResponses[iStage]->StatusCode() ==  CCRRtspResponse::ERTSPRespCreated )  // 201
            {
            // Extract useful information from response depending on stage:
            switch ( iStage )
                {
                case ERTSPSetupAudioSent: // From setups take session id
                case ERTSPSetupVideoSent:
                    if ( !iSessionId.Ptr() )
                        {
                        iResponses[iStage]->SessionId( iSessionId );
                        }
                    // Check for sdp parser and send punch packets for UDP transport
                    // (TCP or multicast: session setup and PLAY in SendRTSPCommand)
                    if ( iSdpParser && iTransport == ERTPOverUDP )
                        {
                        // If we see that we don't need to send further setups,
                        // do send punch packets now.
                        if ( ( iSdpParser->VideoControlAddr().Length() && // if we have video
                             iResponses[ERTSPSetupVideoSent] &&   // and we have video se tup
                             iSdpParser->AudioControlAddr().Length() &&   // and we have audio
                             iResponses[ERTSPSetupAudioSent] ) || // and we have audio set up or...
                           ( !iSdpParser->VideoControlAddr().Length() &&  // if we have no video
                             !iResponses[ERTSPSetupVideoSent] &&  // and we've video not set up
                             iSdpParser->AudioControlAddr().Length() &&   // and it shows we have audio
                             iResponses[ERTSPSetupAudioSent] ) || // and we've audio set up or...
                           ( iSdpParser->VideoControlAddr().Length() &&   // if we have video
                             iResponses[ERTSPSetupVideoSent] &&   // and we have video set up
                             !iSdpParser->AudioControlAddr().Length() &&  // and we have no audio
                             !iResponses[ERTSPSetupAudioSent] ) ) // and we have no audio set up
                            {
                            SendPunchPacketsL();
                            }
                        }
                    
                    // Notify sink that SETUP repply received
                    iSessionObs.StatusChanged(
                        MCRPacketSource::ERtpStateSetupRepply );
                    break;
                
                case ERTSPDescSent: // From desc take sdp
                    if ( iObserver && iResponses[iStage]->ContentLen() <= 0 )
                        { 
                        // This should not happen
                        if ( iObserver ) 
                            {
                            iObserver->ConnectionStatusChange(
                                iOwningSession.SourceChecksum(), ECRConnectionError, KErrUnderflow );
                            }
                        iOwningSession.SourceStop();
                        }
                    else
                        {
                        delete iSdpParser; iSdpParser = NULL;
                        iSdpParser = CDvrSdpParser::NewL();
                        if ( iResponses[iStage]->ContentBase().Length() )
                            {
                            iSdpParser->TryParseL( iResponses[iStage]->Content(), 
                                                   iResponses[iStage]->ContentBase() );
                            }
                        else
                            {
                            iSdpParser->TryParseL( iResponses[iStage]->Content(), 
                                                   iRtspUri8->Des() );
                            }                  
                        // Check for multicast address in SDP
                        if ( iSdpParser->IsMultiCastSdp() )
                            {
                            iTransport = ERTPOverMulticast;
                            }
                        if ( iObserver && iSdpParser->IsRealMediaContent() )
                            {
                            iObserver->ConnectionStatusChange(
                                iOwningSession.SourceChecksum(),
                                ECRStreamIsRealMedia, KErrNotSupported );
                            iOwningSession.SourceStop();
                            return; // Make sure we don't continue with SETUP commands
                            }
                        else // do not send realmedia sdp to sinks
                            { 
                            if ( iObserver && iSdpParser->IsLiveStream() )
                                {
                                iObserver->ConnectionStatusChange(
                                    iOwningSession.SourceChecksum(),
                                    ECRStreamIsLiveStream, KErrNone );
                                }
                            
                            // then check for bandwidth requirements even before we start:
                            if ( iObserver )
                                {
                                // Unknown bitrate or bandwidth are returned as zero.
                                // Bitrates in kbit/s
                                TInt bitrate( iSdpParser->VideoBitrate() + 
                                              iSdpParser->AudioBitrate() );
                                TInt bandwidth( iConnection.MaximumBandwidth() / 1000 );
                                if ( bitrate && bandwidth && bandwidth < bitrate )
                                    {
                                    LOG2( "CCRRtspPacketSource::ProcessRtspResponseL(), bitrate:%d, bandwidth: %d -> NotEnoughBandwidth",
                                                                                        bitrate, bandwidth);
                                    iObserver->ConnectionStatusChange(
                                        iOwningSession.SourceChecksum(),
                                        ECRNotEnoughBandwidth, KErrNone );
                                    return; // Make sure we don't tell sinks anything about
                                            // sdp that has too high bitrate for our network bearer
                                    }
                                }
                            
                            // But if we didn't have realmedia stream and the bandwidth check
                            // is also all right, then go on and tell the sinks ->
                            iSessionObs.StatusChanged(
                                MCRPacketSource::ERtpStateSdpAvailable );
                            }
                        }
                    break;
                
                case ERTSPPlaySent:
                    {
                    CCRRtspResponse::SRTPInfoHeader rtpInfo;
                    iResponses[ERTSPPlaySent]->RTPInfoHeader( rtpInfo );

                    TPtrC8 videoAddr ( NULL, 0 ); 
                    if ( iSdpParser->VideoControlAddr().Length() ) 
                        {
                        videoAddr.Set ( iSdpParser->VideoControlAddr() );
                        }
                    TPtrC8 audioAddr ( NULL , 0 );
                    if ( iSdpParser->AudioControlAddr().Length() ) 
                        {
                        audioAddr.Set ( iSdpParser->AudioControlAddr() );
                        }
                    
                    if ( iSdpParser->VideoControlAddr().Length() && 
                        rtpInfo.iFirstURL.Length() &&
                        videoAddr.Find( rtpInfo.iFirstURL ) >= 0 )
                        {
                        iRTPTimeStampVideo = rtpInfo.iFirstTS ? rtpInfo.iFirstTS : 1;
                        iSeqFromRtpInfoForVideo = rtpInfo.iFirstSeq;
                        }
                    if ( iSdpParser->VideoControlAddr().Length() && 
                         rtpInfo.iSecondURL.Length() &&
                         videoAddr.Find( rtpInfo.iSecondURL ) >= 0 )
                        {
                        iRTPTimeStampVideo = rtpInfo.iSecondTS ? rtpInfo.iSecondTS : 1;
                        iSeqFromRtpInfoForVideo = rtpInfo.iSecondSeq;
                        }
                    if ( iSdpParser->AudioControlAddr().Length() && 
                         rtpInfo.iFirstURL.Length() &&
                         audioAddr.Find( rtpInfo.iFirstURL) >= 0 )
                        {
                        iRTPTimeStampAudio = rtpInfo.iFirstTS ? rtpInfo.iFirstTS : 1;
                        iSeqFromRtpInfoForAudio = rtpInfo.iFirstSeq;
                        }
                    if ( iSdpParser->AudioControlAddr().Length() && 
                         rtpInfo.iSecondURL.Length() &&
                         audioAddr.Find( rtpInfo.iSecondURL) >= 0 )
                        {
                        iRTPTimeStampAudio = rtpInfo.iSecondTS ? rtpInfo.iSecondTS : 1;
                        iSeqFromRtpInfoForAudio = rtpInfo.iSecondSeq;
                        }

                    // ok, if we don't have rtp-info header, we don't know yet.
                    if (  rtpInfo.iFirstURL.Length() == 0 &&
                          rtpInfo.iSecondURL.Length() == 0 )
                        {
                        iNoRtpInfoHeader++;
                        }
                    else    
                        {
                        // We have RTP-info, so control stream is no longer mandatory
                        // Mark control streams as "found"                        
                        StreamFound( EAudioControlStream );
                        StreamFound( EVideoControlStream );
                        //StreamFound( ESubTitleControlStream );
                        
                        iSessionObs.StatusChanged( 
                            MCRPacketSource::ERtpStateSeqAndTSAvailable );
                        }
                    
                    // Live state
                    if ( iResponses[ERTSPPlaySent]->IsLiveStream() || iSdpParser->IsLiveStream() ) 
                        {
                        if ( iObserver )
                            {
                            iObserver->ConnectionStatusChange(
                                iOwningSession.SourceChecksum(),
                                ECRStreamIsLiveStream, KErrNone );
                            }
                        }
                    
                    // Notify seeking
                    if ( iObserver )
                        {
                        iObserver->ConnectionStatusChange(
                            iOwningSession.SourceChecksum(),
                            ECRReadyToSeek, KErrNone );
                        }
                    }
                    break;
                    
            default:
                // by default extract no information
                break;
                }
                
            // Then continue with business:
            SendRtspCommandL(); // will change iStage also
            }
            
        // Authentication needed..
        else if ( iResponses[iStage]->StatusCode() ==  
                  CCRRtspResponse::ERTSPRespUnauthorized || // 401
                  iResponses[iStage]->StatusCode() ==
                  CCRRtspResponse::ERTSPRespProxyAuthenticationRequired )  // 407
            {
            iAuthFailedCount++;
            if ( iUserName && 
                 iUserName->Length() && 
                 iPassword && 
                 iAuthFailedCount == 1 )
                {
                iAuthenticationNeeded = ETrue;
                iAuthType = iResponses[iStage]->AuthenticationTypeL().AllocL();
                iRealm = iResponses[iStage]->RealmL().AllocL();
                iOpaque = iResponses[iStage]->OpaqueL().AllocL();
                iNonce = iResponses[iStage]->NonceL().AllocL();
                SendAuthDescribeL();
                }
            else
                {
                iAuthFailedCount = 0;
                LOG( "CCRRtspPacketSource::ProcessRtspResponseL() Authentication failure !" );
                
                // Cleanup
                iOwningSession.SourceStop();
                if ( iObserver )
                    {
                    iObserver->ConnectionStatusChange(
                        iOwningSession.SourceChecksum(), 
                        ECRAuthenticationNeeded, KErrNone );
                    }
                }
            }
        else if ( iResponses[iStage]->StatusCode() ==  CCRRtspResponse::ERTSPRespUnsupportedTransport )  // 461
            {            
            LOG1( "CCRRtspPacketSource::ProcessRtspResponseL() - Unsupported Transport: %d", iTransport );
                        
            if ( iConnection.GetHeuristic( CCRConnection::EUdpStreamingBlocked ) )
                {
                // Using TCP, change to UDP                                
                LOG( "CCRRtspPacketSource::ProcessRtspResponseL() - Change TCP to UDP" );
                iConnection.SetHeuristic( CCRConnection::EUdpStreamingBlocked, EFalse );                
                // Notify observer at client side:
                ProcessRtspErrorResponseL( iResponses[iStage]->StatusCode() );
                }
            else
                {
                // Using UDP, change to TCP
                LOG( "CCRRtspPacketSource::ProcessRtspResponseL() - Change UDP to TCP");
                iConnection.SetHeuristic( CCRConnection::EUdpStreamingBlocked, ETrue );
                // Notify observer at client side:
                ProcessRtspErrorResponseL( iResponses[iStage]->StatusCode() );
                }
            }
        else
            {
            // before doing cleanup, notify observer at client side:
            ProcessRtspErrorResponseL( iResponses[iStage]->StatusCode() );
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::ProcessRtspErrorResponseL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::ProcessRtspErrorResponseL(
    CCRRtspResponse::TResponseCode aErrorCode )
    {
    SCRQueueEntry entry;
    entry.iMsg = ECRMsgQueueConnectionError;

    switch ( aErrorCode )
        {
        case CCRRtspResponse::ERTSPRespLowOnStorageSpace:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespMultipleChoices:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespMovedPermanently:
            entry.iErr = KErrNotFound;
            break;
        
        case CCRRtspResponse::ERTSPRespMovedTemporarily:
            entry.iErr = KErrNotFound;
            break;
        
        case CCRRtspResponse::ERTSPRespSeeOther:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespNotModified:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespUseProxy:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespBadRequest:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespPaymentRequired:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespForbidden:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespGone:
        case CCRRtspResponse::ERTSPRespConferenceNotFound:
        case CCRRtspResponse::ERTSPRespNotFound:
            entry.iErr = KErrNotFound;
            break;
        
        case CCRRtspResponse::ERTSPRespMethodNotAllowed:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespNotAcceptable:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespRequestTimeOut:
            entry.iErr = KErrTimedOut;
            break;
        
        case CCRRtspResponse::ERTSPRespLengthRequired:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespPreconditionFailed:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespRequestEntityTooLarge:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespRequestURITooLarge:
            entry.iErr = KErrGeneral;
            break;
                
        case CCRRtspResponse::ERTSPRespParameterNotUnderstood:
            entry.iErr = KErrArgument;
            break;
        
        case CCRRtspResponse::ERTSPRespNotEnoughBandwidth:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespSessionNotFound:
            entry.iErr = KErrCouldNotConnect;
            break;
        
        case CCRRtspResponse::ERTSPRespMethodNotValidInThisState:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespHeaderFieldNotValidForResource:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespInvalidRange:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespParameterIsReadOnly:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespAggregateOperationNotAllowed:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespOnlyAggregateOperationAllowed:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespUnsupportedTransport:
            entry.iErr = KErrCouldNotConnect;
            break;
        
        case CCRRtspResponse::ERTSPRespDestinationUnreachable:
            entry.iErr = KErrCouldNotConnect;
            break;
        
        case CCRRtspResponse::ERTSPRespInternalServerError:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespNotImplemented:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespBadGateway:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespServiceUnavailable:
            entry.iErr = KErrCouldNotConnect;
            break;
        
        case CCRRtspResponse::ERTSPRespGatewayTimeOut:
            entry.iErr = KErrGeneral;
            break;
        
        case CCRRtspResponse::ERTSPRespUnsupportedMediaType:
        case CCRRtspResponse::ERTSPRespOptionNotSupported:
        case CCRRtspResponse::ERTSPRespRTSPVersionNotSupported:
            entry.iErr = KErrNotSupported;
            break;
        
        default:
            entry.iErr = KErrGeneral;
            break;
        }

    if ( iObserver )
        {
        iObserver->ConnectionStatusChange(
            iOwningSession.SourceChecksum(), ECRConnectionError, entry.iErr );
        }

    // Try tear down first
    if ( Stop() == KErrDisconnected )
        {
        iOwningSession.SourceStop();
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::StartRtspTimeout
// Starts RTSP command response timeout.
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::StartRtspTimeout( TTimeIntervalMicroSeconds32 aTime )
    {
    // Start a timeout timer to wait for the server to respond.
    // If the server doesn't respond in time, cleanup will be initialized.
    if ( !iRtspTimeout )
        {
        TRAPD( err, iRtspTimeout = 
            CPeriodic::NewL( CActive::EPriorityStandard ) );
        if ( err != KErrNone )
            {
            // Timer creation failed, start cleanup immediately
            iOwningSession.SourceStop();
            }
        }
    else
        {
        iRtspTimeout->Cancel(); 
        }

    // Start timeout timer
    iRtspTimeout->Start( 
        aTime,
        aTime,
        TCallBack( RtspTimeoutCallback, this ) );
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::RtspTimeoutCallback
// Callback for RTSP response timeout. Just ask session to start cleanup
// -----------------------------------------------------------------------------
//
TInt CCRRtspPacketSource::RtspTimeoutCallback( TAny* aPtr )
    {
    LOG( "CCRRtspPacketSource::RtspTimeoutCallback()" );

    CCRRtspPacketSource* self = static_cast<CCRRtspPacketSource*>( aPtr );
    self->iRtspTimeout->Cancel();
    self->iOwningSession.SourceStop();
    return 0;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SendRtspCommandL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::SendRtspCommandL()
    {
    LOG1( "CCRRtspPacketSource::SendRtspCommandL(), iStage: %d", iStage );
    
    if ( iPostPonedPlay )
        {
        iPostPonedPlay = EFalse;
        Play( iStartPos, iEndPos );
        }
    else
        {
        switch ( iStage )
            {
            case ERTSPInit:
            case ERTSPOptSent:
                {
                delete iPrevCommands[ERTSPDescSent];
                iPrevCommands[ERTSPDescSent] = NULL;
                iPrevCommands[ERTSPDescSent] = CCRRtspCommand::NewL();
                iPrevCommands[ERTSPDescSent]->SetCommand( 
                    CCRRtspCommand::ERTSPCommandDESCRIBE );
                
                TPtrC8 uriDes ( iRtspUri8->Des() );
                iPrevCommands[ERTSPDescSent]->SetURL( uriDes );
                iPrevCommands[ERTSPDescSent]->SetCSeq( iCSeq++ );
                if ( iUserAgent )
                    {
                    iPrevCommands[ERTSPDescSent]->SetUserAgentL( *iUserAgent );
                    }
                if ( iWapProfile )
                    {
                    iPrevCommands[ERTSPDescSent]->SetWapProfileL( *iWapProfile );
                    }
                if ( iBandwidth )
                    {
                    iPrevCommands[ERTSPDescSent]->SetBandwidth( iBandwidth );
                    }
                
                if ( iRtspSock )
                    {
                    iRtspSock->SendData( iPrevCommands[ERTSPDescSent]->ProduceL() );
                    StartRtspTimeout( KCRRtspResponseTimeout );
                    iStage = ERTSPDescSent;
                    }
                }
                break;
            
            case ERTSPDescSent:
                if ( iSdpParser )
                    {
                    const TInt audio( iSdpParser->MediaIdentifierAudio() );
                    const TInt video( iSdpParser->MediaIdentifierVideo() );
                    TBool videoExists( iSdpParser->VideoControlAddr().Length() > 0 );
                    TBool audioExists( iSdpParser->AudioControlAddr().Length() > 0 );
                    
                    /* If both medias are reported with dynamic payload 
                     * type and audio stream is reported with lower 
                     * payload type, then some servers don't work correctly 
                     * if the SETUP commands are not in correct order, ie.
                     * we need to first SETUP the audio stream here.
                     */
                    const TBool audioBeforeVideo(
                        audioExists && audio >= 96 && video >= 96 && audio < video );

                    if ( videoExists && !audioBeforeVideo )
                        {
                        SendSetupCommandL( iSdpParser->VideoControlAddr(), EFalse );
                        iStage = ERTSPSetupVideoSent;
                        }
                    else if ( audioExists )
                        {
                        SendSetupCommandL( iSdpParser->AudioControlAddr(), ETrue );
                        iStage = ERTSPSetupAudioSent;
                        }
                    else
                        {
                        LOG1( "CCRRtspPacketSource::SendRtspCommand stag %d have no audio nor video",
                            ( TInt )iStage );
                        // no audio, no video, el panique grande
                        iOwningSession.SourceStop();
                        }
                    }
                break;
                
            case ERTSPSetupAudioSent:
                {
                const TInt audio( iSdpParser->MediaIdentifierAudio() );
                const TInt video( iSdpParser->MediaIdentifierVideo() );
                
                if ( audio >= 96 && video >= 96 && audio < video &&
                     iSdpParser && iSdpParser->VideoControlAddr().Length() )
                    {
                    // Video exists also and has not been setup before, so 
                    // let's setup it now.
                    
                    TPtrC8 ctrlAddr ( iSdpParser->VideoControlAddr() );
                    SendSetupCommandL( ctrlAddr, EFalse );
                    iStage = ERTSPSetupVideoSent;
                    }
                else
                    {
                    ConditionallySetupMultiCastOrTcpStreamingL();
                    }
                }
                break;
                
            case ERTSPSetupVideoSent:
                {
                const TInt audio( iSdpParser->MediaIdentifierAudio() );
                const TInt video( iSdpParser->MediaIdentifierVideo() );
                
                // Check explanation for this in case ERTSPDescSent above.
                const TBool audioBeforeVideo(
                    audio >= 96 && video >= 96 && audio < video );
                
                // Then send audio, if applicable:
                if ( iSdpParser && iSdpParser->AudioControlAddr().Length() &&
                     !audioBeforeVideo )
                    {
                    TPtrC8 ctrlAddr ( iSdpParser->AudioControlAddr() );
                    SendSetupCommandL( ctrlAddr, ETrue );
                    iStage = ERTSPSetupAudioSent;
                    }
                else
                    { 
                    // there is no audio that need setup so lets check also multicast+tcp
                    ConditionallySetupMultiCastOrTcpStreamingL();                  
                    }
                }
                break;
            
            case ERTSPPauseSent:
                // If we're paused, do zero the buffer, in tcp streaming case
                // some servers seem to send packets even after play..
                break; 

            case ERTSPReadyToPlay:
                // In these stages send no further commands
                break;
            
            case ERTSPPlaySent:
                // Start timer for UDP reception and start streaming
                if ( iTransport == ERTPOverUDP )
                    {
                    iUdpReceptionTimer->Cancel();
                    iUdpReceptionTimer->After( KCRRtspRtpUdpTimeout );
                    }
                
                iStage = ERTSPPlaying;
                if ( !iNoRtpInfoHeader )
                    {
                    iSessionObs.StatusChanged( MCRPacketSource::ERtpStatePlaying );
                    }
                break;
            
            case ERTSPPlaying:
                // None
                break;
            
            case ERTSPTearDownSent:
                iPostPonedPlay = EFalse; 
                iOwningSession.SourceStop();
                break;

            default:
                // By default send no further commands
                break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SendPlayCommandL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::SendPlayCommandL(void)
    {
    delete iPrevCommands[ERTSPPlaySent];
    iPrevCommands[ERTSPPlaySent] = NULL;
    iPrevCommands[ERTSPPlaySent] = CCRRtspCommand::NewL();
    iPrevCommands[ERTSPPlaySent]->SetCommand ( CCRRtspCommand::ERTSPCommandPLAY );
    TPtrC8 uriDes( iRtspUri8->Des() );
    iPrevCommands[ERTSPPlaySent]->SetURL( uriDes );
    iPrevCommands[ERTSPPlaySent]->SetCSeq( iCSeq ++ );
    iPrevCommands[ERTSPPlaySent]->SetRange( iStartPos , iEndPos );
    
    if ( iUserAgent )
        {
        iPrevCommands[ERTSPPlaySent]->SetUserAgentL( *iUserAgent );
        }
    if ( iSessionId.Ptr() )
        {
        iPrevCommands[ERTSPPlaySent]->SetSessionId( iSessionId );
        }
    if ( iAuthenticationNeeded )
        {
        AddAuthenticationL( ERTSPPlaySent );
        }

    if ( iRtspSock ) 
        {
        iRtspSock->SendData( iPrevCommands[ERTSPPlaySent]->ProduceL() );
        StartRtspTimeout( KCRRtspResponseTimeout );
        iStage = ERTSPPlaySent;
        }

    iStartPos = KRealZero;
    iEndPos = KRealMinusOne;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SendPauseCommandL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::SendPauseCommandL(void)
    {
    delete iPrevCommands[ERTSPPauseSent];
    iPrevCommands[ERTSPPauseSent] = NULL;
    iPrevCommands[ERTSPPauseSent] = CCRRtspCommand::NewL();
    iPrevCommands[ERTSPPauseSent]->SetCommand ( CCRRtspCommand::ERTSPCommandPAUSE );
    TPtrC8 uriDes( iRtspUri8->Des() );
    iPrevCommands[ERTSPPauseSent]->SetURL( uriDes );
    iPrevCommands[ERTSPPauseSent]->SetCSeq( iCSeq ++ );
    
    if ( iUserAgent )
        {
        iPrevCommands[ERTSPPauseSent]->SetUserAgentL( *iUserAgent );
        }
    if ( iSessionId.Ptr() )
        {
        iPrevCommands[ERTSPPauseSent]->SetSessionId( iSessionId );
        }
    if ( iAuthenticationNeeded )
        {
        AddAuthenticationL( ERTSPPauseSent );
        }
    
    if ( iRtspSock )
        {
        iRtspSock->SendData( iPrevCommands[ERTSPPauseSent]->ProduceL() );
        StartRtspTimeout( KCRRtspResponseTimeout );
        iStage = ERTSPPauseSent;
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SendSetupCommandL
// -----------------------------------------------------------------------------
//
TInt CCRRtspPacketSource::SendSetupCommandL(
    const TDesC8& aControlAddr,
    TBool aForAudio )
    {
    TCRRTSPStage newStage = aForAudio ? ERTSPSetupAudioSent : ERTSPSetupVideoSent;

    delete iPrevCommands[newStage];
    iPrevCommands[newStage] = NULL;
    iPrevCommands[newStage] = CCRRtspCommand::NewL();
    iPrevCommands[newStage]->SetCommand ( CCRRtspCommand::ERTSPCommandSETUP );
    iPrevCommands[newStage]->SetURL( aControlAddr );
    iPrevCommands[newStage]->SetCSeq( iCSeq ++ );
    iPrevCommands[newStage]->SetTransport( iTransport );

    // Map stream to port number (when streaming over UDP) or channel (over TCP)
    // base: iClientPort for UDP, 0 for TCP
    // video: (base+0, base+1)
    // audio: (base+2, base+3) or (base+0, base+1) when audio only
    TInt portbase( ( iTransport == ERTPOverUDP )? iClientPort: 0 );
    TInt portoffset( ( aForAudio && iSdpParser->VideoControlAddr().Length() )? 2: 0 );
    iPrevCommands[newStage]->SetClientPort( portbase + portoffset );

    if ( iSessionId.Ptr() )
        {
        iPrevCommands[newStage]->SetSessionId ( iSessionId );
        }
    if ( iAuthenticationNeeded )
        {
        AddAuthenticationL( newStage );
        }
    if ( iUserAgent )
        {
        iPrevCommands[newStage]->SetUserAgentL( *iUserAgent );
        }
    if ( iWapProfile )
        {
        iPrevCommands[newStage]->SetWapProfileL( *iWapProfile );
        }

    if ( iRtspSock ) 
        {
        iRtspSock->SendData( iPrevCommands[newStage]->ProduceL() );
        StartRtspTimeout( KCRRtspResponseTimeout );
        }
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SendTearDownCommandL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::SendTearDownCommandL()
    {
    CCRRtspCommand*& teardowncmd = iPrevCommands[ERTSPTearDownSent];
    iPostPonedPlay = EFalse; 
    if ( teardowncmd )
        {
        delete teardowncmd; teardowncmd = NULL;
        }

    teardowncmd = CCRRtspCommand::NewL();
    teardowncmd->SetCommand( CCRRtspCommand::ERTSPCommandTEARDOWN );
    TPtrC8 uri( iRtspUri8->Des() );
    teardowncmd->SetURL( uri );
    teardowncmd->SetCSeq( iCSeq++ );

    if ( iSessionId.Ptr() )
        {
        teardowncmd->SetSessionId( iSessionId );
        }
    if ( iUserAgent )
        {
        teardowncmd->SetUserAgentL( *iUserAgent );
        }
    if ( iAuthenticationNeeded )
        {
        AddAuthenticationL( ERTSPTearDownSent );
        }
    
    if ( iRtspSock )
        {
        iRtspSock->SendData( teardowncmd->ProduceL() );
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SendOptionsCommandL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::SendOptionsCommandL(void)
    {
    delete iPrevCommands[ERTSPOptSent];
    iPrevCommands[ERTSPOptSent] = NULL;
    iPrevCommands[ERTSPOptSent] = CCRRtspCommand::NewL();
    iPrevCommands[ERTSPOptSent]->SetCommand ( CCRRtspCommand::ERTSPCommandOPTIONS );
    TPtrC8 uriDes ( iRtspUri8->Des() );
    iPrevCommands[ERTSPOptSent]->SetURL ( uriDes );
    iPrevCommands[ERTSPOptSent]->SetCSeq ( iCSeq ++ );
    
    if ( iUserAgent )
        {
        iPrevCommands[ERTSPOptSent]->SetUserAgentL( *iUserAgent );
        }
    if ( iSessionId.Ptr() )
        {
        iPrevCommands[ERTSPOptSent]->SetSessionId ( iSessionId );
        }
    if ( iAuthenticationNeeded )
        {
        AddAuthenticationL( ERTSPOptSent );
        }

    if ( iRtspSock )
        {
        iRtspSock->SendData( iPrevCommands[ERTSPOptSent]->ProduceL() );
        }
    // Sending options ping does not change our state
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SetupRTPSessions
// -----------------------------------------------------------------------------
//
TInt CCRRtspPacketSource::SetupRTPSessions( void )
    {
    TInt retval( KErrNone );
    if ( !iRtspSock )
        {
        retval = KErrNotReady;
        }
    else
        {
        TInetAddr localAddr( iRtspSock->LocalAddr() );
        TInetAddr remoteAddr( iRtspSock->ConnectedAddr() );
        
        // Clear used streams
        iReceiveStreams.Reset();
        iTrafficFound = EFalse;
        
        // First audio:
        if ( iRtspSock && iResponses[ERTSPSetupAudioSent] )
            {
            if ( iTransport == ERTPOverMulticast )
                {
                retval = CreateMulticastSocket( ERTPAudioSend1, 
                    iResponses[ERTSPSetupAudioSent]->Destination(),
                    iResponses[ERTSPSetupAudioSent]->ClientPort() );
                if ( retval == KErrNone )
                    {
                    retval = CreateMulticastSocket( ERTPAudioSend2, 
                        iResponses[ERTSPSetupAudioSent]->Destination(),
                        iResponses[ERTSPSetupAudioSent]->ClientPort()+1 );
                    }
                }
            else
                {
                localAddr.SetPort( iResponses[ERTSPSetupAudioSent]->ClientPort() );
                remoteAddr.SetPort( iResponses[ERTSPSetupAudioSent]->ServerPort() );

#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE )
                TName _addr;
                localAddr.Output( _addr );
                LOG2( "localaddr for video is %S:%d", &_addr, localAddr.Port() );
                remoteAddr.Output( _addr );
                LOG2( "remoteAddr for video is %S:%d", &_addr, remoteAddr.Port() );
#endif // LIVE_TV_FILE_TRACE || LIVE_TV_RDEBUG_TRACE

                retval = CreateUnicastSocket( ERTPAudioSend1, localAddr, remoteAddr );
                if ( retval == KErrNone )
                    {
                    localAddr.SetPort( localAddr.Port()+1 );
                    remoteAddr.SetPort( remoteAddr.Port()+1 );
                    retval = CreateUnicastSocket( ERTPAudioSend2, localAddr, remoteAddr );
                    }
                }

            if ( retval == KErrNone )
                {
                TRAP( retval, iAudioSession.OpenL(
                    iRTPSockArr[ERTPAudioSend1]->Socket(),
                    KAverageExpectedRtpPacketMaxSize,
                    iRTPSockArr[ERTPAudioSend2]->Socket(),
                    EPriorityNormal, KCRCName() ) );
                }

            LOG1( "CCRRtspPacketSource::SetupRTPSessions audio sess open: %d", retval );
            if ( !retval )
                {
                SetRtpSession( iAudioSession , iSdpParser->AudioTimerGranularity() );
                iAudioSession.SetBandwidth( iSdpParser->AudioBitrate() * 1000 ); 
                TRAP( retval, iAudioSession.PrivRegisterEventCallbackL( ERtpNewSource, 
                    ( TRtpCallbackFunction )CCRRtspPacketSource::AudioRTPCallBack, this ) );

                TReceiveStream audioDataStream;
                audioDataStream.iStreamType = EAudioStream;
                audioDataStream.iDataReceived = EFalse;                
                iReceiveStreams.Append( audioDataStream );
                LOG( "CCRRtspPacketSource::SetupRTPSessions - AudioStream found" );
                TReceiveStream audioControlStream;
                audioControlStream.iStreamType = EAudioControlStream;
                audioControlStream.iDataReceived = EFalse;
                LOG( "CCRRtspPacketSource::SetupRTPSessions - AudioControlStream found" );
                iReceiveStreams.Append( audioControlStream );
                
                LOG2( "CCRRtspPacketSource::SetupRTPSessions audio stat: %d, ts: %u",
                    retval, ( TUint )iRTPTimeStampAudio );
                }
            else
                {
                if ( iObserver )
                    {
                    iObserver->ConnectionStatusChange(
                        iOwningSession.SourceChecksum(),
                        ECRConnectionError, retval );
                    }
                iOwningSession.SourceStop();                    
                }
            }

        // Then video
        if ( retval == KErrNone && iRtspSock && iResponses[ERTSPSetupVideoSent] )
            {
            if ( iTransport==ERTPOverMulticast )
                {
                retval = CreateMulticastSocket( ERTPVideoSend1, 
                    iResponses[ERTSPSetupVideoSent]->Destination(),
                    iResponses[ERTSPSetupVideoSent]->ClientPort() );
                if ( retval==KErrNone )
                    {
                    retval = CreateMulticastSocket( ERTPVideoSend2, 
                        iResponses[ERTSPSetupVideoSent]->Destination(),
                        iResponses[ERTSPSetupVideoSent]->ClientPort()+1 );
                    }
                }
            else
                {
                localAddr.SetPort( iResponses[ERTSPSetupVideoSent]->ClientPort() );
                remoteAddr.SetPort( iResponses[ERTSPSetupVideoSent]->ServerPort() );

#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE )
                TName _addr;
                localAddr.Output( _addr );
                LOG2( "localaddr for video is %S:%d", &_addr, localAddr.Port() );
                remoteAddr.Output( _addr );
                LOG2( "remoteAddr for video is %S:%d", &_addr, remoteAddr.Port() );
#endif // LIVE_TV_FILE_TRACE || LIVE_TV_RDEBUG_TRACE

                retval = CreateUnicastSocket( ERTPVideoSend1, localAddr, remoteAddr );
                if ( retval == KErrNone )
                    {
                    localAddr.SetPort( localAddr.Port() + 1 );
                    remoteAddr.SetPort( remoteAddr.Port() + 1 );
                    retval = CreateUnicastSocket( ERTPVideoSend2, localAddr, remoteAddr );
                    }
                }

            if ( retval == KErrNone )
                {
                TRAP( retval, iVideoSession.OpenL( iRTPSockArr[ERTPVideoSend1]->Socket(),
                    KAverageExpectedRtpPacketMaxSize, iRTPSockArr[ERTPVideoSend2]->Socket(),
                    EPriorityNormal, KCRCName() ) );
                }

            LOG1( "CCRRtspPacketSource::SetupRTPSessions video sess open: %d", retval );
            if ( !retval )
                {
                SetRtpSession( iVideoSession , iSdpParser->VideoTimerGranularity() );
                iVideoSession.SetBandwidth( iSdpParser->VideoBitrate() * 1000 ); 
                TRAP( retval, iVideoSession.PrivRegisterEventCallbackL( ERtpNewSource,
                    ( TRtpCallbackFunction )CCRRtspPacketSource::VideoRTPCallBack, this ) );

                TReceiveStream videoDataStream;
                videoDataStream.iStreamType = EVideoStream;
                videoDataStream.iDataReceived = EFalse;
                LOG( "CCRRtspPacketSource::SetupRTPSessions - VideoStream found" );
                iReceiveStreams.Append( videoDataStream );
                TReceiveStream videoControlStream;
                videoControlStream.iStreamType = EVideoControlStream;
                videoControlStream.iDataReceived = EFalse;
                LOG( "CCRRtspPacketSource::SetupRTPSessions - VideoControlStream found" );
                iReceiveStreams.Append( videoControlStream );
                    
                LOG2( "CCRRtspPacketSource::SetupRTPSessions video stat: %d, ts: %u",
                    retval, ( TUint )iRTPTimeStampVideo );
                }
            else
                {
                if ( iObserver )
                    {
                    iObserver->ConnectionStatusChange(
                        iOwningSession.SourceChecksum(),
                        ECRConnectionError, retval );
                    }
                iOwningSession.SourceStop();                    
                }
            }
        }
    
    return retval;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::CreateMulticastSocket
// -----------------------------------------------------------------------------
//
TInt CCRRtspPacketSource::CreateMulticastSocket(
    TCRRTPSockId aSockId, 
    const TInetAddr& aGroupAddr,
    TInt aPort )
    {
    // Alias for socket being created
    CCRSock*& sock = iRTPSockArr[aSockId];

    // Delete if already existing
    if ( sock )
        {
        delete sock;
        sock = NULL;
        }

    // Create socket
    TRAPD( err, sock = CCRSock::NewL( *this, aSockId, iConnection.Connection(),
                                      iSockServer, EFalse, EFalse) );
    if ( err != KErrNone )
        {
        LOG2( "CCRRtspPacketSource::CreateMulticastSocket: CCRSock::NewL FAILED, sockId: %d, err: %d",
            aSockId, err );
        return err;
        }

    // Bind socket to local UDP port, issue no reads -> handled by RRtpSession
    err = sock->ListenPort( aPort );
    if ( err != KErrNone )
        {
        LOG2( "CCRRtspPacketSource::CreateMulticastSocket: ListenPort FAILED, port: %d, err: %d",
            aPort, err );
        return err;
        }

    err = sock->JoinGroup( aGroupAddr );
    if ( err != KErrNone )
        {
        LOG1( "CCRRtspPacketSource::CreateMulticastSocket: JoinGroup FAILED, err: %d", err );
        return err;
        }

#if defined(LIVE_TV_FILE_TRACE) || defined(LIVE_TV_RDEBUG_TRACE)
    TName group;
    aGroupAddr.Output( group );
    LOG3( "CCRRtspPacketSource::CreateMulticastSocket: sockid: %d, group: '%S', port: %d OK",
        aSockId, &group, aPort );
#endif // LIVE_TV_FILE_TRACE || LIVE_TV_RDEBUG_TRACE

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::CreateUnicastSocket
// -----------------------------------------------------------------------------
TInt CCRRtspPacketSource::CreateUnicastSocket(
    TCRRTPSockId aSockId, 
    const TInetAddr& aLocalAddr,
    const TInetAddr& /*aRemoteAddr*/ )
    {
    // Alias for socket being created
    CCRSock*& sock = iRTPSockArr[aSockId];

    // Delete if already existing
    if ( sock )
        {
        delete sock;
        sock = NULL;
        }

    // Create socket: EFalse=UDP, EFalse=issue no read (handled by RRtpSession)
    TRAPD( err, sock = CCRSock::NewL( *this,aSockId, iConnection.Connection(),
                                      iSockServer, EFalse, EFalse ) );
    if ( err != KErrNone )
        {
        LOG2( "CCRRtspPacketSource::CreateUnicastSocket: CCRSock::NewL FAILED, sockId: %d, err: %d",
            aSockId, err );
        return err;
        }

    // Bind to local port, ignore remote address and port
    TInt port = aLocalAddr.Port();
    err = sock->ListenPort( port );
    if ( err != KErrNone )
        {
        LOG2( "CCRRtspPacketSource::CreateUnicastSocket: ListenPort FAILED, port: %d, err: %d",
            port, err );
        return err;
        }

#if defined(LIVE_TV_FILE_TRACE) || defined(LIVE_TV_RDEBUG_TRACE)
    LOG2( "CCRRtspPacketSource::CreateUnicastSocket: sockid: %d, port: %d OK",
        aSockId, port );
#endif // LIVE_TV_FILE_TRACE || LIVE_TV_RDEBUG_TRACE

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::RTPPayloadProcessor
// This is called from audio and video callbacks when real payload packet
// is received from rtp stack.
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::RTPPayloadProcessor(
    const TRtpEvent& aEvent,
    const TBool aIsAudio )
    {
    // If udp traffic hasn't been flagged as found
    // keep marking streams as found
    if ( !iTrafficFound )
        {            
        if ( aIsAudio )
            {
            StreamFound( EAudioStream );        
            }
        else
            {
            StreamFound( EVideoStream );        
            }

        // Cancel UDP timer, so as not to trigger TCP streaming
        if ( CheckReceiveOfStreams() )
            {
            // We have traffic from all needed streams, cancel reception timer
            // and set UDP flag.            
            iUdpReceptionTimer->Cancel();
            iUdpFound = ETrue;
            iTrafficFound = ETrue;
            }
        }    

    // Here process packet
    RRtpReceivePacket p = aEvent.ReceiveSource().Packet();
    TUint32 flag( 0 );
    BigEndian::Put32( ( TUint8* )&flag, p.Flags() );

    // Header
    TCRRtpMessageHeader packetHeader;
    memcpy( &packetHeader, &flag, sizeof( flag ) );
    BigEndian::Put32( ( TUint8* )&packetHeader.iTimestamp, p.Timestamp() );
    BigEndian::Put32( ( TUint8* )&packetHeader.iSSRC, p.SSRC() );
    TPtrC8 rtpHeader( ( TUint8* )&packetHeader, sizeof( packetHeader ) );

    if ( iNoRtpInfoHeader )
        {
        ConstructSeqAndTsForSink(
            aIsAudio ? MCRPacketSource::EAudioStream : MCRPacketSource::EVideoStream,
            0 /*nop*/, 0 /*nop*/, 0 /*nop*/, p.SequenceNumber() );
        }

    // Stream
    MCRPacketSource::TCRPacketStreamId stream( 
        ( aIsAudio )? MCRPacketSource::EAudioStream : 
                      MCRPacketSource::EVideoStream );
    iBuffer->AddPacket( stream, rtpHeader, p.Payload() );
    
    // Count of packets
    if ( aIsAudio )
        {
        iAudioBytes += p.Payload( ).Length();
        iAudioPackets ++;
        }
    else
        {
        iVideoBytes += p.Payload( ).Length();
        iVideoPackets ++;
        }

    p.Close();
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::AudioRTPCallBack
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::AudioRTPCallBack(
    CCRRtspPacketSource* aPtr,
    const TRtpEvent& aEvent )
    {
    switch ( aEvent.Type() )
        {
        case ERtpPacketReceived:
            static_cast<CCRRtspPacketSource*>( aPtr )->
                RTPPayloadProcessor( aEvent, ETrue );
            break;

        // RTCP
        case ERtpSR:
            {
            // We have audio control traffic
            if ( !aPtr->iTrafficFound )
                {                    
                aPtr->StreamFound( EAudioControlStream );
                if ( aPtr->CheckReceiveOfStreams() )
                    {                    
                    // Cancel UDP timer, so as not to trigger TCP streaming
                    aPtr->iUdpReceptionTimer->Cancel();
                    aPtr->iUdpFound = ETrue;
                    aPtr->iTrafficFound = ETrue;
                    }
                }

            // Sender report
            SenderReport( aPtr, aEvent, MCRPacketSource::EAudioControlStream );
            }
            break;

        case ERtpNewSource:
            {
            // Handle audio 
            TRAPD( err, HandleNewSourceL( aPtr, aPtr->iRtpRecvSrcAudio, aEvent,
                ( TRtpCallbackFunction )CCRRtspPacketSource::AudioRTPCallBack ) );
            if ( err )
                {
                LOG1( "CCRRtspPacketSource::AudioRTPCallBack(), HandleNewSourceL Leaved: %d", err );
                aPtr->iOwningSession.SourceStop();
                }
            }
            break;

        case ERtpSessionFail:
        case ERtpSourceFail:
            LOG( "CCRRtspPacketSource::VideoRTPCallBack(), source/session fail" );
            aPtr->iOwningSession.SourceStop();
            if ( aPtr->iObserver )
                {
                aPtr->iObserver->ConnectionStatusChange( 
                     aPtr->iOwningSession.SourceChecksum(),
                     ECRNormalEndOfStream, KErrSessionClosed );
                }
            break;

        case ERtpBYE:
            LOG( "CCRRtspPacketSource::AudioRTPCallBack(), ERtpBYE" );
            if ( aPtr->iObserver )
                {
                aPtr->iObserver->ConnectionStatusChange( 
                     aPtr->iOwningSession.SourceChecksum(), 
                     ECRNormalEndOfStream, KErrNone );
                }
            break;
        
        default:
            LOG1( "CCRRtspPacketSource::AudioRTPCallBack default case, type 0x%x",
                ( TUint )( aEvent.Type() ) );
            // by do nothing
            break;
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::VideoRTPCallBack
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::VideoRTPCallBack(
    CCRRtspPacketSource* aPtr,
    const TRtpEvent& aEvent )
    {
    switch ( aEvent.Type() )
        {
        case ERtpPacketReceived:
            static_cast<CCRRtspPacketSource*>( aPtr )->
                RTPPayloadProcessor( aEvent, EFalse );
            break;

        // RTCP
        case ERtpSR:
            {
            // We have video control traffic
            if ( !aPtr->iTrafficFound )
                {                    
                aPtr->StreamFound( EVideoControlStream );
                if ( aPtr->CheckReceiveOfStreams() )
                    {                    
                    // Cancel UDP timer, so as not to trigger TCP streaming
                    aPtr->iUdpReceptionTimer->Cancel();
                    aPtr->iUdpFound = ETrue;
                    aPtr->iTrafficFound = ETrue;
                    }
                }

            // Sender report
            SenderReport( aPtr, aEvent, MCRPacketSource::EVideoControlStream );
            }
            break;

        case ERtpNewSource:
            {
            // Handle video
            TRAPD( err, HandleNewSourceL( aPtr, aPtr->iRtpRecvSrcVideo, aEvent,
                ( TRtpCallbackFunction )CCRRtspPacketSource::VideoRTPCallBack ) );
            if ( err )
                {
                LOG1( "CCRRtspPacketSource::VideoRTPCallBack(), HandleNewSourceL Leaved: %d", err );
                aPtr->iOwningSession.SourceStop();
                }
            }
            break;
        
        case ERtpSessionFail:
        case ERtpSourceFail:
            LOG( "CCRRtspPacketSource::VideoRTPCallBack(), Source/session fail" );
            aPtr->iOwningSession.SourceStop();
            if ( aPtr->iObserver )
                {
                aPtr->iObserver->ConnectionStatusChange( 
                     aPtr->iOwningSession.SourceChecksum(),
                     ECRNormalEndOfStream, KErrSessionClosed );
                }
            break;

        case ERtpBYE:
            LOG( "CCRRtspPacketSource::VideoRTPCallBack(), ERtpBYE" );
            if ( aPtr->iObserver )
                {
                aPtr->iObserver->ConnectionStatusChange( 
                     aPtr->iOwningSession.SourceChecksum(), 
                     ECRNormalEndOfStream, KErrNone );
                }
            break;

        default:
            LOG1( "CCRRtspPacketSource::VideoRTPCallBack default case, type 0x%x",
                ( TUint )( aEvent.Type() ) );
            // By do nothing
            break;
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SenderReport
// rfc-1305:
// NTP timestamps are represented as a 64-bit unsigned fixed-
// point number, in seconds relative to 0h on 1 January 1900.
// The integer part is in the first 32 bits and the fraction
// part in the last 32 bits.
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::SenderReport(
    CCRRtspPacketSource* aPtr,
    const TRtpEvent& aEvent,
    MCRPacketSource::TCRPacketStreamId aStreamId )
    {
    TCRRtpSRReportHeader srReport;
    srReport.iVersion = KRtpPacketVersion; // value is 2
    srReport.iPadding = 0;
    srReport.iReportCount = 0;
    srReport.iPacketType = KSenderReportPacketType;
    RRtpReceiveSource source( aEvent.ReceiveSource() );
    BigEndian::Put16( ( TUint8* )&srReport.iLength, 6 );
    BigEndian::Put32( ( TUint8* )&srReport.iSenderSSRC,
        source.SSRC() );
    BigEndian::Put32( ( TUint8* )&srReport.iMSWTimestamp,
        source.GetSR().iSrPtr.ntp_sec );
    BigEndian::Put32( ( TUint8* )&srReport.iLSWTimestamp,
        source.GetSR().iSrPtr.ntp_frac );
    BigEndian::Put32( ( TUint8* )&srReport.iRTPTimestamp, 
        source.GetSR().RTPTimestamp() );
    BigEndian::Put32( ( TUint8* )&srReport.iSenderPacketCount,
        aPtr->iAudioPackets );
    BigEndian::Put32( ( TUint8* )&srReport.iSenderOctetCount,
        aPtr->iAudioBytes );
    TPtrC8 rtcpHeader( ( TUint8* )&srReport, sizeof( srReport ) );
    aPtr->iBuffer->AddPacket( aStreamId, rtcpHeader );

    // Verify Seq and Ts 
    if ( aPtr->iNoRtpInfoHeader )
        {
        aPtr->ConstructSeqAndTsForSink (
            aStreamId,
            source.GetSR().iSrPtr.ntp_sec,
            source.GetSR().iSrPtr.ntp_frac,
            source.GetSR().RTPTimestamp(),
            0 ); // 0 not used
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::HandleNewSourceL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::HandleNewSourceL(
    CCRRtspPacketSource* aPtr,
    RRtpReceiveSource& aSource,
    const TRtpEvent& aEvent,
    TRtpCallbackFunction aCallback )
    {
    // Cancel UDP timer, so as not to trigger TCP streaming
    aPtr->iUdpReceptionTimer->Cancel();
    delete aPtr->iPunchPacketSenderAudio;
    aPtr->iPunchPacketSenderAudio = NULL;
    if ( aSource.IsOpen() )
        {
        aSource.Close();
        }

    // Source
    aSource = aEvent.Session().NewReceiveSourceL();
    aSource.PrivRegisterEventCallbackL( ERtpPacketReceived, aCallback, aPtr );
    aSource.PrivRegisterEventCallbackL( ERtpSR, aCallback, aPtr );
    aSource.PrivRegisterEventCallbackL( ERtpBYE, aCallback, aPtr );
    aSource.PrivRegisterEventCallbackL( ERtpSessionFail, aCallback, aPtr );
    aSource.PrivRegisterEventCallbackL( ERtpSourceFail, aCallback, aPtr );
    
    // Ping Timer
    if ( !aPtr->iRtspPingTimer )
        {
        aPtr->iRtspPingTimer = CPeriodic::NewL( CActive::EPriorityLow );
        aPtr->iRtspPingTimer->Start(
            KDVR10Seconds, 2 * KDVR10Seconds, TCallBack( SendRtspPing, aPtr ) );
        }

    aEvent.Session().SendAPPL( KCRCName() );
    aEvent.Session().SetRTCPAutoSend( ETrue );
    }
    
// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SendAuthDescribeL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::SendAuthDescribeL( )
    {
    delete iPrevCommands[ERTSPDescSent];
    iPrevCommands[ERTSPDescSent] = NULL;
    iPrevCommands[ERTSPDescSent] = CCRRtspCommand::NewL();
    iPrevCommands[ERTSPDescSent]->SetCommand (
        CCRRtspCommand::ERTSPCommandDESCRIBE );
    TPtrC8 uriDes ( iRtspUri8->Des() );
    iPrevCommands[ERTSPDescSent]->SetURL ( uriDes );
    iPrevCommands[ERTSPDescSent]->SetCSeq ( iCSeq ++ );
    
    if ( iAuthType )
        {
        iPrevCommands[ERTSPDescSent]->SetAuthenticationTypeL( iAuthType->Des() );
        }
    if ( iNonce )
        {
        iPrevCommands[ERTSPDescSent]->SetNonceL( iNonce->Des() );
        }
    if ( iRealm )
        {
        iPrevCommands[ERTSPDescSent]->SetRealmL( iRealm->Des() );
        }
    if ( iOpaque )
        {
        iPrevCommands[ERTSPDescSent]->SetOpaqueL( iOpaque->Des() );
        }
    if ( iUserAgent )
        {
        iPrevCommands[ERTSPDescSent]->SetUserAgentL( *iUserAgent );
        }
    if ( iWapProfile )
        {
        iPrevCommands[ERTSPDescSent]->SetWapProfileL( *iWapProfile );
        }
    if ( iBandwidth )
        {
        iPrevCommands[ERTSPDescSent]->SetBandwidth( iBandwidth );
        }
    
    iPrevCommands[ERTSPDescSent]->SetUserNameL( iUserName->Des() );
    iPrevCommands[ERTSPDescSent]->SetPassWdL( iPassword->Des() );
    iPrevCommands[ERTSPDescSent]->SetRtspUriL( iRtspUri->Des() );
    iPrevCommands[ERTSPDescSent]->SetAuthentication ( iAuthenticationNeeded );
    if ( iRtspSock ) 
        {
        iRtspSock->SendData( iPrevCommands[ERTSPDescSent]->ProduceL() );
        StartRtspTimeout( KCRRtspResponseTimeout );
        }
    iStage = ERTSPDescSent;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::AddAuthenticationL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::AddAuthenticationL( TInt aCommand )
    {
    if ( iPrevCommands[aCommand] && iNonce && 
         iOpaque && iUserName && iPassword )
        {
        iPrevCommands[aCommand]->SetAuthenticationTypeL( iAuthType->Des() );
        iPrevCommands[aCommand]->SetNonceL( iNonce->Des() );
        iPrevCommands[aCommand]->SetRealmL( iRealm->Des() );
        iPrevCommands[aCommand]->SetOpaqueL( iOpaque->Des() );
        iPrevCommands[aCommand]->SetUserNameL( iUserName->Des() );
        iPrevCommands[aCommand]->SetPassWdL( iPassword->Des() );
        iPrevCommands[aCommand]->SetRtspUriL( iRtspUri->Des() );
        iPrevCommands[aCommand]->SetAuthentication ( iAuthenticationNeeded );
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::PunchPacketsSent
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::PunchPacketsSent( CCRPunchPacketSender* aPuncher )
    {
    if ( iPunchPacketSenderVideo && aPuncher == iPunchPacketSenderVideo )
        {
        iPunchPacketSentForVideo = ETrue;
        }
    if ( iPunchPacketSenderAudio && aPuncher == iPunchPacketSenderAudio )
        {
        iPunchPacketSentForAudio = ETrue;
        }
    if ( ( iPunchPacketSenderVideo && !iPunchPacketSenderAudio && 
           iPunchPacketSentForVideo ) ||
         ( !iPunchPacketSenderVideo && iPunchPacketSenderAudio &&
           iPunchPacketSentForAudio ) ||
         ( iPunchPacketSenderVideo && iPunchPacketSenderAudio && 
           iPunchPacketSentForVideo && iPunchPacketSentForAudio ) )
        {
        LOG1( "PunchPacketsSent, play readiness: %d", iReadyToPlay );
        SetupSessionsAndPlay();
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SetupSessionsAndPlay
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::SetupSessionsAndPlay()
    {
    // all needed punch packets are sent:
    if ( SetupRTPSessions() != KErrNone )
        {
        iOwningSession.SourceStop();
        }
    else
        {
        // if we're ready to play, play
        if ( iReadyToPlay )
            {
            TRAPD( err, SendPlayCommandL() );
            if ( err != KErrNone )
                {
                iOwningSession.SourceStop();
                }
            }
        else
            {
            iStage = ERTSPReadyToPlay;
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SendPunchPackets
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::SendPunchPacketsL( void )
    {
    LOG( "CCRRtspPacketSource::SendPunchPacketsL in" );
    delete iPunchPacketSenderAudio;
    iPunchPacketSenderAudio = NULL;
    delete iPunchPacketSenderVideo;
    iPunchPacketSenderVideo = NULL;

    if ( iSdpParser &&iRtspSock && iResponses[ERTSPSetupVideoSent] )
        {
        TInetAddr localAddr = iRtspSock->LocalAddr();
        TInetAddr remoteAddr = iRtspSock->ConnectedAddr();
        localAddr.SetPort(iResponses[ERTSPSetupVideoSent]->ClientPort());
        remoteAddr.SetPort(iResponses[ERTSPSetupVideoSent]->ServerPort());
        iPunchPacketSenderVideo = CCRPunchPacketSender::NewL(
            iConnection.Connection(), iSockServer,
            localAddr, remoteAddr, 0, *this );
        }
    if ( iSdpParser && iRtspSock && iResponses[ERTSPSetupAudioSent] )
        {
        TInetAddr localAddr = iRtspSock->LocalAddr();
        TInetAddr remoteAddr = iRtspSock->ConnectedAddr();
        localAddr.SetPort(iResponses[ERTSPSetupAudioSent]->ClientPort());
        remoteAddr.SetPort(iResponses[ERTSPSetupAudioSent]->ServerPort());
        iPunchPacketSenderAudio = CCRPunchPacketSender::NewL(
            iConnection.Connection(), iSockServer,
            localAddr, remoteAddr, 0, *this );
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::ConnectionStatusChange
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::ConnectionStatusChange(
    TInt /*aSessionId*/,
    TCRConnectionStatus aStatus,
    TInt /* aErr */ )
    {
    switch( aStatus )
        {
        // Connection has gone up or bearer has changed -> check bandwidth
        case ECRBearerChanged:
            {
            LOG( "CCRRtspPacketSource::ConnectionStatusChange: IapUp or IapUp2G" );
            if ( iSdpParser && iObserver )
                {
                // Unknown bitrate or bandwidth are returned as zero. Bitrates in kbit/s
                TInt bitrate( iSdpParser->VideoBitrate() + 
                              iSdpParser->AudioBitrate() );
                TInt bandwidth( iConnection.MaximumBandwidth() / 1000 );
                if ( bitrate > 0 && bandwidth > 0 && bandwidth < bitrate )
                    {
                    LOG2( "CCRRtspPacketSource::ConnectionStatusChange: clip_bitrate: %d, connection_bandwidth: %d -> NotEnoughBandwidth",
                        bitrate, bandwidth );
                    iObserver->ConnectionStatusChange(
                        iOwningSession.SourceChecksum(), ECRNotEnoughBandwidth, KErrNone );
                    }
                }
            break;
            }

        // Connection has gone down or error occured -> switch back to RTP/UDP transport
        case ECRConnectionError:
        case ECRIapDown:
            {
            LOG( "CCRRtspPacketSource::ConnectionStatusChange: IapDown or ConnectionError -> switch to RTP/UDP streaming" );
            iConnection.SetHeuristic( CCRConnection::EUdpStreamingBlocked, EFalse );
            break;
            }

        // Nothing to do for:
        // ECRConnecting
        // ECRAuthenticationNeeded
        // ECRNotEnoughBandwidth
        // ECRNormalEndOfStream
        default:
            {
            LOG1( "CCRRtspPacketSource::ConnectionStatusChange: unhandled status: %d", aStatus );
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::RegisterConnectionObs
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::RegisterConnectionObs( MCRConnectionObserver* aObserver )
    {
    iObserver = aObserver;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::UnregisterConnectionObs
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::UnregisterConnectionObs( )
    {
    iObserver = NULL;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SetRtpSession
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::SetRtpSession(
    RRtpSession& aSession,
    TReal aGranularity )
    {
    // Unit is 1/second
    __ASSERT_DEBUG( iSdpParser != NULL, User::Panic( _L( "RTSP source" ), KErrBadHandle ) );
    TUint32 howManyNanoSecondsIsOneTick( 
        ( TUint32 )( TReal( 1000000000.0L ) / aGranularity ) );
    LOG1( "CCRRtspPacketSource::SetRtpSession clock tick: %u", howManyNanoSecondsIsOneTick );        
    aSession.SetRTPTimeConversion( 0, howManyNanoSecondsIsOneTick );
    aSession.SetRtpStreamParameters( KDVRMinSequential, // 1
                                     KDVRMaxMisorder,   // 50
                                     KDVRMaxDropOut );  // 3000
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::SendRtspPing
// -----------------------------------------------------------------------------
//
TInt CCRRtspPacketSource::SendRtspPing( TAny* aSelfPtr )
    {
    CCRRtspPacketSource* ptr = static_cast<CCRRtspPacketSource*> ( aSelfPtr );
    TRAPD( err, ptr->SendOptionsCommandL() );
    return err;
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::ConstructSeqAndTsForSink
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::ConstructSeqAndTsForSink ( 
    MCRPacketSource::TCRPacketStreamId aStreamId,
    TUint32 aMSWTimestamp,
    TUint32 aLSWTimestamp,
    TUint32 aRTPTimestamp,
    TUint aSeq ) 
    {
    switch ( aStreamId )
        {
        case EAudioStream:
            if ( iRTPTimeStampAudio )
                {
                iSeqFromRtpInfoForAudio = aSeq;
                if ( iSeqFromRtpInfoForAudio == 0 )
                    {
                    iSeqFromRtpInfoForAudio++;
                    }
                LOG1( "CCRRtspPacketSource::ConstructSeqAndTsForSink(), Audio seq: %d ", ( int )aSeq );  
                // We may declare that we have seq+ts if we're here and have only audio or
                // if we're here and have both audio and video and have also seq for video
                if ( ( iSdpParser->SupportedContent() == CDvrSdpParser::EDvrAudioOnly )  ||
                     ( iSdpParser->SupportedContent() == CDvrSdpParser::EDvrBothAudioAndVideo &&
                       iSeqFromRtpInfoForVideo && iRTPTimeStampVideo ) )
                    {
                    iSessionObs.StatusChanged( 
                        MCRPacketSource::ERtpStateSeqAndTSAvailable );                   
                    iNoRtpInfoHeader = EFalse;  
                    if ( iStage == ERTSPPlaying )
                        {
                        iSessionObs.StatusChanged( 
                            MCRPacketSource::ERtpStatePlaying );                                            
                        }
                    }
                }
            break;
        
        case EAudioControlStream:
            if ( !iMSWTimestamp ) 
                { // no wall clock time yet set
                iMSWTimestamp = aMSWTimestamp; 
                iLSWTimestamp = aLSWTimestamp; 
                iRTPTimeStampAudio = aRTPTimestamp; 
                if ( iRTPTimeStampAudio == 0 ) 
                    {
                    iRTPTimeStampAudio++; 
                    }
                }
            else
                { 
                // Sync audio with video
                TInt64 wallClockOfVideo = MAKE_TINT64 ( iMSWTimestamp , iLSWTimestamp ); 
                TInt64 wallClockOfAudio = MAKE_TINT64 ( aMSWTimestamp , aLSWTimestamp ); 
                // Then figure out the difference. unit is now difficult ; upper 
                // 32 bits contain whole seconds, lower contains fraction
                TInt64 wallClockDifference( wallClockOfVideo - wallClockOfAudio );
                // Now, the aRTPTimestamp has different scale, declared in SDP. 
                // first make one second that has same scale as wallClockDifference
                TInt64 granularity( MAKE_TINT64( 1, 0 ) ); 
                // Then divide that one second with the given granularity. variable
                // granularity will now contain in its low 32 bits the fraction of the
                // second that re-presents one clock tick (e.g. 1/90000 sec for video)
                granularity = granularity / static_cast<TInt64>(
                    iSdpParser->AudioTimerGranularity() ); 
                // Then divide our difference with this fraction of second
                TInt64 wallClockDifferenceGranular = wallClockDifference / granularity;
                // unit of wallClockDifferenceGranular is now 2^32 / granularity             
                TInt32 wallClockDifferenceGranular32 = wallClockDifferenceGranular;
                LOG2( "CCRRtspPacketSource::ConstructSeqAndTsForSink(), Audio ts: %u adjust by: %d",
                    aRTPTimestamp , wallClockDifferenceGranular32 );        
                iRTPTimeStampAudio = aRTPTimestamp + wallClockDifferenceGranular32;                
                if ( iRTPTimeStampAudio == 0 ) 
                    {
                    iRTPTimeStampAudio++; 
                    }
                }
            break;                         
        
        case EVideoStream:
            if ( iRTPTimeStampVideo )
                {
                iSeqFromRtpInfoForVideo = aSeq;
                if ( iSeqFromRtpInfoForVideo == 0 )
                    {
                    iSeqFromRtpInfoForVideo++;
                    }
                LOG1( "CCRRtspPacketSource::ConstructSeqAndTsForSink(), Video seq: %d ",
                    ( int )aSeq );        

                // We may declare that we have seq+ts if we're here and have only video or
                // if we're here and have both and have also seq for video
                if ( ( iSdpParser->SupportedContent() == CDvrSdpParser::EDvrVideoOnly )  ||
                     ( iSdpParser->SupportedContent() == CDvrSdpParser::EDvrBothAudioAndVideo &&
                       iSeqFromRtpInfoForAudio && iRTPTimeStampAudio ) )
                    {
                    iSessionObs.StatusChanged( 
                        MCRPacketSource::ERtpStateSeqAndTSAvailable );                   
                    iNoRtpInfoHeader = EFalse;  
                    if ( iStage == ERTSPPlaying )
                        {
                        iSessionObs.StatusChanged( 
                            MCRPacketSource::ERtpStatePlaying );                                            
                        }
                    }
                }
            break;                         
        
        case EVideoControlStream:
            if ( !iMSWTimestamp ) 
                { // No wall clock time yet set
                iMSWTimestamp = aMSWTimestamp; 
                iLSWTimestamp = aLSWTimestamp; 
                iRTPTimeStampVideo = aRTPTimestamp; 
                if ( iRTPTimeStampVideo == 0 ) 
                    {
                    iRTPTimeStampVideo++; 
                    }               
                }
            else
                { 
                // Sync audio with video
                TInt64 wallClockOfAudio = MAKE_TINT64 ( iMSWTimestamp , iLSWTimestamp ); 
                TInt64 wallClockOfVideo = MAKE_TINT64 ( aMSWTimestamp , aLSWTimestamp ); 
                // Then figure out the difference. unit is now difficult ; upper 
                // 32 bits contain whole seconds, lower contains fraction
                TInt64 wallClockDifference( wallClockOfAudio - wallClockOfVideo );
                // Now, the aRTPTimestamp has different scale, declared in SDP. 
                // first make one second that has same scale as wallClockDifference
                TInt64 granularity( MAKE_TINT64( 1, 0 ) ); 
                // Then divide that one second with the given granularity. variable
                // granularity will now contain in its low 32 bits the fraction of the
                // second that re-presents one clock tick (e.g. 1/90000 sec for video)
                granularity = granularity / static_cast<TInt64>(
                    iSdpParser->VideoTimerGranularity()); 
                // Then divide our difference with this fraction of second
                TInt64 wallClockDifferenceGranular = wallClockDifference / granularity;
                // Unit of wallClockDifferenceGranular is now 2^32 / granularity             
                TInt32 wallClockDifferenceGranular32 = wallClockDifferenceGranular;
                LOG2( "CCRRtspPacketSource::ConstructSeqAndTsForSink(), Video ts: %u adjust by: %d",
                    aRTPTimestamp , wallClockDifferenceGranular32 );        
                iRTPTimeStampVideo = aRTPTimestamp + wallClockDifferenceGranular32;
                if ( iRTPTimeStampVideo == 0 ) 
                    {
                    iRTPTimeStampVideo++; 
                    }               
                }
            break;
        
        default:
            // no thing
            break;
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::ConditionallySetupMultiCastOrTcpStreamingL
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::ConditionallySetupMultiCastOrTcpStreamingL ( void ) 
    {
    // UDP: Punch packets or play sent in ProcessRTSPResponseL, so do nothing.
    if ( iTransport == ERTPOverUDP )
        { 
        }
    // Multicast: no punch packets needed but session setup yes
    else if ( iTransport == ERTPOverMulticast )
        {
        SetupSessionsAndPlay();
        }
    
    // TCP: no punch packets or session, just send PLAY .. but wait for UI
    else if ( iTransport == ERTPOverTCP  )
        {
        if ( iReadyToPlay )
            {
            SendPlayCommandL();
            }
        else
            {
            iStage = ERTSPReadyToPlay;
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspPacketSource::CheckReceiveOfStreams
// -----------------------------------------------------------------------------
//
TBool CCRRtspPacketSource::CheckReceiveOfStreams()
    {
    TBool retVal( ETrue );
    
    // Go through all streams and check that all streams have receive flag on,
    // if not return false.
    for ( TInt i = 0 ; i < iReceiveStreams.Count() ; i++ )
        {
        if ( iReceiveStreams[i].iDataReceived == EFalse )
            {
            LOG1( "CCRRtspPacketSource::CheckReceiveOfStreams - Missing atleast stream %d", iReceiveStreams[i].iStreamType );
            retVal = EFalse;
            break;
            }        
        }
        
    if ( retVal )
        {
        LOG( "CCRRtspPacketSource::CheckReceiveOfStreams - Receiving from all streams!" );
        }
        
    return retVal;    
    }
    
// -----------------------------------------------------------------------------
// CCRRtspPacketSource::StreamFound
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::StreamFound( TCRPacketStreamId aStreamType )
    {
     // Go through streams and find correct stream to set the receive flag.
    for ( TInt i = 0 ; i < iReceiveStreams.Count(); i++ )
        {
        if ( iReceiveStreams[i].iStreamType == aStreamType )
            {
            iReceiveStreams[i].iDataReceived = ETrue;
            LOG1( "CCRRtspPacketSource::StreamFound - Stream %d found", iReceiveStreams[i].iStreamType );            
            break;
            }
        }
    }
    
// -----------------------------------------------------------------------------
// CCRRtspPacketSource::ResetStreamFlags
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::ResetStreamFlags( )
    {
     // Go through streams and clear receiving flag.
    for ( TInt i = 0 ; i < iReceiveStreams.Count() ; i++ )
        {        
        iReceiveStreams[i].iDataReceived = EFalse;         
        }
        
    // We have to check receive again    
    iTrafficFound = EFalse;
    }    
    
#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE )
// -----------------------------------------------------------------------------
// CCRRtspPacketSource::ShowHeader
// -----------------------------------------------------------------------------
//
void CCRRtspPacketSource::ShowHeader(
    const TDesC8& aRtcpHeader,
    const TCRRtpSRReportHeader& aSrReport )
    {
    TBuf<100> b( KNullDesC );
    LOG1( "CCRRtspPacketSource::TCP control packet len: %d", aRtcpHeader.Length() );
    for ( TInt j( 0 ); j < 32 && j < aRtcpHeader.Length(); j++ )
        {
        b.AppendFormat( _L( "%2X " ), ( unsigned )( aRtcpHeader[j] ) );
        if ( j > 0 && ( ( j % 16 ) == 0 ) )
            {
            LOG2( "%d -> %S", j, &b );
            b.Zero();
            }
        }

    LOG1( "iVersion %u", ( unsigned )aSrReport.iVersion  );
    LOG1( "iPadding %u", ( unsigned )aSrReport.iPadding );
    LOG1( "iReportCount %u",( unsigned )aSrReport.iReportCount );
    LOG1( "iPacketType %u", ( unsigned )aSrReport.iPacketType );
    LOG1( "iLength %u",
        ( unsigned)BigEndian::Get16( ( const TUint8* )&aSrReport.iLength ) );
    LOG1( "iSenderSSRC %u",
        ( unsigned )BigEndian::Get32( ( const TUint8* )&aSrReport.iSenderSSRC ) );
    LOG1( "iMSWTimestamp %u",
        ( unsigned )BigEndian::Get32( ( const TUint8* )&aSrReport.iMSWTimestamp) );
    LOG1( "iLSWTimestamp %u",
        ( unsigned)BigEndian::Get32( ( const TUint8* )&aSrReport.iLSWTimestamp ) );
    LOG1( "iRTPTimestamp %u",
        ( unsigned )BigEndian::Get32( ( const TUint8* )&aSrReport.iRTPTimestamp ) );
    LOG1( "iSenderPacketCount %u",
        ( unsigned )BigEndian::Get32( ( const TUint8* )&aSrReport.iSenderPacketCount) );
    LOG1( "iSenderOctetCount %u",
        ( unsigned )BigEndian::Get32( ( const TUint8* )&aSrReport.iSenderOctetCount ) );
        
    }
#endif // LIVE_TV_FILE_TRACE || LIVE_TV_RDEBUG_TRACE

//  End of File
