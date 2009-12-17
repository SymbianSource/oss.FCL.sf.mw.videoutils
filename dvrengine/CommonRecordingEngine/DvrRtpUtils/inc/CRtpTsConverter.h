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
* Description:    Class to provide TS conversion for an RTP-stream based on*
*/





#ifndef CRTPTSCONVERTER_H
#define CRTPTSCONVERTER_H

//  INCLUDES
#include <RtpHeader.h>

// CONSTANTS
const TUint KPipelineClockRate( 1000 );

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION

/**
*  Class for converting RTP-timestamps.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CRtpTsConverter : public CBase
    {

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @param aClockRate the clockrate for the media stream.
    */
    IMPORT_C static CRtpTsConverter* NewL( const TUint aClockRate );
    
    /**
    * Destructor.
    */
    IMPORT_C virtual ~CRtpTsConverter();

public: // New functions

    /**
    * Initialize timestamp converter
    * @since Series 60 3.0
    * @param aRtcpPkt a RTCP packet for the media stream to be sychronized.
    * @return none.
    */
    IMPORT_C void Init( const TDesC8& aRtcpPkt );
    
    /**
    * Initialize timestamp converter
    * @since Series 60 3.0
    * @param aTs intial value of RTP timestamp
    * @return none.
    */
    IMPORT_C void Init( const TUint& aTs );

    /**
    * Gets status on initialization.
    * @since Series 60 3.0
    * @param none.
    * @return true if converter initialized, otherwise false.
    */
    IMPORT_C TBool Initiated();

    /**
    * Uninitializes the converter.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    IMPORT_C void UnInitiate();

    /**
    * Gets time stamp from a RTP packet.
    * @since Series 60 3.0
    * @param aTimestamp a RTP packet's TS to convert.
    * @param aUseWallClock if true converts to wall clock time.
    * @return a time stamp value.
    */
    IMPORT_C TUint ConvertTs( const TUint aTimestamp,
                              const TBool aUseWallClock = EFalse );

private: // Constructors and destructor

    /**
    * By default Symbian 2nd phase constructor is private.
    */
    void ConstructL();

    /**
    * C++ default constructor.
    * @param aClockRate the clockrate for the media stream.
    */
    CRtpTsConverter( const TUint aClockRate );


private: // New functions

    /**
    * Calculate the offset for adjusting the RTP timestamps.
    * @since Series 60 3.0
    * @param aRtpTime a RTP time stamp.
    * @param aSeekTime a wall clock time.
    * @param clockRate a wall clock time.
    * @return a offset for RTP TS adjustment.
    */
    TUint ComputeOffset( const TUint aRtpTime,
                         const TUint aSeekTime,
                         const TInt clockRate );

private: // Data

    /**
    * Clockrate.
    */
    const TUint iClockRate;
    
    /**
    * Offset.
    */
    TUint iOffset;

    /**
    * Initiated.
    */
    TBool iInitiated;
        
    };

#endif // CRTPTSCONVERTER_H

// End of File
