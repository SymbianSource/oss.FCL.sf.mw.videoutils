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
* Description:    Offers interface to RMsgQueue in civilized manner.*
*/




// INCLUDE FILES
#include "CCRMsgQueueObserver.h"
#include <ipvideo/MCREngineObserver.h>
#include <ipvideo/CRTypeDefs.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRMsgQueueObserver::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCRMsgQueueObserver* CCRMsgQueueObserver::NewL()
    {
    CCRMsgQueueObserver* self = new( ELeave ) CCRMsgQueueObserver();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRMsgQueueObserver::CCRMsgQueueObserver()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCRMsgQueueObserver::CCRMsgQueueObserver() : CActive( EPriorityAbsoluteHigh )
    {
    // Note! CActive's priority must be absolute high for ensure that 
    // the messages are arrived in right order to VIA.  
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CCRMsgQueueObserver::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRMsgQueueObserver::ConstructL()
    {
    LOG( "CCRMsgQueueObserver::ConstructL() in" );
    
    RProcess process;
    TInt handleId( process.Handle() );
    TBuf<KCRQueueNameLength> queueName;
    queueName.Format(KCRMsgQueueName, handleId);
    queueName.Trim();
    TInt err( iQueue.CreateGlobal( queueName, 
                                   sizeof( SCRQueueEntry ) ) ); 
    LOG2( "CCRMsgQueueObserver createglobal %S = %d", &queueName, err );
    err = iQueue.OpenGlobal( queueName ) ;                            
    LOG1( "CCRMsgQueueObserver openglobal = %d", err );
    User::LeaveIfError( err );
    iQueue.NotifyDataAvailable( iStatus ); 
    LOG1( "CCRMsgQueueObserver queue message size is %d", iQueue.MessageSize() );
    SetActive(); 

    LOG( "CCRMsgQueueObserver::ConstructL() out" );
    }
    
// -----------------------------------------------------------------------------
// CCRMsgQueueObserver::~CCRMsgQueueObserver()
// Destructor.
// -----------------------------------------------------------------------------
//
CCRMsgQueueObserver::~CCRMsgQueueObserver()
    {
    LOG( "CCRMsgQueueObserver::~CCRMsgQueueObserver() in" );

    Cancel();
    iQueue.Close(); 
    iObservers.Reset();     
    iObservers.Close(); 

    LOG( "CCRMsgQueueObserver::~CCRMsgQueueObserver() out" );
    }


// -----------------------------------------------------------------------------
// CCRMsgQueueObserver::RunL()
// From CActive
// -----------------------------------------------------------------------------
//
void CCRMsgQueueObserver::RunL()
    {
    LOG1( "CCRMsgQueueObserver::RunL() in, iStatus: %d", iStatus.Int() );
    
    if ( iStatus.Int() == KErrNone ) 
        {
        TInt i( 0 ); 
        if ( iQueue.Receive( iReceivedMessage ) == KErrNone )
            {
            if ( !iReceivedMessage.iSessionId || // or our id has not been set yet
                  iReceivedMessage.iSessionId == iSessionId ) // if the id matches
                {
                switch ( iReceivedMessage.iMsg )
                    {
                    case ECRMsgQueueAuthenticationNeeded:
                        LOG( "CCRMsgQueueObserver EAuthenticationFailed" );
                        SCRObserverMsg authNotify; 
                        authNotify.iMsg = ECRAuthenticationNeeded;
                        authNotify.iErr = iReceivedMessage.iErr; 
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( authNotify ) );
                            }                       
                        
                        break;

                    case ECRMsgQueueAttachCompleted:
                        {
                        LOG( "CCRMsgQueueObserver ECRMsgQueueAttachCompleted" );
                        SCRObserverMsg iapNotify; 
                        iapNotify.iMsg = ECRAttachCompleted;
                        iapNotify.iErr = iReceivedMessage.iErr; 
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( iapNotify ) );
                            }                       
                        }
                        break;

                    case ECRMsgQueueConnectionError:
                        {
                        LOG1( "CCRMsgQueueObserver ECRMsgQueueConnectionError, iReceivedMessage.iErr: %d",
                                                                               iReceivedMessage.iErr );
                        SCRObserverMsg iapNotify; 
                        iapNotify.iMsg = ECRConnectionError;
                        iapNotify.iErr = iReceivedMessage.iErr; 
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( iapNotify ) );
                            }                       
                        }
                        break;

                    case ECRMsgQueueNotEnoughBandwidth:
                        {
                        LOG( "CCRMsgQueueObserver ECRMsgQueueNotEnoughBandwidth" );
                        SCRObserverMsg iapNotify; 
                        iapNotify.iMsg = ECRNotEnoughBandwidth;
                        iapNotify.iErr = iReceivedMessage.iErr; 
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( iapNotify ) );
                            }                       
                        }
                        break;

                    case ECRMsgQueueSwitchingToTcp:
                        {
                        LOG( "CCRMsgQueueObserver ECRMsgQueueSwitchingToTcp" );
                        SCRObserverMsg iapNotify; 
                        iapNotify.iMsg = ECRSwitchingToTcp;
                        iapNotify.iErr = iReceivedMessage.iErr; 
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( iapNotify ) );
                            }                       
                        }
                        break;

                    case ECRMsgQueueNormalEndOfStream:
                        {
                        LOG( "CCRMsgQueueObserver ECRMsgQueueNormalEndOfStream" );
                        SCRObserverMsg iapNotify; 
                        iapNotify.iMsg = ECRNormalEndOfStream;
                        iapNotify.iErr = iReceivedMessage.iErr; 
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( iapNotify ) );
                            }                       
                        }
                        break;
                                            
                    case ECRMsgQueueStreamIsLiveStream:
                        {
                        LOG( "CCRMsgQueueObserver ECRMsgStreamIsLiveStream" );
                        SCRObserverMsg liveStreamNotify; 
                        liveStreamNotify.iMsg = ECRStreamIsLiveStream;
                        liveStreamNotify.iErr = iReceivedMessage.iErr; 
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( liveStreamNotify ) );
                            }                       
                        }
                        break;

                    case ECRMsgQueueStreamIsRealMedia:
                        {
                        LOG( "CCRMsgQueueObserver ECRMsgQueueStreamIsRealMedia" );
                        SCRObserverMsg realStreamNotify; 
                        realStreamNotify.iMsg = ECRRealNetworksStream;
                        realStreamNotify.iErr = iReceivedMessage.iErr; 
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( realStreamNotify ) );
                            }                       
                        }
                        break;

                    case ECRMsgQueueTestSinkData:
                        {
                        LOG( "CCRMsgQueueObserver ECRMsgQueueTestSinkData" );
                        SCRObserverMsg testSinkDataNotify;
                        testSinkDataNotify.iMsg = ECRTestSinkData;
                        testSinkDataNotify.iErr = iReceivedMessage.iErr;
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( testSinkDataNotify ) );
                            }
                        }
                        break;

                    case ECRMsgQueueSdpAvailable:
                        {
                        LOG( "CCRMsgQueueObserver ECRMsgQueueSdpAvailable" );
                        SCRObserverMsg sdpAvailableNotify;
                        sdpAvailableNotify.iMsg = ECRSdpAvailable;
                        sdpAvailableNotify.iErr = iReceivedMessage.iErr;
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( sdpAvailableNotify ) );
                            }
                        }
                        break;
                        
                    case ECRMsgQueueReadyToSeek:
                        {
                        LOG( "CCRMsgQueueObserver ECRMsgQueueReadyToSeek" );
                        SCRObserverMsg readyToSeekNotify;
                        readyToSeekNotify.iMsg = ECRReadyToSeek;
                        readyToSeekNotify.iErr = iReceivedMessage.iErr;
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( readyToSeekNotify ) );
                            }
                        }
                        break;
                        
                    case ECRMsgQueueRecordingStarted:
                        {
                        LOG( "CCRMsgQueueObserver ECRMsgQueueRecordingStarted" );
                        SCRObserverMsg realStreamNotify;
                        realStreamNotify.iMsg = ECRRecordingStarted;
                        realStreamNotify.iErr = iReceivedMessage.iErr;
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( realStreamNotify ) );
                            }
                        }
                        break;

                    case ECRMsgQueueRecordingPaused:
                        {
                        LOG( "CCRMsgQueueObserver ECRMsgQueueRecordingState" );
                        SCRObserverMsg realStreamNotify;
                        realStreamNotify.iMsg = ECRRecordingPaused;
                        realStreamNotify.iErr = iReceivedMessage.iErr;
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( realStreamNotify ) );
                            }
                        }
                        break;

                    case ECRMsgQueueRecordingEnded:
                        {
                        LOG( "CCRMsgQueueObserver ECRMsgQueueRecordingState" );
                        SCRObserverMsg realStreamNotify;
                        realStreamNotify.iMsg = ECRRecordingEnded;
                        realStreamNotify.iErr = iReceivedMessage.iErr;
                        for ( i = 0; i < iObservers.Count(); i ++ )
                            {
                            TRAP_IGNORE( iObservers[i]->NotifyL( realStreamNotify ) );
                            }
                        }
                        break;

                    default:
                        {
                        LOG1( "Unknown message: %d", iReceivedMessage.iMsg );
                        }
                        break;
                    }
                }
            else // message belongs to different session
                {
                LOG2( "Ignoring message: msg.sessionId=%d, iSessionId=%d",
                       iReceivedMessage.iSessionId, iSessionId );
                }
	    	}
        }
    
    iQueue.NotifyDataAvailable( iStatus ); 
    SetActive();    
    LOG( "CCRMsgQueueObserver::RunL() out" );
    }

