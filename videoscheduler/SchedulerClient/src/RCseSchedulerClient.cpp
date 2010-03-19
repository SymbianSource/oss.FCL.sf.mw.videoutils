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
* Description:    Implementation of CseScheduler client's Server*
*/



// INCLUDE FILES
#include <ipvideo/rcseschedulerclient.h>            // Header for this class
#include "CseDebug.h"                       // Debug macros
#include <ipvideo/cseschedulerclientservercommon.h> // Common defines for client and server
#include <e32math.h>
#include <f32file.h>

// EXTERNAL DATA STRUCTURES
// None

// EXTERNAL FUNCTION PROTOTYPES  
// None

// CONSTANTS
const TUint KSchedulingDefaultMessageSlots( 4 );
const TUint KSchedulingNumberOfServerStartupAttempts( 2 );

// MACROS
// None

// LOCAL CONSTANTS AND MACROS
// None

// MODULE DATA STRUCTURES
// None

// LOCAL FUNCTION PROTOTYPES
// LOCAL FUNCTION PROTOTYPES
static TInt StartServer();
static TInt CreateServerProcess();

// FORWARD DECLARATIONS
// None

// ============================= LOCAL FUNCTIONS ===============================
// -----------------------------------------------------------------------------
// StartServer 
// Returns: Error code
// -----------------------------------------------------------------------------
//
static TInt StartServer()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerClient::StartServer");
    TFindServer findServer( KCseSchedulerServerName );
    TFullName name;
    
    // Server already running?    
    if ( !findServer.Next( name ) )
        {
        return KErrNone;
        }
        
    TInt result( CreateServerProcess() );
    
    CSELOGSTRING2_HIGH_LEVEL("<<<RCseSchedulerClient::StartServer: %d", result);
    return result;
    }

// -----------------------------------------------------------------------------
// CreateServerProcess 
//
// -----------------------------------------------------------------------------
//
static TInt CreateServerProcess()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerClient::CreateServerProcess");
    RProcess server;
    TInt result( server.Create( KCseSchedulerServerFileName , _L( "" ) ) );

    if ( result )
        {
        return  result;
        }

    // Makes the server eligible for execution.    
    TRequestStatus reqStatus;
    server.Rendezvous(reqStatus);
    
    server.Resume();
    
    User::WaitForRequest(reqStatus);
    
    if( reqStatus.Int() != KErrNone )
        {
        server.Close();
        }
    
    CSELOGSTRING2_HIGH_LEVEL("<<<RCseSchedulerClient::CreateServerProcess: %d", reqStatus.Int());
    return reqStatus.Int();
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// RCseSchedulerClient::RCseSchedulerClient
//
// -----------------------------------------------------------------------------
//
RCseSchedulerClient::RCseSchedulerClient() : RSessionBase()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerClient::RCseSchedulerClient");
    // None
    CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerClient::RCseSchedulerClient");
    }

// -----------------------------------------------------------------------------
// RCseSchedulerClient::~RCseSchedulerClient
//
// -----------------------------------------------------------------------------
//
RCseSchedulerClient::~RCseSchedulerClient()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerClient::~RCseSchedulerClient");
    CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerClient::~RCseSchedulerClient");
    }

// -----------------------------------------------------------------------------
// RCseSchedulerClient::Connect
//
// -----------------------------------------------------------------------------
// 
TInt RCseSchedulerClient::Connect()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerClient::Connect");
    TInt ret( KErrNotFound );
    TInt retry( KSchedulingNumberOfServerStartupAttempts );
    do
        {
        // Start
        ret = StartServer();
        if ( ret!=KErrNone && ret!=KErrAlreadyExists )
            {
            return ret;
            }
        
        // Subsession
        ret = CreateSession( KCseSchedulerServerName, Version(), KSchedulingDefaultMessageSlots );
        if ( ret != KErrNotFound && ret != KErrServerTerminated )
            {
            return ret;
            }

        retry--;
        }
    while( retry >= 0 );

    CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerClient::Connect");
    return ret;
    }
    
// -----------------------------------------------------------------------------
// RCseSchedulerClient::Close
// 
// -----------------------------------------------------------------------------
//
void RCseSchedulerClient::Close()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerClient::Close");

    TIpcArgs args( TIpcArgs::ENothing );
    SendReceive( ECseServCloseSession, args );
     
    RHandleBase::Close();

    CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerClient::Connect");
    }

// -----------------------------------------------------------------------------
// RCseSchedulerClient::Version
// 
// -----------------------------------------------------------------------------
//
TVersion RCseSchedulerClient::Version( void ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerClient::Version");

    return( TVersion( KCseServMajorVersionNumber, 
                      KCseServMinorVersionNumber,
                      KCseServBuildVersionNumber ) );
    }

// End of File
