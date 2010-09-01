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
* Description:    RTP file save format for Common Recording Engine.*
*/




#ifndef CRTPTOFILE_H
#define CRTPTOFILE_H

// INCLUDES
#include <ipvideo/CRtpFileBase.h>
#include <ipvideo/MRtpFileObserver.h>
#include <ipvideo/MRtpFileWriteObserver.h>

// CONSTANTS
// None

// MACROS
// none

// DATA TYPES
// none

// FORWARD DECLARATIONS
class CRtpMetaHeader;

// CLASS DECLARATION

/**
*  RTP format write functionalities for RTP Clip Handler.
*
*  @lib RtpClipHandler.lib
*  @since Series 60 3.0
*/
class CRtpToFile : public CRtpFileBase
    {

public: // Constructors and destructor
    
    /**
    * Two-phased constructor.
    */
    static CRtpToFile* NewL( MRtpFileObserver& aObs,
                             MRtpFileWriteObserver& aHandler );

    /**
    * Destructor.
    */
    virtual ~CRtpToFile();

public: // New functions

    /**
    * Initiates RTP packets saving.
    * @since Series 60 3.0
    * @param aObs a RTP save observer.
    * @param aParams a recording parameters.
    * @param aAction a save action for current group.
    * @return none.
    */
    void InitRtpSaveL( const MRtpFileWriteObserver::SRtpRecParams& aParams,
                       const MRtpFileWriteObserver::TRtpSaveAction& aAction );

    /**
    * Setter for packets re-use flag.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void ActivateGroupsReuseL();

    /**
    * Swaps new clip for packets saving.
    * @since Series 60 3.0
    * @param aParams a recording parameters.
    * @return none.
    */
    void SwapClipL( const MRtpFileWriteObserver::SRtpRecParams& aParams );

    /**
    * Saves next RTP packet group to a file.
    * @since Series 60 3.0
    * @param aGroup a RTP group data.
    * @param aGroupLength a length of group.
    * @param aAction a save action for current group.
    * @return a file location of saved group.
    */
    TInt SaveNextGroupL( TPtr8& aGroup,
                         TUint& aGroupLength,
                         const MRtpFileWriteObserver::TRtpSaveAction& aAction );

    /**
    * Updates previous time after pause.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void UpdatePreviousTimeL();

    /**
    * Stops RTP packet saving.
    * @since Series 60 3.0
    * @param aError a stop error code.
    * @return none.
    */
    void StopRtpSave( const TInt aError );

    /**
    * Getter for a clip path.
    * @since Series 60 3.0
    * @param none.
    * @return reference to current path.
    */
    HBufC* ClipPath();

    /**
    * Getter current length of the clip.
    * @since Series 60 3.0
    * @param none.
    * @return a total length of the clip.
    */
    TUint GetCurrentLength();

    /**
    * Setter recording end time of the clip.
    * @since Series 60 3.0
    * @param aEndTime new end time for the clip.
    * @return none.
    */
    void UpdateRecordEndTime( const TTime& aEndTime );

    /**
    * Updates playback count and spot attributes.
    * @since Series 60 3.0
    * @param aNewSpot a new play start spot.
    * @return none.
    */
    void UpdatePlayAttL( const TInt aNewSpot );

public: // New inline functions

    /**
    * Getter for packet groups total count.
    * @since Series 60 3.0
    * @param none.
    * @return a value of groups total count variable.
    */
    inline TInt GroupsTotalCount();

    /**
    * Getter for first group address.
    * @since Series 60 3.0
    * @param none.
    * @return a value of first address variable.
    */
    inline TInt FirstSeekAddr();

    /**
    * Getter for last group address.
    * @since Series 60 3.0
    * @param none.
    * @return a value of last address variable.
    */
    inline TInt LastSeekAddr();

    /**
    * Getter for seek header point.
    * @since Series 60 3.0
    * @param none.
    * @return a value of seek header variable.
    */
    inline TInt SeekHeaderPoint();

    /**
    * Getter for seek array.
    * @param none.
    * @since Series 60 3.0
    * @return a pointer to seek array.
    */
    inline CArrayFix<SSeek>* SeekArray();
    
    /**
    * Getter for current save action
    * @param none.
    * @since Series 60 3.0
    * @return current saving action (status)
    */
    inline MRtpFileWriteObserver::TRtpSaveAction Action() const;

private: // Constructors and destructor

    /**
    * C++ default constructor.
    */
    CRtpToFile( MRtpFileObserver& aObs,
                MRtpFileWriteObserver& aHandler );

    /**
    * Symbian 2nd phase constructor can leave and is private by default.
    */
    void ConstructL();
    
private: // Functions from base classes

    /**
    * From CRtpFileBase : Called when request completion event occurs.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void RunL();

    /**
    * From CRtpFileBase : Handles a leave occurring in the request.
    *                     completion event handler RunL().
    * @since Series 60 3.0
    * @param aError the leave code
    * @return a status of function
    */
    TInt RunError( TInt aError );

    /**
    * From CRtpFileBase : Called when request completion event cancelled.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void DoCancel();

private: // New functions

    /**
    * Opens new clip and creates initial headers.
    * @since Series 60 3.0
    * @param aParams a recording parameters.
    * @return none.
    */
    void CreateNewClipL( 
        const MRtpFileWriteObserver::SRtpRecParams& aParams );

    /**
    * Sets variables for a new group.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void AddGroupL();

    /**
    * Calculates group time.
    * @since Series 60 3.0
    * @param aGroupLength a length of incoming group.
    * @return none.
    */
    void GroupTimeL( TUint& aGroupLength );

    /**
    * Writes clip's initial meta data header of the clip.
    * @since Series 60 3.0
    * @param aParams a recording parameters.
    * @return none.
    */
    void WriteInitialMetaHeaderL( 
        const MRtpFileWriteObserver::SRtpRecParams& aParams );

    /**
    * Writes clip's final meta data header to a clip.
    * @since Series 60 3.0
    * @param aStatus a status of recording.
    * @return none.
    */
    void WriteFinalMetaHeaderL( const TInt aStatus );

    /**
    * Adds RTP group header to a group.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void AddGroupHeaderL();

    /**
    * Updates recording duration.
    * @since Series 60 3.0
    * @param aMetaHeader a pointer to meta header.
    * @return none.
    */
    void UpdateDurationL( CRtpMetaHeader* aMetaHeader );

    /**
    * Getter for current network time.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void UpdateCurrentTimeL();
    
private: // Data

	/**
	* RTP file observer.
	*/
    MRtpFileObserver& iFileObs;

    /**
    * RTP write observer.
    */
    MRtpFileWriteObserver& iWriteObs;
    
    /**
    * Current network time.
    */
    TTime iCurrentTime;

    /**
    * Previous network time.
    */
    TInt64 iPreviousTime;
    
    /**
    * Previous time delta.
    */
    TInt iPreviousDelta;
    
    /**
    * Reference recording time.
    */
    TInt64 iReferenceTime;
    
    /**
    * Record end time.
    */
    TInt64 iRecordEndTime;
    
    /**
    * Reference to seek array delta.
    */
    TUint iSeekArrayReference;

    /**
    * Group time of the group where recording started.
    */
    TUint iStartGroupTime;
    
    /**
    * Total groups for re-use.
    */
    TInt iGroupReUse;
    
    /**
    * Current save action.
    */
    MRtpFileWriteObserver::TRtpSaveAction iAction;
        
  };

#include "CRtpToFile.inl"

#endif // CRTPTOFILE_H

// End of File