// -----------------------------------------------------------------------------
// CCRMsgQueueObserver::RunError()
// From CActive
// -----------------------------------------------------------------------------
//
TInt CCRMsgQueueObserver::RunError( TInt aError )
    {
    LOG1( "CCRMsgQueueObserver::RunError(), aError: %d", aError );
    return aError;
    }

// -----------------------------------------------------------------------------
// CCRMsgQueueObserver::DoCancel()
// From CActive
// -----------------------------------------------------------------------------
//
void CCRMsgQueueObserver::DoCancel()
    {
    LOG( "CCRMsgQueueObserver::DoCancel()" );
    iQueue.CancelDataAvailable();    
    }

// -----------------------------------------------------------------------------
// CCRMsgQueueObserver::AddMsgQueueObserverL()
// Addes object to observer array
// -----------------------------------------------------------------------------
//
void CCRMsgQueueObserver::AddMsgQueueObserverL( MCREngineObserver* aObserver ) 
    {
    User::LeaveIfError( iObservers.Append( aObserver ) );
    }

// -----------------------------------------------------------------------------
// CCRMsgQueueObserver::SetSessionId()
// Sets session id to listen to
// -----------------------------------------------------------------------------
//
void CCRMsgQueueObserver::SetSessionId( TInt aSessionId )
    {
    iSessionId = aSessionId;
    }

// End of File
