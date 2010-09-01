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
* Description:    Interface for RTP file actions.*
*/




#ifndef CRTPCLIPHANDLER_H
#define CRTPCLIPHANDLER_H

//  INCLUDES
#include <ipvideo/CRtpFileBase.h>
#include <ipvideo/MRtpFileObserver.h>
#include <ipvideo/MRtpFileWriteObserver.h>
#include <ipvideo/MRtpFileReadObserver.h>
#include <ipvideo/CRTypeDefs.h>

// CONSTANTS
const TInt KDvrMaximumTimeShift( 12 * 60 * 60 ); // 12 hours

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CRtpToFile;
class CRtpFromFile;

// CLASS DECLARATION

/**
*  Handles RTP storing/reading in a clip with a proprietary file format.
*  
*  @lib RtpClipHandler.lib
*  @since Series 60 3.0
*/
class CRtpClipHandler : public CBase,
                        public MRtpFileObserver
    {

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @return CRtpClipHandler pointer to CRtpClipHandler class.
    */
    IMPORT_C static CRtpClipHandler* NewL();

    /**
    * Destructor.
    */
    IMPORT_C virtual ~CRtpClipHandler();

public: // New functions

    /**
    * Registers RTP file write observer.
    * @since Series 60 3.0
    * @param aObs a pointer to class which handles callback. 
    * @return none.
    */
    IMPORT_C void RegisterWriteObserver( MRtpFileWriteObserver* aObs );

    /**
    * Registers RTP file read observer.
    * @since Series 60 3.0
    * @param aObs a pointer to class which handles callback. 
    * @return none.
    */
    IMPORT_C void RegisterReadObserver( MRtpFileReadObserver* aObs );

    /**
    * Starts RTP recording to a file.
    * @since Series 60 3.0
    * @param aRecParams a recording info structure.
    * @param aAction a save action for clip mode.
    * @return none.
    */
    IMPORT_C void StartRecordingL(
        const MRtpFileWriteObserver::SRtpRecParams& aRecParams,
        const MRtpFileWriteObserver::TRtpSaveAction& aAction );

    /**
    * Saves next RTP group to a clip.
    * @since Series 60 3.0
    * @param aGroup a RTP group data.
    * @param aGroupLength a length of group.
    * @param aAction a save action for current group.
    * @return none.
    */
    IMPORT_C void SaveNextGroupL(
        TPtr8& aGroup,
        TUint aGroupLength,
        const MRtpFileWriteObserver::TRtpSaveAction& aAction );

    /**
    * Getter for writing activity state.
    * @since Series 60 3.0
    * @param none.
    * @return true if writing ongoing, otherwise false.
    */
    IMPORT_C TBool WritingActive( void ) const;

    /**
    * Getter for current length of the clip.
    * @since Series 60 3.0
    * @param none.
    * @return a total length of the clip.
    */
    IMPORT_C TUint GetCurrentLength( void ) const;

    /**
    * Clip loop mode set for time shift.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    IMPORT_C void TimeShiftPauseL();

    /**
    * Default values for rbf record params.
    * @since Series 60 3.0
    * @param aParams a recording parameters.
    * @param aIndex a index for time shift clip name.
    * @return none.
    */
    IMPORT_C void DefaultRecParams( MRtpFileWriteObserver::SRtpRecParams& aParams,
                                    const TInt aIndex );

    /**
    * Setter recording end time of the clip.
    * @since Series 60 3.0
    * @param aEndTime new end time for the clip.
    * @return none.
    */
    IMPORT_C void UpdateRecordEndTime( const TTime& aEndTime );

    /**
    * Resumes RTP recording.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    IMPORT_C void ResumeRecordingL( void );

    /**
    * Stops RTP recording.
    * @since Series 60 3.0
    * @param aError a stop error code.
    * @return none.
    */
    IMPORT_C void StopRecording( const TInt aError );

    /**
    * Starts RTP playback from a clip.
    * @since Series 60 3.0
    * @param aParams a RTP clip play params.
    * @param aTimeShift a time shift mode or not.
    * @return none.
    */
    IMPORT_C void StartPlayBackL( const SCRRtpPlayParams& aParams,
                                  const TBool aTimeShift = EFalse );

    /**
    * Starts RTP playback from a clip.
    * @since Series 60 3.0
    * @param aRtpHandle a open file handle for RTP file.
    * @return none.
    */
    IMPORT_C void StartPlayBackL( const RFile& aFileHandle );

    /**
    * Getter for SDP of the clip.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    IMPORT_C HBufC8* GetClipSdpL( void ) const;

    /**
    * Reads next RTP group from a clip.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    IMPORT_C void NextClipGroupL( void );

    /**
    * Sets Seek point of the clip.
    * @since Series 60 3.0
    * @param aTime a time where to seek in clip.
    * @return none.
    */
    IMPORT_C void SetSeekPointL( const TUint aTime );

    /**
    * Stops RTP playback.
    * @since Series 60 3.0
    * @param aError a stop error code.
    * @param aPlayerBuf a buffer length in player.
    * @return none.
    */
    IMPORT_C void StopPlayBack( const TInt aError,
                                const TUint aPlayerBuf );

    /**
    * Getter for clip version.
    * @since Series 60 3.0
    * @param none.
    * @return a internal version of the clip.
    */
    inline TInt8 ClipVersion( void ) const;

private: // Constructors and destructor

    /**
    * C++ default constructor.
    */
    CRtpClipHandler();

    /**
    * By default Symbian 2nd phase constructor is private.
    */
    void ConstructL();
            
private: // Functions from base classes

    /**
    * From MRtpFileObserver.
    * Called when RTP group save is ready.
    * @since Series 60 3.0
    * @param aAction a save action for last saved group.
    * @return None.
    */
    void RtpGroupSaved( const TInt aAction );
    
    /**
    * From MRtpFileObserver.
    * Getter for current file reader point.
    * @since Series 60 3.0
    * @param aIndex a index of shift seek array.
    * @return a read point in file reader.
    */
    TInt CurrentFileReadPoint( const TInt aIndex );

private: // New methods

    /**
    * Swap to next available clip in time shift array if needed.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void SwapClipIfNeededL( void );

private: // Data

    /**
    * RTP write observer.
    */
    MRtpFileWriteObserver* iWriteObs;
    
    /**
    * RTP read observer.
    */
    MRtpFileReadObserver* iReadObs;

    /**
    * RTP file save.
    */
    CRtpToFile* iRtpSave;
    
    /**
    * RTP file read.
    */
    CRtpFromFile* iRtpRead;
    
    /**
    * Clip version.
    */
    TInt8 iClipVersion;
    
    /**
    * Time shift save clip name index.
    */
    TInt iSaveNameIndex;

    /**
    * Time shift read clip name index.
    */
    TInt iReadNameIndex;

    /**
    * Time shift paused state.
    */
    TBool iTsPauseState;
    
    /**
    * Time shift seek handling array.
    */
    RArray<CRtpFileBase::STimeShiftSeek> iShiftSeek;
        
    };

#include <ipvideo/CRtpClipHandler.inl>

#endif // CRTPCLIPHANDLER_H

// End of file
