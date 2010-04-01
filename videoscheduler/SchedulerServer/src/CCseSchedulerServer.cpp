/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    Implementation of Scheduler server's Server class*
*/




// INCLUDE FILES
#include "CCseSchedulerServer.h"                // Header file for this class
#include "CCseSchedulerServerSession.h"         // Server session
#include "CCseSchedulerServerEngine.h"          // Server intelligence aka engine
#include <ipvideo/CseSchedulerClientServerCommon.h>     // Common defines for client and server
#include "CseSchedulerServer.pan"               // Server panic codes
#include "CseDebug.h"                           // Debug macros
#include <e32svr.h>
#include <e32math.h>
#include <e32uid.h>
#include <e32def.h>


// EXTERNAL DATA STRUCTURES
// None

// EXTERNAL FUNCTION PROTOTYPES  
// None

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


// MACROS
// None

// LOCAL CONSTANTS AND MACROS
const TInt KDbStartTimeout              = 2000000;

// MODULE DATA STRUCTURES
// None

// LOCAL FUNCTION PROTOTYPES
// None

// FORWARD DECLARATIONS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCseSchedulerServer::CCseSchedulerServer()
// C++ constructor 
// -----------------------------------------------------------------------------
//
CCseSchedulerServer::CCseSchedulerServer()
    : CPolicyServer( EPriorityRealTimeServer, Policy, ESharableSessions ),
    iContainerIx( NULL ),
    iObjectCon( NULL ),
    iSessionCount( 0 ),
    iSchedulerEngine( NULL )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::CCseSchedulerServer");
    __DECLARE_NAME( _S( "CCseSchedulerServer" ) );
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::CCseSchedulerServer");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServer::NewLC()
// 
// -----------------------------------------------------------------------------
//
CCseSchedulerServer* CCseSchedulerServer::NewLC( )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::NewLC");
    
    CCseSchedulerServer* self = new ( ELeave ) CCseSchedulerServer;
    CleanupStack::PushL( self );
    
    self->ConstructL( );
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::NewLC");
    return self;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServer::ConstructL()
// second-phase constructor; create the object container index.
// -----------------------------------------------------------------------------
//
void CCseSchedulerServer::ConstructL( )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::ConstructL");
    iContainerIx = CObjectConIx::NewL();
    iObjectCon = iContainerIx->CreateL(); 
    StartL( KCseSchedulerServerName );
    GetEngineObjectL();
    iStarterBreaker = CPeriodic::NewL( CActive::EPriorityStandard );
    
    // break call chain
	if( !iStarterBreaker->IsActive() )
        {
        // Give client 2 secs to make server session before we check 
        // if server is needed or not.
        iStarterBreaker->Start( KDbStartTimeout,
                                KDbStartTimeout,
                                TCallBack( StarterCallback, this ));
        }
    else
        {
        CSELOGSTRING_HIGH_LEVEL(
            "CCseSchedulerServer::ConstructL - Breaker already active");
        }
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::ConstructL");    
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServer::~CCseSchedulerServer()
// Destructor.
// -----------------------------------------------------------------------------
//
CCseSchedulerServer::~CCseSchedulerServer()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::~CCseSchedulerServer");
    DeleteSchedulerEngine();
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::~CCseSchedulerServer");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServer::ThreadFunction()
// The active scheduler is installed and started here.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerServer::ThreadFunction( )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::ThreadFunction");
	CTrapCleanup* cleanupStack = CTrapCleanup::New();    
    if ( !cleanupStack )
        {
        PanicServer( ECseCreateTrapCleanup );
        }

    TRAPD( err, ThreadFunctionL( ) );
    if ( err )
        {
        PanicServer( ECseSrvCreateServer );
        }

    delete cleanupStack;    
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::ThreadFunction");
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServer::ThreadFunctionL()
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServer::ThreadFunctionL( )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::ThreadFunctionL");

    // Construct active scheduler
    CActiveScheduler* activeScheduler = new ( ELeave ) CActiveScheduler;
    CleanupStack::PushL( activeScheduler ); // |-> 1

    // Install active scheduler. 
    // We don't need to check whether an active scheduler is already installed
    // as this is a new thread, so there won't be one
    CActiveScheduler::Install( activeScheduler );

    // Construct our server, pushed cleanup stack and leaved there
    CCseSchedulerServer* server = CCseSchedulerServer::NewLC( );  // |-> 2    
    
    // Signal server is up
    RProcess::Rendezvous( KErrNone );

    // Start handling requests
    CActiveScheduler::Start();

    CleanupStack::PopAndDestroy( server ); // 2<-|
    CleanupStack::PopAndDestroy( activeScheduler ); // 1<-|
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::ThreadFunctionL");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServer::PanicServer
// Utility - panic the server
// (other items were commented in a header).
// -----------------------------------------------------------------------------
void CCseSchedulerServer::PanicServer( TCseSchedulerServPanic aPanic )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::PanicServer");
    _LIT( KSchedulerServer, "SchedulerServer" );
    User::Panic( KSchedulerServer, aPanic );
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::PanicServer");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServer::GetEngineObjectL()
//
// -----------------------------------------------------------------------------
//
CCseSchedulerServerEngine* CCseSchedulerServer::GetEngineObjectL()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::GetEngineObjectL");
        
    if ( iObjectCon->Count() == 0 )
        {
        // Create scheduler engine
        iSchedulerEngine = CCseSchedulerServerEngine::NewL( *this );
       
        // Add our engine to container
        iObjectCon->AddL( iSchedulerEngine );
        }
    else
        {
        // default implementation return KErrNone.
        if ( KErrNone != iSchedulerEngine->Open() )
            {
            User::Leave( KErrGeneral );
            }
        }
    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::GetEngineObjectL");
    
    // We have only one object in our container
    return iSchedulerEngine;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServer::Inc()
