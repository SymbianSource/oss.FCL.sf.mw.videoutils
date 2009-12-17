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

#include <e32base.h>
#include <f32file.h>
#include <DownloadMgrClient.h>
#include <HttpDownloadMgrCommon.h>

#include "IptvTestDownloadManager.h"
#include "VCXTestLog.h"
#include "CIptvTestTimer.h"

// ========================== MEMBER FUNCTIONS ===============================

// ---------------------------------------------------------------------------
// CTestUtilConnection::NewL()
//
// Constructs CTestUtilConnection object
// ---------------------------------------------------------------------------
//
EXPORT_C CIptvTestDownloadManager* CIptvTestDownloadManager::NewL(MTestUtilDownloadObserver *aObs)
    {
    VCXLOGLO1(">>>CIptvTestDownloadManager::NewL");
    CIptvTestDownloadManager* self = NewLC(aObs);
    CleanupStack::Pop(self);
    VCXLOGLO1("<<<CIptvTestDownloadManager::NewL");
    return self;
    }

// ---------------------------------------------------------------------------
// CTestUtilConnection::NewLC()
//
// Constructs CTestUtilConnection object
// ---------------------------------------------------------------------------
//
EXPORT_C CIptvTestDownloadManager* CIptvTestDownloadManager::NewLC(MTestUtilDownloadObserver *aObs)
    {
    VCXLOGLO1(">>>CIptvTestDownloadManager::NewLC");
    CIptvTestDownloadManager* self = new (ELeave) CIptvTestDownloadManager(aObs);
    CleanupStack::PushL(self);
    self->ConstructL();
    VCXLOGLO1("<<<CIptvTestDownloadManager::NewLC");
    return self;
    }

// ---------------------------------------------------------------------------
// CTestUtilConnection::CTestUtilConnection()
//
// Constructor
// ---------------------------------------------------------------------------
//
CIptvTestDownloadManager::CIptvTestDownloadManager(MTestUtilDownloadObserver *aObs)
: iObserver(aObs)
    {
    VCXLOGLO1(">>>CIptvTestDownloadManager::CIptvTestUtilConnection");

    VCXLOGLO1("<<<CIptvTestDownloadManager::CIptvTestUtilConnection");
    }

// ---------------------------------------------------------------------------
// CTestUtilConnection::~CTestUtilConnection()
//
// Destructor
// ---------------------------------------------------------------------------
//
EXPORT_C CIptvTestDownloadManager::~CIptvTestDownloadManager()
    {
    VCXLOGLO1(">>>CIptvTestDownloadManager::~CIptvTestDownloadManager");

	iDownloadMgr.DeleteAll();
	iDownloadMgr.RemoveObserver(*this);
	iDownloadMgr.Close();

	if( iDownload )
		{
		iDownload->Delete();
		iDownload = NULL;
		}

    if( iTimer )
        {
        delete iTimer;
        iTimer = NULL;
        }

    VCXLOGLO1("<<<CIptvTestDownloadManager::~CIptvTestDownloadManager");
    }

// ---------------------------------------------------------------------------
// CIptvTestDownloadManager::ConstructL()
//
// Second phase constructor
// ---------------------------------------------------------------------------
//
EXPORT_C void CIptvTestDownloadManager::ConstructL()
    {
    VCXLOGLO1(">>>CIptvTestDownloadManager::ConstructL");

	iDownloadRunning = EFalse;
	TUid uid;
	uid.iUid = 0x101FB3E3;
	iDownloadMgr.ConnectL(uid, *this, ETrue);

    iTimer = CIptvTestTimer::NewL(*this, 0);

   	VCXLOGLO1("<<<CIptvTestDownloadManager::ConstructL");
    }


