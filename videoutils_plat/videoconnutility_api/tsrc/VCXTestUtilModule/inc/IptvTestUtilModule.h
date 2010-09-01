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


#ifndef IPTVTESTUTILMODULE_H
#define IPTVTESTUTILMODULE_H

//  INCLUDES
#include <f32file.h>
#include <BAUTILS.H>

#include <StifLogger.h>
#include <TestScripterInternal.h>
#include <StifTestModule.h>

#include "MTestUtilConnectionObserver.h"
#include "MTestUtilDownloadObserver.h"
#include "MIptvTestTimerObserver.h"

// CONSTANTS
const TInt KConnectionTimerId = 500;

// Logging path
_LIT( KIptvTestUtilModuleLogPath, "\\logs\\testframework\\IptvTestUtilModule\\" );
// Log file
_LIT( KIptvTestUtilModuleLogFile, "IptvTestUtilModule.txt" );

// MACROS

// FUNCTION PROTOTYPES
//?type ?function_name(?arg_list);

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;
class CIptvTestUtilModule;
class CTestUtilConnection;
class CTestUtilConnectionWaiter;
class CIptvTestDownloadManager;
class CIptvTestTimer;
class CIptvTestMobilecrashWatcher;
class CIptvTestUtilALR;
class CVCXTestCommon;
class CZipFile;

// DATA TYPES
//enum ?declaration
//typedef ?declaration
//extern ?data_type;

// CLASS DECLARATION

