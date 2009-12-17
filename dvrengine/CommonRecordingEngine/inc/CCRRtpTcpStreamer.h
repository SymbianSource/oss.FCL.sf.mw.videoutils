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
* Description:    RTP/TCP streamer for RTSP source.*
*/




#ifndef CCRRTPTCPSTREAMER_H
#define CCRRTPTCPSTREAMER_H

// INCLUDE FILES
#include <e32base.h>

// CONSTANTS
const TInt KCRRtpTcpStreamCount( 2 ); /** Max of two streams, audio and video */

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class MCRRtpTcpObserver;
class CCRRtpTcpStream;

// CLASS DECLARATION

/**
*  RTP/TCP streamer for RTSP packet source.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRRtpTcpStreamer : public CBase
    {

public: // Constructors and destructors   

    /**
    * Two-phased constructor
    * @param aObserver a observer to TCP streamer.
    */
    static CCRRtpTcpStreamer* NewL( MCRRtpTcpObserver& aObserver );

    /**
    *  Destructor
    */
    virtual ~CCRRtpTcpStreamer();

public: // New methods

    /**
    * Signals that input IP packet is available for parsing.
    * @since Series 60 3.0
    * @param aIpData raw RTSP/RTP/TCP data received (IP packet).
    * @param aInterleaved a TCP interleaving state.
    * @return none.
    */
    void DataAvailable( const TDesC8& aIpData,
                        const TBool& aInterleaved );

private: // Constructors and destructors
    
    /**
    *  C++ default constructor
    */
    CCRRtpTcpStreamer( MCRRtpTcpObserver& aObserver );

    /**
    *  Second phase constructor
    */
    void ConstructL();

private: // New methods

    /**
    * Handles IP packet when more data is expected.
    * @since Series 60 3.0
    * @param aIpData a data of IP packet.
    * @return true if not enougth data in IP packet.
    */
    TBool HandleMoreExpected( TPtrC8& aData );

    /**
    * Handles TCP interleave packet.
    * @since Series 60 3.0
    * @param aData a data of IP packet.
    * @return true if not enougth data in IP packet.
    */
    TBool HandleTcpPacket( TPtrC8& aData );

    /**
    * Handles RTSP control response.
    * @since Series 60 3.0
    * @param aData a data of IP packet.
    * @param aInterleaved a TCP interleaving state.
    * @return true if not enougth data in IP packet.
    */
    TBool HandleRtspResponse( TPtrC8& aData,
                              const TBool& aInterleaved );

    /**
    * Creates an packet from IP data buffer.
    * @since Series 60 3.0
    * @param aData a data of IP packet.
    * @param aLength a length of RTSP/RTP packet.
    * @return none.
    */
    void MakePacket( TPtrC8& aData, const TInt aLength );

    /**
    * Handles incoming RTP or RTCP packet, forwards to user and to RTCP engine.
    * @since Series 60 3.0
    * @param aPacket a data of RTP packet.
    * @return none.
    */
    void ForwardPacket( const TDesC8& aPacket );

private: // Data
    
    /**
    * Observer for streaming session.
    */
    MCRRtpTcpObserver& iObserver;

    /**
    * More expected bytes count.
    */
    TInt iMoreExpected;
    
    /**
    * Data received so far.
    */
    HBufC8* iIpData;
    
    /**
    * RTSP data received so far.
    */
    HBufC8* iRtspData;

    /**
    * Media streams
    */
    CCRRtpTcpStream* iStreams[KCRRtpTcpStreamCount];

    };

#endif // CCRTCPSTREAMER_H

//  End of File

