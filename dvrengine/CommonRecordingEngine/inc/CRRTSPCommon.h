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
* Description:    Common definitions for rtsp implementation*
*/




#ifndef CCRRTSPCOMMON_H
#define CCRRTSPCOMMON_H

// INCLUDES
#include <e32base.h>
#include <in_sock.h>

// CONSTANTS
/** Max len for synchronization source id texttual presentation. SSRC is 32-bit number 
 *  so this needs to be able to hold 2^^32 numerical value */
const TInt KSSRCMaxLen( 30 );
/** Max expected RTP packet len, given to symbian rtp stack that wants to know */
const TInt KAverageExpectedRtpPacketMaxSize( 1500 );

// following five are guestimates
/** Average expected audio packet size */
const TInt KAverageAudioPacketSize( 400 );
/** Average expected video packet size */
const TInt KAverageVideoPacketSize( 700 );
/** Average expected rtp control packet size */
const TInt KAverageRTPControlPacketSize( 30 );
/** Number of rtp packets in our buffer, max */
const TInt KRTPDataPacketBufferLen( 100 );   
/** Number of rtpc packets in our buffer, max */
const TInt KRTPControlPacketBufferLen( 20 ); 

// followint strings _MAY_NOT_ be localized. 
_LIT8( KCR2NewLines, "\r\n\r\n" );
_LIT8( KCRCSeq, "CSeq: " ); 
_LIT8( KCRRTSP10, "RTSP/1.0" ); 
_LIT8( KCROPTIONS, "OPTIONS " );
_LIT8( KCROPTIONSNoSpace, "OPTIONS" );
_LIT8( KCRDESCRIBE, "DESCRIBE " );
_LIT8( KCRDESCRIBENoSpace, "DESCRIBE" );
_LIT8( KCRTEARDOWN,"TEARDOWN " );
_LIT8( KCRTEARDOWNNoSpace,"TEARDOWN" );
_LIT8( KCRPAUSE,"PAUSE " ); 
_LIT8( KCRPAUSENoSpace,"PAUSE" ); 
_LIT8( KCRSETUP, "SETUP " ); 
_LIT8( KCRSETUPNoSpace, "SETUP" ); 
_LIT8( KCRClient_Port, "client_port=" );
_LIT8( KCRInterleaved, "interleaved=" );
_LIT8( KCRAcceptSDP, "Accept: application/sdp\r\n" ); 
_LIT8( KCRPort, "port=" ); 
_LIT8( KCRDestination, "destination=" ); 
_LIT8( KCRPLAY, "PLAY " );
_LIT8( KCRPLAYNoSpace, "PLAY" );
_LIT8( KCRRTSPReplyHeader, "RTSP/1.0 " );
_LIT8( KCRRTSPContentLength, "Content-length: " );
_LIT8( KCRRTSPDefaultUserAgent, "NokiaVideoCenter" );
_LIT8( KCRRTSPUserAgentHeader, "User-Agent: %S\r\n" );
_LIT8( KCRRTSPXWapProfile, "x-wap-profile: %S\r\n" );
_LIT8( KCRRTSPBandwidth, "Bandwidth: %d\r\n" );
_LIT8( KCRSessionStr, "Session: " );
_LIT8( KCRNewLine, "\r\n" );
_LIT8( KCRSpace, " " );
_LIT8( KCRSemiColon, ";" );
_LIT8( KCRDash, "-" );
_LIT8( KCRLineFeed, "\n" ); 
_LIT8( KCRSDPmLine, "m=" );
_LIT8( KCRSDPmLineIsVideo, "m=video" );
_LIT8( KCRSDPRTPAVP, "RTP/AVP" );
_LIT8( KCRSDPRTPAVPTCP, "RTP/AVP/TCP" );
_LIT8( KCRSDPRTPAVPUDP, "RTP/AVP/UDP" );
_LIT8( KCRSDPMulticast, "multicast" );
_LIT8( KCRSDPmLineIsAudio, "m=audio" );
_LIT8( KCRSDPcLine, "c=" );
_LIT8( KCRSDPbLine, "b=AS" );
_LIT8( KCRNlAndSession, "\r\nSession: " ); 
_LIT8( KCRRtpInfo, "RTP-Info: " ); 
_LIT8( KCRUrlStr, "url=" ); 
_LIT8( KCRSeqStr, "seq=" ); 
_LIT8( KCRRtptimeStr, "rtptime=" ); 
_LIT8( KCRServerPort, "server_port" );
_LIT8( KCRSSRC,  ";ssrc=" ); 
_LIT8( KCRTransportHeaderUDP, "Transport: RTP/AVP;unicast;"
                              "client_port=%d-%d;mode=play\r\n" );
