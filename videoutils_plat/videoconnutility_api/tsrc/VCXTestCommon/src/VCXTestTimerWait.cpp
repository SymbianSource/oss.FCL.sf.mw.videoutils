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


//  INCLUDES
#include <e32base.h>

#include "CIptvTestActiveWait.h"
#include "CIptvTestTimer.h"
#include "VCXTestTimerWait.h"

// -----------------------------------------------------------------------------
// CTestTimerWait::NewL
// -----------------------------------------------------------------------------
//
EXPORT_C CVCXTestTimerWait* CVCXTestTimerWait::NewL()
    {
    CVCXTestTimerWait* self = new (ELeave) CVCXTestTimerWait();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CVCXTestTimerWait::CVCXTestTimerWait
// -----------------------------------------------------------------------------
//
CVCXTestTimerWait::CVCXTestTimerWait()
    {

    }

// -----------------------------------------------------------------------------
// CVCXTestTimerWait::ConstructL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestTimerWait::ConstructL()
    {
    iWait = CIptvTestActiveWait::NewL();
    iTimer = CIptvTestTimer::NewL(*this, 0);
    }

// -----------------------------------------------------------------------------
// CVCXTestTimerWait::~CVCXTestTimerWait
// -----------------------------------------------------------------------------
//
EXPORT_C CVCXTestTimerWait::~CVCXTestTimerWait()
    {
    if(iTimer != NULL)
        {
        iTimer->CancelTimer();
        delete iTimer;
        iTimer = NULL;
        }

    if( iWait )
        {
        iWait->Stop();
        delete iWait;
        iWait = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CVCXTestTimerWait::WaitL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestTimerWait::WaitL( TUint32 aMicroSeconds )
    {
    iTimer->After( aMicroSeconds );
    iWait->Start();
    }

// -----------------------------------------------------------------------------
// CVCXTestTimerWait::TimerComplete
// -----------------------------------------------------------------------------
//
void CVCXTestTimerWait::TimerComplete( TInt /* aTimerId */, TInt /* aError */ )
    {
    if( iWait ) iWait->Stop();
    }

// EOF

