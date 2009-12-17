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


#ifndef TESTUTILCONNECTION_H
#define TESTUTILCONNECTION_H

#include <e32base.h>
#include <CommDbConnPref.h>
#include <es_sock.h>
#include <e32std.h>
#include "MTestUtilConnectionObserver.h"

// FORWARD DECLARATIONS;
//class CCommDbOverrideSettings;

// CONSTANTS
const TUint32 KUndefinedIAPid = 0x00;
const TUint32 KTimeout = 1000000;

// CLASS DECLARATION


/**
* CTestUtilConnection  Controls the internet connection initiation and shutdown
*/
class CTestUtilConnection : public CActive
    {
public:

    enum TConnectionState
    	{
    	EIdle = 0, EConnecting, EFailed, EConnected
    	};

    /*
     * NewL()
     */
    IMPORT_C static CTestUtilConnection* NewL(MTestUtilConnectionObserver* aObserver);

    /*
     * NewL()
     */
    IMPORT_C static CTestUtilConnection* NewLC(MTestUtilConnectionObserver* aObserver);

    /*
     * ~CTestUtilConnection()
     */
    IMPORT_C virtual ~CTestUtilConnection();

    /*
     * ConnectL()
     *
     * Creates a connection to IAP. ASYNC.
     *
     */
    IMPORT_C void ConnectL();

    /*
     * TerminateConnectionL()
     *
     * Terminate an active connection
     *
     */
    IMPORT_C void TerminateConnectionL();

    /*
     * SetConnectionPreferences()
     *
     * Sets connection preferences
     *
     * Params:
     * TUint aBearer    Bearer type
     * TUint32 aIapId   IAP to be connected, default KUndefinedIAPid
     *
     */
    IMPORT_C void SetConnectionPreferences(TUint aBearer, TUint32 aIapId = KUndefinedIAPid);

    /*
     * AttachL()
     *
     * Attach to active connection.
     *
     */
    IMPORT_C void AttachL();

    /*
     * GetState()
     *
     * Get connection state
     *
     * Params:
     *
     */
    IMPORT_C TConnectionState GetState();

protected: // from CActive

    /*
     * RunL()
     *
     * Handle request completion events
     *
     */
    void RunL();

    /*
     * DoCancel()
     *
     * Cancel any outstanding requests
     *
     */
    void DoCancel();

    // From MTestUtilConnectionObserver

private:

    /*
     * CTestUtilConnection()
     */
    CTestUtilConnection(MTestUtilConnectionObserver* aObserver);

    /*
     * ConstructL()
     */
    void ConstructL();

private:

    TCommDbConnPref iPref;

    // Owns
    RConnection iConnect;
    RSocketServ iSocketServ;

    TRequestStatus iTimerStatus;

    TConnectionState iConnectionState;

    MTestUtilConnectionObserver* iObserver;

    };


#endif // IAPCONNECT_V20ENGINE_H
