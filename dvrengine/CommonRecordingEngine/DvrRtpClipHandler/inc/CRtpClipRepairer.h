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
* Description:    RTP clip repairer for common recording engine.*
*/




#ifndef CRTPCLIPREPAIRER_H
#define CRTPCLIPREPAIRER_H

// INCLUDES
#include <ipvideo/CRtpFileBase.h>
#include "CCRRtpFileSource.h"
#include <ipvideo/MRtpClipRepairObserver.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CRtpMetaHeader;

// CLASS DECLARATION

/**
*  Repairs corrupted DVB-H RTP clip.
*  
*  @lib DvrRtpClipHandler.lib
*  @since Series 60 3.0
*/
class CRtpClipRepairer : public CRtpFileBase
    {

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @since Series 60 3.0
    * @param aObs observer for repair status
    * @return None.
    */
    static CRtpClipRepairer* NewL( MRtpClipRepairObserver* aObs );

    /**
    * Destructor.
    */
    virtual ~CRtpClipRepairer();

public: // New functions

    /**
    * Check if corrupted meta header can be fixed.
    * @since Series 60 3.0
    * @param aClipName a full path of the clip.
    * @return None.
    */
    void CheckMetaHeaderL( const TDesC& aClipName ); 

    /**
    * Getter for the clip name under repairing.
    * @since Series 60 3.0
    * @return a full path of the clip under repairing.
    */
    TPtrC CurrentClipName(); 

private: // Constructors and destructor

    /**
    * C++ default constructor.
    * @since Series 60 3.0
    * @param aObs observer for repair status
    */
    CRtpClipRepairer( MRtpClipRepairObserver* aObs );

    /**
    * Symbian 2nd phase constructor can leave and is private by default.
    */
    void ConstructL();
  	
private: // Functions from base classes

    /**
    * From CRtpFileBase.
    * Called when request completion event occurs.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void RunL();

    /**
    * From CRtpFileBase.
    * Handles a leave occurring in the request completion event handler RunL().
    * @since Series 60 3.0
    * @param aError the leave code.
    * @return a status of function.
    */
    TInt RunError( TInt aError );

    /**
    * From CRtpFileBase : Called when request completion event cancelled.
    * @since Series 60 3.0
    * @param none.
    * @return None.
    */
    void DoCancel();

private: // New functions

    /**
    * Veryfies that seek header and array are valid.
    * @since Series 60 3.0
    * @param none.
    * @return true if are, otherwise false.
    */
    TBool ValidSeekHeaderL();

    /**
    * Scans whole clip and updates seek array for it.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void UpdateSeekArrayL();

    /**
    * Activates asyncronous group header reading.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void ReadNextGroupHeaderFromFileL();

    /**
    * Appends rec groups to seek array in 30s interval.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void GroupToSeekArrayL();

    /**
    * Adds special packet to the end.
    * @since Series 60 3.0
    * @param aType a type of special packet.
    * @return none.
    */
    void AddSpecialPacketL( const MRtpFileWriteObserver::TRtpType aType );

    /**
    * Saves readed seek array.
    * @since Series 60 3.0
    * @param aError a error code.
    * @return none.
    */
    void FinalizeSeekArrayL( const TInt aError );

private: // Data

    /**
    * Repair observer.
    */
    MRtpClipRepairObserver* iObs;

    /**
    * File data buffer.
    */
    HBufC8* iFileData;
    
    /**
    * Meta header.
    */
    CRtpMetaHeader* iMetaHeader;
    
    /**
    * Last group time.
    */
    TUint iLastGroupTime;
    
    /**
    * Seek array point.
    */
    TInt iSeekArrayPoint;
        
  };

#endif // CRTPCLIPREPAIRER_H

// End of File
