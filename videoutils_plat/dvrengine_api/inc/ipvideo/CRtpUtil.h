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
* Description:    Convertion methods for RTP file format.*
*/




#ifndef CRTPUTIL_H
#define CRTPUTIL_H

// INCLUDES
#include <e32base.h>
#include <etelmm.h>

// CONSTANTS
const TInt KSiKilo( 1000 );
const TInt KIntegerBytes( 4 );
const TInt KPacketSizeBytePoint( 0 );
const TInt KPacketSizeBytesLen( 4 );
const TInt KPacketTypeBytesLen( 1 );
const TInt KSpecialPacketLength( KPacketSizeBytesLen + KPacketTypeBytesLen + KIntegerBytes );
const TInt KPacketTypeBytePoint( KPacketSizeBytePoint + KPacketSizeBytesLen );
const TInt KUserIdLength( RMobilePhone::KIMSISize );
const TInt KDeviceIdLength( RMobilePhone::KPhoneSerialNumberSize );

_LIT8( KRtpClipMimetype, "application/x-nokia-teh-rtp" );
_LIT( KDvrTimeShiftFile, "c:\\timeshift." );

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION

/**
*  RTP format common functionalities for Common Recording Engine.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CRtpUtil : public CBase
    {

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    */
    IMPORT_C static CRtpUtil* NewL();

    /**
    * Two-phased constructor.
    */
    IMPORT_C static CRtpUtil* NewLC();

    /**
    * Destructor.
    */
    IMPORT_C virtual ~CRtpUtil();

public: // New functions

    /**
    * Converts integer to a buffer.
    * @since Series 60 3.0
    * @param aValue a value to write.
    * @return a buffer where value stored.
    */
    IMPORT_C static HBufC8* MakeBytesLC( const TInt& aValue );

    /**
    * Converts integer to a buffer.
    * @since Series 60 3.0
    * @param aValue a value to write.
    * @param aBuf a buffer where value stored.
    * @return none.
    */
    IMPORT_C static void MakeBytesL( const TInt& aValue,
                                     TDes8& aBuf );

    /**
    * Converts integer to a buffer.
    * @since Series 60 3.0
    * @param aValue a value to write.
    * @param aBuf a buffer where value stored.
    * @return a system wide error code.
    */
    IMPORT_C static TInt MakeBytes( const TInt& aValue,
                                    TDes8& aBuf );

    /**
    * Converts bytes to integer.
    * @since Series 60 3.0
    * @param aBytes a buffer to convert.
    * @return an integer converted from bytes.
    */
    IMPORT_C static TInt GetValueL( const TDesC8& aBytes );

    /**
    * Converts bytes to integer.
    * @since Series 60 3.0
    * @param aBytes a buffer to convert.
    * @param aValue a integer converted from bytes.
    * @return none.
    */
    IMPORT_C static void GetValueL( const TDesC8& aBytes,
                                    TInt& aValue );

    /**
    * Converts bytes to integer.
    * @since Series 60 3.0
    * @param aBytes a buffer to convert.
    * @param aValue a integer converted from bytes.
    * @return a system wide error code.
    */
    IMPORT_C static TInt GetValue( const TDesC8& aBytes,
                                   TInt& aValue );

    /**
    * Converts bytes to integer.
    * @since Series 60 3.0
    * @param aBytes a buffer to convert.
    * @param aValue a integer converted from bytes.
    * @return a system wide error code.
    */
    IMPORT_C static TInt GetValue( const TDesC8& aBytes,
                                   TUint& aValue );

    /**
    * Generates special RTP packet.
    * @since Series 60 3.0
    * @param aType a type of RTP packet.
    * @return a buffer containing generated RTP packet.
    */
    IMPORT_C static HBufC8* SpecialPacketL( const TInt aType );
        
    /**
    * Getter for RTP clip mime type info.
    * @since Series 60 3.0
    * @param aBuf on return contains the mime type.
    * @return None.
    */
    IMPORT_C static void GetMimeInfo( TDes& aMime );
    
    /**
    * Getter for RTP clip mime type info.
    * @since Series 60 3.0
    * @param aBuf on return contains the mime type.
    * @return None.
    */
    IMPORT_C static void GetMimeInfo( TDes8& aMime );
    
    /**
    * Reads IMEI of the phone HW.
    * @since Series 60 3.0
    * @param aBuf on return contains the IMEI.
    * @return none.
    */
    IMPORT_C static void GetImeiL( TDes& aImsi );

private: // Constructors and destructor

    /**
    * C++ default constructor.
    */
    CRtpUtil();

    /**
    * Symbian 2nd phase constructor can leave and is private by default.
    */
    void ConstructL();
  	    
private: // New methods

    /**
    * Reads mobile info of the phone.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void static GetMobilePhoneInfoL( RTelServer& aServer,
                                     RMobilePhone& aPhone );

  };

#endif // CRTPUTIL_H

// End of File
