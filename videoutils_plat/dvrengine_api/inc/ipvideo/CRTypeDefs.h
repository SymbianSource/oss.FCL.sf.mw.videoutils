/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    Common type definitions for both client and server parts*
*/


#ifndef CRTYPEDEFS_H
#define CRTYPEDEFS_H

// INCLUDES
#include <e32base.h>
#include <f32file.h>

// CONSTANTS
_LIT( KCRServerNameExe, "CommonRecordingEngine.exe" );
_LIT( KCRServerSemaphoreName, "CommonRecServerSemaphore" );
_LIT( KCRMsgQueueName, "CRMsgQueue%d" );

/** Max RTSP Name/password len */
const TInt KMaxNameLen( 40 );
/** Max RTSP URI len */
const TUint KMaxUrlLen( 256 );

// The server version. A version must be specified 
// when creating a session with the server
const TUint KCRServMajorVersionNumber( 0 );
const TUint KCRServMinorVersionNumber( 1 );
const TUint KCRServBuildVersionNumber( 0 );
const TUint KCRStackSize( 2 * KDefaultStackSize );
const TUint KCRHeapSize( 0xF0000 );
const TUint KCRMaxHeapSize( 0x1F0000 );

// Queue name length (10 from fixed part and 
// 10 from maximum numbers in 32 value
const TUint KCRQueueNameLength( 20 );

// MACROS
// None.

// DATA TYPES
/**
* Different messagetypes going from DVR engine to client lib. 
* Note that client lib maps this to yet another enum before 
* reporting the event via MCRStreamObserver::NotifyL
*/
enum TCRQueueMsg
    {
    ECRMsgQueueAuthenticationNeeded, /**< Needs new username and password for authentication */
    ECRMsgQueueAttachCompleted,      /**< Connection Attach completed and connection ready to be used */
    ECRMsgQueueConnectionError,      /**< Unable to open connection in RTP engine & CR engine */
    ECRMsgQueueNotEnoughBandwidth,   /**< Stream bitrate higher than (estimated) available connection bandwidth */
    ECRMsgQueueSwitchingToTcp,       /**< Switching to tcp-streaming; client is supposed to init player */
    ECRMsgQueueNormalEndOfStream,    /**< clip ends normally */   
    ECRMsgQueueStreamIsLiveStream,   /**< indicates that we are streaming clip that cannot be paused */		
    ECRMsgQueueStreamIsRealMedia,    /**< Clip would have been (unsupported) realnetworks realmedia over rdt */
    ECRMsgQueueTestSinkData,         /**< Data from TestSink (former NullSink) to test client */
    ECRMsgQueueSdpAvailable,         /**< Used to communicate SDP availability to client */
    ECRMsgQueueReadyToSeek,          /**< Used to communicate seeking possibility */
    ECRMsgQueueRecordingStarted,     /**< Used to communicate recording state */
    ECRMsgQueueRecordingPaused,      /**< Used to communicate recording state */
    ECRMsgQueueRecordingEnded        /**< Used to communicate recording state */
    };

/**
*  Different message types going from TestSink (NullSink) to client lib.
*  Embedded in TCRQueueMsg::ECRMsgQueueTestSinkData
*/
enum TCRTestSinkData
    {
    ECRTestSinkStateIdle,
    ECRTestSinkStateSdpAvailable,
    ECRTestSinkStateSeqAndTSAvailable,
    ECRTestSinkStatePlaying,
    ECRTestSinkStateClosing,
    };

/**
* Different record formats. 
*/
enum TCRRecordFormat
    {
    ECRRecordFormatRtp, /**< Nokia's propriatary RTP clip format */
    ECRRecordFormat3gp, /**< Normal 3GPP file format */
    ECRRecordTimeShift  /**< Time shifting with Nokia's propriatary format */
    };

/**
 * This structure is sent over message queue where
 * api sits listening
 */
struct SCRQueueEntry
    {
    TCRQueueMsg iMsg; /**< Identifies the message */
    TInt iErr;        /**< Can be used to pass error value related to message */
	TInt iSessionId;  /**< Points to session generating the message. 0 points no session, broadcast */
    };

/**
* Structure used to communicate the Internet Access Point that
* DVR engine may use 
*/
struct SCRRtspIapParams
    {
    TInt32 iIap;  /**< Internet Access Point ID from COMMS db. Currently not used */
	TUint32 iConnectionId; /**< Handle of opened RConnection. Currently used */
	TBuf<KCRQueueNameLength> iQueueName; /**< Name of RMSgQueue that DVR engine will use to report progress back */
    };

/**
* Data structure specifying RTSP address 
*/
struct SCRRtspParams
    {
    TBuf<KMaxUrlLen> iUrl ;  /**< usually something like rtsp://server.name... */
	TBuf<KMaxNameLen> iUserName; /**< RTSP Username. May be blank if server is not going to reply 401 */
	TBuf<KMaxNameLen> iPassword; /**< RTSP password. May be blank if server is not going to reply 401 */    
	TBuf<KCRQueueNameLength> iQueueName; /**< Name of RMSgQueue that DVR engine will use to report progress back */
	TName iProxyServerAddr ; /**< Possible rtsp proxy server */
	TInt iProxyServerPort ;  /**< Rtsp proxy port number */
    };
    
/**
* Defines live stream params.
*/
struct SCRLiveParams
    {
    TPtrC8 iSdpData;
    };
    
/**
* Defines RTP playback params.
*/
struct SCRRtpPlayParams
    {
    TFileName iFileName;
    };
    
/**
* Defines recording params.
*/
struct SCRRecordParams
    {
    TPtrC iFileName;
    TCRRecordFormat iFormat;
    TPtrC iServiceName;
    TPtrC iProgramName; 
    TPtrC8 iSdpData;
    TInt iPostRule;
    TInt iParental;
    TTime iEndTime;
    };
        
/**
* Opcodes used in message passing between client and server.
*/
enum TCRServRqst
    {
    // Server request
    ECRServBase = 450,
    ECRServConnect,
    ECRServCreateSubSession,
    ECRServCloseSubSession,
    ECRServCloseSession,
    ECRServReset,
    ECRServCancelAll,
    ECRServLastEnum,
    // API for LiveTV specific commands
    ECRLiveTvBase = 500,
    ECRSetIap,
    ECRCancelSetIap,
    ECRPlayRtspUrl,
    ECRPlayDvbhLive,
    ECRChangeService,
    ECRPlayRtpFile,
    ECRPlayRtpHandle,
    ECRRecordCurrentStream,
    ECRRecordRtspStream,
    ECRRecordDvbhStream,
    ECRPauseRecordStream,
    ECRStopRecordStream,
    ECRStartTimeShift,
    ECRStopTimeShift,
    ECRPlayCommand,
    ECRPauseCommand,
    ECRStopCommand,
    ECRSetPosition,
    ECRGetPosition,
    ECRCloseSession,
    ECRPlayNullSource,
    ECRPlayRtspUrlToNullSink,
    ECRLiveTvLastEnum
    };
    
/**
* Opcodes used in message passing from server.
*/
enum TCRServResponse
    {
    ECRStreamPauseHanling = 550,
    ECRStreamEndHandling
    };

#endif // CRTYPEDEFS_H

// End of File