// 
// -----------------------------------------------------------------------------
//
void CCseSchedulerServer::Inc()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::Inc");
    if ( iSessionCount!=KErrNotFound )
        {
        iSessionCount++;
        }
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::Inc");
    }

// -----------------------------------------------------------------------------
// CCseRTPServer::Dec()
// 
// -----------------------------------------------------------------------------
//
void CCseSchedulerServer::Dec()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::Dec");
    iSessionCount--;

    if ( iSessionCount<=0 )
        {
        StopServer();
        }
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::Dec");
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerServer::DeleteSchedulerEngine
// Stops active scheduler and deletes object container and other objects.
// -----------------------------------------------------------------------------
void CCseSchedulerServer::DeleteSchedulerEngine()
    {
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::DeleteSchedulerEngine");
    
    if ( iContainerIx )
        {
        iSchedulerEngine->Close();
        iContainerIx->Remove( iObjectCon );
        delete iContainerIx; 
		iContainerIx = NULL;
		iSchedulerEngine = NULL;
        }

    if( iStarterBreaker->IsActive() )
        {
        iStarterBreaker->Cancel();
        }
    delete iStarterBreaker;    
    iStarterBreaker = NULL;
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::DeleteSchedulerEngine");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServer::NewSessionL()
// 
// -----------------------------------------------------------------------------
//

CSession2* CCseSchedulerServer::NewSessionL( const TVersion& aVersion,
                                             const RMessage2& /*aMessage*/ ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::NewSessionL");
    
    // Check version is ok
    TVersion v( KCseServMajorVersionNumber,
                KCseServMinorVersionNumber,
                KCseServBuildVersionNumber );
    
    if ( !User::QueryVersionSupported( v, aVersion ) )
        {
        User::Leave( KErrNotSupported );
        }
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::NewSessionL");        
    
    // Make new session
	return CCseSchedulerServerSession::NewL( ( CCseSchedulerServer* ) this );
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerServer::StopServer
// Stops the server thread if no sessions active.
// -----------------------------------------------------------------------------
//
void CCseSchedulerServer::StopServer()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::StopServer");
    TBool schedulerActive( iSchedulerEngine->IsSchedulerActive() );    
    CSELOGSTRING2_HIGH_LEVEL("CCseSchedulerServer::StopServer - Session count: %d", iSessionCount);    
    if ( (iSessionCount<=0) && (schedulerActive == EFalse))
        {
        CSELOGSTRING_HIGH_LEVEL("CCseSchedulerServer::StopServer - Call ActiveScheduler::Stop()");
        CActiveScheduler::Stop();
        }        

    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::StopServer");
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================
// -----------------------------------------------------------------------------
// StartThread()
// Start the server thread. This is called from the client.
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerServer::StartThread()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::StartThread");
#if _DEBUG
    __UHEAP_MARK;
#endif
       
    // Check server not already started
    TFindServer findCountServer( KCseSchedulerServerName );
    TFullName name;
    if ( findCountServer.Next( name ) == KErrNone )
        { 
        // Found server already
        RProcess::Rendezvous( KErrNone );
        CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::StartThread - Return value: KErrAlreadyExists");
        return KErrAlreadyExists;
        }

    ThreadFunction( );

#if _DEBUG
    __UHEAP_MARKEND;
#endif    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::StartThread - Server down!");
	
    // All well
    return KErrNone;
    }

// --------------------------------------------------------------------------
// CCseSchedulerServer::CustomSecurityCheckL()
// --------------------------------------------------------------------------
//
CPolicyServer::TCustomResult CCseSchedulerServer::CustomSecurityCheckL(
    const RMessage2& aMsg,
    TInt& /*aAction*/,
    TSecurityInfo& /*aMissing*/ )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::CustomSecurityCheckL");
    TCustomResult retVal ( EFail );
    
    // Check the messagge function range
    if ( aMsg.Function() > ECseRequestBase && 
         aMsg.Function() < ECseRequestLast )
        {
        
        // Check if the client has required capabilities
        // From .mmp-file capablity NetworkControl
        if( // From .mmp-file following are CAP_SERVER capabilities
            ! aMsg.HasCapability(ECapabilityNetworkServices ) ||
            ! aMsg.HasCapability(ECapabilityLocalServices ) ||
            ! aMsg.HasCapability(ECapabilityLocation ) ||
            ! aMsg.HasCapability(ECapabilityReadUserData ) ||
            ! aMsg.HasCapability(ECapabilityWriteUserData ) ||
            ! aMsg.HasCapability(ECapabilityReadDeviceData ) ||
            ! aMsg.HasCapability(ECapabilityWriteDeviceData ) ||        
            ! aMsg.HasCapability(ECapabilityUserEnvironment ) ||
            ! aMsg.HasCapability(ECapabilitySwEvent ) )
            {         
            CSELOGSTRING2_HIGH_LEVEL(
                "CCseSchedulerServer::CustomSecurityCheckL() No capability for message %d!!!",
                aMsg.Function() );                 
            }
        else
            {
            CSELOGSTRING2_HIGH_LEVEL(
                "CCseSchedulerServer::CustomSecurityCheckL() Message %d inside range and capabilities ok",
                aMsg.Function() );
            retVal = EPass;
            }
        }
    else
        {
        CSELOGSTRING2_HIGH_LEVEL(
            "CCseSchedulerServer::CustomSecurityCheckL() Message %d outside known range!!!",
            aMsg.Function() );            
        }
    
    if( retVal == EPass )
        {
        CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::CustomSecurityCheckL - Passed");
        }
    else
        {
        CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::CustomSecurityCheckL - Failed");
        }    
    return retVal;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::StarterCallback
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerServer::StarterCallback( TAny* aPtr )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::StarterCallback");
	TBool retVal( EFalse);
	retVal = static_cast<CCseSchedulerServer*>( aPtr )->HandleStarterCallback();
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::StarterCallback");
	return retVal;
	}

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::HandleCompletitionCallback
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerServer::HandleStarterCallback() 
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServer::HandleStarterCallback ");
	
	// We dont want to use periodic anymore, so we cancel it. We just wanted to
	// make call here 2 secs after server has been created.
	iStarterBreaker->Cancel();  
	
	// Check if server can be stopped.
    iSchedulerEngine->RequestReschedule();
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServer::HandleStarterCallback ");
	return EFalse;
	}

// -----------------------------------------------------------------------------
// E32Main()
// Server startup
// Returns: KErrNone
// -----------------------------------------------------------------------------
//
GLDEF_C TInt E32Main()
    {
    return CCseSchedulerServer::StartThread();
    }

// End of File
