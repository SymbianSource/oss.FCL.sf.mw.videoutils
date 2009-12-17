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
* Description:    Wrapper for CTimer*
*/




// INCLUDE FILES
#include "CCRTimer.h"
#include "MCRTimerObserver.h"

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRTimer::NewL()
// -----------------------------------------------------------------------------
//
CCRTimer* CCRTimer::NewL( const TInt aPriority, MCRTimerObserver& aTimerObserver )
    {
    CCRTimer* self = CCRTimer::NewLC( aPriority, aTimerObserver );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRTimer::NewLC()
// -----------------------------------------------------------------------------
//
CCRTimer* CCRTimer::NewLC( const TInt aPriority, MCRTimerObserver& aTimerObserver )
    {
    CCRTimer* self = new (ELeave) CCRTimer( aPriority, aTimerObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CCRTimer::CCRTimer()
// -----------------------------------------------------------------------------
//
CCRTimer::CCRTimer( const TInt aPriority, MCRTimerObserver& aTimerObserver )
  : CTimer( aPriority ), iObserver( aTimerObserver )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRTimer::ConstructL()
// -----------------------------------------------------------------------------
//
void CCRTimer::ConstructL()
    {
    CTimer::ConstructL();
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CCRTimer::~CCRTimer()
// -----------------------------------------------------------------------------
//
CCRTimer::~CCRTimer()
    {
    Cancel();
    }

// -----------------------------------------------------------------------------
// CCRTimer::RunL()
// Timer request has completed, so notify the timer's owner.
// -----------------------------------------------------------------------------
//
void CCRTimer::RunL()
    {
    if ( iStatus == KErrNone )
        {
        iObserver.TimerExpired( this );
        }
    }

//  End of File
