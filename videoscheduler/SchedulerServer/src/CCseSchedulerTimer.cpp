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
* Description:    Timer for the notifier*
*/




// INCLUDE FILES
#include "CCseSchedulerTimer.h"         // Header file for this class
#include "MCseSchedulerTimerObserver.h" // Observer for informing engine
#include <e32const.h>
#include <e32cmn.h>
#include <e32base.h>
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
// None

// MODULE DATA STRUCTURES
// None

// LOCAL FUNCTION PROTOTYPES
// None

// FORWARD DECLARATIONS
// None



// ============================ MEMBER FUNCTIONS ===============================
// ---------------------------------------------------------------------------
// CCseSchedulerTimer::CCseSchedulerTimer()
//
// ---------------------------------------------------------------------------
CCseSchedulerTimer::CCseSchedulerTimer( MCseSchedulerTimerObserver* aObserver ) : CTimer( CActive::EPriorityStandard )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerTimer::CCseSchedulerTimer");
	
	// C++ default constructor
	iObserver = aObserver;
		
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerTimer::CCseSchedulerTimer");
	}

// ---------------------------------------------------------------------------
// CCseSchedulerTimer::~CCseSchedulerTimer()
//
// ---------------------------------------------------------------------------	
CCseSchedulerTimer::~CCseSchedulerTimer()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerTimer::~CCseSchedulerTimer");
	Cancel();
	iObserver = NULL;
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerTimer::~CCseSchedulerTimer");
	}

// ---------------------------------------------------------------------------
// CCseSchedulerTimer::NewL()
//
// ---------------------------------------------------------------------------	
CCseSchedulerTimer* CCseSchedulerTimer::NewL( 
	MCseSchedulerTimerObserver* aObserver )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerTimer::NewL");
	
	// Symbian C++ constructor
	CCseSchedulerTimer* self = new ( ELeave ) CCseSchedulerTimer(aObserver);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerTimer::NewL");
    return self;
	}
	
// ---------------------------------------------------------------------------
// CCseSchedulerTimer::ConstructL()
//
// ---------------------------------------------------------------------------
void CCseSchedulerTimer::ConstructL()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerTimer::ConstructL");	
	
	CTimer::ConstructL();
    CActiveScheduler::Add( this );
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerTimer::ConstructL");
	}
			
// ---------------------------------------------------------------------------
// CCseSchedulerTimer::SetTimer()
//
// ---------------------------------------------------------------------------
void CCseSchedulerTimer::SetTimer( const TTime& aTime )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerTimer::SetTimer");
	
	// Just cancel existing and start new one
	Cancel();	
	AtUTC( aTime );

#ifdef _DEBUG
	TBuf<100> startTimeBuf;
	_LIT( KDateTimeFormat,"CCseSchedulerTimer::SetTimer (UTC): %1%*D/%2%*M/%3%*Y %H:%T:%S.%C#" ); 
	TRAP_IGNORE( aTime.FormatL( startTimeBuf, KDateTimeFormat ) );
    CSELOGTEXT_HIGH_LEVEL( startTimeBuf );
#endif // _DEBUG

	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerTimer::SetTimer");
	}
	
// ---------------------------------------------------------------------------
// CCseSchedulerTimer::DoCancel()
//
// ---------------------------------------------------------------------------
void CCseSchedulerTimer::DoCancel()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerTimer::DoCancel");
	
	CTimer::DoCancel();
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerTimer::DoCancel");
	}

// ---------------------------------------------------------------------------
// CCseSchedulerTimer::RunL()
//
// ---------------------------------------------------------------------------		
void CCseSchedulerTimer::RunL()
	{
	CSELOGSTRING2_HIGH_LEVEL(">>>CCseSchedulerTimer::RunL, iStatus: %d", iStatus.Int());
	
	// Timer has fired. If everything went fine just kick the observer
	if ( iStatus.Int() == KErrNone )
		{		
	    iObserver->RunPluginsL();
		}
	else
		{
		// Something went propably wrong, let the observer error handler decide what.
		iObserver->TimerErrorL( iStatus.Int() );
		}
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerTimer::RunL");
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerTimer::RunError
// From CActive, called when RunL leaves.
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerTimer::RunError( 
    TInt aError )
    {
	  CSELOGSTRING2_HIGH_LEVEL(">>>CCseSchedulerServerSession::RunError: aError = %d", aError );	
    // ATM there isn't leaving code in RunL so we just cancel timer if it is active.    
    Cancel();

#if CSE_LOGGING_METHOD == 0 
    ( void )aError;
#endif

    // Return KErrNone to avoid crash.
	  return KErrNone;
    }

// End of file
