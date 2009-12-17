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
* Description:    Class for streaming session. Owns a source, number of buffers*
*/




// INCLUDES
#include "VideoServiceUtilsConf.hrh"
#include "CCRStreamingSession.h"
#include "CCRRtspSink.h"
#include "CCRNullSink.h"
#include "CCRPacketBuffer.h"
#include <ipvideo/CRTypeDefs.h>
#include "CCRRtspPacketSource.h"
#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT
#include "CCRXpsSink.h"
#include "CCRRtpRecordSink.h"
#include "CCRRtpFileSource.h"
#include "CCRNullSource.h"
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
#include "MCRStreamObserver.h"
#include "CCRPacketSinkBase.h"
#include "CCRConnection.h"
#include "CCRengine.h"
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KMaxRtspPackets( 400 );
#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT
const TInt KMaxRtpPackets( 500 );
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRStreamingSession::NewL
//
// -----------------------------------------------------------------------------
//      
CCRStreamingSession* CCRStreamingSession::NewL(
    RSocketServ& aSockServer,
    CCRConnection* aConnection,
    CCREngine& aEngine )
    {
    CCRStreamingSession* self = new( ELeave ) CCRStreamingSession(
                                aSockServer, aConnection, aEngine );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRStreamingSession::CCRStreamingSession
//
// -----------------------------------------------------------------------------
//      
CCRStreamingSession::CCRStreamingSession(
    RSocketServ& aSockServer,
    CCRConnection* aConnection,
    CCREngine& aEngine ) 
  : iSockServer( aSockServer ),
    iConnection( aConnection ),
    iEngine( aEngine )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRStreamingSession::ConstructL
//
// -----------------------------------------------------------------------------
//          
void CCRStreamingSession::ConstructL()
    {
    LOG( "CCRStreamingSession::ConstructL()" );

    // Note, quite high priority
    iCleanUp = new ( ELeave ) CAsyncCallBack( CActive::EPriorityStandard ); 
    }
    
// -----------------------------------------------------------------------------
// CCRStreamingSession::~CCREStreamingSession
//
// -----------------------------------------------------------------------------
//          
CCRStreamingSession::~CCRStreamingSession()
    {
    LOG( "CCRStreamingSession::~CCRStreamingSession()" );
    
    delete iCleanUp;     
    iSinks.ResetAndDestroy();
    iSinksToDelete.Reset(); 
    delete iSource;
    delete iBuffer;
    iConnection = NULL;
    }
    
// -----------------------------------------------------------------------------
// CCRStreamingSession::OpenSourceL
// Opens RTSP streaming source.
// -----------------------------------------------------------------------------
//      
void CCRStreamingSession::OpenSourceL(
    const SCRRtspParams& aParams,
    const TDesC& aSessionDefinition )
    {
    LOG( "CCRStreamingSession::OpenSourceL(), RTSP Url" )
    
    if ( !iSource && iConnection )
        {
        iSourceChecksum = SourceDefinition( aSessionDefinition );
        iSource = CCRRtspPacketSource::NewL(
                  aParams, *iConnection, iSockServer, *this, *this );
        iSource->RegisterConnectionObs( &iEngine );
        }    
    }
    
// -----------------------------------------------------------------------------
// CCRStreamingSession::OpenSourceL
// Opens RTP clip streaming source.
// -----------------------------------------------------------------------------
//      
void CCRStreamingSession::OpenSourceL(
    const SCRRtpPlayParams& aParams,
    CRtpClipHandler*& aClipHandler,
    const TDesC& aSessionDefinition )
    {
    LOG( "CCRStreamingSession::OpenSourceL(), RTP clip" )

#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT

    if ( !iSource )
        {
        iSourceChecksum = SourceDefinition( aSessionDefinition );
        iBuffer = CCRPacketBuffer::NewL( KMaxRtpPackets );
        iSource = CCRRtpFileSource::NewL( aParams, aClipHandler, *this, *this );
        iSource->SetBuffer( iBuffer );
        }

#else // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    ( void )aParams;
    ( void )aClipHandler;
    ( void )aSessionDefinition;
    User::Leave( KErrNotSupported );
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    }

// -----------------------------------------------------------------------------
// CCRStreamingSession::OpenSourceL
// Opens RTP clip streaming source.
// -----------------------------------------------------------------------------
//      
void CCRStreamingSession::OpenSourceL(
    const RFile& aRtpHandle,
    CRtpClipHandler*& aClipHandler,
    const TDesC& aSessionDefinition )
    {
    LOG( "CCRStreamingSession::OpenSourceL(), RTP handle" )

#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT

    if ( !iSource )
        {
        iSourceChecksum = SourceDefinition( aSessionDefinition );
        iBuffer = CCRPacketBuffer::NewL( KMaxRtpPackets );
        iSource = CCRRtpFileSource::NewL( aRtpHandle, aClipHandler, *this, *this );
        iSource->SetBuffer( iBuffer );
        }

#else // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    ( void )aRtpHandle;
    ( void )aClipHandler;
    ( void )aSessionDefinition;
    User::Leave( KErrNotSupported );
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    }

// -----------------------------------------------------------------------------
// CCRStreamingSession::OpenSourceL
// Opens DVB-H live streaming source.
// -----------------------------------------------------------------------------
//      
void CCRStreamingSession::OpenSourceL(
    const SCRLiveParams& /*aLiveParams*/,
    const TDesC& /*aSessionDefinition*/ )
    {
    LOG( "CCRStreamingSession::OpenSourceL(), DVB-H live" )

    User::Leave( KErrNotSupported );
    }

// -----------------------------------------------------------------------------
// CCRStreamingSession::OpenSourceL
// Opens RTP clip as a live streaming source.
// -----------------------------------------------------------------------------
//      
void CCRStreamingSession::OpenSourceL( const TDesC& aSessionDefinition )
    {
    LOG( "CCRStreamingSession::OpenSourceL(), Null" )

#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT

    if ( !iSource )
        {
        iSourceChecksum = SourceDefinition( aSessionDefinition );
        iBuffer = CCRPacketBuffer::NewL( KMaxRtpPackets );
        iSource = CCRNullSource::NewL( aSessionDefinition, *this, *this );
        iSource->SetBuffer( iBuffer );
        }

#else // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    ( void )aSessionDefinition;
    User::Leave( KErrNotSupported );
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    }

// -----------------------------------------------------------------------------
// CCRStreamingSession::SourceDefinition
//
// -----------------------------------------------------------------------------
//      
TUint CCRStreamingSession::SourceDefinition( const TDesC& aName )
    {
    TUint checkSum( 0 );
    for ( TInt i( aName.Length() - 1 ); i >= 0; i-- )
        {
        checkSum += aName[i];
        }
    
	// And for rtsp packet source do use different id
	// if udp is blocked and we're using tcp then.    
    if ( iSource && iSource->Id() == ECRRtspSourceId &&
    	 iConnection && 
    	 iConnection->GetHeuristic ( CCRConnection::EUdpStreamingBlocked ) )
    	 {
    	 checkSum++;
    	 }
            
    return checkSum;
    }

// -----------------------------------------------------------------------------
// CCRStreamingSession::SourceChecksum
//
// -----------------------------------------------------------------------------
//      
TUint CCRStreamingSession::SourceChecksum()
    {
    return iSourceChecksum;
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::CreateAndSetBufferL
//
//-----------------------------------------------------------------------------
//    
void CCRStreamingSession::CreateAndSetBufferL()
    {
    if ( iSource && iBuffer == NULL )
        {
        iBuffer = CCRPacketBuffer::NewL( KMaxRtspPackets );
        iSource->SetBuffer( iBuffer );
        }
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::CreateRtspSinkL
//
//-----------------------------------------------------------------------------
//
void CCRStreamingSession::CreateRtspSinkL( const TInt& aLoopbackPort )
    {
    LOG( "CCRStreamingSession::CreateRtspSinkL()" )

    if ( iConnection )
        {
        // Only one RTSP sink at the time
        DeleteSink( ECRRtspSinkId ); 
        
        // New sink
        CCRRtspSink* sink = CCRRtspSink::NewL(
            *iConnection, iSockServer, ECRRtspSinkId, aLoopbackPort, *this );
        CleanupStack::PushL( sink );
        User::LeaveIfError( iSinks.Append( sink ) );
        CleanupStack::Pop( sink );
        }
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::CreateXpsSinkL
//
//-----------------------------------------------------------------------------
//    
void CCRStreamingSession::CreateXpsSinkL()
    {
    LOG( "CCRStreamingSession::CreateXpsSinkL()" )

#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT

    CCRXpsSink* sink = CCRXpsSink::NewL( ECRXpsSinkId, *this );
    CleanupStack::PushL( sink );
    User::LeaveIfError( iSinks.Append( sink ) );
    CleanupStack::Pop( sink );
#else // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    User::Leave( KErrNotSupported );
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::Create3gpRecordSinkL
//
//-----------------------------------------------------------------------------
//    
void CCRStreamingSession::Create3gpRecordSinkL(
    const SCRRecordParams& /*aRecordParams*/ )
    {
    LOG( "CCRStreamingSession::Create3gpRecordSinkL()" )

    User::Leave( KErrNotSupported );
    /*
    CCR3gpRecordSink* sink = CCR3gpRecordSink::NewL( ECR3gpRecSinkId, *this );
    CleanupStack::PushL( sink );
    User::LeaveIfError( iSinks.Append( sink ) );
    CleanupStack::Pop( sink );
    */
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::CreateRtpRecordSinkL
//
//-----------------------------------------------------------------------------
//    
void CCRStreamingSession::CreateRtpRecordSinkL(
    const SCRRecordParams& aRecordParams,
    CRtpClipHandler*& aClipHandler )
    {
    LOG( "CCRStreamingSession::CreateRtpRecordSinkL()" )

#ifdef RD_IPTV_FEA_RTP_CLIP_SUPPORT
    // Create record sink
    CCRRtpRecordSink* sink = CCRRtpRecordSink::NewL( 
        aRecordParams, ECRRtpRecSinkId, *this, &iEngine, aClipHandler );
    CleanupStack::PushL( sink );
    User::LeaveIfError( iSinks.Append( sink ) );
    CleanupStack::Pop( sink );

#else // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    ( void )aRecordParams;
    ( void )aClipHandler;
    User::Leave( KErrNotSupported );
#endif // RD_IPTV_FEA_RTP_CLIP_SUPPORT
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::PostActionL
//
//-----------------------------------------------------------------------------
//    
void CCRStreamingSession::PostActionL()
    {
    User::LeaveIfNull( iSource );
    iSource->PostActionL();
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::PlayCommand
//
//-----------------------------------------------------------------------------
//    
TInt CCRStreamingSession::PlayCommand(
    const TReal& aStartPos,
    const TReal& aEndPos )
    {
    if ( iSource )
        {
        return iSource->Play( aStartPos, aEndPos );
        }
    
    return KErrCompletion;
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::PauseCommand
//
//-----------------------------------------------------------------------------
//    
TInt CCRStreamingSession::PauseCommand()
    {
    if ( iSource )
        {
        return iSource->Pause();
        }
    
    return KErrCompletion;
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::StopCommand
//
//-----------------------------------------------------------------------------
//    
TInt CCRStreamingSession::StopCommand()
    {
    if ( iSource && iSinks.Count() >= 1 )
        {
        return iSource->Stop();
        }
    
    return KErrCompletion;
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::SetPosition
//
//-----------------------------------------------------------------------------
//    
TInt CCRStreamingSession::SetPosition( const TInt64 aPosition )
    {
    if ( iSource )
        {
        return iSource->SetPosition( aPosition );
        }

    return KErrCompletion;
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::GetPosition
//
//-----------------------------------------------------------------------------
//    
TInt CCRStreamingSession::GetPosition( TInt64& aPosition, TInt64& aDuration )
    {
    if ( iSource )
        {
        return iSource->GetPosition( aPosition, aDuration );
        }

    return KErrCompletion;
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::PauseCommand
//
//-----------------------------------------------------------------------------
//    
TInt CCRStreamingSession::PauseCommand( const TCRSinkId& aSinkId )
    {
    // Pauses current sink action
    for ( TInt i( iSinks.Count() - 1 ); i >= 0; i-- )
        {
        if ( aSinkId == iSinks[i]->Id() )
            {
            return iSinks[i]->Pause();
            }
        }

    return KErrCompletion;
    }
    
//-----------------------------------------------------------------------------
// CCRStreamingSession::RestoreCommand
//
//-----------------------------------------------------------------------------
//    
TInt CCRStreamingSession::RestoreCommand( const TCRSinkId& aSinkId )
    {
    // Pauses current sink action
    for ( TInt i( iSinks.Count() - 1 ); i >= 0; i-- )
        {
        if ( aSinkId == iSinks[i]->Id() )
            {
            return iSinks[i]->Restore();
            }
        }

    return KErrCompletion;
    }
    
//-----------------------------------------------------------------------------
// CCRStreamingSession::StopCommand
//
//-----------------------------------------------------------------------------
//    
TInt CCRStreamingSession::StopCommand( const TCRSinkId& aSinkId )
    {
    // Stop current sink action
    for ( TInt i( iSinks.Count() - 1 ); i >= 0; i-- )
        {
        if ( aSinkId == iSinks[i]->Id() )
            {
            iSinks[i]->Stop();
            return KErrNone;
            }
        }

    return KErrCompletion;
    }
    
//-----------------------------------------------------------------------------
// CCRStreamingSession::TransferSink
//
//-----------------------------------------------------------------------------
//    
TInt CCRStreamingSession::TransferSink(
    const TCRSinkId& aSinkId,
    CCRStreamingSession& aTargetSession )
    {
    LOG1( "CCRStreamingSession::TransferSink(), aSinkId: %d", aSinkId );
    
    for ( TInt i( iSinks.Count() - 1 ); i >= 0; i-- )
        {
        if ( iSinks[i]->Id() == aSinkId )
            {
            TInt err( aTargetSession.AddNewSink( iSinks[i] ) );
            if ( !err )
                {
                iBuffer->RemoveSink( iSinks[i] );
                iSinks.Remove( i );
                }
            
            return err;
            }
        }

    return KErrCompletion;
    }
    
//-----------------------------------------------------------------------------
// CCRStreamingSession::AddNewSink
//
//-----------------------------------------------------------------------------
//    
TInt CCRStreamingSession::AddNewSink( CCRPacketSinkBase* aSink )
    {
    LOG1( "CCRStreamingSession::AddNewSink(), aSink->Id: %d", aSink->Id() );

    for ( TInt i( iSinks.Count() - 1 ); i >= 0; i-- )
        {
        if ( aSink->Id() == iSinks[i]->Id() )
            {
            LOG( "CCRStreamingSession::AddNewSink(), Sink already exist !" );
            return KErrInUse;
            }
        }

    // Add new sink
    TInt err( iSinks.Append( aSink ) );
    if ( !err )
        {
        err = iBuffer->AddSink( iSinks[iSinks.Count() - 1] );
        }

    return err;
    }
    
//-----------------------------------------------------------------------------
// CCRStreamingSession::ClipHandlerUsed
//
//-----------------------------------------------------------------------------
//    
TBool CCRStreamingSession::ClipHandlerUsed()
    {
    // Used in source
    if ( iSource && iSource->Id() == ECRRtpSourceId )
        {
        return ETrue;
        }
    
    // Used in any Sink
    for ( TInt i( iSinks.Count() - 1 ); i >= 0; i-- )
        {
        if ( iSinks[i]->Id() == ECRRtpRecSinkId )
            {
            return ETrue;
            }
        }
    
    return EFalse;
    }
    
//-----------------------------------------------------------------------------
// CCRStreamingSession::CreateNullSinkL
//
//-----------------------------------------------------------------------------
//
void CCRStreamingSession::CreateNullSinkL()
    {
    CCRNullSink* sink = CCRNullSink::NewL( ECRNullSinkId, *this );
    sink->RegisterConnectionObs( &iEngine );
    CleanupStack::PushL( sink );
    User::LeaveIfError( iSinks.Append( sink ) );
    CleanupStack::Pop( sink );
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::InitializeSinks
//
//-----------------------------------------------------------------------------
//    
void CCRStreamingSession::InitializeSinks() 
    {
    if ( iSource && iBuffer )
        {
        TPtrC8 sdp( NULL, 0 );
        TInt err( iSource->GetSdp( sdp ) );
        if ( err )
            {
            LOG1( "CCRStreamingSession::InitializeSinks(), GetSdp() Failed: %d", err );
            SourceStop();
            }
        else
            {
            for ( TInt i( 0 ); i < iSinks.Count(); i++ ) 
                {
                TRAP( err, iSinks[i]->SetSdpL( sdp ) );
                if ( err )
                    {
                    LOG1( "CCRStreamingSession::InitializeSinks(), SetSdpL() Failed: %d", err );
                    SinkStops( iSinks[i]->Id() ); 
                    return;
                    }
            
                iSinks[i]->SetBuffer( iBuffer );
                err = iBuffer->AddSink( iSinks[i] );
                if ( err )
                    {
                    LOG1( "CCRStreamingSession::InitializeSinks(), AddSink() Failed: %d", err );
                    SourceStop();
                    return;
                    }
                }

            iEngine.ConnectionStatusChange(
                SourceChecksum(), MCRConnectionObserver::ECRSdpAvailable, KErrNone );            
            }
        }
    }    

//-----------------------------------------------------------------------------
// CCRStreamingSession::SetSeqAndTs()
//-----------------------------------------------------------------------------
//
void CCRStreamingSession::SetSeqAndTs() 
    {
    if ( iSource )
        {
        TUint audioSeq( 0 );
        TUint audioTS( 0 );
        TUint videoSeq( 0 );
        TUint videoTS( 0 );
        TReal lowerRange( KRealZero ); 
        TReal upperRange( KRealMinusOne ); 
        
        iSource->GetRange( lowerRange,upperRange ); 
        iSource->SeqAndTS( audioSeq, audioTS, videoSeq, videoTS );

        for ( TInt j( 0 ); j < iSinks.Count(); j++ ) 
            {
            if ( !( lowerRange == KRealZero && upperRange == KRealMinusOne ) )
                {
                iSinks[j]->SetRange( lowerRange,upperRange ); 
                }

            iSinks[j]->SetSeqAndTS( audioSeq, audioTS, videoSeq, videoTS );
            }
        }
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::SinkStops()
// So, a sink wants to quit. we can't just delete it  here as return. Statement
// would then take us to deleted instance: put up a cleanup CAsyncCallBack and 
// return.
//-----------------------------------------------------------------------------
//
void CCRStreamingSession::SinkStops( const TCRSinkId& aSinkId )
    {
    LOG1( "CCRStreamingSession::SinkStops(), aSinkId: %d", aSinkId );

    // InsertInSignedKeyOrderL checks for duplicate, if there is already
    // entry for that sink, the array will remain unchanged
    TRAPD( err, iSinksToDelete.InsertInSignedKeyOrderL( aSinkId ) ); 
    if ( err )
        {
        LOG1( "CCRStreamingSession::SinkStops(), InsertInSignedKeyOrderL leaved %d", err );      
        }

    // If not already active and sinks to delete?
    if ( !iCleanUp->IsActive() && iSinksToDelete.Count() ) 
        {
        TCallBack cb( SinkStopCallBack, this );
        iCleanUp->Set( cb );
        iCleanUp->CallBack();
        }               
    }
    
//-----------------------------------------------------------------------------
// CCRStreamingSession::SourceRestore()
//-----------------------------------------------------------------------------
//
void CCRStreamingSession::SourceRestore()
    {
    if ( iSource )
        {
        iSource->Restore();
        }
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::SourceStop()
//-----------------------------------------------------------------------------
//
void CCRStreamingSession::SourceStop()
    {
    // Session is useless without a source so ask engine to clean it all up
    iEngine.SessionStop( this );
    }
    
// -----------------------------------------------------------------------------
// CCRStreamingSession::StatusChanged
//
// -----------------------------------------------------------------------------
//      
void CCRStreamingSession::StatusChanged(
    MCRPacketSource::TCRPacketSourceState aStatus )
    {
    LOG1( "CCRStreamingSession::StatusChanged(), aStatus: %d", aStatus );
    
    switch ( aStatus )
        {
        case MCRPacketSource::ERtpStateIdle:
            break;
        
        case MCRPacketSource::ERtpStateSdpAvailable:
            TRAPD( err, CreateAndSetBufferL() );
            if ( err )
                {
                LOG1( "CCRStreamingSession::StatusChanged(), CreateAndSetBuffers leaved: %d", err );
                }
            
            InitializeSinks();
            break;
        
        case MCRPacketSource::ERtpStateSeqAndTSAvailable:
            SetSeqAndTs();
            break;
        
        case MCRPacketSource::ERtpStateSetupRepply:
        case MCRPacketSource::ERtpStatePlaying:
            {                
            for ( TInt j( iSinks.Count() - 1 ); j >= 0; j-- )
                {
                iSinks[j]->StatusChanged( aStatus ) ;
                }
            }
            break;
        
        case MCRPacketSource::ERtpStateClosing:
            SourceStop();
            break;
        
        default:
            // None.
            break;
        }
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::SinkStopCallBack()
//
//-----------------------------------------------------------------------------
//
TInt CCRStreamingSession::SinkStopCallBack( TAny* aThis ) 
    {
    CCRStreamingSession* self = static_cast<CCRStreamingSession*>( aThis );    
    LOG1( "CCRStreamingSession::SinkStopCallBack(), iSinksToDelete count: %d",
        self->iSinksToDelete.Count() );
    self->DoSinkStop();
    return self->iSinksToDelete.Count(); 
    }
    
//-----------------------------------------------------------------------------
// CCRStreamingSession::DoSinkStop()
//
//-----------------------------------------------------------------------------
//
void CCRStreamingSession::DoSinkStop( void ) 
    {
    LOG( "CCRStreamingSession::DoSinkStop() in" );  

    for ( TInt i( iSinksToDelete.Count() - 1 ); i >= 0; i-- )
        {
        for ( TInt j( iSinks.Count() - 1 ); j >= 0; j-- )
            {
            if ( iSinks[j]->Id() == iSinksToDelete[i] )
                {
                TInt remainingSinks( 0 );
                if ( iBuffer ) 
                    {
                    // If we never got sdp, we never had a buffer
                	remainingSinks = iBuffer->RemoveSink( iSinks[j] );
                	}
                
                if ( remainingSinks < 1 )
                    {
                    // No sinks remaing for our buffers, I'm feeling useless
                    if ( iSource ) 
                        {
                        iSource->Stop(); 
                        }
                    }
                
                delete iSinks[j];
                iSinks[j] = NULL;
                iSinks.Remove( j );
                }
            }
        }

    iSinksToDelete.Reset(); 
    LOG( "CCRStreamingSession::DoSinkStop() out" );
    }

//-----------------------------------------------------------------------------
// CCRStreamingSession::DeleteSink
//
//-----------------------------------------------------------------------------
//
void CCRStreamingSession::DeleteSink( const TCRSinkId& aSinkId )
    {
    for ( TInt i( iSinks.Count() - 1 ); i >= 0 ; i-- ) 
        {
        if ( iSinks[i]->Id() == aSinkId )
            {
            // Remove sink from buffer
            if ( iBuffer )
                {
                iBuffer->RemoveSink( iSinks[i] );
                }
            
            // Delete sink
            delete iSinks[i];
            iSinks[i] = NULL;
            iSinks.Remove( i );
            LOG2( "CCRStreamingSession::DeleteSink(), deleted index: %d, aSinkId: %d", i, aSinkId );
            }
        }
    }

//  End of File
