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
* Description:    Class that instructs rtsp client about getting rtp*
*/




// INCLUDE FILES
#include "CCRRtspSink.h"
#include "CCRPacketBuffer.h"
#include "CRRTSPCommon.h"
#include "CCRConnection.h"
#include "CCRStreamingSession.h"
#include <ipvideo/CDvrSdpParser.h>
#include <e32msgqueue.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KCCRRtspSinkDefaultServerPort( 20042 );

_LIT( KCCRRtspSink, "Rtsp sink" );
_LIT ( KCRLocalIPAddr, "127.0.0.1" );
_LIT8( KBaseUrl, "rtsp://127.0.0.1/" ); 
_LIT8( KVis0, "v=0\r\n" );
_LIT8( KSdpOLine, "o=- 1 2 IN IP4 127.0.0.1\r\n" );
_LIT8( KSdpSLine, "s=cre\r\n"); 
_LIT8( KSdpCLine, "c=IN IP4 0.0.0.0\r\n"); 
_LIT8( KSdpTLine, "t=0 0\r\n"); 
_LIT8( KSdpBLine, "b=AS:"); 
_LIT8( KSdpAudioMLine, "m=audio 0 RTP/AVP %d\r\n" );
_LIT8( KSdpAudioAControlLine,
       "a=control:rtsp://127.0.0.1/default.3gp/AudioControlAddress\r\n" );
_LIT8( KSdpvideoMLine, "m=video 0 RTP/AVP %d\r\n" );
_LIT8( KSdpvideoAControlLine,
       "a=control:rtsp://127.0.0.1/default.3gp/VideoControlAddress\r\n" );
_LIT8( KDescribeReply,
       "RTSP/1.0 200 OK\r\nCseq: %d\r\nContent-length: %d\r\n"
       "Content-Type: application/sdp\r\n\r\n%S" );
_LIT8( KSetupReply, 
       "RTSP/1.0 200 OKr\nCseq: %dr\nSession: 42\r\n"
       "Transport: RTP/AVP;unicast;mode=play;client_port=%d-%d;"
       "server_port=%d-%d\r\n\r\n" );
_LIT8( KControlAddr,"VideoControlAddress" );
_LIT8( KPlayReply,
       "RTSP/1.0 200 OK\r\n" "Cseq: %d\r\n"
       "RTP-Info: url=rtsp://127.0.0.1/default.3gp/VideoControlAddress"
       ";seq=%u;rtptime=%u,url=rtsp://127.0.0.1/default.3gp/AudioControlAddress;"
       "seq=%u;rtptime=%u\r\n"
       "Session: 42\r\n" );
_LIT8( KPlayReplyAudioOnly,
       "RTSP/1.0 200 OK\r\n" "Cseq: %d\r\n"
       "RTP-Info: url=rtsp://127.0.0.1/default.3gp/AudioControlAddress;"
       "seq=%u;rtptime=%u\r\n"
       "Session: 42\r\n" );
_LIT8( KPlayReplyVideoOnly,
       "RTSP/1.0 200 OK\r\n" "Cseq: %d\r\n"
       "RTP-Info: url=rtsp://127.0.0.1/default.3gp/VideoControlAddress"
       ";seq=%u;rtptime=%u\r\n"
       "Session: 42\r\n" );

