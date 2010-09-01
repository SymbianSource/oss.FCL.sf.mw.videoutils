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
* Description:    Implementation of CR servers server class*
*/




// INCLUDE FILES
#include "CCRServer.h"
#include "CCRSession.h"
#include "CCREngine.h"
#include <ipvideo/CRTypeDefs.h> // Constants exported from client library
#include <e32svr.h>
#include <e32math.h>
#include <e32uid.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
// Platform security. Custom check is applied to all IPCs.
static const int KRangeCount( 1 );

static const TInt SecurityRanges[KRangeCount] =
    {
    0, // Range is from 0 to KMaxTInt
    };

static const TUint8 SecurityRangesPolicy[KRangeCount] =
    {
    CPolicyServer::ECustomCheck
    };

static const CPolicyServer::TPolicy Policy =
    {
    CPolicyServer::EAlwaysPass,
    KRangeCount,
    SecurityRanges,
    SecurityRangesPolicy,
    NULL,
    };

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRServer::NewLC()
// 
// -----------------------------------------------------------------------------
//
CCRServer* CCRServer::NewLC(  )
    {
    CCRServer* self = new( ELeave ) CCRServer;
    CleanupStack::PushL( self );  
    self->ConstructL( );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRServer::CCRServer()
// C++ constructor 
// -----------------------------------------------------------------------------
//
CCRServer::CCRServer()
  : CPolicyServer( EPriorityRealTimeServer, Policy, ESharableSessions ),
    iSessionCount( 0 )
    {
    __DECLARE_NAME( _S( "CCRServer" ) );
    }

// -----------------------------------------------------------------------------
// CCRServer::ConstructL()
// second-phase constructor; create the object container index.
// -----------------------------------------------------------------------------
//
void CCRServer::ConstructL(  )
    {
    LOG( "CCRServer::ConstructL()" );    
    
    iContainerIx = CObjectConIx::NewL();
    iObjectCon = iContainerIx->CreateL();
    
    StartL( KCRServerNameExe );
    }

// -----------------------------------------------------------------------------
// CCRServer::~CCRServer()
// Destructor.
// -----------------------------------------------------------------------------
//
CCRServer::~CCRServer()
    {
    LOG( "CCRServer::~CCRServer()" );

    DeleteRtpEngine();
    }

// -----------------------------------------------------------------------------
// CCRServer::ThreadFunction()
// The active scheduler is installed and started here.
// -----------------------------------------------------------------------------
//
TInt CCRServer::ThreadFunction(  )
    {
    CTrapCleanup* cleanupStack = CTrapCleanup::New();    
    if ( !cleanupStack )
        {
        PanicServer( KErrNoMemory );
        }

    TRAPD( err, ThreadFunctionL( ) );
    if ( err )
        {
        PanicServer( KErrServerTerminated );
        }

    delete cleanupStack;
    cleanupStack = NULL;
    
    return KErrNone;    
    }

// -----------------------------------------------------------------------------
// CCRServer::ThreadFunctionL()
//
// -----------------------------------------------------------------------------
//
void CCRServer::ThreadFunctionL(  )
    {
    LOG( "CCRServer::ThreadFunctionL() - In" );

    // Construct active scheduler
    CActiveScheduler* activeScheduler = new ( ELeave ) CActiveScheduler;
    CleanupStack::PushL( activeScheduler ); // |-> 1

    // Install active scheduler. 
    // We don't need to check whether an active scheduler is already installed
    // as this is a new thread, so there won't be one
    CActiveScheduler::Install( activeScheduler );

    // Construct our server, pushed cleanup stack and leaved there
    CCRServer* server = CCRServer::NewLC( );  // |-> 2    
    
    // Signal server is up
    RProcess::Rendezvous( KErrNone );

    // Start handling requests
    CActiveScheduler::Start();

    CleanupStack::PopAndDestroy( server ); // 2<-|
    CleanupStack::PopAndDestroy( activeScheduler ); // 1<-|
    
    LOG( "CCRServer::ThreadFunctionL() - Out" );    
    }

// -----------------------------------------------------------------------------
// CCRServer::SignalClientL
// Signal the client that server is running.
// -----------------------------------------------------------------------------
void CCRServer::SignalClientL()
    {
    RSemaphore semaphore;
    User::LeaveIfError( semaphore.OpenGlobal( KCRServerSemaphoreName ) );
    semaphore.Signal();
    semaphore.Close();
    }

// -----------------------------------------------------------------------------
// CCRServer::PanicServer
// Utility - panic the server
// -----------------------------------------------------------------------------
void CCRServer::PanicServer( TInt aPanic )
    {
    LOG1( "CCRServer::PanicServer(), aPanic: %d", aPanic );
    
    User::Panic( KCRServerNameExe, aPanic );
    }

// -----------------------------------------------------------------------------
// CCRServer::GetEngineObjectL()
//
// -----------------------------------------------------------------------------
//
CCREngine* CCRServer::GetEngineObjectL()
    {
    if ( iObjectCon->Count() == 0 )
        {
        // Create CR engine
        iEngine = CCREngine::NewL( );

        // Add our engine to container
        iObjectCon->AddL( iEngine );
        }
    else
        {
        // default implementation return KErrNone.
        if ( KErrNone != iEngine->Open() )
            {
            User::Leave( KErrGeneral );
            }
        }

    // We have only one object in our container
    return iEngine;
    }

// -----------------------------------------------------------------------------
// CCRServer::Inc()
// 
// -----------------------------------------------------------------------------
//
void CCRServer::Inc()
    {
    iSessionCount++;
    LOG1( "CCRServer::Inc(), New iSessionCount: %d", iSessionCount );
    }

// -----------------------------------------------------------------------------
// CCRServer::Dec()
// 
// -----------------------------------------------------------------------------
//
void CCRServer::Dec()
    {
    iSessionCount--;
    LOG1( "CCRServer::Dec(), New iSessionCount: %d", iSessionCount );

    if ( iSessionCount <= 0 )
        {
        StopServer();
        }
    }

// -----------------------------------------------------------------------------
// CCRServer::NewSessionL()
// 
// -----------------------------------------------------------------------------
//
CSession2* CCRServer::NewSessionL(
    const TVersion& aVersion,
    const RMessage2& /*aMessage*/ ) const
    {
    // Check version is ok
    TVersion v( KCRServMajorVersionNumber,
                KCRServMinorVersionNumber,
                KCRServBuildVersionNumber );
    
    if ( !User::QueryVersionSupported( v, aVersion ) )
        {
        User::Leave( KErrNotSupported );
        }
        
    // Make new session
    return CCRSession::NewL( ( CCRServer* ) this );
    }

// -----------------------------------------------------------------------------
// CCRServer::StopServer
// Stops the server thread if no sessions active.
// -----------------------------------------------------------------------------
//
void CCRServer::StopServer()
    {
    LOG( "CCRServer::StopServer()" );    
    
    CActiveScheduler::Stop();
    }
    
// -----------------------------------------------------------------------------
// CCRServer::DeleteRtpEngine
// Stops active scheduler and deletes object container and other objects.
// -----------------------------------------------------------------------------
//
void CCRServer::DeleteRtpEngine()
    {
    LOG1( "CCRServer::DeleteRtpEngine(), iContainerIx: %d", iContainerIx );
    
    if ( iContainerIx )
        {
        iContainerIx->Remove( iObjectCon );
        delete iContainerIx; iContainerIx = NULL;
        }
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

// -----------------------------------------------------------------------------
// StartThread()
// Start the server thread. This is called from the client.
// -----------------------------------------------------------------------------
//
TInt CCRServer::StartThread()
    {
#ifdef _DEBUG
    __UHEAP_MARK;
#endif
    
    LOG( "CCRServer::StartThread() - In" );
    // Check server not already started
    TFindServer findCountServer( KCRServerNameExe );
    TFullName name;
    if ( findCountServer.Next( name ) == KErrNone )
        { 
        // Found server already
        TRAP_IGNORE( CCRServer::SignalClientL() );
        return KErrAlreadyExists;
        }
    
    ThreadFunction( );
    
    LOG( "CCRServer::StartThread() - Out" );

#ifdef _DEBUG
    __UHEAP_MARKEND;
#endif
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// E32Main()
// Server startup
// Returns: KErrNone
// -----------------------------------------------------------------------------
//
GLDEF_C TInt E32Main()
    {
    return CCRServer::StartThread();
    }
    
// --------------------------------------------------------------------------
// CCRServer::CustomSecurityCheckL()
// --------------------------------------------------------------------------
//
CPolicyServer::TCustomResult CCRServer::CustomSecurityCheckL(
    const RMessage2& aMsg,
    TInt& /*aAction*/,
    TSecurityInfo& /*aMissing*/ )
    {
    
    TCustomResult retVal ( EFail );
    
    // Check the messagge function range
    if ( ( aMsg.Function() > ECRServBase && 
           aMsg.Function() < ECRServLastEnum ) ||
         ( aMsg.Function() > ECRLiveTvBase && 
           aMsg.Function() < ECRLiveTvLastEnum ) )
        {
        // Check if the client has required capabilities                
        // From .mmp-file following are CAP_APPLICATION capabilities
        if( ! aMsg.HasCapability(ECapabilityNetworkServices ) ||
            ! aMsg.HasCapability(ECapabilityLocalServices ) ||
            ! aMsg.HasCapability(ECapabilityLocation ) ||
            ! aMsg.HasCapability(ECapabilityReadUserData ) ||
            ! aMsg.HasCapability(ECapabilityWriteUserData ) ||
            ! aMsg.HasCapability(ECapabilityReadDeviceData ) ||
            ! aMsg.HasCapability(ECapabilityWriteDeviceData ) ||
            ! aMsg.HasCapability(ECapabilitySwEvent ) )
            {
            LOG1(
                "CCRServer::CustomSecurityCheckL() No capability for message %d!!!",
                aMsg.Function() );                 
            }
        else
            {
            LOG1(
                "CCRServer::CustomSecurityCheckL() Message %d inside range and capabilities ok",
                aMsg.Function() );
            retVal = EPass;
            }        
        }
    else
        {
        LOG1(
            "CCRServer::CustomSecurityCheckL() Message %d outside known range!!!",
            aMsg.Function() );            
        }
       
    return retVal;

    }
// End of File

