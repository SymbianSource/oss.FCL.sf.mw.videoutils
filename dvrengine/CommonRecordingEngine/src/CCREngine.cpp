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
* Description:    Engine part of the engine. In practice keeps count on*
*/




// INCLUDE FILES
#include <pathinfo.h>
#include "VideoServiceUtilsConf.hrh" 
#include "CCREngine.h"
#include "CCRStreamingSession.h"
#include "CCRSession.h"
#include "CCRClientInformer.h"
#include "CCRConnection.h"
#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT
#include <ipvideo/CRtpUtil.h>
#include <ipvideo/CRtpClipHandler.h>
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
#include <e32msgqueue.h>
#include <ipvideo/CRTypeDefs.h>
#include <e32std.h>
#include <es_sock.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TUint KCRELoopbackPortStart( 1106 );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCREngine::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCREngine* CCREngine::NewL( void )
    {
    CCREngine* self = new( ELeave ) CCREngine();

    // Use CleanupClosePushL to make sure the Close function defined in CObject
    // base class gets called. CObject will delete itself once its reference count
    // reaches zero. Using CleanupStack::PushL here results in E32USER-CBase panic 33
    // if there is a leave somewhere in construction
    CleanupClosePushL( *self );

    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCREngine::CCREngine
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCREngine::CCREngine( void ) : iLoopbackPort( KCRELoopbackPortStart )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCREngine::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCREngine::ConstructL()
    {
    LOG( "CCREngine::ConstructL()" );
    
    // Note, quite high priority
    iCleanUp = new ( ELeave ) CAsyncCallBack( CActive::EPriorityStandard ); 
    }

// -----------------------------------------------------------------------------
// CCREngine::~CCREngine
// Destructor.
// -----------------------------------------------------------------------------
//
CCREngine::~CCREngine()
    {
    LOG( "CCREngine::~CCREngine()" );

    delete iCleanUp;
    iSessions.ResetAndDestroy();
    iSessionsToDelete.Reset(); 
    delete iConnection;
#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT
    delete iClipHandler;
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    iSockServer.Close(); 
    delete iInformer; 
    delete iQueueName;
    }

// -----------------------------------------------------------------------------
// Method for actual message handling.
//
// -----------------------------------------------------------------------------
//
void CCREngine::GeneralServiceL( const RMessage2& aMessage )
    {
#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    if ( aMessage.Function() != ECRGetPosition )
        {
        LOG1( "CCREngine::GeneralService(), aMessage: %d", aMessage.Function() );
        }
#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE

    // Verify cababilities
    VerifyCapabilitiesL( aMessage );

    // Handle message
    TInt err( KErrNone );
    switch( aMessage.Function() )
        {
        case ECRSetIap:
            {
            TPckgBuf<SCRRtspIapParams> pckg;
            Read( aMessage, 0, pckg );
            err = SetConnectionId( pckg() );  
            aMessage.Complete( err );
            }
            break;      
            
        case ECRCancelSetIap:
            aMessage.Complete( CancelSetConnectionId() );
            break;
            
        case ECRPlayRtspUrl:
            {
            TPckgBuf<TUint> pckg0( 0 );
            TPckgBuf<SCRRtspParams> pckg1;
            Read( aMessage, 1, pckg1 );
            TRAP( err, pckg0() = PlayRtspUrlL( pckg1() ) );
            if ( !err )
                {
                Write( aMessage, 0, pckg0 );
                }
            aMessage.Complete( err );
            }
            break;      
            
        case ECRPlayDvbhLive:
            {
            TPckgBuf<TUint> pckg0( 0 );
            TPckgBuf<SCRLiveParams> pckg1;
            Read( aMessage, 1, pckg1 );
            TRAP( err, pckg0() = PlayDvbhLiveL( pckg1() ) );
            if ( !err )    
                {
                Write( aMessage, 0, pckg0 );
                }
            aMessage.Complete( err );
            }
            break;      
            
        case ECRChangeService:
            {
            TPckgBuf<TUint> pckg0( 0 );
            TPckgBuf<SCRLiveParams> pckg1;
            Read( aMessage, 0, pckg0 );
            Read( aMessage, 1, pckg1 );
            TRAP( err, pckg0() = ChangeDvbhServiceL( pckg0(), pckg1() ) );
            if ( !err )    
                {
                Write( aMessage, 0, pckg0 );
                }
            aMessage.Complete( err );
            }
            break;
            
        case ECRPlayRtpFile:
            {
            TPckgBuf<TUint> pckg0( 0 );
            TPckgBuf<SCRRtpPlayParams> pckg1;
            Read( aMessage, 1, pckg1 );
            TRAP( err, pckg0() = PlayRtpFileL( pckg1() ) );
            if ( !err )
                {
                Write( aMessage, 0, pckg0 );
                }
            aMessage.Complete( err );
            }
            break;      
            
        case ECRPlayRtpHandle:
            {
            RFile fileHandle;
            err = fileHandle.AdoptFromClient( aMessage, 1, 2 );
            if ( !err )
                {
                TPckgBuf<TUint> pckg( 0 );
                TRAP( err, pckg() = PlayRtpFileL( fileHandle ) );
                if ( !err )
                    {
                    Write( aMessage, 0, pckg );
                    }
                }
            fileHandle.Close();
            aMessage.Complete( err );
            }
            break;      
            
        case ECRRecordCurrentStream:
            {
            TPckgBuf<TUint> pckg0( 0 );
            Read( aMessage, 0, pckg0 );
            TPckgBuf<SCRRecordParams> pckg1;
            Read( aMessage, 1, pckg1 );
            TRAP( err, RecordCurrentStreamL( pckg0(), pckg1() ) );
            aMessage.Complete( err );
            }
            break;
            
        case ECRRecordRtspStream:
            {
            TPckgBuf<TUint> pckg0( 0 );
            TPckgBuf<SCRRtspParams> pckg1;
            Read( aMessage, 1, pckg1 );
            TPckgBuf<SCRRecordParams> pckg2;
            Read( aMessage, 2, pckg2 );
            TRAP( err, pckg0() = RecordRtspStreamL( pckg1(), pckg2() ) );
            aMessage.Complete( err );
            }
            break;
            
        case ECRRecordDvbhStream:
            {
            TPckgBuf<TUint> pckg0( 0 );
            TPckgBuf<SCRLiveParams> pckg1;
            Read( aMessage, 1, pckg1 );
            TPckgBuf<SCRRecordParams> pckg2;
            Read( aMessage, 2, pckg2 );
            TRAP( err, pckg0() = RecordDvbhStreamL( pckg1(), pckg2() ) );
            aMessage.Complete( err );
            }
            break;
            
        case ECRPauseRecordStream:
            {
            TPckgBuf<TUint> pckg0( 0 );
            Read( aMessage, 0, pckg0 );
            TPckgBuf<TBool> pckg1( 0 );
            Read( aMessage, 1, pckg1 );
            aMessage.Complete( PauseRecordStream( pckg0(), pckg1() ) );
            }
            break;
            
        case ECRStopRecordStream:
            {
            TPckgBuf<TUint> pckg0( 0 );
            Read( aMessage, 0, pckg0 );
            aMessage.Complete( StopRecordStream( pckg0() ) );
            }
            break;
            
        case ECRStartTimeShift:
            {
            TPckgBuf<TUint> pckg0( 0 );
            TPckgBuf<TUint> pckg1( 0 );
            Read( aMessage, 1, pckg1 );
            TRAP( err, pckg0() = StartTimeShiftL( pckg1() ) );
            if ( !err )
                {
                Write( aMessage, 0, pckg0 );
                }
            aMessage.Complete( err );
            }
            break;
            
        case ECRStopTimeShift:
            {
            TPckgBuf<TUint> pckg0( 0 );
            TPckgBuf<TUint> pckg1( 0 );
            Read( aMessage, 0, pckg0 );
            Read( aMessage, 1, pckg1 );
            aMessage.Complete( StopTimeShift( pckg0(), pckg1() ) );
            }
            break;
            
        case ECRPlayCommand:
            {
            TPckgBuf<TUint> pckg0( 0 );
            Read( aMessage, 0, pckg0 );
            TPckgBuf<TInt64> pckg1( 0 );
            Read( aMessage, 1, pckg1 );
            TPckgBuf<TInt64> pckg2( 0 );
            Read( aMessage, 2, pckg2 );
            aMessage.Complete( PlayCommand( pckg0(), pckg1(), pckg2() ) );
            }
            break;
        
        case ECRPauseCommand:
            {
            TPckgBuf<TUint> pckg0( 0 );
            Read( aMessage, 0, pckg0 );
            aMessage.Complete( PauseCommand( pckg0() ) );
            }
            break;
        
        case ECRStopCommand:
            {
            TPckgBuf<TUint> pckg0( 0 );
            Read( aMessage, 0, pckg0 );
            aMessage.Complete( StopCommand( pckg0() ) );
            }
            break;
        
        case ECRSetPosition:
            {
            TPckgBuf<TUint> pckg0( 0 );
            Read( aMessage, 0, pckg0 );
            TPckgBuf<TInt64> pckg1( 0 );
            Read( aMessage, 1, pckg1 );
            err = SetPosition( pckg0(), pckg1() );
            aMessage.Complete( err );
            }
            break;
        
        case ECRGetPosition:
            {
            TPckgBuf<TUint> pckg0( 0 );
            Read( aMessage, 0, pckg0 );
            TPckgBuf<TInt64> pckg1( 0 );
            Read( aMessage, 1, pckg1 );
            TPckgBuf<TInt64> pckg2( 0 );
            err = GetPosition( pckg0(), pckg1(), pckg2() );
            Write( aMessage, 1, pckg1 );
            Write( aMessage, 2, pckg2 );
            aMessage.Complete( err );
            }
            break;
            
        case ECRCloseSession:
            {
            TPckgBuf<TUint> pckg0( 0 );
            Read( aMessage, 0, pckg0 );
            aMessage.Complete( CloseSession( pckg0() ) );
            }
            break;
        
        case ECRPlayNullSource:
            {
            TPckgBuf<TUint> pckg0( 0 );
            TRAP( err, pckg0() = PlayNullSourceL() );
            Write( aMessage, 0, pckg0 );
            aMessage.Complete( err );
            }
            break;     
        
        case ECRPlayRtspUrlToNullSink:
            {
            TPckgBuf<TUint> pckg0( 0 );
            TPckgBuf<SCRRtspParams> pckg1;
            Read( aMessage, 1, pckg1 );
            TRAP( err, pckg0() = PlayRtspUrlToNullSinkL( pckg1() ) );
            Write( aMessage, 0, pckg0 );
            aMessage.Complete( err );
            }
            break;     
        
        default:
            aMessage.Complete( KErrNotSupported ); 
            break; 
        }
    }
    
//-----------------------------------------------------------------------------
// CCREngine::SessionStop()
// So, a session wants to quit. we can't just delete it here as return.
// Statement would then take us to deleted instance: put up a cleanup 
// CAsyncCallBack and return.
//-----------------------------------------------------------------------------
//
void CCREngine::SessionStop( CCRStreamingSession* aSession )
    {
    LOG1( "CCREngine::SessionStop(), Delete count: %d", iSessionsToDelete.Count() );    

    // InsertInAddressOrder checks for duplicate, if there is already
    // entry for that session, the array will remain unchanged
    TRAPD( err, iSessionsToDelete.InsertInAddressOrderL( aSession ) ); 
    if ( err )
        {
        LOG1( "CCREngine::SessionStop(), InsertInAddressOrder leaved: %d", err );      
        }

    // If not already active and sessions to delete?
    if ( !iCleanUp->IsActive() && iSessionsToDelete.Count() ) 
        {
        TCallBack cb( SessionStopCallBack, this );
        iCleanUp->Set( cb );
        iCleanUp->CallBack();
        }               
    }
    
// -----------------------------------------------------------------------------
// CCREngine::ConnectionStatusChange
//
// -----------------------------------------------------------------------------
//
void CCREngine::ConnectionStatusChange(
    TInt aSessionId,
    TCRConnectionStatus aStatus,
    TInt aErr )
    {
    LOG3( "CCREngine::ConnectionStatusChange(), aSessionId: %d, aStatus: %d, Error: %d",
                                                aSessionId, aStatus, aErr );
    
    SCRQueueEntry entry = { ECRMsgQueueConnectionError, KErrNone };
    entry.iSessionId = aSessionId;

    switch ( aStatus )
        {           
        case ECRConnectionError:
            {               
            entry.iMsg = ECRMsgQueueConnectionError;
            entry.iErr = aErr;
            }
            break;
        
        case ECRAuthenticationNeeded:
            {               
            entry.iMsg = ECRMsgQueueAuthenticationNeeded;
            entry.iErr = KErrNone;
            }      
            break;  
        
        case ECRNotEnoughBandwidth:
            {               
            entry.iMsg = ECRMsgQueueNotEnoughBandwidth;
            entry.iErr = KErrNone;
            }      
            break;  
        
        case ECRNormalEndOfStream:
            {               
            entry.iMsg = ECRMsgQueueNormalEndOfStream;
            entry.iErr = aErr;
            }
            break;
        
        case ECRAttachCompleted: // fall through
        case ECRSwitchingToTcp:
            {               
            aStatus == ECRAttachCompleted ? 
                           ( entry.iMsg = ECRMsgQueueAttachCompleted ) :
                           ( entry.iMsg = ECRMsgQueueSwitchingToTcp );
            
            if ( aErr == KErrNone )
                {
                // Releasing RSocket takes almost 40 seconds so we need 
                // to always use different port number, otherwise an  
                // "Already in use" error will be occurred.
                TTime now;
                now.UniversalTime();                  
                TInt port = KCRELoopbackPortStart + now.DateTime().Second();   
                iLoopbackPort == port ? ( iLoopbackPort = port + 1 ) : 
                                        ( iLoopbackPort = port );

                // Loopback port for VIA
                entry.iErr = iLoopbackPort;
                }
            else
                {
                entry.iErr = aErr;
                }
            }           
            break;  
            
        case ECRStreamIsLiveStream:
            {
            entry.iMsg = ECRMsgQueueStreamIsLiveStream; 
            entry.iErr = aErr; 
            }
            break; 

        case ECRStreamIsRealMedia:
            {
            entry.iMsg = ECRMsgQueueStreamIsRealMedia; 
            entry.iErr = aErr; 
            }
            break; 

        case ECRBearerChanged:
            {
            // Nobody is interested about this on the other side...
            return;         
            }

        case ECRTestSinkData:
            {
            entry.iMsg = ECRMsgQueueTestSinkData;
            entry.iErr = aErr;
            }
            break;

        case ECRSdpAvailable:
            {
            entry.iMsg = ECRMsgQueueSdpAvailable;
            entry.iErr = aErr;
            }
            break;
        
        case ECRReadyToSeek:
            {
            entry.iMsg = ECRMsgQueueReadyToSeek; 
            entry.iErr = aErr; 
            }
            break; 

        case ECRRecordingStarted:
            {
            entry.iMsg = ECRMsgQueueRecordingStarted;
            entry.iErr = aErr;
            }
            break;
        
        case ECRRecordingPaused:
            {
            entry.iMsg = ECRMsgQueueRecordingPaused;
            entry.iErr = aErr;
            }
            break;
        
        case ECRRecordingEnded:
            {
            entry.iMsg = ECRMsgQueueRecordingEnded;
            entry.iErr = aErr;
            }
            break;
        
        default:
            // None
            break;
        }
    
    if ( iQueueName && iInformer )
        {
        iInformer->SendMessage( iQueueName->Des(), entry ); 
        }
    }

// -----------------------------------------------------------------------------
// CCREngine::VerifyCapabilitiesL
// Checks capabilities of user of DVR API.
// -----------------------------------------------------------------------------
//
void CCREngine::VerifyCapabilitiesL( const RMessage2& aMessage )
    {
    if ( !aMessage.HasCapability( ECapabilityNetworkServices, 
         __PLATSEC_DIAGNOSTIC_STRING( "NetworkServices" ) ) )
        {
        LOG( "CCREngine::VerifyRbfCapabilitiesL(), Missing: NetworkServices !" );
        User::Leave( KErrPermissionDenied );
	    }
    
    if ( !aMessage.HasCapability( ECapabilityReadUserData,
         __PLATSEC_DIAGNOSTIC_STRING( "ReadUserData" ) ) )
	    {
        LOG( "CCREngine::VerifyRbfCapabilitiesL(), Missing: ReadUserData !" );
        User::Leave( KErrPermissionDenied );
	    }
                                 
    if ( !aMessage.HasCapability( ECapabilityWriteUserData,
         __PLATSEC_DIAGNOSTIC_STRING( "WriteUserData" ) ) )
	    {
        LOG( "CCREngine::VerifyRbfCapabilitiesL(), Missing: WriteUserData !" );
        User::Leave( KErrPermissionDenied );
	    }
    }

// -----------------------------------------------------------------------------
// CCREngine::SetConnectionId
//
// -----------------------------------------------------------------------------
//
TInt CCREngine::SetConnectionId( const SCRRtspIapParams& aIap ) 
    {
    LOG( "CCREngine::SetConnectionId() in" );

    TRAPD ( err, CreateConnectionL() ) ;
    if ( err == KErrNone )
        {
        delete iQueueName; iQueueName = NULL;
        TRAP( err, iQueueName = aIap.iQueueName.AllocL() );
        if ( err == KErrNone )
            {
            iConnection->Attach( aIap.iConnectionId );  
            }
        
        }
    LOG1( "CCREngine::SetConnectionId() out, err: %d", err );
    return err;
    }

// -----------------------------------------------------------------------------
// CCREngine::CancelSetConnectionId
//
// -----------------------------------------------------------------------------
//
TInt CCREngine::CancelSetConnectionId() 
    {
    LOG( "CCREngine::CancelSetConnectionId() in" );
    
    TInt err( KErrNone );
    if ( iConnection ) 
        {
        iSessions.ResetAndDestroy();
        delete iConnection; iConnection = NULL;
        iSockServer.Close(); 
        err = iSockServer.Connect( KESockDefaultMessageSlots * 3 );
        if ( err == KErrNone ) 
            {
            TRAP( err, iConnection = CCRConnection::NewL( iSockServer ) );
            if ( err == KErrNone && iConnection )
                {
                err = iConnection->RegisterObserver( this );                    
                }
            }
        }
    
    LOG1( "CCREngine::CancelSetConnectionId() out, err: %d", err );
    return err; 
    }

// -----------------------------------------------------------------------------
// CCREngine::PlayRtspUrlL
// Go through stream sessions and try to find if given url is already playing.
// If not, create new RTSP session and add sink to it.
// -----------------------------------------------------------------------------
//
TUint CCREngine::PlayRtspUrlL( const SCRRtspParams& aRtspParams ) 
    {
    LOG( "CCREngine::PlayRtspUrlL() in" );
    
    const TInt sessionIndex( VerifyRtspSessionL( aRtspParams ) );
    iSessions[sessionIndex]->CreateRtspSinkL( iLoopbackPort );
    TUint sessionChk( iSessions[sessionIndex]->SourceChecksum() );
    LOG1( "CCREngine::PlayRtspUrlL() out, sessionChk: %d", sessionChk );
    return sessionChk;
    }

// -----------------------------------------------------------------------------
// CCREngine::PlayDvbhLiveL
// Go through stream sessions and try to find if current service is already
// If not, create new DVB-H session and add sink to it.
// -----------------------------------------------------------------------------
//
TUint CCREngine::PlayDvbhLiveL( const SCRLiveParams& aLiveParams ) 
    {
    LOG( "CCREngine::PlayDvbhLiveL() in" );

    const TInt sessionIndex( VerifyDvbhSessionL( aLiveParams ) );
    iSessions[sessionIndex]->CreateXpsSinkL();
    iSessions[sessionIndex]->PostActionL();
    TUint sessionChk( iSessions[sessionIndex]->SourceChecksum() );
    LOG1( "CCREngine::PlayDvbhLiveL() out, sessionChk: %d", sessionChk );
    return sessionChk;
    }

// -----------------------------------------------------------------------------
// CCREngine::ChangeDvbhServiceL
// Go through stream sessions and try to find if current service is already
// If not, create new DVB-H session and add sink to it.
// -----------------------------------------------------------------------------
//
TUint CCREngine::ChangeDvbhServiceL(
    const TUint aSessionChk,
    const SCRLiveParams& aLiveParams ) 
    {
    LOG( "CCREngine::ChangeDvbhServiceL() in" );

    // Verify exist session
    const TInt currentSession( VerifySession( aSessionChk ) );
    User::LeaveIfError( currentSession );
    
    // Can't perform if recording or timeshift ongoing
    if ( iSessions[currentSession]->ClipHandlerUsed() )
        {
        User::Leave( KErrInUse );
        }

    // New session for new service
    const TInt sessionIndex( VerifyDvbhSessionL( aLiveParams ) );
    if ( sessionIndex == currentSession )
        {
        // Same service, no actions
        return aSessionChk;
        }
    
    // Stop sockets of DVB-H source (filters released)
    User::LeaveIfError( iSessions[currentSession]->StopCommand() );
    
    // Transfer ownership of existing XPS sink to a new session
    User::LeaveIfError( iSessions[currentSession]->TransferSink( 
        CCRStreamingSession::ECRXpsSinkId, *iSessions[sessionIndex] ) );
    iSessions[sessionIndex]->PostActionL();
    TUint sessionChk( iSessions[sessionIndex]->SourceChecksum() );
    
    // Delete existing unused session
    SessionStop( iSessions[currentSession] );
    LOG1( "CCREngine::ChangeDvbhServiceL() out, sessionChk: %d", sessionChk );
    return sessionChk;
    }

// -----------------------------------------------------------------------------
// CCREngine::PlayRtpFileL
//
// -----------------------------------------------------------------------------
//
TUint CCREngine::PlayRtpFileL( const SCRRtpPlayParams& aRtpParams ) 
    {
    LOG( "CCREngine::PlayRtpFileL() in" );

    const TInt sessionIndex( VerifyRtpFileSessionL( aRtpParams ) );
    iSessions[sessionIndex]->CreateXpsSinkL();
    iSessions[sessionIndex]->PostActionL();
    TUint sessionChk( iSessions[sessionIndex]->SourceChecksum() );
    LOG1( "CCREngine::PlayRtpFileL() out, sessionChk: %d", sessionChk );
    return sessionChk;
    }

// -----------------------------------------------------------------------------
// CCREngine::PlayRtpFileL
//
// -----------------------------------------------------------------------------
//
TUint CCREngine::PlayRtpFileL( const RFile& aFileHandle ) 
    {
    LOG( "CCREngine::PlayRtpFileL() in, with hadle" );

    TInt sessionIndex( VerifyRtpFileSessionL( aFileHandle ) );
    iSessions[sessionIndex]->CreateXpsSinkL();
    iSessions[sessionIndex]->PostActionL();
    TUint sessionChk( iSessions[sessionIndex]->SourceChecksum() );
    LOG1( "CCREngine::PlayRtpFileL() out, sessionChk: %d", sessionChk );
    return sessionChk;
    }

// -----------------------------------------------------------------------------
// CCREngine::RecordCurrentStreamL
//
// -----------------------------------------------------------------------------
//
void CCREngine::RecordCurrentStreamL(
    const TUint aSessionChk,
    const SCRRecordParams& aRecordParams ) 
    {
    LOG1( "CCREngine::StartRecordStreamL() in, aSessionChk: %d", aSessionChk );

    // Verify session
    const TInt sessionIndex( VerifySession( aSessionChk ) );
        
    // Session exist?
    if ( sessionIndex > KErrNotFound )
        {
        CreateRecordingSinkL( sessionIndex, aRecordParams );
        iSessions[sessionIndex]->PostActionL();
        }

    LOG1( "CCREngine::StartRecordStreamL() out, sessionIndex: %d", sessionIndex );
    }

// -----------------------------------------------------------------------------
// CCREngine::RecordRtspStreamL
//
// -----------------------------------------------------------------------------
//
TUint CCREngine::RecordRtspStreamL(
    const SCRRtspParams& aRtspParams,
    const SCRRecordParams& aRecordParams ) 
    {
    LOG( "CCREngine::RecordRtspStreamL() in" );

    // Verify session
    const TInt sessionIndex( VerifyRtspSessionL( aRtspParams ) );

    // Recording sink
    CreateRecordingSinkL( sessionIndex, aRecordParams );
    TUint sessionChk( iSessions[sessionIndex]->SourceChecksum() );
    LOG1( "CCREngine::RecordRtspStreamL() out, sessionChk: %d", sessionChk );
    return sessionChk;
    }

// -----------------------------------------------------------------------------
// CCREngine::RecordDvbhStreamL
//
// -----------------------------------------------------------------------------
//
TUint CCREngine::RecordDvbhStreamL(
    const SCRLiveParams& aLiveParams,
    const SCRRecordParams& aRecordParams ) 
    {
    LOG( "CCREngine::RecordDvbhStreamL() in" );

    // Verify session
    const TInt sessionIndex( VerifyDvbhSessionL( aLiveParams ) );

    // Recording sink
    CreateRecordingSinkL( sessionIndex, aRecordParams );
    iSessions[sessionIndex]->PostActionL();
    TUint sessionChk( iSessions[sessionIndex]->SourceChecksum() );
    LOG1( "CCREngine::RecordDvbhStreamL() out, sessionChk: %d", sessionChk );
    return sessionChk;
    }

// -----------------------------------------------------------------------------
// CCREngine::PauseRecordStream
//
// -----------------------------------------------------------------------------
//
TInt CCREngine::PauseRecordStream( const TUint aSessionChk, const TBool& aStart ) 
    {
    LOG2( "CCREngine::PauseRecordStream() in, aSessionChk: %d, aStart: %d", 
                                              aSessionChk, aStart );
    // Verify session
    const TInt sessionIndex( VerifySession( aSessionChk ) );
        
    // Session exist?
    if ( sessionIndex > KErrNotFound )
        {
        TInt err( KErrNone );
        if ( aStart )
            {
            // Clip format not known, so all types must try to pause
            err = iSessions[sessionIndex]->PauseCommand(
                  CCRStreamingSession::ECRRtpRecSinkId );
            if ( err == KErrCompletion )
                {
                err = iSessions[sessionIndex]->PauseCommand(
                      CCRStreamingSession::ECR3gpRecSinkId );
                }
            
            ConnectionStatusChange( aSessionChk, ECRRecordingPaused, err );
            }
        else
            {
            // Clip format not known, so all types must try to pause
            err = iSessions[sessionIndex]->RestoreCommand(
                  CCRStreamingSession::ECRRtpRecSinkId );
            if ( err == KErrCompletion )
                {
                err = iSessions[sessionIndex]->RestoreCommand(
                      CCRStreamingSession::ECR3gpRecSinkId );
                }

            ConnectionStatusChange( aSessionChk, ECRRecordingStarted, err );
            }
        
        LOG1( "CCREngine::PauseRecordStream() out, err: %d", err );
        return err;
        }
        
    LOG1( "CCREngine::PauseRecordStream() out, sessionIndex: %d", sessionIndex );
    return sessionIndex;    
    }

// -----------------------------------------------------------------------------
// CCREngine::StopRecordStream
//
// -----------------------------------------------------------------------------
//
TInt CCREngine::StopRecordStream( const TUint aSessionChk ) 
    {
    LOG1( "CCREngine::StopRecordStream() in, aSessionChk: %d", aSessionChk );
    
    // Verify session
    const TInt sessionIndex( VerifySession( aSessionChk ) );
        
    // Session exist?
    if ( sessionIndex > KErrNotFound )
        {
        // Clip format not known, so all types must try to stop
        iSessions[sessionIndex]->StopCommand(
            CCRStreamingSession::ECRRtpRecSinkId );
        iSessions[sessionIndex]->StopCommand(
            CCRStreamingSession::ECR3gpRecSinkId );
        // Possible error(s) ignored
        }
        
    LOG1( "CCREngine::StopRecordStream() out, sessionIndex: %d", sessionIndex );
    return sessionIndex;    
    }

// -----------------------------------------------------------------------------
// CCREngine::StartTimeShift
// Existing stream play will be splitted to two sessions. Existing source will
// be directed to recording and existing sink will get new source from playback.
// -----------------------------------------------------------------------------
//
TInt CCREngine::StartTimeShiftL( const TUint aSessionChk ) 
    {
    LOG1( "CCREngine::StartTimeShiftL() in, aSessionChk: %d", aSessionChk );

#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT

    // Verify session
    const TInt currentSession( VerifySession( aSessionChk ) );
    User::LeaveIfError( currentSession );

    // Initial time shift clip name
    TPath initialName( KDvrTimeShiftFile );
    initialName.AppendNum( 0 );
    
    // RTP clip handler
    CreateClipHandlerL();

    // Start recording of stream
    SCRRecordParams recordParams;
    recordParams.iFileName.Set( initialName );
    recordParams.iFormat = ECRRecordTimeShift;
    iSessions[currentSession]->CreateRtpRecordSinkL( recordParams, iClipHandler );
    iSessions[currentSession]->PostActionL();

    // Create new session for time shift clip playback
    SCRRtpPlayParams params;
    params.iFileName = initialName;
    const TInt timeShiftSession( VerifyRtpFileSessionL( params ) );
    iSessions[timeShiftSession]->PostActionL();

    // Transfer ownership of existing XPS sink to a new session
    User::LeaveIfError( iSessions[currentSession]->TransferSink( 
        CCRStreamingSession::ECRXpsSinkId, *iSessions[timeShiftSession] ) );

    // Session checksum
    TUint sessionChk( iSessions[timeShiftSession]->SourceChecksum() );
    LOG1( "CCREngine::StartTimeShiftL() out, sessionChk: %d", sessionChk );
    return sessionChk;

#else // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    ( void )aSessionChk;
    return KErrNotSupported;
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    }

// -----------------------------------------------------------------------------
// CCREngine::StopTimeShift
// Streaming is set back to one session. Streaming orginal source stays and sink
// from temporary playback will be moved back to orginal session.
// -----------------------------------------------------------------------------
//
TInt CCREngine::StopTimeShift(
    const TUint aTimeShiftChk,
    const TUint aCurrentChk ) 
    {
    LOG2( "CCREngine::StopTimeShift() in, aTimeShiftChk: %d, aCurrentChk: %d", 
                                          aTimeShiftChk, aCurrentChk );
#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT

    // Verify session
    int ret( KErrNotFound );
    const TInt currentSession( VerifySession( aCurrentChk ) );
    const TInt timeShiftSession( VerifySession( aTimeShiftChk ) );
        
    // Session exist?
    if ( currentSession > KErrNotFound && timeShiftSession > KErrNotFound )
        {
        // Stop time shift clip recording
        iSessions[currentSession]->StopCommand( 
            CCRStreamingSession::ECR3gpRecSinkId );
        
        // Stop time shift clip playback
        iSessions[timeShiftSession]->StopCommand();

        // Transfer ownership of existing XPS sink back to the old session
        ret = iSessions[timeShiftSession]->TransferSink( 
            CCRStreamingSession::ECRXpsSinkId, *iSessions[currentSession] );
        
        // Delete second session
        SessionStop( iSessions[timeShiftSession] );
        }
    
    // Clip handler not needed any longer
    DeleteClipHandler( currentSession );

    LOG1( "CCREngine::StopTimeShift() out, ret: %d", ret );
    return ret;    

#else // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    ( void )aTimeShiftChk;
    ( void )aCurrentChk;
    return KErrNotSupported;
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    }

// -----------------------------------------------------------------------------
// CCREngine::PlayCommand
//
// -----------------------------------------------------------------------------
//
TInt CCREngine::PlayCommand(
    const TUint aSessionChk,
    const TReal aStartPos,
    const TReal aEndPos ) 
    {
    LOG1( "CCREngine::PlayCommand(), aSessionChk: %d", aSessionChk );

    // Verify session
    const TInt sessionIndex( VerifySession( aSessionChk ) );
        
    // New session needed?
    if ( sessionIndex > KErrNotFound )
        {
        return iSessions[sessionIndex]->PlayCommand( aStartPos, aEndPos );
        }
        
    return KErrNotReady;
    }

// -----------------------------------------------------------------------------
// CCREngine::PauseCommand
//
// -----------------------------------------------------------------------------
//
TInt CCREngine::PauseCommand( const TUint aSessionChk )
    {
    LOG1( "CCREngine::PauseCommand(), aSessionChk: %d", aSessionChk );

    // Verify session
    const TInt sessionIndex( VerifySession( aSessionChk ) );
        
    // New session needed?
    if ( sessionIndex > KErrNotFound )
        {
        return iSessions[sessionIndex]->PauseCommand();
        }
        
    return KErrNotReady;
    }

// -----------------------------------------------------------------------------
// CCREngine::StopCommand
//
// -----------------------------------------------------------------------------
//
TInt CCREngine::StopCommand( const TUint aSessionChk )
    {
    LOG1( "CCREngine::StopCommand(), aSessionChk: %d", aSessionChk );

    // Verify session
    const TInt sessionIndex( VerifySession( aSessionChk ) );
        
    // New session needed?
    if ( sessionIndex > KErrNotFound )
        {
        return iSessions[sessionIndex]->StopCommand();
        }
        
    return KErrNotReady;
    }

// -----------------------------------------------------------------------------
// CCREngine::SetPosition
//
// -----------------------------------------------------------------------------
//
TInt CCREngine::SetPosition(
    const TUint aSessionChk,
    const TInt64 aPosition )
    {
    LOG1( "CCREngine::SetPosition(), aSessionChk: %d", aSessionChk );

    const TInt sessionIndex( VerifySession( aSessionChk ) );
    if ( sessionIndex > KErrNotFound )
        {
        return iSessions[sessionIndex]->SetPosition( aPosition );
        }
   
    return KErrNotReady;
    }

// -----------------------------------------------------------------------------
// CCREngine::GetPosition
//
// -----------------------------------------------------------------------------
//
TInt CCREngine::GetPosition(
    const TUint aSessionChk,
    TInt64& aPosition,
    TInt64& aDuration ) 
    {
    const TInt sessionIndex( VerifySession( aSessionChk ) );
    if ( sessionIndex > KErrNotFound )
        {
        return iSessions[sessionIndex]->GetPosition( aPosition, aDuration );
        }

    return KErrNotReady;
    }

// -----------------------------------------------------------------------------
// CCREngine::CloseSession
//
// -----------------------------------------------------------------------------
//
TInt CCREngine::CloseSession( const TUint aSessionChk )
    {
    LOG1( "CCREngine::CloseSession(), aSessionChk: %d", aSessionChk );
    
    // Verify session
    const TInt sessionIndex( VerifySession( aSessionChk ) );
    if ( sessionIndex > KErrNotFound )
        {
        SessionStop( iSessions[sessionIndex] );
        }
    
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CCREngine::PlayNullSourceL
//
// -----------------------------------------------------------------------------
//
TUint CCREngine::PlayNullSourceL() 
    {
    LOG( "CCREngine::PlayNullSourceL() in" );

    _LIT( KNullSourceClip, "NullSource.rtp" );
    TFileName nullSourcePath;
#if ( defined( __WINS__ ) || defined( __WINSCW__ ) )
    nullSourcePath = PathInfo::PhoneMemoryRootPath();
#else // __WINS__ || __WINSCW__
    nullSourcePath = PathInfo::MemoryCardRootPath();
#endif // __WINS__ || __WINSCW__
    nullSourcePath.Append( KNullSourceClip );

    TInt sessionIndex( VerifySession( nullSourcePath ) );
    if ( sessionIndex == KErrNotFound )
        {
        CCRStreamingSession* session = CCRStreamingSession::NewL(
                                       iSockServer, iConnection, *this );        
        CleanupStack::PushL( session );
        User::LeaveIfError( iSessions.Append( session ) );
        CleanupStack::Pop( session );

        // Only one source per session
        sessionIndex = iSessions.Count() - 1;
        iSessions[sessionIndex]->OpenSourceL( nullSourcePath );
        }

    iSessions[sessionIndex]->CreateXpsSinkL();
    //iSessions[sessionIndex]->CreateNullSinkL();
    iSessions[sessionIndex]->PostActionL();
    TUint sessionChk( iSessions[sessionIndex]->SourceChecksum() );
    LOG1( "CCREngine::PlayNullSourceL() out, sessionChk: %d", sessionChk );
    return sessionChk;
    }

// -----------------------------------------------------------------------------
// CCREngine::PlayRtspUrlToNullSinkL
//
// -----------------------------------------------------------------------------
//
TUint CCREngine::PlayRtspUrlToNullSinkL( const SCRRtspParams& aRtspParams ) 
    {
    LOG( "CCREngine::PlayRtspUrlToNullSinkL() in" );
    
    const TInt sessionIndex( VerifyRtspSessionL( aRtspParams ) );
    iSessions[sessionIndex]->CreateNullSinkL();
    TUint sessionChk( iSessions[sessionIndex]->SourceChecksum() );
    LOG1( "CCREngine::PlayRtspUrlToNullSinkL() out, sessionChk: %d", sessionChk );
    return sessionChk;
    }

// -----------------------------------------------------------------------------
// CCREngine::CreateConnectionL
// 
// -----------------------------------------------------------------------------
//
void CCREngine::CreateConnectionL( void )
    {
    LOG( "CCREngine::CreateConnectionL() in" );

    if ( iConnection == NULL )
        {
        User::LeaveIfError( iSockServer.Connect( KESockDefaultMessageSlots * 3 ) );
        iConnection = CCRConnection::NewL( iSockServer );
        User::LeaveIfError( iConnection->RegisterObserver( this ) );
        
        // Informer
        if ( iInformer == NULL )
            {
            iInformer = CCRClientInformer::NewL();
            }
        }
    
    LOG( "CCREngine::CreateConnectionL() out" );
    }

// -----------------------------------------------------------------------------
// CCREngine::CreateClipHandlerL
// 
// -----------------------------------------------------------------------------
//
void CCREngine::CreateClipHandlerL( void )
    {
    LOG1( "CCREngine::CreateClipHandlerL(), iClipHandler: %d", iClipHandler );

#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT

    if ( !iClipHandler )
        {
        iClipHandler = CRtpClipHandler::NewL();
        }
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    }

// -----------------------------------------------------------------------------
// CCREngine::VerifyRtspSessionL
// Go through stream sessions and try to find if RTSP stream is already playing.
// -----------------------------------------------------------------------------
//
TInt CCREngine::VerifyRtspSessionL( const SCRRtspParams& aRtspParams ) 
    {
    TInt sessionIndex( VerifySession( aRtspParams.iUrl ) );
    
    // New session needed?
    if ( sessionIndex == KErrNotFound )
        {
        CCRStreamingSession* session = CCRStreamingSession::NewL(
                                       iSockServer, iConnection, *this ); 
        CleanupStack::PushL( session );       
        User::LeaveIfError( iSessions.Append( session ) );
        CleanupStack::Pop( session );

        // Only one source per session
        sessionIndex = iSessions.Count() - 1;
        iSessions[sessionIndex]->OpenSourceL( aRtspParams, aRtspParams.iUrl );
        }
    
    User::LeaveIfError( sessionIndex );
    return sessionIndex;
    }

// -----------------------------------------------------------------------------
// CCREngine::VerifyDvbhSessionL
// Go through stream sessions and try to find if DVB-H live is already playing.
// -----------------------------------------------------------------------------
//
TInt CCREngine::VerifyDvbhSessionL( const SCRLiveParams& aLiveParams ) 
    {
    // Verify session
    HBufC* definition = HBufC::NewLC( aLiveParams.iSdpData.Length() );
    definition->Des().Copy( aLiveParams.iSdpData );
    TInt sessionIndex( VerifySession( *definition ) );

    // New session needed?
    if ( sessionIndex == KErrNotFound )
        {
        CCRStreamingSession* session = CCRStreamingSession::NewL(
                                       iSockServer, iConnection, *this );        
        CleanupStack::PushL( session );
        User::LeaveIfError( iSessions.Append( session ) );
        CleanupStack::Pop( session );
        
        // Only one source per session
        sessionIndex = iSessions.Count() - 1;
        iSessions[sessionIndex]->OpenSourceL( aLiveParams, *definition );
        }
    
    CleanupStack::PopAndDestroy( definition );
    User::LeaveIfError( sessionIndex );
    return sessionIndex;
    }

// -----------------------------------------------------------------------------
// CCREngine::VerifyRtpFileSessionL
// Go through stream sessions and try to find if RTP clip is already playing.
// -----------------------------------------------------------------------------
//
TInt CCREngine::VerifyRtpFileSessionL( const SCRRtpPlayParams& aRtpParams ) 
    {
#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT

    // Verify session
    TInt sessionIndex( VerifySession( aRtpParams.iFileName ) );
        
    // New session needed?
    if ( sessionIndex == KErrNotFound )
        {
        
        CCRStreamingSession* session = CCRStreamingSession::NewL(
                                       iSockServer, iConnection, *this );        
        CleanupStack::PushL( session );
        User::LeaveIfError( iSessions.Append( session ) );
        CleanupStack::Pop( session );

        // RTP clip handler
        CreateClipHandlerL();
        
        // Only one source per session
        sessionIndex = iSessions.Count() - 1;
        iSessions[sessionIndex]->OpenSourceL( aRtpParams, iClipHandler, 
                                              aRtpParams.iFileName );
        }

    User::LeaveIfError( sessionIndex );
    return sessionIndex;

#else // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    ( void )aRtpParams;
    return KErrNotSupported;
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    }

// -----------------------------------------------------------------------------
// CCREngine::VerifyRtpFileSessionL
// Go through stream sessions and try to find if RTP clip is already playing.
// -----------------------------------------------------------------------------
//
TInt CCREngine::VerifyRtpFileSessionL( const RFile& aFileHandle ) 
    {
#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT

    TFileName fileName( KNullDesC );
    aFileHandle.FullName( fileName );
    TInt sessionIndex( VerifySession( fileName ) );
        
    // New session needed?
    if ( sessionIndex == KErrNotFound )
        {
        CCRStreamingSession* session = CCRStreamingSession::NewL(
                                       iSockServer, iConnection, *this );        
        CleanupStack::PushL( session );
        User::LeaveIfError( iSessions.Append( session ) );
        CleanupStack::Pop( session );

        // RTP clip handler
        CreateClipHandlerL();
        
        // Only one source per session
        sessionIndex = iSessions.Count() - 1;
        iSessions[sessionIndex]->OpenSourceL( aFileHandle, iClipHandler, fileName );
        }

    User::LeaveIfError( sessionIndex );
    return sessionIndex;

#else // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    ( void )aFileHandle;
    return KErrNotSupported;
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    }

// -----------------------------------------------------------------------------
// CCREngine::VerifySession
// Go through stream sessions and try to find stream is already playing.
// -----------------------------------------------------------------------------
//
TInt CCREngine::VerifySession( const TDesC& aName ) 
    {
    if ( iSessions.Count() > 0 )
        {
        for ( TInt i( iSessions.Count() - 1 ); i >= 0; i-- )
            {
            const TUint chksm( iSessions[i]->SourceDefinition( aName ) );
            if ( chksm == iSessions[i]->SourceChecksum() )
                {
                return i;
                }
            }
        }

    return KErrNotFound;
    }

// -----------------------------------------------------------------------------
// CCREngine::VerifySession
// Go through stream sessions and try to find session index from active sessions.
// -----------------------------------------------------------------------------
//
TInt CCREngine::VerifySession( const TUint aSessionChk ) 
    {
    if ( iSessions.Count() > 0 )
        {
        for ( TInt i( iSessions.Count() - 1 ); i >= 0; i-- )
            {
            if ( aSessionChk == iSessions[i]->SourceChecksum() )
                {
                return i;
                }
            }
        }

    return KErrNotFound;
    }

//-----------------------------------------------------------------------------
// CCREngine::CreateRecordingSinkL
//
//-----------------------------------------------------------------------------
//
void CCREngine::CreateRecordingSinkL(
    const TInt aSessionIndex,
    const SCRRecordParams& aRecordParams )
    {
    LOG1( "CCREngine::CreateRecordingSinkL(), aSessionIndex: %d", aSessionIndex );
    
    User::LeaveIfError( aSessionIndex );
    switch ( aRecordParams.iFormat )
        {
#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT
        case ECRRecordFormatRtp:
            // RTP clip handler
            CreateClipHandlerL();
            iSessions[aSessionIndex]->CreateRtpRecordSinkL(
                aRecordParams, iClipHandler );
            break;
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT

        case ECRRecordFormat3gp:
            iSessions[aSessionIndex]->Create3gpRecordSinkL( aRecordParams );
            break;

        default:
            LOG( "CCREngine::CreateRecordingSinkL(), Clip format invalid !");
            User::Leave( KErrNotSupported );
            break;
        }
    }

//-----------------------------------------------------------------------------
// CCREngine::SessionStopCallBack
//
//-----------------------------------------------------------------------------
//
TInt CCREngine::SessionStopCallBack ( TAny* aThis ) 
    {
    LOG( "CCREngine::SessionStopCallBack()" );
    
    CCREngine* self = static_cast<CCREngine*>( aThis );
    self->DoSessionStop();
    return self->iSessionsToDelete.Count(); 
    }
    
//-----------------------------------------------------------------------------
// CCREngine::DoSessionStop
//
//-----------------------------------------------------------------------------
//
void CCREngine::DoSessionStop( void ) 
    {
    LOG1( "CCREngine::DoSessionStop() in, iSessionsToDelete.Count = %d", iSessionsToDelete.Count() );
    
    for ( TInt i( iSessionsToDelete.Count() - 1 ); i >= 0; i-- )
        {
        for ( TInt j( iSessions.Count() - 1 ); j >= 0; j-- )
            {
            if ( iSessions[j] == iSessionsToDelete[i] )
                {
                delete iSessions[j];
                iSessions.Remove( j ); 
                }
            }
        }
    
    // Supposed to be empty by now
    DeleteClipHandler( KErrNotFound );
    iSessionsToDelete.Reset();
    LOG( "CCREngine::DoSessionStop() out" );
    }
    
//-----------------------------------------------------------------------------
// CCREngine::DeleteClipHandler
//
//-----------------------------------------------------------------------------
//
void CCREngine::DeleteClipHandler( const TInt aCurrentSessionIndex ) 
    {
    LOG1( "CCREngine::DeleteClipHandler(), aCurrentSessionIndex: %d",
                                           aCurrentSessionIndex );
#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT

    if ( iClipHandler )
        {
        // Find out clip handler usage
        TBool notUsed( ETrue );
        for ( TInt i( iSessions.Count() - 1 ); i >= 0; i-- )
            {
            if ( i != aCurrentSessionIndex && iSessions[i]->ClipHandlerUsed() )
                {
                notUsed = EFalse;
                }
            }

        // Clip handler not needed any longer?
        if ( notUsed )
            {
            delete iClipHandler; iClipHandler = NULL;    
            LOG( "CCREngine::DeleteClipHandler(), RTP ClipHandler deleted !" );
            }
        }
    
#else // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    ( void )aCurrentSessionIndex;
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    }
    
// -----------------------------------------------------------------------------
// CCREngine::Read
// Read from the client thread, if unsuccessful, panic the client.
// -----------------------------------------------------------------------------
//
void CCREngine::Read(
    const RMessage2& aMessage, 
    const TInt& aParam,
    TDes8& aDes ) 
    {
    TRAPD( err, aMessage.ReadL( aParam, aDes ) );
    if ( err )
        {
        PanicClient( ECRPanicBadDescriptor, aMessage );
        }
    }

// -----------------------------------------------------------------------------
// CCREngine::Read
// Read from the client thread, if unsuccessful, panic the client.
// -----------------------------------------------------------------------------
//
void CCREngine::Read(
    const RMessage2& aMessage, 
    const TInt& aParam,
    TDes16& aDes ) 
    {
    TRAPD( err, aMessage.ReadL( aParam, aDes ) );
    if ( err )
        {
        PanicClient( ECRPanicBadDescriptor, aMessage );
        }
    }

// -----------------------------------------------------------------------------
// CCREngine::Write
// Write to the client thread, if unsuccessful, panic the client.
// -----------------------------------------------------------------------------
//
void CCREngine::Write(
    const RMessage2& aMessage, 
    const TInt& aParam,
    const TDesC8& aDes ) 
    {
    TRAPD( err, aMessage.WriteL( aParam, aDes ) );
    if ( err )
        {
        PanicClient( ECRPanicBadDescriptor, aMessage );
        }
    }

// -----------------------------------------------------------------------------
// CCREngine::Write
// Write to the client thread, if unsuccessful, panic the client.
// -----------------------------------------------------------------------------
//
void CCREngine::Write(
    const RMessage2& aMessage, 
    const TInt& aParam,
    const TDesC16& aDes ) 
    {
    TRAPD( err, aMessage.WriteL( aParam, aDes ) );
    if ( err )
        {
        PanicClient( ECRPanicBadDescriptor, aMessage );
        }
    }

// -----------------------------------------------------------------------------
// CCREngine::PanicClient
//
// -----------------------------------------------------------------------------
//
void CCREngine::PanicClient( TInt aPanic, const RMessage2& aMessage )
    {
    // Panic the client on server side
    _LIT( KRbfClientFault, "Common recording engine" );
    aMessage.Panic( KRbfClientFault, aPanic );
    }   
    
//  End of File