_LIT8( KCRTransportHeaderTCP, "Transport: RTP/AVP/TCP;interleaved=%d-%d\r\n" );
_LIT8( KCRTransportHeaderMulticast, "Transport: RTP/AVP;multicast\r\n" );
_LIT8( KCRRangeHeader, "Range: npt=" ); 
_LIT8( KCRRangeHeaderLiveStream, "Range: npt=now-" ); 
_LIT8( KCRRtpMap, "a=rtpmap" ); 
_LIT8( KCROptionsReply,
       "RTSP/1.0 200 OK\r\nCseq: %d\r\n"
       "Public: DESCRIBE, SETUP, PAUSE, "
       "TEARDOWN, PLAY, OPTIONS\r\n\r\n" );
       
// Authentication strings
_LIT8( KCRAuthBasic, "Basic" );
_LIT8( KCRAuthDigest, "Digest" );
_LIT8( KCRAuthRealm, "realm=\"" );
_LIT8( KCRAuthNonce, "nonce=\"" );
_LIT8( KCRAuthOpaque, "opaque=\"" );
_LIT8( KCRAuthorizationHeader,
       "Authorization: Digest username=\"%S\", realm=\"%S\","
       " nonce=\"%S\", uri=\"%S\", response=\"%S\", opaque=\"%S\"\r\n" );
_LIT8( KCRAuthorizationHeaderNoOpaque,
       "Authorization: Digest username=\"%S\","
       " realm=\"%S\", nonce=\"%S\", uri=\"%S\", response=\"%S\"\r\n" );
_LIT8( KCRContentBaseHeader, "Content-Base: ") ; 
_LIT8( KCRAuthorizationBasicHeader, "Authorization: Basic %S\r\n" );

// MACROS
// None

// DATA TYPES
/** 
* TCRRTSPStage is used by both RTSP packet source and ROP sink. It is used
* to indicate the stage in RTSP negotiation where the packet source or rop sink
* currently is in
*/
enum TCRRTSPStage
    {
    ERTSPInit = 0,       /**< nothing done yet , must have numerical id 0 */
    ERTSPOptSent,        /**< Options command has been sent */
    ERTSPDescSent,       /**< Describe command has been sent */
    ERTSPDelayedSetup,   /**< Setup command need to be delayed */
    ERTSPSetupAudioSent, /**< Setup command for audio has been sent */
    ERTSPSetupVideoSent, /**< Setup command has video been sent */
    ERTSPReadyToPlay,    /**< Everything in order with remote server */
    ERTSPPlaySent,       /**< Play command has been sent */
    ERTSPPauseSent,      /**< Pause command has been sent */    
    ERTSPPlaying,        /**< Play command has been sent and 200 OK received */ 
    ERTSPTearDownSent,   /**< Teardown command has been sent */
    ERTSPLastStage       /**< This is not a real stage, keep this identifier as item, used for indexing */
    };

/**
 *  Enum indicating which transport to use for RTP streaming
 */
enum TCRRTPTransport
    {
    ERTPOverUDP = 0,     /**< stream RTP over UDP, default */
    ERTPOverTCP,         /**< stream RTP over TCP, interleaved in RTSP stream */
    ERTPOverMulticast    /**< stream RTP over multicast UDP */
    };

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION

/**
*  Base-class for RTSP commands and responses. 
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRRtspCommon : public CBase
    {

public: // Constructors and destructor

    /**
    * Destructor.
    */
    virtual ~CCRRtspCommon( );

