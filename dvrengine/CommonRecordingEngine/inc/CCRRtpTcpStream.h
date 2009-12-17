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
* Description:    Abstraction for a media stream for RTP/TCP streamer*
*/




#ifndef CCRRTPTCPSTREAM_H
#define CCRRTPTCPSTREAM_H

// INCLUDE FILES
#include <e32base.h>

// FORWARD DECLARATIONS
class MCRRtpTcpObserver;

/**
*  RTP media stream for RTP/TCP streamer
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRRtpTcpStream : public CBase
    {

public: // Constructors and destructors   
    
    /**
    * Two-phased constructor
    * @param aObserver 
    */
    static CCRRtpTcpStream* NewL( MCRRtpTcpObserver& aObserver );

    /**
    * Destructor
    */
    virtual ~CCRRtpTcpStream();

public: // New methods

    /**
    * Handles incoming RTP or RTCP packet
    * @since Series 60 3.0
    * @param aChannel Interleaved channel.
    * @return none.
    */
    void PacketAvailable( TInt aChannel );

private: // Constructors and destructors

    /**
    * C++ default constructor
    */
    CCRRtpTcpStream( MCRRtpTcpObserver& aObserver );

    /**
    * Second phase constructor
    */
    void ConstructL();

private: // Data
    
    /**
    * Observer for streaming session.
    */
    MCRRtpTcpObserver& iObserver;

    /** 
    * SSRC for this media stream.
    */
    TUint32 iSSRC;
    
    };

#endif // CCRRTPTCPSTREAM_H

//  End of File