/**
*  CIptvTestUtilModule test class for STIF Test Framework TestScripter.
*  This class should have no depencies to Video Center code.
*
*  @lib ?library
*  @since ?Series60_version
*/
NONSHARABLE_CLASS(CIptvTestUtilModule) : public CScriptBase, public MTestUtilConnectionObserver, public MTestUtilDownloadObserver, public MIptvTestTimerObserver
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CIptvTestUtilModule* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~CIptvTestUtilModule();

    public: // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    public: // Functions from base classes

        /**
        * From CScriptBase Runs a script line.
        * @since ?Series60_version
        * @param aItem Script line containing method name and parameters
        * @return Symbian OS error code
        */
        virtual TInt RunMethodL( CStifItemParser& aItem );

    	void ConnectionCreated();
    	void ConnectionClosed();
    	void ConnectionFailed();
	    void ConnectionAlreadyExists();
	    void ConnectionTimeout();

	    void DownloadFinished(TInt aError);

	    void TimerComplete(TInt aTimerId, TInt aError);

    protected:  // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    protected:  // Functions from base classes

    private:

        /**
        * C++ default constructor.
        */
        CIptvTestUtilModule( CTestModuleIf& aTestModuleIf );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        // Prohibit copy constructor if not deriving from CBase.
        // ?classname( const ?classname& );
        // Prohibit assigment operator if not deriving from CBase.
        // ?classname& operator=( const ?classname& );

        /**
        * Frees all resources allocated from test methods.
        * @since ?Series60_version
        */
        void Delete();

        /**
        * Test methods are listed below.
        */

        /**
        * Creates MobilecrashesWatcher
        * @return Symbian OS error code.
        */
        virtual TInt CreateMobilecrashWatcherL( CStifItemParser& aItem );

        /**
        * Checks if there's any new mobilecrashes since the watcher was created.
        * @param aItem Script line containing IptvTest Case ID. Required.
        * @return KErrNone if there's no new mobilecrash files, else KErrAbort.
        */
        virtual TInt CheckMobilecrashesL( CStifItemParser& aItem );

        /**
        * Checks if there's any mobilecrash files.
        * @param aItem Script line containing IptvTest Case ID. Required.
        * @return KErrNone if there's no mobilecrash files, else KErrAbort.
        */
        virtual TInt CheckMobilecrashesZeroToleranceL( CStifItemParser& aItem );

        /**
        * Writes line ">>> Case start: <parameters>" using Iptv logging methods.
        * @param aItem Script line containing IptvTest Case ID. Required.
        * @return Symbian OS error code.
        */
        virtual TInt IptvLogCaseStart( CStifItemParser& aItem );

        /**
        * Writes line "<<< Case end." using Iptv logging methods.
        * @param
        * @return Symbian OS error code.
        */
        virtual TInt IptvLogCaseEnd( CStifItemParser& aItem );

        /**
        * Writes line using Iptv logging methods.
        * @param aItem Text which is written into the log. Not required.
        * @return Symbian OS error code.
        */
        virtual TInt IptvLogWrite( CStifItemParser& aItem );

        /**
        * Opens a socket to a specified IAP.
        * ASYNC.
        * @param aItem IAP name.
        * @return Symbian OS error code.
        */
        virtual TInt ConnectL( CStifItemParser& aItem );

        /**
        * Attached to open connection specified by IAP.
        * SYNC.
        * @param aItem IAP name.
        * @return Symbian OS error code.
        */
        virtual TInt AttachL( CStifItemParser& aItem );

        /**
        * Disconnects from the currently open connection.
        * SYNC.
        * @param none
        * @return Symbian OS error code.
        */
        virtual TInt DisconnectL( CStifItemParser& aItem );

        /**
        * Terminates the currently open connection, i.e. closes it immediately.
        * The connection can be opened by this module or other application.
        * SYNC.
        * @param none
        * @return Symbian OS error code.
        */
        virtual TInt TerminateConnectionL( CStifItemParser& aItem );

        /**
        * Starts observing the connection until it's activated, Signals test script.
        * @param IAP name
        * @return Symbian OS error code.
        */
		virtual TInt WaitForConnectionActivityL( CStifItemParser& aItem );

        /**
        * Starts observing the connection until it's closed, Signals test script.
        * @param IAP name
        * @return Symbian OS error code.
        */
		virtual TInt WaitUntilConnectionIsClosed(CStifItemParser& aItem );

		/**
        * Start a download.
        */
		virtual TInt DownloadL( CStifItemParser& aItem );

        /**
        * Stop downloads.
        */
		virtual TInt StopDownloadsL( CStifItemParser& aItem );

        /**
        * Advance system time for 1 second.
        */
		virtual TInt AdvanceSystemTime1Second( CStifItemParser& aItem );

        /**
        * Advance system time for X seconds.
        */
        virtual TInt AdvanceSystemTimeSeconds( CStifItemParser& aItem );

        /**
        * Advance system time for X minute.
        */
        virtual TInt AdvanceSystemTimeMinutes( CStifItemParser& aItem );

        /**
        * Advance system time for X hour.
        */
        virtual TInt AdvanceSystemTimeHours( CStifItemParser& aItem );

        /**
        * Advance system time for specified number of days.
        */
		virtual TInt AdvanceSystemTimeDays(CStifItemParser& aItem );

        /**
        * Set the system time.
        * Must be in format YYYYMMDD:HHMMSS.MMMMMM (First day / month is 0)
        */
		virtual TInt SetSystemTime(CStifItemParser& aItem );

        /**
        * Set the system time within the ongoing day.
        *
        */
		virtual TInt SetSystemTimeToday(CStifItemParser& aItem );

        /**
        * Set the system timezone.
        */
		virtual TInt SetTimeZone(CStifItemParser& aItem );

        /**
        * Sets the default iap id value to cenrep.
        */
		virtual TInt SetDefaultIapCenRepL(CStifItemParser& aItem );
		
        /**
        * Removes the used destination cenrep value.
        */
        virtual TInt DeleteUsedDestinationCenRepL( CStifItemParser& aItem );

        /**
        * Set the used destination cenrep value for Video Center & IPTV_Engine.
        */
        virtual TInt SetUsedDestinationL( CStifItemParser& aItem );

        /**
        * Create new destination (SNAP)
        */
        virtual TInt CreateDestinationL( CStifItemParser& aItem );

        /**
        * Delete a destination (SNAP)
        */
        virtual TInt DeleteDestinationL( CStifItemParser& aItem );

        /**
        * Copy existing connection method to defined SNAP.
        */
        virtual TInt CopyMethodL( CStifItemParser& aItem );

        /**
        * Delete a connection method.
        */
        virtual TInt DeleteMethodL( CStifItemParser& aItem );

        /**
        * Set priority for a connection method.
        */
        virtual TInt SetMethodPriorityL( CStifItemParser& aItem );

        /**
        * Delete the destination when classes destructor is run.
        */
        virtual TInt DeleteDestinationAfterwardsL( CStifItemParser& aItem );

        /**
        * Sets string attribute for connection method.
        */
        virtual TInt SetConnectionMethodStringAttributeL( CStifItemParser& aItem );

        /**
        * Sets integer attribute for connection method.
        */
        virtual TInt SetConnectionMethodIntAttributeL( CStifItemParser& aItem );

        /**
        * Sets boolean attribute for connection method.
        */
        virtual TInt SetConnectionMethodBoolAttributeL( CStifItemParser& aItem );

        /**
         * CreateFileL
         */
        virtual TInt CreateFileL( CStifItemParser& aItem );

        /**
         * DeleteFileL
         */
        virtual TInt DeleteFileL( CStifItemParser& aItem );

        /*
         * CreateFolderL
         */
        virtual TInt CreateFolderL( CStifItemParser& aItem );

        /*
         * DeleteFolderL
         */
        virtual TInt DeleteFolderL( CStifItemParser& aItem );

        /*
         * LockFileL
         */
        virtual TInt LockFileL( CStifItemParser& aItem );

        /*
         * UnlockFileL
         */
        virtual TInt UnlockFileL( CStifItemParser& aItem );

        /*
         * ExtractFile
         */
        virtual TInt ExtractFileL( CStifItemParser& aItem );

        /*
         * FileExistsInZip
         */
        virtual TBool FileExistsInZip( CZipFile* aZipFile, const TDesC& aFileName );

        /**
        * Prints IAPs from commdb to debug output.
        */
		virtual void PrintIAPs();

        /**
        * Gets IAp by name.
        */
        virtual void GetIap(TDesC& aIapName, TUint32& aIapId);

        /**
        * Sets free space on drive.
        * @param aDriveLetter
        * @param aDesiredSpace, bytes
        */
        virtual TInt SetDriveFreeSpaceL( CStifItemParser& aItem );

    public:     // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    protected:  // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    private:    // Data

        // ?one_line_short_description_of_data
        //?data_declaration;

        // Reserved pointer for future extension
        //TAny* iReserved;

		CTestUtilConnection* iConnect;
		CTestUtilConnectionWaiter* iConnectionWaiter;
		TInt iRetryCount;
		CIptvTestDownloadManager* iDownload;
		CIptvTestTimer* iTimer;
		CIptvTestMobilecrashWatcher* iIptvTestMobilecrashWatcher;
		HBufC* iObservedServicePath;
		RPointerArray<HBufC> iObservedFiles;
		RArray<TTime> iFileModifiedDates;

		RPointerArray<HBufC> iToBeDeletedDestinations;

        CIptvTestUtilALR* iTestUtilALR;
        RFs iFs;
        RFile iLockedFile;
        TBool iFileIsLocked;

        TBool iCaseStarted;

        CVCXTestCommon* iTestCommon;

        TBool iDummyFilesCreated;
    };

#endif      // IPTVTESTUTILMODULE_H

// End of File