public: // New functions

    /**
    * Method that is used to give rtsp response / command to this class.
    * @since Series 60 3.0
    * @param aString is the response string
    * @return none. May leave with symbian error codes. At least following may be 
    *         seen: KErrUnderflow if there is \r\n\r\n sequence in the string
    *         marking end of response or if there is content(like SDP) and the
    *         content lenght is too short; KErrNotSupported if it doesn't look
    *         like RTSP at all.
    */
    virtual void TryParseL( const TDesC8 &aString ) = 0;

    /**
    * Method for getting content len. KErrNotFound if no content. KErrNotReady
    * if TryParseL previously did leave
    * @since Series 60 3.0
    * @param none .
    * @return content len.
    */
    TInt ContentLen( void );       
    
    /**
    * Method for getting content. Most usually the content is SDP.
    * @since Series 60 3.0
    * @param none
    * @return tring descriptor allocated by this class so when
    *         this class instance gets deleted, the buffer
    *         content will point to invalid data. 
    */
    TPtrC8& Content( void ); 
    
    /**
    * Method for getting sequence number
    * @since Series 60 3.0
    * @param none.
    * @return CSeq. 
    */
    TInt CSeq( void ); 

    /**
    * Method for setting sequence number
    * @since Series 60 3.0
    * @param aCSeq is CSeq number. 
    * @return none. 
    */
    void SetCSeq( TInt aCSeq ); 

    /**
    * Method for getting session id. 
    * @since Series 60 3.0
    * @param aId is string descriptor allocated by caller that
    *        will have its content set to block allocated by instance
    *        of this class containing the session id. 
    * @return KErrNone if no error, KErrNotFound if session id is not there.
    */
    TInt SessionId( TPtrC8& aId ); 

    /**
    * Method for setting session id.
    * @since Series 60 3.0
    * @param aId is string descriptor allocated by caller that contains
    *        new session id and whose content(memory area pointed by pointer
    *        returned by Ptr() method call) will remain intact for the
    *        lifetime of instance of this class.
    * @return none.
    */
    void SetSessionId( TPtrC8& aId ); 
        
    /** 
    * Method for getting client port
    * @since Series 60 3.0
    * @param none.
    * @return client port of KErrNotFound if none found.
    */
    TInt ClientPort( void ); 
    
    /** 
    * Method for setting client port
    * @since Series 60 3.0
    * @param aPort is the client port number.
    * @return none.
    */
    void SetClientPort( TInt aPort );

    /**
    * Method for setting range 
    * @since Series 60 3.0
    * @param aLower is where to start from.
    * @param aUpper is where to stop. Negative values mean eternity.
    * @return none
    */      
    void SetRange( TReal aLower, TReal aUpper ); 

    /**
    * Method for getting range 
    * @since Series 60 3.0
    * @param aLower is where to start from.
    * @param aUpper is where to stop. Negative values mean eternity.
    * @return none.
    */      
    void GetRange( TReal& aLower, TReal& aUpper ); 

    /**
    * Method for parsing range header
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */      
    void ParseRange( void ); 
    
    /** 
    * Method for getting session opaque
    * @since Series 60 3.0
    * @param none.
    * @return reference to opaque or NULL.
    */
    TDesC8& OpaqueL( void );
    
    /** 
    * Method for getting session realm
    * @since Series 60 3.0
    * @param none.
    * @return reference to realm or NULL.
    */
    TDesC8& RealmL( void );
    
    /**
    * Method for getting authentication type.
    * @since Series 60 3.0
    * @param none.
    * @return Reference to auth type found from RTSP reply.
    */
    TDesC8& AuthenticationTypeL( void );
    
    /** 
    * Method for getting session Nonce
    * @since Series 60 3.0
    * @param none.
    * @return Reference to nonce found from RTSP reply.
    */
    TDesC8& NonceL( void );
    
    /**
    * Method for setting session nonce 
    * @since Series 60 3.0
    * @param aNonce is the nonce part of authenticated RTSP command/response.
    * @return none.
    */
    void SetNonceL( const TDesC& aNonce ); 
    
    /**
    * Mothod for setting authentication type
    * @since Series 60 3.0
    * @param aAuthType is the authentication type.
    * @return none.
    */ 
    void SetAuthenticationTypeL( const TDesC8& aAuthType );
   
    /**
    * Method for setting session opaque 
    * @since Series 60 3.0
    * @param aOpaque is the opaque part of authenticated response.
    * @return none.
    */
    void SetOpaqueL( const TDesC& aOpaque ); 
    
    /**
    * Method for setting session realm 
    * @since Series 60 3.0
    * @param aRealm is the realm part of authenticated rtsp command.
    * @return none.
    */
    void SetRealmL( const TDesC& aRealm );
    
    /**
    * Method for setting session nonce 
    * @since Series 60 3.0
    * @param aNonce is the new nonce that will be sent with the command.
    * @return none.
    */
    void SetNonceL( const TDesC8& aNonce ); 
    
    /**
    * Method for setting session opaque 
    * @since Series 60 3.0
    * @param aOpaque is the new opaque that will be sent with the command.
    * @return none.
    */
    void SetOpaqueL( const TDesC8& aOpaque ); 
    
    /**
    * Method for setting session realm 
    * @since Series 60 3.0
    * @param aRealm is the new realm that will be sent with the command.
    * @return none.
    */
    void SetRealmL( const TDesC8& aRealm );
    
    /**
    * Method for setting rtsp username. 
    * @since Series 60 3.0
    * @param aUserName is the new username
    * @return none.
    */
    void SetUserNameL( const TDesC& aUserName );
    
    /**
    * Method for setting session password.
    * @since Series 60 3.0
    * @param aPassWd is the password to use when generating digest for authenticated rtsp
    * @return none.
    */
    void SetPassWdL( const TDesC& aPassWd );
    
    /**
    * Method for setting session rtsp uri.
    * @since Series 60 3.0
    * @param aUri is the string that will in RTSP command be written between command and text RTSP/1.0 strings.
    * @return none.
    */
    void SetRtspUriL( const TDesC& aUri );

    /** 
    * Gets transport method.
    * @since Series 60 3.0
    * @param none.
    * @return transport method, default(ERTPOverUDP) if not present on command    
    */
    TCRRTPTransport Transport();

    /** 
    * Sets transport method.
    * @since Series 60 3.0
    * @param aTransport is the transport method to. This is used when constructing
    *        a SETUP command. 
    * @return none.
    */
    void SetTransport( TCRRTPTransport aTransport );

    /** 
    * Gets destination IP address if present
    * @since Series 60 3.0
    * @param none.
    * @return destination IP address or KAFUnspec    
    */
    const TInetAddr& Destination();

    /**
    * Gets flag whether stream is a live stream.
    * @since Series 60 3.0
    * @param none.
    * @return ETrue if stream is known to be a live stream.
    */
    TBool IsLiveStream( void ); 

    /**
    * Getter for Content-base rtsp header content
    * @since Series 60 3.0
    * @param none.
    * @return content of content-base header or empty string 
    */
    TPtrC8 ContentBase( void ); 
    
