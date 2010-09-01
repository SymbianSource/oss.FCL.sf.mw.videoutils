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


//  INCLUDES
#include <f32file.h>
#include <BAUTILS.H>

#include "CIptvTestVideoCreator.h"
#include "VCXTestLog.h"

// -----------------------------------------------------------------------------
//  NewL
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestVideoCreator* CIptvTestVideoCreator::NewL()
    {
    CIptvTestVideoCreator* self;
    self = new (ELeave) CIptvTestVideoCreator();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
//  CIptvTestVideoCreator
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestVideoCreator::~CIptvTestVideoCreator()
    {
    delete iFileMan;
    iFileMan = NULL;
    iFs.Close();
    }

// -----------------------------------------------------------------------------
//  CIptvTestVideoCreator
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestVideoCreator::CIptvTestVideoCreator()
    {

    }

// -----------------------------------------------------------------------------
//  ConstructL
// -----------------------------------------------------------------------------
//
EXPORT_C void CIptvTestVideoCreator::ConstructL()
    {
    User::LeaveIfError( iFs.Connect() );
    iFileMan = CFileMan::NewL( iFs );
    }

// -----------------------------------------------------------------------------
//  CreateVideoL
// -----------------------------------------------------------------------------
//
EXPORT_C void CIptvTestVideoCreator::CreateVideoL(CIptvTestVideoCreator::TIptvTestVideoType aVideoType, TDesC& aFileName, TInt aSize)
    {
    VCXLOGLO1(">>>CIptvTestVideoCreator::CreateVideoL");

    // Resolve source filename
    TBuf<256> srcFileName;

    GetVideoFile( srcFileName, aVideoType, _L("C") );
    if( !BaflUtils::FileExists(iFs, srcFileName) )
        {
        VCXLOGLO2("CIptvTestVideoCreator:: %S does not exist.", &srcFileName);
        GetVideoFile( srcFileName, aVideoType, _L("E") );
        if( !BaflUtils::FileExists(iFs, srcFileName) )
            {
            VCXLOGLO2("CIptvTestVideoCreator:: %S does not exist.", &srcFileName);
            VCXLOGLO2("CIptvTestVideoCreator:: test video file %S missing! PANIC.", &srcFileName);
            User::Panic(_L("Video files missing!"), KErrNotFound);
            }
        }
    
    BaflUtils::EnsurePathExistsL( iFs, aFileName.Left( aFileName.LocateReverse('\\') ) );

    BaflUtils::DeleteFile( iFs, aFileName );
    
    VCXLOGLO2("CIptvTestVideoCreator:: aSize = %d", aSize);
    
    TInt64 wantedSize( 0 );
    
    // Check the size
    if( aSize == KVcxTestLargeFile3GB ) {
        wantedSize = 3000000000;
    }
    else {
        wantedSize = aSize;
    }
    
    //wantedSize = wantedSize == 0 ? wantedSize -1 : wantedSize;
    
    VCXLOGLO2("CIptvTestVideoCreator:: Wanted file size: %Ld", wantedSize);

    // Read source file into memory, won't work on huge files.
    RFile64 srcFile;
    VCXLOGLO2("CIptvTestVideoCreator:: Opening %S", &srcFileName);
    User::LeaveIfError( srcFile.Open( iFs, srcFileName, EFileRead ) );
    CleanupClosePushL( srcFile );

    TInt64 srcSize(0);
    VCXLOGLO2("CIptvTestVideoCreator:: Getting size of %S", &srcFileName);
    User::LeaveIfError( srcFile.Size( srcSize ) );

    HBufC8* data = HBufC8::NewL( srcSize );
    TPtr8 ptr( data->Des() );
    srcFile.Read( ptr, srcSize );
    CleanupStack::PopAndDestroy( &srcFile );

    CleanupStack::PushL( data );

    // Write new file.
    RFile64 dstFile;
    VCXLOGLO1("CIptvTestVideoCreator:: Replace");
    User::LeaveIfError( dstFile.Replace( iFs, aFileName, EFileWrite ) );
    CleanupClosePushL(dstFile);

    if( wantedSize <= srcSize )
        {
        if( wantedSize == -1 )
            {
            VCXLOGLO2("CIptvTestVideoCreator:: Writing %Ld", srcSize);
            User::LeaveIfError( dstFile.Write( *data, srcSize ) );
            }
        else
            {
            VCXLOGLO2("CIptvTestVideoCreator:: Writing %Ld", wantedSize);
            User::LeaveIfError( dstFile.Write( *data, wantedSize ) );
            }
        }
    else
        {
        VCXLOGLO2("CIptvTestVideoCreator:: Writing %Ld", srcSize);
        User::LeaveIfError( dstFile.Write( *data, srcSize ) );

        const TInt KIptvTest200Kilos = 1024*200;
        HBufC8* buff = HBufC8::NewL( KIptvTest200Kilos );
        buff->Des().SetLength( KIptvTest200Kilos );
        CleanupStack::PushL( buff );
        TInt64 bytesToWrite = wantedSize - srcSize;
        while( bytesToWrite > 0 )
            {
            if( bytesToWrite >= KIptvTest200Kilos )
                {
                bytesToWrite -= KIptvTest200Kilos;
                User::LeaveIfError( dstFile.Write( *buff ) );
                }
            else
                {
                User::LeaveIfError( dstFile.Write( *buff, bytesToWrite ) );
                bytesToWrite = 0;
                }
            }
        CleanupStack::PopAndDestroy( buff );
        }

    CleanupStack::PopAndDestroy( &dstFile );
    CleanupStack::PopAndDestroy( data );

    VCXLOGLO1("<<<CIptvTestVideoCreator::CreateVideoL");
    }


// -----------------------------------------------------------------------------
//  CreateVideosL
// -----------------------------------------------------------------------------
//
EXPORT_C void CIptvTestVideoCreator::CreateVideosL(CIptvTestVideoCreator::TIptvTestVideoType aVideoType, TDesC& aFileName, TInt aCount, RPointerArray<HBufC>& aFileArray )
    {
    VCXLOGLO1(">>>CIptvTestVideoCreator::CreateVideosL");

    // Resolve source filename
    TBuf<256> srcFileName;

    GetVideoFile( srcFileName, aVideoType, _L("C") );

    if( !BaflUtils::FileExists(iFs, srcFileName) )
        {
        VCXLOGLO2("CIptvTestVideoCreator:: %S does not exist.", &srcFileName);
        GetVideoFile( srcFileName, aVideoType, _L("E") );
        if( !BaflUtils::FileExists(iFs, srcFileName) )
            {
            VCXLOGLO2("CIptvTestVideoCreator:: %S does not exist.", &srcFileName);
            VCXLOGLO2("CIptvTestVideoCreator:: test video file %S missing! PANIC.", &srcFileName);
            // Comment next line if you want dummy files to be created. They won't be recognized by MDS.
            User::Panic(_L("Video files missing!"), KErrNotFound);
            }
        }

    TBool fileExists = BaflUtils::FileExists(iFs, srcFileName);

    HBufC* newFileName = HBufC::NewL( 256 );
    CleanupStack::PushL( newFileName );

    newFileName->Des().Copy( aFileName.Left( aFileName.LocateReverse('\\') ) );
    BaflUtils::EnsurePathExistsL(iFs, *newFileName);

    TInt dotPos = aFileName.LocateReverse('.');

    for( TInt i = 0; i < aCount; i++ )
        {
        newFileName->Des().Copy( aFileName.Left( dotPos ) );
        newFileName->Des().Append( _L("_") );
        newFileName->Des().AppendNum( i );
        newFileName->Des().Append( aFileName.Right( aFileName.Length() - dotPos ) );

        HBufC* fileForClient = newFileName->Des().AllocL();
        aFileArray.Append( fileForClient );

        if( fileExists )
            {
            User::LeaveIfError( iFileMan->Copy(srcFileName, *newFileName) );
            VCXLOGLO2("CIptvTestVideoCreator:: copy file: '%S'", newFileName);
            }
        else
            {
            VCXLOGLO2("CIptvTestVideoCreator:: new fake file: '%S'", newFileName);
            RFile file;
            CleanupClosePushL(file);
            User::LeaveIfError( file.Replace(iFs, *newFileName, EFileWrite) );
            User::LeaveIfError( file.SetSize( 1024*10 ) );
            CleanupStack::PopAndDestroy( &file );
            }
        User::After( 100000 ); // Wait tenth of a second.
        }

    CleanupStack::PopAndDestroy( newFileName );

    VCXLOGLO1("<<<CIptvTestVideoCreator::CreateVideosL");
    }

// -----------------------------------------------------------------------------
//  GetVideoFile
// -----------------------------------------------------------------------------
//
EXPORT_C void CIptvTestVideoCreator::GetVideoFile( TDes& aFileName, CIptvTestVideoCreator::TIptvTestVideoType aVideoType, const TDesC& aDrive )
    {
    VCXLOGLO1(">>>CIptvTestVideoCreator::GetVideoFile");
    aFileName.Copy( aDrive );

    _LIT(KIptvTestVideoBasePath, ":\\testing\\data\\");

    aFileName.Append( KIptvTestVideoBasePath );
    
    VCXLOGLO2("CIptvTestVideoCreator::GetVideoFile -- using %S", &aFileName);

    switch (aVideoType)
        {
        case CIptvTestVideoCreator::IptvTestVideo3Gp:
            {
            aFileName.Append( _L("video_3gp.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideo3Gp2:
            {
            aFileName.Append( _L("video_3gp2.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoMp2:
            {
            aFileName.Append( _L("video_mp2.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoMpeg1:
            {
            aFileName.Append( _L("video_mpeg1.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoMpeg2:
            {
            aFileName.Append( _L("video_mpeg2.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoSuperVideoCd:
            {
            aFileName.Append( _L("video_supervideocd.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoMp3:
            {
            aFileName.Append( _L("video_mp3.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoAppleMpeg:
            {
            aFileName.Append( _L("video_applempeg.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoMpeg4:
            {
            aFileName.Append( _L("video_mpeg4.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoMpegAvc:
            {
            aFileName.Append( _L("video_mpegavc.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoQuicktime:
            {
            aFileName.Append( _L("video_quicktime.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoRealVideo:
            {
            aFileName.Append( _L("video_realvideo.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoAvi:
            {
            aFileName.Append( _L("video_avi.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoWmv:
            {
            aFileName.Append( _L("video_wmv.xxx") );
            }
            break;

        case CIptvTestVideoCreator::IptvTestVideoAviAC3:
            {
            aFileName.Append( _L("video_aviac3.xxx") );
            }
            break;            
        default:
            User::Panic(_L("IptvMyVideosApiTest: Unknown video type!"), KErrCorrupt);
        }
    VCXLOGLO1("<<<CIptvTestVideoCreator::GetVideoFile");
    }

// End of File
