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

#include "TestUtilConnection.h"

#include "VCXTestLog.h"
#include "MTestUtilConnectionObserver.h"
#include <es_enum.h>

// ========================== MEMBER FUNCTIONS ===============================

// ---------------------------------------------------------------------------
// CTestUtilConnection::NewL()
//
// Constructs CTestUtilConnection object
// ---------------------------------------------------------------------------
//
EXPORT_C CTestUtilConnection* CTestUtilConnection::NewL(MTestUtilConnectionObserver* aObserver)
    {
    VCXLOGLO1(">>>CIptvTestUtilConnection::NewL");
    CTestUtilConnection* self = NewLC(aObserver);
    CleanupStack::Pop(self);
    VCXLOGLO1("<<<CIptvTestUtilConnection::NewL");
    return self;
    }

// ---------------------------------------------------------------------------
// CTestUtilConnection::NewLC()
//
// Constructs CTestUtilConnection object
// ---------------------------------------------------------------------------
//
EXPORT_C CTestUtilConnection* CTestUtilConnection::NewLC(MTestUtilConnectionObserver* aObserver)
    {
    VCXLOGLO1(">>>CIptvTestUtilConnection::NewLC");
    CTestUtilConnection* self = new (ELeave) CTestUtilConnection(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    VCXLOGLO1("<<<CIptvTestUtilConnection::NewLC");
    return self;
    }

// ---------------------------------------------------------------------------
// CTestUtilConnection::CTestUtilConnection()
//
// Constructor
// ---------------------------------------------------------------------------
//
CTestUtilConnection::CTestUtilConnection(MTestUtilConnectionObserver* aObserver)
: CActive(CActive::EPriorityHigh), iConnectionState(EIdle), iObserver(aObserver)
    {
    VCXLOGLO1(">>>CIptvTestUtilConnection::CIptvTestUtilConnection");
    SetConnectionPreferences(ECommDbBearerUnknown, 0);
    VCXLOGLO1("<<<CIptvTestUtilConnection::CIptvTestUtilConnection");
    }

// ---------------------------------------------------------------------------
// CTestUtilConnection::~CTestUtilConnection()
//
// Destructor
// ---------------------------------------------------------------------------
//
EXPORT_C CTestUtilConnection::~CTestUtilConnection()
    {
    VCXLOGLO1(">>>CIptvTestUtilConnection::~CIptvTestUtilConnection");
   	Cancel();

    iConnect.Close();
    iSocketServ.Close();

//	iTimer.Cancel();
//	iTimer.Close();

   	if( IsAdded() )
        {
        Deque(); // calls also Cancel()
        }

    VCXLOGLO1("<<<CIptvTestUtilConnection::~CIptvTestUtilConnection");
    }

// ---------------------------------------------------------------------------
// CTestUtilConnection::ConstructL()
//
// Second phase constructor
// ---------------------------------------------------------------------------
//
void CTestUtilConnection::ConstructL()
    {
    VCXLOGLO1(">>>CIptvTestUtilConnection::ConstructL");

   	iSocketServ.Connect();
    iConnect.Open(iSocketServ);

//    User::LeaveIfError(iTimer.CreateLocal());

	CActiveScheduler::Add(this);

   	VCXLOGLO1("<<<CIptvTestUtilConnection::ConstructL");
    }

// ---------------------------------------------------------------------------
// CTestUtilConnection::ConnectL()
//
// Start connecting to IAP.
// ---------------------------------------------------------------------------
//
EXPORT_C void CTestUtilConnection::ConnectL()
	{
	VCXLOGLO1(">>>CIptvTestUtilConnection::ConnectL");

	if(iConnectionState == CTestUtilConnection::EConnected)
		{
		VCXLOGLO1(">>>CIptvTestUtilConnection:: Already connected");
		return;
		}

	if (!IsActive())
		{
        iConnect.Start(iPref, iStatus);
        iConnectionState = CTestUtilConnection::EConnecting;

	//if the line below panics it's either because you made a request but you haven't
	//SetActive the object (pR->iStatus.iFlags&TRequestStatus::EActive==0) or you didn't set the iStatus
	//to KRequestPending (pR->iStatus.iFlags&TRequestStatus::ERequestPending==0)
//		iTimerStatus = KRequestPending;
//	    iTimer.After(iTimerStatus, KTimeout);

		SetActive();
		VCXLOGLO1(">>>CIptvTestUtilConnection:: Connecting");
        }
	else
		{
		//globalNote->ShowNoteL(EAknGlobalInformationNote, KTextNotReady);
		}
	VCXLOGLO1("<<<CIptvTestUtilConnection::ConnectL");
	}


// ---------------------------------------------------------------------------
// CTestUtilConnection::AttachL()
//
// Attach to open IAP.
// ---------------------------------------------------------------------------
//
EXPORT_C void CTestUtilConnection::AttachL()
	{
	VCXLOGLO1(">>>CIptvTestUtilConnection::AttachL");

	if(iConnectionState == CTestUtilConnection::EConnected)
		{
		VCXLOGLO1(">>>CIptvTestUtilConnection:: Already connected");
		VCXLOGLO1("<<<CIptvTestUtilConnection::AttachL");
		User::Leave(KErrAlreadyExists);
		}

	if (!IsActive())
		{
        TUint activeConnections = 0;
        User::LeaveIfError(iConnect.EnumerateConnections(activeConnections));

        TConnectionInfo info;
        TPckg<TConnectionInfo> infoPckg(info);

        TInt found = EFalse;
        for(TInt i=1;i<=activeConnections;i++)
        	{

			User::LeaveIfError(iConnect.GetConnectionInfo(i, infoPckg));

			VCXLOGLO3("CIptvTestUtilConnection:: %d vs %d.", iPref.IapId(), info.iIapId);

			if(info.iIapId == iPref.IapId())
				{
				found = ETrue;
				break;
				}

        	}
        if(!found)
        	{
			VCXLOGLO1(">>>CIptvTestUtilConnection:: No connection found to that AP.");
			VCXLOGLO1("<<<CIptvTestUtilConnection::AttachL");
        	User::Leave(KErrNotFound);
        	}

        User::LeaveIfError(iConnect.Attach(infoPckg, RConnection::EAttachTypeNormal));
        iConnectionState = CTestUtilConnection::EConnected;
        }

	VCXLOGLO1("<<<CIptvTestUtilConnection::AttachL");
	return;
	}

// ---------------------------------------------------------------------------
// CTestUtilConnection::RunL()
//
// Handle request completion events
// ---------------------------------------------------------------------------
//
void CTestUtilConnection::RunL()
	{
 	VCXLOGLO1(">>>CIptvTestUtilConnection::RunL");

	/*if(iTimerStatus.Int() == KErrNone)
		{
		VCXLOGLO1("CTestUtilConnection:: Connection timeout.");
		iObserver->ConnectionTimeout();
		iTimer.Cancel();
		}
 */
    TInt statusCode = iStatus.Int();

    switch (statusCode)
	    {
	    case KErrNone: // Connection created succesfully
	        {
	        //PrintNoteL(KTextConnectionCreated);
	        VCXLOGLO1("CTestUtilConnection:: Connection created.");
	        iConnectionState = CTestUtilConnection::EConnected;
//	        iTimer.Cancel();
	        iObserver->ConnectionCreated();
	        break;
	        }

	    case KErrNotFound: // Connection failed
	        {
	        //PrintNoteL(KTextGeneralError);
	        VCXLOGLO1("CTestUtilConnection:: Connection failed.");
	        iConnectionState = CTestUtilConnection::EFailed;
	        //iTimer.Cancel();
	        iObserver->ConnectionFailed();
	        break;
	        }

	    case KErrCancel: // Connection attempt cancelled
	        {
	        //PrintNoteL(KTextConnectionCancelled);
	        VCXLOGLO1("CTestUtilConnection:: Connection attemption canceled.");
	        iConnectionState = CTestUtilConnection::EFailed;
//	        iTimer.Cancel();
	        iObserver->ConnectionFailed();
	        break;
	        }

	    case KErrAlreadyExists: // Connection already exists
	        {
	        //PrintNoteL(KTextConnectionExists);
	        VCXLOGLO1("CTestUtilConnection:: Connection already exists.");
	        iConnectionState = CTestUtilConnection::EFailed;
//	        iTimer.Cancel();
	        iObserver->ConnectionAlreadyExists();
	        break;
	        }

	    default:
	        {
	        //TBuf<KErrorResolverMaxTextLength> errorText;
	        // Use text resolver to resolve error text
	        //errorText = iTextResolver->ResolveError(statusCode);
	        //PrintErrorL(errorText, statusCode);
	        VCXLOGLO2("CTestUtilConnection:: Unknown error: %d", statusCode);
	        iConnectionState = CTestUtilConnection::EFailed;
//	        iTimer.Cancel();
	        iObserver->ConnectionFailed();
	        break;
	        }
        }

    VCXLOGLO1("<<<CIptvTestUtilConnection::RunL");
	}

// ---------------------------------------------------------------------------
// CTestUtilConnection::DoCancel()
//
// Cancels ongoing requests
// ---------------------------------------------------------------------------
//
void CTestUtilConnection::DoCancel()
	{
	VCXLOGLO1(">>>CIptvTestUtilConnection::DoCancel");
	VCXLOGLO1("<<<CIptvTestUtilConnection::DoCancel");
    // Nothing to do here
	}


// ---------------------------------------------------------------------------
// CTestUtilConnection::TerminateConnectionL()
//
// This function should not be used in normal situation, since it terminates
// the active connection, even if there are other application using
// the connection. Calling the RConnection::Close() closes the connection.
// ---------------------------------------------------------------------------
//
EXPORT_C void CTestUtilConnection::TerminateConnectionL()
    {
    VCXLOGLO1(">>>CIptvTestUtilConnection::TermínateConnectionL");

/*
    if (iProgressNotifier->GetState() == EIAPConnectStateConnected)
        {
        // Show confirmation dialog first
        if (ShowQueryDialogL())
            {
            // RConnection::Stop() terminates connection by stopping whole
            // network interface even if there are other clients using
            // the connection. It is not recommended to use it unless it is
            // really meaningful.
            User::LeaveIfError(iConnect.Stop());

            }
        }
    else
        {
        PrintNoteL(KTextNotConnected);
        }
        */
    User::LeaveIfError(iConnect.Stop());
    VCXLOGLO1("<<<CIptvTestUtilConnection::TermínateConnectionL");
    }

CTestUtilConnection::TConnectionState CTestUtilConnection::GetState()
    {
    return iConnectionState;
    }

// ---------------------------------------------------------------------------
// CTestUtilConnection::SetConnectionPreferences(TUint aBearer,
//                                                 TBool aDialog,
//                                                 TUint32 aIapId)
//
// This function sets connection preferences.
// ---------------------------------------------------------------------------
//
EXPORT_C void CTestUtilConnection::SetConnectionPreferences(
    TUint aBearer,
    TUint32 aIapId)
    {
    VCXLOGLO1(">>>CIptvTestUtilConnection::SetConnectionPreferences");
    iPref.SetDirection(ECommDbConnectionDirectionOutgoing);
	iPref.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
    iPref.SetIapId(aIapId);
    iPref.SetBearerSet(aBearer);
    iPref.SetDirection(ECommDbConnectionDirectionOutgoing);
	VCXLOGLO1("<<<CIptvTestUtilConnection::SetConnectionPreferences");
    }

// End of file
