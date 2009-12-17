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
* Description:    Singleton to hold server session*
*/




// INCLUDE FILES
#include "CCRServerHandleSingleton.h"

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
//
CCRServerHandleSingleton::CCRServerHandleSingleton() : iReferenceCount( 1 )
    {
    // None
    }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
//
CCRServerHandleSingleton::~CCRServerHandleSingleton()
    {
    iService.Close();
    iClient.Close();
    }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
//
void CCRServerHandleSingleton::ConstructL()
    {
    User::LeaveIfError( iClient.Connect() );
	User::LeaveIfError( iService.Open( iClient ) );
    }

// -----------------------------------------------------------------------------
// Returns the singleton instance.
// -----------------------------------------------------------------------------
//
CCRServerHandleSingleton* CCRServerHandleSingleton::InstanceL()
    {
    CCRServerHandleSingleton* self = NULL;

    TAny* tlsPtr = Dll::Tls();
    if ( tlsPtr == NULL )
        {
        self = new( ELeave ) CCRServerHandleSingleton;
        CleanupStack::PushL( self );
        self->ConstructL();
        User::LeaveIfError( Dll::SetTls( self ) );
        CleanupStack::Pop( self );
        }
    else
        {
        self = static_cast<CCRServerHandleSingleton*>( tlsPtr );
        ++self->iReferenceCount;
        }

    return self;
    }
    
// -----------------------------------------------------------------------------
// Releases the singleton instance.
// -----------------------------------------------------------------------------
//
void CCRServerHandleSingleton::Release()
    {
    TAny* tlsPtr = Dll::Tls();
    __ASSERT_DEBUG( tlsPtr != NULL, User::Panic( _L( "CCRApiBase" ), KErrNotFound ) );

    if ( tlsPtr != NULL )
        {
        CCRServerHandleSingleton* self = static_cast<CCRServerHandleSingleton*>( tlsPtr );
        if ( --self->iReferenceCount == 0 )
            {
            delete self;
            Dll::FreeTls();
            }
        }
    }

//  End of File
