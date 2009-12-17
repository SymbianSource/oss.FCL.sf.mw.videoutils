/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   ?Description*
*/


#include <S32STRM.H>
#include <S32MEM.H>

#include "VCXTestLog.h"

#include "VCXConnUtilTester.h"
#include "vcxconnectionutility.h"
#include "CIptvTestTimer.h"
#include "CIptvTestActiveWait.h"

// -----------------------------------------------------------------------------
// CVCXConnUtilTester::NewL
// -----------------------------------------------------------------------------
//
CVCXConnUtilTester* CVCXConnUtilTester::NewL()
    {
    VCXLOGLO1(">>>CVCXConnUtilTester::NewL");
    CVCXConnUtilTester* self = new (ELeave) CVCXConnUtilTester();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    VCXLOGLO1("<<<CVCXConnUtilTester::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTester::CVCXConnUtilTester
// -----------------------------------------------------------------------------
//
CVCXConnUtilTester::CVCXConnUtilTester()
    {

    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTester::ConstructL
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTester::ConstructL()
    {
    VCXLOGLO1(">>>CVCXConnUtilTester::ConstructL");

    iIsRoamingAllowed = ETrue;
    iLeaveAtRoamingRequest = EFalse;
    iDelaySecondsAtRoamingRequest = 0;

    iConnUtil = CVcxConnectionUtility::InstanceL();
    iConnUtil->RegisterObserverL( this );

    iTimer = CIptvTestTimer::NewL( *this, 0 );

    iWait = CIptvTestActiveWait::NewL();

    VCXLOGLO1("<<<CVCXConnUtilTester::ConstructL");
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTester::~CVCXConnUtilTester
// -----------------------------------------------------------------------------
//
CVCXConnUtilTester::~CVCXConnUtilTester()
    {
    VCXLOGLO2(">>>CVCXConnUtilTester::~CVCXConnUtilTester (%S)", &iName);

    if( iConnUtil )
        {
        iConnUtil->RemoveObserver( this );
        TRAP_IGNORE( iConnUtil->DisconnectL() );
        iConnUtil->DecreaseReferenceCount();
        }
    iConnUtil = NULL;

    if( iTimer )
        {
        iTimer->CancelTimer();
        delete iTimer;
        iTimer = NULL;
        }

    delete iWait;
    iWait = NULL;

    VCXLOGLO2("<<<CVCXConnUtilTester::~CVCXConnUtilTester (%S)", &iName);
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTester::GetIap
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTester::GetIap( TUint32& aIapId, TBool aSilent, TBool aIsTimed )
    {
    VCXLOGLO2(">>>CVCXConnUtilTester::GetIap (%S)", &iName);
    VCXLOGLO2("CVCXConnUtilTester:: isTimed: %d", aIsTimed);

    if( aIsTimed )
        {
        // Time GetIap request when next minute starts
        TInt seconds( 0 );
        TTime now;
        now.HomeTime();

        TDateTime time = now.DateTime();

        if( time.Second() >= 55 )
            {
            seconds += 10; // Wait extra 10 seconds to make sure any other test module is ready.
            }

        seconds += 60 - time.Second(); // Wait to end of minute

        VCXLOGLO3("CVCXConnUtilTester:: waiting %d seconds (%S)", seconds, &iName);

        TUint32 microSeconds( 0 );
        microSeconds = seconds * 1000000;

        microSeconds += 1 - time.MicroSecond(); // And any extra microseconds

        iTimer->After( microSeconds );
        iWait->Start();
        }

    VCXLOGLO2("CVCXConnUtilTester:: making GetIap call (%S) ----->", &iName);
    TInt err = iConnUtil->GetIap( aIapId, aSilent );
    VCXLOGLO2("CVCXConnUtilTester:: GetIap returns: %d", err);
    VCXLOGLO2("<<<CVCXConnUtilTester::GetIap (%S)", &iName);
    return err;
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTester::WapIdFromIapIdL
// -----------------------------------------------------------------------------
//
TUint32 CVCXConnUtilTester::WapIdFromIapIdL( const TInt32& aIapId )
    {
    VCXLOGLO2(">>>CVCXConnUtilTester::WapIdFromIapIdL (%S) ----->", &iName);
    TUint32 aWapId = iConnUtil->WapIdFromIapIdL( aIapId );
    VCXLOGLO2("CVCXConnUtilTester:: WapIdFromIapIdL returns %d", aWapId);
    VCXLOGLO2("<<<CVCXConnUtilTester::WapIdFromIapIdL (%S)", &iName);
    return aWapId;
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTester::Disconnect
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTester::Disconnect()
    {
    VCXLOGLO2(">>>CVCXConnUtilTester::Disconnect (%S) ----->", &iName);
    TRAPD( err, iConnUtil->DisconnectL() );
    VCXLOGLO2("CVCXConnUtilTester:: Disconnect returns %d", err);
    VCXLOGLO2("<<<CVCXConnUtilTester::Disconnect (%S)", &iName);
    return err;
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTester::RequestIsRoamingAllowedL
// -----------------------------------------------------------------------------
//
TBool CVCXConnUtilTester::RequestIsRoamingAllowedL()
    {
    VCXLOGLO2(">>>CVCXConnUtilTester::RequestIsRoamingAllowedL (%S) ----->", &iName);

    if( iLeaveAtRoamingRequest )
        {
        VCXLOGLO1("CVCXConnUtilTester:: iLeaveAtRoamingRequest set. LEAVING!");
        User::Leave( KErrTotalLossOfPrecision  );
        }

    if( iDelaySecondsAtRoamingRequest > 0 )
        {
        VCXLOGLO2("CVCXConnUtilTester:: delay: %d", iDelaySecondsAtRoamingRequest );
        iTimer->After( iDelaySecondsAtRoamingRequest * 1000000 );
        iWait->Start();
        }

    VCXLOGLO2("CVCXConnUtilTest:: iIsRoamingAllowed = %d", iIsRoamingAllowed);

    VCXLOGLO2("<<<CVCXConnUtilTester::RequestIsRoamingAllowedL (%S)", &iName);
    return iIsRoamingAllowed;
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTester::IapChangedL
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTester::IapChangedL()
    {
    VCXLOGLO2(">>>CVCXConnUtilTester::IapChangedL (%S)", &iName);
    VCXLOGLO2("<<<CVCXConnUtilTester::IapChangedL (%S)", &iName);
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTest::TimerComplete
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTester::TimerComplete( TInt /* aTimerId */, TInt aError )
    {
    VCXLOGLO3(">>>CVCXConnUtilTester::TimerComplete, err: %d (%S)", aError, &iName);
    iWait->Stop();
    VCXLOGLO2("<<<CVCXConnUtilTester::TimerComplete (%S)", &iName);
    }


// EOF
