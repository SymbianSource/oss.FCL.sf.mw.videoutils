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
* Description:    Class that negotiaties RTP stream using RTSP*
*/




#ifndef CCRRTSPPACKETSOURCE_H
#define CCRRTSPPACKETSOURCE_H

//  INCLUDES
#include "CCRPacketSourceBase.h"
#include "CCRStreamingSession.h"
#include "CCRConnection.h"
#include "CCRRtspResponse.h"
#include "CCRRtpTcpObserver.h"
#include "CCRSock.h"
#include "MCRTimerObserver.h"
#include <rtp.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRRtspCommand;
class CDvrSdpParser;
class RRtpSession;
class RRtpReceiveSource;
class TRtpEvent;
class CCRTimer;
class CCRPunchPacketSender;
class CCRRtpTcpStreamer;

// CLASS DECLARATION

/**
*  Class that negotiaties RTP stream using RTSP.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRRtspPacketSource : public CCRPacketSourceBase,
                            public MCRSockObserver,
                            public MCRRtpTcpObserver,
                            public MCRConnectionObserver,
                            public MCRTimerObserver
    {

public: // Data types

    /**
    * RTP packet memory layout structure
    */
    struct TCRRtpMessageHeader
        {
        TUint iVersion:2;
        TUint iPadding:1;
        TUint iExtension:1;
        TUint iCsrcCount:4;
        TUint iMarker:1;
        TUint iPayloadType:7;
        TUint iSeq:16;
        TUint32 iTimestamp;
        TUint32 iSSRC;
        };

    /**
    * RTCP sender report memory layout 
    */
    struct TCRRtpSRReportHeader
        {
        // Note the order of first 3 items:
        unsigned char iReportCount:5; // this and
        unsigned char iPadding:1;     // this and
        unsigned char iVersion:2;     // this are first 8 bits. 
        // Note that above order is reversed from spec.
        unsigned char iPacketType;
        TUint16 iLength;
        TUint32 iSenderSSRC;
        TUint32 iMSWTimestamp;
        TUint32 iLSWTimestamp;
        TUint32 iRTPTimestamp;
        TUint32 iSenderPacketCount;
        TUint32 iSenderOctetCount;
        };

    /**
    * Enum for identifying sockets. Sockets return this via Id()
    * and this is also used as an array index. 
    */
    enum TCRRTPSockId
        {
        ERTPControl=0,     /**< placeholder for RTSP sock */
        ERTPVideoSend1,    /**< RTP video payload */
        ERTPVideoSend2,    /**< RTCP video */
        ERTPAudioSend1,    /**< RTP audio payload */
        ERTPAudioSend2,    /**< RTCP audio */
        ERTPMaxSockets     /**< Just max value, no real socket assigned for this */
        };

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * Calling this method will finally cause a call to DoConnectL() so
    * this class will start connecting as soon as it is constructed, 
    * address where to connect to is in aParams. 
    * @param aParams is the stream address
    * @param aConnection is handle to RConnection wrapper to use
    * @param aSessionObs is pointer to our observer
    * @aOwningSession is the streaming session that owns this packet source
    * @return CCRRtspPacketSource pointer to CCRRtspPacketSource class
    */
    static CCRRtspPacketSource* NewL( const SCRRtspParams& aParams,
                                      CCRConnection& aConnection,
                                      RSocketServ& aSockServer,
                                      MCRStreamObserver& aSessionObs,
                                      CCRStreamingSession& aOwningSession );

    /**
    * Destructor.
    */
    virtual ~CCRRtspPacketSource();