protected: // Constructors and destructor
    
    /**
    * default constructor
    */
    CCRRtspCommon( void ); 
     
protected: // New methods
    
    /**
    * Method for digging out CSeq from response. 
    * @since Series 60 3.0
    * @param none.
    * @return none but will leave with KErrNotSupported if CSeq is not there.
    */
    void FindCSeqL( void ); 
    
    /**
    * Method for digging out session id from response. Id will be stored in instance variable for future use.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void FindSessionIdL( void ); 

    /**
    * Method for digging ContentBase
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void FindContentBase( void ); 
    
    /**
    * Method for digging out content/content len from response
    * @since Series 60 3.0
    * @param none.
    * @return none but will leave will KErrUnderflow
    *         if all content not yet received.
    */
    void FindContentL( void ); 

    /**
    * method for finding client port from response or command.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void FindClientPorts( void ); 
    
    /**
    * finds transport method from response or command.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void FindTransport();

    /**
    * finds destination IP address from SETUP response or command
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void FindDestination(); 

protected: // Data
    
    /**
    * Buffer that holds the text being parsed.
    */
    HBufC8* iRtspText;          
    
    /**
    * Len of possible content.
    */
    TInt iContentLen; 
    
    /**
    * Possible content, if no content, set to( NULL, 0 ).
    */
    TPtrC8 iContent; 
    
    /**
    * Sequence number, must be there.
    */
    TInt iCSeq; 
            
    /**
    * Possible session id.
    */
    TPtrC8 iSessionId; 
    
    /**
    * Possible client_port.
    */
    TInt iClientPort; 

    /**
    * Transport method.
    */
    TCRRTPTransport iTransport;

    /**
    * Destination address
    */
    TInetAddr iDestination;

    /**
    * Range lower limit for Range: header.
    */
    TReal iLowerRange; 

    /**
    * Range lower limit for Range: header.
    */
    TReal iUpperRange; 
    
    /**
    * Authentication type.
    */  
    HBufC8* iAuthType;
    
    /**
    * Realm.
    */
    HBufC8* iRealm;

    /**
    * Nonce.
    */
    HBufC8* iNonce;

    /**
    * Opaque.
    */
    HBufC8* iOpaque;
    
    /**
    * possible username, if server requires.
    */
    HBufC8* iUserName;
    
    /**
    * rtsp URL in 8bit descriptor.
    */
    HBufC8* iUri; 
    
    /**
    * possible password, if server requires.
    */
    HBufC8* iPassword;
    
    /**
    * When this is set, it indicates that rtsp command/reply 
    * describes a live stream that cannot be paused or position set.
    */
    TBool iIsLiveStream; 
    
	/**
	* Content of Content-base -header
	*/
	TPtrC8 iContentBase ; 
    };

#endif // CCRRTSPCOMMON_H

// End of file
