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
#include "VCXTestMessageWait.h"
#include "VCXTestLog.h"
#include "CIptvTestActiveWait.h"
#include "CIptvTestTimer.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS
const TInt KTimeoutSecond = 1000000;

// MACROS

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CVCXTestMessageWait::
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C CVCXTestMessageWait* CVCXTestMessageWait::NewL( MVCXTestMessageWaitObserver* aObserver )
    {
    VCXLOGLO1(">>>CVCXTestMessageWait::NewL");
    CVCXTestMessageWait* self = new (ELeave) CVCXTestMessageWait( aObserver );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    VCXLOGLO1("<<<CVCXTestMessageWait::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CVCXTestMessageWait::~CVCXTestMessageWait
// destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CVCXTestMessageWait::~CVCXTestMessageWait()
    {
    VCXLOGLO1(">>>CVCXTestMessageWait::~CVCXTestMessageWait");
    
    iActiveWaitBlocking->Stop();
    delete iActiveWaitBlocking;
    iActiveWaitBlocking = NULL;

    if( iTimeoutTimer )
        {
        iTimeoutTimer->CancelTimer();
        delete iTimeoutTimer;
        iTimeoutTimer = NULL;
        }

    iReceivedMessages.Reset();
    iWaitedMessages.Reset();
    
    VCXLOGLO1("<<<CVCXTestMessageWait::~CVCXTestMessageWait");
    }

// -----------------------------------------------------------------------------
// CVCXTestMessageWait::~CVCXTestMessageWait
// destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CVCXTestMessageWait::CVCXTestMessageWait( MVCXTestMessageWaitObserver* aObserver )
 : iObserver( aObserver )
    {
    VCXLOGLO1(">>>CVCXTestMessageWait::CVCXTestMessageWait");

    VCXLOGLO1("<<<CVCXTestMessageWait::CVCXTestMessageWait");
    }

// -----------------------------------------------------------------------------
// CVCXTestMessageWait::ConstructL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestMessageWait::ConstructL()
    {
    VCXLOGLO1(">>>CVCXTestMessageWait::ConstructL");
    
    iActiveWaitBlocking = CIptvTestActiveWait::NewL();
    iTimeoutTimer = CIptvTestTimer::NewL(*this, 0);

    VCXLOGLO1("<<<CVCXTestMessageWait::ConstructL");
    }

// -----------------------------------------------------------------------------
// CVCXTestMessageWait::Reset
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestMessageWait::Reset()
    {
    VCXLOGLO1(">>>CVCXTestMessageWait::Reset");
    iCoolingDown = EFalse;
    iWaitStarted = EFalse;
    iWaitedMessages.Reset();
    iReceivedMessages.Reset();
    iTimeoutTimer->Cancel();
    iActiveWaitBlocking->Stop();
    VCXLOGLO1("<<<CVCXTestMessageWait::Reset");
    }

// -----------------------------------------------------------------------------
// CVCXTestMessageWait::ResetReceivedMessages
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestMessageWait::ResetReceivedMessages()
	{
    VCXLOGLO1(">>>CVCXTestMessageWait::ResetReceivedMessages");
    iReceivedMessages.Reset();    
    VCXLOGLO1("<<<CVCXTestMessageWait::ResetReceivedMessages");
	}

// -----------------------------------------------------------------------------
// CVCXTestMessageWait::AddMessage
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestMessageWait::AddMessage( TInt32 aMsg )
    {
    VCXLOGLO1(">>>CVCXTestMessageWait::AddMessage");
    iWaitedMessages.Append( aMsg );    
    VCXLOGLO1("<<<CVCXTestMessageWait::AddMessage");
    }

// -----------------------------------------------------------------------------
// CVCXTestMessageWait::WaitForMessageL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestMessageWait::WaitForMessageL( TInt32 aMsg, TInt aTimeoutSeconds, TBool aDoBlock )
	{
    VCXLOGLO1(">>>CVCXTestMessageWait::WaitForMessageL");

    iWaitedMessages.Reset();
    iWaitedMessages.Append( aMsg );
    
    iTimeoutSeconds = aTimeoutSeconds;
    WaitForAllL( aTimeoutSeconds, aDoBlock );
    
    VCXLOGLO1("<<<CVCXTestMessageWait::WaitForMessageL");
	}

// -----------------------------------------------------------------------------
// CVCXTestMessageWait::WaitForAllL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestMessageWait::WaitForAllL( TInt aTimeoutSeconds, TBool aDoBlock )
	{
    VCXLOGLO1(">>>CVCXTestMessageWait::WaitForAllL");

    iWaitStarted = ETrue;
    
    iTimeoutSeconds = aTimeoutSeconds;
    
    // Nothing to wait.
    if( iWaitedMessages.Count() <= 0 )
        {
        VCXLOGLO1("<<<CVCXTestMessageWait::WaitForAllL");
        User::Leave( KErrNotReady );
        }    
    
    // Check already received messages.
    for( TInt i = iReceivedMessages.Count()-1; i >= 0; i-- )
        {
        for( TInt e = iWaitedMessages.Count()-1; e >= 0; e-- )
            {
            if( iReceivedMessages[i] == iWaitedMessages[e] )
                {
                iWaitedMessages.Remove( e );
                iReceivedMessages.Remove( i );
                break;
                }
            }
        }
    
    if( iWaitedMessages.Count() > 0 )
        {
        // There's messages to wait. Start timeout timer.
        iTimeoutTimer->After( aTimeoutSeconds * KTimeoutSecond );
    
        // Client wants blocking call.
        if( aDoBlock )
            {
            iActiveWaitBlocking->Start();
            User::LeaveIfError( iError );
            }
        }
    else
        {
        iWaitStarted = EFalse;
        
        // Nothing to wait for.
        iTimeoutTimer->CancelTimer();
        
        // Not blocking, inform client with callback.
        if( !aDoBlock )
            {
            iObserver->MessageWaitComplete( KErrNone );
            }
        }
    
    VCXLOGLO1("<<<CVCXTestMessageWait::WaitForAllL");
	}

// -----------------------------------------------------------------------------
// CVCXTestMessageWait::CoolDownL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestMessageWait::CoolDownL( TInt aSeconds )
    {
    VCXLOGLO1(">>>CVCXTestMessageWait::CoolDownL");

    Reset();
    
    iTimeoutSeconds = aSeconds;
    
    iCoolingDown = ETrue;
    
    iTimeoutTimer->After( iTimeoutSeconds * KTimeoutSecond );
    
    iActiveWaitBlocking->Start();
        
    VCXLOGLO1("<<<CVCXTestMessageWait::CoolDownL");
    }

// -----------------------------------------------------------------------------
// CVCXTestMessageWait::ReceiveMessage
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestMessageWait::ReceiveMessage( TInt32 aMsg, TInt aError )
	{
    if( !iWaitStarted )
        {
        VCXLOGLO1("CVCXTestMessageWait::ReceiveMessage: Wait not active.");
        return;
        }

    VCXLOGLO1(">>>CVCXTestMessageWait::ReceiveMessage");

    if( aError != KErrNone )
        {
        iWaitStarted = EFalse;
        if( iActiveWaitBlocking->IsWaiting() )
            {
            iError = aError;
            iActiveWaitBlocking->Stop();
            }
        else
            {
            iObserver->MessageWaitComplete( aError );
            }
        iTimeoutTimer->CancelTimer();
        VCXLOGLO1("<<<CVCXTestMessageWait::ReceiveMessage");
        return;
        }
    
    if( iCoolingDown ) 
        {
        iTimeoutTimer->After( iTimeoutSeconds * KTimeoutSecond );
        VCXLOGLO1("<<<CVCXTestMessageWait::ReceiveMessage");
        return;
        }
    
    if( !iWaitStarted ) 
        {
        iReceivedMessages.Append( aMsg );
        VCXLOGLO1("<<<CVCXTestMessageWait::ReceiveMessage");
        return;
        }

    if( iWaitedMessages.Count() > 0 )
        {
        // Check if message is in wait queue.
        for( TInt i = iWaitedMessages.Count()-1; i >= 0; i-- )
            {
            if( iWaitedMessages[i] == aMsg )
                {
                iWaitedMessages.Remove( i );
                break;
                }
            }
        
        if( iWaitedMessages.Count() <= 0 )
            {
            iWaitStarted = EFalse;
            // Stop wait.
            if( iActiveWaitBlocking->IsWaiting() )
                {
                iError = KErrNone;
                iActiveWaitBlocking->Stop();
                }
            // Not waiting, client wants callback.
            else
                {
                iObserver->MessageWaitComplete( KErrNone );
                }
            // Stop timer too.
            iTimeoutTimer->CancelTimer();
            }
        }
    
    VCXLOGLO1("<<<CVCXTestMessageWait::ReceiveMessage");
	}

// -----------------------------------------------------------------------------
// CVCXTestMessageWait::TimerComplete
// -----------------------------------------------------------------------------
//
void CVCXTestMessageWait::TimerComplete( TInt /* aTimerId */, TInt aError )
    {
    if( aError != KErrCancel )
        {
        if( iCoolingDown )
            {
            VCXLOGLO1("CVCXTestMessageWait:: Cooldown done.");
            iActiveWaitBlocking->Stop();
            iCoolingDown = EFalse;
            }
        else
        if( iWaitedMessages.Count() > 0 )
            {
            iWaitStarted = EFalse;
            VCXLOGLO2("CVCXTestMessageWait:: Timeout. Messages left: %d", iWaitedMessages.Count());
            if( iActiveWaitBlocking->IsWaiting() )
                {
                iActiveWaitBlocking->Stop();
                iError = KErrTimedOut;
                }
            else
                {
                // Client wants callback.
                iObserver->MessageWaitComplete( KErrTimedOut );
                }
            }
        }
    }

//  End of File
