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


// [INCLUDE FILES] - do not remove
#include <e32svr.h>
#include <commdbconnpref.h>
#include <centralrepository.h>
#include <mmf/common/mmfcontrollerpluginresolver.h> // CleanupResetAndDestroyPushL

#include <StifParser.h>
#include <Stiftestinterface.h>

#include "VCXConnUtilTest.h"
#include "VCXConnUtilTestCommon.h"
#include "VCXTestCommon.h"
#include "IptvTestUtilALR.h"
#include "VCXConnUtilTestSubscriber.h"
#include "CIptvTestTimer.h"
#include "CIptvTestActiveWait.h"
#include "VCXTestTimerWait.h"

#include "vcxconnectionutility.h"
#include "VCXTestLog.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CVCXConnUtilTest::Delete
// Delete here all resources allocated and opened from test methods.
// Called from destructor.
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTest::Delete()
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::Delete (%S)", &iName);

    delete iTestCommon;
    iTestCommon = NULL;

    if( iConnectionStatusSubscriber )
        {
        iConnectionStatusSubscriber->EndWait();
        }
    delete iConnectionStatusSubscriber;
    iConnectionStatusSubscriber = NULL;

    if( iIapIdSubscriber )
        {
        iIapIdSubscriber->EndWait();
        }
    delete iIapIdSubscriber;
    iIapIdSubscriber = NULL;

    if( iSnapIdSubscriber )
        {
        iSnapIdSubscriber->EndWait();
        }
    delete iSnapIdSubscriber;
    iSnapIdSubscriber = NULL;

    if( iMasterExistsSubscriber )
        {
        iMasterExistsSubscriber->EndWait();
        }
    delete iMasterExistsSubscriber;
    iMasterExistsSubscriber = NULL;

    if( iNbrConnInstancesSubscriber )
        {
        iNbrConnInstancesSubscriber->EndWait();
        }
    delete iNbrConnInstancesSubscriber;
    iNbrConnInstancesSubscriber = NULL;

    if( iRoamingRequestStatusSubscriber )
        {
        iRoamingRequestStatusSubscriber->EndWait();
        }
    delete iRoamingRequestStatusSubscriber;
    iRoamingRequestStatusSubscriber = NULL;

    if( iNbrRoamRespSubscriber )
        {
        iNbrRoamRespSubscriber->EndWait();
        }
    delete iNbrRoamRespSubscriber;
    iNbrRoamRespSubscriber = NULL;

    if( iNbRoamAcceptedSubscriber )
        {
        iNbRoamAcceptedSubscriber->EndWait();
        }
    delete iNbRoamAcceptedSubscriber;
    iNbRoamAcceptedSubscriber = NULL;

    delete iTesterCountSubscriber;
    iTesterCountSubscriber = NULL;

    iWaitedPSChanges.Reset();
    iWaitedPSChanges.Close();

    iNotAllowedPSChanges.Reset();
    iNotAllowedPSChanges.Close();

    if( iTimeoutTimer )
        {
        iTimeoutTimer->CancelTimer();
        delete iTimeoutTimer;
        iTimeoutTimer = NULL;
        }

    delete iWait;
    iWait = NULL;

    if( !iShutdownCalled )
        {
        CStifItemParser* fakeParser = NULL;
        ShutdownTesterL( *fakeParser );
        }

    VCXLOGLO2("<<<CVCXConnUtilTest::Delete (%S)", &iName);
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTest::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::RunMethodL(
    CStifItemParser& aItem )
    {
    static TStifFunctionInfo const KFunctions[] =
        {
        // Copy this line for every implemented function.
        // First string is the function name used in TestScripter script file.
        // Second is the actual implementation member function.
        ENTRY( "PrepareCase", CVCXConnUtilTest::PrepareCaseL ),
        ENTRY( "ShutdownTester", CVCXConnUtilTest::ShutdownTesterL ),
        ENTRY( "Create", CVCXConnUtilTest::CreateL ),
        ENTRY( "GetIap", CVCXConnUtilTest::GetIapL ),
        ENTRY( "WapIdFromIapId", CVCXConnUtilTest::WapIdFromIapIdL ),
        ENTRY( "Disconnect", CVCXConnUtilTest::DisconnectL ),
        ENTRY( "CheckProperty", CVCXConnUtilTest::CheckPropertyL ),
        ENTRY( "SetRoamingAllowed", CVCXConnUtilTest::SetRoamingAllowedL ),
        ENTRY( "SetLeaveAtRoamingRequest", CVCXConnUtilTest::SetLeaveAtRoamingRequestL ),
        ENTRY( "SetDelayBeforeRoamingRequest", CVCXConnUtilTest::SetDelayBeforeRoamingRequestL ),
        ENTRY( "PrintConnUtilDetails", CVCXConnUtilTest::PrintConnUtilDetails ),
        ENTRY( "AddWaitedProperty", CVCXConnUtilTest::AddWaitedPropertyL ),
        ENTRY( "AddNotAllowedProperty", CVCXConnUtilTest::AddNotAllowedPropertyL ),
        ENTRY( "ResetWaitedProperties", CVCXConnUtilTest::ResetWaitedPropertiesL ),
        ENTRY( "WaitForPropertyQueue", CVCXConnUtilTest::WaitForPropertyQueueL ),
        };

    const TInt count = sizeof( KFunctions ) /
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::PrepareCaseL
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::PrepareCaseL( CStifItemParser& /* aItem */ )
    {
    VCXLOGLO1(">>>CVCXConnUtilTest::PrepareCaseL");
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In PrepareCaseL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt err = CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory, KVCXConnUtilTestExeGlobalShutdownKey, 1 );
    if( err != KErrNone )
        {
        VCXLOGLO2("CVCXConnUtilTest:: Could not read KVCXConnUtilTestExeGlobalShutdownKey from PS. err: %d", err);
        }

    VCXLOGLO1("<<<CVCXConnUtilTest::PrepareCaseL");
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::ShutdownTesterL
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::ShutdownTesterL( CStifItemParser& /* aItem */ )
    {
    VCXLOGLO1(">>>CVCXConnUtilTest::ShutdownTesterL");
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In ShutdownTesterL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    iShutdownCalled = ETrue;

    // Stop listening P&S changes.

    delete iTesterGetAckResponseSubscriber;
    iTesterGetAckResponseSubscriber = NULL;

    delete iTesterResponseSubscriber;
    iTesterResponseSubscriber = NULL;

    // Tell tester exe to shutdown.

    VCXLOGLO1("CVCXConnUtilTest:: Setting cmd EVcxConnUtilCommandShutdown");

    CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExePsKeyCmd + iPSKeyBase, EVcxConnUtilCommandShutdown );

    User::After( 3000000 );

    // Check the shutdown.

    VCXLOGLO2("CVCXConnUtilTest:: Exit type: %d", iProcess.ExitType());
    VCXLOGLO2("CVCXConnUtilTest:: Exit reason: %d", iProcess.ExitReason());
    TExitCategoryName exitCatName = iProcess.ExitCategory();
    VCXLOGLO2("CVCXConnUtilTest:: Exit category: %S", &exitCatName);

    TInt err = iProcess.ExitReason();

    if( err == KErrNone && iProcess.ExitType() != EExitKill && exitCatName.Length() == 0 )
        {
        VCXLOGLO1("CVCXConnUtilTest:: ERROR: No exit error, no panic but exit type is unusual or exe is stil running.");
        err = KErrGeneral;
        }

    VCXLOGLO1("<<<CVCXConnUtilTest::ShutdownTesterL");
    return err;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::CreateL
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::CreateL( CStifItemParser& aItem )
    {
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In CreateL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TPtrC name;
    if( aItem.GetNextString( name ) == KErrNone )
        {
        iName = name;
        }
    else
        {
        iName = _L("DEFAULT");
        }

    VCXLOGLO2(">>>CVCXConnUtilTest::CreateL (%S) ----->", &iName);

    if( aItem.GetNextInt( iMode ) != KErrNone )
        {
        iMode = KVcxConnUtilTestModeBoth;
        }

    iTestCommon = CVCXTestCommon::NewL();
    iWait = CIptvTestActiveWait::NewL();
    iTimeoutTimer = CIptvTestTimer::NewL( *this, KTimeoutTimerId );
    iPSKeyBase = 0;

    if( iMode == KVcxConnUtilTestModeBoth || iMode == KVcxConnUtilTestModeConnectivity )
        {
        CreateTesterProcessL();

        User::LeaveIfError( CVCXTestCommon::GetPSProperty( KVCXConnUtilTestPScategory, 
                KVCXConnUtilTestExeGlobalTesterCount, iPSKeyBase ) );
        
        iPSKeyBase = iPSKeyBase * 1000 + iProcess.Id();
        VCXLOGLO2("CVCXConnUtilTest:: iPSKeyBase: %d", iPSKeyBase);
        
        CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
                KVCXConnUtilTestExePsKeyCmd + iPSKeyBase, EVcxConnUtilCommandSetName );

        CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
                KVCXConnUtilTestExePsKeyCmdDescParam1 + iPSKeyBase, iName );

        // Start listening P&S for the tester responses.
        iTesterGetAckResponseSubscriber = CVCXConnUtilTestSubscriber::NewL( KVCXConnUtilTestPScategory,
                KVCXConnUtilTestExePsKeyResponseAck + iPSKeyBase, RProperty::EInt, this );
        iTesterGetAckResponseSubscriber->Start();
        VCXLOGLO2("CVCXConnUtilTestExeTester:: Listening P&S key KVCXConnUtilTestExePsKeyResponseAck: %d", KVCXConnUtilTestExePsKeyResponseAck + iPSKeyBase);

        iTesterResponseSubscriber = CVCXConnUtilTestSubscriber::NewL( KVCXConnUtilTestPScategory,
                KVCXConnUtilTestExePsKeyResponseCmd + iPSKeyBase, RProperty::EInt, this );
        iTesterResponseSubscriber->Start();
        VCXLOGLO2("CVCXConnUtilTestExeTester:: Listening P&S key KVCXConnUtilTestExePsKeyResponseCmd: %d", KVCXConnUtilTestExePsKeyResponseCmd + iPSKeyBase);
        }

    // Start listening P&S for connutil status changes.
    iConnectionStatusSubscriber = CVCXConnUtilTestSubscriber::NewL( KVcxConnUtilPScategory,
            EVCxPSConnectionStatus, RProperty::EInt, this );

    iIapIdSubscriber = CVCXConnUtilTestSubscriber::NewL( KVcxConnUtilPScategory,
            EVCxPSIapId, RProperty::EInt, this );

    iSnapIdSubscriber = CVCXConnUtilTestSubscriber::NewL( KVcxConnUtilPScategory,
            EVCxPSSnapId, RProperty::EInt, this );

    iMasterExistsSubscriber = CVCXConnUtilTestSubscriber::NewL( KVcxConnUtilPScategory,
            EVCxPSMasterExists, RProperty::EInt, this );

    iNbrConnInstancesSubscriber = CVCXConnUtilTestSubscriber::NewL( KVcxConnUtilPScategory,
            EVCxPSNbrConnInstances, RProperty::EInt, this );

    iRoamingRequestStatusSubscriber = CVCXConnUtilTestSubscriber::NewL( KVcxConnUtilPScategory,
            EVCxPSRoamingRequestStatus, RProperty::EInt, this );

    iNbrRoamRespSubscriber = CVCXConnUtilTestSubscriber::NewL( KVcxConnUtilPScategory,
            EVCxPSNbrRoamResp, RProperty::EInt, this );

    iNbRoamAcceptedSubscriber = CVCXConnUtilTestSubscriber::NewL( KVcxConnUtilPScategory,
            EVCxPSNbRoamAccepted, RProperty::EInt, this );

    if( iMode == KVcxConnUtilTestModeSubscriber || iMode == KVcxConnUtilTestModeBoth )
        {
        iConnectionStatusSubscriber->Start();
        iIapIdSubscriber->Start();
        iSnapIdSubscriber->Start();
        iMasterExistsSubscriber->Start();
        iNbrConnInstancesSubscriber->Start();
        iRoamingRequestStatusSubscriber->Start();
        iNbrRoamRespSubscriber->Start();
        iNbRoamAcceptedSubscriber->Start();
        }

    VCXLOGLO2("<<<CVCXConnUtilTest::CreateL (%S)", &iName);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::CreateTesterProcessL
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTest::CreateTesterProcessL()
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::CreateTesterProcessL (%S)", &iName);
    // Define tester count P&S key before starting the exe.    
    TInt err = CVCXTestCommon::GetPSProperty( 
            KVCXConnUtilTestPScategory, KVCXConnUtilTestExeGlobalTesterCount, iPSKeyBase );

    if( err == KErrNotFound )
        {
        User::LeaveIfError( RProperty::Define( 
                KVCXConnUtilTestPScategory, KVCXConnUtilTestExeGlobalTesterCount, RProperty::EInt ) );
        VCXLOGLO1("CVCXConnUtilTest:: Defined KVCXConnUtilTestExeGlobalTesterCount");
        User::LeaveIfError( CVCXTestCommon::SetPSProperty( 
                KVCXConnUtilTestPScategory, KVCXConnUtilTestExeGlobalTesterCount, 1 ) );
        }
    else
    if( err != KErrNone ) 
        {
        VCXLOGLO2("CVCXConnUtilTest:: err getting tester count P&S: %d", err);
        User::Leave( err );
        }
    
    // Lets subscribe tester count for changes.
    iTesterCountSubscriber = CVCXConnUtilTestSubscriber::NewL( KVCXConnUtilTestPScategory,
                KVCXConnUtilTestExeGlobalTesterCount, RProperty::EInt, this );
    iTesterCountSubscriber->Start();
    VCXLOGLO1("CVCXConnUtilTestExeTester:: Listening P&S key KVCXConnUtilTestExeGlobalTesterCount");

    // Create the process.    
    User::LeaveIfError( iProcess.Create( _L("VCXConnUtilTestExe.exe"), _L("") ) );
    iProcess.Resume();       

    TesterExeAliveL();

    // Wait until tester count P&S key has been updated.
    if( !iTesterCountChangedAlready )
        {
        iTesterCountSubscriber->WaitChangeL();
        }
    
    // Not interested anymore.
    delete iTesterCountSubscriber;
    iTesterCountSubscriber = NULL;
    
    VCXLOGLO2("<<<CVCXConnUtilTest::CreateTesterProcessL (%S)", &iName);
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::GetIapL
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::GetIapL( CStifItemParser& aItem )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::GetIapL (%S)", &iName);
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In GetIapL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    if( iMode == KVcxConnUtilTestModeSubscriber )
        {
        User::Leave( KErrNotSupported );
        }
    TesterExeAliveL();

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TPtrC destName;
    User::LeaveIfError( aItem.GetNextString( destName ) );

    CIptvTestUtilALR* alrUtil = CIptvTestUtilALR::NewLC();
    TInt destId = alrUtil->GetDestinationIdL( destName );
    CleanupStack::PopAndDestroy( alrUtil );
    
    VCXLOGLO1("CVCXConnUtilTest:: Create cenrep.");
    CRepository* cenRep = CRepository::NewLC( VCXTEST_KIptvAlrCenRepUid );
    VCXLOGLO1("CVCXConnUtilTest:: Set cenrep.");
    User::LeaveIfError( cenRep->Set( VCXTEST_KIptvCenRepUsedSnapIdKey, destId ) );
    CleanupStack::PopAndDestroy( cenRep );

    TInt isTimed(0);
    if( aItem.GetNextInt( isTimed ) != KErrNone )
        {
        isTimed = 0;
        }

    VCXLOGLO2("CVCXConnUtilTest:: Setting cmd EVcxConnUtilCommandGetIap (%S) ----->", &iName);

    // Silent call.
    User::LeaveIfError( CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExePsKeyCmdIntParam1 + iPSKeyBase, 1 ) );

    // Is this timed call.
    User::LeaveIfError( CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExePsKeyCmdIntParam2 + iPSKeyBase, isTimed ) );

    // Set the cmd.
    User::LeaveIfError( CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExePsKeyCmd + iPSKeyBase, EVcxConnUtilCommandGetIap ) );

    iTesterGetAckResponseSubscriber->WaitChangeL();

    VCXLOGLO2("<<<CVCXConnUtilTest::GetIapL (%S)", &iName);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::WapIdFromIapIdL
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::WapIdFromIapIdL( CStifItemParser& aItem )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::WapIdFromIapIdL (%S)", &iName);
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In WapIdFromIapIdL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TesterExeAliveL();

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TPtrC iapName;
    User::LeaveIfError( aItem.GetNextString( iapName ) );

    VCXLOGLO3("CVCXConnUtilTest:: Iap name: %S (%S) ----->", &iapName, &iName);
    
    TUint32 iapId( 0 );
    
    TInt err( KErrNone );
    if( !iTestCommon->GetIapIdL( iapName, iapId ) )
        {
        VCXLOGLO2("CVCXConnUtilTest:: Iap not found, error! (%S) ----->", &iName);
        err = KErrNotFound;
        }
    else
        {
        VCXLOGLO2("CVCXConnUtilTest:: Setting cmd EVcxConnUtilCommandGetWapIdForIap (%S) ----->", &iName);
    
        // IAP ID
        User::LeaveIfError( CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
                KVCXConnUtilTestExePsKeyCmdIntParam1 + iPSKeyBase, iapId ) );
    
        // Set the cmd.
        User::LeaveIfError( CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
                KVCXConnUtilTestExePsKeyCmd + iPSKeyBase, EVcxConnUtilCommandGetWapIdForIap ) );
    
        iTesterGetAckResponseSubscriber->WaitChangeL();
        }
    
    VCXLOGLO2("<<<CVCXConnUtilTest::WapIdFromIapIdL (%S)", &iName);
    return err;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::DisconnectL
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::DisconnectL( CStifItemParser& aItem )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::DisconnectL (%S)", &iName);
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In DisconnectL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    if( iMode == KVcxConnUtilTestModeSubscriber )
        {
        User::Leave( KErrNotSupported );
        }
    TesterExeAliveL();

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    VCXLOGLO2("CVCXConnUtilTest:: Setting cmd EVcxConnUtilCommandDisconnect (%S) ----->", &iName);

    // Set the cmd.
    User::LeaveIfError( CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExePsKeyCmd + iPSKeyBase, EVcxConnUtilCommandDisconnect ) );

    iTesterGetAckResponseSubscriber->WaitChangeL();

    VCXLOGLO2("<<<CVCXConnUtilTest::DisconnectL (%S)", &iName);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::CheckPropertyL
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::CheckPropertyL( CStifItemParser& aItem )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::CheckPropertyL (%S)", &iName);
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In CheckPropertyL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt property;
    User::LeaveIfError( aItem.GetNextInt( property ) );

    TInt intValue(0);
    TBuf<256> stringValue;
    stringValue.Zero();

    TBuf<256> propertyName;
    GetPropertyNameL( property, propertyName );

    VCXLOGLO2("CVCXConnUtilTest:: check %S", &propertyName);

    switch( property )
        {
        case EVCxPSConnectionStatus:
            {
            User::LeaveIfError( iConnectionStatusSubscriber->Get( intValue ) );
            break;
            }

        case EVCxPSIapId:
            {
            User::LeaveIfError( iIapIdSubscriber->Get( intValue ) );
            break;
            }

        case EVCxPSSnapId:
            {
            User::LeaveIfError( iSnapIdSubscriber->Get( intValue ) );
            break;
            }

        case EVCxPSMasterExists:
            {
            User::LeaveIfError( iMasterExistsSubscriber->Get( intValue ) );
            break;
            }

        case EVCxPSNbrConnInstances:
            {
            User::LeaveIfError( iNbrConnInstancesSubscriber->Get( intValue ) );
            break;
            }

        case EVCxPSRoamingRequestStatus:
            {
            User::LeaveIfError( iRoamingRequestStatusSubscriber->Get( intValue ) );
            break;
            }

        case EVCxPSNbrRoamResp:
            {
            User::LeaveIfError( iNbrRoamRespSubscriber->Get( intValue ) );
            break;
            }

        case EVCxPSNbRoamAccepted:
            {
            User::LeaveIfError( iNbRoamAcceptedSubscriber->Get( intValue ) );
            break;
            }

        default:
            {
            User::Leave( KErrArgument );
            }
            break;
        }

    TInt err(KErrNone);
    TInt expectedValue(0);
    if( aItem.GetNextInt( expectedValue ) != KErrNone )
        {
        err = KErrArgument;
        }
    else
    if( err == KErrNone && intValue != expectedValue )
        {
        VCXLOGLO3("CVCXConnUtilTest:: mismatch: %d, expected: %d", intValue, expectedValue );
        err = KErrCorrupt;
        }

    VCXLOGLO2("<<<CVCXConnUtilTest::CheckPropertyL (%S)", &iName);
    return err;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::PrintConnUtilDetails
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::PrintConnUtilDetails( CStifItemParser& /* aItem */ )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::PrintConnUtilDetails (%S)", &iName);

    TInt connectionStatus( 0 );
    TInt iapId( 0 );
    TInt snapId( 0 );
    TInt masterExists( 0 );
    TInt nbrConnInstances( 0 );
    TInt roamingRequestStatus( 0 );
    TInt nbrRoamResp( 0 );
    TInt nbrRoamAccepted( 0 );
    TInt err( KErrNone );

    err = iConnectionStatusSubscriber->Get( connectionStatus );
    if( err != KErrNone ) VCXLOGLO2("CVCXConnUtilTest:: err: %d getting PS iConnectionStatusSubscriber", err);

    err = iIapIdSubscriber->Get( iapId );
    if( err != KErrNone ) VCXLOGLO2("CVCXConnUtilTest:: err: %d getting PS iIapIdSubscriber", err);

    err = iSnapIdSubscriber->Get( snapId );
    if( err != KErrNone ) VCXLOGLO2("CVCXConnUtilTest:: err: %d getting PS iSnapIdSubscriber", err);

    err = iMasterExistsSubscriber->Get( masterExists );
    if( err != KErrNone ) VCXLOGLO2("CVCXConnUtilTest:: err: %d getting PS iMasterExistsSubscriber", err);

    err = iNbrConnInstancesSubscriber->Get( nbrConnInstances );
    if( err != KErrNone ) VCXLOGLO2("CVCXConnUtilTest:: err: %d getting PS iNbrConnInstancesSubscriber", err);

    err = iRoamingRequestStatusSubscriber->Get( roamingRequestStatus );
    if( err != KErrNone ) VCXLOGLO2("CVCXConnUtilTest:: err: %d getting PS iRoamingRequestStatusSubscriber", err);

    err = iNbrRoamRespSubscriber->Get( nbrRoamResp );
    if( err != KErrNone ) VCXLOGLO2("CVCXConnUtilTest:: err: %d getting PS iNbrRoamRespSubscriber", err);

    err = iNbRoamAcceptedSubscriber->Get( nbrRoamAccepted );
    if( err != KErrNone ) VCXLOGLO2("CVCXConnUtilTest:: err: %d getting PS iNbRoamAcceptedSubscriber", err);

    TBuf<256> statusStr;
    TRAP_IGNORE( GetConnectionStatusL( connectionStatus, statusStr ) );

    VCXLOGLO3("CVCXConnUtilTest:: status: %S (%d)", &statusStr, connectionStatus);
    VCXLOGLO2("CVCXConnUtilTest:: iapId: %d", iapId);
    VCXLOGLO2("CVCXConnUtilTest:: snapId: %d", snapId);
    VCXLOGLO2("CVCXConnUtilTest:: masterExists: %d", masterExists);
    VCXLOGLO2("CVCXConnUtilTest:: nbrConnInstances: %d", nbrConnInstances);
    VCXLOGLO2("CVCXConnUtilTest:: roamingRequestStatus: %d", roamingRequestStatus);
    VCXLOGLO2("CVCXConnUtilTest:: nbrRoamResp: %d", nbrRoamResp);
    VCXLOGLO2("CVCXConnUtilTest:: nbrRoamAccepted: %d", nbrRoamAccepted);

    VCXLOGLO2("<<<CVCXConnUtilTest::PrintConnUtilDetails (%S)", &iName);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::SetRoamingAllowedL
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::SetRoamingAllowedL( CStifItemParser& aItem )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::SetRoamingAllowedL (%S)", &iName);
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In SetRoamingAllowedL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    if( iMode == KVcxConnUtilTestModeSubscriber )
        {
        User::Leave( KErrNotSupported );
        }
    TesterExeAliveL();

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt temp;
    User::LeaveIfError( aItem.GetNextInt( temp ) );

    VCXLOGLO2("CVCXConnUtilTest:: Setting cmd EVcxConnUtilCommandSetRoamingAllowed (%S) ----->", &iName);

    // Set value.
    User::LeaveIfError( CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExePsKeyCmdIntParam1 + iPSKeyBase, temp ) );

    // Set the cmd.
    User::LeaveIfError( CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExePsKeyCmd + iPSKeyBase, EVcxConnUtilCommandSetRoamingAllowed ) );

    iTesterGetAckResponseSubscriber->WaitChangeL();

    VCXLOGLO2("<<<CVCXConnUtilTest::SetRoamingAllowedL (%S)", &iName);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::SetLeaveAtRoamingRequestL
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::SetLeaveAtRoamingRequestL( CStifItemParser& aItem )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::SetLeaveAtRoamingRequestL (%S)", &iName);
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In SetLeaveAtRoamingRequestL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    if( iMode == KVcxConnUtilTestModeSubscriber )
        {
        User::Leave( KErrNotSupported );
        }
    TesterExeAliveL();

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt temp;
    User::LeaveIfError( aItem.GetNextInt( temp ) );

    VCXLOGLO2("CVCXConnUtilTest:: Setting cmd EVcxConnUtilCommandSetRoamingLeaveAtRoamingRequest (%S) ----->", &iName);

    // Set value.
    User::LeaveIfError( CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExePsKeyCmdIntParam1 + iPSKeyBase, temp ) );

    // Set the cmd.
    User::LeaveIfError( CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExePsKeyCmd + iPSKeyBase, EVcxConnUtilCommandSetRoamingLeaveAtRoamingRequest ) );

    iTesterGetAckResponseSubscriber->WaitChangeL();

    VCXLOGLO2("<<<CVCXConnUtilTest::SetLeaveAtRoamingRequestL (%S)", &iName);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::SetDelayBeforeRoamingRequest
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::SetDelayBeforeRoamingRequestL( CStifItemParser& aItem )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::SetDelayBeforeRoamingRequestL (%S)", &iName);
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In SetDelayBeforeRoamingRequestL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    if( iMode == KVcxConnUtilTestModeSubscriber )
        {
        User::Leave( KErrNotSupported );
        }
    TesterExeAliveL();

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt seconds(0);
    User::LeaveIfError( aItem.GetNextInt( seconds ) );

    VCXLOGLO2("CVCXConnUtilTest:: Setting cmd EVcxConnUtilCommandSetDelayBeforeRoamingRequest (%S) ----->", &iName);

    // Set value.
    User::LeaveIfError( CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExePsKeyCmdIntParam1 + iPSKeyBase, seconds ) );

    // Set the cmd.
    User::LeaveIfError( CVCXTestCommon::SetPSProperty( KVCXConnUtilTestPScategory,
            KVCXConnUtilTestExePsKeyCmd + iPSKeyBase, EVcxConnUtilCommandSetDelayBeforeRoamingRequest ) );

    iTesterGetAckResponseSubscriber->WaitChangeL();

    VCXLOGLO2("<<<CVCXConnUtilTest::SetDelayBeforeRoamingRequestL (%S)", &iName);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::AddWaitedProperty
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::AddWaitedPropertyL( CStifItemParser& aItem )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::AddWaitedPropertyL (%S)", &iName);
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In AddWaitedPropertyL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    if( iMode == KVcxConnUtilTestModeConnectivity )
        {
        User::Leave( KErrNotSupported );
        }

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt property( -1 );
    User::LeaveIfError( aItem.GetNextInt( property ) );

    TBuf<256> propertyName;
    GetPropertyNameL( property, propertyName );

    TInt value(0);
    User::LeaveIfError( aItem.GetNextInt( value ) );

    if( property == EVCxPSConnectionStatus )
        {
        TBuf<256> connStr;
        GetConnectionStatusL( value, connStr );
        VCXLOGLO3("CVCXConnUtilTest:: %S = %S", &propertyName, &connStr);
        }
    else
        {
        VCXLOGLO3("CVCXConnUtilTest:: %S = %d", &propertyName, value);
        }

    TVcxConnTestPSProperty prop( property, value );
    iWaitedPSChanges.Append( prop );

    VCXLOGLO2("<<<CVCXConnUtilTest::AddWaitedPropertyL (%S)", &iName);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::AddNotAllowedPropertyL
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::AddNotAllowedPropertyL( CStifItemParser& aItem )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::AddNotAllowedPropertyL (%S)", &iName);
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In AddNotAllowedPropertyL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    if( iMode == KVcxConnUtilTestModeConnectivity )
        {
        User::Leave( KErrNotSupported );
        }

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt property( -1 );
    User::LeaveIfError( aItem.GetNextInt( property ) );

    TBuf<256> propertyName;
    GetPropertyNameL( property, propertyName );

    TInt value(0);
    User::LeaveIfError( aItem.GetNextInt( value ) );

    if( property == EVCxPSConnectionStatus )
        {
        TBuf<256> connStr;
        GetConnectionStatusL( value, connStr );
        VCXLOGLO3("CVCXConnUtilTest:: %S = %S", &propertyName, &connStr);
        }
    else
        {
        VCXLOGLO3("CVCXConnUtilTest:: %S = %d", &propertyName, value);
        }

    TVcxConnTestPSProperty prop( property, value );
    iNotAllowedPSChanges.Append( prop );

    VCXLOGLO2("<<<CVCXConnUtilTest::AddNotAllowedPropertyL (%S)", &iName);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::ResetWaitedProperties
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::ResetWaitedPropertiesL( CStifItemParser& aItem )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::ResetWaitedPropertiesL (%S)", &iName);
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In ResetWaitedPropertiesL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    if( iMode == KVcxConnUtilTestModeConnectivity )
        {
        User::Leave( KErrNotSupported );
        }

    iWaitedPSChanges.Reset();
    iWaitingForEmptyPropertyQueue = EFalse;
    iNotAllowedPSChanges.Reset();

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);
    VCXLOGLO2("<<<CVCXConnUtilTest::ResetWaitedPropertiesL (%S)", &iName);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CVCXMyVideosCollectionPluginTest::WaitForPropertyQueueL
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTest::WaitForPropertyQueueL( CStifItemParser& aItem )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::WaitForPropertyQueueL (%S)", &iName);
    // Print to UI
    _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
    _LIT( KWhere, "In WaitForPropertyQueueL" );
    TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    if( iMode == KVcxConnUtilTestModeConnectivity )
        {
        User::Leave( KErrNotSupported );
        }
    TesterExeAliveL();

    iWaitingForEmptyPropertyQueue = ETrue;

    // Timer is started always again when we correct property changes.
    const TInt KTimeoutMinute = 60 * 1000000;
    iTimeoutTimer->CancelTimer();
    iTimeoutTimer->After( KTimeoutMinute * 1 );

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);
    VCXLOGLO2("<<<CVCXConnUtilTest::WaitForPropertyQueueL (%S)", &iName);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTest::ValueChangedL
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTest::ValueChangedL( const TUid& aUid, const TUint32& aKey, const TInt&  aValue )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::ValueChangedL (%S)", &iName);

    if( aUid == KVcxConnUtilPScategory )
        {
        TBuf<256> pName;
        GetPropertyNameL( aKey, pName );
        if( aKey == EVCxPSConnectionStatus )
            {
            TBuf<256> connStr;
            GetConnectionStatusL( aValue, connStr );
            VCXLOGLO4("CVCXConnUtilTest::ValueChanged: %S = %S (%S) ----->", &pName, &connStr, &iName);

            _LIT( KVCXConnUtilTest, "S:" );
            TestModuleIf().Printf( 0, KVCXConnUtilTest, connStr );
            }
        else
            {
            VCXLOGLO4("CVCXConnUtilTest::ValueChanged: %S = %d (%S) ----->", &pName, aValue, &iName);
            }

        TBool found = EFalse;
        for( TInt i = iWaitedPSChanges.Count() - 1; i >= 0; i-- )
            {
            if( iWaitedPSChanges[i].iProperty == aKey && iWaitedPSChanges[i].iIntegerValue == aValue )
                {
                iWaitedPSChanges.Remove( i );
                found = ETrue;
                break;
                }
            }

        if( !found )
            {
            VCXLOGLO1("CVCXConnUtilTest:: The key was not found from wait queue.");

            // Check not allowed property changes.
            for( TInt i = iNotAllowedPSChanges.Count() - 1; i >= 0; i-- )
                {
                if( iNotAllowedPSChanges[i].iProperty == aKey && iNotAllowedPSChanges[i].iIntegerValue == aValue )
                    {
                    VCXLOGLO1("CVCXConnUtilTest:: Property change is not allowed. Error! SIGNAL.");
                    iNotAllowedPSChanges.Remove( i );
                    Signal( KErrCorrupt );
                    break;
                    }
                }
            }
        else
            {
            VCXLOGLO1("CVCXConnUtilTest:: Key found and removed from wait queue.");

            // Restart timer.
            const TInt KTimeoutMinute = 60 * 1000000;
            iTimeoutTimer->CancelTimer();
            iTimeoutTimer->After( KTimeoutMinute * 1 );

            if( iWaitedPSChanges.Count() == 0 )
                {
                if( iWaitingForEmptyPropertyQueue )
                    {
                    iWaitingForEmptyPropertyQueue = EFalse;
                    VCXLOGLO1("CVCXConnUtilTest:: Property queue empty, SIGNAL.");
                    iTimeoutTimer->CancelTimer();
                    Signal();
                    }
                }
            }

        if( iWaitedPSChanges.Count() == 0 )
            {
            iNotAllowedPSChanges.Reset();
            }
        }

    TInt err( KErrNone );

    if( aUid == KVCXConnUtilTestPScategory )
        {
        if( aKey == KVCXConnUtilTestExeGlobalTesterCount )
            {
            iTesterCountChangedAlready = ETrue;
            VCXLOGLO1("CVCXConnUtilTest:: KVCXConnUtilTestExeGlobalTesterCount changed.");
            }
        
        if( aKey == KVCXConnUtilTestExePsKeyResponseAck + iPSKeyBase )
            {
            VCXLOGLO3("CVCXConnUtilTest:: received KVCXConnUtilTestExePsKeyResponseAck, value: %d (%S) ----->", aValue, &iName);
            }

        if( aKey == KVCXConnUtilTestExePsKeyResponseCmd + iPSKeyBase )
            {
            VCXLOGLO2("CVCXConnUtilTest:: received KVCXConnUtilTestExePsKeyResponseCmd (%S) ----->", &iName);

            TInt cmd( aValue );
            
            TInt response( 0 );
            
            CVCXTestCommon::GetPSProperty( KVCXConnUtilTestPScategory,
                    KVCXConnUtilTestExePsKeyResponseParam + iPSKeyBase, response );

            CVCXTestCommon::GetPSProperty( KVCXConnUtilTestPScategory,
                    KVCXConnUtilTestExePsKeyResponseError + iPSKeyBase, err );
            
            if( cmd == EVcxConnUtilCommandDisconnect )
                {
                VCXLOGLO3("CVCXConnUtilTest:: response for EVcxConnUtilCommandDisconnect, error: %d (%S) ----->", err, &iName);
                }
            
            if( cmd == EVcxConnUtilCommandGetIap )
                {
                if( response == 0 )
                    {
                    VCXLOGLO1("CVCXConnUtilTest:: response for EVcxConnUtilCommandGetIap, IAP is 0, error!");
                    err = KErrGeneral;
                    }
                else
                    {
                    VCXLOGLO2("CVCXConnUtilTest:: response for EVcxConnUtilCommandGetIap returned IAP: %d", response);
                    }
                }
            
            if( cmd == EVcxConnUtilCommandGetWapIdForIap )
                {
                VCXLOGLO2("CVCXConnUtilTest:: response for EVcxConnUtilCommandGetWapIdForIap returned: %d", response);
                }            

            Signal( err );
            }
        }

    VCXLOGLO2("<<<CVCXConnUtilTest::ValueChangedL (%S)", &iName);
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTest::ValueChangedL
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTest::ValueChangedL( const TUid& aUid, const TUint32& aKey, const TDesC& aValue )
    {
    VCXLOGLO2(">>>CVCXConnUtilTest::ValueChangedL (%S)", &iName);

    if( aUid == KVcxConnUtilPScategory )
        {
        TBuf<256> pName;
        GetPropertyNameL( aKey, pName );
        VCXLOGLO3("CVCXConnUtilTest::ValueChanged: %S = %S", &pName, &aValue);

        TBool found = EFalse;
        for( TInt i = iWaitedPSChanges.Count() - 1; i >= 0; i-- )
            {
            if( iWaitedPSChanges[i].iProperty == aKey && iWaitedPSChanges[i].iStringValue.Compare( aValue ) == 0 )
                {
                iWaitedPSChanges.Remove( i );
                found = ETrue;
                break;
                }
            }

        if( !found )
            {
            VCXLOGLO1("CVCXConnUtilTest:: The key was not found from wait queue.");

            // Check not allowed property changes.
            for( TInt i = iNotAllowedPSChanges.Count() - 1; i >= 0; i-- )
                {
                if( iNotAllowedPSChanges[i].iProperty == aKey && iWaitedPSChanges[i].iStringValue.Compare( aValue ) )
                    {
                    VCXLOGLO1("CVCXConnUtilTest:: Property change is not allowed. Error!");
                    iNotAllowedPSChanges.Remove( i );
                    Signal( KErrCorrupt );
                    break;
                    }
                }
            }
        else
            {
            VCXLOGLO1("CVCXConnUtilTest:: Key found and removed from wait queue.");

            const TInt KTimeoutMinute = 60 * 1000000;
            iTimeoutTimer->CancelTimer();
            iTimeoutTimer->After( KTimeoutMinute * 1 );

            if( iWaitedPSChanges.Count() == 0 )
                {
                if( iWaitingForEmptyPropertyQueue )
                    {
                    iWaitingForEmptyPropertyQueue = EFalse;
                    VCXLOGLO1("CVCXConnUtilTest:: Property queue empty, SIGNAL.");
                    iTimeoutTimer->CancelTimer();
                    Signal();
                    }
                }
            }

        if( iWaitedPSChanges.Count() == 0 )
            {
            iNotAllowedPSChanges.Reset();
            }
        }

    if( aUid == KVCXConnUtilTestPScategory )
        {

        }

    VCXLOGLO2("<<<CVCXConnUtilTest::ValueChangedL (%S)", &iName);
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTest::TimerComplete
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTest::TimerComplete( TInt aTimerId, TInt aError )
    {
    VCXLOGLO3(">>>CVCXConnUtilTest::TimerComplete, err: %d (%S)", aError, &iName);

    if( aError == KErrNone && aTimerId == KTimeoutTimerId && iWaitingForEmptyPropertyQueue )
        {
        if( iWaitedPSChanges.Count() > 0 )
            {
            VCXLOGLO1("CVCXConnUtilTest:: ---- TIMEOUT --- when waiting P&S changes ----->");

            _LIT( KVCXConnUtilTest, "CVCXConnUtilTest" );
            _LIT( KWhere, "TIMEOUT!" );
            TestModuleIf().Printf( 0, KVCXConnUtilTest, KWhere );

            Signal( KErrTimedOut );
            }
        else
            {
            VCXLOGLO1("CVCXConnUtilTest:: Timed out but property queue empty. Signal ok.");
            Signal( KErrNone );
            }
        }

    VCXLOGLO2("<<<CVCXConnUtilTest::TimerComplete (%S)", &iName);
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTest::GetConnectionStatusL
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTest::GetConnectionStatusL( TInt aConnectionStatus, TDes& aStatusString )
    {
    switch( aConnectionStatus )
        {
        case EVCxNotConnected:
            {
            aStatusString.Copy( _L("EVCxNotConnected") );
            }
            break;

        case EVCxConnecting:
            {
            aStatusString.Copy( _L("EVCxConnecting") );
            }
            break;

        case EVCxConnected:
            {
            aStatusString.Copy( _L("EVCxConnected") );
            }
            break;

        case EVCxDisconnecting:
            {
            aStatusString.Copy( _L("EVCxDisconnecting") );
            }
            break;

        case EVCxRoamingRequest:
            {
            aStatusString.Copy( _L("EVCxRoamingRequest") );
            }
            break;

        case EVCxRoamingAccepted:
            {
            aStatusString.Copy( _L("EVCxRoamingAccepted") );
            }
            break;

        case EVCxError:
            {
            aStatusString.Copy( _L("EVCxError") );
            }
            break;

        default:
            {
            VCXLOGLO2("CVCXConnUtilTest:: state %d is UKNOWN!", aConnectionStatus);
            User::Leave( KErrArgument );
            }
            break;
        }
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTest::TesterExeAliveL
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTest::TesterExeAliveL()
    {
    if( iProcess.ExitType() != EExitPending || iProcess.ExitReason() != KErrNone )
        {
        VCXLOGLO2("CVCXConnUtilTest:: ERROR: Tester process has terminated! (%S)", &iName);
        VCXLOGLO2("CVCXConnUtilTest:: Exit type: %d", iProcess.ExitType());
        VCXLOGLO2("CVCXConnUtilTest:: Exit reason: %d", iProcess.ExitReason());
        TExitCategoryName exitCatName = iProcess.ExitCategory();
        VCXLOGLO2("CVCXConnUtilTest:: Exit category: %S", &exitCatName);
        User::Leave( KErrGeneral );
        }
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTest::GetPropertyNameL
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTest::GetPropertyNameL( TInt aProperty, TDes& aPropertyName )
    {
    switch( aProperty )
        {
        case EVCxPSConnectionStatus:
            {
            aPropertyName.Copy( _L("EVCxPSConnectionStatus") );
            break;
            }

        case EVCxPSIapId:
            {
            aPropertyName.Copy( _L("EVCxPSIapId") );
            break;
            }

        case EVCxPSSnapId:
            {
            aPropertyName.Copy( _L("EVCxPSSnapId") );
            break;
            }

        case EVCxPSMasterExists:
            {
            aPropertyName.Copy( _L("EVCxPSMasterExists") );
            break;
            }

        case EVCxPSNbrConnInstances:
            {
            aPropertyName.Copy( _L("EVCxPSNbrConnInstances") );
            break;
            }

        case EVCxPSRoamingRequestStatus:
            {
            aPropertyName.Copy( _L("EVCxPSRoamingRequestStatus") );
            break;
            }

        case EVCxPSNbrRoamResp:
            {
            aPropertyName.Copy( _L("EVCxPSNbrRoamResp") );
            break;
            }

        case EVCxPSNbRoamAccepted:
            {
            aPropertyName.Copy( _L("EVCxPSNbRoamAccepted") );
            break;
            }

        default:
            {
            VCXLOGLO2("CVCXConnUtilTest:: Unknown property: %d!", aProperty);
            User::Leave( KErrArgument );
            }
            break;
        }
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  [End of File] - Do not remove

