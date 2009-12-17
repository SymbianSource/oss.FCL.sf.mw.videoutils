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
* Description:    Observer class for wrapping RConnection.*
*/




#ifndef MCRCONNECTIONOBSERVER_H
#define MCRCONNECTIONOBSERVER_H

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
*  Observer for connection interface status.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class MCRConnectionObserver
    {

public: // Data types
    
    /**
    * TCRConnectionStatus is set of indications to be sent to client
    */
    enum TCRConnectionStatus
        {                        
    	ECRBearerChanged,        /**< Indicates change in network bearer */
		ECRIapDown,				 /**< Network connection lost */
    	ECRConnectionError,      /**< Unable to open connection in RTP engine & CR engine */
    	ECRConnecting,			 /**< Indicates "connection on progress" state to ui */
    	ECRAuthenticationNeeded, /**< Used to request username/password from ui */
        ECRNotEnoughBandwidth,	 /**< Stream has too high bitrate for our network */
    	ECRNormalEndOfStream,    /**< Clip ends normally */
    	ECRSwitchingToTcp,       /**< Indicates a udp->tcp switch, client may need to reset player */
    	ECRStreamIsLiveStream,   /**< Indicates that we have stream that can't be paused */
    	ECRAttachCompleted,      /**< Sent when attached to RConnection */
    	ECRStreamIsRealMedia,    /**< Indicates that stream is (unsupported) realmedia format */
        ECRTestSinkData,         /**< Data from TestSink (former NullSink) to test client observer */
		ECRSdpAvailable,         /**< Used to communicate availability of SDP */
        ECRReadyToSeek,          /**< Used to communicate seeking posibility */
        ECRRecordingStarted,     /**< Used to communicate recording state */
        ECRRecordingPaused,      /**< Used to communicate recording state */
        ECRRecordingEnded        /**< Used to communicate recording state */
        };     
    
    /**
    * This method is called after connection status changes or there is an error
    * @since Series 60 3.0
    * @param aSessionId Id to session generating the status change or message. 0 points to no session.
    * @param aConnection Reference to connection object being used.
    * @param aInterfaceUp flags if interface has gone up (ETrue)
    *        or down (EFalse).
    * @param aError error value associated.
    * @return none.
    */
    virtual void ConnectionStatusChange( TInt aSessionId,
                                         const TCRConnectionStatus aStatus,
                                         TInt aErr ) = 0;
    };

#endif // MCRCONNECTIONOBSERVER_H

// End of file
