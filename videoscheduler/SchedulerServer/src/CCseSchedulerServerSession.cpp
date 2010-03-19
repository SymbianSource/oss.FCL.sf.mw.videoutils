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
* Description:    Implementation of Scheduler server's*
*/




// INCLUDE FILES
#include "CCseSchedulerServerSession.h"         // Header file for this class
#include "CCseSchedulerServer.h"                // Common methods for server
#include "CCseSchedulerServerEngine.h"          // Intelligence aka engine of server
#include "CseSchedulerServer.pan"               // Server panic codes
#include <ipvideo/cseschedulerclientservercommon.h>     // Common defines for client and server
#include "CseDebug.h"                           // Debug macros

// EXTERNAL DATA STRUCTURES
// None

// EXTERNAL FUNCTION PROTOTYPES  
// None

// CONSTANTS
// None

// MACROS
// None

// LOCAL CONSTANTS AND MACROS
// None

// MODULE DATA STRUCTURES
// None

// LOCAL FUNCTION PROTOTYPES
// None

// FORWARD DECLARATIONS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCseSchedulerSession::CCseSchedulerSession()
// C++ constructor
// -----------------------------------------------------------------------------
//

CCseSchedulerServerSession::CCseSchedulerServerSession() : CSession2(),
    iResourceCount( 0 )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerSession::CCseSchedulerServerSession");
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerSession::CCseSchedulerServerSession");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerSession::NewL()
// Two-phased constructor. 
// -----------------------------------------------------------------------------
//
CCseSchedulerServerSession* CCseSchedulerServerSession::NewL( CCseSchedulerServer* aServer )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerSession::NewL");
    
    CCseSchedulerServerSession* self = new ( ELeave ) CCseSchedulerServerSession();
    CleanupStack::PushL( self );
    self->ConstructL( aServer );
    CleanupStack::Pop( self );
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerSession::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerSession::ConstructL()
// second-phase C++ constructor
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerSession::ConstructL( CCseSchedulerServer* aServer )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerSession::ConstructL");
    
	if ( aServer )
		{
		aServer->Inc();	
		}

    // Create new object index
    iObjects = CObjectIx::NewL();
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerSession::ConstructL");
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerSession::~CCseSchedulerSession()
// Destructor.
// -----------------------------------------------------------------------------
//
CCseSchedulerServerSession::~CCseSchedulerServerSession()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerSession::~CCseSchedulerServerSession");
    
    delete iObjects;
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerSession::~CCseSchedulerServerSession");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerSession::ServiceL()
// 
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerSession::ServiceL( const RMessage2& aMessage )
    {    
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerSession::ServiceL");
    
    switch ( aMessage.Function() )
        {
        case ECseServCreateSubSession:
        case ECseServCloseSubSession:
        case ECseServCloseSession:    
            {
            // "Common messages"
            TRAPD( err, DispatchMessageL( aMessage ) );
            aMessage.Complete( err );
            }
            break;

        default:
            {
            // Ok, but must be subsession relative
            CCseSchedulerServerEngine* engine = 
				(CCseSchedulerServerEngine*)iObjects->At( aMessage.Int3() );
            if ( !engine )
                {    
                aMessage.Complete( KErrBadHandle );
                PanicClient( ECsePanicBadSubSessionHandle );
                }
            else
                {
                // this is the normal route, 
                // all engine commands go this way
                TRAPD( error, engine->GeneralServiceL( aMessage ) );
				if ( error != KErrNone )
					{
					CSELOGSTRING2_HIGH_LEVEL("CCseSchedulerServerSession::ServiceL - engine->GeneralServiceL() LEAVED: %d", error);
					aMessage.Complete( error );
					}
                }
            }
            break;
        }
        
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerSession::ServiceL");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServerSession::DispatchMessageL()
// 
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerSession::DispatchMessageL( const RMessage2& aMessage )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerSession::DispatchMessageL");
    
    // Check for session-relative requests
    switch ( aMessage.Function() )
        {
        case ECseServCreateSubSession:
            NewObjectL( aMessage );
            break;

        case ECseServCloseSubSession:
            DeleteObject( aMessage.Int3() );
            break;
        
        case ECseServCloseSession:
            Server()->Dec();
            break;
        
        default:
            break;
        }
        
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerSession::DispatchMessageL");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServerSession::NewObjectL()
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerSession::NewObjectL( const RMessage2& aMessage )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerSession::NewObjectL");
    
    CObject* obj = Server()->GetEngineObjectL();
    TInt handle( iObjects->AddL( obj ) );
    // Write the handle to client
    TPckg<TInt> handlePckg( handle );
    TRAPD( err, aMessage.WriteL( 3, handlePckg ) );
    
    if ( err )
        {
        PanicClient( ECsePanicBadRequest );
        return;
        }
        
    // Notch up another resource
    iResourceCount++;
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerSession::NewObjectL");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerSession::DeleteObject()
// 
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerSession::DeleteObject( TUint aHandle )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerSession::DeleteObject");
    
    // Panic if bad handle
    CCseSchedulerServerEngine* engine = ( CCseSchedulerServerEngine* )iObjects->At( aHandle );
    if ( !engine )
        {
        PanicClient( ECsePanicBadSubSessionHandle ); 
        }
        
    iResourceCount--;
    // Deletes engine
    iObjects->Remove( aHandle );

	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerSession::DeleteObject");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServerSession::CountResources()
//
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerServerSession::CountResources()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerSession::CountResources");
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerSession::CountResources");
    return iResourceCount;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerSession::PanicClient()
// 
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerSession::PanicClient( TCseSchedulerServPanic aPanic ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerSession::PanicClient");
    
    _LIT( KTxtSessionPanic, "SchedulerSession" );
    CSELOGSTRING2_HIGH_LEVEL("CCseSchedulerServerSession::PanicClient - panic code: %d", aPanic);
    User::Panic( KTxtSessionPanic, aPanic );
    }

// -----------------------------------------------------------------------------
// CCseSchedulerSession::PanicClient()
// 
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//    
CCseSchedulerServer* CCseSchedulerServerSession::Server()
    {
    return reinterpret_cast< CCseSchedulerServer* > ( 
                   const_cast< CServer2* > ( CSession2::Server( ) ) );
    }
//  End of File
