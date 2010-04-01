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
* Description:    Plugin starter, this is run in separate thread from engine*
*/





// INCLUDE FILES
#include "CCseSchedulerPluginStarter.h"
#include "CseDebug.h"                           // Debug macros
#include <ipvideo/ccsescheduledprogram.h>   // Represent one schedule in database
#include <ipvideo/ccseschedulerpluginif.h>
#include "CCseSchedulerThreadPacket.h"
#include "CCseSemaphoreController.h"

// EXTERNAL DATA STRUCTURES
// None.

// EXTERNAL FUNCTION PROTOTYPES  
// None.

// CONSTANTS
// None

// MACROS
// None.

// LOCAL CONSTANTS AND MACROS
// None

// MODULE DATA STRUCTURES
// None.

// LOCAL FUNCTION PROTOTYPES
// None.


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCseSchedulerPluginStarter::CCseSchedulerPluginStarter
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCseSchedulerPluginStarter::CCseSchedulerPluginStarter ( CCseSchedulerThreadPacket* aThreadPacket ) :                                
                                iThreadPacket( aThreadPacket ),
                                iPluginRdyToTakedown( EFalse )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginStarter::CCseSchedulerPluginStarter");
        
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginStarter::CCseSchedulerPluginStarter");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerPluginStarter::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCseSchedulerPluginStarter* CCseSchedulerPluginStarter::NewL( CCseSchedulerThreadPacket* aThreadPacket )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginStarter::NewL");
    
    CCseSchedulerPluginStarter* self = new( ELeave ) CCseSchedulerPluginStarter( aThreadPacket );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginStarter::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerPluginStarter::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginStarter::ConstructL()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginStarter::ConstructL");
    
    iCompletitionBreaker = CIdle::NewL( CActive::EPriorityLow );
    // Start completition breaker. This time we do not inform about completition of
    // plugin but we break call chain and start running plugin from RunL
    iCompletitionBreaker->Start(TCallBack( PluginCompleteCallback, this ));
    
    iSemaphoreMonitor = CCseSemaphoreController::NewL( iThreadPacket->ShutdownSemaphore(),
                                                       *this );
           
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginStarter::ConstructL");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerEngineBackdoorClient::~CCseSchedulerPluginStarter
// Destructor.
// -----------------------------------------------------------------------------
//
CCseSchedulerPluginStarter::~CCseSchedulerPluginStarter()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginStarter::~CCseSchedulerPluginStarter");

    delete iCompletitionBreaker;    
    delete iPlugin;
    iPlugin = NULL;
    REComSession::FinalClose();
        
    delete iSemaphoreMonitor;    
    iSemaphoreMonitor = NULL;
        
    // We DO NOT destroy thread packet, we just null the pointer.
    // Information is still needed on other side after thread dies
    iThreadPacket = NULL;
    		
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginStarter::~CCseSchedulerPluginStarter");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerPluginStarter::StartThread
//
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerPluginStarter::StartThread( TAny* aParam )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginStarter::RunPlugin");
    
    TInt err( KErrNone );
    
    //create cleanupstack
    CTrapCleanup* cleanup = CTrapCleanup::New(); // get clean-up stack
    if (!cleanup)
        {
        err = KErrNoMemory;
        }
    if (!err)
        {
        TRAP(err, DoStartThreadL( aParam ));
        }
    delete cleanup;

    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginStarter::RunPlugin");
    return err;	
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerPluginStarter::DoStartThreadL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginStarter::DoStartThreadL( TAny* aParam )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginStarter::DoStartThreadL");
    
    CCseSchedulerThreadPacket* threadPacket = static_cast<CCseSchedulerThreadPacket*>(aParam);
    
    CActiveScheduler* scheduler = new ( ELeave ) CActiveScheduler;
    
    // If schedule doesn't exist there is really nothing we can do
    if( scheduler )
        {            
        CleanupStack::PushL( scheduler );          //   |->1
        CActiveScheduler::Install( scheduler );

        // Create the starter
        CCseSchedulerPluginStarter* starter = CCseSchedulerPluginStarter::NewL( threadPacket );
        CleanupStack::PushL(starter);              //   |->2
            
        // Ready to run
        CActiveScheduler::Start();
        
        CSELOGSTRING_HIGH_LEVEL("CCseSchedulerPluginStarter::DoStartThreadL - Out from CActiveScheduler::Start!");
        
        // Cleanup the server and scheduler
        CleanupStack::PopAndDestroy( starter );   // 2<-|
        CleanupStack::PopAndDestroy( scheduler ); // 1<-|
        
        threadPacket->ShutdownCompleteSemaphore().Signal();  
        }
    else
        {
        CSELOGSTRING_HIGH_LEVEL("CCseSchedulerPluginStarter::DoStartThreadL - No schedule available, GTFO");
        }
        
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginStarter::DoStartThreadL");
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerPluginStarter::RunPluginL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginStarter::RunPluginL( )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginStarter::RunPluginL");
    
    // Create plugin
    TUid pluginUid = TUid::Uid( iThreadPacket->Schedule()->PluginUid() );
    
    iPlugin = CCseSchedulerPluginIF::NewL( pluginUid );

    iPlugin->RunTaskL( *iThreadPacket->Schedule(), this );

    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginStarter::RunPluginL");
	}

