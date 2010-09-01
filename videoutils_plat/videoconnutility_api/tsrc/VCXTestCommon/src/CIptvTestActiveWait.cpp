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
#include "CIptvTestActiveWait.h"
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
// CIptvTestActiveWait::
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestActiveWait* CIptvTestActiveWait::NewL()
    {
    VCXLOGLO1(">>>CIptvTestActiveWait::NewL");
    CIptvTestActiveWait* self = new (ELeave) CIptvTestActiveWait();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    VCXLOGLO1("<<<CIptvTestActiveWait::NewL");
    return self;
    }
// -----------------------------------------------------------------------------
// CIptvTestActiveWait::~CIptvTestActiveWait
// destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestActiveWait::~CIptvTestActiveWait()
    {
    VCXLOGLO1(">>>CIptvTestActiveWait::~CIptvTestActiveWait");

    ActiveWait(EActiveWaitStop);
    delete iActiveSchedulerWait;
    iActiveSchedulerWait = NULL;

    VCXLOGLO1("<<<CIptvTestActiveWait::~CIptvTestActiveWait");
    }

// -----------------------------------------------------------------------------
// CIptvTestActiveWait::~CIptvTestActiveWait
// destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestActiveWait::CIptvTestActiveWait()
    {
    VCXLOGLO1(">>>CIptvTestActiveWait::CIptvTestActiveWait");


    VCXLOGLO1("<<<CIptvTestActiveWait::CIptvTestActiveWait");
    }


// -----------------------------------------------------------------------------
// CIptvTestActiveWait::ConstructL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CIptvTestActiveWait::ConstructL()
    {
    VCXLOGLO1(">>>CIptvTestActiveWait::ConstructL");

    iActiveSchedulerWait = new (ELeave) CActiveSchedulerWait;

    VCXLOGLO1("<<<CIptvTestActiveWait::ConstructL");
    }

// -----------------------------------------------------------------------------
// CIptvTestActiveWait::ActiveWait
// -----------------------------------------------------------------------------
//
EXPORT_C void CIptvTestActiveWait::ActiveWait( TIptvTestActiveWaitCmd aActiveWaitCmd )
    {
    VCXLOGLO1(">>>CIptvTestActiveWait::ActiveWait");

  	VCXLOGLO2("aActiveWaitCmd=%d", aActiveWaitCmd );

	switch( aActiveWaitCmd )
		{
		case EActiveWaitStart:
		    {
		    if( !iActiveSchedulerWait->IsStarted() )
		        {
		        iActiveSchedulerWait->Start();
		        }
		    else
		        {
		        VCXLOGLO1("ActiveSchedulerWait already started");
		        }
		    }
			break;
		case EActiveWaitStop:
		    {
			if( iActiveSchedulerWait->IsStarted() )
				{
				iActiveSchedulerWait->AsyncStop();
				}
			else
		        {
		        VCXLOGLO1("ActiveSchedulerWait already stopped");
		        }
			}
			break;
		default:
			break;
		}

    VCXLOGLO1("<<<CIptvTestActiveWait::ActiveWait");
    }

// -----------------------------------------------------------------------------
// CIptvTestActiveWait::IsWaiting
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CIptvTestActiveWait::IsWaiting()
    {
    return iActiveSchedulerWait->IsStarted();
    }

// -----------------------------------------------------------------------------
// CIptvTestActiveWait::Start
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestActiveWait::Start( )
    {
    TInt err = KErrNotReady;
    if( !iActiveSchedulerWait->IsStarted() )
        {
        iActiveSchedulerWait->Start();
        err = KErrNone;
        }
    else
        {
        VCXLOGLO1("ActiveSchedulerWait already started");
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CIptvTestActiveWait::Stop
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestActiveWait::Stop( )
    {
    TInt err = KErrNotReady;
    if( iActiveSchedulerWait->IsStarted() )
        {
        iActiveSchedulerWait->AsyncStop();
        err = KErrNone;
        }
    else
        {
        VCXLOGLO1("ActiveSchedulerWait already stopped");
        }
    return err;
    }

//  End of File
