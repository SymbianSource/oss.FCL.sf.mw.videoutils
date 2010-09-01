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


#ifndef TESTUTILCONNECTIONWAITER_H
#define TESTUTILCONNECTIONWAITER_H

#include <e32base.h>
#include <CommDbConnPref.h>

#include <es_enum.h>
#include <es_sock.h>
#include <in_sock.h>


// FORWARD DECLARATIONS;
//class CCommDbOverrideSettings;
class MTestUtilConnectionObserver;

// CONSTANTS

// CLASS DECLARATION

/**
* CTestUtilConnectionWaiter
*/
class CTestUtilConnectionWaiter : public CActive
    {
public:

    /*
     * NewL()
     */
    IMPORT_C static CTestUtilConnectionWaiter* NewL(MTestUtilConnectionObserver* aObserver);

    /*
     * NewLC()
     */
    IMPORT_C static CTestUtilConnectionWaiter* NewLC(MTestUtilConnectionObserver* aObserver);

    /*
     * ~CTestUtilConnection()
     */
    IMPORT_C virtual ~CTestUtilConnectionWaiter();

    /*
     * WaitForConnection(TUint32 aIapId)
     *
     * Waits until connection is created. Asynchronous.
     *
     */
    IMPORT_C void WaitForConnection(TUint32 aIapId);

    /*
     * WaitUntilConnectionIsClosed(TUint32 aIapId)
     *
     * Waits until connection is closed. Asynchronous.
     *
     */
    IMPORT_C void WaitUntilConnectionIsClosed(TUint32 aIapId);

    /*
     * TBool IsConnectionActive()
     *
     * Checks if connection is active
     *
     */
    IMPORT_C TBool IsConnectionActive(TUint32 aIapId);


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
     * CTestUtilConnectionWaiter()
     */
    CTestUtilConnectionWaiter(MTestUtilConnectionObserver* aObserver);

    /*
     * ConstructL()
     */
    void ConstructL();

    enum TConnectionWaiterState
    	{
    	ENoWaitState,
    	EWaitingConnection,
    	EWaitingConnectionClosed
	    };

private:

    MTestUtilConnectionObserver* iObserver;

    TUint32 iIapId;

    RTimer iTimer;

 	RSocketServ iServ;

	TInt iCheckCount;

	TConnectionWaiterState iWaitState;

    };


#endif
