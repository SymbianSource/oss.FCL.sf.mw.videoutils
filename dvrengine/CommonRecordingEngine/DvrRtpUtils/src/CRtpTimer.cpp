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
* Description:    A timer.*
*/




// INCLUDE FILES
#include "CRtpTimer.h"
#include "MRtpTimerObserver.h"

// CONSTANTS
// None.

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpTimer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpTimer* CRtpTimer::NewL( 
    MRtpTimerObserver& aObs,
    const TPriority& aPrior )
    {
    CRtpTimer* self= new( ELeave ) CRtpTimer( aObs, aPrior );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
    
// -----------------------------------------------------------------------------
// CRtpTimer::CRtpTimer
// C++ parameter constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CRtpTimer::CRtpTimer( MRtpTimerObserver& aObs, const TPriority& aPrior )
  : CTimer( aPrior ),
    iObs( aObs )
    {
    // None
    }
    
// -----------------------------------------------------------------------------
// CRtpTimer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CRtpTimer::ConstructL()
    {
    CTimer::ConstructL();
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpTimer::~CRtpTimer()
    {
    Cancel();
    }

// -----------------------------------------------------------------------------
// CRtpTimer::RunL
// To inform the notifier that time is up.
// -----------------------------------------------------------------------------
//
void CRtpTimer::RunL()
    {
    iObs.TimerEventL();
    }

// -----------------------------------------------------------------------------
// CRtpTimer::RunError
// Returns: System wide error code of indication send leave reason
// -----------------------------------------------------------------------------
//
TInt CRtpTimer::RunError( TInt aError )
    {
    iObs.TimerError( aError );
    return KErrNone;
    }

// End of File
