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
* Description:    Class to parse RTP packets.*
*/




#ifndef CRTPPACKET_H
#define CRTPPACKET_H

// INCLUDES
#include <RtpHeader.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION

/**
*  Class for RTP header.
*
*  @lib RtpClipHandler.lib
*  @since Series 60 3.0
*/
class CRtpPacket : public CBase 
    {

public: // Constructors and destructor
        
    /**
    * Two-phased constructor.
    * @return a pointer to the newly created RTP packet object.
    */
    IMPORT_C static CRtpPacket* NewL();
    
    /**
    * Two-phased constructor.
    * @return a pointer to the newly created RTP packet object.
    */
    IMPORT_C static CRtpPacket* NewLC();

    /**
    * Destructor.
    */
    IMPORT_C virtual ~CRtpPacket();

public: // New functions

    /**
    * Parse RTCP packet buffer to retrive RTCP fields.
    * @since Series 60 3.0
    * @param aPktBuf a RTP packet.
    * @return system wide error code.
    */
    IMPORT_C TInt ParseRtp( const TDesC8& aPktBuf );
    
    /**
    * Sets syncronised time stamp of packet.
    * @since Series 60 3.0
    * @param aTs a new timestamp value.
    * @return none.
    */
    IMPORT_C void SetTimeStamp( const TUint32 aTs );
    
private: // Constructors and destructor

    /**
    * By default Symbian 2nd phase constructor is private.
    */
    void ConstructL();

    /**
    * C++ default constructor.
    * @since Series 60 3.0
    */
    CRtpPacket();

private: // New functions

    /**
    * Read 32 bits from current offset.
    * @since Series 60 3.0
    * @param aPktBuf a packet buffer.
    * @param aByte a byte index.
    * @return an readed unsigned integer.
    */
    TUint Read32Bits( const TPtrC8& aPktBuf,
                      TInt& aByte );

public: // Data

    /**
    * Header of the packet.
    */
    TRtpRecvHeader iRtpRecvHeader;

    /**
    * Payload off the packet.
    */
    TPtrC8 iPayload;

    };

#endif // CRTPPACKET_H

// End of File
