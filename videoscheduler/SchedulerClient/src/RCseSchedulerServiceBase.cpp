/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    Implementation of CseScheduler client's ServiceBase*
*/





// INCLUDE FILES
#include <ipvideo/RCseSchedulerServiceBase.h>       // Client common methods (open, close etc)
#include <ipvideo/RCseSchedulerClient.h>            // Client common methods (server start up etc)
#include "CseDebug.h"                       // Debug macros
#include <ipvideo/CseSchedulerClientServerCommon.h> // Common defines for client and server

// EXTERNAL DATA STRUCTURES
// None.

// EXTERNAL FUNCTION PROTOTYPES  
// None.

// CONSTANTS
// None.

// MACROS
// None.

// LOCAL CONSTANTS AND MACROS
// None.

// MODULE DATA STRUCTURES
// None.

// LOCAL FUNCTION PROTOTYPES
// None.

// FORWARD DECLARATIONS
// None.

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// RCseSchedulerServiceBase::RCseSchedulerServiceBase
//
// -----------------------------------------------------------------------------
//
RCseSchedulerServiceBase::RCseSchedulerServiceBase() 
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerServiceBase::RCseSchedulerServiceBase");
    CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerServiceBase::RCseSchedulerServiceBase");
    }

// -----------------------------------------------------------------------------
// RCseSchedulerServiceBase::~RCseSchedulerServiceBase
//
// -----------------------------------------------------------------------------
//    
RCseSchedulerServiceBase::~RCseSchedulerServiceBase() 
    {
    CSELOGSTRING_HIGH_LEVEL(">>>~RCseSchedulerServiceBase::~RCseSchedulerServiceBase");
    CSELOGSTRING_HIGH_LEVEL("<<<~RCseSchedulerServiceBase::~RCseSchedulerServiceBase");
    }

// -----------------------------------------------------------------------------
// RCseSchedulerServiceBase::Open
// 
// -----------------------------------------------------------------------------
//
TInt RCseSchedulerServiceBase::Open( RCseSchedulerClient& aClient )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerServiceBase::Open");
    __ASSERT_ALWAYS( aClient.Handle(), PanicClient( KErrBadHandle ) );
    iClient = &aClient;

    TIpcArgs args( TIpcArgs::ENothing );
    CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerServiceBase::Open");
    return CreateSubSession( aClient, ECseServCreateSubSession, args );  
    }

// -----------------------------------------------------------------------------
// RCseSchedulerServiceBase::Close
// 
// -----------------------------------------------------------------------------
//
void RCseSchedulerServiceBase::Close()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerServiceBase::Close");
    RSubSessionBase::CloseSubSession( ECseServCloseSubSession );
    CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerServiceBase::Close");
    }

// -----------------------------------------------------------------------------
// RCseSchedulerServiceBase::PanicClient
// 
// -----------------------------------------------------------------------------
//
void RCseSchedulerServiceBase::PanicClient( TInt aFault ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerServiceBase::PanicClient");
    _LIT( KCseRTPClientFault, "CseRTPApi Client Fault" );
    User::Panic( KCseRTPClientFault, aFault );
    }

// End of File
