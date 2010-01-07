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

#include <zipfile.h>

#include "CIptvTestVerifyData.h"
#include "VCXTestLog.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

_LIT(KDriveC, "C:\\");
_LIT(KLineBreak, "\r\n");

_LIT(KPathVerifyWrite, "testing\\data\\verifynew\\");
_LIT(KVerifyExtension, ".txt");

// -----------------------------------------------------------------------------
// CVerifyFile::NewL
// -----------------------------------------------------------------------------
//
CVerifyFile* CVerifyFile::NewL()
	{
	CVerifyFile* self = new (ELeave) CVerifyFile();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

// -----------------------------------------------------------------------------
// CVerifyFile::ConstructL
// -----------------------------------------------------------------------------
//
void CVerifyFile::ConstructL()
	{
	iFs.Connect();
	iId.Zero();
	}

// -----------------------------------------------------------------------------
// CVerifyFile::CVerifyFile
// -----------------------------------------------------------------------------
//
CVerifyFile::CVerifyFile()
	{

	}

// -----------------------------------------------------------------------------
// CVerifyFile::~CVerifyFile
// -----------------------------------------------------------------------------
//
CVerifyFile::~CVerifyFile()
	{
	delete iFileName;
	iFileName = NULL;
	CloseFileHandles();
	iFs.Close();
	}

// -----------------------------------------------------------------------------
// CVerifyFile::CloseFileHandles
// -----------------------------------------------------------------------------
//
void CVerifyFile::CloseFileHandles()
	{
	VCXLOGLO1(">>>CVerifyFile::CloseFileHandles");
	if(iFileOpen)
		{
			iFile.Close();
			iFileStream.Close();
		}
	iFileOpen = EFalse;
	VCXLOGLO1("<<<CVerifyFile::CloseFileHandles");
	}

// -----------------------------------------------------------------------------
// CVerifyFile::ResolveFileName
// -----------------------------------------------------------------------------
//
void CVerifyFile::ResolveFileName()
	{
	if(iFileName == NULL)
		{
		iFileName = HBufC::NewL(256);
		}
	else
		{
		return;
		}

	VCXLOGLO1(">>>CVerifyFile::ResolveFileName");

	iFileName->Des().Zero();

	iFileName->Des().Append(KDriveC);

	iFileName->Des().Append(KPathVerifyWrite);

	BaflUtils::EnsurePathExistsL(iFs, iFileName->Des());

	iFileName->Des().Append(iId);
	iFileName->Des().Append(KVerifyExtension);

	VCXLOGLO1("<<CVerifyFile::ResolveFileName");
	}

// -----------------------------------------------------------------------------
// CVerifyFile::OpenStream
// -----------------------------------------------------------------------------
//
void CVerifyFile::OpenStream()
	{
	if(iFileOpen)
		{
		return;
		}

	VCXLOGLO1(">>>CVerifyFile::OpenStream");

	ResolveFileName();

	// create the stream

	TInt filePos = 0;
	if(BaflUtils::FileExists(iFs, iFileName->Des()))
		{
	   	TInt result = iFile.Open(iFs, iFileName->Des(), EFileShareAny | EFileWrite | EFileStreamText);
		if(result != KErrNone)
			{
			TPtrC fileName = iFileName->Des();
			VCXLOGLO2("Could not open file: %S", &fileName);
			VCXLOGLO1("<<<CIptvTestVerifyData::WriteVerifyData");
			iFile.Close();
			User::Leave(result);
			}
	   	iFile.Seek(ESeekEnd, filePos);

		iFileStream.Attach(iFile, filePos);
		}
	else
		{
		User::Leave(KErrCorrupt);
		}

	iFileOpen = ETrue;
	VCXLOGLO1("<<<CVerifyFile::OpenStream");
	}

// -----------------------------------------------------------------------------
// CVerifyFile::CreateNewFile
// -----------------------------------------------------------------------------
//
void CVerifyFile::CreateNewFile()
	{
	ResolveFileName();

	VCXLOGLO1(">>>CVerifyFile::CreateNewFile");

	if(BaflUtils::FileExists(iFs, iFileName->Des() ))
		{
		iFs.Delete(iFileName->Des());
		}

	if(KErrNone == iFile.Create(iFs, iFileName->Des(), EFileShareAny | EFileWrite | EFileStreamText))
		{
		iFile.Close();
		}

	VCXLOGLO1("<<<CVerifyFile::CreateNewFile");
	}

// -----------------------------------------------------------------------------
// CVerifyFile::WriteData
// -----------------------------------------------------------------------------
//
void CVerifyFile::WriteData(TDesC& aData)
	{
	OpenStream();

	iFileStream.WriteL(aData);
	}

// -----------------------------------------------------------------------------
// CVerifyFile::GetFileToWrite
// -----------------------------------------------------------------------------
//
void CVerifyFile::GetFileToWrite(TDes& aFileName)
    {
	ResolveFileName();

	if(iFileName->Des().Length() > 0)
		{
		aFileName = iFileName->Des();
		}
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestVerifyData* CIptvTestVerifyData::NewL()
    {
    VCXLOGLO1(">>>CIptvTestVerifyData::NewL");
    CIptvTestVerifyData* self = new (ELeave) CIptvTestVerifyData( );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    VCXLOGLO1("<<<CIptvTestVerifyData::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::~CIptvTestVerifyData
// destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestVerifyData::~CIptvTestVerifyData()
    {
    VCXLOGLO1(">>>CIptvTestVerifyData::~CIptvTestVerifyData");
	
    delete iZipFile;
	iZipFile = NULL;
	
    iFs.Close();
	
    iVerifyFiles.ResetAndDestroy();
    
    delete iBuffer;
    iBuffer = NULL;
    
    VCXLOGLO1("<<<CIptvTestVerifyData::~CIptvTestVerifyData");
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::GetVerifyFileIndex
// -----------------------------------------------------------------------------
//
TInt CIptvTestVerifyData::GetVerifyFileIndex(TInt& aIndex, TDesC& aVerifyId)
    {
    aIndex = -1;

    TInt i;

    for(i=0; i<iVerifyFiles.Count(); i++)
    	{
    	if(iVerifyFiles[i]->iId == aVerifyId)
    		{
    		aIndex = i;
    		}
    	}

    if(aIndex == -1)
    	{
    	return KErrNotFound;
    	}

    return KErrNone;
	}

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::CreateVerifyData
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestVerifyData::CreateVerifyDataL(TDesC& aVerifyId, TBool aAppend)
    {
    VCXLOGLO1(">>>CIptvTestVerifyData::CreateVerifyData");

	if(aVerifyId.Length() <= 0)
		{
		VCXLOGLO1("No verify Id");
		VCXLOGLO1("<<<CIptvTestVerifyData::CreateVerifyData");
		return KErrNone;
		}

//BAFL::EnsurePathExistsL

    TInt result = KErrNone;

    TInt index;

    if(KErrNone != GetVerifyFileIndex(index, aVerifyId))
    	{
    	CVerifyFile *vf = CVerifyFile::NewL();
    	vf->iId = aVerifyId;
    	iVerifyFiles.Append(vf);
    	if(KErrNone != GetVerifyFileIndex(index, aVerifyId))
    		{
    		VCXLOGLO1("CIptvTestVerifyData:: Verify file not found even after creating one.");
    		VCXLOGLO1("<<<CIptvTestVerifyData::CreateVerifyData");
    		return KErrGeneral;
    		}
	   	}

	TBuf<256> fileName;

	if(!aAppend)
		{
		iVerifyFiles[index]->CreateNewFile();
		}

	iVerifyFiles[index]->GetFileToWrite(fileName);

	VCXLOGLO2("CIptvTestVerifyData:: writePath %S", &fileName);

    VCXLOGLO1("<<<CIptvTestVerifyData::CreateVerifyData");
    return result;
    }


// -----------------------------------------------------------------------------
// CIptvTestVerifyData::WriteVerifyData
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestVerifyData::WriteVerifyDataL(TDesC& aVerifyId, TDesC& aVerifyData)
    {
    TInt index;

    if(KErrNone != GetVerifyFileIndex(index, aVerifyId))
    	{
    	//VCXLOGLO1("Call to WriteVerifyData before CreateVerifyData is called.");
    	//VCXLOGLO1("<<<CIptvTestVerifyData::WriteVerifyData");
    	return KErrNone;
    	}

//    VCXLOGLO1(">>>CIptvTestVerifyData::WriteVerifyData");

    iBuffer->Des().Zero();

    if( iUseTimestamps )
        {
        _LIT(KDateTimeString,"%D%M%Y%/0%1%/1%2%/2%3%/3 %-B%:0%H%:1%T%:2%S%.%*C2%:3%-B");
        TBuf<128> timeString;
        TTime time;
        time.HomeTime();
        TBuf<128> buff;
        time.FormatL( buff, KDateTimeString );
        iBuffer->Des().Append( buff );
        iBuffer->Des().Append( _L(" ") );
        }

    iBuffer->Des().Append( aVerifyData );

	TInt result = KErrNone;
	iVerifyFiles[index]->WriteData( *iBuffer );
	TBuf<12> lineChange(KLineBreak);
	iVerifyFiles[index]->WriteData( lineChange );

//    VCXLOGLO1("<<<CIptvTestVerifyData::WriteVerifyData");
    return result;
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::VerifyData
// ----------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestVerifyData::VerifyDataL(TDesC& aVerifyId, TVerifyResult& aVerifyResult)
    {
    // Get index for the verified file info
    // If not found then exit w/o error
    TInt index;

    if(KErrNone != GetVerifyFileIndex(index, aVerifyId))
    	{
    	VCXLOGLO1("CIptvTestVerifyData::VerifyDataL - Data verifying is not enabled.");
    	aVerifyResult = EVerifiedFileNotFound;
    	return KErrNone;
    	}

    VCXLOGLO1(">>>CIptvTestVerifyData::VerifyData");

	TInt result = KErrNone;
	aVerifyResult = EVerifyOk;

	// Connect to file system
	TBuf<256> fileNameRead( _L("c:\\testing\\data\\iptvtempverify.txt") ); // File which is extracted from verify.zip
	TBuf<256> fileNameWrite; // File which is written during the test

	TRAPD( err, ExtractVerifyFileL( aVerifyId, fileNameRead ) );
	if( err != KErrNone )
		{
		VCXLOGLO2("CIptvTestVerifyData:: verify file extract failed: %d", err);
		VCXLOGLO1("<<<CIptvTestVerifyData::VerifyDataL");
		aVerifyResult = EVerifiedFileNotFound;
		return KErrNone;
		}

	iVerifyFiles[index]->GetFileToWrite(fileNameWrite);
	iVerifyFiles[index]->CloseFileHandles();

	// Check that the files exist

	if(!BaflUtils::FileExists(iFs, fileNameRead))
		{
		VCXLOGLO2("File used to verify data not found. %S must be verified by hand and added to the verify.zip.", &fileNameWrite);
		VCXLOGLO1("<<<CIptvTestVerifyData::VerifyDataL");
		aVerifyResult = EVerifiedFileNotFound;
		return KErrNone;
		}

	if(!BaflUtils::FileExists(iFs, fileNameWrite))
		{
		VCXLOGLO2("File %S not found. This should not happen.", &fileNameWrite);
		VCXLOGLO1("<<<CIptvTestVerifyData::VerifyDataL");
		aVerifyResult = EVerifiedFileNotFound;
		return KErrNone;
		}

	// Open files and do comparison line by line

	// Verified file from installation
	RFile fileFromInst;
	TFileText fromInst;
	HBufC* lineFromInstBuff = HBufC::NewL( 1024 * 4 );
	CleanupStack::PushL( lineFromInstBuff );
	TPtr lineFromInst( lineFromInstBuff->Des() );

	// File from test case
	RFile fileFromTest;
	TFileText fromTest;
	HBufC* lineFromTestBuff = HBufC::NewL( 1024 * 4 );
	CleanupStack::PushL( lineFromTestBuff );
	TPtr lineFromTest( lineFromTestBuff->Des() );

   	result = fileFromInst.Open(iFs, fileNameRead, EFileRead | EFileStreamText);
	if(result != KErrNone)
		{
		VCXLOGLO2("Could not open file 1: %S", &fileNameRead);
		CleanupStack::PopAndDestroy( lineFromTestBuff );
		CleanupStack::PopAndDestroy( lineFromInstBuff );
		VCXLOGLO1("<<<CIptvTestVerifyData::VerifyDataL");
		aVerifyResult = ESystemError;
		return result;
		}
	CleanupClosePushL(fileFromInst);

   	result = fileFromTest.Open(iFs, fileNameWrite, EFileRead | EFileStreamText);
	if(result != KErrNone)
		{
		VCXLOGLO2("Could not open file 2: %S", &fileNameRead);
		VCXLOGLO1("<<<CIptvTestVerifyData::VerifyDataL");
		aVerifyResult = ESystemError;
		CleanupStack::PopAndDestroy(&fileFromInst);
		CleanupStack::PopAndDestroy( lineFromTestBuff );
		CleanupStack::PopAndDestroy( lineFromInstBuff );
		return result;
		}
	CleanupClosePushL(fileFromTest);

	TInt lineNumber(0);
	TInt totalResult(0);
	TInt32 totalLength(0);

	fromInst.Set(fileFromInst);
	fromTest.Set(fileFromTest);

	while(result == KErrNone)
		{
		TInt result2(KErrNone);

		result = fromInst.Read(lineFromInst);
		result2 = fromTest.Read(lineFromTest);

		// End of file reached for both files. No differences.
		if(result == KErrEof && result2 == KErrEof)
			{
			result = KErrNone;
			break;
			}

        // Eof reached only for one file. Verify failed.
		if( result == KErrEof )
			{
			aVerifyResult = EVerifyFailed;
			VCXLOGLO1("End of file reached for installed verify file!");
			break;
			}
        else if( result2 == KErrEof )
			{
			aVerifyResult = EVerifyFailed;
			VCXLOGLO1("End of file reached for test verify file!");
			break;
			}

        // Error reading the files.
		if(result != KErrNone && result != KErrTooBig)
			{
			VCXLOGLO3("CIptvTestVerifyData:: reading file (test) failed at line: %d, result: %d", lineNumber, result);
			break;
			}

		if(result2 != KErrNone && result2 != KErrTooBig)
			{
			VCXLOGLO3("CIptvTestVerifyData:: reading file (test) failed at line: %d, result: %d", lineNumber, result);
			result = result2;
			break;
			}

		totalLength += lineFromInst.Length();

		// Don't compare if line is for path because it can have service id which varies
		if( KErrNotFound != lineFromTest.Match(_L("*\\Data\\videocenter\\*")) && KErrNotFound != lineFromTest.Match(_L("*\\data\\videocenter\\*")) )
			{
			TInt res = lineFromInst.Compare(lineFromTest);
			if(res < 0)
				{
				totalResult += (res *- 1);
				}
			else
				{
				totalResult += res;
				}

			if(res != KErrNone)
				{
				VCXLOGLO3("CIptvTestVerifyData:: Lines at line %d are different. Res: %d", lineNumber, res);
				VCXLOGLO2("CIptvTestVerifyData:: %S", &lineFromInst);
				VCXLOGLO2("CIptvTestVerifyData:: %S", &lineFromTest);
				aVerifyResult = EVerifyFailed;
				break; // No more verifying
				}
			}

		lineNumber++;
		}

	VCXLOGLO2("CIptvTestVerifyData:: %d lines verified.", lineNumber);
	VCXLOGLO2("CIptvTestVerifyData:: %d total length of file.", totalLength);
	VCXLOGLO2("CIptvTestVerifyData:: %d total difference.", totalResult);

	CleanupStack::PopAndDestroy(&fileFromTest);
	CleanupStack::PopAndDestroy(&fileFromInst);
	CleanupStack::PopAndDestroy( lineFromTestBuff );
	CleanupStack::PopAndDestroy( lineFromInstBuff );

    VCXLOGLO1("<<<CIptvTestVerifyData::VerifyData");
    return result;
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::VerifyData
// ----------------------------------------------------------------------------
//
EXPORT_C void CIptvTestVerifyData::EnableTimestamps( TBool aUseTimestamps )
    {
    iUseTimestamps = aUseTimestamps;
    }
// -----------------------------------------------------------------------------
// CIptvTestVerifyData::CIptvTestVerifyData
// -----------------------------------------------------------------------------
//
CIptvTestVerifyData::CIptvTestVerifyData()
    {
    VCXLOGLO1(">>>CIptvTestVerifyData::CIptvTestVerifyData");

    VCXLOGLO1("<<<CIptvTestVerifyData::CIptvTestVerifyData");
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::ConstructL()
// -----------------------------------------------------------------------------
//
void CIptvTestVerifyData::ConstructL()
    {
    VCXLOGLO1(">>>CIptvTestVerifyData::ConstructL");
	iVerifyFiles.ResetAndDestroy();
	User::LeaveIfError( iFs.Connect() );
	TRAPD(err, iZipFile = CZipFile::NewL( iFs, KIptvVerifyZipFile ) );
	if( err == KErrNoMemory )
		{
		User::Leave( err );
		}
	if( err != KErrNone )
		{
	    VCXLOGLO2("CIptvTestVerifyData:: CZipFile::NewL err: %d", err);
	    delete iZipFile;
	    iZipFile = NULL;
		}

	iBuffer = HBufC::NewL( 1024 * 5 );

    VCXLOGLO1("<<<CIptvTestVerifyData::ConstructL");
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::ReadFile()
// -----------------------------------------------------------------------------
//
TInt CIptvTestVerifyData::ReadFileL(RFs& aFs, TDes& aFileName, HBufC8** aBuff, TVerifyResult& aVerifyResult)
    {
	VCXLOGLO2(">>>CIptvTestVerifyData::ReadFile: %S", &aFileName);

	TInt result = KErrNone;
	RFile file;
	TInt fileSize;

   	result = file.Open(aFs, aFileName, EFileRead | EFileStreamText);
	if(result != KErrNone)
		{
		VCXLOGLO2("Could not open file 1: %S", &aFileName);
		VCXLOGLO1("<<<CIptvTestVerifyData::VerifyDataL");
		aVerifyResult = ESystemError;
		return result;
		}
	CleanupClosePushL(file);

	file.Size(fileSize);

	TInt filePos = 0;
	file.Seek(ESeekStart, filePos);

	HBufC8* buff = HBufC8::NewL( fileSize);

	TPtr8 ptr( buff->Des() );
	file.Read(ptr);

	*aBuff = buff;

	CleanupStack::PopAndDestroy(&file);

    VCXLOGLO1("<<<CIptvTestVerifyData::ReadFile");
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::ExtractVerifyFile()
// -----------------------------------------------------------------------------
//
void CIptvTestVerifyData::ExtractVerifyFileL( TDesC& aVerifyId, const TDesC& aDestFile )
	{
	VCXLOGLO1(">>>CIptvTestVerifyData::ExtractVerifyFile");

	iFs.Delete( aDestFile );

	if( !iZipFile )
		{
		VCXLOGLO1("CIptvTestVerifyData:: iZipFile == NULL! Leaving..");
		User::Leave( KErrNotReady );
		}

    TBuf<256> verifyFileName( aVerifyId );
    verifyFileName.Append( KVerifyExtension );
    VCXLOGLO2("CIptvTestVerifyData:: verifyFileName: %S", &verifyFileName);
    VCXLOGLO2("CIptvTestVerifyData:: aDestFile: %S", &aDestFile);

	if( !FileExistsInZip( iZipFile, verifyFileName ) )
	  	{
		VCXLOGLO1("CIptvTestVerifyData:: No such file in zip! Leaving..");
		User::Leave( CIptvTestVerifyData::EVerifiedFileNotFound );
	  	}

    // Get zip file member from the archive
    VCXLOGLO1("CIptvTestVerifyData:: Get zip file member from the archive");
    CZipFileMember* member = iZipFile->CaseInsensitiveMemberL( verifyFileName );
    if( member == NULL )
    	{
    	VCXLOGLO1("CIptvTestVerifyData:: zip member is null.  Leaving..");
    	User::Leave(KErrGeneral);
    	}
    CleanupStack::PushL( member );

    // Get read stream for the file
    VCXLOGLO1("CIptvTestVerifyData:: Get read stream for the file");
    RZipFileMemberReaderStream* stream;
    iZipFile->GetInputStreamL( member, stream );
    CleanupStack::PushL( stream );

    // Alloc buffer and read the archived file
    VCXLOGLO1("CIptvTestVerifyData:: Alloc buffer and read the archived file");
    HBufC8* buffer = HBufC8::NewLC( member->UncompressedSize() );
    TPtr8 bufferPtr( buffer->Des() );
    User::LeaveIfError( stream->Read( bufferPtr, member->UncompressedSize() ) );

    // Write the file to the filesystem
    VCXLOGLO1("CIptvTestVerifyData:: Write the file to the filesystem");
    RFile file;
    User::LeaveIfError( file.Replace( iFs, aDestFile, EFileWrite ) );
    CleanupClosePushL( file );
    User::LeaveIfError( file.Write(*buffer) );

    CleanupStack::PopAndDestroy( &file );
    CleanupStack::PopAndDestroy( buffer );
    CleanupStack::PopAndDestroy( stream );
    CleanupStack::PopAndDestroy( member );

    VCXLOGLO1("<<<CIptvTestVerifyData::ExtractVerifyFile");
	}

// -----------------------------------------------------------------------------
// CIptvTestVerifyData::FileExistsInZip()
// -----------------------------------------------------------------------------
//
TBool CIptvTestVerifyData::FileExistsInZip( CZipFile* aZipFile, const TDesC& aFileName )
	{
	VCXLOGLO1(">>>CIptvTestVerifyData::FileExistsInZip");

	CZipFileMember* member = NULL;

	VCXLOGLO1("CIptvTestVerifyData:: list of members");
	CZipFileMemberIterator* members = aZipFile->GetMembersL();

	VCXLOGLO1("CIptvTestVerifyData:: loop all members");

	while ((member = members->NextL()) != 0)
	    {
	    VCXLOGLO3("CIptvTestVerifyData:: member %S, size: %d", &(*member->Name()), member->CompressedSize());

	    if( aFileName == (*member->Name()) )
	    	{
		    delete member;
			delete members;
			VCXLOGLO1("<<<CIptvTestVerifyData::FileExistsInZip");
		    return ETrue;
	    	}

	    delete member;
	    }

	delete members;
	VCXLOGLO1("<<<CIptvTestVerifyData::FileExistsInZip");
	return EFalse;
	}

// ---------------------------------
//  End of File
