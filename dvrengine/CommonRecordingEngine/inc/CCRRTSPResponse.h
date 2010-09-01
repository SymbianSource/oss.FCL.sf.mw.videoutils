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
* Description:    Class for parsing and producing a response string of RTSP as rfc2326*
*/




#ifndef CCRRTSPRESPONSE_H
#define CCRRTSPRESPONSE_H

// INCLUDES
#include "CRRtspCommon.h"
#include <e32base.h>

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
* Class for parsing and producing RTSP responses.  
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRRtspResponse : public CCRRtspCommon
    {

public: // Data types

    /**
    * RTSP response code. From RFC 2326
    */
    enum TResponseCode
        {
    	ERTSPRespContinue = 100, 
        ERTSPRespOK = 200,                 /**< OK code, everything ok */
        ERTSPRespCreated = 201,            /**< Same as 200 */
        ERTSPRespLowOnStorageSpace = 250, 
        ERTSPRespMultipleChoices = 300, 
        ERTSPRespMovedPermanently = 301, 
        ERTSPRespMovedTemporarily = 302, 
        ERTSPRespSeeOther = 303, 
        ERTSPRespNotModified = 304, 
        ERTSPRespUseProxy = 305, 
        ERTSPRespBadRequest = 400,  
        ERTSPRespUnauthorized = 401 ,      /**< We must re-issue command with authentication headers */
        ERTSPRespPaymentRequired = 402, 
        ERTSPRespForbidden = 403, 
        ERTSPRespNotFound = 404, 
        ERTSPRespMethodNotAllowed = 405,
        ERTSPRespNotAcceptable = 406, 
        ERTSPRespProxyAuthenticationRequired = 407 , /**< We must re-issue command with authentication headers */
        ERTSPRespRequestTimeOut = 408, 
        ERTSPRespGone = 410, 
        ERTSPRespLengthRequired = 411, 
        ERTSPRespPreconditionFailed = 412, 
        ERTSPRespRequestEntityTooLarge = 413, 
        ERTSPRespRequestURITooLarge = 414, 
        ERTSPRespUnsupportedMediaType = 415, 
        ERTSPRespParameterNotUnderstood = 451, 
        ERTSPRespConferenceNotFound = 452,
        ERTSPRespNotEnoughBandwidth = 453,
        ERTSPRespSessionNotFound = 454,
        ERTSPRespMethodNotValidInThisState = 455,
        ERTSPRespHeaderFieldNotValidForResource = 456,
        ERTSPRespInvalidRange = 457,
        ERTSPRespParameterIsReadOnly = 458,
        ERTSPRespAggregateOperationNotAllowed = 459,
        ERTSPRespOnlyAggregateOperationAllowed = 460,
        ERTSPRespUnsupportedTransport = 461,
        ERTSPRespDestinationUnreachable = 462,
        ERTSPRespInternalServerError = 500,
        ERTSPRespNotImplemented = 501,
        ERTSPRespBadGateway = 502,
        ERTSPRespServiceUnavailable  = 503,
        ERTSPRespGatewayTimeOut = 504,
        ERTSPRespRTSPVersionNotSupported = 505, 
        ERTSPRespOptionNotSupported = 551
        };

    /**
    * Structure for storing rtp-info header contents. 
    * if len of iFirstURL is zero, then the header has
    * not been seen or parsed. Normally rtp-info-header
    * has 2 urls and seq+ts for both. 
    */
    struct SRTPInfoHeader
        {
	    TPtrC8 iFirstURL ; /**< URL string of first url found from header */
	    TUint iFirstSeq ;  /**< Seq of first URL */
	    TUint iFirstTS ;   /**< TS of first URL */
	    TPtrC8 iSecondURL ;/**< URL string of second url found from header */
	    TUint iSecondSeq ; /**< Seq of second URL */ 
	    TUint iSecondTS ;  /**< TS of second URL */
        }; 

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @return CCRRtspResponse pointer to CCRRtspResponse class
    */
    static CCRRtspResponse* NewL();

    /**
    * Destructor.
    */
    virtual ~CCRRtspResponse();

public: // New functions

	/**
	* Method that is used to give rtsp response to this class.
	* Response is something alike "RTSP/1.0 200 OK\rCSeq: 200000\r..."
    * @since Series 60 3.0
	* @param aString is the response string.
	* @return none. May leave with symbian error codes. At least following may be 
	*         seen: KErrUnderflow if there is \r\n\r\n sequence in the string
	*         marking end of response or if there is content (like SDP) and the
	*         content lenght is too short; KErrNotSupported if it doesn't look
	*         like RTSP at all.
	*/
	virtual void TryParseL( const TDesC8 &aString );

	/**
	* Method for acquiring the response code from command string that was successfully
	* parsed. If TryParseL did leave, this may return anything. 
    * @since Series 60 3.0
	* @param none.
	* @return response code, like ERTSPRespOK if it was 200 OK. 
	*/		
	CCRRtspResponse::TResponseCode StatusCode( void );

	/** 
	* Method for getting server port.
    * @since Series 60 3.0
	* @param none.
	* @return server port of KErrNotFound if none found.
	*/
	TInt ServerPort( void ); 
	
	/**
	* Method for getting SSRC.
    * @since Series 60 3.0
	* @param aSSRC is string descriptor allocated by caller that
	*        will have its content set to block allocated by instance
	*        of this class containing the SSRC.
	* @return KErrNone if no error, KErrNotFound if SSRC is not there.
	*/
	TInt SSRC( TPtrC8& aSSRC );
			
			
	/**
	* Method for getting RTP-Info header.
    * @since Series 60 3.0
	* @param aRTPInfoHeader is header struct allocated by caller that
	*        will have its content set to strings (and numbers) allocated
	*        by instance of this class so once instance of this class
	*        is deleted, it is not wise to try referring to those 
	*        values any more. 
	* @return KErrNone if no error, 
	          KErrNotFound if rtp-info header is not there.
	*/
	TInt RTPInfoHeader( CCRRtspResponse::SRTPInfoHeader &aRTPInfoHeader ); 
	
protected: // New Methods
	
	/**
	* Method for digging out server ports from (setup) reply
    * @since Series 60 3.0
	* @param none.
	* @return none.
	*/
	void FindServerPorts( void ); 
	
	/**
	* Method for digging out SSRC from (setup) reply
    * @since Series 60 3.0
	* @param none.
	* @return none.
	*/
	void FindSSRC( void );
	
	/**
	* Method for digging out and partially  parsing RTP-Info -header
    * @since Series 60 3.0
	* @param none.
	* @return none.
	*/
	void FindRTPInfoHeader( void ); 
	
	/**
	* Method for digging out and partially  parsing RTP-authentication -header
    * @since Series 60 3.0
	* @param none.
	* @return none.
	*/
	void FindRTPAuthenticationL( void );
	
private: // Constructors and destructors

	/**
	 * default constructor
	 */
	 CCRRtspResponse( void ); 
	 
	/** 
	 * 2nd phase constructor 
	 */
	 void ConstructL();
	   
private: // Data types
	
	/**
	* Code how the command went.
	*/
	TResponseCode iStatusCode; 
			
	/**
	* Possible server port.
	*/
	TInt iServerPort; 
	
	/**
	* Possible synchronization source id.
	*/
	TPtrC8 iSSRC;
	
	/**
	* Possible contents of rtp info header.
	*/
	SRTPInfoHeader iRTPInfoHeader; 

    };

#endif // CCRRTSPRESPONSE_H

// End of file

