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
* Description:    Base methods/variables for RTP file format recording.*
*/




#ifndef CRTPFILEBASE_H
#define CRTPFILEBASE_H

// INCLUDES
#include <ipvideo/CRtpUtil.h>
#include <f32file.h>

// CONSTANTS
const TInt KCurrentClipVersion( 3 );
const TInt KMinValidClipVersion( 2 );
const TInt KSeekHeaderBytes( 3 * KIntegerBytes );
const TInt KGroupHeaderBytes( 4 * KIntegerBytes );
const TInt KPacketsCountBytes( 4 );
const TInt KNormalRecGroupLength( 2000 ); // 2 s
const TUint KSeekArrayInterval( 30000 );  // 30 s

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION

/**
*  RTP format common functionalities.
*
*  @lib RtpClipHandler.lib
*  @since Series 60 3.0
*/
class CRtpFileBase : public CActive
    {

public: // Data types

    // Time shift seek array
    class STimeShiftSeek 
        {

    public: // Data

        /**
        * Group time.
        */
        TUint iGroupTime;
        
        /**
        * File seek point.
        */
        TInt iSeekpoint;

        /**
        * File name index.
        */
        TInt iNameIndex;
        
        };


public: // Constructors and destructor
    
    /**
    * Destructor.
    */
    virtual ~CRtpFileBase();

protected: // Constructors and destructor

    /**
    * C++ default constructor.
    */
    CRtpFileBase();

    /**
    * Symbian 2nd phase constructor can leave and is private by default.
    */
    void ConstructL();
    
protected: // Enumeration

    // Defines RTP file mode
    enum TRtpFileMode
        {
        EModeNone = KErrBadHandle,
        EModeNormal = KErrNone,
        EModeHandle,
        EModeTimeShift
        };

public: // New functions

    /**
    * Deletes time shift files.
    * @since Series 60 3.0
    * @param aShiftSeek a reference to time shift seek array.
    * @return None.
    */
    void DeleteTimeShiftFiles( RArray<STimeShiftSeek>& aShiftSeek );

protected: // New functions

    /**
    * Writes RTP seek header to a file.
    * @since Series 60 3.0
    * @return None.
    */
    void WriteSeekHeaderL();

    /**
    * Reads clip's seek header from a file.
    * @since Series 60 3.0
    * @return None.
    */
    void ReadSeekHeaderL();

    /**
    * Reads clip's group header from a file.
    * @since Series 60 3.0
    * @return None.
    */
    void ReadGroupHeaderL();

    /**
    * Updates group header variables from readed data.
    * @since Series 60 3.0
    * @param aDataPtr data from which header is updated.
    * @return None.
    */
    void UpdateGroupHeaderVariablesL( const TDesC8& aDataPtr );

    /**
    * Appends one group to the seek array.
    * @since Series 60 3.0
    * @param aTime a TS time of first packet in group.
    * @param aPoint a seek point in clip for group.
    * @return None.
    */
    void AppendSeekArrayL( const TUint aTime,
                           const TInt aPoint );

    /**
    * Saves seek array to the clip.
    * @since Series 60 3.0
    * @return None.
    */
    void SaveSeekArrayL();

    /**
    * Reads seek array from the clip.
    * @since Series 60 3.0
    * @param aPoint a seek array point in clip.
    * @return true if items in seek array, otherwise false.
    */
    TBool ReadSeekArrayL( const TInt aPoint );

    /**
    * Resets seek array.
    * @since Series 60 3.0
    * @return None.
    */
    inline void ResetSeekArray();

    /**
    * Writes variables to log file.
    * @since Series 60 3.0
    * @param aMethod a method which called this
    */
    void LogVariables( const TDesC& aMethod );

private: // Functions from base classes

    /**
    * From CActive : Called when request completion event occurs.
    * @since Series 60 3.0
    * @return None.
    */
    virtual void RunL() = 0;

    /**
    * From CActive : Handles a leave occurring in the request
    *                completion event handler RunL().
    * @since Series 60 3.0
    * @param aError the leave code
    * @return status of run error handling
    */
    virtual TInt RunError( TInt aError ) = 0;

    /**
    * From CActive : Called when request completion event cancelled.
    * @since Series 60 3.0
    * @return None.
    */
    virtual void DoCancel() = 0;

protected: // Data types
    
    // Defines seek struct
    class SSeek
        {

    public: // Data

        /**
        * Enlapsed time.
        */
        TUint iTime;

        /**
        * Point in clip.
        */
        TInt iPoint;
        
        };

protected: // Data
    
    /**
    * Rtp file usage mode.
    */
    TRtpFileMode iMode;
    
    /**
    * Current group.
    */
    TInt iThisGroup;
    
    /**
    * Packet group counter (Seek header).
    */
    TInt iGroupsTotalCount;
    
    /**
    * First seek address (Seek header).
    */
    TInt iFirstSeekAddr;
    
    /**
    * Last seek address (Seek header).
    */
    TInt iLastSeekAddr;
    
    /**
    * Group total length (Group header).
    */
    TInt iGroupTotalLen;
    
    /**
    * Next group point (Group header).
    */
    TInt iNextGroupPoint;
    
    /**
    * Previous group point (Group header).
    */
    TInt iPrevGroupPoint;
    
    /**
    * Group time (Group header).
    */
    TUint iGroupTime;
    
    /**
    * Seek header point.
    */
    TInt iSeekHeaderPoint;

    /**
    * Current clip path.
    */
    HBufC* iCurrentPath;
    
    /**
    * Seek array.
    */
    CArrayFix<SSeek>* iSeekArray;

    /**
    * File data buffer pointer.
    */
    TPtr8 iDataPtr;
    
    /**
    * File server.
    */
    RFs iFs;
    
    /*
    * File operations.
    */
    RFile iFile;
    
  };

#include <ipvideo/CRtpFileBase.inl>

#endif // CRTPFILEBASE_H

// End of File
