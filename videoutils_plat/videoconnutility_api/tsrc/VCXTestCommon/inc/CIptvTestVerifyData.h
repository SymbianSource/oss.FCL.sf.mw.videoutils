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


/*

Testmodule needs two new blocks:
 EnableVerifying
  - Parameters: verify ID, appendmode: 0 = if verify file exists it will be
    overwritten, 1 data is written to end of file
  - Verify ID can be the Case ID
  - Calls IptvTestUtility::CreateVerifyData
  - The case can call EnableVerifying block of multiple modules. Verify ID should be same in each call,
    just use append mode in other calls but the first.
  - If this has not been called then IptvTestUtility::VerifyData and
    IptvTestUtility::WriteVerifyData do nothing

 Verify
  - Parameters: The verify ID used in the case
  - Calls IptvTestUtility::VerifyData(CaseId) for the ID
  - Case should call Verify block only once, even EnableVerifying is called for multiple test modules

 After call to EnableVerifying the test module can write data with two different
 IptvTestUtility::WriteVerifyData methods.

 VerifyData will fail at the first time it's called. This is because it can't
 find already verified file to compare with the one generated during the case run.
 The unverified file can be found from E (or C if MMC is not installed) drive
 in the folder: \iptvtest\verify\<VerifyID>.txt. This file must be verified by
 hand and copied to IPTV_engine\internal\data\verify.

 Note: Only logs from HW test runs should be accepted. Emulator and HW might give
 different results.

 IPTV_engine\internal\sis\generateverifylist.pl script generates
 internal\data\verifyExports.inf file which exports the verified files to
 correct locations. internal\sis\pkgExports.txt is also created and the
 lines inside must be copied to STIF_IPTV_Engine.pkg. The verified files will
 be installed in c:\Testframework\Iptvtest\verify folder on the terminal.

 File & directory summary.
 1. (E/C):\iptvtest\verify\<VerifyID>.txt (on hardware)
 	- Generated during the case run
 2. internal\data\verify
 	- #1 should be copied here after it has been verified manually for first time.
 3. c:\Testframework\Iptvtest\verify (on hardware)
  	- Verified files are installed here. Compared to #1
 4. internal\sis\generateverifylist.pl
 	- Creates files #5 & #6
 5. internal\data\verifyExports.inf
 	- Exports the files in folders: epoc32\winscw\c\testframework\iptvtest\verify
					epoc32\data\Z\System\Data\Iptvtest\Verify
 6. \internal\sis\pkgExports.txt
  	- Pkg lines for the files, copy to STIF_IPTV_Engine.pkg

Example case:

[Test]
title ET39000 Data verifying example
create IptvProvisioningTest ProvTest
create IptvServiceManagementApiTest SMTest
SMTest DeleteServicesDb
ProvTest Create
ProvTest EnableVerifying ET39000 	// first module and first call will create new data file
ProvTest LocalProvision "c:\TestFramework\testvod_services.xml"
create IptvVodContentApiTest VCTest
VCTest Create SERV_1
VCTest EnableVerifying ET39000 APPEND 	// second module will append to the same file
VCTest SelectIap 3G_AP
VCTest UpdateEcg
waittestclass VCTest
VCTest VerifyAllCategoryContent KIptvVodContentCategoryRootId
ProvTest Verify ET39000  		// only one module will do the verifying
VCTest Destroy
delete VCTest
ProvTest Destroy
delete ProvTest
SMTest Destroy
delete SMTest
[Endtest]

*/


#ifndef CIPTVTESTVERIFYDATA_H
#define CIPTVTESTVERIFYDATA_H

//  INCLUDES
#include <e32base.h>
#include <e32cmn.h> // RArray
#include <f32file.h> // RFs
#include <BAUTILS.H>
#include <S32FILE.H>

// CONSTANTS
const TInt KIptvTestErrVerifyFailed = -1112;
_LIT(KIptvVerifyZipFile, "c:\\testing\\data\\verifyzip\\verify.zip" );

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CZipFile;

// CLASS DECLARATION

class CVerifyFile : public CBase
	{
	public:
		static CVerifyFile* CVerifyFile::NewL();

		virtual ~CVerifyFile();

	// data

		TBuf<64> iId;

		/**
	    * Returns the file name and path where case data is written to be verified
	    * @since
	    * @param aFileName pointer to descriptor where file name is written
	    * @return Error code, KErrNone if no errors,
	    */
		void GetFileToWrite(TDes& aFileName);

		void CreateNewFile();

		void WriteData(TDesC& aData);

		void CloseFileHandles();

	protected:
		CVerifyFile();
		void ConstructL();

	private:
		RFs iFs;
		HBufC* iFileName;
		TBool iFileOpen;
		RFileWriteStream iFileStream;
		RFile iFile;

		void OpenStream();
		void ResolveFileName();
	};


/**
*
*  @since
*/
class CIptvTestVerifyData : public CBase
    {
    public:  // Constructors and destructor

		enum TVerifyResult
			{
			EVerifyOk = 0,
			EVerifyFailed = -1112,
			EVerifiedFileNotFound = -1111,
			EFileToVerifyNotFound = -1110,
			ESystemError = -1109
			};

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CIptvTestVerifyData* NewL();


        /**
        * Destructor.
        */
        IMPORT_C virtual ~CIptvTestVerifyData();


    public: // New functions

  		/**
        * Initializes the verify file, deletes if the file exists
        * @since
        * @param aVerifyId An id to identify the file where verify data is written
        * @return Error code, KErrNone if no errors,
        */
		IMPORT_C TInt CreateVerifyDataL(TDesC& aVerifyId, TBool aAppend);

		/**
        * Writes verify data to a file
        * @since
        * @param aVerifyId An id to identify the file where verify data is written
        * @param aVerifyData Data to write into the file
        * @return Error code, KErrNone if no errors,
        */
		IMPORT_C TInt WriteVerifyDataL(TDesC& aVerifyId, TDesC& aVerifyData);

		/**
        * Verifies the results, result is written to aVerifyResult
        * @since
        * @param aVerifyId An id to identify the files which are compared
        * @return Error code, KErrNone if no errors,
        */
		IMPORT_C TInt VerifyDataL(TDesC& aVerifyId, TVerifyResult& aVerifyResult);

        /**
        * Enables timestamp writing to the file.
        * @since
        * @param aUseTimestamps
        * @return Error code, KErrNone if no errors,
        */
        IMPORT_C void EnableTimestamps( TBool aUseTimestamps );

    protected: //from base classes

    private:

        /**
        * C++ default constructor.
        */
        CIptvTestVerifyData();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    	TInt GetVerifyFileIndex(TInt& aIndex, TDesC& aVerifyId);

    	/*
    	 * Reads a file into the buffer.
    	 */
    	TInt ReadFileL(RFs& aFs, TDes& aFileName, HBufC8** aBuff, TVerifyResult& aVerifyResult);

    	/*
    	 * Extracts a file from zip file.
    	 */
    	void ExtractVerifyFileL( TDesC& aVerifyId, const TDesC& aDestFile );

    	/*
    	 * Checks if file exists in zip file.
    	 */
    	TBool FileExistsInZip( CZipFile* aZipFile, const TDesC& aFileName );

    private:    // Data
		RFs iFs;

		RPointerArray<CVerifyFile> iVerifyFiles;

		CZipFile* iZipFile;

		TBool iUseTimestamps;

		HBufC* iBuffer;
    };

#endif      // CIPTVTESTVERIFYDATA_H

// End of File
