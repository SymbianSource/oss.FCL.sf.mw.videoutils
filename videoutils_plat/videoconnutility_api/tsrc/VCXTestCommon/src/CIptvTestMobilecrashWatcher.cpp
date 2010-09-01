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

#include <BAUTILS.H>
#include <S32FILE.H>

#include "CIptvTestMobilecrashWatcher.h"
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

_LIT(KDriveC, "C:\\");
_LIT(KDriveE, "E:\\");
_LIT(KDriveF, "F:\\");

// -----------------------------------------------------------------------------
// CIptvTestMobilecrashWatcher::
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestMobilecrashWatcher* CIptvTestMobilecrashWatcher::NewL()
    {
    VCXLOGLO1(">>>CIptvTestMobilecrashWatcher::NewL");
    CIptvTestMobilecrashWatcher* self = new (ELeave) CIptvTestMobilecrashWatcher( );
    CleanupStack::PushL(self);
    self->ConstructL(ETrue);
    CleanupStack::Pop();
    VCXLOGLO1("<<<CIptvTestMobilecrashWatcher::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CIptvTestMobilecrashWatcher::
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestMobilecrashWatcher* CIptvTestMobilecrashWatcher::NewL(TBool aPanicEnabled)
    {
    VCXLOGLO1(">>>CIptvTestMobilecrashWatcher::NewL");
    CIptvTestMobilecrashWatcher* self = new (ELeave) CIptvTestMobilecrashWatcher( );
    CleanupStack::PushL(self);
    self->ConstructL(aPanicEnabled);
    CleanupStack::Pop();
    VCXLOGLO1("<<<CIptvTestMobilecrashWatcher::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CIptvTestMobilecrashWatcher::~CIptvTestMobilecrashWatcher
// destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestMobilecrashWatcher::~CIptvTestMobilecrashWatcher()
    {
    VCXLOGLO1(">>>CIptvTestMobilecrashWatcher::~CIptvTestMobilecrashWatcher");

   	iCrashesBefore.ResetAndDestroy();
   	iCrashesNow.ResetAndDestroy();
   	iFsSession.Close();

    VCXLOGLO1("<<<CIptvTestMobilecrashWatcher::~CIptvTestMobilecrashWatcher");
    }

// -----------------------------------------------------------------------------
// CIptvTestMobilecrashWatcher::CIptvTestMobilecrashWatcher
// -----------------------------------------------------------------------------
//
CIptvTestMobilecrashWatcher::CIptvTestMobilecrashWatcher()
    {
    VCXLOGLO1(">>>CIptvTestMobilecrashWatcher::CIptvTestMobilecrashWatcher");


    VCXLOGLO1("<<<CIptvTestMobilecrashWatcher::CIptvTestMobilecrashWatcher");
    }


// -----------------------------------------------------------------------------
// CIptvTestMobilecrashWatcher::ConstructL()
// -----------------------------------------------------------------------------
//
void CIptvTestMobilecrashWatcher::ConstructL(TBool aPanicEnabled)
    {
    VCXLOGLO1(">>>CIptvTestMobilecrashWatcher::ConstructL");

	iPanicEnabled = aPanicEnabled;
	iFsSession.Connect();

    iCrashesBefore.ResetAndDestroy();
	TRAP_IGNORE( ReadMobilecrashesL(iCrashesBefore, KDriveC) );
	TRAP_IGNORE( 	ReadMobilecrashesL(iCrashesBefore, KDriveE) );
	TRAP_IGNORE( ReadMobilecrashesL(iCrashesBefore, KDriveF) );
    VCXLOGLO1("<<<CIptvTestMobilecrashWatcher::ConstructL");
    }

// -----------------------------------------------------------------------------
// CIptvTestMobilecrashWatcher::ReadMobilecrashesL()
// -----------------------------------------------------------------------------
//
EXPORT_C void CIptvTestMobilecrashWatcher::ReadMobilecrashesL( RPointerArray<HBufC>& aCrashes, const TDesC& aDrive )
	{
	VCXLOGLO1(">>>CIptvTestMobilecrashWatcher::ReadMobilecrashes");

    CDir* dirList  = NULL;

    TBuf<KMaxFileName> path(aDrive);

    if(aDrive == KDriveC)
        {
        path.Append(_L("data\\"));
        }

    path.Append(_L("*.*"));

    if( !iFsSession.IsValidDrive(EDriveE) )
        {
        User::Leave( KErrNotSupported );
        }

    if ( iFsSession.GetDir( path, KEntryAttMatchExclude|KEntryAttDir, ESortByName, dirList ) == KErrNone )
        {
        CleanupStack::PushL( dirList );

        for ( TInt file = 0; file < dirList->Count(); file++ )
            {
            HBufC* fileName = HBufC::NewL(KMaxFileName);
            CleanupStack::PushL( fileName );

            fileName->Des().Append(aDrive);
            fileName->Des().Append((*dirList)[file].iName);

            if(fileName->Des().FindF( _L("mobilecrash") ) >= 0)
                {
                CleanupStack::Pop( fileName );
                aCrashes.Append(fileName);
                }
            else
                {
                CleanupStack::PopAndDestroy( fileName );
                }
            }
        CleanupStack::PopAndDestroy( dirList );
        }

    VCXLOGLO1("<<<CIptvTestMobilecrashWatcher::ReadMobilecrashes");
	}

// -----------------------------------------------------------------------------
// CIptvTestMobilecrashWatcher::CompareMobilecrashesL()
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CIptvTestMobilecrashWatcher::CompareMobilecrashesL(RPointerArray<HBufC>& aCrashesBefore, RPointerArray<HBufC>& aCrashesNow)
	{
	VCXLOGLO1(">>>CIptvTestMobilecrashWatcher::CompareMobilecrashesL");
	TBool newCrashes = EFalse;

	TInt i = 0;
	TInt e = 0;

	VCXLOGLO2("CIptvTestMobilecrashWatcher:: crash files before: %d", aCrashesBefore.Count());
	VCXLOGLO2("CIptvTestMobilecrashWatcher:: crash files now: %d", aCrashesNow.Count());

	// remove crashesBefore from crashesNow

	// Check is the mobilecrash log in the before array
	for(e=0;e<aCrashesBefore.Count();e++)
		{
		for(i=0; i<aCrashesNow.Count(); i++)
			{
			TPtrC before = aCrashesBefore[e]->Des();
			TPtrC now = aCrashesNow[i]->Des();

			if(before == now )
				{
				HBufC* buf = aCrashesNow[i];
				aCrashesNow.Remove(i);
				delete buf;
				break;
				}
			}
		}

	if(aCrashesNow.Count() > 0)
		{
		newCrashes = ETrue;
		VCXLOGLO1("CIptvTestMobilecrashWatcher:: New mobilecrashes:");
		for(i=0;i<aCrashesNow.Count(); i++)
			{
			TPtr16 file = aCrashesNow[i]->Des();
			VCXLOGLO2("CIptvTestMobilecrashWatcher:: %S", &file);
			}
		}

	VCXLOGLO1("<<<CIptvTestMobilecrashWatcher::CompareMobilecrashesL");
	return newCrashes;
	}

// -----------------------------------------------------------------------------
// CIptvTestMobilecrashWatcher::CheckNewCrashesL()
// -----------------------------------------------------------------------------
//
EXPORT_C void CIptvTestMobilecrashWatcher::CheckNewCrashesL()
	{
	VCXLOGLO1(">>>CIptvTestMobilecrashWatcher::CheckNewCrashesL");

	iCrashesNow.ResetAndDestroy();
	TRAP_IGNORE( ReadMobilecrashesL(iCrashesNow, KDriveC) );
	TRAP_IGNORE( ReadMobilecrashesL(iCrashesNow, KDriveE) );
	TRAP_IGNORE( ReadMobilecrashesL(iCrashesNow, KDriveF) );
	if( CompareMobilecrashesL(iCrashesBefore, iCrashesNow) )
		{
		VCXLOGLO1("CIptvTestMobilecrashWatcher:: * ERROR * There's new mobilecrashes, panicing!");
		User::Panic(_L("New mobilecrash log"), KErrGeneral);
		}

	VCXLOGLO1("<<<CIptvTestMobilecrashWatcher::CheckNewCrashesL");
	}

// -----------------------------------------------------------------------------
// CIptvTestMobilecrashWatcher::ReturnNewCrashes()
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CIptvTestMobilecrashWatcher::ReturnNewCrashes(RPointerArray<HBufC>& aArray)
	{
	VCXLOGLO1(">>>CIptvTestMobilecrashWatcher::CheckNewCrashesL");

	aArray.ResetAndDestroy();
	TRAP_IGNORE( ReadMobilecrashesL(aArray, KDriveC) );
	TRAP_IGNORE( ReadMobilecrashesL(aArray, KDriveE) );
	TRAP_IGNORE( ReadMobilecrashesL(aArray, KDriveF) );
	TBool newCrashes = CompareMobilecrashesL(iCrashesBefore, aArray);
	if( newCrashes )
		{
		VCXLOGLO1("CIptvTestMobilecrashWatcher:: * ERROR * There's new mobilecrashes!");
		}

	VCXLOGLO1("<<<CIptvTestMobilecrashWatcher::CheckNewCrashesL");
	return newCrashes;
	}

// -----------------------------------------------------------------------------
// CIptvTestMobilecrashWatcher::ReturnMobileCrashCount()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestMobilecrashWatcher::ReturnMobileCrashCount()
	{
	VCXLOGLO1(">>>CIptvTestMobilecrashWatcher::ReturnMobileCrashCount");

    RPointerArray<HBufC> array;

    array.ResetAndDestroy();
    TRAP_IGNORE( ReadMobilecrashesL(array, KDriveC) );
    TRAP_IGNORE( ReadMobilecrashesL(array, KDriveE) );
    TRAP_IGNORE( ReadMobilecrashesL(array, KDriveF) );
	TInt count = array.Count();
	array.ResetAndDestroy();

	VCXLOGLO1("<<<CIptvTestMobilecrashWatcher::ReturnMobileCrashCount");
	return count;
	}

// ---------------------------------
//  End of File
