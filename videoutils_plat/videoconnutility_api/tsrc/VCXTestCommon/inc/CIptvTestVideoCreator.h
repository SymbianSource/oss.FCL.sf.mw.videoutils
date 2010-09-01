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


#ifndef CIptvTestVideoCreator_H
#define CIptvTestVideoCreator_H

//  INCLUDES
#include <e32base.h>
#include <e32std.h>
#include <f32file.h>
#include <BAUTILS.H>

// CONSTANTS
const int KVcxTestLargeFile3GB = -999;

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*
*
*
*
*  @since
*/
class CIptvTestVideoCreator : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CIptvTestVideoCreator* NewL();

        /**
        * C++ default constructor.
        */
        IMPORT_C CIptvTestVideoCreator();

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CIptvTestVideoCreator();

        enum TIptvTestVideoType
			{
            IptvTestVideo3Gp,               // Extension .3GP
            IptvTestVideo3Gp2,              // Extension .3G2
            IptvTestVideoMp2,               // Extension .MP2
            IptvTestVideoMpeg1,             // Extension .MPG
            IptvTestVideoMpeg2,             // Extension .MPG
            IptvTestVideoSuperVideoCd,      // Extension .MPG
            IptvTestVideoMp3,               // Extension .MP3
            IptvTestVideoAppleMpeg,         // Extension .MP4
            IptvTestVideoMpeg4,             // Extension .MP4
            IptvTestVideoMpegAvc,           // Extension .MP4
            IptvTestVideoQuicktime,         // Extension .MOV
            IptvTestVideoRealVideo,         // Extension .RM
            IptvTestVideoAvi,               // Extension .AVI
            IptvTestVideoWmv,                // Extension .WMV
            IptvTestVideoAviAC3              // Extension .AVI
			};

    public: // New functions

        /**
        * Creates video to specified location by copying file from e/c:\testing\data according to the video type.
        * @param aVideoType a type of video which is created
        * @param aFileName path filename where video is created
        * @param aSize size of the video. If -1 then size will not be changed, if any other, playback of
        * video will probably fail.
        */
        IMPORT_C void CreateVideoL( TIptvTestVideoType aVideoType, TDesC& aFileName, TInt aSize );

        /*
         * Creates number of video files to specified location.
         */
        IMPORT_C void CreateVideosL( TIptvTestVideoType aVideoType, TDesC& aFileName, TInt aCount, RPointerArray<HBufC>& aFileArray );

    protected: //from base classes

    private:

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        IMPORT_C void ConstructL();

        /**
        * Returns the file and path for specied file type.
        */
        IMPORT_C void GetVideoFile( TDes& aFileName, CIptvTestVideoCreator::TIptvTestVideoType aVideoType, const TDesC& aDrive );

    private:    // Data
       RFs iFs;
       CFileMan* iFileMan;
    };


#endif      // CIptvTestVideoCreator_H

// End of File
