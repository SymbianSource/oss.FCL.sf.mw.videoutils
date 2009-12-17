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
* Description:    Implementation of CR client's Server*
*/




// INCLUDE FILES
#include "RCRClient.h"
#include <ipvideo/CRTypeDefs.h>
#include <e32math.h>
#include <f32file.h>

// CONSTANTS
const TUint KDefaultMessageSlots( 4 );
const TUint KNumberOfServerStartupAttempts( 2 );

// LOCAL FUNCTION PROTOTYPES
static TInt StartServer();
static TInt CreateServerProcess();

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// StartServer 
// Returns: Error code
// -----------------------------------------------------------------------------
//
static TInt StartServer()
    {
    TFindServer findServer( KCRServerNameExe );
    TFullName name( KNullDesC );
    
    // Server already running?
    TInt result( findServer.Next( name ) );
    if ( !result )
        {
        return KErrNone;
        }    
   
    // Process
    result = CreateServerProcess();
    
    return result;    
    }

// -----------------------------------------------------------------------------
// CreateServerProcess 
//
// -----------------------------------------------------------------------------
//
static TInt CreateServerProcess()
    {
    RProcess server;
    TInt err( server.Create( KCRServerNameExe , _L( "" ) ) );
    if ( err )
        {
        return err;
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
        
    return reqStatus.Int();
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// RCRClient::RCRClient
//
// -----------------------------------------------------------------------------
//
RCRClient::RCRClient() : RSessionBase()
    {
    // None
    }

// -----------------------------------------------------------------------------
// RCRClient::Connect
//
// -----------------------------------------------------------------------------
// 
TInt RCRClient::Connect()
    {
    TInt ret( KErrNotFound );
    TInt retry( KNumberOfServerStartupAttempts );
    do
        {
        // Start
        ret = StartServer();
        if ( ret != KErrNone && ret != KErrAlreadyExists )
            {
            return ret;
            }
        
        // Subsession
        ret = CreateSession( KCRServerNameExe, Version(), KDefaultMessageSlots );
        if ( ret != KErrNotFound && ret != KErrServerTerminated )
            {
            return ret;
            }

        retry--;
        }
        while( retry >= 0 );

    return ret;
    }

// -----------------------------------------------------------------------------
// RCRClient::Close
// 
// -----------------------------------------------------------------------------
//
void RCRClient::Close()
    {
    TIpcArgs args( TIpcArgs::ENothing );
    SendReceive( ECRServCloseSession, args );
    RHandleBase::Close();
    }

// -----------------------------------------------------------------------------
// RCRClient::Version
// 
// -----------------------------------------------------------------------------
//
TVersion RCRClient::Version( void ) const
    {
    return( TVersion( KCRServMajorVersionNumber, 
                      KCRServMinorVersionNumber,
                      KCRServBuildVersionNumber ) );
    }

// End of File
