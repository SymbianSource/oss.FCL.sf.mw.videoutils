/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*
*/



// INCLUDE FILES
#include "CIptvTestTimer.h"
#include "MIptvTestTimerObserver.h"
#include "VCXTestLog.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CIptvTestTimer::
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestTimer* CIptvTestTimer::NewL(MIptvTestTimerObserver& aObserver, TInt aId)
    {
    VCXLOGLO1(">>>CIptvTestTimer::NewL");
    CIptvTestTimer* self = new (ELeave) CIptvTestTimer(aObserver, aId);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    VCXLOGLO1("<<<CIptvTestTimer::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CIptvTestTimer::~CIptvTestTimer
// destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestTimer::~CIptvTestTimer()
    {
    VCXLOGLO1(">>>CIptvTestTimer::~CIptvTestTimer");
    iRestartAfterCancel = EFalse;
    Cancel();
    iTimer.Close();

   	if( IsAdded() )
        {
        Deque(); // calls also Cancel()
        }

    VCXLOGLO1("<<<CIptvTestTimer::~CIptvTestTimer");
    }

// -----------------------------------------------------------------------------
// CIptvTestTimer::~CIptvTestTimer
// destructor
// -----------------------------------------------------------------------------
//
EXPORT_C void CIptvTestTimer::CancelTimer()
    {
    VCXLOGLO1(">>>CIptvTestTimer::CancelTimer");
    if( IsActive() )
    	{
    	iRestartAfterCancel = EFalse;
    	Cancel();
    	}
    VCXLOGLO1("<<<CIptvTestTimer::CancelTimer");
    }

// -----------------------------------------------------------------------------
// CIptvTestTimer::CIptvTestTimer
//
// -----------------------------------------------------------------------------
//
CIptvTestTimer::CIptvTestTimer(MIptvTestTimerObserver& aObserver, TInt aId) : CActive(EPriorityStandard), iObserver(aObserver),
	iId(aId)
    {
    VCXLOGLO1(">>>CIptvTestTimer::CIptvTestTimer");
    CActiveScheduler::Add(this);
    VCXLOGLO1("<<<CIptvTestTimer::CIptvTestTimer");
    }

// -----------------------------------------------------------------------------
// CIptvTestTimer::ConstructL
//
// -----------------------------------------------------------------------------
//
void CIptvTestTimer::ConstructL()
    {
    VCXLOGLO1(">>>CIptvTestTimer::ConstructL");
    User::LeaveIfError(iTimer.CreateLocal());
    VCXLOGLO1("<<<CIptvTestTimer::ConstructL");
    }

// -----------------------------------------------------------------------------
// CIptvTestTimer::After
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CIptvTestTimer::After(TTimeIntervalMicroSeconds32 aInterval)
    {

    iInterval = aInterval;

    if( !IsActive() )
    	{
	    VCXLOGLO2("CIptvTestTimer::After: Setting active. (%d)", this);
    	iTimer.After( iStatus, aInterval );
	    SetActive();
	    iRestartAfterCancel = EFalse; 
    	}
	else
		{
		// Cancel and start again when cancel is done.
		VCXLOGLO2("CIptvTestTimer::After: Restarting! (%d)", this);
		iRestartAfterCancel = ETrue;
		iTimer.Cancel();
		}
    }

// -----------------------------------------------------------------------------
// CIptvTestTimer::RunL
//
// -----------------------------------------------------------------------------
//
void CIptvTestTimer::RunL()
    {
    TInt status = iStatus.Int();

    if( status == KErrNone )
        {
        iObserver.TimerComplete( iId, iStatus.Int() );
        }
    else
    if( status == KErrCancel && iRestartAfterCancel )
        {
        iRestartAfterCancel = EFalse;
        iTimer.After( iStatus, iInterval );
        SetActive();
        VCXLOGLO2("CIptvTestTimer::RunL: Restarted! (%d)", this);
        }
    }

// -----------------------------------------------------------------------------
// CIptvTestTimer::DoCancel
//
// -----------------------------------------------------------------------------
//
void CIptvTestTimer::DoCancel()
    {
    VCXLOGLO1(">>>CIptvTestTimer::DoCancel");
    iTimer.Cancel();
    VCXLOGLO1("<<<CIptvTestTimer::DoCancel");
    }

//  End of File