// ---------------------------------------------------------------------------
// CIptvTestDownloadManager::HandleDMgrEventL()
//
// ---------------------------------------------------------------------------
//
void CIptvTestDownloadManager::HandleDMgrEventL( RHttpDownload& aDownload, THttpDownloadEvent aEvent )
	{
	VCXLOGLO1(">>>CIptvTestDownloadManager::HandleDMgrEventL");

	TInt32 downloadId;
	aDownload.GetIntAttribute(EDlAttrId, downloadId);

	VCXLOGLO3("CIptvTestDownloadManager:: downloadId: %d, aEvent: %d", downloadId, aEvent);

    switch (aEvent.iProgressState)
        {
        case EHttpContentTypeReceived:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpContentTypeReceived");
            User::LeaveIfError(aDownload.Start());
            break;
        case EHttpProgNone:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgNone");
            break;
        case EHttpStarted:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpStarted");
            break;
        case EHttpProgCreatingConnection:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgCreatingConnection");
            break;
        case EHttpProgConnectionNeeded:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgConnectionNeeded");
            break;
        case EHttpProgConnected:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgConnected");
            break;
        case EHttpProgConnectionSuspended:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgConnectionSuspended");
            break;
        case EHttpProgDisconnected:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgDisconnected");
            break;
        case EHttpProgDownloadStarted:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgDownloadStarted");
            break;
        case EHttpContentTypeRequested:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpContentTypeRequested");
            break;
        case EHttpProgSubmitIssued:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgSubmitIssued");
            break;
        case EHttpProgResponseHeaderReceived:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgResponseHeaderReceived");
            break;
        case EHttpProgResponseBodyReceived:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgResponseBodyReceived");
            break;
        case EHttpProgRedirectedPermanently:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgRedirectedPermanently");
            break;
        case EHttpProgRedirectedTemporarily:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgRedirectedTemporarily");
            break;
        case EHttpProgDlNameChanged:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgDlNameChanged");
            break;
        case EHttpProgContentTypeChanged:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgContentTypeChanged");
            break;
        case EHttpProgCodDescriptorDownloaded:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgCodDescriptorDownloaded");
            break;
        case EHttpProgCodDownloadStarted:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgCodDownloadStarted");
            break;
        case EHttpProgCodDescriptorAccepted:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgCodDescriptorAccepted");
            break;
        case EHttpProgCodLoadEnd:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgCodLoadEnd");
            break;
        case EHttpProgSupportedMultiPart:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgSupportedMultiPart");
            break;
        case EHttpProgMovingContentFile:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgMovingContentFilep");
            break;
        case EHttpProgContentFileMoved:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpProgContentFileMoved");
            break;
        default:
            VCXLOGLO1("CIptvTestDownloadManager:: Unknown state");
            break;
        }

    switch (aEvent.iDownloadState)
        {
        case EHttpDlCompleted:
            {
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlCompleted");

			DeleteDownloadAsync( KErrNone );
            }
            break;

        case EHttpDlFailed:
            {
			TInt32 err;
			GetError( aDownload, err );
			VCXLOGLO2("CIptvTestDownloadManager:: Download failed, err: %d", err);
			if( err == KErrNone )
			    {
			    err = KErrGeneral;
			    }
			DeleteDownloadAsync( err );
            }
            break;

        case EHttpDlCreated:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlCreated");
            iDownloadRunning = ETrue;
            break;

        case EHttpDlInprogress:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlInprogress");
            break;

        case EHttpDlPaused:
            {
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlPaused");

			TInt32 err;
			GetError(aDownload, err);
			VCXLOGLO2("CIptvTestDownloadManager:: Download is paused, err: %d", err);
			if(err == KErrNone) err = KErrGeneral;

			DeleteDownloadAsync( err );
            }
	        break;

        case EHttpDlMoved:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlMoved");
            break;

        case EHttpDlMediaRemoved:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlMediaRemoved");
            break;

        case EHttpDlMediaInserted:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlMediaInserted");
            break;

        case EHttpDlPausable:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlPausable");
            break;

        case EHttpDlNonPausable:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlNonPausable");
            break;

        case EHttpDlDeleted:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlDeleted");

            if( !iDownloadFinishInformed )
                {
                VCXLOGLO1("CIptvTestDownloadManager:: Informing observer.");
                iDownloadFinishInformed = ETrue;
                iObserver->DownloadFinished( iError );
                iDownloadRunning = EFalse;
                }
            break;

        case EHttpDlAlreadyRunning:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlAlreadyRunning");
            break;

        case EHttpDlDeleting:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlDeleting");
            if( !iDownloadFinishInformed )
                {
                VCXLOGLO1("CIptvTestDownloadManager:: Informing observer.");
                iDownloadFinishInformed = ETrue;
                iObserver->DownloadFinished( iError );
                iDownloadRunning = EFalse;
                }
            break;

        case EHttpDlCancelTransaction:
            VCXLOGLO1("CIptvTestDownloadManager:: EHttpDlCancelTransaction");
            break;

        default:
            VCXLOGLO1("CIptvTestDownloadManager:: Unknown state");
            break;
        }


	VCXLOGLO1("<<<CIptvTestDownloadManager::HandleDMgrEventL");
	}

