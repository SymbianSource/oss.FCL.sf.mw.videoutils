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



#ifndef CVCXTestCommon_H
#define CVCXTestCommon_H

//  INCLUDES
#include "CIptvTestVerifyData.h"
#include "MTestUtilConnectionObserver.h"
#include "CIptvTestVideoCreator.h"
#include "VCXTestConstants.h"

// MACROS
// FORWARD DECLARATIONS

// DATA TYPES

// FUNCTION PROTOTYPES

// CLASS DECLARATION

/**
*  This class should have no depencies to any Video Center release!
*  Add functionality with depencies to IptvTestUtility.
*
*  @since
*/
class CVCXTestCommon : public CBase, public MTestUtilConnectionObserver
    {
    public:  // Constructors and destructor

		enum TIptvTestIapType
			{
			EWlanIap,  	// Any WLAN AP
			EGprsIap, 	// Any GPRS AP
			};

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CVCXTestCommon* NewL();

        /**
        * Destructor.
        */
        virtual ~CVCXTestCommon();

    public: // New functions

        /**
        * Gets size of a file.
        * @since
        * @param
        */
        IMPORT_C TBool GetFileSize(TDesC& aFullPath, TUint32& aFileSize);

        /**
        * Searches AP by name.
        * First searches if there's IAP matchin for the name. After that following rules are used:
        * If name is Internet tries to find first GPRS AP, Internet2 second GPRS AP
        * If name is Wlan tries to find first WLAN AP, Wlan2 second WLAN AP
        * If AP is not found returns on of the known APs, if one is found.
        */
        IMPORT_C TBool GetIapIdL(const TDesC& aIapName, TUint32& aIapId);

        /**
        * Searches AP by name.
        */
		IMPORT_C TBool GetIapIdByNameL(const TDesC& aIapName, TUint32& aIapId);

        /**
        * Searches for AP as defined in TIptvTestIapType. In emulator always returns "Ethernet with Daemon Dynamic IP"
        * @since
        * @param aType AP type which is wanted
        * @param aIapId id of found AP is written here
        * @param aOrderNumber return nTh AP of the specified type
        * @return KErrNotFound if no AP with the type is found, KErrNone if no errors,
        */
		IMPORT_C TInt GetIapIdByTypeL(TIptvTestIapType aType, TUint32& aIapId, TInt aOrderNumber);

        /**
        * Returns a name of AP.
        */
        IMPORT_C TBool GetIapNameById(TDes& aIapName, TUint32 aIapId);

        /**
        * Prints APs in COMMDB into debug output.
        */
        IMPORT_C void PrintIaps();

  		/**
        * Initializes the verify file, deletes if the file exists
        * @since
        * @param aVerifyId An id to identify the file where verify data is written
        * @return Error code, KErrNone if no errors,
        */
		IMPORT_C TInt CreateVerifyData(TDesC& aVerifyId, TBool aAppend);

		/**
        * Writes verify data to a file
        * @since
        * @param aVerifyId An id to identify the file where verify data is written
        * @param aVerifyData Data to write into the file
        * @return Error code, KErrNone if no errors,
        */
		IMPORT_C TInt WriteVerifyData(TDesC& aVerifyId, TDesC& aVerifyData);

		/**
        * Writes verify data to a file
        * @since
        * @param aVerifyId An id to identify the file where verify data is written
        * @param aVerifyData Data to write into the file
        * @return Error code, KErrNone if no errors,
        */
		IMPORT_C TInt WriteVerifyData(TDesC& aVerifyId, TRefByValue<const TDesC> aFmt, ... );

		/**
        *
        * @since
        * @param aVerifyId An id to identify the files which are compared
        * @param aVerifyResult Result is written here
        * @return Error code, KErrNone if no errors,
        */
		IMPORT_C TInt VerifyData(TDesC& aVerifyId, CIptvTestVerifyData::TVerifyResult& aVerifyResult);

        /**
        * Enables timestamp writing into the verify file.
        * @since
        * @param aUseTimestamps
        */
        IMPORT_C void EnableVerifyTimestamps( TBool aUseTimestamps );

        /**
        * Converts string to integer.
		* @param aInt parsed int is stored here
        * @param aString string containing the number
        * @return KErrNone if succeed, otherwise an error code
        */
		IMPORT_C static TInt ParseIntFromString(TInt& aInt, TDesC& aString);

        /**
        * Creates bad iap for testing purposes
        * @param aName name of the iap to be created
        */
		IMPORT_C TUint32 CreateBadIapL(TDesC& aName);

        /**
        * static TInt AdvanceSystemTimeSeconds();
        * @return KErrNone if succeed, otherwise an error code
        */
        IMPORT_C static TInt AdvanceSystemTimeSeconds( TInt aCount );

        /**
        * static TInt AdvanceSystemTimeMinutes();
        * @return KErrNone if succeed, otherwise an error code
        */
        IMPORT_C static TInt AdvanceSystemTimeMinutes( TInt aCount );

        /**
        * static TInt AdvanceSystemTimeHours();
        * @return KErrNone if succeed, otherwise an error code
        */
        IMPORT_C static TInt AdvanceSystemTimeHours( TInt aCount );

        /**
        * static TInt SetSystemTime();
        * @return KErrNone if succeed, otherwise an error code
        */
		IMPORT_C static TInt SetSystemTime(TTime aTime);

        /**
        * static TInt SetTimeZone();
        * @return KErrNone if succeed, otherwise an error code
        */
		IMPORT_C static TInt SetTimeZone(TDesC& aTimeZone);

	    /*
	     * Creates a video file to the system.
	     */
	    IMPORT_C void CreateVideoFileL( CIptvTestVideoCreator::TIptvTestVideoType aVideoType, TDesC& aFileName, TInt aSize );

        /*
         * Creates video files to the system.
         */
        IMPORT_C void CreateVideoFilesL( CIptvTestVideoCreator::TIptvTestVideoType aVideoType, TDesC& aFileName, TInt aCount, RPointerArray<HBufC>& aFileArray );

	    /*
	     * Sets free space for a drive.
	     */
	    IMPORT_C void SetDriveFreeSpaceL( TInt aDriveNumber, TUint aDesiredFreeSpace );

        /**
         * Deletes files created by SetDriveFreeSpaceL. Should be called by any test module which
         * has reserved disk space.
         */
        IMPORT_C void DeleteDummyFilesL( TInt aDriveNumber );

        /*
         * Returns KErrNone if file is not in use.
         */
        IMPORT_C TInt EnsureFileIsNotInUse( const TPtrC& aFileName );

	    /*
	     * CalculateSpaceToReserveL
	     */
	    void CalculateSpaceToReserveL( RFs &aFs, TInt aDriveNumber, TInt aDesiredSpace, TInt64& aSpaceToReserve );

	    /**
	     *  PrintDriveInfo
	     */
	    void PrintDriveInfo( const TPtrC& aDriveLetter );

	    /**
	     * ListDir
	     */
	    IMPORT_C TInt ListDir( const TDesC& aDir, TInt aLevel );

        /**
        * Reads integer from P&S.
        * @return KErrNone if succeed, otherwise an error code
        */
        IMPORT_C static TInt GetPSProperty( const TUid aCategory, const TUint aKey, TInt& aValue );

        /**
        * Writes integer to P&S.
        * @return KErrNone if succeed, otherwise an error code
        */
        IMPORT_C static TInt SetPSProperty( const TUid aCategory, const TUint aKey, const TInt aValue );

        /**
        * Reads descriptor from P&S.
        * @return KErrNone if succeed, otherwise an error code
        */
        IMPORT_C static TInt GetPSProperty( const TUid aCategory, const TUint aKey, TDes& aValue );

        /**
        * Writes descriptor to P&S.
        * @return KErrNone if succeed, otherwise an error code
        */
        IMPORT_C static TInt SetPSProperty( const TUid aCategory, const TUint aKey, const TDesC& aValue );
        
        /**
         * Prints open files in the file session and which thread / process is keeping the files open.
         * @param aFs file session, for example CCoeEnv::Static()->FsSession() as param is good idea. 
         */
        IMPORT_C static void ListFilesProcessesAndThreadsL( RFs &aFs );        

    public: // From base classes

        void ConnectionCreated() {};
        void ConnectionClosed() {};
        void ConnectionFailed() {};
        void ConnectionAlreadyExists() {};
        void ConnectionTimeout() {};

    private:

        /**
        * C++ default constructor.
        */
        CVCXTestCommon();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data
	    CIptvTestVerifyData*   iIptvVerifyData;
        CIptvTestVideoCreator* iVideoCreator;
        RFs iFs;
    };


#endif      // CVCXTestCommon_H

// End of File
