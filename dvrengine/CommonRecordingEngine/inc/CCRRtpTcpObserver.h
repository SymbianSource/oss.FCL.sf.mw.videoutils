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
* Description:    Observer for RTP/TCP streamer.*
*/




#ifndef CCRRTPTCPOBSERVER_H
#define CCRRTPTCPOBSERVER_H

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
*  Observer for RTP/TCP streamer
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class MCRRtpTcpObserver
    {

public: // New methods

    /**
    * Signals an available incoming RTSP control message
    * @since Series 60 3.0
    * @param aData a RTSP meaage data.
    * @return none.
    */
    virtual void RtspMsgAvailable( const TDesC8& aData ) = 0;

    /**
    * Signals an available incoming RTP/RTCP packet from RTSP/TCP streaming
    * @since Series 60 3.0
    * @param aChannel channel through which packet was received.
    *        Actual interpretation and mapping to media stream (audio, video)
    *        done by user.
    * @param aPacket a RTP/RTCP packet received.
    * @return none.
    */
    virtual void RtpTcpPacketAvailable( TInt aChannel,
                                        const TDesC8& aPacket ) = 0;

    /**
    * Signals an outgoing RTP/RTCP packet to be sent over RTSP/TCP
    * @since Series 60 3.0
    * @param aChannel a channel through which packet was received.
    *        Actual interpretation and mapping to media stream (audio, video)
    *        done by user.
    *  @param aPacket   RTP/RTCP packet received
    * @return none.
    */
    virtual void ForwardRtpTcpChunck( const TDesC8& aChunk ) = 0;

    };

#endif // CCRRTPTCPOBSERVER_H
