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


#ifndef IPTVTESTDOWNLOADMANAGER_H
#define IPTVTESTDOWNLOADMANAGER_H

#include <e32base.h>
#include <CommDbConnPref.h>
#include <es_sock.h>
#include <DownloadMgrClient.h>
#include <HttpDownloadMgrCommon.h>
#include "MTestUtilDownloadObserver.h"
#include "MIptvTestTimerObserver.h"

// FORWARD DECLARATIONS;
class CIptvTestTimer;

// CLASS DECLARATION


/**
* CTestUtilConnection  Controls the internet connection initiation and shutdown
*/
class CIptvTestDownloadManager : public CBase, MHttpDownloadMgrObserver, public MIptvTestTimerObserver
    {
	public:

	    /*
	     * NewL()
	     */
	    IMPORT_C static CIptvTestDownloadManager* NewL(MTestUtilDownloadObserver *aObs);

	    /*
	     * NewL()
	     */
	    IMPORT_C static CIptvTestDownloadManager* NewLC(MTestUtilDownloadObserver *aObs);

	    /*
	     * ~CTestUtilConnection()
	     */
	    IMPORT_C virtual ~CIptvTestDownloadManager();


		IMPORT_C TInt DownloadL( TDesC& aAddress, TDesC16& aOutFile, TDesC& aUserName, TDesC& aPassword, TInt32 aIapId = 0, TInt aTimeoutSeconds = 300);

		IMPORT_C void GetError(RHttpDownload& aDownload, TInt32& aDownloadError);

	private:

	   virtual void HandleDMgrEventL( RHttpDownload& aDownload, THttpDownloadEvent aEvent );

	   virtual void TimerComplete(TInt aTimerId, TInt aError);

	   void DeleteDownloadAsync( TInt aError );

	private:

		void ConstructL();

		CIptvTestDownloadManager(MTestUtilDownloadObserver *aObs);

	private:
		RHttpDownloadMgr iDownloadMgr;
		RHttpDownload* iDownload;

		MTestUtilDownloadObserver *iObserver;

		TBool iDownloadRunning;
		CIptvTestTimer* iTimer;

		TBool iTimerStartedForDownloadDeletion;
		TInt iError;
		TBool iDownloadFinishInformed;
    };


#endif // IAPCONNECT_V20ENGINE_H