_LIT8( KPauseReply, "RTSP/1.0 %d OK\r\nCseq: %d\r\nSession: 42\r\n\r\n" );
_LIT8( KTearDownReply, "RTSP/1.0 200 OK\r\nCseq: %d\r\nSession: 42\r\n\r\n" );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRRtspSink::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//  
CCRRtspSink* CCRRtspSink::NewL(
    CCRConnection& aConnection,
    RSocketServ& aSockServer,
    CCRStreamingSession::TCRSinkId aSinkId,
    const TInt& aLoopbackPort,
    CCRStreamingSession& aOwningSession )
    {
    CCRRtspSink* self = new( ELeave ) 
        CCRRtspSink( aConnection, aSockServer, aSinkId, aOwningSession );
    CleanupStack::PushL( self );
    self->ConstructL( aLoopbackPort );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::CCRRtspSink
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//  
CCRRtspSink::CCRRtspSink(
    CCRConnection& aConnection,
    RSocketServ& aSockServer,
    CCRStreamingSession::TCRSinkId aSinkId,
    CCRStreamingSession& aOwningSession )
  : CCRPacketSinkBase( aOwningSession, aSinkId ), 
    iConnection( aConnection ),
    iSockServer( aSockServer ),
    iStage( ERTSPInit ),
    iSetupReceived( 0 ),
    iAudioSeq( KMaxTUint32 ),
    iAudioTS( KMaxTUint32 ),
    iVideoSeq( KMaxTUint32 ),
    iVideoTS( KMaxTUint32 ),
    iLowerRange( KRealZero ),
    iUpperRange( KRealMinusOne )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::ConstructL
// 2nd phase. 
// -----------------------------------------------------------------------------
//  
void CCRRtspSink::ConstructL( const TInt& aLoopbackPort )
    {
    iReceivedData = HBufC8::NewL( 0 );
    iRopResponse = HBufC8::NewL( 0 );
    iSockArr[EROPControl] = CCRSock::NewL(
         *this, EROPControl, iConnection.Connection(), iSockServer, ETrue, ETrue );
    TInt err( iSockArr[EROPControl]->ListenPort( aLoopbackPort ) );
    LOG2( "CCRRtspSink::ConstructL(), aLoopbackPort: %d, err: %d", aLoopbackPort, err );      
    User::LeaveIfError( err );
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::~CCRRtspSink
// Destructor.
// -----------------------------------------------------------------------------
//
CCRRtspSink::~CCRRtspSink()
    {    
    LOG( "CCRRtspSink::~CCRRtspSink()" );

    for ( TInt i( 0 ); i < EROPMaxSockets; i++ )
        {
        delete iSockArr[i]; iSockArr[i] = NULL;
        }
    for ( TInt i( 0 ); i < CCRRtspCommand::ERTSPCommandNOCOMMAND; i++ )
        {
        delete iCommands[i]; iCommands[i] = NULL; 
        }       
        
    delete iSdpForRop; 
    delete iSdpParser;
    delete iRopResponse;
    delete iReceivedData; 
    }
    
// -----------------------------------------------------------------------------
// CCRRtspSink::ProduceSDPForRopL
// 
// -----------------------------------------------------------------------------
//      
void CCRRtspSink::ProduceSDPForRopL() 
    {
    if ( !iSdpParser )
        {
        User::Leave( KErrNotReady ); 
        }
    
    delete iSdpForRop; iSdpForRop = NULL;
    iSdpForRop = HBufC8::NewL( KMaxName );
        
    iSdpForRop->Des().Zero();
    AppendL( iSdpForRop, KVis0 ); 
    AppendL( iSdpForRop, KSdpOLine ); 
    AppendL( iSdpForRop, KSdpSLine ); 
    AppendL( iSdpForRop, KSdpCLine ); 
    AppendL( iSdpForRop, KSdpTLine );
    if ( ( iSdpParser->AudioBitrate() + iSdpParser->VideoBitrate() ) > 0 )
    	{
	    AppendL( iSdpForRop, KSdpBLine ); 
    	AppendNumL( iSdpForRop, iSdpParser->AudioBitrate() +
    	                        iSdpParser->VideoBitrate() );
    	AppendL( iSdpForRop, KCRNewLine );
    	}
    
    RArray<TPtrC8> &sessionAttributes = iSdpParser->SessionAttributes();
    for ( TInt i( 0 ); i < sessionAttributes.Count(); i++ )
        {
        AppendL( iSdpForRop, sessionAttributes[i] );
        AppendL( iSdpForRop, KCRNewLine );
        }
    
    // Check whether audio exist.
    if ( iSdpParser->AudioControlAddr().Length() )
        { 
        AppendFormatL( iSdpForRop, KSdpAudioMLine, iSdpParser->MediaIdentifierAudio() );
        if ( iSdpParser->AudioBitrate() > 0 ) 
        	{
	        AppendL( iSdpForRop, KSdpBLine ); 
    	    AppendNumL( iSdpForRop, iSdpParser->AudioBitrate() );
       	 	AppendL( iSdpForRop, KCRNewLine );
       	 	}

        AppendL( iSdpForRop, KSdpAudioAControlLine );
        
        RArray<TPtrC8> &audioAttributes = iSdpParser->AudioAttributes();
        for ( TInt i( 0 ); i < audioAttributes.Count(); i++ )
            {
            AppendL( iSdpForRop, audioAttributes[i] );
            AppendL( iSdpForRop, KCRNewLine );
            }
        }

    // Check whether Video exist.
    if ( iSdpParser->VideoControlAddr().Length() )
        {
        AppendFormatL( iSdpForRop, KSdpvideoMLine, iSdpParser->MediaIdentifierVideo() );
		if ( iSdpParser->VideoBitrate() > 0 ) 
			{             
        	AppendL( iSdpForRop, KSdpBLine ); 
        	AppendNumL( iSdpForRop, iSdpParser->VideoBitrate() );
        	AppendL( iSdpForRop, KCRNewLine );
        	}
        	
        AppendL( iSdpForRop, KSdpvideoAControlLine );
        
        RArray<TPtrC8> &videoAttributes = iSdpParser->VideoAttributes();
        for ( TInt i( 0 ); i < videoAttributes.Count(); i++ )
            {
            AppendL( iSdpForRop, videoAttributes[i] );
            AppendL( iSdpForRop, KCRNewLine );
            }
        }       
    }
        
// -----------------------------------------------------------------------------
// CCRRtspSink::SetSdpL
// as a side-effect causes parsing of the sdp
// -----------------------------------------------------------------------------
//  
void CCRRtspSink::SetSdpL( const TDesC8& aSdp )
    {
    LOG1( "CCRRtspSink::SetSdpL(), aSdp len: %d", aSdp.Length() );
    
    // Create SDP parser
    delete iSdpParser; iSdpParser = NULL;
    iSdpParser = CDvrSdpParser::NewL();
    iSdpParser->TryParseL( aSdp, KBaseUrl );
    ProduceSDPForRopL();

    if ( iStage == ERTSPDescSent )
        {
        ReplyToDescribeL();
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::NewPacketAvailable
//
// -----------------------------------------------------------------------------
//      
void CCRRtspSink::NewPacketAvailable()
    {
    if ( iBuffer )
        {
        // Stream of next packet
        MCRPacketSource::TCRPacketStreamId stream( 
            MCRPacketSource::EStreamIdCount );
        const TInt bookKeeping( iBuffer->GetStream( iSinkId, stream ) ); 
        
        // Packets in buffer.
        if ( stream != MCRPacketSource::EStreamIdCount )
            {
            TCRROPSockId socket( SocketFromStream( stream ) );

            // Is previous packet send ready.

            if ( iSockArr[socket] && !iSockArr[socket]->IsActive() )
                {
                // Get packet
                TPtr8 packet( NULL, 0 );
                iBuffer->GetPacket( bookKeeping, packet ); 
                
                // Now we have the packet, send it to rop:
                iSockArr[socket]->SendData( packet ); 

    			if ( iStage == ERTSPPlaySent )
    				{
    				iStage = ERTSPPlaying;
    				}

                }
            else
                {
                iPacketPendingInBuffer = ETrue; 
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::SetSeqAndTS
// 
// -----------------------------------------------------------------------------
//      
void CCRRtspSink::SetSeqAndTS(
    TUint& aAudioSeq,
    TUint& aAudioTS,
    TUint& aVideoSeq,
    TUint& aVideoTS )
    {
    LOG1( "CRE ropsink SetSeqAndTS aseq=%u ", aAudioSeq );
    
    iAudioSeq = aAudioSeq;
    iAudioTS  = aAudioTS;
    iVideoSeq = aVideoSeq;
    iVideoTS  = aVideoTS;
    iSeqAndTSSet = ETrue; 
    
    if ( iStage == ERTSPReadyToPlay )
        {
        TRAPD( err,ReplyToPlayL() );
        if ( err != KErrNone ) 
            {
            LOG1( "CRE ropsink ReplyToPlayL L=%d", err );
            iOwningSession.SinkStops( Id() ); 
            }
        }   
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::SetRange
// 
// -----------------------------------------------------------------------------
//      
void CCRRtspSink::SetRange( TReal aLower, TReal aUpper )
    {
    LOG2( "CRE CCRRtspSink SetRange (%f - %f)", aLower, aUpper );                     
    iLowerRange = aLower;
    iUpperRange = aUpper;
    }
    
// -----------------------------------------------------------------------------
// CCRRtspSink::StatusChanged
// This is used currently for getting to know if we're in playing state or not
// -----------------------------------------------------------------------------
//      
void CCRRtspSink::StatusChanged( MCRPacketSource::TCRPacketSourceState aNewState )
    {
    LOG2( "CCRRtspSink::StatusChanged(), iStage: %d, aNewState: %d", iStage, aNewState );    
    
    if ( aNewState == MCRPacketSource::ERtpStateSetupRepply )
        {
        if ( iStage == ERTSPDelayedSetup && iRopResponse->Length() > 0 )
            {
            SendControlData();
            }

        iSetupReceived++; // SETUP repply received
        }
    else if ( aNewState == MCRPacketSource::ERtpStatePlaying )
        {
        if ( iStage == ERTSPPlaySent || iStage == ERTSPReadyToPlay ) 
            {
            iStage = ERTSPPlaying;
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::DataReceived
//
// This is called when data is received from socket.
// -----------------------------------------------------------------------------
//
void CCRRtspSink::DataReceived( TInt aSockId, const TDesC8 &aData ) 
    {   
#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE ) 
    // Debug output follows
    if ( aSockId == EROPControl )
        {
        LOG2( "CCRRtspSink::DataReceived(), aSockId: %d, len: %d", 
                                            aSockId, aData.Length() );
        TName d( KNullDesC );
        for ( TInt i( 0 );  i < aData.Length(); i++ )
            {
            TChar c( aData[i] );
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
        
    switch ( aSockId )
        {
        case EROPControl: // RTSP is spoken in this sock
            {
            TRAPD( err, HandleReceivedEROPControlL( aData ) );
            if ( KErrNone != err )
                {
                LOG1( "ROPSink ProcessRtspCommandL leave %d", err );
                iOwningSession.SinkStops( Id() ); 
                }
            }
            break;
            
        case EROPVideoSend1:
        case EROPVideoSend2:
        case EROPAudioSend1:
        case EROPAudioSend2:
            {
            // Those packets that rop sends to us we do not need actions
            }
            break; 
        
        default:
            {
            LOG1( "default: Unknown aSockId: %d", aSockId );
            }
            break; 
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::HandleReceivedEROPControlL
//
// This is called after received data from EROPControl socket.
// -----------------------------------------------------------------------------
//
void CCRRtspSink::HandleReceivedEROPControlL( const TDesC8& aData )
    {
    AppendL( iReceivedData, aData );
    ProcessRtspCommandL();
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::SockStatusChange
//
// When socket status changes to something
// -----------------------------------------------------------------------------
//
void CCRRtspSink::SockStatusChange(
    TInt aSockId,
    CCRSock::TCRSockStatus aStatus,
    TInt aError )
    {
    if ( aStatus == CCRSock::EFailed )
        {
        LOG3( "CCRRtspSink::SockStatusChange, id: %d, failure: %d, aError: %d",
            aSockId, ( TInt )aStatus, aError );
        // here do DoCleanup()
        iOwningSession.SinkStops( Id() ); 
        }

	if ( aSockId != EROPControl )
		{
	    // Delete used packet from buffer if the socket was udp packet socket
	    iBuffer->HandleBufferSize();

	    // Is there more packets to send.
	    if ( iPacketPendingInBuffer )
	        {
	        NewPacketAvailable(); 
	        iPacketPendingInBuffer =
	            ( iBuffer->PacketsCount( iSinkId ) > KErrNotFound );
	        }           
        }
     else
        {
        LOG3( "CCRRtspSink::SockStatusChange(), aSockId: %d, aStatus: %d, aError: %d",
            aSockId, ( TInt )aStatus, aError );
        }

#if !defined LIVE_TV_FILE_TRACE && !defined LIVE_TV_RDEBUG_TRACE
    ( void )aError;
#endif
    }
    
// -----------------------------------------------------------------------------
// CCRRtspSink::ProcessRtspCommandL
//
// Causes parsing of command
// -----------------------------------------------------------------------------
//
void CCRRtspSink::ProcessRtspCommandL() 
    {
    LOG1( "CCRRtspSink::ProcessRtspCommandL(), iStage: %d", iStage );  
    
    CCRRtspCommand *command = CCRRtspCommand::NewL();
    CleanupStack::PushL( command );
    command->TryParseL( *iReceivedData ); 
    delete iCommands[command->Command()]; 
    iCommands[command->Command()] = command; 
    CleanupStack::Pop( command ); // it is now safely in instance variable
    ProduceRtspReplyL( command->Command() ); 
    iReceivedData->Des().Zero();
    }
    
// -----------------------------------------------------------------------------
// CCRRtspSink::ProduceRtspReplyL
//
// Causes sending of reply to rop
// -----------------------------------------------------------------------------
//
void CCRRtspSink::ProduceRtspReplyL( CCRRtspCommand::TCommand aLastCommand )    
    {
    LOG2( "CCRRtspSink::ProduceRtspReplyL(), iStage: %d, aLastCommand: %d",
                                             iStage, aLastCommand ); 
    
    switch ( aLastCommand ) 
        {
        case CCRRtspCommand::ERTSPCommandOPTIONS:
            ReplyToOptionsL(); 
            break; 
        
        case CCRRtspCommand::ERTSPCommandDESCRIBE:
            if ( iSdpForRop ) 
                {
                ReplyToDescribeL();
                }
            
            iStage = ERTSPDescSent;
            break;
        
        case CCRRtspCommand::ERTSPCommandSETUP:
            ReplyToSetupL();
            break; 
        
        case CCRRtspCommand::ERTSPCommandPLAY:
            if ( iSeqAndTSSet )
                {
                // we've either audio or video seq set, we can  proceed with play: 
                ReplyToPlayL();
                iStage = ERTSPPlaySent;
                }
            else
                {
                TReal startPos( KRealZero ); 
                TReal endPos( KRealZero );
                iCommands[CCRRtspCommand::ERTSPCommandPLAY]->GetRange( startPos, endPos );
                iOwningSession.PlayCommand( startPos, endPos );
                iStage = ERTSPReadyToPlay;
                }
            iSetupReceived = 0;
            break; 
        
        case CCRRtspCommand::ERTSPCommandPAUSE: 
            ReplyToPauseL( iStage != ERTSPPlaying ? KErrNotReady : iOwningSession.PauseCommand() );
            iSeqAndTSSet = EFalse;
            break;
        
        case CCRRtspCommand::ERTSPCommandTEARDOWN:
            iOwningSession.StopCommand();
            ReplyToTearDownL();
            break;
        
        default:
            // None
            break;
        }   
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::ReplyToOptionsL
//
// Causes sending of reply to rop for options
// -----------------------------------------------------------------------------
//
void CCRRtspSink::ReplyToOptionsL()
    {
    LOG( "CCRRtspSink::ReplyToOptionsL()" );

    iRopResponse->Des().Zero();
    AppendFormatL( iRopResponse, KCROptionsReply, 
        iCommands[CCRRtspCommand::ERTSPCommandOPTIONS]->CSeq() );
    SendControlData();
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::ReplyToDescribeL
//
// Causes sending of reply to rop for describe
// -----------------------------------------------------------------------------
//
void CCRRtspSink::ReplyToDescribeL()
    {
    LOG( "CCRRtspSink::ReplyToDescribeL()" );
    
    User::LeaveIfNull( iSdpForRop );
    iRopResponse->Des().Zero();
    AppendFormatL( iRopResponse, KDescribeReply, 
        iCommands[CCRRtspCommand::ERTSPCommandDESCRIBE]->CSeq(),
        iSdpForRop->Des().Length(), &*iSdpForRop );
    SendControlData();
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::ReplyToSetupL
//
// Causes sending of reply to rop for setup, either audio or video
// -----------------------------------------------------------------------------
//
void CCRRtspSink::ReplyToSetupL()
    {
    LOG( "CCRRtspSink::ReplyToSetupL()" );
    if ( !iSdpParser )
        {
        User::Leave( KErrNotReady ); 
        }
    
    TPtrC8 url( NULL, 0 ); 
    iCommands[CCRRtspCommand::ERTSPCommandSETUP]->URL( url );
    if ( url.Find( KControlAddr) != KErrNotFound )
        { 
        // ROP is setting up video
        TInt videoPort(
            iCommands[CCRRtspCommand::ERTSPCommandSETUP]->ClientPort() );       
        LOG1( "CCRRtspSink::ReplyToSetupL  video port  %d", videoPort );
        iStage = ERTSPSetupVideoSent;
        
        // Setup sockets:           
        iSockArr[EROPVideoSend1] = CCRSock::NewL( *this, EROPVideoSend1,
            iConnection.Connection(), iSockServer, EFalse, ETrue );
        User::LeaveIfError( iSockArr[EROPVideoSend1]->ConnectSock(
            KCRLocalIPAddr, videoPort,
            KCCRRtspSinkDefaultServerPort ) );

        iSockArr[EROPVideoSend2] = CCRSock::NewL( *this, EROPVideoSend2,
            iConnection.Connection(), iSockServer, EFalse, ETrue );
        User::LeaveIfError( iSockArr[EROPVideoSend2]->ConnectSock(
            KCRLocalIPAddr, videoPort + 1,
            KCCRRtspSinkDefaultServerPort + 1 ) );
        }
    else
        { 
        // ROP is setting up audio
        TInt audioPort( 
            iCommands[CCRRtspCommand::ERTSPCommandSETUP]->ClientPort() );
        LOG1( "CCRRtspSink::ReplyToSetupL audio port: %d", audioPort );    
        iStage = ERTSPSetupAudioSent;      
        
        // Setup sockets:           
        iSockArr[EROPAudioSend1] = CCRSock::NewL( *this, EROPAudioSend1,
            iConnection.Connection(), iSockServer, EFalse, ETrue );
        User::LeaveIfError( iSockArr[EROPAudioSend1]->ConnectSock( 
            KCRLocalIPAddr, audioPort,
            KCCRRtspSinkDefaultServerPort + 2 ) );
        
        iSockArr[EROPAudioSend2] = CCRSock::NewL( *this, EROPAudioSend2,
            iConnection.Connection(), iSockServer, EFalse, ETrue );
        User::LeaveIfError( iSockArr[EROPAudioSend2]->ConnectSock( 
            KCRLocalIPAddr, audioPort + 1,
            KCCRRtspSinkDefaultServerPort + 3 ) );
        }

    iRopResponse->Des().Zero();
    AppendFormatL( iRopResponse, KSetupReply, 
        iCommands[CCRRtspCommand::ERTSPCommandSETUP]->CSeq(),
        iCommands[CCRRtspCommand::ERTSPCommandSETUP]->ClientPort(),
        iCommands[CCRRtspCommand::ERTSPCommandSETUP]->ClientPort() + 1,
        ( iStage == ERTSPSetupVideoSent )? KCCRRtspSinkDefaultServerPort:
                                           KCCRRtspSinkDefaultServerPort + 2,
        ( iStage == ERTSPSetupVideoSent )? KCCRRtspSinkDefaultServerPort + 1:
                                           KCCRRtspSinkDefaultServerPort + 3 ); 
    
    // If last setup, delay player response. Otherwise Helix will get prepare completed
    // and sends automatically PLAY command which ruins the state machine
    if ( iSetupReceived < 2 )
        {
        CDvrSdpParser::TDvrPacketProvidings content( iSdpParser->SupportedContent() );
        if ( iStage == ERTSPSetupVideoSent )
            {
            if ( ( iSetupReceived == 0 && content == CDvrSdpParser::EDvrVideoOnly ) ||
                 ( iSetupReceived <= 1 && content == CDvrSdpParser::EDvrBothAudioAndVideo ) )
                {
                iStage = ERTSPDelayedSetup;
                LOG( "CCRRtspSink::ReplyToSetupL(), Video SETUP repply delayed.." );
                }
            }
        else
            {
            if ( ( iSetupReceived == 0 && content == CDvrSdpParser::EDvrAudioOnly ) ||
                 ( iSetupReceived <= 1 && content == CDvrSdpParser::EDvrBothAudioAndVideo ) )
                {
                iStage = ERTSPDelayedSetup;
                LOG( "CCRRtspSink::ReplyToSetupL(), Audio SETUP repply delayed.." );
                }
            }
        }

    // Repply now or later
    if ( iStage != ERTSPDelayedSetup )
        {
        SendControlData();
        }
    }


// -----------------------------------------------------------------------------
// CCRRtspSink::ReplyToPlayL
//
// 
// -----------------------------------------------------------------------------
//
void CCRRtspSink::ReplyToPlayL()
    {
    LOG( "CCRRtspSink::ReplyToPlayL()" );

    iRopResponse->Des().Zero();
    if ( iSdpParser->AudioControlAddr().Length() && 
         iSdpParser->VideoControlAddr().Length() )
        {
        AppendFormatL( iRopResponse, KPlayReply, 
            iCommands[CCRRtspCommand::ERTSPCommandPLAY]->CSeq(),
            iVideoSeq, iVideoTS, iAudioSeq, iAudioTS );
        } 
    else if ( iSdpParser->AudioControlAddr().Length() &&
             !iSdpParser->VideoControlAddr().Length() )
        {
        AppendFormatL( iRopResponse, KPlayReplyAudioOnly, 
            iCommands[CCRRtspCommand::ERTSPCommandPLAY]->CSeq(),
            iAudioSeq, iAudioTS );
        }
    else if ( !iSdpParser->AudioControlAddr().Length() && 
               iSdpParser->VideoControlAddr().Length() )
        {
        AppendFormatL( iRopResponse, KPlayReplyVideoOnly,
            iCommands[CCRRtspCommand::ERTSPCommandPLAY]->CSeq(),
            iVideoSeq, iVideoTS );
        } 
    else
        { // no audio, no video.
        iOwningSession.SinkStops( Id() ); 
        return; 
        }
        
    if ( !( iLowerRange == KRealZero && iUpperRange == KRealMinusOne ) ) 
        {
		TBuf8<KMaxName> buf( KCRRangeHeader );
        TRealFormat format( 10, 3 );
        format.iTriLen = 0; 
        buf.AppendNum( iLowerRange, format );
        buf.Append( '-' ); 
        buf.AppendNum( iUpperRange, format );
        buf.Append( KCRNewLine );
        AppendFormatL( iRopResponse, buf );
        }
    
    AppendL( iRopResponse, KCRNewLine );
    SendControlData();
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::ReplyToPlayL
//
// 
// -----------------------------------------------------------------------------
//
void CCRRtspSink::ReplyToPauseL( TInt aErrorCode ) 
    {
    LOG1( "CCRRtspSink::ReplyToPauseL(), aErrorCode: %d", aErrorCode );

    iRopResponse->Des().Zero();

    switch ( aErrorCode )
        {
        case KErrNone:
            AppendFormatL( iRopResponse, KPauseReply,
                CCRRtspResponse::ERTSPRespOK,  
                iCommands[CCRRtspCommand::ERTSPCommandPAUSE]->CSeq() );
            iStage = ERTSPPauseSent;
            break; 
        
        case KErrNotReady:
            AppendFormatL( iRopResponse, KPauseReply,
                CCRRtspResponse::ERTSPRespMethodNotValidInThisState,  
                iCommands[CCRRtspCommand::ERTSPCommandPAUSE]->CSeq() );
            break; 
        
        default:
            AppendFormatL( iRopResponse, KPauseReply,
                CCRRtspResponse::ERTSPRespMethodNotAllowed,  
                iCommands[CCRRtspCommand::ERTSPCommandPAUSE]->CSeq() );
            break; 
        }
 
    SendControlData();
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::ReplyToTearDownL
//
// 
// -----------------------------------------------------------------------------
//
void CCRRtspSink::ReplyToTearDownL() 
    {
    LOG( "CCRRtspSink::ReplyToTearDownL()" );

    iRopResponse->Des().Zero();
    AppendFormatL( iRopResponse, KTearDownReply,
        iCommands[CCRRtspCommand::ERTSPCommandTEARDOWN]->CSeq() );
    SendControlData();
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::SocketFromStream
//
// -----------------------------------------------------------------------------
//      
CCRRtspSink::TCRROPSockId CCRRtspSink::SocketFromStream(
    MCRPacketSource::TCRPacketStreamId aStreamId )
    {
    switch ( aStreamId ) 
        {
        case MCRPacketSource::EAudioStream:
            return EROPAudioSend1;
        
        case MCRPacketSource::EAudioControlStream:
            return EROPAudioSend2;
        
        case MCRPacketSource::EVideoStream:
            return EROPVideoSend1;
        
        case MCRPacketSource::EVideoControlStream:
            return EROPVideoSend2;
        
        default:
            __ASSERT_ALWAYS( 1!=2, User::Panic( KCCRRtspSink, KErrArgument ) );
            break;
        }

    return EROPMaxSockets; // this is never reached
    }

// -----------------------------------------------------------------------------
// CCRRtspSink::AppendL
//
// -----------------------------------------------------------------------------
//      
void CCRRtspSink::AppendL( HBufC8*& aBuffer, const TDesC8& aStr )
    {
    TPtr8 ptr( aBuffer->Des() );
    if ( ( ptr.Length() + aStr.Length() ) >= ptr.MaxLength() )
        {
        const TInt newLength( ptr.Length() + aStr.Length() + KMaxName );
        aBuffer = aBuffer->ReAllocL( newLength );
        ptr.Set( aBuffer->Des() );
        }
    
    ptr.Append( aStr );
    }
    
// -----------------------------------------------------------------------------
// CCRRtspSink::AppendNumL
//
// -----------------------------------------------------------------------------
//      
void CCRRtspSink::AppendNumL( HBufC8*& aBuffer, const TInt aNum )
    {
    TPtr8 ptr( aBuffer->Des() );
    if ( ( ptr.Length() + KMaxInfoName ) >= ptr.MaxLength() )
        {
        const TInt newLength( ptr.Length() + KMaxInfoName + KMaxName );
        aBuffer = aBuffer->ReAllocL( newLength );
        ptr.Set( aBuffer->Des() );
        }
    
    ptr.AppendNum( aNum );
    }
    
// -----------------------------------------------------------------------------
// CCRRtspSink::AppendFormatL
//
// -----------------------------------------------------------------------------
//      
void CCRRtspSink::AppendFormatL(
    HBufC8*& aBuffer,
    TRefByValue<const TDesC8> aFmt, ... )
    {
    VA_LIST list;
    VA_START( list, aFmt );
    HBufC8* buf = HBufC8::NewLC( KMaxDataSize );
    buf->Des().FormatList( aFmt, list );
    VA_END( list );

    TPtr8 ptr( aBuffer->Des() );
    if ( ( ptr.Length() + buf->Length() ) >= ptr.MaxLength() )
        {
        const TInt newLength( ptr.Length() + buf->Length() + KMaxName );
        aBuffer = aBuffer->ReAllocL( newLength );
        ptr.Set( aBuffer->Des() );
        }
    
    ptr.Append( *buf );
    CleanupStack::PopAndDestroy( buf );
    }
    
// -----------------------------------------------------------------------------
// CCRRtspSink::SendControlData
//
// -----------------------------------------------------------------------------
//
void CCRRtspSink::SendControlData()
    {
    iSockArr[EROPControl]->SendData( *iRopResponse );
    iRopResponse->Des().Zero();
    }

//  End of File