// ---------------------------------------------------------------------------
// CIptvTestDownloadManager::DownloadL()
//
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestDownloadManager::DownloadL( TDesC& aAddress, TDesC16& aOutFile, TDesC& aUserName, TDesC& aPassword, TInt32 aIapId, TInt aTimeoutSeconds)
	{
	VCXLOGLO1(">>>CIptvTestDownloadManager::DownloadL");

	TBool result;

	if( iDownload )
		{
		VCXLOGLO1("Download already ongoing.");
		VCXLOGLO1("<<<CIptvTestDownloadManager::DownloadL");
		return KErrNone;
		}

	iDownloadFinishInformed = EFalse;
	iTimerStartedForDownloadDeletion = EFalse;
	iError = KErrNone;

    iTimer->After(aTimeoutSeconds * 1000000);

    TBuf8<3000> address;
    TBuf8<256> username;
	TBuf8<256> password;

	address.Copy(aAddress);
	username.Copy(aUserName);
	password.Copy(aPassword);

	VCXLOGLO3("CIptvTestDownloadManager:: Starting downloading: %S -> %S", &aAddress, &aOutFile);
	VCXLOGLO3("CIptvTestDownloadManager:: User: %S, PW %S", &aUserName, &aPassword);
	VCXLOGLO2("CIptvTestDownloadManager:: Timeout: %d", aTimeoutSeconds);

	TInt err = KErrNone;

	err = iDownloadMgr.SetIntAttribute( EDlMgrIap, aIapId );
    if ( err != KErrNone )
        {
        VCXLOGLO2("CIptvTestDownloadManager:: Could not set AP! err: %d", err);
        return err;
        }

	RHttpDownload& download = iDownloadMgr.CreateDownloadL( address, result );

	iDownload = &download;

    err = iDownload->SetStringAttribute( EDlAttrDestFilename, aOutFile);
    if(err != KErrNone)
    	{
    	VCXLOGLO1("Setting destfilename failed.");
    	VCXLOGLO1("<<<CIptvTestDownloadManager::DownloadL");
    	return err;
    	}

    download.SetBoolAttribute(EDlAttrNoContentTypeCheck, ETrue);
    download.SetIntAttribute(EDlAttrUserData, 0);

    err = download.SetStringAttribute(EDlAttrUsername, username);
    if(err != KErrNone)
    	{
    	VCXLOGLO1("Setting download username failed.");
    	VCXLOGLO1("<<<CIptvTestDownloadManager::DownloadL");
    	return err;
    	}

    err = download.SetStringAttribute(EDlAttrPassword, password);
    if(err != KErrNone)
    	{
    	VCXLOGLO1("Setting download password failed.");
    	VCXLOGLO1("<<<CIptvTestDownloadManager::DownloadL");
    	return err;
    	}

	err = iDownload->Start();
    if(err != KErrNone)
    	{
    	VCXLOGLO1("Starting download failed.");
    	VCXLOGLO1("<<<CIptvTestDownloadManager::DownloadL");
    	return err;
    	}

	VCXLOGLO1("<<<CIptvTestDownloadManager::DownloadL");

	return err;
	}

