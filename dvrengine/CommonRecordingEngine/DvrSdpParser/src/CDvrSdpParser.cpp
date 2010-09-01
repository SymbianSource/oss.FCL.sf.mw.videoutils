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
* Description:    Parses SDP file.*
*/




// INCLUDE FILES
#include <ipvideo/CDvrSdpParser.h>
#include <in_sock.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
_LIT8( KKeyIpv4Addr, "c=IN IP4 " );
_LIT8( KKeyIpv6Addr, "c=IN IP6 " );
_LIT8( KKeyAttribute, "a=" );
_LIT8( KKeyMedia, "m=" );
_LIT8( KKeyMediaAudio, "m=audio " );
_LIT8( KKeyMediaVideo, "m=video " );
_LIT8( KKeyMediaTitle, "m=title " );
_LIT8( KKeyDataStream, "m=data " );
_LIT8( KKeyClockRate, "a=rtpmap:" );
_LIT8( KKeyControl, "a=control:" ); 
//_LIT8( KKeyStreamId, "a=control:streamid=" );
_LIT8( KRealMediaIndicator, "/x-pn-real" ); 
_LIT8( KKeyBandWidth, "b=AS:" );
_LIT8( KCRSDPRtspUriBegin, "rtsp://" );
_LIT8( KSPStr, " ");
_LIT8( KCRStr, "\r");
_LIT8( KLFStr, "\n");
_LIT8( KCRLFStr, "\r\n");
_LIT8( KSLStr, "/" );
_LIT8( KSdpLiveStream, "a=LiveStream:integer;1");
_LIT8( KSdpRangeHeaderLiveStream, "a=range:npt=now-" );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CDvrSdpParser::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CDvrSdpParser* CDvrSdpParser::NewL()
    {
    CDvrSdpParser* self = CDvrSdpParser::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CDvrSdpParser* CDvrSdpParser::NewLC()
    {
    CDvrSdpParser* self = new( ELeave ) CDvrSdpParser();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::CDvrSdpParser
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CDvrSdpParser::CDvrSdpParser()
  : iAudioPort( KErrNotFound ),
    iVideoPort( KErrNotFound ),
    iTitlePort( KErrNotFound ),
    iAudioBitrate( KErrNotFound ),
    iVideoBitrate( KErrNotFound ),
    iAudioStreamId( KErrNotFound ),
    iVideoStreamId( KErrNotFound ),
    iVideoTimerGranularity( KMaxTUint32 ),
    iAudioTimerGranularity( KMaxTUint32 )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CDvrSdpParser::ConstructL()
    {
    LOG( "CDvrSdpParser::ConstructL()" );
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::~CDvrSdpParser
// Destructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CDvrSdpParser::~CDvrSdpParser()
    {
    LOG( "CDvrSdpParser::~CDvrSdpParser()" );
    delete iBaseUrl; iBaseUrl = NULL; 
    DeleteVariables();
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::TryParseL
// 
// -----------------------------------------------------------------------------
//
EXPORT_C void CDvrSdpParser::TryParseL( const TDesC8& aSdp, const TDesC8& aBaseUrl ) 
    {
#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    HBufC* baseLog = HBufC::NewLC( aBaseUrl.Length() );
    TPtr ptr = baseLog->Des();
    ptr.Copy( aBaseUrl );
    LOG2( "CDvrSdpParser::TryParseL(), aSdp length: %d, aBaseUrl: %S",
                                       aSdp.Length(), &ptr );
    CleanupStack::PopAndDestroy( baseLog );
#endif
    
    if ( aBaseUrl.Length() ) 
        {
        delete iBaseUrl; iBaseUrl = NULL; 
        iBaseUrl = aBaseUrl.AllocL(); 
        }

    TryParseL( aSdp ); 
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::TryParseL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CDvrSdpParser::TryParseL( const TDesC8& aSdp )
    {
    LOG1( "CDvrSdpParser::TryParseL(), aSdp length: %d", aSdp.Length() );
    
    // Find medias
    FindMediasL( aSdp );
    
    // Append found medias to the array
    for ( TInt i( 0 ); i < iMediaBuf.Count(); i++ )
        {
        TInt port( KErrNotFound );
        TPtrC8 ptr( iMediaBuf[i] );

        // Audio media info found ?
        port = GetIntL( ptr, KKeyMediaAudio );    
        if ( port != KErrNotFound )
            {
            iAudioPort = port;
            LOG1( "CDvrSdpParser::TryParseL(), iAudioPort: %d", iAudioPort );
            UpdateMediaInfoL( ptr, iAudioIpAddr );
            // Stream id (Helix takes medias in appearance order)
            iAudioStreamId = i;
            // Audio clock rate
            iAudioTimerGranularity = GetClockRateL( ptr );
            iAudioBitrate = GetIntL( ptr, KKeyBandWidth ); 
            // Audio attributes
            FindAttributesL( ptr, iAudioAttributes ); 
            // Audio control
            GetControlL( ptr, iAudioControlAddr );
            // Find media id 97 from a=rtpmap:97 MP4A-LATM/32000/2
            iMediaIdentifierAudio = GetIntL( ptr, KKeyClockRate ); 
            }
        else
            {
            // Video media info found ?
            port = GetIntL( ptr, KKeyMediaVideo );
            if ( port != KErrNotFound )
                {
                iVideoPort = port;
                LOG1( "CDvrSdpParser::TryParseL(), iVideoPort: %d", iVideoPort );
                UpdateMediaInfoL( ptr, iVideoIpAddr );
                // Stream id (Helix takes medias in appearance order)
                iVideoStreamId = i; 
                // Video clock rate
                iVideoTimerGranularity = GetClockRateL( ptr );
                iVideoBitrate = GetIntL( ptr, KKeyBandWidth ); 
                // Video attributes
                FindAttributesL( ptr, iVideoAttributes );
                // Video control
                GetControlL( ptr, iVideoControlAddr );
                // Find media id 96 from a=rtpmap:96 H264/90000 line
	            iMediaIdentifierVideo = GetIntL( ptr, KKeyClockRate );
                }
            else
                {
                port = GetIntL( ptr, KKeyMediaTitle );
                if ( port != KErrNotFound )
                    {
                    iTitlePort = port;
                    LOG1( "CDvrSdpParser::TryParseL(), iTitlePort: %d", iTitlePort );
                    }
                }
            }
        }

    iMediaBuf.Reset();
    delete iBaseUrl; iBaseUrl = NULL; 
    delete iCommonIp; iCommonIp = NULL;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::NewLineL
// 
// -----------------------------------------------------------------------------
//
EXPORT_C void CDvrSdpParser::NewLineL(
    const TInt aStreamId,
    const TDesC8& aLine ) 
    {
    User::LeaveIfNull( iSdp );
    const TInt newLen( iSdp->Length() + aLine.Length() + KCRLFStr().Length() );
    LOG2( "CDvrSdpParser::NewLineL(), aStreamId: %d, New sdp length: %d",
                                      aStreamId, newLen );
    switch( aStreamId )
        {
        case KErrNotFound: // Common
            {
            // Alloc more room
            iSdp = iSdp->ReAllocL( newLen );
            TPtr8 ptr( iSdp->Des() );
            
            // Find first attribute for new line point
            const TInt insertPoint( ptr.Find( KKeyAttribute ) );
            User::LeaveIfError( insertPoint );
            ptr.Insert( insertPoint, KCRLFStr );
            ptr.Insert( insertPoint, aLine );
            }
            break;
            
        case 0: // Audio (usually)
        case 1: // Video (usually)
            {
            iSdp = iSdp->ReAllocL( newLen );
            InserNewLineL( aStreamId, aLine );
            }
            break;
        
        default:
            User::Leave( KErrCompletion );
        }
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::GetSdp
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CDvrSdpParser::GetSdp( TPtrC8& aSdp ) 
    {
    if ( iSdp )
        {
        aSdp.Set( iSdp->Des() );
        return KErrNone;
        }
    
    return KErrNotFound;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::SupportedContent
// -----------------------------------------------------------------------------
//
EXPORT_C CDvrSdpParser::TDvrPacketProvidings CDvrSdpParser::SupportedContent( void )
    {
    TDvrPacketProvidings retval( EDvrNoProgramAtAll );

    if ( iAudioPort > KErrNotFound && iVideoPort > KErrNotFound )
        {
        retval = EDvrBothAudioAndVideo;
        }
    else if ( iAudioPort > KErrNotFound && iVideoPort <= KErrNotFound )
        {
        retval = EDvrAudioOnly;
        }
    else if ( iVideoPort > KErrNotFound && iAudioPort <= KErrNotFound )
        {
        retval = EDvrVideoOnly;
        }
    else
        {
        LOG( "CDvrSdpParser::SupportedContent(), No media !" );
        }
    
    return retval;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::SessionAttributes
// 
// -----------------------------------------------------------------------------
//
EXPORT_C RArray<TPtrC8>& CDvrSdpParser::SessionAttributes( void ) 
    {
    return iSessionAttributes; 
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::AudioAttributes
// 
// -----------------------------------------------------------------------------
//
EXPORT_C RArray<TPtrC8>& CDvrSdpParser::AudioAttributes( void ) 
    {
    return iAudioAttributes; 
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::VideoAttributes
// 
// -----------------------------------------------------------------------------
//
EXPORT_C RArray<TPtrC8>& CDvrSdpParser::VideoAttributes( void ) 
    {
    return iVideoAttributes; 
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::DataStreams
// 
// -----------------------------------------------------------------------------
//
EXPORT_C RArray<TPtrC8>& CDvrSdpParser::DataStreams( void ) 
    {
    return iDataStreams; 
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::IsIpv4Sdp
// Checks if Ipv4 address in use.
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CDvrSdpParser::IsIpv4Sdp( void ) 
    {
    return iIsIpv4;
    }   

// -----------------------------------------------------------------------------
// CDvrSdpParser::IsMultiCastSdp
// Checks if c= line did specify a multicast addr
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CDvrSdpParser::IsMultiCastSdp( void ) 
    {
    return iIsMulticast;
    }   

// -----------------------------------------------------------------------------
// CDvrSdpParser::AudioControlAddr
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC8 CDvrSdpParser::AudioControlAddr( void ) 
    {
    return ( iAudioControlAddr )? TPtrC8( *iAudioControlAddr ): 
                                  TPtrC8( KNullDesC8 );     
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::VideoControlAddr
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC8 CDvrSdpParser::VideoControlAddr( void ) 
    {
    return ( iVideoControlAddr )? TPtrC8( *iVideoControlAddr ):
                                  TPtrC8( KNullDesC8 );     
    }   

// -----------------------------------------------------------------------------
// CDvrSdpParser::AudioIpAddr
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC8 CDvrSdpParser::AudioIpAddr( void ) 
    {
    return ( iAudioIpAddr )? TPtrC8( *iAudioIpAddr ): TPtrC8( KNullDesC8 );     
    }   

// -----------------------------------------------------------------------------
// CDvrSdpParser::VideoIpAddr
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC8 CDvrSdpParser::VideoIpAddr( void ) 
    {
    return ( iVideoIpAddr )? TPtrC8( *iVideoIpAddr ): TPtrC8( KNullDesC8 );     
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::AudioPort
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CDvrSdpParser::AudioPort( void ) 
    {
    return iAudioPort;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::VideoPort
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CDvrSdpParser::VideoPort( void ) 
    {
    return iVideoPort;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::AudioBitrate
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CDvrSdpParser::AudioBitrate( void ) 
    {
    return iAudioBitrate;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::VideoBitrate
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CDvrSdpParser::VideoBitrate( void ) 
    {
    return iVideoBitrate;   
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::AudioStreamId
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CDvrSdpParser::AudioStreamId( void ) 
    {
    return iAudioStreamId;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::VideoStreamId
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CDvrSdpParser::VideoStreamId( void ) 
    {
    return iVideoStreamId;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::VideoTimerGranularity
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CDvrSdpParser::VideoTimerGranularity( void )  
    {
    return iVideoTimerGranularity; 
    }       

// -----------------------------------------------------------------------------
// CDvrSdpParser::AudioTimerGranularity
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CDvrSdpParser::AudioTimerGranularity( void ) 
    {
    return iAudioTimerGranularity; 
    }       

// -----------------------------------------------------------------------------
// CDvrSdpParser::IsRealMediaContent
// Check for realmedia content.
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CDvrSdpParser::IsRealMediaContent( void ) 
    {
    if ( iSdp && iSdp->Des().Find( KRealMediaIndicator ) != KErrNotFound )
        {
        LOG( "CDvrSdpParser::IsRealMediaContent(), Yes" );
        return ETrue;
        }   
        
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::MediaIdentifierAudio
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CDvrSdpParser::MediaIdentifierAudio( void )
	{
	return iMediaIdentifierAudio ;
	}
	
// -----------------------------------------------------------------------------
// CDvrSdpParser::MediaIdentifierVideo
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CDvrSdpParser::MediaIdentifierVideo( void )
	{
	return iMediaIdentifierVideo ;
	}

// -----------------------------------------------------------------------------
// CDvrSdpParser::IsLiveStream
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CDvrSdpParser::IsLiveStream( void ) 
    {
    TBool isLiveStream = EFalse;
    if ( iSdp &&
       ( iSdp->Des().Find( KSdpLiveStream ) != KErrNotFound || 
         iSdp->Des().Find( KSdpRangeHeaderLiveStream ) != KErrNotFound ) )
        {
        LOG( "CDvrSdpParser::IsLiveStream(), Yes" );
        isLiveStream = ETrue;
        }
        
    return isLiveStream;
    }
    
// -----------------------------------------------------------------------------
// CDvrSdpParser::FindMediasL
//
// -----------------------------------------------------------------------------
//
void CDvrSdpParser::FindMediasL( const TDesC8& aSdp ) 
    {
    LOG1( "CDvrSdpParser::FindMediasL(), aSdp length: %d", aSdp.Length() );

    // Find medias from SDP
    DeleteVariables();
    iSdp = aSdp.AllocL();
    RArray<SMediaPoint> points;
    CleanupClosePushL( points );
    FindMediaPointsL( points );
    MakeMediaBuffersL( points );
    FindSessionAttributesL( points );
    CleanupStack::PopAndDestroy( &points );
    FindDataStreamsL( iSdp->Des(), iDataStreams );
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::FindMediaPointsL
// Find points of all medias.
// -----------------------------------------------------------------------------
//
void CDvrSdpParser::FindMediaPointsL( RArray<SMediaPoint>& aPoints )
    {
    TInt lastPoint( 0 );
    User::LeaveIfNull( iSdp );
    TPtrC8 ptr( iSdp->Des() );
    TInt start( MediaSectionStart( ptr ) );
    
    // Loop all media sections
    while ( start > KErrNotFound && lastPoint < iSdp->Length() )
        {
        // Find whole media section, up to next media or EOF
        start += lastPoint;
        TInt len( MediaSectionStart( ptr.Mid( start + KKeyMedia().Length() ) ) );
        len = ( len > KErrNotFound )? len + KKeyMedia().Length() - 1:
                                      ptr.Length() - start - 1;
        // New media point
        SMediaPoint point;
        point.iStart = start;
        point.iLength = len;
        lastPoint = ( start + len );
        User::LeaveIfError( aPoints.Append( point ) );
        LOG3( "CDvrSdpParser::FindMediaPointsL(), start: %d, len: %d, lastPoint: %d",
                                                  start, len, lastPoint );
        // Next section
        start = MediaSectionStart( ptr.Mid( lastPoint ) );
        }
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::MediaSectionStart
//
// -----------------------------------------------------------------------------
//
TInt CDvrSdpParser::MediaSectionStart( const TDesC8& aPtr ) 
    {
    TInt start( aPtr.Find( KKeyMedia ) );
    while ( start > KErrNotFound )
        {
        // Verify that not data stream keyword? ( i.e not 'm=data' )
        const TInt keywordlen( KKeyDataStream().Length() );
        TPtrC8 ptr( aPtr.Mid( start, keywordlen ) );
        if ( ptr.Find( KKeyDataStream ) == KErrNotFound )
            {
            // Audio, video or subtitle
            return start;
            }
        
        start += keywordlen;
        const TInt next( aPtr.Mid( start ).Find( KKeyMedia ) );
        start = ( next > KErrNotFound )? start + next: KErrNotFound;
        }
    
    return start;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::MakeMediaBuffersL
// Make media buffers
// -----------------------------------------------------------------------------
//
void CDvrSdpParser::MakeMediaBuffersL( RArray<SMediaPoint>& aPoints )
    {
    User::LeaveIfNull( iSdp );
    for ( TInt i( 0 ); i < aPoints.Count(); i++ )
        {
        TPtrC8 media( iSdp->Des().Mid( aPoints[i].iStart, aPoints[i].iLength ) );
        User::LeaveIfError( iMediaBuf.Append( media ) );
        }
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::FindSessionAttributesL
// Find session attributes (common section before any media) 
// -----------------------------------------------------------------------------
//
void CDvrSdpParser::FindSessionAttributesL( RArray<SMediaPoint>& aPoints ) 
    {
    User::LeaveIfNull( iSdp );
    if ( aPoints.Count() )
        {
        TPtrC8 common( iSdp->Des().Left( aPoints[0].iStart ) );
        FindAttributesL( common, iSessionAttributes ); 
        
        // IP in common section
        delete iCommonIp; iCommonIp = NULL;
        iCommonIp = GetIpAddrL( common );
        }
    }
        
// -----------------------------------------------------------------------------
// CDvrSdpParser::InserNewLineL
//
// -----------------------------------------------------------------------------
//
void CDvrSdpParser::InserNewLineL(
    TInt aStreamId,
    const TDesC8& aLine )
    {
    RArray<SMediaPoint> points;
    CleanupClosePushL( points );
    FindMediaPointsL( points );

    // Add new line to first after media description
    if ( aStreamId >= 0 && aStreamId < points.Count() )
        {
        User::LeaveIfNull( iSdp );
        TPtr8 ptr( iSdp->Des() );
        TInt insertPoint( GetLen( ptr.Mid( points[aStreamId].iStart,
                                           points[aStreamId].iLength ), ETrue ) );
        User::LeaveIfError( insertPoint );
        ptr.Insert( insertPoint, aLine );
        ptr.Insert( insertPoint, KCRLFStr );
        }
    else
        {
        User::Leave( KErrCompletion );
        }

    CleanupStack::PopAndDestroy( &points );
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::UpdateMediaInfoL
//
// -----------------------------------------------------------------------------
//
void CDvrSdpParser::UpdateMediaInfoL(
    const TDesC8& aMediaPtr,
    HBufC8*& aAddress )
    {
    // Search for IP address (Ipv6/Ipv4)
    aAddress = GetIpAddrL( aMediaPtr );
    if ( !aAddress && iCommonIp )
        {
        aAddress = iCommonIp->AllocL();
        }
    User::LeaveIfNull( aAddress );
    TPtr8 ptr( aAddress->Des() );

    // IP Address
    if ( iIsIpv4 )
        {
        LOG( "CDvrSdpParser::UpdateMediaInfoL(), Removing v4 subnet mask" );
        // Remove possible subnet mask (e.g. c=IN IP4 225.0.1.15/64)
        const TInt maskPos( aAddress->Find( KSLStr ) ); 
        if ( maskPos != KErrNotFound )
            {
            ptr.SetLength( maskPos );
            }
        }

#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    TName buf; buf.Copy( ptr );
    LOG1( "CDvrSdpParser::UpdateMediaInfoL(), aAddress: %S", &buf );
#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE

    /* Stream Id
    const TInt streamId( GetIntL( aMediaPtr, KKeyStreamId ) );
    LOG1( "CDvrSdpParser::UpdateMediaInfoL(), Stream Id: %d", streamId );
    return streamId;
    */
    }
    
// -----------------------------------------------------------------------------
// CDvrSdpParser::GetIpAddrL
//
// -----------------------------------------------------------------------------
//
HBufC8* CDvrSdpParser::GetIpAddrL( const TDesC8& aPtr ) 
    {
    iIsIpv4 = EFalse;
    HBufC8* ipAddr = GetStringL( aPtr, KKeyIpv6Addr );
    if ( !ipAddr )
        {
        ipAddr = GetStringL( aPtr, KKeyIpv4Addr );
        if ( ipAddr )
            {
            iIsIpv4 = ETrue;
            }
        }
        
    // Verify multicast
    if ( ipAddr )
        {
        TPtrC8 addr( ipAddr->Des() );
        if ( addr.Length() <= KMaxName )
            {
            CheckForMulticast( addr ); 
            }
        else
            {
            LOG1( "Ipaddress length too long: %d, leaving....", addr.Length() );
            delete ipAddr; ipAddr = NULL;
            User::Leave( KErrOverflow );
            }   
        }
    
    return ipAddr;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::GetClockRateL
//
// -----------------------------------------------------------------------------
//
TUint CDvrSdpParser::GetClockRateL( const TDesC8& aPtr )
    {
    // Find clock rate keyword
    TInt start( FindStart( aPtr, KKeyClockRate ) );
    User::LeaveIfError( start );

    // Len up to first slash ( i.e: rtpmap:97 MP4A-LATM/32000/2 )
    TInt slash( aPtr.Mid( start ).Find( KSLStr ) ); 
    User::LeaveIfError( slash );
    start += ( slash + KSLStr().Length() );
    TInt len( GetLen( aPtr, start ) );
    
    // Len up to second slash ( i.e: rtpmap:97 MP4A-LATM/32000/2 )
    slash = aPtr.Mid( start, len ).Find( KSLStr );
    len = ( slash != KErrNotFound )? slash: len;
    return StrToUint( aPtr.Mid( start, len ) );
    }
    
// -----------------------------------------------------------------------------
// CDvrSdpParser::GetControlL() 
// Checks if a=control line specifies a control for media.
// -----------------------------------------------------------------------------
//
void CDvrSdpParser::GetControlL( const TDesC8& aMediaPtr, HBufC8*& aControlAddr )
    {
    delete aControlAddr; aControlAddr = NULL;
    HBufC8* control = GetStringL( aMediaPtr, KKeyControl ); 
    if ( control ) 
        {
        CleanupStack::PushL( control ); 
        if ( control->Des().FindC( KCRSDPRtspUriBegin ) == KErrNotFound )
            { 
            // relative url
            if ( iBaseUrl ) 
                {
                aControlAddr = HBufC8::NewL( iBaseUrl->Des().Length() + 
                                             control->Des().Length() + 
                                             KSLStr().Length() );
                aControlAddr->Des().Append ( iBaseUrl->Des() ); 
                aControlAddr->Des().Append ( KSLStr ) ; 
                aControlAddr->Des().Append ( control->Des() );
                }
            }
        else
            { 
            // absolute url
            aControlAddr = control->Des().AllocL();                     
            }
        
        CleanupStack::PopAndDestroy( control ); 
        }
    }
                
// -----------------------------------------------------------------------------
// CDvrSdpParser::CheckForMulticast() 
// Checks if c= line specifies a multicast addr
// -----------------------------------------------------------------------------
//
void CDvrSdpParser::CheckForMulticast( const TDesC8& aLine )
    {
    TInetAddr controladdr;
    TName addr( KNullDesC );
    addr.Copy( aLine ); 
    TInt err( controladdr.Input( addr ) );
    if ( err != KErrNone )
        {
        LOG1( "CDvrSdpParser: invalid control address in SDP connection line '%S'", &addr );
        }
    else
        {
        // just do check for multicast, actual address is taken from SETUP response
        iIsMulticast = controladdr.IsMulticast();

#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
        /* Commented out, for some reason can crash with RTP playback in debug.
        if ( iIsMulticast )
            { 
            LOG1( "CDvrSdpParser: detected MULTICAST (%S) control address in SDP", &addr );
            }
        else
            {
            LOG1( "CDvrSdpParser: detected unicast (%S) control address in SDP", &addr );
            }
        */
#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE
        }
    }   

// -----------------------------------------------------------------------------
// CDvrSdpParser::FindAttributesL
// 
// -----------------------------------------------------------------------------
//  
void CDvrSdpParser::FindAttributesL(
    const TDesC8& aSdpSection,
    RArray<TPtrC8>& aAttributeList ) 
    {
    TInt last( 0 );
    TInt start( KErrNotFound );
    do
        {
        // Rest of the SDP section
        TPtrC8 rest( aSdpSection.Mid( last ) );
        start = rest.Find( KKeyAttribute );
        if ( start > KErrNotFound )
            {
            last += start;
            TInt len( GetLen( rest, start, ETrue ) );
            if ( len > 0 )
                { 
                // Add other than control attribute
                if ( rest.Mid( start, len ).Find( KKeyControl ) == KErrNotFound )
                    {
                    TPtrC8 ptr( rest.Mid( start, len ) );
                    User::LeaveIfError( aAttributeList.Append( ptr ) ); 
                    }
                
                last += len;
                }
            }
        }
        while( start > KErrNotFound );
    }                           
                               
// -----------------------------------------------------------------------------
// CDvrSdpParser::FindDataStreamsL
// 
// -----------------------------------------------------------------------------
//  
void CDvrSdpParser::FindDataStreamsL(
    const TDesC8& aSdpSection,
    RArray<TPtrC8>& aStreamsList )
    {
    TInt last( 0 );
    TInt start( KErrNotFound );
    do
        {
        // Rest of the SDP block
        TPtrC8 rest( aSdpSection.Mid( last ) );
        start = rest.Find( KKeyDataStream );
        if ( start > KErrNotFound )
            {
            last += start;
            TInt len( GetLen( rest, start, ETrue ) );
            if ( len > 0 )
                { 
                TPtrC8 ptr( rest.Mid( start, len ) );
                User::LeaveIfError( aStreamsList.Append( ptr ) ); 
                last += len;
                }
            }
        }
        while( start > KErrNotFound );
    }                           
                               
// -----------------------------------------------------------------------------
// CDvrSdpParser::GetIntL
//
// -----------------------------------------------------------------------------
//
TInt CDvrSdpParser::GetIntL( const TDesC8& aPtr, const TDesC8& aKeyword ) 
    {
    TInt ret( KErrNotFound );
    HBufC8* buf = GetStringL( aPtr, aKeyword );
    if ( buf )
        {
        ret = StrToUint( buf->Des() );
        delete buf;
        }
   
    return ret;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::GetStringL
//
// -----------------------------------------------------------------------------
//
HBufC8* CDvrSdpParser::GetStringL( const TDesC8& aPtr, const TDesC8& aKeyword ) 
    {
    const TInt start( FindStart( aPtr, aKeyword ) );
    const TInt len( GetLen( aPtr, start ) );

    HBufC8* buf = NULL;
    if ( start > KErrNotFound && len > 0 && ( start + len ) <= aPtr.Length() )
        {
        buf = aPtr.Mid( start, len ).AllocL();
        }
    
    return buf;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::FindStart
//
// -----------------------------------------------------------------------------
//
TInt CDvrSdpParser::FindStart( const TDesC8& aPtr, const TDesC8& aKeyword ) 
    {
    TInt start( aPtr.Find( aKeyword ) ); 
    
    if ( start > KErrNotFound )
        {
        start += aKeyword.Length();
        }
    
    return start;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::GetLen
//
// -----------------------------------------------------------------------------
//
TInt CDvrSdpParser::GetLen(
    const TDesC8& aPtr,
    const TInt aStart,
    const TBool aIgnoreSpace )
    {
    if ( aStart > KErrNotFound && aStart < aPtr.Length() )
        {
        // Find next LF, CR or CRLF combination
        TInt len1( MinNonError( aPtr.Mid( aStart ).Find( KLFStr ),
                                aPtr.Mid( aStart ).Find( KCRStr ) ) );
        // Find space
        TInt len2( ( aIgnoreSpace )? KErrNotFound: 
                                     aPtr.Mid( aStart ).Find( KSPStr ) );

        if ( len1 == KErrNotFound && len2 == KErrNotFound )
            {
            // Rest of the buffer
            return ( aPtr.Length() - aStart );
            }
        else
            {
            // CRLF or space
            return MinNonError( len1, len2 );
            }
        }
    
    return KErrNotFound;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::MinNonError
//
// -----------------------------------------------------------------------------
//
TInt CDvrSdpParser::MinNonError( const TInt aValue1, const TInt aValue2 )
    {
    if ( aValue1 > KErrNotFound && aValue2 > KErrNotFound )
        {
        return Min( aValue1, aValue2 );
        }

    return ( ( aValue1 > KErrNotFound )? aValue1: 
             ( aValue2 > KErrNotFound )? aValue2: KErrNotFound );
    }
    
// -----------------------------------------------------------------------------
// CDvrSdpParser::StrToUint
// Convert string to integer.
// -----------------------------------------------------------------------------
//
TUint CDvrSdpParser::StrToUint( const TDesC8& aString ) 
    {
    TLex8 templex;
    templex.Assign( aString );
    TUint ret( KMaxTUint );
    templex.Val( ret );
    return ret;
    }

// -----------------------------------------------------------------------------
// CDvrSdpParser::DeleteVariables
// 
// -----------------------------------------------------------------------------
//
void CDvrSdpParser::DeleteVariables( void )
    {
    delete iSdp; iSdp = NULL;
    delete iCommonIp; iCommonIp = NULL;
    iMediaBuf.Reset();
    iSessionAttributes.Reset();
    iAudioAttributes.Reset();
    iVideoAttributes.Reset();
    iDataStreams.Reset();
    delete iAudioControlAddr; iAudioControlAddr = NULL;
    delete iVideoControlAddr; iVideoControlAddr = NULL;
    delete iAudioIpAddr; iAudioIpAddr = NULL;
    delete iVideoIpAddr; iVideoIpAddr = NULL;
    }

//  End of File
