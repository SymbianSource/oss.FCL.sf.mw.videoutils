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
* Description:    Observer for plugin thread if shutdown is signalled.*
*/



// INCLUDE FILES
#include "CCseSemaphoreController.h"
#include "MCseSemaphoreSignalObserver.h"
#include "CseDebug.h"               // Debug macros



// EXTERNAL DATA STRUCTURES
// None

// EXTERNAL FUNCTION PROTOTYPES  
// None

// CONSTANTS
// None

// MACROS
// None

// LOCAL CONSTANTS AND MACROS
static TUint KCSeSemaphorePollingInterval = 5000;

// MODULE DATA STRUCTURES
// None

// LOCAL FUNCTION PROTOTYPES
// None

// FORWARD DECLARATIONS
// None


// ============================ MEMBER FUNCTIONS ===============================
// ---------------------------------------------------------------------------
// CCseSchedulerTimer::CCseSemaphoreController()
//
// ---------------------------------------------------------------------------
CCseSemaphoreController::CCseSemaphoreController( RSemaphore& aSemaphore,
                                                  MCseSemaphoreSignalObserver& aObserver )
                            : CTimer( EPriorityNormal ),
                              iSemaphore( aSemaphore ),
                              iObserver( aObserver ),
                              iInterval( KCSeSemaphorePollingInterval )
                              
                            
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSemaphoreController::CCseSemaphoreController");
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSemaphoreController::CCseSemaphoreController");
	}

// ---------------------------------------------------------------------------
// CCseSchedulerTimer::~CCseSemaphoreController()
//
// ---------------------------------------------------------------------------	
CCseSemaphoreController::~CCseSemaphoreController()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSemaphoreController::~CCseSemaphoreController");
	
	Cancel();
		
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSemaphoreController::~CCseSemaphoreController");
	}

// ---------------------------------------------------------------------------
// CCseSchedulerTimer::NewL()
//
// ---------------------------------------------------------------------------	
CCseSemaphoreController* CCseSemaphoreController::NewL( RSemaphore& aSemaphore,
                                                        MCseSemaphoreSignalObserver& aObserver )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSemaphoreController::NewL");
	
	// Symbian C++ constructor
	CCseSemaphoreController* self = new ( ELeave ) CCseSemaphoreController( aSemaphore,
	                                                                        aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSemaphoreController::NewL");
    return self;
	}
	
// ---------------------------------------------------------------------------
// CCseSemaphoreController::ConstructL()
//
// ---------------------------------------------------------------------------
void CCseSemaphoreController::ConstructL()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSemaphoreController::ConstructL");	
	    
    CTimer::ConstructL();
    CActiveScheduler::Add( this );
       
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSemaphoreController::ConstructL");
	}

// -----------------------------------------------------------------------------
// CCseSemaphoreController::Start
//
// -----------------------------------------------------------------------------
//
void CCseSemaphoreController::Start( )
    {                    
    Cancel();

    After( TTimeIntervalMicroSeconds32( iInterval ) );    
    }
    
// -----------------------------------------------------------------------------
// CCseSemaphoreController::Stop
//
// -----------------------------------------------------------------------------
//
void CCseSemaphoreController::Stop( )
    {
    Cancel();
    }    

// -----------------------------------------------------------------------------
// CCseSemaphoreController::SetInterval
//
// -----------------------------------------------------------------------------
//
void CCseSemaphoreController::SetInterval( TUint aInterval )
    {    
    iInterval = aInterval;
    }

// -----------------------------------------------------------------------------
// CCseSemaphoreController::RunL
//
// -----------------------------------------------------------------------------
//
void CCseSemaphoreController::RunL( )
    {
     // Check if shutdown is signalled
	if( KErrNone == iSemaphore.Wait( 1 ) )
	    {    	        
	    CSELOGSTRING_HIGH_LEVEL("CCseSemaphoreController::RunL - Semaphore signalled, inform observer");
	    // Shutdown signalled, start taking down the plugin	    
	    iObserver.SemaphoreSignalled();
	    Cancel();
	    }    
    else
        {
        // If not, start again.
        Start();
        }
    }



// End of file