// ---------------------------------------------------------
// CIptvTestDownloadManager::GetError
// ---------------------------------------------------------
//
EXPORT_C void CIptvTestDownloadManager::GetError(RHttpDownload& aDownload,
                                          TInt32& aDownloadError)
    {
    TInt32 errorId;
    TInt32 globalErrorId;
    aDownload.GetIntAttribute(EDlAttrErrorId, errorId);
    aDownload.GetIntAttribute(EDlAttrGlobalErrorId, globalErrorId);
    VCXLOGLO2("CIptvTestDownloadManager:: DL error ID: %d", errorId);
    VCXLOGLO2("CIptvTestDownloadManager:: DL global error ID: %d", globalErrorId);

    switch(errorId)
        {
        case EConnectionFailed:
            aDownloadError = EConnectionFailed;
            VCXLOGLO1("CIptvTestDownloadManager::  EConnectionFailed -> EIptvDlConnectionFailed");
            if(globalErrorId == KErrServerBusy)
                {
                VCXLOGLO1("CIptvTestDownloadManager:: server busy -> EIptvDlGeneral");
                aDownloadError = KErrServerBusy;
                }
            break;
        case EHttpAuthenticationFailed:
            VCXLOGLO1("CIptvTestDownloadManager::  EHttpAuthenticationFailed -> EIptvDlAuthFailed");
            aDownloadError = EHttpAuthenticationFailed;
            break;
        case EProxyAuthenticationFailed:
            VCXLOGLO1("CIptvTestDownloadManager::  EProxyAuthenticationFailed -> EIptvDlProxyAuthFailed");
            aDownloadError = EProxyAuthenticationFailed;
            break;
        case EDestFileInUse:
            VCXLOGLO1("CIptvTestDownloadManager::  EDestFileInUse -> EIptvDlDestFileInUse");
            aDownloadError = EDestFileInUse;
            break;
        case EBadUrl:
            VCXLOGLO1("CIptvTestDownloadManager::  EBadUrl -> EIptvDlBadUrl");
            aDownloadError = EBadUrl;
            break;
        case EMMCRemoved:
            VCXLOGLO1("CIptvTestDownloadManager::  EMMCRemoved -> EIptvDlMmcRemoved");
            aDownloadError = EMMCRemoved;
            break;
        case EDiskFull:
            VCXLOGLO1("CIptvTestDownloadManager::  EDiskFull -> EIptvDlDiskFull");
            aDownloadError = EDiskFull;
            break;
        case EObjectNotFound:
            VCXLOGLO1("CIptvTestDownloadManager::  EObjectNotFound -> EIptvDlContentNotFound");
            aDownloadError = EObjectNotFound;
            break;

        default:
            VCXLOGLO1("CIptvTestDownloadManager::  unknown -> EIptvDlGeneral");
            aDownloadError = KErrGeneral;
            break;
        }
    }

// ---------------------------------------------------------
// CIptvTestDownloadManager::DeleteDownloadAsync
// ---------------------------------------------------------
//
void CIptvTestDownloadManager::DeleteDownloadAsync( TInt aErrorToObserver )
    {
    VCXLOGLO1(">>>CIptvTestDownloadManager::DeleteDownloadAsync");
    if( iDownload )
        {
        iTimer->After( 1000000 ); // 1 second
        iTimerStartedForDownloadDeletion = ETrue;
        iError = aErrorToObserver;
        }
    VCXLOGLO1("<<<CIptvTestDownloadManager::DeleteDownloadAsync");
    }

void CIptvTestDownloadManager::TimerComplete( TInt aTimerId, TInt aError )
    {
    VCXLOGLO1(">>>CIptvTestDownloadManager::TimerComplete");

    VCXLOGLO3("CIptvTestDownloadManager:: aTimerId: %d, aError: %d", aTimerId, aError);

    if( iTimerStartedForDownloadDeletion )
        {
        iTimerStartedForDownloadDeletion = EFalse;
        VCXLOGLO1("IptvTestDownloadManager:: Deleting the download.");

        if( iDownload )
            {
            iDownload->Delete();
            iDownload = NULL;
            }

        // Obsever is informed at DL deletion.
        }
    else
        {
        iObserver->DownloadFinished(KErrTimedOut);
        }

    VCXLOGLO1("<<<CIptvTestDownloadManager::TimerComplete");
    }

//  End of File
