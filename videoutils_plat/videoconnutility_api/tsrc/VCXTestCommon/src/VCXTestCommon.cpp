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
#include <e32Base.h>
#include <tz.h>
#include <badesca.h>
#include <commdb.h>
#include <mmf/common/mmfcontrollerpluginresolver.h>

#include "VCXTestCommon.h"
#include "IptvTestUtilALR.h"
#include "CIptvTestVerifyData.h"
#include "TestUtilConnectionWaiter.h"
#include "VCXTestLog.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS
_LIT(KDummyfilePathFormat, "%S:\\data\\temp");
_LIT(KDummyfileFormat, "%d.dat");
_LIT(KDoubleBacklash, "\\");

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CVCXTestCommon::CVCXTestCommon
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CVCXTestCommon::CVCXTestCommon()
    {
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CVCXTestCommon::ConstructL()
    {
    VCXLOGLO1(">>>CVCXTestCommon::ConstructL");

    User::LeaveIfError( iFs.Connect() );

    PrintIaps(); 

    PrintDriveInfo( _L("C") );
    PrintDriveInfo( _L("E") );
    PrintDriveInfo( _L("D") );
    PrintDriveInfo( _L("F") );

    //ListDir( _L("C:\\"), 0 );
    //ListDir( _L("E:\\"), 0 );

    iIptvVerifyData = CIptvTestVerifyData::NewL();
    iVideoCreator = CIptvTestVideoCreator::NewL();

    VCXLOGLO1("<<<CVCXTestCommon::ConstructL");
    }

// -----------------------------------------------------------------------------
// CIptvServicesFromFile::~CVCXTestCommon
// Destructor
// -----------------------------------------------------------------------------
//
CVCXTestCommon::~CVCXTestCommon()
    {
    VCXLOGLO1(">>>CVCXTestCommon::~CVCXTestCommon");

    delete iIptvVerifyData;
    iIptvVerifyData = NULL;

    delete iVideoCreator;
    iVideoCreator = NULL;

    iFs.Close();

    VCXLOGLO1("<<<CVCXTestCommon::~CVCXTestCommon");
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CVCXTestCommon* CVCXTestCommon::NewL()
    {
    VCXLOGLO1(">>>CVCXTestCommon::NewL");
    CVCXTestCommon* self = new( ELeave ) CVCXTestCommon;

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    VCXLOGLO1("<<<CVCXTestCommon::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::GetFileSize
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CVCXTestCommon::GetFileSize(TDesC& aFullPath, TUint32& aFileSize)
    {
    VCXLOGLO1(">>>CVCXTestCommon::GetFileSize");
    TBool rv = FALSE;
    TEntry entry;
    TInt err = KErrNone;

    aFileSize=0;

    err = iFs.Entry( aFullPath, entry );

    if(KErrNone == err && !entry.IsDir())
        {
        aFileSize = entry.iSize;
        rv = TRUE;
        }

    VCXLOGLO1("<<<CVCXTestCommon::GetFileSize");
    return rv;
    }
    
// -----------------------------------------------------------------------------
// CVCXTestCommon::GetIapIdL
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CVCXTestCommon::GetIapIdL(const TDesC& aIapName, TUint32& aIapId)
    {
    VCXLOGLO1(">>>CVCXTestCommon::GetIapIdL");

    TBool found( EFalse );

#ifdef __WINSCW__
    if( aIapName != KIptvTestBadIapName && aIapName != _L("invalidiap") )
        {
        _LIT(KEmulatorIap, "Ethernet with Daemon Dynamic IP");
        found = GetIapIdByNameL(KEmulatorIap, aIapId);
        VCXLOGLO1("<<<CVCXTestCommon::GetIapIdL");
        return found;
        }
#endif
    
    if( aIapName == _L("default") )
        {
        VCXLOGLO1("CVCXTestCommon::GetIapIdL -- Searching default iap from cenrep");
        CIptvTestUtilALR* util = CIptvTestUtilALR::NewLC();
        aIapId = util->GetDefaultIapCenRep();
        CleanupStack::PopAndDestroy( util );
        if( aIapId != 0 )
            {
            VCXLOGLO2("CVCXTestCommon::GetIapIdL -- Got default from cenrep: %d", aIapId);
            return ETrue;
            }
        }
    
    if( aIapName == _L("default") )
        {
        VCXLOGLO1("CVCXTestCommon::GetIapIdL -- Getting default iap via ALR util");
        CIptvTestUtilALR* util = CIptvTestUtilALR::NewLC();
        aIapId = util->GetDefaultIap();
        CleanupStack::PopAndDestroy( util );
        if( aIapId != 0 )
            {
            VCXLOGLO2("CVCXTestCommon::GetIapIdL -- Got default: %d", aIapId);
            return ETrue;
            }
        }
    
    VCXLOGLO2("CVCXTestCommon:: Searching IAP: %S", &aIapName);

    found = GetIapIdByNameL(aIapName, aIapId);

    // Iap with exact name found
    if( found )
        {
        VCXLOGLO1("<<<CVCXTestCommon::GetIapIdL");
        return found;
        }
    else
    if(aIapName == KIptvTestBadIapName) // Search bad iap only for a name.
        {
        VCXLOGLO2("CVCXTestCommon:: Iap with name %S not found!", &aIapName);
        VCXLOGLO1("<<<CVCXTestCommon::GetIapIdL");
        return found;
        }

    VCXLOGLO1("<<<CVCXTestCommon:: Iap with exact name was not found.");

    TBuf<128> searchFor(aIapName);
    searchFor.LowerCase();

    if(searchFor == _L("invalidiap") )
        {
        aIapId = 6000;
        VCXLOGLO1("<<<CVCXTestCommon::GetIapIdL");
        return ETrue;
        }

    TBool isWlanIapName( EFalse );

    if( aIapName == _L("wlan") )
        {
        found = GetIapIdByTypeL(CVCXTestCommon::EWlanIap, aIapId, 0);
        isWlanIapName = ETrue;
        }
    else
    if( aIapName == _L("wlan2") )
        {
        found = GetIapIdByTypeL(CVCXTestCommon::EWlanIap, aIapId, 1);
        isWlanIapName = ETrue;
        }
    else
    if( aIapName == _L("internet") )
        {
        found = GetIapIdByTypeL(CVCXTestCommon::EGprsIap, aIapId, 0);
        }
    else
    if( aIapName == _L("internet2") )
        {
        found = GetIapIdByTypeL(CVCXTestCommon::EGprsIap, aIapId, 1);
        }

    if( !found && !isWlanIapName )
        {
        VCXLOGLO1("<<<CVCXTestCommon:: Trying to find alternative IAP.");

        // Try search other known GPRS IAPs
        _LIT(KIapElisaInternet, "elisa internet");
        _LIT(KIapElisaMMS, "elisa mms");
        _LIT(KIapInternet, "internet");
        _LIT(KIapProinternet, "prointernet");
        _LIT(KGprsInternet, "gprs internet");

        if( GetIapIdByNameL(KIapElisaInternet, aIapId) ||
            GetIapIdByNameL(KIapElisaMMS, aIapId) ||
            GetIapIdByNameL(KIapInternet, aIapId) ||
            GetIapIdByNameL(KIapProinternet, aIapId) ||
            GetIapIdByNameL(KGprsInternet, aIapId) )
            {
            found = ETrue;
            }

        if( !found )
            {
            VCXLOGLO1("<<<CVCXTestCommon:: Any GPRS IAP is good...");
            found = GetIapIdByTypeL(CVCXTestCommon::EGprsIap, aIapId, 0);
            }
        }
    else
        {
        VCXLOGLO1("<<<CVCXTestCommon:: WLAN IAP wanted. Not found.");
        }

    VCXLOGLO1("<<<CVCXTestCommon::GetIapIdL");
    return found;
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::GetIapIdByNameL
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CVCXTestCommon::GetIapIdByNameL(const TDesC& aIapName, TUint32& aIapId)
    {
    VCXLOGLO1(">>>CVCXTestCommon::GetIapIdByNameL");
    TBool found = EFalse;
    aIapId = 0;

    TBuf<128> searchFor(aIapName);
    searchFor.LowerCase();
    
    // Get IAP names and ids from the database
    CCommsDatabase* cdb( NULL );
    TRAPD( err, cdb = CCommsDatabase::NewL( EDatabaseTypeIAP ) );
    if( err != KErrNone )
        {
        VCXLOGLO2("CVCXTestCommon::GetIapIdByNameL: CCommsDatabase::NewL left: %d, leaving", err);
        User::Leave( err );
        }
    CleanupStack::PushL( cdb );

    cdb->ShowHiddenRecords();

    CCommsDbTableView* view( NULL );
    TRAP( err, view = cdb->OpenTableLC( TPtrC(IAP) ); CleanupStack::Pop( view ); );
    CleanupStack::PushL( view );
    
    if( err != KErrNone )
        {
        VCXLOGLO2("CVCXTestCommon::GetIapIdByNameL: CCommsDatabase::OpenTableLC left: %d, leaving", err);
        User::Leave( err );
        }
    
    TBuf<40> name;
    TUint32 id;
    TInt res = view->GotoFirstRecord();
    while( res == KErrNone )
        {
        TRAP( err, view->ReadTextL( TPtrC(COMMDB_NAME), name ) );
        if( err != KErrNone ) continue;
        TRAP( err, view->ReadUintL( TPtrC(COMMDB_ID), id ) );
        if( err != KErrNone ) continue;

        name.LowerCase();
        if(searchFor == name)
            {
            aIapId = id;
            VCXLOGLO3("CVCXTestCommon:: Found IAP: %d, %S", aIapId, &aIapName);
            found = ETrue;
            break;
            }
        res = view->GotoNextRecord();
        }

    CleanupStack::PopAndDestroy( view );
    CleanupStack::PopAndDestroy( cdb );

    VCXLOGLO1("<<<CVCXTestCommon::GetIapIdByNameL");
    return found;
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::GetIapIdByTypeL
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CVCXTestCommon::GetIapIdByTypeL(TIptvTestIapType aType, TUint32& aIapId, TInt aOrderNumber)
    {
    VCXLOGLO1(">>>CVCXTestCommon::GetIapIdByTypeL");

    aIapId = 0;
    TBool found( EFalse );

    #ifdef __WINSCW__
        found = GetIapIdByNameL( _L("Ethernet with Daemon Dynamic IP"), aIapId );
        VCXLOGLO1("<<<CVCXTestCommon::GetIapIdByTypeL");
        return found;
    #endif

    // Get AP names and ids from the database
    CCommsDatabase* TheDb = CCommsDatabase::NewL( EDatabaseTypeIAP );
    CleanupStack::PushL( TheDb );

    TheDb->ShowHiddenRecords();

    CCommsDbTableView* view = TheDb->OpenTableLC( TPtrC(IAP) );

    TBuf<40> name;
    TUint32 id;
    TBuf<255> iapType;
    TInt matchCount(0);

    TInt res = view->GotoFirstRecord();

    while( res == KErrNone )
        {
        view->ReadTextL( TPtrC(COMMDB_NAME), name );
        view->ReadUintL( TPtrC(COMMDB_ID), id );
        view->ReadTextL( TPtrC(IAP_SERVICE_TYPE), iapType);

        VCXLOGLO3("CVCXTestCommon:: name: %S, id: %d", &name, id);
        VCXLOGLO2("CVCXTestCommon:: type: %S", &iapType);

        if( ( iapType == _L("LANService") && aType == EWlanIap && name != _L("Easy WLAN") ) ||
            ( iapType == _L("OutgoingGPRS") && aType == EGprsIap ) )
            {
            if( matchCount == aOrderNumber )
                {
                found = ETrue;
                aIapId = id;
                break;
                }
            matchCount++;
            }

        res = view->GotoNextRecord();
        }

    CleanupStack::PopAndDestroy( view ); // view
    CleanupStack::PopAndDestroy( TheDb ); // TheDb

    VCXLOGLO1("<<<CVCXTestCommon::GetIapIdByTypeL");
    return found;
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::GetIapNameById
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CVCXTestCommon::GetIapNameById(TDes& aIapName, TUint32 aIapId)
    {
    TBool found = FALSE;

    // Get AP names and ids from the database
    CCommsDatabase* TheDb = CCommsDatabase::NewL( EDatabaseTypeIAP );
    CleanupStack::PushL( TheDb );

    TheDb->ShowHiddenRecords();

    CCommsDbTableView* view = TheDb->OpenTableLC( TPtrC(IAP) );

    TBuf<40> name;
    TUint32 id;
    TInt res = view->GotoFirstRecord();

    while( res == KErrNone )
        {
        view->ReadTextL( TPtrC(COMMDB_NAME), name );
        view->ReadUintL( TPtrC(COMMDB_ID), id );

        res = view->GotoNextRecord();
        //VCXLOGLO3("IAP name, id: %S, %d", &name, id);
        if(id == aIapId)
            {
            found = TRUE;
            aIapName.Zero();
            aIapName.Append(name);
            break;
            }
        }

    CleanupStack::PopAndDestroy( view ); // view
    CleanupStack::PopAndDestroy( TheDb ); // TheDb

    return found;
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::PrintIaps
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestCommon::PrintIaps()
    {
    VCXLOGLO1(">>>CVCXTestCommon::PrintIaps");
    // Get IAP names and ids from the database
    CCommsDatabase* TheDb( NULL );
    TRAPD( err, TheDb = CCommsDatabase::NewL( EDatabaseTypeIAP ) );

    if( err == KErrNone && TheDb )
        {
        CleanupStack::PushL( TheDb );

        TheDb->ShowHiddenRecords();

        CCommsDbTableView* view( NULL );
        TRAP( err, view = TheDb->OpenTableLC( TPtrC(IAP) ); CleanupStack::Pop( view ); );
        CleanupStack::PushL( view );

        if( err == KErrNone && view )
            {
            TBuf<40> name;
            TUint32 id;
            err = view->GotoFirstRecord();
            while( err == KErrNone )
                {
                TRAP( err, view->ReadTextL( TPtrC(COMMDB_NAME), name ) );
                if( err != KErrNone ) break;
                TRAP( err, view->ReadUintL( TPtrC(COMMDB_ID), id ) );
                if( err != KErrNone ) break;
                VCXLOGLO3("CVCXTestCommon:: IAP name, id: %S, %d", &name, id);

                err = view->GotoNextRecord();
                if( err != KErrNone ) break;
                }
            }
        CleanupStack::PopAndDestroy( view ); // view
        CleanupStack::PopAndDestroy( TheDb ); // TheDb
        }
    VCXLOGLO1("<<<CVCXTestCommon::PrintIaps");
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::CreateVerifyData
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::CreateVerifyData(TDesC& aVerifyId, TBool aAppend)
    {
    VCXLOGLO2(">>>CVCXTestCommon::CreateVerifyData: %S", &aVerifyId);
    TInt result = KErrNone;
    TRAPD(err, result = iIptvVerifyData->CreateVerifyDataL(aVerifyId, aAppend));
    if(err != KErrNone)
        {
        VCXLOGLO2("CIptvVerifyData::CreateVerifyDataL caused a leave. %d", err);
        result = err;
        }
    VCXLOGLO1("<<<CVCXTestCommon::CreateVerifyData");
    return result;
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::WriteVerifyData
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::WriteVerifyData(TDesC& aVerifyId, TDesC& aVerifyData)
    {
    //VCXLOGLO1(">>>CVCXTestCommon::WriteVerifyData");
    TInt result = KErrNone;
    TRAPD(err, result = iIptvVerifyData->WriteVerifyDataL(aVerifyId, aVerifyData));
    if(err != KErrNone)
        {
        VCXLOGLO2("CIptvVerifyData::WriteVerifyDataL caused a leave. %d", err);
        result = err;
        }
    //VCXLOGLO1("<<<CVCXTestCommon::WriteVerifyData");
    return result;
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::WriteVerifyData
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::WriteVerifyData(TDesC& aVerifyId, TRefByValue<const TDesC> aFmt, ... )
    {
    //VCXLOGLO1(">>>CVCXTestCommon::WriteVerifyData(formatted)");

    VA_LIST argptr; //pointer to argument list
    VA_START( argptr, aFmt );

    HBufC* str;
    str = HBufC::NewLC(1024*3);

    //TBuf<512*3> str;
    //TBuf<512> format(_L("TESTI: %d, %S"));

    str->Des().FormatList(aFmt, argptr);

    TInt result = KErrNone;
    TRAPD(err, result = iIptvVerifyData->WriteVerifyDataL(aVerifyId, *str));

    CleanupStack::PopAndDestroy(str);

    if(err != KErrNone)
        {
        VCXLOGLO2("CIptvVerifyData::WriteVerifyDataL caused a leave. %d", err);
        result = err;
        }

    //VCXLOGLO1("<<<CVCXTestCommon::WriteVerifyData(formatted)");
    return result;
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::VerifyData
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::VerifyData(TDesC& aVerifyId, CIptvTestVerifyData::TVerifyResult& aVerifyResult)
    {
    VCXLOGLO1(">>>CVCXTestCommon::VerifyData");
    TInt result = KErrNone;
    TRAPD(err, result = iIptvVerifyData->VerifyDataL(aVerifyId, aVerifyResult));
    if(err != KErrNone)
        {
        VCXLOGLO2("CIptvVerifyData::VerifyDataL caused a leave. %d", err);
        result = err;
        }
    VCXLOGLO1("<<<CVCXTestCommon::VerifyData");
    return result;
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::EnableVerifyTimestamps
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestCommon::EnableVerifyTimestamps( TBool aUseTimestamps )
    {
    VCXLOGLO1(">>>CVCXTestCommon::EnableVerifyTimestamps");
    iIptvVerifyData->EnableTimestamps( aUseTimestamps );
    VCXLOGLO1("<<<CVCXTestCommon::EnableVerifyTimestamps");
    }
   
// -----------------------------------------------------------------------------
// CVCXTestCommon::ParseIntFromString
// -----------------------------------------------------------------------------
EXPORT_C TInt CVCXTestCommon::ParseIntFromString(TInt& aInt, TDesC& aString)
    {
    VCXLOGLO1(">>>CVCXTestCommon::ParseIntFromString");

    TLex lex(aString);
    TInt err = lex.Val(aInt);

    VCXLOGLO1("<<<CVCXTestCommon::ParseIntFromString");
    return err;
    }
    
// -----------------------------------------------------------------------------
// CVCXTestCommon::CreateBadIapL
// Creates a not working, crappy iap with given name.
// Use with caution, because there is no easy way of removing created iaps
//
// @param aName name of the iap to be created
// @return iapId new iap's id.
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CVCXTestCommon::CreateBadIapL(TDesC& aName)
    {
    VCXLOGLO1(">>>CVCXTestCommon::CreateBadIapL");
    CCommsDatabase* cdb = CCommsDatabase::NewL( EDatabaseTypeIAP );
    CleanupStack::PushL( cdb );

    TBuf16<40> name;
    name.Copy( aName );

    TUint32 iapId( 0 );
    TUint32 iapServiceId( 1 );
    TUint32 id( 2 );
    TUint32 iapNetworkId( 3 );
    TUint32 weighting( 4 );
    TUint32 loc( 5 );

    User::LeaveIfError( cdb->BeginTransaction() );

    CCommsDbTableView* iapTable = cdb->OpenTableLC( TPtrC( IAP ) );

    User::LeaveIfError( iapTable->InsertRecord( iapId ) );

    iapTable->WriteTextL( TPtrC( COMMDB_NAME ), name );
    iapTable->WriteUintL( TPtrC( IAP_SERVICE ), iapServiceId );
    iapTable->WriteTextL( TPtrC( IAP_SERVICE_TYPE ), TPtrC( LAN_SERVICE ) );
    iapTable->WriteTextL( TPtrC( IAP_BEARER_TYPE ), TPtrC( LAN_BEARER ) );
    iapTable->WriteUintL( TPtrC( IAP_BEARER ), id );
    iapTable->WriteUintL( TPtrC( IAP_NETWORK ), iapNetworkId );
    iapTable->WriteUintL( TPtrC( IAP_NETWORK_WEIGHTING ), weighting );
    iapTable->WriteUintL( TPtrC( IAP_LOCATION ), loc );

    User::LeaveIfError( iapTable->PutRecordChanges( EFalse, EFalse ) );

    CleanupStack::PopAndDestroy( iapTable );

    User::LeaveIfError( cdb->CommitTransaction() );

    CleanupStack::PopAndDestroy( cdb );
    VCXLOGLO1("<<<CVCXTestCommon::CreateBadIapL");
    return iapId;
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::AdvanceSystemTimeSeconds
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::AdvanceSystemTimeSeconds( TInt aCount )
	{
	VCXLOGLO1(">>>CVCXTestCommon::AdvanceSystemTimeSeconds");

    TTime now;
    now.HomeTime();

    TTimeIntervalSeconds second(1*aCount);
    now += second;

    TInt err = CVCXTestCommon::SetSystemTime(now);

	VCXLOGLO1("<<<CVCXTestCommon::AdvanceSystemTimeSeconds");

	return err;
	}

// -----------------------------------------------------------------------------
// CVCXTestCommon::AdvanceSystemTimeMinutes
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::AdvanceSystemTimeMinutes( TInt aCount )
    {
    VCXLOGLO1(">>>CVCXTestCommon::AdvanceSystemTimeMinutes");

    TTime now;
    now.HomeTime();

    TTimeIntervalSeconds minute(1*60*aCount);
    now += minute;

    TInt err = CVCXTestCommon::SetSystemTime(now);

    VCXLOGLO1("<<<CVCXTestCommon::AdvanceSystemTimeMinutes");

    return err;
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::AdvanceSystemTimeHours
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::AdvanceSystemTimeHours( TInt aCount )
    {
    VCXLOGLO1(">>>CVCXTestCommon::AdvanceSystemTimeHours");

    TTime now;
    now.HomeTime();

    TTimeIntervalSeconds hour(1*60*60*aCount);
    now += hour;

    TInt err = CVCXTestCommon::SetSystemTime(now);

    VCXLOGLO1("<<<CVCXTestCommon::AdvanceSystemTimeHours");
    return err;
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::SetSystemTime()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::SetSystemTime(TTime aTime)
	{
	VCXLOGLO1(">>>CVCXTestCommon::SetSystemTime");

    TTime noDST(aTime);
    RTz tz;
    TInt err = tz.Connect();
    if(err != KErrNone)
    	{
    	VCXLOGLO2("RTz::Connect failed: %d", err);
    	VCXLOGLO1("<<<CVCXTestCommon::SetSystemTime");
    	return err;
    	}
    CleanupClosePushL(tz);
    err = tz.SetHomeTime(noDST);
    if(err != KErrNone)
    	{
    	VCXLOGLO2("RTz::SetHomeTime failed: %d", err);
    	VCXLOGLO1("<<<CVCXTestCommon::SetSystemTime");
    	CleanupStack::PopAndDestroy(&tz);
    	return err;
    	}

	CleanupStack::PopAndDestroy(&tz);

	VCXLOGLO1("<<<CVCXTestCommon::SetSystemTime");

	return err;
	}

// -----------------------------------------------------------------------------
// CVCXTestCommon::SetTimeZone()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::SetTimeZone(TDesC& aTimeZone)
	{
	VCXLOGLO1(">>>CVCXTestCommon::SetTimeZone");

    RTz tz;
    TInt err = tz.Connect();
    if(err != KErrNone)
    	{
    	VCXLOGLO2("RTz::Connect failed: %d", err);
    	VCXLOGLO1("<<<CVCXTestCommon::SetTimeZone");
    	return err;
    	}
    CleanupClosePushL(tz);

    TBuf8<256> timezone;
    timezone.Copy( aTimeZone ); // Conversion

    CTzId* tzId = CTzId::NewL( timezone );
    CleanupStack::PushL(tzId);

    TRAP(err, tz.SetTimeZoneL( *tzId ) );
	CleanupStack::PopAndDestroy(tzId);
	CleanupStack::PopAndDestroy(&tz);

    if(err != KErrNone)
    	{
    	VCXLOGLO2("RTz::SetTimeZone failed: %d", err);
    	VCXLOGLO1("<<<CVCXTestCommon::SetTimeZone");
    	return err;
    	}

	VCXLOGLO1("<<<CVCXTestCommon::SetTimeZone");

	return err;
	}

// -----------------------------------------------------------------------------
// CVCXTestCommon::CreateVideoFileL()
//
// Creates a video file to file system.
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestCommon::CreateVideoFileL( CIptvTestVideoCreator::TIptvTestVideoType aVideoType, TDesC& aFileName, TInt aSize )
    {
    VCXLOGLO1(">>>CVCXTestCommon::CreateVideoFileL");
    iVideoCreator->CreateVideoL( aVideoType, aFileName, aSize );
    VCXLOGLO1("<<<CVCXTestCommon::CreateVideoFileL");
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::CreateVideoFilesL()
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestCommon::CreateVideoFilesL( CIptvTestVideoCreator::TIptvTestVideoType aVideoType, TDesC& aFileName, TInt aCount, RPointerArray<HBufC>& aFileArray )
    {
    VCXLOGLO1(">>>CVCXTestCommon::CreateVideoFilesL");
    iVideoCreator->CreateVideosL( aVideoType, aFileName, aCount, aFileArray );
    VCXLOGLO1("<<<CVCXTestCommon::CreateVideoFilesL");
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::SetDriveFreeSpaceL()
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestCommon::SetDriveFreeSpaceL( TInt aDriveNumber, TUint aDesiredFreeSpace )
    {
    VCXLOGLO1(">>>CVCXTestCommon::SetDriveFreeSpaceL");

    const TInt KIptvTest1KB = 1024;
    const TInt KIptvTest1MB = KIptvTest1KB*KIptvTest1KB;

#ifdef  __WINSCW__
    if(aDriveNumber == EDriveC)
        {
        VCXLOGLO1("CVCXTestCommon:: ABORT! No C drive fill in emulator.");
        User::Leave( KErrAbort );
        }
#endif

    TChar driveLetter;
    User::LeaveIfError( RFs::DriveToChar( aDriveNumber, driveLetter ) );

    TBuf<255> path;
    path.Append( driveLetter );
    path.Append( _L(":\\data\\temp") );
    path.Append( KDoubleBacklash );

    // First check that if there's free space to fill.

    TInt64 spaceToReserve = 0;
    CalculateSpaceToReserveL(iFs, aDriveNumber, aDesiredFreeSpace, spaceToReserve);

    if( spaceToReserve == 0 )
        {
        VCXLOGLO1("<<<CVCXTestCommon::SetDriveFreeSpaceL");
        return;
        }

    // No space, try freeing by deleting existing dummy files.
    if( spaceToReserve < 0 )
        {
        VCXLOGLO1("CVCXTestCommon:: Trying to free some space.");

        for( TInt i=50; i>0; i-- )
            {
            TBuf<256> filePath;
            filePath.Zero();
            filePath.Append( path );
            filePath.AppendFormat( KDummyfileFormat, i );
            if( BaflUtils::FileExists( iFs, filePath ) )
                {
                VCXLOGLO2("CVCXTestCommon:: Deleting %S.", &filePath);
                iFs.Delete( filePath );
                CalculateSpaceToReserveL(iFs, aDriveNumber, aDesiredFreeSpace, spaceToReserve);

                // Is there's enough free space now.
                if( spaceToReserve > 0 )
                    {
                    break;
                    }
                }
            }

        // Recheck free space.
        if( spaceToReserve < 0 )
            {
            VCXLOGLO1("CVCXTestCommon:: Error! There's already less than wanted space!");
            User::Leave( KErrAbort );
            }
        }

    BaflUtils::EnsurePathExistsL(iFs, path);

    // Start creating files of 256 MB size.
    TInt64 dummyFileSize = 256 * KIptvTest1MB;

    TInt count = 0;
    while( ETrue )
        {
        // Check that there's still enough free space for the dummyfile
        CalculateSpaceToReserveL( iFs, aDriveNumber, aDesiredFreeSpace, spaceToReserve );

        if( spaceToReserve <= 0 )
            {
            VCXLOGLO1("CVCXTestCommon:: Nothing to reserve anymore.");
            break;
            }

        // Fill all at once if less than 1MB to go.
        if( spaceToReserve < KIptvTest1MB )
            {
            dummyFileSize = spaceToReserve;
            }

        // Not enough space for dummy file, find smaller size.
        while( dummyFileSize > spaceToReserve )
            {
            dummyFileSize /= 2;
            }

        // Find free filename.
        TBuf<256> filePath;
        do
            {
            filePath.Zero();
            filePath.Append( path );
            count++;
            filePath.AppendFormat(KDummyfileFormat, count);
            }
        while ( BaflUtils::FileExists( iFs, filePath ) );

        VCXLOGLO3("Creating %Ld b file to '%S'", dummyFileSize, &filePath);

        // Create the file.
        RFile file;
        TInt err = file.Replace(iFs, filePath, EFileWrite);
        if(err != KErrNone)
            {
            VCXLOGLO2("file.Replace fail: %d", err);
            }
        User::LeaveIfError(err);
        err = file.SetSize( dummyFileSize );
        if(err != KErrNone)
            {
            VCXLOGLO2("file.SetSize fail: %d", err);
            }
        User::LeaveIfError(err);
        file.Close();

        TVolumeInfo volumeInfo;
        User::LeaveIfError(iFs.Volume(volumeInfo, aDriveNumber));
        VCXLOGLO2("Free space after: %Ld b", volumeInfo.iFree);
        }

    VCXLOGLO1("<<<CVCXTestCommon::SetDriveFreeSpaceL");
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::CalculateSpaceToReserve
// -----------------------------------------------------------------------------
//
void CVCXTestCommon::CalculateSpaceToReserveL( RFs &aFs, TInt aDriveNumber, TInt aDesiredSpace, TInt64& aSpaceToReserve)
    {
    TVolumeInfo volumeInfo;
    User::LeaveIfError(aFs.Volume(volumeInfo, aDriveNumber));

    TInt64 driveSize       = volumeInfo.iSize;
    TInt64 reservedSpace   = 0;
    if(aDriveNumber == EDriveC)
        {
        reservedSpace = 1024*500; // Leave 500KB extra free to C drive
        }

    TInt64 driveFreeSpace  = volumeInfo.iFree;
    TInt64 usableSpace     = driveFreeSpace - reservedSpace;
    aSpaceToReserve        = usableSpace - aDesiredSpace;

    VCXLOGLO2("CVCXTestCommon: driveSize      = %Ld", driveSize);
    VCXLOGLO2("CVCXTestCommon: reservedSpace  = %Ld", reservedSpace);
    VCXLOGLO2("CVCXTestCommon: freeSpace      = %Ld", driveFreeSpace);
    VCXLOGLO2("CVCXTestCommon: usableSpace    = %Ld", usableSpace);
    VCXLOGLO2("CVCXTestCommon: desiredSpace   = %d", aDesiredSpace);
    VCXLOGLO2("CVCXTestCommon: spaceToReserve = %Ld", aSpaceToReserve);
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::DeleteDummyFilesL()
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestCommon::DeleteDummyFilesL( TInt aDriveNumber )
    {
    VCXLOGLO1(">>>CVCXTestCommon::DeleteDummyFilesL");

    CFileMan* fileMan = CFileMan::NewL(iFs);
    CleanupStack::PushL(fileMan);

    TBuf<3> driveLetter;
    TChar driveChar;
    RFs::DriveToChar( aDriveNumber, driveChar );
    driveLetter.Append(driveChar);

    TBuf<255>path;

    path.Format( KDummyfilePathFormat, &driveLetter );
    path.Append(KDoubleBacklash);
    fileMan->RmDir(path);

    CleanupStack::PopAndDestroy(fileMan);

    VCXLOGLO1("<<<CVCXTestCommon::DeleteDummyFilesL");
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::EnsureFileIsNotInUse()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::EnsureFileIsNotInUse( const TPtrC& aFileName )
    {
    TInt error( KErrNone );
    RFile file;
    // MDS or someone else could have the file in exclusive use. Try 6 times in 10 second intervals.
    for(TInt retry = 0; retry < 6; retry++)
        {
        error = file.Open(iFs, aFileName, EFileShareExclusive);
        if( error == KErrInUse )
            {
            User::After( 1000000 * 10 );
            }
        else
            {
            break;
            }
        }

    file.Close();
    VCXLOGLO2("CVCXTestCommon::EnsureFileIsNotInUse: %d", error);
    return error;
    }


// -----------------------------------------------------------------------------
// CVCXTestCommon::PrintDriveInfo()
// -----------------------------------------------------------------------------
//
void CVCXTestCommon::PrintDriveInfo( const TPtrC& aDriveLetter )
    {
    TInt driveNumber(-1);
    TInt64 driveSize(0);
    TInt64 driveFreeSpace(0);

    if( aDriveLetter.Length() > 0 && iFs.CharToDrive( aDriveLetter[0], driveNumber ) == KErrNone )
        {
        TVolumeInfo volumeInfo;
        if( iFs.Volume(volumeInfo, driveNumber) == KErrNone )
            {
            driveSize       = volumeInfo.iSize / 1024 / 1024;
            driveFreeSpace  = volumeInfo.iFree / 1024 / 1024;
            VCXLOGLO2("CVCXTestCommon:: drive: %S", &aDriveLetter );
            VCXLOGLO2("CVCXTestCommon::  - size: %d MB", driveSize);
            VCXLOGLO2("CVCXTestCommon::  - free: %d MB", driveFreeSpace);
            }
        }
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::ListDir
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::ListDir( const TDesC& aDir, TInt aLevel )
    {
    CDir* fileList = NULL;
    CDir* dirList = NULL;

    if( aLevel == 0 )
        {
        VCXLOGLO2("CVCXTestCommon:: Listing directory %S", &aDir);
        }

    TInt err( KErrNone );
    err = iFs.GetDir( aDir, KEntryAttMatchExclusive | KEntryAttDir, ESortByName, dirList );
    if( err == KErrNone )
        {
        err = iFs.GetDir( aDir, KEntryAttMatchExclude | KEntryAttDir, ESortByName, fileList );
        }

    CleanupStack::PushL( dirList );
    CleanupStack::PushL( fileList );

    HBufC* pre = HBufC::NewL( 256 );
    CleanupStack::PushL( pre );

    for( TInt i=0; i<aLevel; i++ )
        {
        pre->Des().Append( _L(" ") );
        }

    if( err == KErrNone )
        {
        HBufC* fileName = HBufC::NewL( 256 );
        CleanupStack::PushL( fileName );

        if( dirList )
        for ( TInt file = 0; file < dirList->Count(); file++ )
            {
            fileName->Des().Zero();
            fileName->Des().Append( aDir );
            fileName->Des().Append( (*dirList)[file].iName );
            VCXLOGLO3("%S (d) %S", pre, fileName);
            fileName->Des().Append( _L("\\") );
            fileName->Des().LowerCase();
#if 0
            ListDir( *fileName, aLevel+1 );
#else
            if( fileName->Des().Find( _L(":\\private\\") ) < 0 &&
                fileName->Des().Find( _L(":\\resource\\") ) < 0 &&
                fileName->Des().Find( _L(":\\sys\\") ) < 0 &&
                fileName->Des().Find( _L(":\\testframework\\") ) < 0 &&
                fileName->Des().Find( _L(":\\nokia\\") ) < 0 &&
                fileName->Des().Find( _L(":\\system\\") ) < 0 )
                {
                ListDir( fileName->Des(), aLevel+1 );
                }
#endif
            }

        TUint32 totalSize(0);
        if( fileList )
        for ( TInt file = 0; file < fileList->Count(); file++ )
            {
            fileName->Des().Zero();
            fileName->Des().Append( aDir );
            fileName->Des().Append( (*fileList)[file].iName );

            RFile rf;
            if( rf.Open( iFs, *fileName, EFileRead|EFileShareAny ) == KErrNone )
                {
                TInt size(0);
                if( rf.Size( size ) == KErrNone )
                    {
                    totalSize += size;
                    VCXLOGLO4("%S (f) %S - %d KB", pre, fileName, size/1024 );
                    }
                rf.Close();
                }
            else
                {
                VCXLOGLO3("%S (f) %S", pre, fileName);
                }

            }
            VCXLOGLO3("%S Total size: %d KB", pre, totalSize/1024 );
            CleanupStack::PopAndDestroy( fileName );
        }

    CleanupStack::PopAndDestroy( pre );
    CleanupStack::PopAndDestroy( fileList );
    CleanupStack::PopAndDestroy( dirList );

    if( aLevel == 0 )
        {
        VCXLOGLO1("CVCXTestCommon:: Directory listed.");
        }

    return err;
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::GetPSProperty
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::GetPSProperty( const TUid aCategory, const TUint aKey, TInt& aValue )
    {
    VCXLOGLO2("CVCXTestCommon:: RProperty::Get key %d.", aKey);
    TInt err = RProperty::Get( aCategory, aKey, aValue );
    if( err != KErrNone )
        {
        VCXLOGLO2("CVCXTestCommon:: RProperty::Get int returned error: %d.", err);
        }
    return err;
    }


// -----------------------------------------------------------------------------
// CVCXTestCommon::SetPSProperty
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::SetPSProperty( const TUid aCategory, const TUint aKey, const TInt aValue )
    {
    VCXLOGLO2("CVCXTestCommon:: RProperty::Set key %d.", aKey);
    TInt err = RProperty::Set( aCategory, aKey, aValue );
    if( err != KErrNone )
        {
        VCXLOGLO2("CVCXTestCommon:: RProperty::Set int returned error: %d.", err);
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::GetPSProperty
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::GetPSProperty( const TUid aCategory, const TUint aKey, TDes& aValue )
    {
    VCXLOGLO2("CVCXTestCommon:: RProperty::Get key %d.", aKey);
    TInt err = RProperty::Get( aCategory, aKey, aValue );
    if( err != KErrNone )
        {
        VCXLOGLO2("CVCXTestCommon:: RProperty::Get desc returned error: %d.", err);
        }
    return err;
    }


// -----------------------------------------------------------------------------
// CVCXTestCommon::SetPSProperty
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestCommon::SetPSProperty( const TUid aCategory, const TUint aKey, const TDesC& aValue )
    {
    VCXLOGLO2("CVCXTestCommon:: RProperty::Set key %d.", aKey);
    TInt err = RProperty::Set( aCategory, aKey, aValue );
    if( err != KErrNone )
        {
        VCXLOGLO2("CVCXTestCommon:: RProperty::Set desc returned error: %d.", err);
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CVCXTestCommon::ListFilesProcessesAndThreadsL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestCommon::ListFilesProcessesAndThreadsL( RFs& aFs )
    {
    RArray<TInt> fileThreadIds;
    CleanupClosePushL( fileThreadIds );
    
    RPointerArray<HBufC> fileNames;
    CleanupResetAndDestroyPushL( fileNames );

    // Get list of open files.
    CFileList* fileList = 0;
    TOpenFileScan fileScan( aFs );
    fileScan.NextL( fileList );
    
    CleanupStack::PushL( fileList );
    
    while( fileList )
        {
        for (TInt i = 0; i < fileList->Count(); ++i)
            {
            fileNames.Append( (*fileList)[i].iName.AllocL() );
            fileThreadIds.Append( fileScan.ThreadId() );
            //VCXLOGLO4("File: %S Size: %d Thread Id: %d", &(*fileList)[i].iName, (*fileList)[i].iSize, fileScan.ThreadId() );
            }
        CleanupStack::PopAndDestroy( fileList );
        fileScan.NextL( fileList );
        CleanupStack::PushL( fileList );
        }
    
    CleanupStack::PopAndDestroy( fileList );
    
    // Get list of processes and their threads.    
    TFindProcess findProcess;
    TFullName processName;
    
    TInt err( KErrNone );
    
    // List processes.
    while( findProcess.Next( processName ) == KErrNone )
        {
        RProcess process;
        err = process.Open( processName );
        if( err != KErrNone ) 
            {
            continue;            
            }
        CleanupClosePushL( process );

        TProcessId pid;
        pid = process.Id();
        TInt id = pid.Id();
        VCXLOGLO4("Process: %S, ID: %d (%x)", &processName, id, pid.Id() );
        
        TFindThread findThread;
        TFullName threadName;
        
        // List threads that belong to current process.
        while( findThread.Next( threadName ) == KErrNone )
            {
            RThread thread;
            err = thread.Open( threadName );
            if( err == KErrNone )
                {
                CleanupClosePushL( thread );
                RProcess threadProcess;
                err = thread.Process( threadProcess ) ;
                if( err == KErrNone )
                    {
                    CleanupClosePushL( threadProcess );
                    if( threadProcess.Id() == process.Id() )
                        {
                        TThreadId tid;
                        tid = thread.Id();
                        id = tid.Id();
                        VCXLOGLO4(" - Thread: %S, ID: %d (%x)", &threadName, id, tid.Id() );
                        
                        // And list open files for the thread.
                        for( TInt i = 0; i < fileThreadIds.Count(); i++ )
                            {
                            if( fileThreadIds[i] == tid.Id() )
                                {
                                VCXLOGLO2("   - %S", fileNames[i] );
                                }
                            }
                        }
                    CleanupStack::PopAndDestroy( &threadProcess );
                    }
                CleanupStack::PopAndDestroy( &thread );
                }        
            }
        
        VCXLOGLO1("---");
        CleanupStack::PopAndDestroy( &process );
        }
    
    CleanupStack::PopAndDestroy( &fileNames );
    CleanupStack::PopAndDestroy( &fileThreadIds );
    }

//  End of File
