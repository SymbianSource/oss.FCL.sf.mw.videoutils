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


#include <mmf/common/mmfcontrollerpluginresolver.h> // CleanupResetAndDestroyPushL
#include <e32property.h>

#include "VCXTestCommon.h"
#include "CIptvTestTimer.h"
#include "VCXConnUtilTestCommon.h"
#include "VCXConnUtilTestExeTester.h"
#include "VCXConnUtilTestSubscriber.h"
#include "VCXConnUtilTestPSObserver.h"
#include "VCXConnUtilTester.h"

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
// CVCXConnUtilTestExeTester::
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CVCXConnUtilTestExeTester* CVCXConnUtilTestExeTester::NewL( TUint aProcessId )
    {
    VCXLOGLO1(">>>CVCXConnUtilTestExeTester::NewL");
    CVCXConnUtilTestExeTester* self = new (ELeave) CVCXConnUtilTestExeTester();
    CleanupStack::PushL(self);
    self->ConstructL( aProcessId );
    CleanupStack::Pop( self );
    VCXLOGLO1("<<<CVCXConnUtilTestExeTester::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestExeTester::~CVCXConnUtilTestExeTester
// destructor
// -----------------------------------------------------------------------------
//
CVCXConnUtilTestExeTester::~CVCXConnUtilTestExeTester()
    {
    VCXLOGLO1(">>>CVCXConnUtilTestExeTester::~CVCXConnUtilTestExeTester");

    delete iGlobalShutdownSubcriber;
    iGlobalShutdownSubcriber = NULL;

    delete iCmdSubcriber;
    iCmdSubcriber = NULL;

    delete iTester;
    iTester = NULL;

    // Remove the P&S keys for this process.
    RProperty::Delete( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyCmd + iPSKeyBase);
    RProperty::Delete( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyCmdIntParam1 + iPSKeyBase);
    RProperty::Delete( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyCmdIntParam2 + iPSKeyBase);
    RProperty::Delete( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyCmdDescParam1 + iPSKeyBase);
    RProperty::Delete( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyResponseAck + iPSKeyBase);
    RProperty::Delete( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyResponseCmd + iPSKeyBase);
    RProperty::Delete( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyResponseParam + iPSKeyBase);
    RProperty::Delete( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyResponseError + iPSKeyBase);

    VCXLOGLO1("<<<CVCXConnUtilTestExeTester::~CVCXConnUtilTestExeTester");
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestExeTester::~CVCXConnUtilTestExeTester
// destructor
// -----------------------------------------------------------------------------
//
CVCXConnUtilTestExeTester::CVCXConnUtilTestExeTester()
    {
    VCXLOGLO1(">>>CVCXConnUtilTestExeTester::CVCXConnUtilTestExeTester");

    VCXLOGLO1("<<<CVCXConnUtilTestExeTester::CVCXConnUtilTestExeTester");
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestExeTester::ConstructL
//
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTestExeTester::ConstructL( TUint aProcessId )
    {
    VCXLOGLO1(">>>CVCXConnUtilTestExeTester::ConstructL");

    iTester = CVCXConnUtilTester::NewL();

    // Define and listen global shutdown key.
    
    TInt err( KErrNone );
    
    err = RProperty::Define( KVCXConnUtilTestPScategory, KVCXConnUtilTestExeGlobalShutdownKey, RProperty::EInt );
    VCXLOGLO3("CVCXConnUtilTestExeTester:: Define KVCXConnUtilTestExeGlobalShutdownKey: %d, err: %d", 
            KVCXConnUtilTestExeGlobalShutdownKey, err);

    iGlobalShutdownSubcriber = CVCXConnUtilTestSubscriber::NewL( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExeGlobalShutdownKey, RProperty::EInt, this );
    iGlobalShutdownSubcriber->Start();
    VCXLOGLO2("CVCXConnUtilTestExeTester:: Listening P&S key KVCXConnUtilTestExeGlobalShutdownKey: %d",
            KVCXConnUtilTestExeGlobalShutdownKey);
    
    // Get PS key base.
    
    TInt testerCount( 0 );
    
    User::LeaveIfError( CVCXTestCommon::GetPSProperty( 
            KVCXConnUtilTestPScategory, KVCXConnUtilTestExeGlobalTesterCount, testerCount ) );

    testerCount++;
    
    iPSKeyBase = testerCount * 1000 + aProcessId;

    VCXLOGLO2("CVCXConnUtilTestExeTester:: iPSKeyBase: %d", iPSKeyBase);
    
    // Define private P&S keys.
    
    err = RProperty::Define( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyCmd + iPSKeyBase,
            RProperty::EInt );
    VCXLOGLO3("CVCXConnUtilTestExeTester:: Define KVCXConnUtilTestExePsKeyCmd: %d, err: %d", 
            KVCXConnUtilTestExePsKeyCmd + iPSKeyBase, err);
    
    err = RProperty::Define( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyCmdIntParam1 + iPSKeyBase,
            RProperty::EInt );
    VCXLOGLO3("CVCXConnUtilTestExeTester:: Define KVCXConnUtilTestExePsKeyCmdIntParam1: %d, err: %d", 
            KVCXConnUtilTestExePsKeyCmdIntParam1 + iPSKeyBase, err);
    
    err = RProperty::Define( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyCmdIntParam2 + iPSKeyBase,
            RProperty::EInt );
    VCXLOGLO3("CVCXConnUtilTestExeTester:: Define KVCXConnUtilTestExePsKeyCmdIntParam2: %d, err: %d", 
            KVCXConnUtilTestExePsKeyCmdIntParam2 + iPSKeyBase, err);
    
    err = RProperty::Define( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyCmdDescParam1 + iPSKeyBase,
            RProperty::EText );
    VCXLOGLO3("CVCXConnUtilTestExeTester:: Define KVCXConnUtilTestExePsKeyCmdDescParam1: %d, err: %d", 
            KVCXConnUtilTestExePsKeyCmdDescParam1 + iPSKeyBase, err);
    
    err = RProperty::Define( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyResponseAck + iPSKeyBase,
            RProperty::EInt );
    VCXLOGLO3("CVCXConnUtilTestExeTester:: Define KVCXConnUtilTestExePsKeyResponseAck: %d, err: %d", 
            KVCXConnUtilTestExePsKeyResponseAck + iPSKeyBase, err);
    
    err = RProperty::Define( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyResponseCmd + iPSKeyBase,
            RProperty::EInt );
    VCXLOGLO3("CVCXConnUtilTestExeTester:: Define KVCXConnUtilTestExePsKeyResponseCmd: %d, err: %d", 
            KVCXConnUtilTestExePsKeyResponseCmd + iPSKeyBase, err);
    
    err = RProperty::Define( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyResponseParam + iPSKeyBase,
            RProperty::EInt );
    VCXLOGLO3("CVCXConnUtilTestExeTester:: Define KVCXConnUtilTestExePsKeyResponseParam: %d, err: %d", 
            KVCXConnUtilTestExePsKeyResponseParam + iPSKeyBase, err);
    
    err = RProperty::Define( KVCXConnUtilTestPScategory, KVCXConnUtilTestExePsKeyResponseError + iPSKeyBase,
            RProperty::EInt );
    VCXLOGLO3("CVCXConnUtilTestExeTester:: Define KVCXConnUtilTestExePsKeyResponseError: %d, err: %d", 
            KVCXConnUtilTestExePsKeyResponseError + iPSKeyBase, err);
    
    // Subscribe P&S cmd key.

    iCmdSubcriber = CVCXConnUtilTestSubscriber::NewL( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExePsKeyCmd + iPSKeyBase, RProperty::EInt, this );
    iCmdSubcriber->Start();
    VCXLOGLO2("CVCXConnUtilTestExeTester:: Listening P&S key KVCXConnUtilTestExePsKeyCmd: %d", 
            KVCXConnUtilTestExePsKeyCmd + iPSKeyBase);

    iRunning = ETrue;

    // And update the tester count to P&S.
    User::LeaveIfError( CVCXTestCommon::SetPSProperty( 
            KVCXConnUtilTestPScategory, KVCXConnUtilTestExeGlobalTesterCount, testerCount ) );
    
    VCXLOGLO1("<<<CVCXConnUtilTestExeTester::ConstructL");
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestExeTester::ValueChangedL
//
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTestExeTester::ValueChangedL( const TUid& aUid, const TUint32& aKey, const TInt&  aValue )
    {
    // Right now this is only category we are subscribing for.
    if( aUid != KVCXConnUtilTestPScategory )
        {
        return;
        }

    if( aKey == KVCXConnUtilTestExeGlobalShutdownKey )
        {
        if( aValue == 1 )
            {
            iRunning = EFalse;
            VCXLOGLO2("CVCXConnUtilTestExeTester::ValueChangedL: cmd GLOBAL SHUTDOWN (%S) ----->", &iName);
            }
        return;
        }

    VCXLOGLO2(">>>CVCXConnUtilTestExeTester::ValueChangedL TInt (%S)", &iName);

    // Store possible parameters.
    TInt intParam1(0);
    TInt intParam2(0);
    TBuf<256> descParam1;

    CVCXTestCommon::GetPSProperty( KVCXConnUtilTestPScategory, 
            KVCXConnUtilTestExePsKeyCmdIntParam1 + iPSKeyBase, intParam1 );
    CVCXTestCommon::GetPSProperty( KVCXConnUtilTestPScategory, 
            KVCXConnUtilTestExePsKeyCmdIntParam2 + iPSKeyBase, intParam2 );
    CVCXTestCommon::GetPSProperty( KVCXConnUtilTestPScategory, 
            KVCXConnUtilTestExePsKeyCmdDescParam1 + iPSKeyBase, descParam1 );

    // Set response that cmd has been gotten.

    VCXLOGLO2("CVCXConnUtilTestExeTester:: Setting KVCXConnUtilTestExePsKeyResponseAck (%S) ----->", &iName);
    CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory, 
            KVCXConnUtilTestExePsKeyResponseAck + iPSKeyBase, iAckCount++ );

    // Handle commands.
    if( aKey == KVCXConnUtilTestExePsKeyCmd + iPSKeyBase )
        {
        switch( aValue )
            {
            case EVcxConnUtilCommandShutdown:
                {
                VCXLOGLO2("CVCXConnUtilTestExeTester:: cmd EVcxConnUtilCommandShutdown (%S) ----->", &iName);
                iRunning = EFalse;
                }
                break;

            case EVcxConnUtilCommandSetName:
                {
                iName.Copy( descParam1 );
                iTester->iName.Copy( descParam1 );
                VCXLOGLO2("CVCXConnUtilTestExeTester:: cmd EVcxConnUtilCommandSetName (%S) ----->", &iName);
                }
                break;

            case EVcxConnUtilCommandGetIap:
                {
                VCXLOGLO2("CVCXConnUtilTestExeTester:: cmd EVcxConnUtilCommandGetIap (%S) ----->", &iName);

                TUint32 iapId(0);
                TBool silent = static_cast<TBool>( intParam1);
                TBool isTimed = static_cast<TBool>( intParam2 );

                TInt err = iTester->GetIap( iapId, silent, isTimed );

                if( err != KErrNone || iapId == 0 )
                    {
                    VCXLOGLO4("CVCXConnUtilTestExeTester:: GetIap error: %d, iap: %d (%S)", err, iapId, &iName);
                    }

                VCXLOGLO2("CVCXConnUtilTestExeTester:: Setting response for EVcxConnUtilCommandGetIap (%S) ----->", 
                        &iName);
                CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
                        KVCXConnUtilTestExePsKeyResponseParam + iPSKeyBase, iapId );
                CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
                        KVCXConnUtilTestExePsKeyResponseError + iPSKeyBase, err );
                CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
                        KVCXConnUtilTestExePsKeyResponseCmd + iPSKeyBase, EVcxConnUtilCommandGetIap );

                }
                break;
                
                
            case EVcxConnUtilCommandGetWapIdForIap:
                {
                VCXLOGLO2("CVCXConnUtilTestExeTester:: cmd EVcxConnUtilCommandGetWapIdForIap (%S) ----->", &iName);

                TInt err( KErrNone );
                
                TUint32 wapId( 0 );
                TRAP( err, iTester->WapIdFromIapIdL( intParam1 ) );

                VCXLOGLO4("CVCXConnUtilTestExeTester:: GetWapIdFromIapIdL returned wapId: %d, err: %d (%S)", wapId, err, &iName);

                VCXLOGLO2("CVCXConnUtilTestExeTester:: Setting response for EVcxConnUtilCommandGetWapIdForIap (%S) ----->", 
                        &iName);
                CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
                        KVCXConnUtilTestExePsKeyResponseParam + iPSKeyBase, wapId );
                CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
                        KVCXConnUtilTestExePsKeyResponseError + iPSKeyBase, err );
                CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
                        KVCXConnUtilTestExePsKeyResponseCmd + iPSKeyBase, EVcxConnUtilCommandGetWapIdForIap );

                }
                break;

            case EVcxConnUtilCommandDisconnect:
                {
                VCXLOGLO2("CVCXConnUtilTestExeTester:: cmd EVcxConnUtilCommandDisconnect (%S) ----->", &iName);

                TInt err = iTester->Disconnect();

                if( err != KErrNone )
                    {
                    VCXLOGLO3("CVCXConnUtilTestExeTester:: disconnect error: %d (%S)", err, &iName);
                    }

                VCXLOGLO2("CVCXConnUtilTestExeTester:: Setting response for EVcxConnUtilCommandDisconnect (%S) ----->", 
                        &iName);
                CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
                        KVCXConnUtilTestExePsKeyResponseError + iPSKeyBase, err );
                CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
                        KVCXConnUtilTestExePsKeyResponseCmd + iPSKeyBase, EVcxConnUtilCommandDisconnect );
                }
                break;

            case EVcxConnUtilCommandSetRoamingAllowed:
                {
                VCXLOGLO2("CVCXConnUtilTestExeTester:: cmd EVcxConnUtilCommandSetRoamingAllowed (%S) ----->", 
                        &iName);
                iTester->iIsRoamingAllowed = intParam1;
                }
                break;

            case EVcxConnUtilCommandSetRoamingLeaveAtRoamingRequest:
                {
                VCXLOGLO2("CVCXConnUtilTestExeTester:: cmd EVcxConnUtilCommandSetRoamingLeaveAtRoamingRequest (%S) ----->", &iName);
                iTester->iLeaveAtRoamingRequest = intParam1;
                }
                break;

            case EVcxConnUtilCommandSetDelayBeforeRoamingRequest:
                {
                VCXLOGLO2("CVCXConnUtilTestExeTester:: cmd EVcxConnUtilCommandSetDelayBeforeRoamingRequest (%S) ----->", &iName);
                iTester->iDelaySecondsAtRoamingRequest = intParam1;
                }
                break;

            default:
                {
                VCXLOGLO3("CVCXConnUtilTestExeTester:: UNKNOWN cmd %d! (%S) ----->", aValue, &iName);
                }
                break;
            }
        }

    VCXLOGLO2("<<<CVCXConnUtilTestExeTester::ValueChangedL TInt (%S)", &iName);
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestExeTester::ValueChangedL
//
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTestExeTester::ValueChangedL( const TUid& aUid, const TUint32& /* aKey */, const TDesC& /* aValue */ )
    {
    VCXLOGLO1(">>>CVCXConnUtilTestExeTester::ValueChangedL TDesC");

    if( aUid == KVCXConnUtilTestPScategory )
        {

        }

    VCXLOGLO1("<<<CVCXConnUtilTestExeTester::ValueChangedL TDesC");
    }

//  End of File