public: // New functions

    /**
    * Method for aquiring the URI being streamed here
    * @since Series 60 3.0
    * @param none.
    * @return URI.
    */
    TPtr URI();

    /**
    * Method that packet puncher calls to tell
    * it has finished punching the firewall.
    * @since Series 60 3.0
    * @param aPuncher tells the number of the port 
    * where the punch packet was sent to.
    * @return none.
    */
    void PunchPacketsSent( CCRPunchPacketSender* aPuncher );

    /**
    * Method for setting observer that this source will report its status to.
    * @since Series 60 3.0
    * @param aObserver is the observer instance implementing MCRConnectionObserver.
    * @return none.
    */
    void RegisterConnectionObs( MCRConnectionObserver* aObserver );

    /**
    * Method for removing status observer
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void UnregisterConnectionObs();

public: // Methods from base classes

    /**
    * From CCRPacketSourceBase.
    * Method for acquiring sdp.
    * @since Series 60 3.0
    * @param aSdp is string pointer that will be .Set() to contain the sdp.
    *        If no sdp is available no .Set() will occur.
    * @return KErrNotReady if no sdp available.
    */
    TInt GetSdp( TPtrC8& aSdp );

    /**
    * From CCRPacketSourceBase.
    * Method for acquiring (almost) up-to-date sequence and ts numbers.
    * @since Series 60 3.0
    * @param aAudioSeq is reference to TUint that will be set by this
    *        method to contain latest available sequence number for
    *        audio stream being received via this packet source.
    * @param aAudioTS rtp timestamp for audio.
    * @param aVideoSeq rtp seq for video. If no video, value will not be touched.
    * @param aVideoTS rtp ts for video.
    * @return KErrNone if data available.
    */
    TInt SeqAndTS( TUint& aAudioSeq,
                   TUint& aAudioTS,
                   TUint& aVideoSeq,
                   TUint& aVideoTS );

    /**
    * From CCRPacketSourceBase.
    * Post action after source initialized.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void PostActionL();

    /**
    * From CCRPacketSourceBase.
    * Method for ordering "play" for packet source.
    * @since Series 60 3.0
    * @param aStartPos is start position in seconds.
    * @param aEndPos is end position in seconds.
    * @return a system wide error code.
    */
    TInt Play( const TReal& aStartPos,
               const TReal& aEndPos );

    /**
    * From CCRPacketSourceBase.
    * Method for pausing play from source.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    TInt Pause();

    /**
    * From CCRPacketSourceBase.
    * Method for stopping play from source.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    TInt Stop();

    /**
    * From CCRPacketSourceBase.
    * Method for getting range of stream. If no range/duration
    * is available this method will set the return values to
    * 0.0,-1.0 and that may be quite normal state live streams.
    * @since Series 60 3.0
    * @param aLower is where to start from. If no value available,
    *        value of aLower must be set to 0
    * @param aUpper is where to stop. Negative values mean eternity.
    * @return none.
    */
    void GetRange( TReal& aLower,
                   TReal& aUpper );

    /**
    * From CCRPacketSourceBase.
    * Setter for current position.
    * @since Series 60 3.0
    * @param aPosition a postion of playback.
    * @return a system wide error code.
    */
    virtual TInt SetPosition( const TInt64 aPosition ) ;

    /**
    * From MCRSockObserver.
    * This method is called after some data has been received from socket.
    * @since Series 60 3.0
    * @param aSockId identifies the socket where the data came from. 
    * @param aData is descriptor containing the data received.
    *        ownership of data is not passed via this call.
    * @return none.
    */
    void DataReceived( TInt aSockId,
                       const TDesC8& aData );

    /**
    * From MCRSockObserver.
    * This method is called after status of socket changes.
    * @since Series 60 3.0
    * @param aSockId Identifies the socket that had the status change
    * @param aStatus is the new status of the sock
    * @param aError if aStatus was an error-status, this may contain error code
    * @return none.
    */
    void SockStatusChange( TInt aSockId, 
                           CCRSock::TCRSockStatus aStatus,
                           TInt aError );

    /**
    * From MCRRtpTcpObserver.
    * Signals an available incoming RTSP control message
    * @since Series 60 3.0
    * @param aData a RTSP meaage data.
    * @return none.
    */
    void RtspMsgAvailable( const TDesC8& aData );

    /**
    * From MCRRtpTcpObserver.
    * Receives a RTP/RTCP packet available from TCP streamer and
    * forwards to streaming sessions (buffers).
    * @since Series 60 3.0
    * @param aChannel specifies channel if in TCP streaming case, in practice
    *        tells if it is a video/audio packet and if it is RTP or RTCP.
    * @param aPacket is the payload of the  packet
    * @return none.
    */
    void RtpTcpPacketAvailable( TInt aChannel,
                                const TDesC8& aPacket );

    /**
    * From MCRRtpTcpObserver.
    * Receives a RTP/RTCP packet and forward to streaming server
    * @since Series 60 3.0
    * @param aChunk contains RTP packet with headers suitable to be sent to remote
    *        via RTSP control socket.
    * @return none.
    */
    void ForwardRtpTcpChunck( const TDesC8& aChunk );
    
    /**
    * From MCRConnectionObserver.
    * receives notifications of connection status changes,
    * used to clear heuristics on streaming capabilities.
    * @since Series 60 3.0
    * @param aSessionId a session id.
    * @param aStatus is the new status of the connection.
    * @param aErr contains error code if new status is failure status.
    * @return none.
    */
    void ConnectionStatusChange( TInt aSessionId,
                                 TCRConnectionStatus aStatus,
                                 TInt aErr );

    /**
    * From MCRTimerObserver.
    * The function to be called when a timeout occurs.
    * @since Series 60 3.0
    * @param aTimer is pointer to timer instance that caused call to this method.
    * @return none.
    */
    void TimerExpired( CCRTimer* aTimer );

