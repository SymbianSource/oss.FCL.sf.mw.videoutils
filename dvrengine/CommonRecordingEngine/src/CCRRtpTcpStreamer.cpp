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




// INCLUDE FILES
#include "CCRRtpTcpStreamer.h"
#include "CCRRtpTcpObserver.h"
#include "CCRRtpTcpStream.h"
#include "CRRTSPCommon.h"
#include <es_sock.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KCRRtpTcpHeaderLength( 4 );
const TInt KCRRtpTcpStartMark( 0x24 );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRRtpTcpStreamer::NewL
// -----------------------------------------------------------------------------
CCRRtpTcpStreamer* CCRRtpTcpStreamer::NewL( MCRRtpTcpObserver& aObserver )
    {
    CCRRtpTcpStreamer* self = new( ELeave ) CCRRtpTcpStreamer( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRRtpTcpStreamer::CCRRtpTcpStreamer
// -----------------------------------------------------------------------------
//
CCRRtpTcpStreamer::CCRRtpTcpStreamer( MCRRtpTcpObserver& aObserver )
  : iObserver( aObserver ),
    iMoreExpected( KErrNotFound ),
    iIpData( NULL )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRRtpTcpStreamer::ConstructL
// -----------------------------------------------------------------------------
//
void CCRRtpTcpStreamer::ConstructL()
    {
    // Construct streams
    for ( TInt i( 0 ); i < KCRRtpTcpStreamCount; i++ )
        {
        iStreams[i] = CCRRtpTcpStream::NewL( iObserver );
        }
    }

// -----------------------------------------------------------------------------
// CCRRtpTcpStreamer::~CCRRtpTcpStreamer
// -----------------------------------------------------------------------------
CCRRtpTcpStreamer::~CCRRtpTcpStreamer()
    {
    // Buffers
    if ( iIpData )
        {
        delete iIpData; iIpData = NULL;
        }
    if ( iRtspData )
        {
        delete iRtspData; iRtspData = NULL;
        }

    // Delete streams
    for ( TInt i( 0 ); i < KCRRtpTcpStreamCount; i++ )
        {
        if ( iStreams[i] )
            {
            delete iStreams[i]; iStreams[i] = NULL;
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtpTcpStreamer::DataAvailable
// -----------------------------------------------------------------------------
//
void CCRRtpTcpStreamer::DataAvailable(
    const TDesC8& aIpData,
    const TBool& aInterleaved )
    {
    TPtrC8 data( aIpData );
    if ( iMoreExpected > KErrNotFound && iIpData != NULL )
        {
        // More data expected
        if ( HandleMoreExpected( data ) )
            {
            return; // Need more
            }
        }

    // Can't be existing IP data at this point
    delete iIpData; iIpData = NULL;
    iMoreExpected = KErrNotFound;
    
    // Find out next packet
    do
        {
        // Search for $ (0x24) sign
        TBool tcp( EFalse );
        for ( TInt i( 0 ); aInterleaved && tcp == EFalse &&
            i < data.Length() && i < KCRRtpTcpHeaderLength; i++ )
            {
            if ( data[i] == KCRRtpTcpStartMark )
                {
                tcp = ETrue;
                data.Set( data.Mid( i ) );
                
                // Received less than full interleved header (4 bytes)
                if ( data.Length() < KCRRtpTcpHeaderLength )
                    {
                    iMoreExpected = KCRRtpTcpHeaderLength - data.Length();
                    iIpData = data.Alloc();
                    LOG1( "CCRRtpTcpStreamer::DataAvailable(), No interleave header, len: %d", data.Length() );
                    return; // Need more
                    }
                }
            }
        
        if ( tcp )
            {
            // TCP packet
            if ( HandleTcpPacket( data ) )
                {
                return; // Need more
                }
            }
        else
            {
            // RTSP response
            if ( HandleRtspResponse( data, aInterleaved ) )
                {
                return; // Need more
                }
        
            delete iRtspData; iRtspData = NULL;
            }
        }
        while ( data.Length() > 0 );
    }

// -----------------------------------------------------------------------------
// CCRRtpTcpStreamer::HandleMoreExpected
// -----------------------------------------------------------------------------
//
TBool CCRRtpTcpStreamer::HandleMoreExpected( TPtrC8& aData )
    {
    TInt len( aData.Length() );
    int used( len );
    if ( len >= iMoreExpected || iMoreExpected == KMaxTInt )
        {
        if ( iMoreExpected >= KCRRtpTcpHeaderLength ||
             iIpData->Des()[0] != KCRRtpTcpStartMark ||
             iIpData->Length() >= KCRRtpTcpHeaderLength )
            {
            // KMaxTInt is indication of unknow length in RTSP response
            if ( iMoreExpected < KMaxTInt )
                {
                used = iMoreExpected;
                iMoreExpected = KErrNotFound;
                }
            else
                {
                // Combine datas and try find out RTSP response
                delete iRtspData; iRtspData = NULL;
                iRtspData = HBufC8::New( iIpData->Length() + len );
                TPtr8 ptr( iRtspData->Des() );
                ptr.Copy( iIpData->Des() );
                ptr.Append( aData );
                aData.Set( iRtspData->Des() );
                return EFalse; // Continue
                }
            }
        else
            {
            // Fill interleave header
            iIpData = iIpData->ReAlloc( iIpData->Length() + iMoreExpected );
            TPtr8 ptr( iIpData->Des() );
            ptr.Append( aData.Mid( 0, iMoreExpected ) );
            aData.Set( aData.Mid( iMoreExpected ) );
            len = aData.Length();
            used = len;
            // Find real wanted packet length 
            iMoreExpected = ( TInt )BigEndian::Get16( ptr.Ptr() + 2 );
            if ( len == 0 )
                {
                return ETrue; // Need more
                }
            if ( len >= iMoreExpected )
                {
                used = iMoreExpected;
                iMoreExpected = KErrNotFound;
                }
            }
        }

    // Add new data to iIpData
    iIpData = iIpData->ReAlloc( iIpData->Length() + used );
    TPtr8 ptr( iIpData->Des() );
    ptr.Append( aData.Mid( 0, used ) );
    aData.Set( aData.Mid( used ) );
    if ( iMoreExpected == KErrNotFound )
        {
        ForwardPacket( ptr );
        if ( used == len )
            {
            delete iIpData; iIpData = NULL;
            return ETrue; // All handled
            }
        }
    else
        {
        iMoreExpected -= used;
        return ETrue; // Need more
        }
        
    return EFalse; // Continue
    }
    
// -----------------------------------------------------------------------------
// CCRRtpTcpStreamer::HandleTcpPacket
// -----------------------------------------------------------------------------
//
TBool CCRRtpTcpStreamer::HandleTcpPacket( TPtrC8& aData )
    {
    const TInt length( KCRRtpTcpHeaderLength + 
        ( TInt )BigEndian::Get16( aData.Ptr() + 2 ) );
    if ( aData.Length() >= length )
        {
        MakePacket( aData, length );
        }
    else
        {
        // Need more data
        iMoreExpected = length - aData.Length();
        iIpData = aData.Alloc();
        return ETrue; // Need more
        }
    
    return EFalse; // Continue
    }
    
// -----------------------------------------------------------------------------
// CCRRtpTcpStreamer::HandleRtspResponse
// -----------------------------------------------------------------------------
//
TBool CCRRtpTcpStreamer::HandleRtspResponse(
    TPtrC8& aData,
    const TBool& aInterleaved )
    {
    TInt point( aData.FindC( KCRRTSPReplyHeader ) );
    if ( point > KErrNotFound )
        {
        aData.Set( aData.Mid( point ) );
        
        // Search for double CRLF combination
        TInt crlf2( aData.FindC( KCR2NewLines ) );
		if ( crlf2 > KErrNotFound )
		    {
		    crlf2 += KCR2NewLines().Length();
		    }

        // Content length
        point = aData.FindC( KCRRTSPContentLength() );
        if ( point > KErrNotFound && crlf2 > KErrNotFound )
            {
            point += KCRRTSPContentLength().Length();
            TInt contentLen( KErrNotFound );
            TLex8 contentLenLex( aData.Mid( point, 5 ) );
            if ( contentLenLex.Val( contentLen ) < KErrNone )
                {
                LOG1( "CCRRtpTcpStreamer::HandleRtspResponse(), Content length parse failed, Dumped %d bytes !", aData.Length() );
                return ETrue;
                }
            
            LOG1( "CCRRtspCommon::HandleRtspResponse(), contentLen %d", contentLen );
            // Verify that enought data in IP packet
            if ( aData.Length() >= ( crlf2 + contentLen ) )
                {
                MakePacket( aData, crlf2 + contentLen );
                }
            else
                {
                // Need more
                iIpData = aData.Alloc();
                iMoreExpected = crlf2 + contentLen - aData.Length();
                return ETrue;
                }
            }
        else
            {
            // Content length not defined, RTSP command should end to double CRLF
            if ( crlf2 > KErrNotFound )
                {
                MakePacket( aData, crlf2 );
                }
            else
                {
                LOG( "CCRRtpTcpStreamer::HandleRtspResponse(), No double CRLF.." );
                
                // Look for single CRLF
                point = aData.FindC( KCRNewLine );
                if ( point > KErrNotFound )
                    {
                    // If not interleaved, all data belongs to RTSP response
                    if ( !aInterleaved )
                        {
                        if ( aData.Mid( aData.Length() - KCR2NewLines().Length() ).
                            FindF( KCRNewLine ) > KErrNotFound )
                            {
                            ForwardPacket( aData );
                            return ETrue;
                            }
                        
                        // Not complete but total length unknown
                        LOG( "CCRRtpTcpStreamer::HandleRtspResponse(), Need more without known length.." );
                        iIpData = aData.Alloc();
                        iMoreExpected = KMaxTInt;
                        return ETrue;
                        }

                    // Only one CRLF after RTSP response, find last
                    point += KCRNewLine().Length();
                    for ( TInt i( point ); i < aData.Length(); )
                        {
                        TInt next( aData.Mid( point ).FindC( KCRNewLine ) );
                        if ( next > KErrNotFound )
                            {
                            point += ( next + KCRNewLine().Length() );
                            i = point;
                            }
                        else
                            {
                            i = aData.Length();
                            }
                        }

                    LOG1( "CCRRtpTcpStreamer::HandleRtspResponse(), Last CRLF at index: %d", point );
                    MakePacket( aData, point );
                    }
                else
                    {
                    // Not any CRLF, can not be RTSP response
                    LOG1( "CCRRtpTcpStreamer::HandleRtspResponse(), No CRLF, Dumped %d bytes !", aData.Length() );
                    return ETrue;
                    }
                }
            }
        }
    else
        {
        LOG1( "CCRRtpTcpStreamer::HandleRtspResponse(), Not RTSP message, Dumped %d bytes !", aData.Length() );
        return ETrue;
        }
    
    return EFalse;
    }
    
// -----------------------------------------------------------------------------
// CCRRtpTcpStreamer::MakePacket
// -----------------------------------------------------------------------------
//
void CCRRtpTcpStreamer::MakePacket( TPtrC8& aData, const TInt aLength )
    {
    ForwardPacket( aData.Mid( 0, aLength ) );
    aData.Set( aData.Mid( aLength ) );
    }
    
// -----------------------------------------------------------------------------
// CCRRtpTcpStreamer::ForwardPacket
// -----------------------------------------------------------------------------
//
void CCRRtpTcpStreamer::ForwardPacket( const TDesC8& aPacket )
    {
    if ( aPacket[0] == KCRRtpTcpStartMark )
        {
        // 1. Forward (actually return or signal reception of) packet to user
        const TInt channel( ( TInt )aPacket[1] );
        iObserver.RtpTcpPacketAvailable(
            channel, aPacket.Mid( KCRRtpTcpHeaderLength ) );

        // 2. Map channel to internal stream, ignore non audio or video
        const TInt streamId( channel / 2 );
        if ( streamId >= 0 && streamId < KCRRtpTcpStreamCount )
            {
            iStreams[streamId]->PacketAvailable( channel );
            }
        }
    else
        {
        // RTSP
        iObserver.RtspMsgAvailable( aPacket );
        }
    }

//  End of File