// -----------------------------------------------------------------------------
// CCseSchedulerPluginStarter::PluginCompleted
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginStarter::PluginCompleted( TInt aCompletitionCode )
	{	
	CSELOGSTRING2_HIGH_LEVEL(
	    ">>>CCseSchedulerPluginStarter::PluginCompleted - Code: %d", aCompletitionCode);
	
    // Start taking down The Thread    
    // Start Completition breaker if not already active to break call chain.
    // Set plugin completition value
    iThreadPacket->SetResultCode( aCompletitionCode );

    iPluginRdyToTakedown = ETrue;
    
    if( !iCompletitionBreaker->IsActive() )
        {                    
        iCompletitionBreaker->Start(TCallBack( PluginCompleteCallback, this ));
        }
    else
        {
        CSELOGSTRING_HIGH_LEVEL(
            "CCseSchedulerPluginStarter::PluginCompleted:Breaker already running!");
        }
        
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginStarter::PluginCompleted");
	}	
	
// -----------------------------------------------------------------------------
// CCseSchedulerPluginStarter::PluginCompleteCallback
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerPluginStarter::PluginCompleteCallback( TAny* aPtr )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>PluginCompleteCallback::ScheduleCompleteCallback");
	CSELOGSTRING_HIGH_LEVEL("<<<PluginCompleteCallback::ScheduleCompleteCallback");
	return static_cast<CCseSchedulerPluginStarter*>( aPtr )->HandlePluginCompleteCallback();
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerUserInformer::HandlePluginCompleteCallback
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerPluginStarter::HandlePluginCompleteCallback() 
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginStarter::HandleScheduleCompleteCallback ");
	
	if( iPluginRdyToTakedown )
	    {
	    // First clear flag
	    iPluginRdyToTakedown = EFalse;    	
    	// Takedown thread
    	CActiveScheduler::Stop();
	    }
    else
	    {
	    // First start monitor
	    iSemaphoreMonitor->Start();
	    // Plugin ready to be run	    
	    TRAPD( err, RunPluginL() );
	    if ( err != KErrNone )
	        {
	        // Run plugin leaved for some reason. Set error code and get outta here!
	        iThreadPacket->SetResultCode( err );
            iPluginRdyToTakedown = ETrue;
            iCompletitionBreaker->Start(TCallBack( PluginCompleteCallback, this ));	        
	        }
	    }

	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginStarter::HandleScheduleCompleteCallback ");
	return EFalse;
	}

// -----------------------------------------------------------------------------
// CCseSchedulerUserInformer::SemaphoreSignalled
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginStarter::SemaphoreSignalled()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginStarter::SemaphoreSignalled");

    delete iPlugin;
    iPlugin = NULL;

    CActiveScheduler::Stop();
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginStarter::SemaphoreSignalled");
    }

// End of file.
