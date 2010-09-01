/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    Implementation of IptvRTP client's API*
*/




// INCLUDE FILES
#include <ipvideo/CCRApiBase.h>
#include "CCRServerHandleSingleton.h"

// CONSTANTS
// None.

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRApiBase::CCRApiBase
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCRApiBase::CCRApiBase()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRApiBase::BaseConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRApiBase::BaseConstructL()
    {
    iSingleton = CCRServerHandleSingleton::InstanceL();
    }

// -----------------------------------------------------------------------------
// CCRApiBase::~CCRApiBase
// Destructor.
// -----------------------------------------------------------------------------
//
CCRApiBase::~CCRApiBase()
    {
    CCRServerHandleSingleton::Release();
    }

// End of File
