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
* Description:    Class for parsing a SDP.*
*/




#ifndef CDVRSDPPARSER_H
#define CDVRSDPPARSER_H

// INCLUDES
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
*  Class that parses SDP. Not fully but enough to set up a 
*  rtp stream from rtsp based on information extracted by this class.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CDvrSdpParser : public CBase
    {

public: // Data types

    /**
    * Enum for indicating contents of the rtp stream(s).
    */
    enum TDvrPacketProvidings
        {
        EDvrAudioOnly = 0,     /**< This packet source feeds only audio stream */
        EDvrVideoOnly,         /**< This packet source feeds only video stream */
        EDvrBothAudioAndVideo, /**< This packet source feeds audio and video */
        EDvrNoProgramAtAll,    /**< Nothing useful found */
        EDvrSourceCount        /**< Final number of different sources */
        };    

    /**
    * Defines media point struct.
    */
    struct SMediaPoint
        {
        /*
        * Start point of media in SDP.
        */
        TInt iStart;
            
        /*
        * Length in bytes of media in SDP.
        */
        TInt iLength;
            
        };
            
public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @return CDvrSdpParser pointer to CDvrSdpParser class.
    */
    IMPORT_C static CDvrSdpParser* NewL();

    /**
    * Two-phased constructor.
    */
    IMPORT_C static CDvrSdpParser* NewLC();

    /**
    * Destructor.
    */
    IMPORT_C virtual ~CDvrSdpParser();

public: // New functions

    /**
    * Method that is used to give SDP to this class.
    * "PLAY rtsp://vishnu.wipsl.com/oops/winter_fun.mp4/ RTSP/1.0\r\n..."
    * @since Series 60 3.0
    * @param aSDP is the SDP data.
    * @param aBaseUrl is the rtsp url used to fetch this sdp.
    *        If it is not available, a (NULL,0) descriptor needs to be passed.
    *        It will be used to construct media urls in case where there is no
    *        absolute media urls in the sdp. 
    * @return none.
    */
    IMPORT_C void TryParseL( const TDesC8 &aSdp ,
                             const TDesC8 &aBaseUrl );

    /**
    * Parses SDP file.
    * @since Series 60 3.0
    * @param aSDP is the SDP data.
    * @return none.
    */
    IMPORT_C void TryParseL( const TDesC8& aSdp );

    /**
    * Method for adding new SDP line.
    * @since Series 60 3.0
    * @param aStreamId a section where to add the string.
    *        KErrNotFound: common part
    *        0 (usually) : video
    *		 1 (usually) : audio
    *        2 (usually) : subtitle (not supported yet)
    * @param aLine a string to add.
    * @rerturn a system wide error code.
    */
    IMPORT_C void NewLineL( const TInt aStreamId,
                            const TDesC8& aLine ); 

    /**
    * Method for getting SDP data.
    * @since Series 60 3.0
    * @param aSdp a pointer to SDP data.
    * @rerturn KErrNone if SDP available, otherwise KErrNotFound.
    */
    IMPORT_C TInt GetSdp( TPtrC8& aSdp ); 

    /** 
    * Method for asking whether audio and/or video is included.
    * @since Series 60 3.0
    * @param none.
    * @return indication about stream contents.
    */
    IMPORT_C CDvrSdpParser::TDvrPacketProvidings SupportedContent( void );
        
    /**
    * Method for getting attributes related to session.
    * @since Series 60 3.0
    * @param none.
    * @rerturn array of strings, may be empty.
    */
    IMPORT_C RArray<TPtrC8>& SessionAttributes( void ); 

    /**
    * Method for getting attributes related to session.
    * @since Series 60 3.0
    * @param none.
    * @rerturn array of strings, may be empty.
    */
    IMPORT_C RArray<TPtrC8>& VideoAttributes( void ); 
    
    /**
    * Method for getting attributes related to session.
    * @since Series 60 3.0
    * @param none.
    * @rerturn array of strings, may be empty.
    */
    IMPORT_C RArray<TPtrC8>& AudioAttributes( void ); 
    
    /**
    * Method for getting all data streams related to session.
    * @since Series 60 3.0
    * @param none.
    * @rerturn array of strings, may be empty.
    */
    IMPORT_C RArray<TPtrC8>& DataStreams( void ); 

    /**
    * Method for getting Ipv6 of the SDP having been parsed.
    * @since Series 60 3.0
    * @param none.
    * @return ETrue if SDP had Ipv6 address in it.
    */
    IMPORT_C TBool IsIpv4Sdp( void ); 

    /**
    * Method for getting multicastness of the SDP having been parsed.
    * @since Series 60 3.0
    * @param none.
    * @return ETrue if SDP had multicast characteristics in it.
    */
    IMPORT_C TBool IsMultiCastSdp( void ); 

    /**
    * Method for getting audio control addr e.g. the text from sdp line
    * that begins with a=control: and is located in audio media block
    * @since Series 60 3.0
    * @param none.
    * @return a audio control addr.
    */
    IMPORT_C TPtrC8 AudioControlAddr( void ); 

    /**
    * Method for getting video control addr e.g. the text from sdp line
    * that begins with a=control: and is located in video media block
    * @since Series 60 3.0
    * @param none.
    * @return a video control addr.
    */
    IMPORT_C TPtrC8 VideoControlAddr( void ); 

    /**
    * Method for getting audio ip addr e.g. the text from sdp line
    * that begins with c=IN IP... : and is located in audio media block
    * @since Series 60 3.0
    * @param none.
    * @return video control addr. ownership is NOT transferred. may be NULL.
    */
    IMPORT_C TPtrC8 AudioIpAddr( void ); 
    
    /**
    * Method for getting video ip addr e.g. the text from sdp line
    * that begins with c=IN IP... : and is located in video media block
    * @since Series 60 3.0
    * @param none.
    * @return video control addr. ownership is NOT transferred. may be NULL.
    */
    IMPORT_C TPtrC8 VideoIpAddr( void ); 

    /**
    * Getter for audio port.
    * @since Series 60 3.0
    * @param none.
    * @return a audio port number.
    */
    IMPORT_C TInt AudioPort( void ); 

    /**
    * Getter for video port.
    * @since Series 60 3.0
    * @param none.
    * @return a video port number.
    */
    IMPORT_C TInt VideoPort( void ); 

    /**
    * Getter for audio bitrate.
    * @since Series 60 3.0
    * @param none.
    * @return a audio bit rate.
    */
    IMPORT_C TInt AudioBitrate( void ); 

    /**
    * Getter for video bitrate.
    * @since Series 60 3.0
    * @param none.
    * @return a video bit rate.
    */
    IMPORT_C TInt VideoBitrate( void ); 
    
    /**
    * Getter for audio stream id.
    * @since Series 60 3.0
    * @param none.
    * @return a audio stream id.
    */
    IMPORT_C TInt AudioStreamId( void ); 

    /**
    * Getter for video stream id.
    * @since Series 60 3.0
    * @param none.
    * @return a video stream id.
    */
    IMPORT_C TInt VideoStreamId( void ); 
    
    /**
    * Getter for video timer granularity
    * @since Series 60 3.0
    * @param none.
    * @return Video stream timestamp granularity as 1/s.
    */
    IMPORT_C TUint32 VideoTimerGranularity( void ); 

    /**
    * Getter for audio timer granularity
    * @since Series 60 3.0
    * @param none.
    * @return Audio stream timestamp granularity as 1/s.
    */
    IMPORT_C TUint32 AudioTimerGranularity( void ); 

    /**
    * Checker method for SDP representing realnetworks realmedia content
    * @since Series 60 3.0
    * @param none.
    * @return ETrue if sdp presents realmedia stream.
    */
    IMPORT_C TBool IsRealMediaContent( void );

    /**
    * Media identifier for audio. From a=rtpmap:96 H264/90000
	* the number 96. Usually value is 96 for video and 
	* 97 for audio but might be other
    * @since Series 60 3.0
    * @param none.
    * @return Media id
    */
	IMPORT_C TInt MediaIdentifierAudio( void );
    /**
    * Media identifier for video. From a=rtpmap:96 H264/90000
	* the number 96. Usually value is 96 for video and 
	* 97 for audio but might be other
    * @since Series 60 3.0
    * @param none.
    * @return Media id.
    */
	IMPORT_C TInt MediaIdentifierVideo( void );

    /**
    * Checker method for SDP representing livestream.
    * @since Series 60 3.0
    * @param none.
    * @return ETrue if sdp presents livestream.
    */
    IMPORT_C TBool IsLiveStream( void );

protected: // Constructors and destructor

    /**
    * C++ default constructor.
    */
    CDvrSdpParser();

    /**
    * By default Symbian 2nd phase constructor is private.
    * @param a a reference to socket server .
    */
    void ConstructL();
            
private: // New methods

    /**
    * Finds media(s) from SDP file.
    * @since Series 60 3.0
    * @param aSDP is the SDP data.
    * @return none.
    */
    void FindMediasL( const TDesC8& aSdp );

    /**
    * Finds media(s) from SDP file.
    * @since Series 60 3.0
    * @param aPoints a array for media points.
    * @return none.
    */
    void MakeMediaBuffersL( RArray<SMediaPoint>& aPoints );

    /**
    * Finds session attributes from SDP file.
    * @since Series 60 3.0
    * @param aPoints a array for media points.
    * @return none.
    */
    void FindSessionAttributesL( RArray<SMediaPoint>& aPoints );

    /**
    * Finds media(s) from SDP file.
    * @since Series 60 3.0
    * @param aStreamId a section where to add the string.
    * @param aLine a string to add.
    * @return none.
    */
    void InserNewLineL( TInt aStreamId,
                        const TDesC8& aLine );

    /**
    * Method for finding points of medias in SDP.
    * @since Series 60 3.0
    * @param aPoints a array for media points.
    * @rerturn none.
    */
    void FindMediaPointsL( RArray<SMediaPoint>& aPoints ); 
    
    /**
    * Method for finding next media section start in SDP.
    * @since Series 60 3.0
    * @param aPtr a pointer to buffer where serch from.
    * @rerturn a next media section start point.
    */
    TInt MediaSectionStart( const TDesC8& aPtr );
    
    /**
    * Append media info from media section to array.
    * @since Series 60 3.0
    * @param aMediaPtr a pointer to buffer of media section.
    * @param aAddress a IP address of media.
    * @return none.
    */
    void UpdateMediaInfoL( const TDesC8& aMediaPtr,
                           HBufC8*& aAddress );

    /**
    * Finds IP addr from a buffer.
    * @since Series 60 3.0
    * @param aPtr a pointer to buffer where to search.
    * @return a IP address if found.
    */
    HBufC8* GetIpAddrL( const TDesC8& aPtr );

    /**
    * Getter for clock rate of media.
    * @since Series 60 3.0
    * @param aPtr a pointer to buffer where serch from.
    * @return a clock rate value of the media.
    */
    TUint GetClockRateL( const TDesC8& aPtr );

    /**
    * Append media info from media section to array.
    * @since Series 60 3.0
    * @param aMediaPtr a pointer to buffer of media section.
    * @param aControlAddr a control address of media.
    * @return none.
    */
    void GetControlL( const TDesC8& aMediaPtr,
                      HBufC8*& aControlAddr );

    /**
    * Method for checking if addr is multicast addr.
    * @since Series 60 3.0
    * @param aCLine is c= line from SDP.
    * @return none.
    */
    void CheckForMulticast( const TDesC8& aLine );

    /**
    * Method that picks up all attribute lines from sdp
    * block and stores them in given array
    * @since Series 60 3.0
    * @param aSdpSection is section from sdp. In practice m=video section
    *        or m=audio section or the common section from beginning.
    * @aAttributeList is an array where each attribute-line is stored.
    */
    void FindAttributesL ( const TDesC8& aSdpSection, 
                           RArray<TPtrC8>& aAttributeList ); 
                               
    /**
    * Method that picks up all data stream lines from sdp.
    * @since Series 60 3.0
    * @param aSdpSection is section from sdp. In practice m=video section
    *        or m=audio section or the common section from beginning.
    * @aStreamsList is an array where each data stream-line is stored.
    * @return none.
    */
    void FindDataStreamsL( const TDesC8& aSdpSection,
                           RArray<TPtrC8>& aStreamsList ); 
                               
    /**
    * Reads integer value from a buffer.
    * @since Series 60 3.0
    * @param aPtr a pointer to buffer where serch from.
    * @param aKeyword a keyword for line.
    * @return a integer value base on keyword.
    */
    TInt GetIntL( const TDesC8& aPtr,
                  const TDesC8& aKeyword );

    /**
    * Reads string value from a buffer.
    * @since Series 60 3.0
    * @param aPtr a pointer to buffer where serch from.
    * @param aKeyword a keyword for line.
    * @return a buffer base on keyword.
    */
    HBufC8* GetStringL( const TDesC8& aPtr,
                        const TDesC8& aKeyword );

    /**
    * Finds location where keyword data start.
    * @since Series 60 3.0
    * @param aPtr a pointer to buffer where serch from.
    * @param aKeyword a keyword for line.
    * @return a point in buffer where keyword data start.
    */
    TInt FindStart( const TDesC8& aPtr,
                    const TDesC8& aKeyword );

    /**
    * Finds location where keyword data ends.
    * @since Series 60 3.0
    * @param aPtr a pointer to buffer where serch from.
    * @param aStart a point where keyword data starts.
    *        Return a point to line change.
    * @param aIgnoreSpace if true, points to the line feed only if.
    * @return a point in buffer where keyword data end.
    */
    TInt GetLen( const TDesC8& aPtr,
                 const TInt aStart,
                 const TBool aIgnoreSpace = EFalse );
                 
    /**
    * Finds smaller non error value integer.
    * @since Series 60 3.0
    * @param aValue1 a value 1 for comparison.
    * @param aValue2 a value 2 for comparison.
    * @return smaller integer if one or both inputs are positive,
              otherwise return KErrNotFound.
    */
    TInt MinNonError( const TInt aValue1, 
                      const TInt aValue2 );

    /**
    * Convert a string to unsigned integer.
    * @since Series 60 3.0
    * @param aString a string to convert.
    * @return a integer value base on string.
    */
    TUint StrToUint( const TDesC8& aString );

    /**
    * Delete variables.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void DeleteVariables( void );

private: // Data

    /**
    * Pointer to actual SDP text being parsed.
    */
    HBufC8* iSdp; 

    /**
    * String containing (possible) base url.
    */
    HBufC8* iBaseUrl; 
        
    /**
    * IP in common section.
    */
    HBufC8* iCommonIp;
    
    /**
    * Media section buffer(s)
    */
    RArray<TPtrC8> iMediaBuf;
        
    /**
    * Media attributes for both audio and video.
    */
    RArray<TPtrC8> iSessionAttributes; 
    
    /**
    * Media attributes for audio not containing a=control attr.
    */
    RArray<TPtrC8> iAudioAttributes; 
    
    /**
    * Media attributes for video not containing a=control attr.
    */
    RArray<TPtrC8> iVideoAttributes; 
    
    /**
    * Data streams found from SDP.
    */
    RArray<TPtrC8> iDataStreams; 
    
    /**
    * Url that may be used to SETUP iAudioControlAddr RTSP/1.0\r.
    */
    HBufC8* iAudioControlAddr;
    
    /**
    * Url that may be used to SETUP iVideoControlAddr RTSP/1.0\r.
    */
    HBufC8* iVideoControlAddr;
    
    /**
    * Ip number from audio block from c=IP.. line.
    */
    HBufC8* iAudioIpAddr;
    
    /**
    * Ip number from video block from c=IP.. line.
    */
    HBufC8* iVideoIpAddr;
        
    /**
    * Audio port or KErrNotFound if not available.
    */
    TInt iAudioPort;
    
    /**
    * Video port or KErrNotFound if not available.
    */
    TInt iVideoPort;
    
    /**
    * Subtitle port or KErrNotFound if not available.
    */
    TInt iTitlePort;
    
    /**
    * Audio bitrate or KErrNotFound if not available.
     */
    TInt iAudioBitrate; 
    
    /**
    * Video bitrate or KErrNotFound if not available.
    */
    TInt iVideoBitrate; 
    
    /**
    * Audio strema id.
    */
    TInt iAudioStreamId; 
    
    /**
    * Video stream id.
    */
    TInt iVideoStreamId; 
    
    /**
    * How much is one video rtp timer tick in wall clock time
    * (usually 1/90000 seconds).
    */
    TUint32 iVideoTimerGranularity;
    
    /**
    * How much is one audio rtp timer tick in wall clock time
    * (usually 1/<samplerate> seconds).
    */
    TUint32 iAudioTimerGranularity; 

    /**
    * If the sdp parsed was Ipv4 or not.
    */
    TBool iIsIpv4; 
    
    /**
    * If the sdp parsed was multicast or not.
    */
    TBool iIsMulticast; 

	/**
	* Media identifier for audio. From a=rtpmap:96 H264/90000 the number 96.
	* Usually value 96 is for video and 97 for audio, but might be other.
	*/
	TInt iMediaIdentifierAudio;
	
	/**
	* Media identifier for video. 
	*/
	TInt iMediaIdentifierVideo;
    
    };

#endif // CDVRSDPPARSER_H

// End of file
