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
* Description:    Engine observer to get event notifications.*
*/


#ifndef MCRENGINEOBSERVER_H
#define MCRENGINEOBSERVER_H

// INCLUDES
#include <e32def.h>

// CONSTANTS
// None

// MACROS
// None.

// DATA TYPES
/**
* Different messages that DVR engine may send via client lib callback
*/
enum TCRObserverInfo
    {
    ECRAuthenticationNeeded, /**< CR needs new username and password for authentication */
    ECRAttachCompleted,      /**< Connection attach is completed and is ready to be used */
    ECRConnectionError,      /**< Unable to open connection in RTP engine & CR engine */
    ECRNotEnoughBandwidth,   /**< Stream bitrate higher than (estimated) available connection bandwidth */
    ECRNormalEndOfStream,    /**< Used to communicate end of streaming */
    ECRSwitchingToTcp,       /**< Switch to tcp, client needs to initiate possible player */   
    ECRStreamIsLiveStream,   /**< We have a stream that cannot be paused */
    ECRRealNetworksStream,   /**< RealMedia stream format, cannot support */
    ECRTestSinkData,         /**< Placeholder for TestSink (NullSink) feedback information to test client */
    ECRSdpAvailable,         /**< Used to communicate availability of SDP */            
    ECRReadyToSeek,          /**< Used to communicate seeking posibility */
    ECRRecordingStarted,     /**< Used to communicate recording state */
    ECRRecordingPaused,      /**< Used to communicate recording state */
    ECRRecordingEnded        /**< Used to communicate recording state */
    };

/**
* This structure is sent over message queue where
* api sits listening
*/
struct SCRObserverMsg
    {
    TCRObserverInfo iMsg;    /**< Identifies the message */
    TInt            iErr;    /**< Can be used to pass error value related to message */
    };

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION

/**
*  Engine observer to get event notifications.
*
*  @lib CommonRecordingEngineClient.lib
*  @since Series 60 3.0
*/
class MCREngineObserver 
    {

public: // New methods

    /**
    * Notify call back for statuses.
    * @since Series 60 3.0
    * @param aInfo a notify info.
    * @return none.
    */
    virtual void NotifyL( const SCRObserverMsg& aInfo ) = 0;
    
    };
    
#endif MCRENGINEOBSERVER_H

// End of file.
