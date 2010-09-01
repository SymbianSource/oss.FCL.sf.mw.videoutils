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
* Description:    Class that takes packet from the buffer and forward them thrue*
*/




// INCLUDES
#include "CCRXpsSink.h"
#include "CCRPacketBuffer.h"
#include "CCRStreamingSession.h"
#include <ipvideo/CDvrSdpParser.h>
#include "CCRTimer.h"
#include "CRtpPacket.h"
#include "CRtpTsConverter.h"
#include <CXPSPacketSink.h>

// CONSTANTS
_LIT( KCRXpsServerName, "IpVideoXps" );
_LIT8( KAttributeDefRange, "a=range:npt=0-86400.0" );
#ifdef VIA_FEA_IPTV_USE_IPDC
_LIT8( KMtvAvc, "X-MTV-AVC" );
_LIT8( KHxAvc1, "X-HX-AVC1" );
#endif // VIA_FEA_IPTV_USE_IPDC
const TInt KRangeIdentifierLen( 8 );
const TInt KXpsBufferedPackets( 300 );      // about 3s
const TInt KXpsOverflowDelay( 300 * 1000 ); // 300ms

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRXpsSink::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//  
CCRXpsSink* CCRXpsSink::NewL(
    CCRStreamingSession::TCRSinkId aSinkId,
    CCRStreamingSession& aOwningSession )
    {
    CCRXpsSink* self = new( ELeave ) 
    CCRXpsSink( aSinkId, aOwningSession );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::CCRXpsSink
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//  
CCRXpsSink::CCRXpsSink(
    CCRStreamingSession::TCRSinkId aSinkId,
    CCRStreamingSession& aOwningSession )
  : CCRPacketSinkBase( aOwningSession, aSinkId ),
    iWaitPlayer( KErrNotFound ),
    iRequested( KErrNotFound ),
    iXpsResetOk( EFalse ),
    iAudioStreamId( KErrNotFound ),
    iVideoStreamId( KErrNotFound ),
    iTitleStreamId( KErrNotFound )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::ConstructL
// 2nd phase. 
// -----------------------------------------------------------------------------
//  
void CCRXpsSink::ConstructL()
    {
    LOG( "CCRXpsSink::ConstructL() in" );

    iRtpPacket = CRtpPacket::NewL();
    iPacketSink = CXPSPacketSink::New();
    LOG1( "CCRXpsSink::ConstructL(), iPacketSink: %d", iPacketSink );
    User::LeaveIfNull( iPacketSink );
    TInt err( iPacketSink->Init( KCRXpsServerName, this ) );
    if ( err )
        {
        LOG1( "CCRXpsSink::ConstructL(), iPacketSink->Init() err: %d", err );
        User::Leave( err );
        }

    LOG( "CCRXpsSink::ConstructL() out" );
    }
    
// -----------------------------------------------------------------------------
// CCRXpsSink::~CCRXpsSink
// Destructor.
// -----------------------------------------------------------------------------
//
CCRXpsSink::~CCRXpsSink()
    {    
    LOG( "CCRXpsSink::~CCRXpsSink()" );

    // Delete variables
    StopTimer();
    delete iPacketSink;
    delete iAudioConv;
    delete iVideoConv;
    delete iTitleConv;
    delete iRtpPacket;

#ifdef VIA_FEA_IPTV_USE_IPDC
    delete iVideoDepacketizer;
#endif // VIA_FEA_IPTV_USE_IPDC
    }
    
// -----------------------------------------------------------------------------
// CCRXpsSink::SetSdpL
// Sets SDP, parses it and initiates XPS.
// -----------------------------------------------------------------------------
//  
void CCRXpsSink::SetSdpL( const TDesC8& aSdp )
    {
    LOG1( "CCRXpsSink::SetSdpL(), SDP len: %d", aSdp.Length() );

#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    LOG( "CCRXpsSink::SetSdpL(), SDP content:" );
    TName d( KNullDesC );
    for ( TInt i( 0 );  i < aSdp.Length(); i++ )
        {
        TChar c = aSdp[i];
        d.Append( c );
        if ( ( i > 0 ) && ( i % 80 ) == 0 )
            {
            LOG1( ">%S<", &d );
            d.Zero();
            }
        }
    
    LOG1( ">%S<", &d );
#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE
    
    if ( iWaitPlayer != KErrNotFound )
        {
        LOG( "CCRXpsSink::SetSdpL(), SDP already set !" );
        return;
        }
         
    // SDP parser
    CDvrSdpParser* sdpParser = CDvrSdpParser::NewLC();
    sdpParser->TryParseL( aSdp );
    
    // TS converter
    delete iAudioConv; iAudioConv = NULL;
    iAudioConv = CRtpTsConverter::NewL( sdpParser->AudioTimerGranularity() );
    LOG1( "CCRXpsSink::SetSdpL(), AudioTimerGranularity: %d",
                                  sdpParser->AudioTimerGranularity() );
    
    delete iVideoConv; iVideoConv = NULL;
    iVideoConv = CRtpTsConverter::NewL( sdpParser->VideoTimerGranularity() );
    LOG1( "CCRXpsSink::SetSdpL(), VideoTimerGranularity: %d",
                                  sdpParser->VideoTimerGranularity() );

    delete iTitleConv; iTitleConv = NULL;
    
    // Streams count
    TInt streams( 0 );
    const CDvrSdpParser::TDvrPacketProvidings providings(
        sdpParser->SupportedContent() );
    if ( providings == CDvrSdpParser::EDvrAudioOnly ||
         providings == CDvrSdpParser::EDvrVideoOnly )
        {
        streams = 1;
        }
    if ( providings == CDvrSdpParser::EDvrBothAudioAndVideo )
        {
        streams = 2;
        }

    // Stream ids
    iAudioStreamId = sdpParser->AudioStreamId();
    iVideoStreamId = sdpParser->VideoStreamId();
    LOG2( "CCRXpsSink::SetSdpL(), iAudioStreamId: %d, iVideoStreamId: %d",
                                  iAudioStreamId, iVideoStreamId );
    // Verify/update range
    if ( aSdp.FindC( 
         KAttributeDefRange().Left( KRangeIdentifierLen ) ) == KErrNotFound )
    	{
    	LOG( "CCRXpsSink::SetSdpL(), setting default range" );
    	iRangeKnown = EFalse;
    	sdpParser->NewLineL( KErrNotFound, KAttributeDefRange );
	    }
	else
        {
        LOG( "CCRXpsSink::SetSdpL() sdp already did contain range, not changed" );
        iRangeKnown = ETrue;
        }

    // Get SDP data
    TPtrC8 sdp( NULL, 0 );
    User::LeaveIfError( sdpParser->GetSdp( sdp ) );
    HBufC8* tmpSdpData = NULL;
    
    // See if recorded from ISMA crypted content
#ifdef VIA_FEA_IPTV_USE_IPDC
    TInt mimePos( sdp.Find( KMtvAvc ) );
    if ( mimePos != KErrNotFound || sdp.Find( KHxAvc1 ) != KErrNotFound )
        {
        LOG( "CCRXpsSink::SetSdpL(), Playback of ISMA clip.." );
        delete iVideoDepacketizer; iVideoDepacketizer = NULL;
        iVideoDepacketizer = CH264Mpeg4GenrToFileformat::New();
        User::LeaveIfNull( iVideoDepacketizer );
        HBufC8* fmtp = FindFmtpLC( sdp );
        TInt err( iVideoDepacketizer->Init( *fmtp ) );
        CleanupStack::PopAndDestroy( fmtp );
        if ( err )
            {
            delete iVideoDepacketizer; iVideoDepacketizer = NULL;
            LOG1( "CCRXpsSink::SetSdpL(), Depacketizer Init() failed: %d", err );
            }
        else
            {
            // Change MIME type from X-MTV-AVC to X-HX-AVC1 for playback
            // KMtvAvc mime prevents Helix crash in non DVB-H phones
            if ( mimePos != KErrNotFound )
                {
                tmpSdpData = HBufC8::NewLC( sdp.Length() -
                    KMtvAvc().Length() + KHxAvc1().Length() );
                TPtr8 ptr( tmpSdpData->Des() );
                ptr.Copy( sdp.Left( mimePos ) );
                ptr.Append( KHxAvc1 );
                ptr.Append( sdp.Mid( mimePos + KMtvAvc().Length() ) );
                sdp.Set( ptr );
                }
            }
        }
#endif // VIA_FEA_IPTV_USE_IPDC

    // Pass SDP to XPS
    LOG( "CCRXpsSink::SetSdpL(), iPacketSink->SetSessionDescription.." );
    User::LeaveIfError( iPacketSink->SetSessionDescription( sdp, streams ) );
    if ( tmpSdpData != NULL )
        {
        CleanupStack::PopAndDestroy( tmpSdpData );
        }
    
    // Config streams
    for ( TInt i( 0 ); i < streams; i++ )
        {
        LOG2( "CCRXpsSink::SetSdpL(), iPacketSink->ConfigStream: %d, KXpsBufferedPackets: %d",
            i, KXpsBufferedPackets );
        User::LeaveIfError( iPacketSink->ConfigStream( i, KXpsBufferedPackets ) ); 
        }

    CleanupStack::PopAndDestroy( sdpParser );
    iWaitPlayer = KErrNone;
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::NewPacketAvailable
// From CCRPacketSinkBase.
// -----------------------------------------------------------------------------
//      
void CCRXpsSink::NewPacketAvailable()
    {
    // Kill flow timer
    StopTimer();

    // Ok to enqueue?
    if ( iBuffer )
        {
        if ( iWaitPlayer == KErrNone )
            {
            // Enqueue packet
            if ( SendPacket() )
                {
                // Keep buffer size reasonable
                iBuffer->HandleBufferSize();
                }
        	
            if ( iBuffer->ContinousStream() )
                {
                // Make sure all will be sent from the buffer in continous stream case
                if ( iBuffer->PacketsCount( iSinkId ) > KErrNotFound )
                    {
                    StartTimer( 0 );
                    }
                }
            else
                {
                // Group done, need request more
                if ( !iBuffer->MoreComing() )
                    {
                    StartTimer( 0 );
                    }
                }
            }
        else
            {
            iBuffer->HandleBufferSize();
            // Make sure that process never end  
            if ( !iBuffer->ContinousStream() )
                {
                StartTimer( KXpsOverflowDelay );
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::BufferResetting
// From CCRPacketSinkBase.
// -----------------------------------------------------------------------------
//      
void CCRXpsSink::BufferResetDone()
    {
    StopTimer();
    iWaitPlayer = KErrNone;
	iRequested = KErrNotFound;
    
    // XPS reset possible?
    if ( iXpsResetOk )
        {
        LOG( "CCRXpsSink::BufferResetDone(), Resets XPS.." );
        iPacketSink->Reset();
        iXpsResetOk = EFalse;
        }
    
    // Uninit TS converters
    LOG( "CCRXpsSink::BufferResetDone(), Uninitiates TS converters.." );
    if ( iAudioConv )
        {
        iAudioConv->UnInitiate();
        }
    if ( iVideoConv )
        {
        iVideoConv->UnInitiate();
        }
    if ( iTitleConv )
        {
        iTitleConv->UnInitiate();
        }

#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    iLogXps = 0;
#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE
    }
    
// -----------------------------------------------------------------------------
// CCRXpsSink::TimerExpired
// From MCRTimerObserver.
// -----------------------------------------------------------------------------
//
void CCRXpsSink::TimerExpired( CCRTimer* /*aTimer*/ )
    {
    RestoreSink();
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::RestorePacketSupply
// From CCRPacketSinkBase.
// -----------------------------------------------------------------------------
//      
void CCRXpsSink::RestorePacketSupply( TUint aStreamId )
    {
#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    if ( iLogXps < 5 )
        {
        iLogXps++;
        LOG2( "CCRXpsSink::RestorePacketSupply(), aStreamId: %d, iWaitPlayer: %d",
            aStreamId, iWaitPlayer );
        }
#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE

    if ( iWaitPlayer != TInt( ETrue ) )
        {
        if ( iBuffer->ContinousStream() )
            {
            iRequested = aStreamId;
            iWaitPlayer = KErrNone;
            RestoreSink();
            }
        else
            {
            iWaitPlayer = KErrNone;
            StartTimer( KXpsOverflowDelay );
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::RestoreSink
// -----------------------------------------------------------------------------
//
void CCRXpsSink::RestoreSink()
    {
    if ( iBuffer )
        {
        // See if more waits in packet buffer
        if ( iBuffer->PacketsCount( iSinkId ) > KErrNotFound )
            {
            NewPacketAvailable();
            }
        else
            {
            StopTimer();
            // This is only for non continous stream, like .rtp clip
            iOwningSession.SourceRestore();
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::SendPacket
// Initialises time stamp converter for audio and video stream and passes RTP
// packets with adjusted time stamp to XPS interface. All packets before
// converter has beeen initialized are dumbed.
// -----------------------------------------------------------------------------
//
TBool CCRXpsSink::SendPacket()
    {
    TBool packetSent( ETrue );
    MCRPacketSource::TCRPacketStreamId bufferId( MCRPacketSource::EStreamIdCount );
    const TInt book( iBuffer->GetStream( iSinkId, bufferId ) );
    TPtr8 packet( NULL, 0 );

    // Packet type related action
    TInt err( KErrNone );
    switch ( bufferId )
        {
        case MCRPacketSource::EAudioStream:
            if ( iAudioConv->Initiated() )
                {
                iBuffer->GetPacket( book, packet );
                if ( iRequested == KErrNotFound || iRequested == iAudioStreamId )
                	{
                	iRequested = KErrNotFound;
                	err = SendAudioPacket( packet );
                	}
                else
                	{
                    LOG( "CCRXpsSink::SendPacket(), Audio packet DROPPED after XPS overflow !" ) 
                	}
                }
            else
                {
                packetSent = EFalse; 
                SearchForControlStreamPackets();
                }
            break;

        case MCRPacketSource::EAudioControlStream:
            {
            iBuffer->GetPacket( book, packet );
            if ( !iAudioConv->Initiated() )
                {
                iAudioConv->Init( packet );
                LOG1( "CCRXpsSink::SendPacket(), Audio TS initiated, status: %d", 
                    iAudioConv->Initiated() )
                }
            }
            break;

        case MCRPacketSource::EVideoStream:
            if ( iVideoConv->Initiated() )
                {
                iBuffer->GetPacket( book, packet );
                if ( iRequested == KErrNotFound || iRequested == iVideoStreamId )
                	{
                	iRequested = KErrNotFound;
                	err = SendVideoPacket( packet );
                	}
                else
                	{
                    LOG( "CCRXpsSink::SendPacket(), Video packet DROPPED after XPS overflow !" ) 
                	}
                }
            else
                {
                packetSent = EFalse;
                SearchForControlStreamPackets();
                }
            break;

        case MCRPacketSource::EVideoControlStream:
            {
            iBuffer->GetPacket( book, packet );
            if ( !iVideoConv->Initiated() )
                {
                iVideoConv->Init( packet );
                LOG1( "CCRXpsSink::SendPacket(), Video TS initiated, status: %d", 
                    iVideoConv->Initiated() )
                }
            }
            break;
        
        case MCRPacketSource::ESubTitleStream:
            if ( iTitleConv->Initiated() )
                {
                iBuffer->GetPacket( book, packet );
                err = SendTitlePacket( packet );
                }
            else
                {
                packetSent = EFalse;
                SearchForControlStreamPackets();
                }
            break;

        case MCRPacketSource::ESubTitleControlStream:
            {
            iBuffer->GetPacket( book, packet );
            if ( !iTitleConv->Initiated() )
                {
                iTitleConv->Init( packet );
                LOG1( "CCRXpsSink::SendPacket(), Title TS initiated, status: %d", 
                    iTitleConv->Initiated() )
                }
            }
            break;
        
        case MCRPacketSource::EDisContinousStream:
        	{
            LOG( "CCRXpsSink::SendPacket(), EDiscontinousStream" );
            // Just wait player's "MvloLoadingStartedL" event,
            // PlayCommand( -1.0, -1.0 ) will then handle pause packet
            iWaitPlayer = ETrue;
            // Used packet out from the buffer
            iBuffer->GetPacket( book, packet );
        	}
            break;

        case MCRPacketSource::EStreamEndTag:
            LOG1( "CCRXpsSink::SendPacket(), EStreamEndTag, iRangeKnown: %d", iRangeKnown );
        	if ( iRangeKnown )
        		{
                if ( iAudioStreamId > KErrNotFound )
                	{
        		    iPacketSink->StreamEnd( iAudioStreamId );
                	}
                if ( iVideoStreamId > KErrNotFound )
                	{
        	        iPacketSink->StreamEnd( iVideoStreamId );
                	}
        		}
        	//else
        	//	{
        	//  Just wait player's "MvloLoadingStartedL" event,
            //  Play ends with PlayCommand( -1.0, -1.0 ) in .rtp clip case and/or
        	//  VIA will stop the play if play position goes beond the clip's lenght
            //  }
            
        	// Used packet out from the buffer
            iBuffer->GetPacket( book, packet );
            break;
        
        default:
            LOG1( "CCRXpsSink::SendPacket(), Bad bufferId: %d", bufferId );
        	// Used packet out from the buffer
            iBuffer->GetPacket( book, packet );
            break;
        }
    
    // Stop sink if error?
    if ( err )
        {
        LOG2( "CCRXpsSink::SendPacket(), error: %d, bufferId: %d", err, bufferId );
        LOG2( "CCRXpsSink::SendPacket(), iAudioStreamId: %d, iVideoStreamId: %d",
                                         iAudioStreamId, iVideoStreamId );
        iOwningSession.SinkStops( Id() );
        }
    
    return packetSent;
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::SendAudioPacket
// Adjust RTP timestamp and enqueue the packet.
// -----------------------------------------------------------------------------
//
TInt CCRXpsSink::SendAudioPacket( const TDesC8& aPacket )
    {
    // Parse packet
    TInt err( iRtpPacket->ParseRtp( aPacket ) );
    if ( err )
        {
        LOG1( "CCRXpsSink::SendAudioPacket(), Parsing error: %d", err );
        return err;
        }
    
    // Adjust time stamp
    iRtpPacket->SetTimeStamp(
        iAudioConv->ConvertTs( iRtpPacket->iRtpRecvHeader.iTimestamp ) );

    // Send to player
    return EnqueuePacket( iAudioStreamId );
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::SendVideoPacket
// Adjust RTP timestamp and enqueue the packet.
// -----------------------------------------------------------------------------
//
TInt CCRXpsSink::SendVideoPacket( const TDesC8& aPacket )
    {
    TPtrC8 packet( aPacket );

#ifdef VIA_FEA_IPTV_USE_IPDC
    // Do ISMA Depacketizer
    if ( iVideoDepacketizer != NULL )
        {
        TInt result( iVideoDepacketizer->PushPacket( packet ) );
        if ( result != KErrCompletion ) // KErrCompletion means Ok
            {
            return KErrNone;
            }

        // Next packet should be available
        TInt err( iVideoDepacketizer->NextFrame( packet ) );
        if ( err )
            {
            LOG1( "CCRXpsSink::SendVideoPacket(), NextFrame error: %d", err );
            return err;
            }
        }
#endif // VIA_FEA_IPTV_USE_IPDC

    // Parse packet
    TInt err( iRtpPacket->ParseRtp( packet ) );
    if ( err )
        {
        LOG1( "CCRXpsSink::SendVideoPacket(), Parsing error: %d", err );
        return err;
        }
    
    // Adjust time stamp
    iRtpPacket->SetTimeStamp( 
        iVideoConv->ConvertTs( iRtpPacket->iRtpRecvHeader.iTimestamp ) );
    
    // Send to player
    return EnqueuePacket( iVideoStreamId );
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::SendTitlePacket
// Adjust RTP timestamp and enqueue the packet.
// -----------------------------------------------------------------------------
//
TInt CCRXpsSink::SendTitlePacket( const TDesC8& /*aPacket*/ )
    {
    // Title implementation unknown
    return KErrNotSupported;
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::EnqueuePacket
// Sends packet to the player.
// -----------------------------------------------------------------------------
//
TInt CCRXpsSink::EnqueuePacket( const TUint aStreamId )
    {
    TInt err( iPacketSink->Enqueue(
        aStreamId, iRtpPacket->iRtpRecvHeader, iRtpPacket->iPayload ) );

#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    if ( err && ( iLogXps < 5 || err != KErrOverflow ) )
        {
        LOG3( "CCRXpsSink::EnqueuePacket(), aStreamId: %d, err: %d, payload len: %d",
            aStreamId, err, iRtpPacket->iPayload.Length() );
        }
#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE

    if ( err == KErrOverflow )
        {
        iWaitPlayer = err;
        return KErrNone;
        }
    
    // XPS reset can not be done before first packet is enqued
    iXpsResetOk = ETrue;
    return err; 
    }
    
// -----------------------------------------------------------------------------
// CCRXpsSink::SearchForControlStreamPackets
// Checks buffer for control stream packets.
// -----------------------------------------------------------------------------
//
void CCRXpsSink::SearchForControlStreamPackets()
    {
    // Check if RTCP packet already in buffer
    if ( CheckBufferForControlStreamPackets() )
        {
        iBuffer->AdjustBuffer();
        }
    else
        {
        // Get more packets if group not contains any RTCP packet(s)
        if ( !iBuffer->ContinousStream() && !iBuffer->MoreComing() )
            {
            iBuffer->AdjustBuffer();
            iOwningSession.SourceRestore();
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::CheckBufferForControlStreamPackets
// Checks buffer for control stream packets.
// -----------------------------------------------------------------------------
//
TBool CCRXpsSink::CheckBufferForControlStreamPackets()
    {
    TBool audioOk( iAudioConv->Initiated() || iAudioStreamId == KErrNotFound );
    TBool videoOk( iVideoConv->Initiated() || iVideoStreamId == KErrNotFound );
    
    // Loop packets, oldest first
    for ( TInt offset( iBuffer->PacketsCount( iSinkId ) - 1 );
        ( !audioOk || !videoOk ) && offset >= 0; offset-- )
        {
        MCRPacketSource::TCRPacketStreamId streamId( MCRPacketSource::EStreamIdCount );
        const TInt book( iBuffer->GetStream( iSinkId, offset, streamId ) );
        TPtr8 packet( NULL, 0 );
        
        switch ( streamId )
            {
            case MCRPacketSource::EAudioControlStream:
                if ( !iAudioConv->Initiated() )
                    {
                    audioOk = ETrue;
                    iBuffer->PeekPacket( book, packet, offset );
                    iAudioConv->Init( packet );
                    LOG1( "CCRXpsSink::CheckBufferForControlStreamPackets(), Audio TS initiated, status: %d", 
                            iAudioConv->Initiated() )
                    }
                break;
                
            case MCRPacketSource::EVideoControlStream:
                if ( !iVideoConv->Initiated() )
                    {
                    videoOk = ETrue;
                    iBuffer->PeekPacket( book, packet, offset );
                    iVideoConv->Init( packet );
                    LOG1( "CCRXpsSink::CheckBufferForControlStreamPackets(), Video TS initiated, status: %d", 
                            iVideoConv->Initiated() )
                    }
                break;
                
            case MCRPacketSource::ESubTitleControlStream:
                if ( !iTitleConv->Initiated() )
                    {
                    iBuffer->PeekPacket( book, packet, offset );
                    iTitleConv->Init( packet );
                    LOG1( "CCRXpsSink::CheckBufferForControlStreamPackets(), Title TS initiated, status: %d", 
                            iTitleConv->Initiated() )
                    }
                break;
            
            default:
                break;
            }
        }
    
    return ( audioOk && videoOk );
    }

// -----------------------------------------------------------------------------
// CCRXpsSink::StartTimer
// Starts packet flow timer.
// -----------------------------------------------------------------------------
//
void CCRXpsSink::StartTimer( const TInt& aInterval )
    {
    StopTimer();
    TRAPD( err, iFlowTimer = CCRTimer::NewL( 
                             CActive::EPriorityLow, *this ) );
    if ( !err )
        {
        iFlowTimer->After( aInterval );
        }
    else
        {
        LOG1( "CCRXpsSink::StartTimer(), Flowtimer err: %d", err );
        iOwningSession.SinkStops( Id() );
        }
    }
    
// -----------------------------------------------------------------------------
// CCRXpsSink::StopTimer
// Starts packet flow timer.
// -----------------------------------------------------------------------------
//
void CCRXpsSink::StopTimer()
    {
    delete iFlowTimer; iFlowTimer = NULL;
    }
    
#ifdef VIA_FEA_IPTV_USE_IPDC
// -----------------------------------------------------------------------------
// CCRXpsSink::FindFmtpL
// Finds the fmtp string.
// -----------------------------------------------------------------------------
//
HBufC8* CCRXpsSink::FindFmtpLC( const TDesC8& aSdpData )
    {
    LOG( "CCRXpsSink::FindFmtpLC() in" );
    _LIT8( KCRStr, "\r" );
    _LIT8( KLFStr, "\n" );
    _LIT8( KHxAVCfmtp, "a=hxavcfmtp:" );
    
    // Get the video fmtp string
    HBufC8* fmtp = NULL;
    TInt pos = aSdpData.Find( KHxAVCfmtp );
    if ( pos > KErrNotFound )
        {
        // Extract the right most from the fist char after KHxAVCfmtp
        TPtrC8 rightPtr( aSdpData.Mid( pos + KHxAVCfmtp().Length() ) );
        
        // We need the first line of rightPtr
        TInt posLFStr( rightPtr.Find( KLFStr ) );
        TInt posCRStr( rightPtr.Find( KCRStr ) );
        if ( posLFStr > 0 && posCRStr > 0 )
            {
            fmtp = rightPtr.Left( Min( posLFStr, posCRStr ) ).AllocLC();
            }
        else if ( posLFStr > 0 )
            {
            fmtp = rightPtr.Left( posLFStr ).AllocLC();
            }
        else if ( posCRStr > 0 )
            {
            fmtp = rightPtr.Left( posCRStr ).AllocLC();
            }
        else
        	{
        	fmtp = rightPtr.AllocLC();
            }
        }
    
    User::LeaveIfNull( fmtp );
#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    HBufC* buf = HBufC::NewL( fmtp->Length() );
    TPtr ptr( buf->Des() ); ptr.Copy( *fmtp );
    LOG1( "CCRXpsSink::FindFmtpLC() out, Fmtp: %S", &ptr );
#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE
    return fmtp;
    }

#endif // VIA_FEA_IPTV_USE_IPDC

    //  End of File