private: // Constructors and destructor

    /**
    * C++ default constructor.
    * @param aConnection is RConnection wrapper to use.
    * @param aSockServer is socket server to use when opening sockets.
    * @param aSessionObs is pointer to our observer.
    * @param aOwningSession is pointer to session that owns this source.
    */
    CCRRtspPacketSource( CCRConnection& aConnection,
                         RSocketServ& aSockServer,
                         MCRStreamObserver& aSessionObs,
                         CCRStreamingSession& aOwningSession );

    /**
    * By default Symbian 2nd phase constructor is private.
    * @aParams aParams contains stream address parameters that this packet source
    *          will try to use to obtain the stream.
    * @return none
    */
    void ConstructL( const SCRRtspParams& aParams );

private: // New methods

    /**
    * Method for connecting.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void DoConnectL();

    /**
    * Method for cleaning up.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void CleanUp();

    /**
    * Method for initiating cleanup.
    * @since Series 60 3.0
    * @param aSelfPtr is pointer to "this".
    * @return none.
    */
    static TInt CleanUpCallback( TAny* aSelfPtr );

    /**
    * Method for asking for cleanup in async way.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void DoCleanup();

    /**
    * Method for sending RTSP command. Command in question depends on state.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void SendRtspCommandL();

    /**
    * Method for parsing and reacting to RTSP reply.
    * @since Series 60 3.0
    * @param aData a RTSP response data.
    * @return none.
    */
    void ProcessRtspResponseL( const TDesC8& aData );

    /**
    * Method for parsing and reacting to RTSP command that
    * remote server may send to us
    * @since Series 60 3.0
    * @param aData a RTSP response data.
    * @return none.
    */
    void ProcessRTSPCommandL( const TDesC8& aData );

    /**
    * Method for sending error indication to client side
    * if rtsp response was something errorneous.
    * @since Series 60 3.0
    * @param aErrorCode is RTSP response code,something else than 200 OK.
    * @return none.
    */
    void ProcessRtspErrorResponseL( CCRRtspResponse::TResponseCode aErrorCode );

    /**
    * Method for doing SETUP.
    * @since Series 60 3.0
    * @param aControlAddr is either absolute or relative controladdr for stream.
    * @param aForAudio is ETrue if control addr is for audio.
    * @return client port number.
    */
    TInt SendSetupCommandL( const TDesC8& aControlAddr,
                            TBool aForAudio );

    /**
    * Method for doing PLAY.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void SendPlayCommandL();

    /**
    * Method for doing PAUSE that is sent to remote server.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void SendPauseCommandL();

    /**
    * Method for doing OPTIONS ping that is sent to remote server.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void SendOptionsCommandL();

    /**
    * Method for doing TEARDOWN.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void SendTearDownCommandL();

    /**
    * Method that sets up rtp sessions. Must be called before play is issued.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    TInt SetupRTPSessions();

    /**
    * Method that starts RTSP command response timeout.
    * @since Series 60 3.0
    * @param aTime a timer interval.
    * @return none.
    */
    void StartRtspTimeout( TTimeIntervalMicroSeconds32 aTime );

    /**
    * Callback method from RTP session for audio.
    * @since Series 60 3.0
    * @param aPtr is pointer to "this".
    * @param aEvent is the event. In practice it contains RTP packet or RTCP packet.
    * @return none.
    */
    static void AudioRTPCallBack( CCRRtspPacketSource* aPtr,
                                  const TRtpEvent& aEvent );

    /**
    * Callback method from RTP session for video.
    * @since Series 60 3.0
    * @param aPtr is pointer to "this".
    * @param aEvent is the event. In practice it contains RTP packet or RTCP packet.
    * @return none.
    */
    static void VideoRTPCallBack( CCRRtspPacketSource* aPtr,
                                  const TRtpEvent& aEvent );

    /**
    * Callback for RTSP timeout. Just ask session to start cleanup.
    * @since Series 60 3.0
    * @param aPtr is pointer to "this".
    * @return none.
    */
    static TInt RtspTimeoutCallback( TAny* aPtr );

    /**
    * Method for creating sender report header.
    * @param aPtr is pointer to "this".
    * @param aEvent is the rtp packet event received.
    * @param aStreamId a stream id.
    * @return none
    */
    static void SenderReport( CCRRtspPacketSource* aPtr,
                              const TRtpEvent& aEvent,
                              MCRPacketSource::TCRPacketStreamId aStreamId );

    /**
    * Method for handling audio.
    * @param aPtr is pointer to "this".
    * @param aSource a receiver source.
    * @param aEvent is the rtp packet event received.
    * @return none
    */
    static void HandleNewSourceL( CCRRtspPacketSource* aPtr,
                                  RRtpReceiveSource& aSource,
                                  const TRtpEvent& aEvent,
                                  TRtpCallbackFunction aCallback );

    /**
    * Generate DESCRIBE message with authentication information.
    * used when server responses with 401 or 407.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void SendAuthDescribeL();

    /**
    * Adds authentication info to given command.
    * @since Series 60 3.0
    * @param aCommand the command where to add authentication info.
    * @return none.
    */
    void AddAuthenticationL( TInt aCommand );

    /**
    * Setup sessions for RTP stack and issue play command
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void SetupSessionsAndPlay();

    /**
    * Initiates sending of punch packets
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void SendPunchPacketsL();

    /**
    * Creates a socket in iRTPSockArr for multicast streaming:
    * binds to local port and join multicast group
    *
    * @since Series 60 3.0
    * @param aSockId  id for RTP socket in iRTPSockArr
    * @param aGroupAddr multicast group address, in IPv6 or mapped IPv4
    * @param aPort local UDP port to listen
    * @return system error code
    */
    TInt CreateMulticastSocket( TCRRTPSockId aSockId,
                                const TInetAddr& aGroupAddr,
                                TInt aPort );

    /**
    * Creates a socket in iRTPSockArr for unicast streaming:
    * binds to local port and connects to remote address
    *
    * @since Series 60 3.0
    * @param aSockId  id for RTP socket in iRTPSockArr
    * @param aLocalAddr  local address and port to listen
    * @param aRemoteAddr remote address and port to connect to
    * @return system error code
    */
    TInt CreateUnicastSocket( TCRRTPSockId aSockId,
                              const TInetAddr& aLocalAddr,
                              const TInetAddr& aRemoteAddr );

    /**
    * Method that handles rtp payload packet to buffer.
    * @since Series 60 3.0
    * @param aEvent is the rtp packet event received.
    * @param aIsAudio indicates whether this is audio or video packet.
    * @return none.
    */
    void RTPPayloadProcessor ( const TRtpEvent& aEvent,
                               const TBool aIsAudio );

    /**
    * Method for setting session parameters.
    * @since Series 60 3.0
    * @param aSession is reference to either audio or video rtp session.
    *        from instance variables of this class ; the parameters will
    *        be set in similar way for both audio and video
    * @param aGranularity is number of clock ticks per second, usually found from SDP
    * @return none.
    */
    void SetRtpSession( RRtpSession& aSession ,
                        TReal aGranularity );

    /**
    * Method for initiating cleanup.
    * @since Series 60 3.0
    * @param aSelfPtr is pointer to "this".
    * @return symbian error code.
    */
    static TInt SendRtspPing( TAny* aSelfPtr );

    /**
    * Method for synchronizing of timestamps and notifying sinks.
    * This should be called after sender reports are received
    * and with normal packet reception in the beginning of the stream.
    * In practice this will be called for about every packet while
    * iNoRtpInfoHeader is on, when we know the seq+ts then 
    * iNoRtpInfoHeader will be turned off and this method will be no longer
    * used ; detection of no rtp info header in rtsp play reply will turn
    * iNoRtpInfoHeader on. 
    *
    * @param aStreamId identifies the stream
    * @param aMSWTimestamp more significant part of wall-clock. Seconds.
    * @param aLSWTimestamp less significant part of wall-clock. Fraction of seconds.
    * @param aRTPTimestamp rtptimestamp of aMSW+aLSWTimestamp
    * @param aSeq sequence number of aRTPTimestamp
    */
    void ConstructSeqAndTsForSink ( MCRPacketSource::TCRPacketStreamId aStreamId,
                                    TUint32 aMSWTimestamp,
                                    TUint32 aLSWTimestamp,
                                    TUint32 aRTPTimestamp,
                                    TUint aSeq );
    
    /**
    * Method for setting up multicast or tcp streaming from setup reply.
    * @param none
    * @return none
    */
    void ConditionallySetupMultiCastOrTcpStreamingL();

    /**
    * Method checks that we all receiveing from all streams, if not returns false.
    * @param none
    * @return ETrue if receiving from all streams.
    */
    TBool CheckReceiveOfStreams();
    
    /**
    * Method that flags given stream as receiving stream.
    * @param aStreamType stream that is receiving   
    * @return none
    */
    void StreamFound( TCRPacketStreamId aStreamType );
    
    /**
    * Method that resets all created receive streams as non receiving.  
    * @param none
    * @return none
    */
    void ResetStreamFlags();

