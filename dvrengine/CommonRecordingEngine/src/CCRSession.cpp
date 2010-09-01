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
* Description:    Implementation of RBF server's CCRSession class*
*/




// INCLUDE FILES
#include "CCRSession.h"
#include "CCRServer.h"
#include "CCREngine.h"
#include <ipvideo/CRTypeDefs.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRSession::CCRSession()
// C++ constructor
// -----------------------------------------------------------------------------
//
CCRSession::CCRSession() : CSession2(), iResourceCount( 0 )
    {
    __DECLARE_NAME( _S( "CCRSession" ) );
    }

// -----------------------------------------------------------------------------
// CCRSession::NewL()
// Two-phased constructor. 
// -----------------------------------------------------------------------------
//
CCRSession* CCRSession::NewL( CCRServer* aServer )
    {
    CCRSession* self = new( ELeave ) CCRSession();
    CleanupStack::PushL( self );
    self->ConstructL( aServer );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRSession::ConstructL()
// second-phase C++ constructor
// -----------------------------------------------------------------------------
//
void CCRSession::ConstructL( CCRServer* aServer )
    {
    LOG( "CCRSession::ConstructL" );

    aServer->Inc();
    // Create new object index
    iObjects = CObjectIx::NewL();
    }
    
// -----------------------------------------------------------------------------
// CCRSession::~CCRSession()
// Destructor.
// -----------------------------------------------------------------------------
//
CCRSession::~CCRSession()
    {
    LOG( "CCRSession::~CCRSession" );
    
    // Deletes objects
    delete iObjects;
    }

// -----------------------------------------------------------------------------
// CCRSession::ServiceL()
// -----------------------------------------------------------------------------
//
void CCRSession::ServiceL( const RMessage2& aMessage )
    {
    switch ( aMessage.Function() )
        {
        case ECRServCreateSubSession:
        case ECRServCloseSubSession:
        case ECRServCloseSession:    
            {
            LOG1( "CCRSession::ServiceL(), aMessage: %d", aMessage.Function() );
            TRAPD( err, DispatchMessageL( aMessage ) );
            aMessage.Complete( err );
            }
            break;

        default:
            {
            // Ok, but must be subsession relative
            CCREngine* engine = ( CCREngine* )iObjects->At( aMessage.Int3() );
            if ( engine == NULL )
                {    
                LOG( "CCRSession::ServiceL(), Null engine !" );
                aMessage.Complete( KErrBadHandle );
                PanicClient( KErrBadHandle );
                }
            else
                {
                // this is the normal route, 
                // all engine commands go this way
                engine->GeneralServiceL( aMessage );
                }
            }
            break;
        }
    }

// -----------------------------------------------------------------------------
// CCRSession::DispatchMessageL()
// -----------------------------------------------------------------------------
//
void CCRSession::DispatchMessageL( const RMessage2& aMessage )
    {
    // Check for session-relative requests
    switch ( aMessage.Function() )
        {
        case ECRServCreateSubSession:
            NewObjectL( aMessage );
            break;

        case ECRServCloseSubSession:
            DeleteObject( aMessage.Int3() );
            break;
        
        case ECRServCloseSession:
            Server()->Dec();
            break;
        
        default:
            // None
            break;
        }
    }

// -----------------------------------------------------------------------------
// CCRSession::NewObjectL()
//
// -----------------------------------------------------------------------------
//
void CCRSession::NewObjectL( const RMessage2& aMessage )
    {
    LOG( "CCRSession::NewObjectL() in" );

    CObject* obj = Server()->GetEngineObjectL();
    TInt handle( iObjects->AddL( obj ) );
    LOG1( "CCRSession::NewObjectL(), handle: %d", handle );
    
    // Write the handle to client
    TPckg<TInt> handlePckg( handle );
    TRAPD( err, aMessage.WriteL( 3, handlePckg ) );
    if ( err )
        {
        PanicClient( KErrBadDescriptor );
        return;
        }
        
    // Notch up another resource
    iResourceCount++;
    LOG1( "CCRSession::NewObjectL() out, iResourceCount: %d", iResourceCount );
    }

// -----------------------------------------------------------------------------
// CCRSession::DeleteObject()
// -----------------------------------------------------------------------------
//
void CCRSession::DeleteObject( TUint aHandle )
    {
    LOG1( "CCRSession::DeleteObject() in, aHandle: %u", aHandle );

    // Panic if bad handle
    CCREngine* engine = ( CCREngine* )iObjects->At( aHandle );
    if ( engine == NULL )
        {
        PanicClient( KErrNotFound ); 
        }
        
    // Deletes engine
    iResourceCount--;
    iObjects->Remove( aHandle );

    LOG1( "CCRSession::DeleteObject() out, iResourceCount: %d", iResourceCount );
    }

// -----------------------------------------------------------------------------
// CCRSession::CountResources()
//
// -----------------------------------------------------------------------------
//
TInt CCRSession::CountResources()
    {
    return iResourceCount;
    }

// -----------------------------------------------------------------------------
// CCRSession::PanicClient()
// -----------------------------------------------------------------------------
//
void CCRSession::PanicClient( TInt aPanic ) const
    {
    _LIT( KTxtSessionPanic, "RbfSession" );
    User::Panic( KTxtSessionPanic, aPanic );
    }

//  End of File
