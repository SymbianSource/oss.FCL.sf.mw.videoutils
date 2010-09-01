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


// INCLUDE FILES

#include "TestUtilConnectionWaiter.h"
#include "VCXTestLog.h"
#include "MTestUtilConnectionObserver.h"

const TInt KMillion = 1000000;

// ========================== MEMBER FUNCTIONS ===============================

// ---------------------------------------------------------------------------
// CTestUtilConnectionWaiter::NewL()
//
// Constructs CTestUtilConnectionWaiter object
// ---------------------------------------------------------------------------
//
EXPORT_C CTestUtilConnectionWaiter* CTestUtilConnectionWaiter::NewL(MTestUtilConnectionObserver* aObserver)
    {
    VCXLOGLO1(">>>CTestUtilConnectionWaiter::NewL");
    CTestUtilConnectionWaiter* self = NewLC(aObserver);
    CleanupStack::Pop(self);
    VCXLOGLO1("<<<CTestUtilConnectionWaiter::NewL");
    return self;
    }

// ---------------------------------------------------------------------------
// CTestUtilConnectionWaiter::NewLC()
//
// Constructs CTestUtilConnectionWaiter object
// ---------------------------------------------------------------------------
//
EXPORT_C CTestUtilConnectionWaiter* CTestUtilConnectionWaiter::NewLC(MTestUtilConnectionObserver* aObserver)
    {
    VCXLOGLO1(">>>CTestUtilConnectionWaiter::NewLC");
    CTestUtilConnectionWaiter* self = new (ELeave) CTestUtilConnectionWaiter(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    VCXLOGLO1("<<<CTestUtilConnectionWaiter::NewLC");
    return self;
    }

// ---------------------------------------------------------------------------
// CTestUtilConnectionWaiter::CTestUtilConnectionWaiter()
//
// Constructor
// ---------------------------------------------------------------------------
//
EXPORT_C CTestUtilConnectionWaiter::CTestUtilConnectionWaiter(MTestUtilConnectionObserver* aObserver)
: CActive(CActive::EPriorityHigh), iObserver(aObserver)
    {
    VCXLOGLO1(">>>CTestUtilConnectionWaiter::CTestUtilConnectionWaiter");

    VCXLOGLO1("<<<CTestUtilConnectionWaiter::CTestUtilConnectionWaiter");
    }

// ---------------------------------------------------------------------------
// CTestUtilConnectionWaiter::~CTestUtilConnectionWaiter()
//
// Destructor
// ---------------------------------------------------------------------------
//
EXPORT_C CTestUtilConnectionWaiter::~CTestUtilConnectionWaiter()
    {
    VCXLOGLO1(">>>CTestUtilConnectionWaiter::~CTestUtilConnectionWaiter");

   	if( IsAdded() )
        {
        Deque(); // calls also Cancel()
        }

   	iServ.Close();

   	iTimer.Close();

    VCXLOGLO1("<<<CTestUtilConnectionWaiter::~CTestUtilConnectionWaiter");
    }

// ---------------------------------------------------------------------------
// CTestUtilConnectionWaiter::ConstructL()
//
// Second phase constructor
// ---------------------------------------------------------------------------
//
void CTestUtilConnectionWaiter::ConstructL()
    {
    VCXLOGLO1(">>>CTestUtilConnectionWaiter::ConstructL");

   	// get connection for the iap

   	TInt err = iServ.Connect();
   	if(err != KErrNone)
   		{
   		VCXLOGLO2("Could not connect to socket server, %d.", err);
   		User::Leave(err);
   		}

   	User::LeaveIfError(iTimer.CreateLocal());

	CActiveScheduler::Add(this);


   	VCXLOGLO1("<<<CTestUtilConnectionWaiter::ConstructL");
    }

// ---------------------------------------------------------------------------
// CTestUtilConnectionWaiter::WaitForConnection()
//
// Start waiting the IAP to be active
// ---------------------------------------------------------------------------
//
EXPORT_C void CTestUtilConnectionWaiter::WaitForConnection(TUint32 aIapId)
	{
	VCXLOGLO1(">>>CTestUtilConnectionWaiter::WaitForConnection");

	if (!IsActive())
		{
		iWaitState = EWaitingConnection;

		iIapId = aIapId;
		iTimer.After(iStatus, KMillion);
		SetActive();
		iCheckCount = 0;

		VCXLOGLO2(">>>CTestUtilConnectionWaiter:: Waiting until connection is created for Iap: %d", aIapId);
        }
	else
		{
		VCXLOGLO1(">>>CTestUtilConnectionWaiter:: error - already ACTIVE");
		}

	VCXLOGLO1("<<<CTestUtilConnectionWaiter::WaitForConnection");
	}


// ---------------------------------------------------------------------------
// CTestUtilConnectionWaiter::WaitUntilConnectionIsClosed()
//
// Start waiting the IAP to be active
// ---------------------------------------------------------------------------
//
EXPORT_C void CTestUtilConnectionWaiter::WaitUntilConnectionIsClosed(TUint32 aIapId)
	{
	VCXLOGLO1(">>>CTestUtilConnectionWaiter::WaitUntilConnectionIsClosed");

	if (!IsActive())
		{
		iWaitState = EWaitingConnectionClosed;

		iIapId = aIapId;
		iTimer.After(iStatus, KMillion);
		SetActive();
		iCheckCount = 0;

		VCXLOGLO2(">>>CTestUtilConnectionWaiter:: Waiting until connection closed for Iap: %d", aIapId);
        }
	else
		{
		VCXLOGLO1(">>>CTestUtilConnectionWaiter:: error - already ACTIVE");
		}

	VCXLOGLO1("<<<CTestUtilConnectionWaiter::WaitUntilConnectionIsClosed");
	}

// ---------------------------------------------------------------------------
// CTestUtilConnectionWaiter::RunL()
//
// Handle request completion events
// ---------------------------------------------------------------------------
//
void CTestUtilConnectionWaiter::RunL()
	{
 	VCXLOGLO1(">>>CTestUtilConnectionWaiter::RunL");

//    TInt statusCode = iStatus.Int();

	if( iCheckCount++ == 30 )
		{
		VCXLOGLO1("CTestUtilConnectionWaiter:: Connection request timed out.");
		iObserver->ConnectionTimeout();
		iCheckCount = 0;
		Cancel();
		}
    else
        {
    	switch(iWaitState)
    		{
    		case EWaitingConnection:
    			{
    			if(IsConnectionActive(iIapId))
    				{
    				iObserver->ConnectionCreated();
    				iWaitState = ENoWaitState;
    				VCXLOGLO1("Connection is active!");
    				}
    		    else
    		    	{
    		    	SetActive();
    				iTimer.After(iStatus, KMillion);
    		    	}
    			}
    			break;

    		case EWaitingConnectionClosed:
    			{
    			if(!IsConnectionActive(iIapId))
    				{
    				iObserver->ConnectionClosed();
    				iWaitState = ENoWaitState;
    				VCXLOGLO1("Connection is closed!");
    				}
    			else
    				{
    		    	SetActive();
    				iTimer.After(iStatus, KMillion);
    				}
    			}
    			break;

    		default:
    			break;
    		}
    	}

    VCXLOGLO1("<<<CTestUtilConnectionWaiter::RunL");

	}

// ---------------------------------------------------------------------------
// CTestUtilConnectionWaiter::DoCancel()
//
// Cancels ongoing requests
// ---------------------------------------------------------------------------
//
void CTestUtilConnectionWaiter::DoCancel()
	{
	VCXLOGLO1(">>>CTestUtilConnectionWaiter::DoCancel");
	iTimer.Cancel();
	VCXLOGLO1("<<<CTestUtilConnectionWaiter::DoCancel");
    }

// ---------------------------------------------------------------------------
// CTestUtilConnectionWaiter::IsConnectionActive(TUint32 aIapId)
//
// Checks if IAP specified in iWaitConnectionForIapId is active
// ---------------------------------------------------------------------------
EXPORT_C TBool CTestUtilConnectionWaiter::IsConnectionActive(TUint32 aIapId)
	{
	VCXLOGLO1(">>>CTestUtilConnectionWaiter::IsConnectionActive");

    RConnection connection;

	TUint count;

 	TInt err = connection.Open(iServ, KAfInet);

 	if(err != KErrNone)
 		{
 		iServ.Close();
   		VCXLOGLO2("CTestUtilConnectionWaiter::Could not connect to open connection: %d.", err);
   		User::Leave(err);
 		}

    err = connection.EnumerateConnections(count);
 	if(err != KErrNone)
 		{
 		connection.Close();
 		iServ.Close();
   		VCXLOGLO2("CTestUtilConnectionWaiter::Could not enum connections, %d.", err);
   		User::Leave(err);
 		}

    for(; count; --count)
        {
        TPckgBuf<TConnectionInfo> connInfo;

        if(connection.GetConnectionInfo(count, connInfo) == KErrNone)
            {
            VCXLOGLO2("CTestUtilConnectionWaiter:: Connection IAP id: %d", connInfo().iIapId);

            if(aIapId == connInfo().iIapId)
            	{
            	VCXLOGLO1("CTestUtilConnectionWaiter:: IAP is connected.");
            	VCXLOGLO1("<<<CTestUtilConnectionWaiter::IsConnectionActive");
            	connection.Close();
				return ETrue;
            	}
            }
        }

 	connection.Close();

	VCXLOGLO1("<<<CTestUtilConnectionWaiter::IsConnectionActive");
    return EFalse;
	}


// End of file
