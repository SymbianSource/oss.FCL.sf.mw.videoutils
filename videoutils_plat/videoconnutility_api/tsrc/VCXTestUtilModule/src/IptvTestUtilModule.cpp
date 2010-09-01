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
#include <Stiftestinterface.h>
#include "IptvTestUtilModule.h"

#include <commdb.h>

#include "VCXTestCommon.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::CIptvTestUtilModule
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CIptvTestUtilModule::CIptvTestUtilModule(
    CTestModuleIf& aTestModuleIf ):
        CScriptBase( aTestModuleIf )
    {
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CIptvTestUtilModule::ConstructL()
    {
    iLog = CStifLogger::NewL( KIptvTestUtilModuleLogPath,
                          KIptvTestUtilModuleLogFile,
                          CStifLogger::ETxt,
                          CStifLogger::EFile,
                          EFalse );
    User::LeaveIfError( iFs.Connect() );
    iTestCommon = CVCXTestCommon::NewL();
    TRAP_IGNORE( iTestCommon->DeleteDummyFilesL( EDriveC ) );
    TRAP_IGNORE( iTestCommon->DeleteDummyFilesL( EDriveE ) );
    TRAP_IGNORE( iTestCommon->DeleteDummyFilesL( EDriveF ) );
    TRAP_IGNORE( iTestCommon->DeleteDummyFilesL( EDriveG ) );
    
    TestModuleIf().SetBehavior( CTestModuleIf::ETestLeaksHandles );
    
    CStifItemParser* nullParser( NULL );
    CreateMobilecrashWatcherL( *nullParser );    
    }

// -----------------------------------------------------------------------------
// CIptvTestUtilModule::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CIptvTestUtilModule* CIptvTestUtilModule::NewL(
    CTestModuleIf& aTestModuleIf )
    {
    CIptvTestUtilModule* self = new (ELeave) CIptvTestUtilModule( aTestModuleIf );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }

// Destructor
CIptvTestUtilModule::~CIptvTestUtilModule()
    {

    // Delete resources allocated from test methods
    Delete();

    // Delete logger
    delete iLog;

    delete iTestCommon;
    iTestCommon = NULL;

    iFs.Close();
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

// -----------------------------------------------------------------------------
// LibEntryL is a polymorphic Dll entry point.
// Returns: CScriptBase: New CScriptBase derived object
// -----------------------------------------------------------------------------
//
EXPORT_C CScriptBase* LibEntryL(
    CTestModuleIf& aTestModuleIf ) // Backpointer to STIF Test Framework
    {

    return ( CScriptBase* ) CIptvTestUtilModule::NewL( aTestModuleIf );

    }

// -----------------------------------------------------------------------------
// E32Dll is a DLL entry point function.
// Returns: KErrNone
// -----------------------------------------------------------------------------
//
#ifndef EKA2 // Hide Dll entry point to EKA2
GLDEF_C TInt E32Dll(
    TDllReason /*aReason*/) // Reason code
    {
    return(KErrNone);

    }
#endif // EKA2

//  End of File
