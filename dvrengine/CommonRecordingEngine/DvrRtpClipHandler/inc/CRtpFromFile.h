/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    RTP file read format for Common Recording Engine.*
*/





#ifndef __CRTPFROMFILE_H
#define __CRTPFROMFILE_H

// INCLUDES
#include <ipvideo/CRtpFileBase.h>
#include <ipvideo/MRtpFileReadObserver.h>
#include "MRtpTimerObserver.h"

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CRtpToFile;
class CRtpTimer;

// CLASS DECLARATION

/**
*  RTP format read functionalities for RTP Clip Handler.
*
*  @lib RtpClipHandler.lib
*  @since Series 60 3.0
*/
class CRtpFromFile : public CRtpFileBase,
                     public MRtpTimerObserver
    {

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @param aReadObs a reference to file read observer.
    * @param aToFile a pointer to RTP save object.
    * @return pointer to CRtpFromFile class.
    */
    static CRtpFromFile* NewL( MRtpFileReadObserver& aReadObs,
                               CRtpToFile* aToFile );

    /**
    * Destructor.
    */
    virtual ~CRtpFromFile();

public: // New functions

    /**
    * Initiates RTP packet reading.
    * @since Series 60 3.0
    * @param aClipPath a full path of the clip.
    * @param aVersion a version of the clip.
    * @param aTimeShift a mode selection.
    * @return none.
    */
    void InitRtpReadL( const TDesC& aClipPath,
                       TInt8& aVersion,
                       const TBool aTimeShift );

    /**
    * Initiates RTP packet reading.
    * @since Series 60 3.0
    * @param aRtpHandle a open file handle for RTP file.
    * @param aVersion a version of the clip.
    * @param aTimeShift a mode selection.
    * @return none.
    */
    void InitRtpReadL( const RFile& aFileHandle,
                       TInt8& aVersion );

    /**
    * Swaps new clip for RTP packet reading.
    * @since Series 60 3.0
    * @param aClipPath a full path of clip.
    * @return a version of the clip.
    */
    TInt8 SwapClipL( const TDesC& aClipPath );

    /**
    * Getter for SDP data from the clip.
    * @since Series 60 3.0
    * @param none.
    * @return a buffer keeping SDD data.
    */
    HBufC8* GetClipSdpL();

    /**
    * Reads next RTP packets group from a clip.
    * @since Series 60 3.0
    * @param aGroupPoint a group to read.
    * @return KErrInUse if reading already active,
              KErrEof if too close to live point,
              KErrNone otherwise.
    */
    TInt ReadNextGroupL( const TInt aGroupPoint );

    /**
    * Reads skipped RTP packet from a clip.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void ReadSkippedGroup();

    /**
    * Setter for last seek addres.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void UpdateLastSeekAddr();
    
    /**
    * Sets Seek point of the clip.
    * @since Series 60 3.0
    * @param aTime a time where to seek.
    * @return none.
    */
    void SetSeekPointL( const TUint aTime );

    /**
    * Stops RTP packet reading.
    * @since Series 60 3.0
    * @param aStatus a stopping status.
    * @param aPlayerBuf a buffer length in player.
    * @return none.
    */
    void StopRtpRead( const TInt aStatus,
                      const TUint aPlayerBuf );

    /**
    * Getter for duration of the clip.
    * @since Series 60 3.0
    * @param none.
    * @return a duration of the clip.
    */
    inline TUint Duration();

    /**
    * Getter for time shift mode.
    * @since Series 60 3.0
    * @param none.
    * @return true if time shift ongoing, otherwise false.
    */
    inline TBool IsTimeShift();

    /**
    * Getter for currently active group point.
    * @since Series 60 3.0
    * @param none.
    * @return a group point of current read.
    */
    inline TInt ThisGroup();

private: // Constructors and destructor

    /**
    * C++ default constructor.
    * @param aObs a RTP save observer.
    * @param aToFile a pointer to RTP save object.
    */
    CRtpFromFile( MRtpFileReadObserver& aReadObs,
                  CRtpToFile* aToFile );

    /**
    * Symbian 2nd phase constructor can leave and is private by default.
    */
    void ConstructL();
    
private: // Functions from base classes

    /**
    * From CRtpFileBase.
    * Called when request completion event occurs.
    * @since Series 60 3.0
    * @return none.
    */
    void RunL();

    /**
    * From CRtpFileBase.
    * Handles a leave occurring in the request completion event handler RunL().
    * @since Series 60 3.0
    * @param aError the leave code.
    * @return status of run error handling.
    */
    TInt RunError( TInt aError );

    /**
    * From CRtpFileBase.
    * Called when request completion event cancelled.
    * @since Series 60 3.0
    * @return none.
    */
    void DoCancel();

    /**
    * From MRmTimerObserver : Called when timer completion event occurs.
    * @since Series 60 3.0
    * @return none
    */
    void TimerEventL();

    /**
    * From MRtpTimerObserver.
    * Handles a leave occurring in the request completion event handler RunL().
    * @since Series 60 3.0
    * @param aError the leave code.
    * @return none.
    */
    void TimerError( const TInt aError );

private: // New functions

    /**
    * Reads clip header from a clip.
    * @since Series 60 3.0
    * @param aVersion a version of clip.
    * @return none.
    */
    void ReadClipHeaderL( TInt8& aVersion );

    /**
    * Reads meta header from a clip.
    * @since Series 60 3.0
    * @param aSeekHeaderPoint a seek header point in clip.
    * @param aSeekArrayPoint a seek array point in clip.
    * @return Version of clip.
    */
    TInt8 ReadMetaHeaderL( TInt& aSeekHeaderPoint,
                           TInt& aSeekArrayPoint );

    /**
    * Reads RTP payload from a clip.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void ReadNextGroupFromFileL();

    /**
    * Finds group base on seek time.
    * @since Series 60 3.0
    * @param aTime a time to use for search.
    * @param aArray a array to use for search.
    * @return point to group where time with TS exist.
    */
    TInt FindSeekGroup( const TUint aTime,
                        CArrayFix<SSeek>* aArray );

    /**
    * Updates playback count and spot attributes.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void UpdatePlayAttL();

    /**
    * Getter for last seek addres.
    * @since Series 60 3.0
    * @param none.
    * @return a last seek address.
    */
    TInt LastSeekAddr();
    
private: // Data

    /**
    * RTP read observer.
    */
    MRtpFileReadObserver& iReadObs;

    /**
    * Pointer to RTP save object.
    */
    CRtpToFile* iToFile;
    
    /**
    * File data buffer.
    */
    HBufC8* iFileData;
    
    /**
    * Request skipped.
    */
    TBool iSkippedRead;
    
    /**
    * Duration of the clip.
    */
    TInt iDuration;
    
    /**
    * Group read timer.
    */
    CRtpTimer* iTimer;

  };

#include "CRtpFromFile.inl"

#endif  //__CRTPFROMFILE_H

// End of File
