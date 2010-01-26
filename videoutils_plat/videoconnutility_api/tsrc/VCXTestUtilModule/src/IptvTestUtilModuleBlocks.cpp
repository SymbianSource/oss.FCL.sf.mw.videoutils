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
#include <e32svr.h>
#include <f32fsys.h>
#include <StifParser.h>
#include <Stiftestinterface.h>

#include <BADESCA.H>
#include <COMMDB.H>
#include <zipfile.h>

#include "VCXTestLog.h"
#include "IptvTestUtilModule.h"
#include "TestUtilConnection.h"
#include "TestUtilConnectionWaiter.h"
#include "IptvTestDownloadManager.h"
#include "VCXTestCommon.h"
#include "CIptvTestMobilecrashWatcher.h"
#include "CIptvTestTimer.h"

#include "IptvTestUtilALR.h"

// CONSTANTS
_LIT( KDateTimeString, "%D%M%Y%/0%1%/1%2%/2%3%/3 %-B%:0%H%:1%T%:2%S%.%*C2%:3%-B" );

// MACROS

// LOCAL CONSTANTS AND MACROS

// FORWARD DECLARATIONS
class CIptvTestTimer;
class CZipFile;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::Delete
// Delete here all resources allocated and opened from test methods.
// Called from destructor.
// -----------------------------------------------------------------------------
//
void CIptvTestUtilModule::Delete()
    {
    VCXLOGLO1(">>>CIptvTestUtilModule::Delete");
    if(iConnect)
    	{
    	delete iConnect;
    	iConnect = NULL;
    	}

    if(iConnectionWaiter)
    	{
    	delete iConnectionWaiter;
    	iConnectionWaiter = NULL;
    	}

	if(iDownload)
		{
		delete iDownload;
		iDownload = NULL;
		}

	if(iTimer)
		{
		delete iTimer;
		iTimer = NULL;
		}

    if( iFileIsLocked )
    	{
    	VCXLOGLO1("CIptvTestUtilModule:: Release locked file.");
    	iLockedFile.Close();
    	}
	iFileIsLocked = EFalse;

	delete iObservedServicePath;
	iObservedServicePath = NULL;
	iObservedFiles.ResetAndDestroy();
	//iObservedFiles.Close();
	VCXLOGLO1("CIptvTestUtilModule::Observed array deleted.");

	iFileModifiedDates.Reset();
	iFileModifiedDates.Close();
	VCXLOGLO1("CIptvTestUtilModule::File date array deleted.");

	iFs.Close();

	TInt i;
	for( i = 0; i < iToBeDeletedDestinations.Count(); i++ )
        {
        TPtr ptr = iToBeDeletedDestinations[i]->Des();
        iTestUtilALR->DeleteDestinationL( ptr );
        }

    iToBeDeletedDestinations.ResetAndDestroy();

    if( iTestUtilALR )
        {
        delete iTestUtilALR;
        iTestUtilALR = NULL;
        }

    if( iCaseStarted )
        {
        VCXLOGLO1("<<< Case end.");
        iCaseStarted = EFalse;
        }

    if( iDummyFilesCreated )
        {
        TRAP_IGNORE( iTestCommon->DeleteDummyFilesL( EDriveC ) );
        TRAP_IGNORE( iTestCommon->DeleteDummyFilesL( EDriveE ) );
        TRAP_IGNORE( iTestCommon->DeleteDummyFilesL( EDriveF ) );
        TRAP_IGNORE( iTestCommon->DeleteDummyFilesL( EDriveG ) );
        }
        
	if(iIptvTestMobilecrashWatcher)
		{
        CStifItemParser* nullParser( NULL );
        CheckMobilecrashesL( *nullParser );

		delete iIptvTestMobilecrashWatcher;
		iIptvTestMobilecrashWatcher = NULL;
		}
    

	VCXLOGLO1("<<<CIptvTestUtilModule::Delete");
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::RunMethodL(
    CStifItemParser& aItem )
    {

    static TStifFunctionInfo const KFunctions[] =
        {
        // Copy this line for every implemented function.
        // First string is the function name used in TestScripter script file.
        // Second is the actual implementation member function.
        ENTRY( "IptvLogCaseStart", CIptvTestUtilModule::IptvLogCaseStart ),
        ENTRY( "IptvLogCaseEnd", CIptvTestUtilModule::IptvLogCaseEnd ),
        ENTRY( "IptvLogWrite", CIptvTestUtilModule::IptvLogWrite ),

        ENTRY( "Connect", CIptvTestUtilModule::ConnectL ),
        ENTRY( "Attach", CIptvTestUtilModule::AttachL ),
        ENTRY( "Disconnect", CIptvTestUtilModule::DisconnectL ),
		ENTRY( "Terminate", CIptvTestUtilModule::TerminateConnectionL ),
		ENTRY( "WaitForConnectionActivity", CIptvTestUtilModule::WaitForConnectionActivityL ),
		ENTRY( "WaitUntilConnectionIsClosed", CIptvTestUtilModule::WaitUntilConnectionIsClosed ),
		ENTRY( "Download", CIptvTestUtilModule::DownloadL ),
		ENTRY( "StopDownloads", CIptvTestUtilModule::StopDownloadsL ),

		ENTRY( "CreateMobilecrashWatcher", CIptvTestUtilModule::CreateMobilecrashWatcherL ),
		ENTRY( "CheckMobilecrashes", CIptvTestUtilModule::CheckMobilecrashesL ),
		ENTRY( "CheckMobilecrashesZeroTolerance", CIptvTestUtilModule::CheckMobilecrashesZeroToleranceL ),

		ENTRY( "SetDefaultIapCenrep", CIptvTestUtilModule::SetDefaultIapCenRepL ), 
		ENTRY( "DeleteUsedDestinationCenRep", CIptvTestUtilModule::DeleteUsedDestinationCenRepL ),
		ENTRY( "SetUsedDestination", CIptvTestUtilModule::SetUsedDestinationL ),
		ENTRY( "CreateDestination", CIptvTestUtilModule::CreateDestinationL ),
		ENTRY( "SetConnectionMethodStringAttribute", CIptvTestUtilModule::SetConnectionMethodStringAttributeL ),
		ENTRY( "SetConnectionMethodIntAttribute", CIptvTestUtilModule::SetConnectionMethodIntAttributeL ),
		ENTRY( "SetConnectionMethodBoolAttribute", CIptvTestUtilModule::SetConnectionMethodBoolAttributeL ),
		ENTRY( "DeleteDestination", CIptvTestUtilModule::DeleteDestinationL ),
		ENTRY( "CopyMethod", CIptvTestUtilModule::CopyMethodL ),
		ENTRY( "DeleteMethod", CIptvTestUtilModule::DeleteMethodL ),
		ENTRY( "SetMethodPriority", CIptvTestUtilModule::SetMethodPriorityL ),
		ENTRY( "DeleteDestinationAfterwards", CIptvTestUtilModule::DeleteDestinationAfterwardsL ),

		ENTRY( "SetSystemTimeToday", CIptvTestUtilModule::SetSystemTimeToday ),
        ENTRY( "SetSystemTime", CIptvTestUtilModule::SetSystemTime ),
		ENTRY( "AdvanceSystemTime1Second", CIptvTestUtilModule::AdvanceSystemTime1Second ),
		ENTRY( "AdvanceSystemTimeSeconds", CIptvTestUtilModule::AdvanceSystemTimeSeconds ),
		ENTRY( "AdvanceSystemTimeMinutes", CIptvTestUtilModule::AdvanceSystemTimeMinutes ),
		ENTRY( "AdvanceSystemTimeHours", CIptvTestUtilModule::AdvanceSystemTimeHours ),
		ENTRY( "AdvanceSystemTimeDays", CIptvTestUtilModule::AdvanceSystemTimeDays ),
		ENTRY( "SetTimeZone", CIptvTestUtilModule::SetTimeZone ),

		ENTRY( "CreateFile", CIptvTestUtilModule::CreateFileL ),
		ENTRY( "DeleteFile", CIptvTestUtilModule::DeleteFileL ),
		ENTRY( "CreateFolder", CIptvTestUtilModule::CreateFolderL ),
		ENTRY( "DeleteFolder", CIptvTestUtilModule::DeleteFolderL ),
		ENTRY( "LockFile", CIptvTestUtilModule::LockFileL ),
		ENTRY( "UnlockFile", CIptvTestUtilModule::UnlockFileL ),
		ENTRY( "ExtractFile", CIptvTestUtilModule::ExtractFileL ),
		ENTRY( "SetDriveFreeSpace",  CIptvTestUtilModule::SetDriveFreeSpaceL ),
        };

    const TInt count = sizeof( KFunctions ) /
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::IptvLogCaseStart
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::IptvLogCaseStart( CStifItemParser& aItem )
    {
    //VCXLOGLO1(">>>CIptvTestUtilModule::IptvLogCaseStart");

    _LIT( KIptvTestUtilModule, "IptvTestUtilModule" );
    _LIT( KWhere, "In IptvLogCaseStart" );
    TestModuleIf().Printf( 0, KIptvTestUtilModule, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TPtrC string;

    if( aItem.GetNextString ( string ) != KErrNone )
        {
		VCXLOGLO1("CIptvTestUtilModule::IptvLogCaseStart: Could not read parameter!");
		return KErrGeneral;
        }

	VCXLOGLO2(">>> Case start: %S", &string);

	iCaseStarted = ETrue;

    return KErrNone;
	//VCXLOGLO1("<<<CIptvTestUtilModule::IptvLogCaseStart");
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::IptvLogCaseEnd
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::IptvLogCaseEnd( CStifItemParser& /* aItem */ )
    {
    //VCXLOGLO1(">>>CIptvTestUtilModule::IptvLogCaseEnd");

    _LIT( KIptvTestUtilModule, "IptvTestUtilModule" );
    _LIT( KWhere, "In IptvLogCaseEnd" );
    TestModuleIf().Printf( 0, KIptvTestUtilModule, KWhere );
    // Print to log file
    iLog->Log( KWhere );

	VCXLOGLO1("<<< Case end.");
	iCaseStarted = EFalse;

    return KErrNone;
	//VCXLOGLO1("<<<CIptvTestUtilModule::IptvLogCaseEnd");
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::IptvLogWrite
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::IptvLogWrite( CStifItemParser& aItem )
    {
    //VCXLOGLO1(">>>CIptvTestUtilModule::IptvLogWrite");

    _LIT( KIptvTestUtilModule, "IptvTestUtilModule" );
    _LIT( KWhere, "In IptvLogWrite" );
    TestModuleIf().Printf( 0, KIptvTestUtilModule, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TPtrC string;

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    if( aItem.GetNextString ( string ) != KErrNone )
        {
		VCXLOGLO1("CIptvTestUtilModule::IptvLogWrite: Could not read parameter!");
		return KErrGeneral;
        }

	VCXLOGLO2("%S", &string);

    return KErrNone;
	//VCXLOGLO1("<<<CIptvTestUtilModule::IptvLogWrite");
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::ConnectL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::ConnectL( CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::ConnectL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In ConnectL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    if(iConnect)
    	{
    	VCXLOGLO1("CIptvTestUtilModule:: Connection already exists.");
    	return KErrAlreadyExists;
    	}

	aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt err = KErrNone;

	TPtrC iapName;

	if(KErrNone != aItem.GetNextString(iapName) )
    	{
	    VCXLOGLO1("** FAIL ** Invalid testcase parameter! Iap name was not specified.");
        iLog->Log( _L("** FAIL ** Invalid testcase parameter! Iap name was not specified.") );
        VCXLOGLO1("<<<CIptvTestUtilModule::ConnectL");
    	return KErrGeneral;
    	}

	TBufC<256> name(iapName);
	TUint32 iapId;
	if(!iTestCommon->GetIapIdL(name, iapId))
		{
	    VCXLOGLO2("** FAIL ** Could not find iap with name %S.", &name);
        iLog->Log( _L("** FAIL ** Could not find specified iap.") );
        VCXLOGLO1("<<<CIptvTestUtilModule::ConnectL");
    	return KErrGeneral;
		}

	iRetryCount = 10;

    iConnect = CTestUtilConnection::NewL(this);

    iConnect->SetConnectionPreferences(KCommDbBearerUnknown, iapId);
    iConnect->ConnectL();

	if(!iTimer)
		{
		iTimer = CIptvTestTimer::NewL(*this, KConnectionTimerId);
		}
	iTimer->After(1000000 * 20);

	VCXLOGLO1("<<<CIptvTestUtilModule::ConnectL");
    return err;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::AttachL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::AttachL( CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::AttachL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In AttachL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    if(iConnect)
    	{
    	VCXLOGLO1("<<<CIptvTestUtilModule:: Connection already exists.");
    	return KErrAlreadyExists;
    	}

	aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt err = KErrNone;

	TRAP(err, PrintIAPs() );

	TPtrC iapName;

	if(KErrNone != aItem.GetNextString(iapName) )
    	{
	    VCXLOGLO1("** FAIL ** Invalid testcase parameter! Iap name was not specified.");
        iLog->Log( _L("** FAIL ** Invalid testcase parameter! Iap name was not specified.") );
        VCXLOGLO1("<<<CIptvTestUtilModule::AttachL");
    	return KErrGeneral;
    	}

	TBufC<256> name(iapName);
	TUint32 iapId;
	if(!iTestCommon->GetIapIdL(name, iapId))
		{
	    VCXLOGLO2("** FAIL ** Could not find iap with name %S.", &name);
        iLog->Log( _L("** FAIL ** Could not find specified iap.") );
        VCXLOGLO1("<<<CIptvTestUtilModule::AttachL");
    	return KErrGeneral;
		}

    iConnect = CTestUtilConnection::NewL(this);

    iConnect->SetConnectionPreferences(KCommDbBearerUnknown, iapId);
    iConnect->AttachL();

    if(!iTimer)
        {
        iTimer = CIptvTestTimer::NewL(*this, KConnectionTimerId);
        }
	iTimer->After(1000000 * 20);

	VCXLOGLO1("<<<CIptvTestUtilModule::AttachL");
    return err;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::DisconnectL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::DisconnectL( CStifItemParser& /* aItem */ )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::DisconnectL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In DisconnectL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt err = KErrNone;

    if(iTimer)
    	{
    	iTimer->CancelTimer();
    	}

    delete iConnect;
    iConnect = NULL;

	VCXLOGLO1("<<<CIptvTestUtilModule::DisconnectL");
    return err;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::TerminateConnectionL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::TerminateConnectionL( CStifItemParser& /* aItem */ )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::TerminateConnectionL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In TerminateConnectionL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt err = KErrNone;

    if(iConnect)
    	{
    	iConnect->TerminateConnectionL();

	    delete iConnect;
	    iConnect = NULL;
    	}

    if(iTimer)
    	{
    	iTimer->CancelTimer();
    	}

	VCXLOGLO1("<<<CIptvTestUtilModule::TerminateConnectionL");
    return err;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::DownloadL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::DownloadL( CStifItemParser& aItem  )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::DownloadL");
    // Print to UI
    _LIT( KIptvTestUtilModule, "IptvTestUtilModule" );
    _LIT( KWhere, "In DownloadL" );
    TestModuleIf().Printf( 0, KIptvTestUtilModule, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt err = KErrNone;

 	aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

 	TPtrC pAddress, pOutFile, pUserName, pPassword, pIapName;

    if( aItem.GetNextString ( pAddress ) != KErrNone )
        {
		VCXLOGLO1("CIptvTestUtilModule::DownloadL: Could not read parameter address!");
		return KErrGeneral;
        }

    HBufC* heapBuffer = HBufC::NewL(4000);
    TPtr ptr( heapBuffer->Des() );
    ptr.Copy( pAddress );

    if( aItem.GetNextString ( pOutFile ) != KErrNone )
        {
		VCXLOGLO1("CIptvTestUtilModule::DownloadL: Could not read parameter outputfile!");
		return KErrGeneral;
        }

    if( aItem.GetNextString ( pUserName ) != KErrNone )
        {
		VCXLOGLO1("CIptvTestUtilModule::DownloadL: no user name specified!");
        }

    if( aItem.GetNextString ( pPassword ) != KErrNone )
        {
		VCXLOGLO1("CIptvTestUtilModule::DownloadL: no password specified!");
        }

    if( aItem.GetNextString ( pIapName ) != KErrNone )
        {
		VCXLOGLO1("CIptvTestUtilModule::DownloadL: no iap specified!");
        }

	VCXLOGLO2("Address: %S", &pAddress);
	VCXLOGLO2("OutFile: %S", &pOutFile);
	VCXLOGLO2("name: %S", &pUserName);
	VCXLOGLO2("Password: %S", &pPassword);
	VCXLOGLO2("Iap: %S", &pIapName);

	if(!iDownload)
		{
		iDownload = CIptvTestDownloadManager::NewL(this);
		}

    TUint32 iapId(0);
    GetIap(pIapName, iapId);

	err = iDownload->DownloadL(ptr, pOutFile, pUserName, pPassword, iapId);
	if(err != KErrNone)
		{
		VCXLOGLO2("iDownload returned: %d", err);
		}

	VCXLOGLO1("<<<CIptvTestUtilModule::DownloadL");
    return err;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::StopDownloadsL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::StopDownloadsL( CStifItemParser& /* aItem */ )
	{

    VCXLOGLO1(">>>CIptvTestUtilModule::StopDownloads");
    // Print to UI
    _LIT( KIptvTestUtilModule, "IptvTestUtilModule" );
    _LIT( KWhere, "In StopDownloadsL" );
    TestModuleIf().Printf( 0, KIptvTestUtilModule, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt err = KErrNone;

	if(iDownload)
		{
		delete iDownload;
		iDownload = NULL;
		}

	VCXLOGLO1("<<<CIptvTestUtilModule::StopDownloads");
    return err;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::ConnectionCreated
// -----------------------------------------------------------------------------
//
void CIptvTestUtilModule::ConnectionCreated()
	{
	VCXLOGLO1("CIptvTestUtilModule:: Connection created.");
    if(iTimer)
    	{
    	iTimer->CancelTimer();
    	}
	Signal();
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::ConnectionClosed
// -----------------------------------------------------------------------------
//
void CIptvTestUtilModule::ConnectionClosed()
	{
	VCXLOGLO1("CIptvTestUtilModule:: Connection closed.");
	Signal();
    if(iTimer)
    	{
    	iTimer->CancelTimer();
    	}

	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::ConnectionTimeout
// -----------------------------------------------------------------------------
//
void CIptvTestUtilModule::ConnectionTimeout()
	{
	VCXLOGLO1("CIptvTestUtilModule:: Connection timeout!");
	Signal(KErrTimedOut);
    if(iTimer)
    	{
    	iTimer->CancelTimer();
    	}

	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::ConnectionFailed
// -----------------------------------------------------------------------------
//
void CIptvTestUtilModule::ConnectionFailed()
	{
	if(--iRetryCount <= 0)
		{
		Signal(KErrGeneral);
		VCXLOGLO1("CIptvTestUtilModule:: Connection failed!");
		}
	else
		{
		VCXLOGLO1("CIptvTestUtilModule:: retrying");
		iConnect->ConnectL();
		}
    if(iTimer)
    	{
    	iTimer->CancelTimer();
    	}
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::ConnectionAlreadyExists
// -----------------------------------------------------------------------------
//
void CIptvTestUtilModule::ConnectionAlreadyExists()
	{
	VCXLOGLO1("CIptvTestUtilModule:: Connection already exists.");
    if(iTimer)
    	{
    	iTimer->CancelTimer();
    	}
	Signal();
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::DownloadFinished
// -----------------------------------------------------------------------------
//
void CIptvTestUtilModule::DownloadFinished(TInt aError)
	{
	VCXLOGLO1("CIptvTestUtilModule:: Download finished.");
	Signal(aError);
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::PrintIAPs
// -----------------------------------------------------------------------------
//
void CIptvTestUtilModule::PrintIAPs()
    {
    RArray<TUint32> idArray;
    CleanupClosePushL( idArray );

    CDesCArrayFlat* nameArray = new (ELeave) CDesCArrayFlat(16);
    CleanupStack::PushL( nameArray );

    // Get IAP names and ids from the database
    CCommsDatabase* TheDb = CCommsDatabase::NewL( EDatabaseTypeIAP );
    CleanupStack::PushL( TheDb );

    TheDb->ShowHiddenRecords();

    CCommsDbTableView* view = TheDb->OpenTableLC( TPtrC(IAP) );
    //Ethernet with Daemon Dynamic IP
    TBuf<40> name;
    TUint32 id;
    TInt res = view->GotoFirstRecord();
    while( res == KErrNone )
    {
        view->ReadTextL( TPtrC(COMMDB_NAME), name );
        view->ReadUintL( TPtrC(COMMDB_ID), id );

        idArray.Insert( id, 0 );
        nameArray->InsertL( 0, name );

        res = view->GotoNextRecord();
        VCXLOGLO3("IAP name, id: %S, %d", &name, id);
    }

    CleanupStack::PopAndDestroy( view ); // view
    CleanupStack::PopAndDestroy( TheDb ); // TheDb

    CleanupStack::PopAndDestroy( 2, &idArray );  // nameArray, idArray
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::GetIap
// -----------------------------------------------------------------------------
//
void CIptvTestUtilModule::GetIap(TDesC& aIapName, TUint32& aIapId)
    {
    VCXLOGLO1(">>>CIptvTestUtilModule::GetIap");
    RArray<TUint32> idArray;
    CleanupClosePushL( idArray );

    CDesCArrayFlat* nameArray = new (ELeave) CDesCArrayFlat(16);
    CleanupStack::PushL( nameArray );

    // Get IAP names and ids from the database
    CCommsDatabase* TheDb = CCommsDatabase::NewL( EDatabaseTypeIAP );
    CleanupStack::PushL( TheDb );

    TheDb->ShowHiddenRecords();

    CCommsDbTableView* view = TheDb->OpenTableLC( TPtrC(IAP) );
    //Ethernet with Daemon Dynamic IP
    TBuf<40> name;
    TUint32 id;
    TInt res = view->GotoFirstRecord();
    while( res == KErrNone )
    {
        view->ReadTextL( TPtrC(COMMDB_NAME), name );
        view->ReadUintL( TPtrC(COMMDB_ID), id );

#ifdef __WINSCW__
        if(_L("Ethernet with Daemon Dynamic IP") == name)
            {
            aIapId = id;
            VCXLOGLO3("CIptvTestUtilModule:: Found IAP: %d, %S", _L("Ethernet with Daemon Dynamic IP"), &aIapName);
            }
#else
        if(aIapName == name)
            {
            aIapId = id;
            VCXLOGLO3("CIptvTestUtilModule:: Found IAP: %d, %S", aIapId, &aIapName);
            }
#endif

        idArray.Insert( id, 0 );
        nameArray->InsertL( 0, name );

        res = view->GotoNextRecord();
    }

    CleanupStack::PopAndDestroy( view ); // view
    CleanupStack::PopAndDestroy( TheDb ); // TheDb

    CleanupStack::PopAndDestroy( 2, &idArray );  // nameArray, idArray
    VCXLOGLO1("<<<CIptvTestUtilModule::GetIap");
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::WaitForConnectionActivityL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::WaitForConnectionActivityL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::WaitForConnectionActivityL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In WaitForConnectionActivityL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

	aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt err = KErrNone;

	TPtrC iapName;

	if( KErrNone != aItem.GetNextString(iapName) )
    	{
	    VCXLOGLO1("** FAIL ** Invalid testcase parameter! Iap name was not specified.");
        iLog->Log( _L("** FAIL ** Invalid testcase parameter! Iap name was not specified.") );
        VCXLOGLO1("<<<CIptvTestUtilModule::WaitForConnectionActivityL");
    	return KErrGeneral;
    	}

	if( iConnectionWaiter == NULL )
		{
		iConnectionWaiter = CTestUtilConnectionWaiter::NewL(this);
		}

	TUint32 iapId;
	if( !iTestCommon->GetIapIdL( iapName, iapId ) )
		{
	    VCXLOGLO2("** FAIL ** Could not find iap with name %S.", &iapName);
        iLog->Log( _L("** FAIL ** Could not find specified iap.") );
        VCXLOGLO1("<<<CIptvTestUtilModule::WaitForConnectionActivityL");
    	return KErrArgument;
		}
	
	iConnectionWaiter->WaitForConnection(iapId);

	VCXLOGLO1("<<<CIptvTestUtilModule::WaitForConnectionActivityL");
    return err;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::WaitUntilConnectionIsClosed
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::WaitUntilConnectionIsClosed(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::WaitUntilConnectionIsClosed");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In WaitForConnectionActivityL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

	aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt err = KErrNone;

	TRAP(err, PrintIAPs() );

	TPtrC iapName;

	if(KErrNone != aItem.GetNextString(iapName) )
    	{
	    VCXLOGLO1("** FAIL ** Invalid testcase parameter! Iap name was not specified.");
        iLog->Log( _L("** FAIL ** Invalid testcase parameter! Iap name was not specified.") );
        VCXLOGLO1("<<<CIptvTestUtilModule::WaitUntilConnectionIsClosed");
    	return KErrGeneral;
    	}

	if(iConnectionWaiter == NULL)
		{
		iConnectionWaiter = CTestUtilConnectionWaiter::NewL(this);
		}

	TUint32 iapId;
	if(!iTestCommon->GetIapIdL(iapName, iapId))
		{
	    VCXLOGLO2("** FAIL ** Could not find iap with name %S.", &iapName);
        iLog->Log( _L("** FAIL ** Could not find specified iap.") );
        VCXLOGLO1("<<<CIptvTestUtilModule::WaitUntilConnectionIsClosed");
    	return KErrArgument;
		}

	iConnectionWaiter->WaitUntilConnectionIsClosed(iapId);

	VCXLOGLO1("<<<CIptvTestUtilModule::WaitUntilConnectionIsClosed");
    return KErrNone;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::TimerComplete
// -----------------------------------------------------------------------------
//
void CIptvTestUtilModule::TimerComplete(TInt aTimerId, TInt aError )
	{
	VCXLOGLO1(">>>CIptvTestUtilModule::TimerComplete");
	VCXLOGLO2(">>>CIptvTestUtilModule:: aError: %d", aError);

	if(aError == KErrNone && aTimerId == KConnectionTimerId)
	{
		VCXLOGLO1("* ERROR * Connection timedout!");
		Signal(KErrTimedOut);
	}

	VCXLOGLO1("<<<CIptvTestUtilModule::TimerComplete");
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::AdvanceSystemTime1Second
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::AdvanceSystemTime1Second(CStifItemParser& /* aItem */ )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::AdvanceSystemTime1Second");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In AdvanceSystemTime1Second" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt err = CVCXTestCommon::AdvanceSystemTimeSeconds(1);

	VCXLOGLO1("<<<CIptvTestUtilModule::AdvanceSystemTime1Second");
	return err;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::AdvanceSystemTimeSeconds
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::AdvanceSystemTimeSeconds(CStifItemParser& aItem )
    {
    VCXLOGLO1(">>>CIptvTestUtilModule::AdvanceSystemTimeSeconds");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In AdvanceSystemTimeSeconds" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt count;
    if(aItem.GetNextInt(count) != KErrNone)
        {
        VCXLOGLO1("* ERROR * Parameter missing.");
        VCXLOGLO1("<<<CIptvTestUtilModule::AdvanceSystemTimeDays");
        return KErrArgument;
        }

    TInt err = CVCXTestCommon::AdvanceSystemTimeSeconds(count);

    VCXLOGLO1("<<<CIptvTestUtilModule::AdvanceSystemTimeSeconds");
    return err;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::AdvanceSystemTimeMinutes
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::AdvanceSystemTimeMinutes(CStifItemParser& aItem )
    {
    VCXLOGLO1(">>>CIptvTestUtilModule::AdvanceSystemTimeMinutes");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In AdvanceSystemTimeMinutes" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt count;
    if(aItem.GetNextInt(count) != KErrNone)
        {
        VCXLOGLO1("* ERROR * Parameter missing.");
        VCXLOGLO1("<<<CIptvTestUtilModule::AdvanceSystemTimeDays");
        return KErrArgument;
        }

    TInt err = CVCXTestCommon::AdvanceSystemTimeMinutes(count);

    VCXLOGLO1("<<<CIptvTestUtilModule::AdvanceSystemTimeMinutes");
    return err;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::AdvanceSystemTimeHours
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::AdvanceSystemTimeHours(CStifItemParser& aItem )
    {
    VCXLOGLO1(">>>CIptvTestUtilModule::AdvanceSystemTimeHours");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In AdvanceSystemTimeHours" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt count;
    if(aItem.GetNextInt(count) != KErrNone)
        {
        VCXLOGLO1("* ERROR * Parameter missing.");
        VCXLOGLO1("<<<CIptvTestUtilModule::AdvanceSystemTimeDays");
        return KErrArgument;
        }

    TInt err = CVCXTestCommon::AdvanceSystemTimeHours(count);

    VCXLOGLO1("<<<CIptvTestUtilModule::AdvanceSystemTimeHours");
    return err;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::AdvanceSystemTimeDays
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::AdvanceSystemTimeDays(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::AdvanceSystemTimeDays");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In AdvanceSystemTimeDays" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TInt daysInt;
	if(aItem.GetNextInt(daysInt) != KErrNone)
		{
		VCXLOGLO1("* ERROR * Parameter days missing.");
		VCXLOGLO1("<<<CIptvTestUtilModule::AdvanceSystemTimeDays");
		return KErrArgument;
    	}

    TTime now;
    now.HomeTime();

    TTimeIntervalDays days(daysInt);
    now += days;

    TInt err = CVCXTestCommon::SetSystemTime(now);

	VCXLOGLO1("<<<CIptvTestUtilModule::AdvanceSystemTimeDays");

	return err;
}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::SetSystemTime
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::SetSystemTime(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::SetSystemTime");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In SetSystemTime" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TPtrC dayString;
	if(aItem.GetNextString(dayString) != KErrNone)
		{
		VCXLOGLO1("* ERROR * Parameter missing.");
		VCXLOGLO1("<<<CIptvTestUtilModule::SetSystemTime");
		return KErrArgument;
    	}

    TTime time( dayString );
    TInt err = CVCXTestCommon::SetSystemTime(time);

	VCXLOGLO1("<<<CIptvTestUtilModule::SetSystemTime");
	return err;
}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::SetSystemTimeToday
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::SetSystemTimeToday(CStifItemParser& aItem )
    {
    VCXLOGLO1(">>>CIptvTestUtilModule::SetSystemTimeToday");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In SetSystemTimeToday" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt hour( 0 );
    TInt minute( 0 );

    aItem.GetNextInt( hour );
    aItem.GetNextInt( minute );

    TTime time;
    TBuf<256> timeString;
    time.HomeTime();
    time.FormatL( timeString, KDateTimeString );
    VCXLOGLO2("CIptvTestUtilModule::SetSystemTimeToday -- HomeTime() %S", &timeString);

    TDateTime dateTime( time.DateTime() );
    dateTime = time.DateTime();
    dateTime.SetHour( hour );
    dateTime.SetMinute( minute );

    time = dateTime;
    time.FormatL( timeString, KDateTimeString );
    VCXLOGLO2("CIptvTestUtilModule::SetSystemTimeToday -- System time set to: %S", &timeString);

	/*
		time.UniversalTime();
    time.FormatL( timeString, KDateTimeString );
    VCXLOGLO2("CIptvTestUtilModule::SetSystemTimeToday -- Universal time: %S", &timeString);
    */

    TInt err = CVCXTestCommon::SetSystemTime(time);

    VCXLOGLO1("<<<CIptvTestUtilModule::SetSystemTimeToday");
    return err;
}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::SetTimeZone
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::SetTimeZone(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::SetTimeZone");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In SetTimeZone" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TPtrC timeZone;
	if(aItem.GetNextString(timeZone) != KErrNone)
		{
		VCXLOGLO1("* ERROR * Parameter missing.");
		VCXLOGLO1("<<<CIptvTestUtilModule::SetTimeZone");
		return KErrArgument;
    	}

    TInt err = CVCXTestCommon::SetTimeZone(timeZone);

	VCXLOGLO1("<<<CIptvTestUtilModule::SetTimeZone");
	return err;
}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::CreateMobilecrashWatcherL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::CreateMobilecrashWatcherL(CStifItemParser& /* aItem */ )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::CreateMobilecrashWatcherL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In CreateMobilecrashWatcherL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

	if(iIptvTestMobilecrashWatcher)
		{
		delete iIptvTestMobilecrashWatcher;
		iIptvTestMobilecrashWatcher = NULL;
		}

	iIptvTestMobilecrashWatcher = CIptvTestMobilecrashWatcher::NewL(EFalse);

	VCXLOGLO1("<<<CIptvTestUtilModule::CreateMobilecrashWatcherL");
	return KErrNone;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::CheckMobilecrashesL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::CheckMobilecrashesL(CStifItemParser& /* aItem */ )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::CheckMobilecrashesL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In CheckMobilecrashesl" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

	if(!iIptvTestMobilecrashWatcher)
		{
		VCXLOGLO1("<<<CIptvTestUtilModule::CheckMobilecrashesL");
		return KErrNotReady;
		}

	RPointerArray<HBufC> crashes;
	if(iIptvTestMobilecrashWatcher->ReturnNewCrashes(crashes))
		{
		VCXLOGLO1("<<<CIptvTestUtilModule::CheckMobilecrashesL");
		crashes.ResetAndDestroy();
		return KErrAbort;
		}
	crashes.ResetAndDestroy();

	VCXLOGLO1("<<<CIptvTestUtilModule::CheckMobilecrashesL");
	return KErrNone;
}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::CheckMobilecrashesZeroToleranceL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::CheckMobilecrashesZeroToleranceL(CStifItemParser& /* aItem */ )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::CheckMobilecrashesZeroToleranceL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In CheckMobilecrashesl" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

	if(!iIptvTestMobilecrashWatcher)
		{
		iIptvTestMobilecrashWatcher = CIptvTestMobilecrashWatcher::NewL(EFalse);
		}

	TInt ret = KErrNone;
	if(iIptvTestMobilecrashWatcher->ReturnMobileCrashCount() > 0)
		{
		ret = KErrAbort;
		}

	VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
	VCXLOGLO1("<<<CIptvTestUtilModule::CheckMobilecrashesZeroToleranceL");
	return ret;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::DeleteUsedDestinationCenRepL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::DeleteUsedDestinationCenRepL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::DeleteUsedDestinationCenRepL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In DeleteUsedDestinationCenRepL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt ret( KErrNone );

    if( !iTestUtilALR )
        {
        iTestUtilALR = CIptvTestUtilALR::NewL();
        }

	aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TRAP( ret, iTestUtilALR->RemoveUsedDestinationCenRepL() );

	VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
	VCXLOGLO1("<<<CIptvTestUtilModule::DeleteUsedDestinationCenRepL");
	return ret;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::SetDefaultIapCenRep
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::SetDefaultIapCenRepL(CStifItemParser& aItem )
    {
    VCXLOGLO1(">>>CIptvTestUtilModule::SetDefaultIapCenRep");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In SetDefaultIapCenRep" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt ret( KErrNone );

    if( !iTestUtilALR )
        {
        iTestUtilALR = CIptvTestUtilALR::NewL();
        }

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TRAP( ret, iTestUtilALR->SetDefaultIapCenRep() );

    VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
    VCXLOGLO1("<<<CIptvTestUtilModule::SetDefaultIapCenRep");
    return ret;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::SetUsedDestinationL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::SetUsedDestinationL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::SetUsedDestinationL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In SetUsedDestinationL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt ret( KErrNone );

    if( !iTestUtilALR )
        {
        iTestUtilALR = CIptvTestUtilALR::NewL();
        }

	aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

	TPtrC destinationName;
	User::LeaveIfError( aItem.GetNextString( destinationName ) );

    TRAP( ret, iTestUtilALR->SetUsedDestinationL( destinationName ) );

	VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
	VCXLOGLO1("<<<CIptvTestUtilModule::SetUsedDestinationL");
	return ret;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::CreateDestinationL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::CreateDestinationL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::CreateDestinationL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In CreateDestinationL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt ret( KErrNotSupported );

    if( !iTestUtilALR )
        {
        iTestUtilALR = CIptvTestUtilALR::NewL();
        }

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

	TPtrC destinationName;
	User::LeaveIfError( aItem.GetNextString( destinationName ) );

    TRAP(ret, iTestUtilALR->CreateDestinationL( destinationName ) );

    VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
	VCXLOGLO1("<<<CIptvTestUtilModule::CreateDestinationL");
	return ret;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::DeleteDestinationL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::DeleteDestinationL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::DeleteDestinationL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In DeleteDestinationL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt ret( KErrNotSupported );

    if( !iTestUtilALR )
        {
		iTestUtilALR = CIptvTestUtilALR::NewL();
        }

	aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

	TPtrC destinationName;
	User::LeaveIfError( aItem.GetNextString( destinationName ) );

    TRAP(ret, iTestUtilALR->DeleteDestinationL( destinationName ) );

	VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
	VCXLOGLO1("<<<CIptvTestUtilModule::DeleteDestinationL");
	return ret;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::CopyMethodL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::CopyMethodL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::CopyMethodL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In CopyMethodL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt ret( KErrNotSupported );

    if( !iTestUtilALR )
        {
        iTestUtilALR = CIptvTestUtilALR::NewL();
        }

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

	TPtrC sourceMethodName;
	TPtrC targetMethodName;
	TPtrC targetDestinationName;

	User::LeaveIfError( aItem.GetNextString( sourceMethodName ) );
	User::LeaveIfError( aItem.GetNextString( targetMethodName ) );
	User::LeaveIfError( aItem.GetNextString( targetDestinationName ) );

	TRAP(ret, iTestUtilALR->CopyMethodL( sourceMethodName, targetMethodName, targetDestinationName ) );

	VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
	VCXLOGLO1("<<<CIptvTestUtilModule::CopyMethodL");
	return ret;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::DeleteMethodL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::DeleteMethodL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::DeleteMethodL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In DeleteMethodL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt ret( KErrNotSupported );

    if( !iTestUtilALR )
        {
        iTestUtilALR = CIptvTestUtilALR::NewL();
        }

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TPtrC destinationName;
	TPtrC methodName;

	User::LeaveIfError( aItem.GetNextString( destinationName ) );
	User::LeaveIfError( aItem.GetNextString( methodName ) );

	TRAP(ret, iTestUtilALR->DeleteMethodL( destinationName, methodName ) );

	VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
	VCXLOGLO1("<<<CIptvTestUtilModule::DeleteMethodL");
	return ret;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::SetMethodPriorityL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::SetMethodPriorityL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::SetMethodPriorityL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In SetMethodPriorityL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt ret( KErrNotSupported );

    if( !iTestUtilALR )
        {
        iTestUtilALR = CIptvTestUtilALR::NewL();
        }

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

	TPtrC destinationName;
	TPtrC methodName;
	TInt priority;

	User::LeaveIfError( aItem.GetNextString( destinationName ) );
	User::LeaveIfError( aItem.GetNextString( methodName ) );
	User::LeaveIfError( aItem.GetNextInt( priority ) );

	TRAP(ret, iTestUtilALR->SetMethodPriorityL( destinationName, methodName, priority ) );

	VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
	VCXLOGLO1("<<<CIptvTestUtilModule::SetMethodPriorityL");
	return ret;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::DeleteDestinationAfterwardsL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::DeleteDestinationAfterwardsL(CStifItemParser& aItem )
    {
    VCXLOGLO1(">>>CIptvTestUtilModule::DeleteDestinationAfterwardsL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In DeleteDestinationAfterwardsL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt ret( KErrNotSupported );

    if( !iTestUtilALR )
        {
        iTestUtilALR = CIptvTestUtilALR::NewL();
        }

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TPtrC destinationName;

    User::LeaveIfError( aItem.GetNextString( destinationName ) );

    iToBeDeletedDestinations.AppendL( destinationName.AllocL() );

    ret = KErrNone;

    VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
    VCXLOGLO1("<<<CIptvTestUtilModule::DeleteDestinationAfterwardsL");
    return ret;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::SetConnectionMethodStringAttributeL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::SetConnectionMethodStringAttributeL( CStifItemParser& aItem )
    {
    VCXLOGLO1(">>>CIptvTestUtilModule::SetConnectionMethodStringAttributeL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In SetConnectionMethodStringAttributeL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt ret( KErrNotSupported );

    if( !iTestUtilALR )
        {
        iTestUtilALR = CIptvTestUtilALR::NewL();
        }

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TPtrC destinationName;
    TPtrC methodName;
    TInt attribute;
    TPtrC value;

    User::LeaveIfError( aItem.GetNextString( destinationName ) );
    User::LeaveIfError( aItem.GetNextString( methodName ) );
    User::LeaveIfError( aItem.GetNextInt( attribute ) );
    User::LeaveIfError( aItem.GetNextString( value ) );

    TRAP(ret, iTestUtilALR->SetMethodStringAttributeL( destinationName, methodName, attribute, value ) );

    VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
    VCXLOGLO1("<<<CIptvTestUtilModule::SetConnectionMethodStringAttributeL");
    return ret;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::SetConnectionMethodIntAttributeL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::SetConnectionMethodIntAttributeL( CStifItemParser& aItem )
    {
        VCXLOGLO1(">>>CIptvTestUtilModule::SetConnectionMethodIntAttributeL");
        // Print to UI
        _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
        _LIT( KWhere, "In SetConnectionMethodIntAttributeL" );
        TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
        // Print to log file
        iLog->Log( KWhere );

        TInt ret( KErrNotSupported );

        if( !iTestUtilALR )
            {
            iTestUtilALR = CIptvTestUtilALR::NewL();
            }

        aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

        TPtrC destinationName;
        TPtrC methodName;
        TInt attribute;
        TInt value;

        User::LeaveIfError( aItem.GetNextString( destinationName ) );
        User::LeaveIfError( aItem.GetNextString( methodName ) );
        User::LeaveIfError( aItem.GetNextInt( attribute ) );
        User::LeaveIfError( aItem.GetNextInt( value ) );

        TRAP(ret, iTestUtilALR->SetMethodIntAttributeL( destinationName, methodName, attribute, value ) );

        VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
        VCXLOGLO1("<<<CIptvTestUtilModule::SetConnectionMethodIntAttributeL");
        return ret;
        }
// -----------------------------------------------------------------------------
// CIptvTestUtilModule::DeleteDestinationL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::SetConnectionMethodBoolAttributeL( CStifItemParser& aItem )
    {
    VCXLOGLO1(">>>CIptvTestUtilModule::SetConnectionMethodBoolAttributeL");
    // Print to UI
    _LIT( KIptvTestUtilConnect, "IptvTestUtilModule" );
    _LIT( KWhere, "In SetConnectionMethodBoolAttributeL" );
    TestModuleIf().Printf( 0, KIptvTestUtilConnect, KWhere );
    // Print to log file
    iLog->Log( KWhere );

    TInt ret( KErrNotSupported );

    if( !iTestUtilALR )
        {
        iTestUtilALR = CIptvTestUtilALR::NewL();
        }

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TPtrC destinationName;
    TPtrC methodName;
    TInt attribute;
    TInt value;

    User::LeaveIfError( aItem.GetNextString( destinationName ) );
    User::LeaveIfError( aItem.GetNextString( methodName ) );
    User::LeaveIfError( aItem.GetNextInt( attribute ) );
    User::LeaveIfError( aItem.GetNextInt( value ) );

    TRAP(ret, iTestUtilALR->SetMethodBoolAttributeL( destinationName, methodName, attribute, static_cast<TBool>(value) ) );

    VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", ret);
    VCXLOGLO1("<<<CIptvTestUtilModule::SetConnectionMethodBoolAttributeL");
    return ret;
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::CreateFileL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::CreateFileL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::CreateFileL");
    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);
	TPtrC fileName;
	User::LeaveIfError( aItem.GetNextString( fileName ) );

    RFile file;
    CleanupClosePushL(file);
    TInt err = file.Replace(iFs, fileName, EFileWrite);
    CleanupStack::PopAndDestroy( &file );
    VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", err);
    VCXLOGLO1("<<<CIptvTestUtilModule::CreateFileL");
    return err;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::DeleteFileL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::DeleteFileL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::DeleteFileL");
    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);
	TPtrC fileName;
	User::LeaveIfError( aItem.GetNextString( fileName ) );
    TInt err = iFs.Delete( fileName );
    VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", err);
    VCXLOGLO1("<<<CIptvTestUtilModule::DeleteFileL");
    return err;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::CreateFolderL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::CreateFolderL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::CreateFolderL");
    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);
	TPtrC path;
	User::LeaveIfError( aItem.GetNextString( path ) );
    BaflUtils::EnsurePathExistsL(iFs, path);
    VCXLOGLO1("<<<CIptvTestUtilModule::CreateFolderL");
    return KErrNone;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::DeleteFolderL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::DeleteFolderL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::DeleteFolderL");
    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);
	TPtrC path;
	User::LeaveIfError( aItem.GetNextString( path ) );
    CFileMan* fileMan = CFileMan::NewL(iFs);
    CleanupStack::PushL(fileMan);
	fileMan->RmDir( path );
	CleanupStack::PopAndDestroy( fileMan );
    VCXLOGLO1("<<<CIptvTestUtilModule::DeleteFolderL");
    return KErrNone;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::LockFileL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::LockFileL(CStifItemParser& aItem )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::LockFileL");
    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);
	TPtrC fileName;
	User::LeaveIfError( aItem.GetNextString( fileName ) );

    TInt err( KErrInUse );
    if( !iFileIsLocked )
    	{
    	err = iLockedFile.Open(iFs, fileName, EFileShareExclusive | EFileOpen);
    	iFileIsLocked = ETrue;
    	}
    VCXLOGLO2("<<<CIptvTestUtilModule:: returning: %d", err);
    VCXLOGLO1("<<<CIptvTestUtilModule::LockFileL");
    return err;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::LockFileL
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::UnlockFileL(CStifItemParser& /* aItem */ )
	{
    VCXLOGLO1(">>>CIptvTestUtilModule::UnlockFileL");
    if( iFileIsLocked )
    	{
    	iLockedFile.Close();
    	}
	iFileIsLocked = EFalse;
    VCXLOGLO1("<<<CIptvTestUtilModule::UnlockFileL");
    return KErrNone;
	}

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::SetDriveFreeSpaceL(TUint size in bytes)
//
// Creates dummy file so that the remaining disk space equals to given parameter
//
// OLD INFO???
// Note that the videocenter considers C: drive to be full when there is 10% left,
// so if "free space" param is 0, this leaves actually 10% of drive space free.
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::SetDriveFreeSpaceL( CStifItemParser& aItem )
    {
    VCXLOGLO1(">>>CIptvTestUtilModule::SetDriveFreeSpaceL");
    // Print to UI
    _LIT( KWhere, "IptvTestUtilModule" );
    _LIT( KSetCFreeSpace, "In SetDriveFreeSpaceL" );
    TestModuleIf().Printf( 0, KWhere, KSetCFreeSpace );
    // Print to log file
    iLog->Log( KSetCFreeSpace );

    TPtrC driveLetter;
    User::LeaveIfError(aItem.GetNextString( driveLetter ));

    VCXLOGLO2("CVCXTestCommon:: drive: %S", &driveLetter);

    TInt driveNumber(0);
    User::LeaveIfError( iFs.CharToDrive( driveLetter[0], driveNumber ) );

    TUint desiredSpace(0);
    User::LeaveIfError(aItem.GetNextInt( desiredSpace ));

    VCXLOGLO2("CVCXTestCommon:: desired space: %d", desiredSpace);

    iDummyFilesCreated = ETrue;

    iTestCommon->SetDriveFreeSpaceL( driveNumber, desiredSpace );

    VCXLOGLO1("<<<CIptvTestUtilModule::SetDriveFreeSpaceL");
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CIptvTestVerifyData::ExtractFile()
// -----------------------------------------------------------------------------
//
TInt CIptvTestUtilModule::ExtractFileL( CStifItemParser& aItem )
    {
    VCXLOGLO1(">>>CIptvTestUtilModule::ExtractFile");

    aItem.SetParsingType(CStifItemParser::EQuoteStyleParsing);

    TPtrC zipFileName;
    User::LeaveIfError(aItem.GetNextString( zipFileName ));

    TPtrC sourceFileName;
    User::LeaveIfError(aItem.GetNextString( sourceFileName ));

    TPtrC destFileName;
    User::LeaveIfError(aItem.GetNextString( destFileName ));

    iFs.Delete( sourceFileName );

    CZipFile* zipFile;
    TRAPD(err, zipFile = CZipFile::NewL( iFs, zipFileName ) );

    if( err != KErrNone )
        {
        VCXLOGLO2("CIptvTestUtilModule:: error %d", err);
        delete zipFile;
        return err;
        }

    VCXLOGLO2("CIptvTestUtilModule:: zipFileName:    %S", &zipFileName);
    VCXLOGLO2("CIptvTestUtilModule:: sourceFileName: %S", &sourceFileName);
    VCXLOGLO2("CIptvTestUtilModule:: destFileName:   %S", &destFileName);

    if( !FileExistsInZip( zipFile, sourceFileName ) )
        {
        VCXLOGLO1("CIptvTestUtilModule:: No such file in zip! Leaving..");
        delete zipFile;
        User::Leave( KErrNotFound );
        }

    // Get zip file member from the archive
    VCXLOGLO1("CIptvTestUtilModule:: Get zip file member from the archive");
    CZipFileMember* member = zipFile->CaseInsensitiveMemberL( sourceFileName );
    if( member == NULL )
        {
        VCXLOGLO1("CIptvTestUtilModule:: zip member is null.  Leaving..");
        delete zipFile;
        User::Leave(KErrGeneral);
        }
    CleanupStack::PushL( member );

    // Get read stream for the file
    VCXLOGLO1("CIptvTestUtilModule:: Get read stream for the file");
    RZipFileMemberReaderStream* stream;
    zipFile->GetInputStreamL( member, stream );
    CleanupStack::PushL( stream );

    // Alloc buffer and read the archived file
    VCXLOGLO1("CIptvTestUtilModule:: Alloc buffer and read the archived file");
    HBufC8* buffer = HBufC8::NewLC( member->UncompressedSize() );
    TPtr8 bufferPtr( buffer->Des() );
    User::LeaveIfError( stream->Read( bufferPtr, member->UncompressedSize() ) );

    // Write the file to the filesystem
    VCXLOGLO1("CIptvTestUtilModule:: Write the file to the filesystem");
    RFile file;
    User::LeaveIfError( file.Replace( iFs, destFileName, EFileWrite ) );
    CleanupClosePushL( file );
    User::LeaveIfError( file.Write(*buffer) );

    delete zipFile;
    CleanupStack::PopAndDestroy( &file );
    CleanupStack::PopAndDestroy( buffer );
    CleanupStack::PopAndDestroy( stream );
    CleanupStack::PopAndDestroy( member );

    VCXLOGLO1("<<<CIptvTestUtilModule::ExtractFile");
    return 0;
    }


// -----------------------------------------------------------------------------
// CIptvTestVerifyData::FileExistsInZip()
// -----------------------------------------------------------------------------
//
TBool CIptvTestUtilModule::FileExistsInZip( CZipFile* aZipFile, const TDesC& aFileName )
    {
    VCXLOGLO1(">>>CIptvTestVerifyData::FileExistsInZip");

    CZipFileMember* member = NULL;

    VCXLOGLO1("CIptvTestVerifyData:: list of members");
    CZipFileMemberIterator* members = aZipFile->GetMembersL();

    VCXLOGLO1("CIptvTestVerifyData:: loop all members");

    while ((member = members->NextL()) != 0)
        {
        VCXLOGLO3("CIptvTestVerifyData:: member %S, size: %d", &(*member->Name()), member->CompressedSize());

        if( aFileName == (*member->Name()) )
            {
            delete member;
            delete members;
            VCXLOGLO1("<<<CIptvTestVerifyData::FileExistsInZip");
            return ETrue;
            }

        delete member;
        }

    delete members;
    VCXLOGLO1("<<<CIptvTestVerifyData::FileExistsInZip");
    return EFalse;
    }


// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  End of File
