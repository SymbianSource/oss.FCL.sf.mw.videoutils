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


//  Include Files

#include "VCXConnUtilTestExe.h"
#include <e32base.h>
#include <e32std.h>
#include "VCXTestLog.h"

#include "VCXConnUtilTestCommon.h"
#include "CIptvTestActiveWait.h"
#include "CIptvTestTimer.h"
#include "VCXConnUtilTestExeTester.h"
#include "VCXTestTimerWait.h"

//  Constants

//  Global Variables

//  Local Functions

// Implements just Error() to avoid panic
class CSimpleScheduler : public CActiveScheduler
    {
    void Error( TInt ) const{} // From CActiveScheduler
    };

LOCAL_C void MainL()
    {
    VCXLOGLO1(">>>VCXConnUtilTestExe ## MainL");

    RProcess process;

    CVCXTestTimerWait* wait = CVCXTestTimerWait::NewL();
    CleanupStack::PushL( wait );

    // Create stopper class.
    CVCXConnUtilTestExeTester* stopper = CVCXConnUtilTestExeTester::NewL( process.Id() );
    CleanupStack::PushL( stopper );

    // Loop until done.
    while( stopper->Running() )
        {
        wait->WaitL( 1000000 );
        }

    VCXLOGLO1("VCXConnUtilTestExe ## Loop done!");

    CleanupStack::PopAndDestroy( stopper );
    CleanupStack::PopAndDestroy( wait );

    VCXLOGLO1("<<<VCXConnUtilTestExe ## MainL");
    }

LOCAL_C void DoStartL()
    {
    // Create active scheduler (to run active objects)
    CSimpleScheduler* scheduler = new (ELeave) CSimpleScheduler();
    CleanupStack::PushL(scheduler);
    CActiveScheduler::Install(scheduler);

    MainL();

    // Delete active scheduler
    CleanupStack::PopAndDestroy(scheduler);
    }

//  Global Functions

GLDEF_C TInt E32Main()
    {
    // Create cleanup stack
    //__UHEAP_MARK;
    CTrapCleanup* cleanup = CTrapCleanup::New();

    // Run application code inside TRAP harness, wait keypress when terminated
    TRAPD(mainError, DoStartL());
    if (mainError)

    delete cleanup;
    //__UHEAP_MARKEND;
    return KErrNone;
    }