#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE )
    /**                                        
    * Method for showing a packet header.
    * @since Series 60 3.0
    * @param aRtcpHeader a pointer to packet header.
    * @param aEvent is the rtp packet event received.
    * @return none.
    */
    static void ShowHeader( const TDesC8& aRtcpHeader,
                            const TCRRtpSRReportHeader& aSrReport );
#endif // LIVE_TV_FILE_TRACE || LIVE_TV_RDEBUG_TRACE

private: // Data

    /**
    * rtsp URL we're supposed to view.
    */
    HBufC* iRtspUri;
    
    /**
    * Hostname -part of iRTSPURI.
    */
    TPtrC iRtspUriHost;
    /**
    * rtsp URL in 8bit descriptor, only because there is no uri parser in 8 bit.
    */
    HBufC8* iRtspUri8;
    /**
    * possible username, if server requires.
    */
    HBufC* iUserName;
    
    /**
    * possible password, if server requires.
    */
    HBufC* iPassword;
    
    /**
    * Sequence number from rtp info header concerning audio.
    */
    TUint iSeqFromRtpInfoForAudio;
    
    /**
    * Sequence number from rtp info header concerning video.
    */
    TUint iSeqFromRtpInfoForVideo;
    
    /**
    * RTP Session for audio.
    */
    RRtpSession iAudioSession;
    
    /**
    * RTP Session for video.
    */
    RRtpSession iVideoSession;
    
    /**
    * Receive stream for audio.
    */
    RRtpReceiveSource iRtpRecvSrcAudio;
    
    /**
    * Receive stream for audio.
    */
    RRtpReceiveSource iRtpRecvSrcVideo;
    
    /*
    * Socket server reference ; all sockets we use must be via common sock serv. 
    */
    RSocketServ& iSockServer;
    
    /*
    * RConnection wrapper reference ; all sockets we use must share an RConnection.
    */
    CCRConnection& iConnection;

    /*
    * Socket to use to talk RTSP to remote server. 
    */
    CCRSock* iRtspSock;

    /**
    * Sockets for incoming multicast RTP data, to bypass RTP stack limitations.
    */
    CCRSock* iRTPSockArr[ERTPMaxSockets];

    /**
    * pointer to data sent by us via socket.
    */
    HBufC8* iSentData;
    
    /**
    * State of this rtsp client, or stage in discussion.
    */
    TCRRTSPStage iStage;
    
    /**
    * Pointers to previous commands sent in various stages.
    */
    CCRRtspCommand* iPrevCommands[ERTSPLastStage];
    
    /**
    * Pointers to responses of various commands.
    */
    CCRRtspResponse* iResponses[ERTSPLastStage];
    
    /** 
    * CSeq for rtsp negotiation.
    */
    TInt iCSeq;
    
    /**
    * SDP parser.
    */
    CDvrSdpParser* iSdpParser;
    
    /**
    * Client portbase. Value used for video, for audio it is this value + 2.
    */
    TInt iClientPort;
    
    /**
    * Transport method for RTP.
    */
    TCRRTPTransport iTransport;
    
    /**
    * Timer for UDP reception timeout, to fire TCP usage.
    */
    CCRTimer* iUdpReceptionTimer;
    
    /**
    * IP packet parser.
    */
    CCRRtpTcpStreamer* iRtpTcpStreamer;
    
    /**
    * Session id from server.
    */
    TPtrC8 iSessionId;
    
    /**
    * If sink is ready.
    */
    TBool iReadyToPlay;
    
    /**
    * RTP time initial value for audio.
    */
    TUint32 iRTPTimeStampAudio;
    
    /**
    * RTP time initial value for video.
    */
    TUint32 iRTPTimeStampVideo;
    
    /**
    * If authentication is needed.
    */
    TBool iAuthenticationNeeded;
    
    /**
    * Nonce of the authentication header.
    */
    HBufC8* iNonce;
    
    /**
    * Realm of the authentication header.
    */
    HBufC8* iRealm;
    
    /**
    * Authentication type ("Basic" or "Digest"). 
    */
    HBufC8* iAuthType;
    
    /** 
    * Opaque of the authentication header.
    */
    HBufC8* iOpaque;
    
    /**
    * User Agent header if present.
    */
    HBufC8* iUserAgent;
    
    /**
    * Connection bandwidth in bit/s for 'Bandwidth' header.
    */
    TInt iBandwidth;
    
    /**
    * x-wap-profile if present.
    */
    HBufC8* iWapProfile;
    
    /**
    * Number of times authentication has failed.
    */
    TInt iAuthFailedCount;
    
    /**
    * Our observer.
    */
    MCRStreamObserver& iSessionObs;
    
    /**
    * Number of bytes sent in audio rtp packets.
    */
    TUint32 iAudioBytes;
    
    /**
    * Number os audio rtp packets sent.
    */
    TUint32 iAudioPackets;
    
    /**
    * Number of bytes sent in video rtp packets.
    */
    TUint32 iVideoBytes;
    
    /**
    * Number of video packets sent.
    */
    TUint32 iVideoPackets;
    
    /**
    * Instance for punch packet sender ; lifetime will be from setup stage
    * to delivery of first audio packet.
    */
    CCRPunchPacketSender* iPunchPacketSenderAudio;
    
    /**
    * Instance for punch packet sender ; lifetime will be from setup stage
    * to delivery of first video packet.
    */
    CCRPunchPacketSender* iPunchPacketSenderVideo;
    
    /**
    * Flag for successful sending of punch packets for audio stream.
    */
    TBool iPunchPacketSentForAudio;
    
    /**
    * Flag for successful sending of punch packets for video stream.
    */
    TBool iPunchPacketSentForVideo;
    
    /**
    * Connection status observer.
    */
    MCRConnectionObserver* iObserver;
    
    /**
    * Position where playback will start. Usually will have value 0 but will
    * be changed if user seeks the clip.
    */
    TReal iStartPos;
    
    /**
    * Position where playback is supposed to end. Most streaming servers won't
    * obey this at all but rtsp gives possibility to specify end position so
    * we'll support that anyway.
    */
    TReal iEndPos;
    
    /**
    * Sends RTSP ping.
    */
    CPeriodic* iRtspPingTimer;
    
    /**
    * Timer started when RTSP message is sent and cancelled when reply is received. 
    * If this timer fires, it means that we didn't get reply in time that in turn
    * forces cleanup of this source.
    */
    CPeriodic* iRtspTimeout;

    /**
    * Sequence number of last rtsp reply received. 
    */    
    TInt iLastReceivedSeq;

    /**
    * We have a play-command pending, not yet sent. 
    */    
    TInt iPostPonedPlay;

    /**
    * Proxy server addr to use while streaming.
    */
    TName iProxyServerAddr; 

    /**
    * Proxy server port to use while streaming. 
    */
    TInt iProxyServerPort;     

    /**
    * Flag telling that there has been no rtp-info header.
    * in play-response 
    */
    TBool iNoRtpInfoHeader; 
    
    /**
    * Wall-clock timestamp MSW part for syncronizing.
    */
    TUint32 iMSWTimestamp; 
    
    /**
    * Wall-clock timestamp LSW part for syncronizing.
    */
    TUint32 iLSWTimestamp;
    
    /**
    * Flag set when we manage to get UDP traffic. If we some reason loose
    * traffic completely (eg staying too long in PAUSE -state while streaming)
    * we dont try TCP at first but after we have tried UDP again.
    */  
    TBool iUdpFound;
    
    /**
    * Simple container class representing one receive stream.
    */
    class TReceiveStream
        {
        public:
        
        /**
        * Default constructor setting default values.
        */
        TReceiveStream()
            {
            iStreamType = EStreamIdCount;
            iDataReceived = EFalse;
            };
        
        /**
        * Stream type, defined in MCRPacketSource.
        */
        TCRPacketStreamId iStreamType;
        
        /**
        * Boolean flagging if we have traffic from this stream.
        */
        TBool iDataReceived;
        };
    
    /**
    * Array of receive streams.
    */    
    RArray<TReceiveStream> iReceiveStreams;
    
    /**
    * Boolean flagging if we have found traffic from all receive streams.
    */
    TBool iTrafficFound;
    
    };

#endif // CCRRTSPPACKETSOURCE_H

// End of file
