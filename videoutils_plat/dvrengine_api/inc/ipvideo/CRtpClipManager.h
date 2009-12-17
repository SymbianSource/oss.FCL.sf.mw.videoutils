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
* Description:    Finds access objects (SDP file) and ESG related names.*
*/




#ifndef CRTPCLIPMANAGER_H
#define CRTPCLIPMANAGER_H

// INCLUDES
#include <ipvideo/MRtpClipRepairObserver.h>
#include <ipvideo/MRtpFileWriteObserver.h>
#include <ipvideo/CRtpMetaHeader.h>
#include <ipvideo/CRTypeDefs.h>
#include <f32file.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CRtpMetaHeader;
class CRtpClipRepairer;

// CLASS DECLARATION

/**
*  Handles DVB-H RTP clip related functions.
*  
*  @lib DvrRtpClipHandler.lib
*  @since Series 60 3.0
*/
class CRtpClipManager : public CBase
    {

public: // Data types

    /**
    * Defines Clip details.
    */
    class SRtpClipDetails
        {

    public: // Data

        /**
        * Service name.
        */
        TName iService;
        
        /**
        * Program name.
        */
        TName iProgram;
        
        /**
        * Recording start time.
        */
        TTime iStartTime;
        
        /**
        * Recording end time.
        */
        TTime iEndTime;
        
        /**
        * Recording duration.
        */
        TInt  iDuration;
        
        /**
        * Recording currently ongoing state.
        */
        TBool iRecOngoing;
        
        /**
        * Recording completed status.
        */
        TBool iCompleted;
        
        /**
        * Clip protected state.
        */
        TBool iProtected;
        
        /**
        * Recording failed status.
        */
        TBool iFailed;
        
        /**
        * Clip's quality.
        */
        TUint8 iQuality;

        /**
        * Clip watched counter.
        */
        TInt iPlayCount;
        
        /*
        * Point where last watch were stopped.
        */
        TInt iPlaySpot;
        
        /**
        * Post acquisition rule ok with this device.
        */
        TBool iPostRuleOk;
        
        /**
        * Parental rate of the clip.
        */
        TInt iParental;
    };

public: // Constructors and destructor
    
    /**
    * Two-phased constructor.
    */
    IMPORT_C static CRtpClipManager* NewL();

    /**
    * Two-phased constructor.
    */
    IMPORT_C static CRtpClipManager* NewLC();

    /**
    * Destructor.
    * @return None.
    */
    IMPORT_C virtual ~CRtpClipManager();

public: // New methods

    /**
    * Getter for recorded clip details.
    * @since Series 60 3.0
    * @param aClipPath a full path of the clip.
    * @param aDetails a struct for details.
    * @return None.
    */
    IMPORT_C void GetClipDetailsL( const TDesC& aClipPath,
                                   SRtpClipDetails& aDetails );

    /**
    * Getter for recorded clip details.
    * @since Series 60 3.0
    * @param aFile File handle to the clip.
    * @param aDetails a struct for details.
    * @return None.
    */
    IMPORT_C void GetClipDetailsL( RFile& aFile,
                                   SRtpClipDetails& aDetails );

    /**
    * Setter for protected state of the clip.
    * @since Series 60 3.0
    * @param aClipPath a full path of the clip.
    * @param aProtected a protected state.
    * @return None.
    */
    IMPORT_C void ProtectClipL( const TDesC& aClipPath,
                                const TBool aProtected );

    /**
    * Asyncronous fix for clip's meta header if possible.
    * @since Series 60 3.0
    * @param aClipPath a full path of the clip.
    * @param aObs a file repairer observer.
    * @return None.
    */
    IMPORT_C void FixMetaHeaderL( MRtpClipRepairObserver* aObs,
                                  const TDesC& aClipPath );

    /**
    * Syncronous fix for clip's meta header if possible.
    * @since Series 60 3.0
    * @param aClipPath a full path of the clip.
    * @return None.
    */
    IMPORT_C void FixMetaHeaderL( const TDesC& aClipPath );

    /**
    * Deletes asyncronous clip fixer.
    * @since Series 60 3.0
    * @param aObs a file repairer observer.
    * @return None.
    */
    IMPORT_C void DeleteRtpRepairer( MRtpClipRepairObserver* aObs );

    /**
    * Verifies post acquisition rule of clip.
    * @since Series 60 3.0
    * @param aPostRule a post acquisition rule of clip.
    * @param aMetaHeader a pointer to meat header.
    * @return KErrNone if rule ok,
    *         otherwise another of the system-wide error codes.
    */
    TInt VerifyPostRuleL( const TUint8 aPostRule,
                          CRtpMetaHeader* aMetaHeader );

private: // Constructors and destructor

    /**
    * C++ default constructor.
    */
    CRtpClipManager();

    /**
    * Symbian 2nd phase constructor can leave.
    */
    void ConstructL();

private: // New methods

    /**
    * Updates details from meta header attributes.
    * @since Series 60 3.0
    * @param aAttributes a meta header attributes.
    * @param aDetails a struct for details.
    * @return None
    */
    void GetDetailsL( const CRtpMetaHeader::SAttributes& aAttributes,
                      SRtpClipDetails& aDetails,
                      CRtpMetaHeader* aMetaHeader );

    /**
    * Provides root path of clip base on memory setting.
    * @since Series 60 3.0
    * @param aClipPath a path of the clip as return value.
    * @param aDrive a drive number where to store.
    * @return None
    */
    void NewClipRootL( TDes& aClipPath,
                       const TDriveNumber aDrive );
                       
    /**
    * Generates new clip name from service/program names.
    * Adds index number to the end if file exist.
    * @since Series 60 3.0
    * @param aClipPath a path of the clip.
    * @param aProgram a program name of recorded channel.
    * @return none.
    */
    void NewIndexNameL( TDes& aClipPath,
                         const TDesC& aProgram );

    /**
    * Adds clip to the repairing queue.
    * @since Series 60 3.0
    * @param aClipPath a full path of the clip.
    * @return None
    */
    void AddClipToRepairQueueL( const TDesC& aClipPath );

private: // Data

    /**
    * File server.
    */
    RFs iFs;

    /**
    * File operations.
    */
    RFile iFile;
    
    /**
    * Devices IMEI.
    */
    HBufC* iImei;
    
    /**
    * RTP clip repairer.
    */
    CRtpClipRepairer* iClipRepairer;
    
    /**
    * RTP file repairer queue.
    */
    RPointerArray<HBufC> iRepairQueue;
    
    };

#endif // CRTPCLIPMANAGER_H

// End of File
