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
* Description:    Implementation of RTP file recognizer class.*
*/




#ifndef CRTPCLIPRECOGNIZER_H
#define CRTPCLIPRECOGNIZER_H

// INCLUDES
#include <apmrec.h>
#include <etelmm.h>

// CONSTANTS
const TInt KUserIdLength( RMobilePhone::KIMSISize );
const TInt KDeviceIdLength( RMobilePhone::KPhoneSerialNumberSize );

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION

/**
*   Class to make recognisation for RTP propriatary clip.
*  
*  @lib RtpClipRecognizer.lib
*  @since Series 60 3.0
*/
class CRtpClipRecognizer : public CApaDataRecognizerType
    {

public: // Constructors and destructor

    /**
    * Static method to create instance of this recognizer. This method is called
    * by the framework. Method pointer is delivered to the framework by
    * ImplementationTable table returned by ImplementationGroupProxy.
    * @since Series 60 3.0
    * @param none.
    * @return Pointer to newly created instance of the recognizer.
    */
    static CApaDataRecognizerType* CreateRecognizerL();
    
    /**
    * C++ default constructor. Calls CApaDataRecognizerType in it's initializer
    * list to complete construction.
    */
    CRtpClipRecognizer();
    
private: // Functions from base classes

    /**
    * From CApaDataRecognizerType.
    * Method called by the framework. This method returns the amount of data
    * to be wanted for recognisation.
    * @since Series 60 3.0
    * @param none.
    * @return Preferred buffer size for recognisation.
    */
    virtual TUint PreferredBufSize();
    
    /**
    * From CApaDataRecognizerType.
    * Method to deliver supported data types by this recognizer.
    * @since Series 60 3.0
    * @param aIndex a index pointing out which data type
    *        of the supported ones is returned.
    * @return Corresponding data type of the requested index.
    */
    virtual TDataType SupportedDataTypeL( TInt aIndex ) const;

    /**
    * From CApaDataRecognizerType.
    * Overwritten method to handle recognisation.
    * @param aName the name of the data. Typically this is a file name
    *        containing the data to be recognized.
    * @param aBuffer a buffer containing PreferredBufSize() from the
    *        beginning of the file to be recognized.
    * @return none.
    */
    virtual void DoRecognizeL( const TDesC& aName,
                               const TDesC8& aBuffer );
    
private: // New methods

    /**
    * Converts bytes to integer.
    * @since Series 60 3.0
    * @param aBytes a buffer to convert.
    * @return an integer converted from bytes.
    */
    TInt GetValueL( const TDesC8& aBytes );

    /**
    * Converts bytes to integer.
    * @since Series 60 3.0
    * @param aBytes a buffer to convert.
    * @param aValue a integer converted from bytes.
    * @return none.
    */
    void GetValueL( const TDesC8& aBytes, TInt& aValue );

    /**
    * Converts bytes to integer.
    * @since Series 60 3.0
    * @param aBytes a buffer to convert.
    * @param aValue a integer converted from bytes.
    * @return a system wide error code.
    */
    TInt GetValue( const TDesC8& aBytes, TInt& aValue );

    /**
    * Converts bytes to integer.
    * @since Series 60 3.0
    * @param aBytes a buffer to convert.
    * @param aValue a integer converted from bytes.
    * @return a system wide error code.
    */
    TInt GetValue( const TDesC8& aBytes, TUint& aValue );

    /**
    * Converts bytes to 64 bit integer (TInt64).
    * @since Series 60 3.0
    * @param aBytes a buffer to convert.
    * @return a integer value converted from bytes.
    */
    TInt64 GetTInt64L( const TDesC8& aBytes );

    /**
    * Getter for RTP clip mime type info.
    * @since Series 60 3.0
    * @param aBuf on return contains the mime type.
    * @return None.
    */
    void GetMimeInfo( TDes8& aMime );

    /**
    * Reads IMEI of the phone HW.
    * @since Series 60 3.0
    * @param aBuf on return contains the IMEI.
    * @return none.
    */
    void GetImeiL( TDes& aImsi );
    
    /**
    * Reads mobile info of the phone.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void GetMobilePhoneInfoL( RTelServer& aServer,
                              RMobilePhone& aPhone );

private: // Data

    TBuf8<KDeviceIdLength> iImei;
    
    };

#endif // CRTPCLIPRECOGNIZER_H

//  End of File
