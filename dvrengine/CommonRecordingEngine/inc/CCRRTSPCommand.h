/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    Class for parsing and producing a command string*
*/




#ifndef CCRRTSPCOMMAND_H
#define CCRRTSPCOMMAND_H

// INCLUDES
#include "CRRtspCommon.h"
#include <Hash.h>

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
*  Class that parses and produces rtsp commands as in rfc2326.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRRtspCommand : public CCRRtspCommon
    {
    
public: // Data types 

    /**
    * Enum for subset of RTSP commands 
    */
    enum TCommand
        {
		ERTSPCommandPLAY = 0,  /**< Command for triggering clip that has been SETUP */
        ERTSPCommandPAUSE,     /**< Command for pausing a clip that is already  playing */
        ERTSPCommandTEARDOWN,  /**< Command for quitting a session */
        ERTSPCommandOPTIONS,   /**< No operation, except ping */
        ERTSPCommandDESCRIBE,  /**< Command for acquiring an SDP */
        ERTSPCommandSETUP,     /**< Command for setting up audio or video stream */
        ERTSPCommandNOCOMMAND  /**< init value, no-operation */
        };    

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @return CCRRtspCommand pointer to CCRRtspCommand class.
    */
    static CCRRtspCommand* NewL();

    /**
    * Destructor.
    */
    virtual ~CCRRtspCommand( );

public: // New functions

    /**
    * Method that is used to give rtsp command to this class.
    * command is something alike:
    * "PLAY rtsp://vishnu.wipsl.com/oops/winter_fun.mp4/ RTSP/1.0\r\n..."
    * @since Series 60 3.0
    * @param aString is the response string
    * @return none. 
    *         May leave with symbian error codes. At least following may be 
    *         seen: KErrUnderflow if there is \r\n\r\n sequence in the string
    *         marking end of response or if there is content (like SDP) and the
    *         content lenght is too short; KErrNotSupported if it doesn't look
    *         like RTSP at all.
    */
    void TryParseL( const TDesC8 &aString );

    /**
    * Method for getting URL. 
    * @since Series 60 3.0
    * @param aURL is string descriptor allocated by caller that
    *        will have its content set to block allocated by instance
    *        of this class containing the session id. 
    * @return KErrNone if no error, KErrNotFound if URL is not there.
    */
    TInt URL( TPtrC8& aURL ); 
            
    /**
    * Method for setting URL.
    * @since Series 60 3.0
    * @param aURL is string descriptor allocated by caller. 
    * @return none.
    */
    void SetURL( const TDesC8& aURL ); 

    /**
    * Method for setting command.
    * @since Series 60 3.0
    * @param aCommand is the command that will be set.
    * @return none.
    */
    void SetCommand( TCommand aCommand  ); 
    
    /**
    * Method for setting authentication info.
    * @since Series 60 3.0
    * @param aAuth is ETrue if authentication is to be used.
    *        When constructing command string, authentication headers
    *        will be added. See also SetUserNameL et al. 
    * @return none.
    */
    void SetAuthentication( TBool aAuth  ); 

    /**
    * Sets identification string to be sent as 'UserAgent' header.
    * No header is sent if not identification is not set.
    * @since Series 60 3.0
    * @param aUserAgent user agent identification.
    * @return none.
    */
    void SetUserAgentL( const TDesC8& aUserAgent );

    /**
    * Sets connection bandwidth to be sent as 'Bandwidth' header.
    * No header is sent if bandwidth is not set.
    * @since Series 60 3.0
    * @param aBandwidth connection bandwidth in bit/s.
    * @return none.
    */
    void SetBandwidth( TInt aBandwidth );

    /**
    * Sets profile identification for 'x-wap-profile' header.
    * @since Series 60 3.0
    * No header is sent if identification is not set.
    * @param aWapProfile profile identification.
    * @return none.
    */
    void SetWapProfileL( const TDesC8& aWapProfile );

    /**
    * Method for getting command.
    * @since Series 60 3.0
    * @param none.
    * @return the command.
    */
    CCRRtspCommand::TCommand Command( void ) const; 
        
    /**
    * Method that does opposite of parse: it produces a RTSP command.
    * @since Series 60 3.0
    * @param none.
    * @return string descriptor containing the command
    */
    TPtrC8& ProduceL( void ); 

    /**
    * Appends string to the buffer.
    * @since Series 60 3.0
    * @param aBuffer a buffer where to add text.
    * @param aTxt a string to append.
    * @return none.
    */
    void AppendL( HBufC8*& aBuffer,
                  const TDesC8& aTxt );

    /**
    * Appends integer value as text to the buffer.
    * @since Series 60 3.0
    * @param aBuffer a buffer where to add text.
    * @param aNum a numerical value to append.
    * @return none.
    */
    void AppendNumL( HBufC8*& aBuffer,
                     const TInt aNum );

    /**
    * Appends formatted text to the buffer.
    * @since Series 60 3.0
    * @param aBuffer a buffer where to add text.
    * @param aFmt a format string to use.
    * @return none.
    */
    void AppendFormatL( HBufC8*& aBuffer,
                        TRefByValue<const TDesC8> aFmt, ... );

private: // Constructors and destructors

    /**
     * default constructor
     */
     CCRRtspCommand(); 
     
    /** 
     * 2nd phase constructor 
     */
     void ConstructL();
    
private: // new methods:

    /** 
    * Method for finding the URL this command is about
    * @since Series 60 3.0
    * @param none.
    * @return none but will leave with KErrNotSupported 
    *         if it doesn't look like RTSP
    */
    void FindURLL( void ); 

    /**
    * Generates authentication header iAuthHeader using given values.
    * Method described in RFC 2069 "An Extension to HTTP : Digest Access 
    * Authentication".
    * @since Series 60 3.0
    * @param aMethod Method name (e.g. "DESCRIBE" / "SETUP" / etc.)
    */
    void CalculateDigestResponseL( const TDesC8& aMethod ); 
    
    /**
    * Generates authentication header iAuthHeader using given values.
    * Method described in RFC 2617 "HTTP Authentication: Basic and Digest 
    * Access Authentication".
    * @since Series 60 3.0
    * @param aMethod Method name (e.g. "DESCRIBE" / "SETUP" / etc.)
    */
    void CalculateBasicResponseL( const TDesC8& aMethod );
    
    /**
    * Calculates MD5 hash of the message.
    * @since Series 60 3.0
    * @param aMessage Message.
    * @param aHash On return, contains message hash.
    */
    void HashL( const TDesC8& aMessage, TDes8& aHash );  
       
private: // Data

    /** 
    * What command this is.
    */
    TCommand iCommand; 
    
    /**
    * What is the URL that this command is about.
    */
    TPtrC8 iURL; 
    
    /**
    * String descriptor that we retur with ProduceL.
    */
    TPtrC8 iProductDescriptor; 
    
    /**
    * Authentication header.
    */
    HBufC8* iAuthHeader;
        
    /**
    * MD5 message digest class. Required for HTTP digest authentication.
    */
    CMD5* iMD5Calculator;
    
    /**
    * If authentication is needed.
    */
    TBool iAuthenticationNeeded;

    /**
    * User Agent header if present.
    */
    HBufC8* iUserAgent;

    /**
    * Connection bandwidth in bit/s for 'Bandwidth' header.
    */
    TInt iBandwidth;

    /**
    * Bandwidth present.
    */
    TBool iBandwidthAvailable;

    /**
    * x-wap-profile if present.
    */
    HBufC8* iWapProfile;
    
    };

#endif // CCRRTSPCOMMAND_H

// End of file
