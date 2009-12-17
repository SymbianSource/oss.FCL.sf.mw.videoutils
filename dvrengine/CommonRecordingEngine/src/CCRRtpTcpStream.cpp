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
* Description:    Single media stream for RTP/TCP streamer*
*/




// INCLUDE FILES
#include "CCRRtpTcpStream.h"
#include "CCRRtpTcpObserver.h"
#include <es_sock.h>
#include <e32math.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KReceiverReportLength( 12 );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRRtpTcpStream::CCRRtpTcpStream
// -----------------------------------------------------------------------------
//
CCRRtpTcpStream::CCRRtpTcpStream( MCRRtpTcpObserver& aObserver )
  : iObserver(aObserver)
    {
    iSSRC = Math::Random();
    }

// -----------------------------------------------------------------------------
// CCRRtpTcpStream::NewL
// -----------------------------------------------------------------------------
//
CCRRtpTcpStream* CCRRtpTcpStream::NewL( MCRRtpTcpObserver& aObserver )
    {
    CCRRtpTcpStream* self = new( ELeave ) CCRRtpTcpStream( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRRtpTcpStream::ConstructL
// -----------------------------------------------------------------------------
//
void CCRRtpTcpStream::ConstructL()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRRtpTcpStream::~CCRRtpTcpStream
// -----------------------------------------------------------------------------
//
CCRRtpTcpStream::~CCRRtpTcpStream()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRRtpTcpStream::ForwardPacketAvailable
// -----------------------------------------------------------------------------
//
void CCRRtpTcpStream::PacketAvailable( TInt aChannel )
    {
    // Nothing to to do for RTP, just ignore
    if ( !( aChannel % 2 ) )
        {
        return;
        }

    // Very simple Receiver Report generation:
    // - RC=0, no SSRC report blocks -> no statistics keeping
    // - one RR for every SR received -> no timing, back-off, etc
    TBuf8<KReceiverReportLength> chunk( KNullDesC8 );
    chunk.SetLength( KReceiverReportLength );

    // RTSP header
    chunk[0] = ( TUint8 )( 0x24 );     // magic '$' for embedded binary in RTSP     
    chunk[1] = ( TUint8 )( aChannel ); // enbedded RTSP channel
    // Embedded packet length, fixed
    BigEndian::Put16( ( TUint8* )chunk.Ptr() + 2, 8 ); 

    // RR
    chunk[4] = 0x80; // v=2, p=0, rc=0
    chunk[5] = 201;  // PT= RR= 201
    // Length=1 ( in 32bits, -1 )
    BigEndian::Put16( ( TUint8* )chunk.Ptr() + 6, 1 );     
    BigEndian::Put32( ( TUint8* )chunk.Ptr() + 8, iSSRC );

    iObserver.ForwardRtpTcpChunck( chunk );
    }

//  End of File
