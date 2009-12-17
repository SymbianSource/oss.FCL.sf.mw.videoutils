/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    Implementation of RC client's ServiceBase*
*/




// INCLUDE FILES
#include "RCRServiceBase.h"
#include "RCRClient.h"
#include <e32math.h>

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// RCRServiceBase::RCRServiceBase
//
// -----------------------------------------------------------------------------
//
RCRServiceBase::RCRServiceBase() 
  : RSubSessionBase(),
    iClient( NULL )
    {
    // None
    }

// -----------------------------------------------------------------------------
// RCRServiceBase::Open
// 
// -----------------------------------------------------------------------------
//
TInt RCRServiceBase::Open( RCRClient& aClient )
    {
    __ASSERT_ALWAYS( aClient.Handle(), PanicClient( KErrBadHandle ) );
    iClient = &aClient;
    TIpcArgs args( TIpcArgs::ENothing );
    return CreateSubSession( aClient, ECRServCreateSubSession, args );
    }

// -----------------------------------------------------------------------------
// RCRServiceBase::Close
// 
// -----------------------------------------------------------------------------
//
void RCRServiceBase::Close()
    {
    RSubSessionBase::CloseSubSession( ECRServCloseSubSession );
    }

// -----------------------------------------------------------------------------
// RCRServiceBase::PanicClient
// 
// -----------------------------------------------------------------------------
//
void RCRServiceBase::PanicClient( TInt aFault )
    {
    _LIT( KCRClientFault, "CRApi Client Fault" );
    User::Panic( KCRClientFault, aFault );
    }

// End of File
