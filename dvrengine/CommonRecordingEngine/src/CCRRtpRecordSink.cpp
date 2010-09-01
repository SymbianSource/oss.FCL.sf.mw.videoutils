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
* Description:    Class that takes packet from buffer and does not put them*
*/




// INCLUDES
#include "CCRRtpRecordSink.h"
#include "CCRPacketBuffer.h"
#include "CCRStreamingSession.h"
#include "MCRConnectionObserver.h"
#include <ipvideo/CRtpClipHandler.h>
#include <ipvideo/CRtpClipManager.h>
#include <ipvideo/CDvrSdpParser.h>
#include "CRtpTsConverter.h"
#include "CRtpPacket.h"
#include <bsp.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KDefaultBitRate( 256 + 64 ); // 320 kbps
const TInt KDefGroupSize( 70 * 1024 );  // 70k
const TInt KMaxGroupSize( 140 * 1024 ); // 140k
const TInt KMaxGrouplength( 3000 );		// 3 s
const TInt KGroupHeaderSize( KGroupHeaderBytes + KPacketsCountBytes );
const TInt KGroupLenghtAccuracy( 20 );  // 20ms

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//  

CCRRtpRecordSink* CCRRtpRecordSink::NewL(
    const SCRRecordParams& aRecordParams,
    CCRStreamingSession::TCRSinkId aSinkId,
    CCRStreamingSession& aOwningSession,
    MCRConnectionObserver* aObserver,
    CRtpClipHandler*& aClipHandler )
    {
    CCRRtpRecordSink* self = new( ELeave ) 
    CCRRtpRecordSink( aSinkId, aOwningSession, aObserver, aClipHandler );
    CleanupStack::PushL( self );
    self->ConstructL( aRecordParams );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::CCRRtpRecordSink
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//  
CCRRtpRecordSink::CCRRtpRecordSink(
    CCRStreamingSession::TCRSinkId aSinkId,
    CCRStreamingSession& aOwningSession,
    MCRConnectionObserver* aObserver,
    CRtpClipHandler*& aClipHandler )
  : CCRPacketSinkBase( aOwningSession, aSinkId ),
    iObserver( aObserver ),
    iClipHandler( aClipHandler ),
    iGroupPointer( NULL, 0 ),
    iGroupSize( KGroupHeaderSize ),
    iPacketsCount( 0 ),
    iWantedGroup( KMaxTInt ),
    iOldestTs( KMaxTUint ),
    iLatestAudio( NULL, 0 ),
    iSaveMode( MRtpFileWriteObserver::ESaveNormal ),
    iGroupMode( MRtpFileWriteObserver::ESaveIdle )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::ConstructL
// 2nd phase. 
// -----------------------------------------------------------------------------
//  
void CCRRtpRecordSink::ConstructL( const SCRRecordParams& aRecordParams )
    {
    LOG( "CCRRtpRecordSink::ConstructL()" );
    
    // Params
    iRecParams.iClipPath = aRecordParams.iFileName;
    iRecParams.iSdpData.Set( aRecordParams.iSdpData );
    iRecParams.iService.Set( aRecordParams.iServiceName );
    iRecParams.iProgram.Set( aRecordParams.iProgramName );
    iRecParams.iPostRule = aRecordParams.iPostRule;
    iRecParams.iParental = aRecordParams.iParental;
    iRecParams.iEndTime = aRecordParams.iEndTime;
    
    if ( aRecordParams.iFormat == ECRRecordTimeShift )
        {
        iRecParams.iStartTime = 0;
        iRecParams.iEndTime = KDvrMaximumTimeShift * 1e6;
        iSaveMode = MRtpFileWriteObserver::ESaveTimeShift;
        }
    
#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    LOG1( "CCRRtpRecordSink::ConstructL(), iClipPath: %S", &iRecParams.iClipPath );
    TName buf( KNullDesC ); iRecParams.iStartTime.FormatL( buf, KTimeDateFormat );
    LOG1( "CCRRtpRecordSink::ConstructL(), iStartTime: %S", &buf );
    iRecParams.iEndTime.FormatL( buf, KTimeDateFormat );
    LOG1( "CCRRtpRecordSink::ConstructL(), iEndTime: %S", &buf );
#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE

    // Clip handler and group buffer
    User::LeaveIfNull( iClipHandler );
    iGroupBuffer = HBufC8::NewL( 0 );
    iGroupPointer.Set( iGroupBuffer->Des() );
    }
    
// -----------------------------------------------------------------------------
// CCRRtpRecordSink::~CCRRtpRecordSink
// Destructor.
// -----------------------------------------------------------------------------
//
CCRRtpRecordSink::~CCRRtpRecordSink()
    {    
    LOG( "CCRRtpRecordSink::~CCRRtpRecordSink()" );

    if ( iClipHandler )
        {
        iClipHandler->StopRecording( KErrCancel );
        }
    
    delete iGroupBuffer;
    delete iAudioConv;
    }
    
// -----------------------------------------------------------------------------
// CCRRtpRecordSink::SetSdpL
// Sets SDP, parses it and initiates XPS.
// -----------------------------------------------------------------------------
//  
void CCRRtpRecordSink::SetSdpL( const TDesC8& aSdp )
    {
    TInt initiated( iRecParams.iSdpData.Length() );
    LOG2( "CCRRtpRecordSink::SetSdpL(), aSdp len: %d, initiated: %d",
                                        aSdp.Length(), initiated );
    if ( !initiated && iClipHandler )
        {
        iRecParams.iSdpData.Set( aSdp );
        iClipHandler->RegisterWriteObserver( this );
        iClipHandler->StartRecordingL( iRecParams, iSaveMode );

        // SDP parser
        CDvrSdpParser* sdpParser = CDvrSdpParser::NewLC();
        sdpParser->TryParseL( aSdp );
        
        // Bit rates
        TUint total( sdpParser->VideoBitrate() + sdpParser->AudioBitrate() );
        TReal angle( TReal( total ) / KDefaultBitRate );
        iWantedGroup = TInt( angle * KDefGroupSize );
        LOG1( "SetSdpL::SetSdpL(), iWantedGroup: %d", iWantedGroup );
        iGroupBuffer = iGroupBuffer->ReAllocL( iWantedGroup + KGroupHeaderSize );
        iGroupPointer.Set( iGroupBuffer->Des() );
        
        // TS converter
        delete iAudioConv; iAudioConv = NULL;
        iAudioConv = CRtpTsConverter::NewL( sdpParser->AudioTimerGranularity() );
        LOG1( "CCRRtpRecordSink::SetSdpL(), AudioTimerGranularity: %d",
                                            sdpParser->AudioTimerGranularity() );
        CleanupStack::PopAndDestroy( sdpParser );
        
        // Recording can start
        iGroupMode = MRtpFileWriteObserver::ESaveNormal;
        iObserver->ConnectionStatusChange( iOwningSession.SourceChecksum(),
            MCRConnectionObserver::ECRRecordingStarted, KErrNone );
        }
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::NewPacketAvailable
// From CCRPacketSinkBase. New packet(s) to a group.
// -----------------------------------------------------------------------------
//      
void CCRRtpRecordSink::NewPacketAvailable()
    {
    // Keep group buffer untouch during clip writing
    if ( iBuffer && iClipHandler && !iClipHandler->WritingActive() )
        {
        if ( iGroupMode == MRtpFileWriteObserver::ESaveNormal )
            {
            // New packets to a group
            AddToGroup();

            // Group size big enougth to write to clip?
            if ( iGroupSize >= iWantedGroup )
                {
                SaveGroup( iGroupMode );
                }

            // Keep buffer size reasonable
            iBuffer->HandleBufferSize();
            }
        else
            {
            if ( iGroupMode != MRtpFileWriteObserver::ESaveIdle )
                {
                AddToGroup();
                
                // Handle user pause
                if ( iGroupMode == MRtpFileWriteObserver::ESavePause )
                    {
                    AddPausePacket();
                    }

                SaveGroup( iGroupMode );
                iGroupMode = MRtpFileWriteObserver::ESaveIdle;
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::BufferResetting
// From CCRPacketSinkBase.
// -----------------------------------------------------------------------------
//      
void CCRRtpRecordSink::BufferResetDone()
    {
    AddPausePacket();
    if ( iClipHandler && !iClipHandler->WritingActive() )
        {
        SaveGroup( MRtpFileWriteObserver::ESavePause );
        }
    }
    
// -----------------------------------------------------------------------------
// CCRRtpRecordSink::Pause
// -----------------------------------------------------------------------------
//
TInt CCRRtpRecordSink::Pause()
    {
    LOG1( "CCRRtpRecordSink::Pause(), iGroupMode: %d", iGroupMode );
    
    TInt err( KErrCompletion );
    if ( iClipHandler )
        {
        if ( iSaveMode == MRtpFileWriteObserver::ESaveNormal )
            {
            // Normal pause
            err = KErrNone;
            iGroupMode = MRtpFileWriteObserver::ESavePause;
            }
        else
            {
            // Time shift pause
            TRAP( err, iClipHandler->TimeShiftPauseL() );
            }
        }
    
    return err;
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::Restore
// -----------------------------------------------------------------------------
//
TInt CCRRtpRecordSink::Restore()
    {
    LOG1( "CCRRtpRecordSink::Restore(), iGroupMode: %d", iGroupMode );
    
    iGroupMode = MRtpFileWriteObserver::ESaveNormal;
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::Stop
// -----------------------------------------------------------------------------
//
void CCRRtpRecordSink::Stop()
    {
    LOG1( "CCRRtpRecordSink::Stop(), iGroupMode: %d", iGroupMode );

    iGroupMode = MRtpFileWriteObserver::ESaveEnd;
    if ( iClipHandler && !iClipHandler->WritingActive() )
        {
        iWantedGroup = KMaxTInt;
        SaveGroup( iGroupMode );
        }
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::GroupSaved
// From MRtpFileWriteObserver.
// -----------------------------------------------------------------------------
//
void CCRRtpRecordSink::GroupSaved()
    {
    ResetGroupVariables();
    if ( iGroupMode != MRtpFileWriteObserver::ESaveNormal )
        {
        SaveGroup( iGroupMode );
        iGroupMode = MRtpFileWriteObserver::ESaveIdle;
        }
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::WriteStatus
// From MRtpFileWriteObserver.
// -----------------------------------------------------------------------------
//
void CCRRtpRecordSink::WriteStatus( const TInt aStatus )
    {
    LOG1( "CCRRtpRecordSink::WriteStatus(), aStatus: %d", aStatus );

    ForceStopRecording( aStatus );
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::AddToGroup
// Initialises time stamp converter for audio stream and adds packets to a group.
// -----------------------------------------------------------------------------
//
void CCRRtpRecordSink::AddToGroup()
    {
    const TInt packets( iBuffer->PacketsCount( iSinkId ) );
    for ( TInt i( packets ); i > KErrNotFound; i-- )
        {
        // Packet
        TPtr8 packet( NULL, 0 );
        MCRPacketSource::TCRPacketStreamId streamId(
            MCRPacketSource::EStreamIdCount );
        const TInt book( iBuffer->GetStream( iSinkId, streamId ) );
        iBuffer->GetPacket( book, packet ); 
        
        // TS converter
        if ( streamId == MCRPacketSource::EAudioControlStream &&
             iAudioConv && !iAudioConv->Initiated() )
            {
            iAudioConv->Init( packet );
            }
        
        // Type valid
        MRtpFileWriteObserver::TRtpType type( MRtpFileWriteObserver::ERtpNone );
        if ( packet.Length() && StreamToType( streamId, type ) )
            {
            TRAPD( err, AddPacketToGroupL( packet, type ) );
            if ( err )
                {
                LOG1( "CCRRtpRecordSink::AddToGroup(), AddPacketToGroupL leaved: %d", err );
                ForceStopRecording( err );
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::AddPacketToGroupL
// -----------------------------------------------------------------------------
//
void CCRRtpRecordSink::AddPacketToGroupL(
    const TDesC8& aPacket,
    const MRtpFileWriteObserver::TRtpType& aType )
    {
    const TUint total( KPacketSizeBytesLen + 
                       KPacketTypeBytesLen + aPacket.Length() );
    iGroupSize += total;
    if ( iGroupSize > iGroupPointer.MaxLength() )
        {
        iGroupBuffer = iGroupBuffer->ReAllocL( iGroupSize );
        iGroupPointer.Set( iGroupBuffer->Des() );
        LOG1( "CCRRtpRecordSink::AddPacketToGroupL(), New iGroupSize: %d", iGroupSize );
        }
    
    // Packet length (PTL), type and data
    TBuf8<KPacketSizeBytesLen + KPacketTypeBytesLen> header;
    CRtpUtil::MakeBytesL( total, header );
    header.Append( KCharSpace );
    header[KPacketTypeBytePoint] = ( TUint8 )( aType );
    iGroupPointer.Append( header );
    iGroupPointer.Append( aPacket );
    iPacketsCount++;

#ifdef CR_ALL_LOGS
    const TUint8* pointer( &aPacket[2] );
    TInt seq( BigEndian::Get16( pointer ) );
    LOG3( "CCRRtpRecordSink::AddPacketToGroupL(), type: %d, packet: %d, seq: %d", 
                                                  aType, aPacket.Length(), seq );
    //RFileLogger::WriteFormat( _L( "livetv" ), _L( "record.log" ), EFileLoggingModeAppend, 
    //    _L( "AddPacketToGroupL(), type: %d, packet: %d, seq: %d" ), aType, aPacket.Length(), seq );
    
#endif // CR_ALL_LOGS
    
    // Variables for TS delta
    if ( aType == MRtpFileWriteObserver::ERtpAudio && 
         iAudioConv && iAudioConv->Initiated() )
        {
        if ( iOldestTs == KMaxTUint )
            {
            iOldestTs = TsFromPacketL( aPacket );
            }
        else
            {
            iLatestAudio.Set( iGroupPointer.Right( aPacket.Length() ) );
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::SaveGroup
// Saves RTP packets group to a clip.
// -----------------------------------------------------------------------------
//
void CCRRtpRecordSink::SaveGroup( MRtpFileWriteObserver::TRtpSaveAction aAction )
    {
    TRAPD( err, SaveGroupL( aAction ) );
    if ( err )
    	{
        ForceStopRecording( err );
    	}
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::SaveGroup
// Saves RTP packets group to a clip.
// -----------------------------------------------------------------------------
//
void CCRRtpRecordSink::SaveGroupL( MRtpFileWriteObserver::TRtpSaveAction aAction )
    {
	// TS delta
    TBool forceSave( aAction != MRtpFileWriteObserver::ESaveNormal );
    TInt length( TReal( iGroupSize ) / iWantedGroup * KNormalRecGroupLength );
    if ( iOldestTs != KMaxTUint )
        {
        length = TsFromPacketL( iLatestAudio ) - iOldestTs;
        }
    if ( length >= ( KNormalRecGroupLength - KGroupLenghtAccuracy ) )
        {
        forceSave = ETrue;
        if ( length <= ( KNormalRecGroupLength + KGroupLenghtAccuracy ) )
            {
            iWantedGroup = ( iWantedGroup + iGroupSize ) / 2;
            }
        else
            {
            TReal angle( TReal( iGroupSize ) / length );
            TInt wanted(  TReal( KNormalRecGroupLength ) * angle );
            if ( wanted > ( KDefGroupSize / 2 ) && wanted < KMaxGroupSize )
                {
                iWantedGroup = ( iWantedGroup + wanted ) / 2;
                }
            }
        }

	// Group ok to save?
    if ( forceSave || iGroupSize > KMaxGroupSize )
        {
        // Group packets count (PTC)
        HBufC8* bytes = CRtpUtil::MakeBytesLC( iPacketsCount );
        iGroupPointer.Insert( 0, bytes->Des() );
        CleanupStack::PopAndDestroy( bytes );

        // Make sure that nasty length not end to the clip in case TS overflow
        length = ( length <= KMaxGrouplength )? length: KMaxGrouplength;

        // Save to clip
        TInt err( KErrNotReady );
        if ( iClipHandler )
            {
            TRAP( err, iClipHandler->SaveNextGroupL( iGroupPointer, 
                                                     length, aAction ) );
            }
        if ( err )
            {
            LOG1( "CCRRtpRecordSink::SaveGroup(), SaveNextGroupL Leaved: %d", err );
            ForceStopRecording( err );
            }
        
        LOG3( "CCRRtpRecordSink::SaveGroup(), iPacketsCount: %d, length: %u, iWantedGroup: %d", 
                                              iPacketsCount, length, iWantedGroup );
        }
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::StreamToType
// -----------------------------------------------------------------------------
//
TBool CCRRtpRecordSink::StreamToType(
    const MCRPacketSource::TCRPacketStreamId& aStream,
    MRtpFileWriteObserver::TRtpType& aType )
    {
    switch ( aStream )
        {
        case MCRPacketSource::EAudioStream:
            aType = MRtpFileWriteObserver::ERtpAudio;
            break;

        case MCRPacketSource::EAudioControlStream:
            aType = MRtpFileWriteObserver::ERtcpAudio;
            break;

        case MCRPacketSource::EVideoStream:
            aType = MRtpFileWriteObserver::ERtpVideo;
            break;

        case MCRPacketSource::EVideoControlStream:
            aType = MRtpFileWriteObserver::ERtcpVideo;
            break;

        case MCRPacketSource::ESubTitleStream:
            aType = MRtpFileWriteObserver::ERtpSubTitle;
            break;

        case MCRPacketSource::ESubTitleControlStream:
            aType = MRtpFileWriteObserver::ERtcpSubTitle;
            break;

        case MCRPacketSource::EDisContinousStream:
            LOG( "CCRRtpRecordSink::StreamToType(), ERtpClipPause" );
            aType = MRtpFileWriteObserver::ERtpClipPause;
            break;
        
        case MCRPacketSource::EStreamEndTag:
            LOG( "CCRRtpRecordSink::StreamToType(), ERtpClipEnd" );
            aType = MRtpFileWriteObserver::ERtpClipEnd;
            break;

        default:
            return EFalse;
        }
    
    return ETrue;
    }
    
// -----------------------------------------------------------------------------
// CCRRtpRecordSink::TsFromPacketL
// -----------------------------------------------------------------------------
//
TUint CCRRtpRecordSink::TsFromPacketL( const TDesC8& aPacket )
    {
    CRtpPacket* rtpPacket = CRtpPacket::NewLC();
    TUint ts( KMaxTUint );
    if ( !rtpPacket->ParseRtp( aPacket ) )
        {
        ts = iAudioConv->ConvertTs( rtpPacket->iRtpRecvHeader.iTimestamp, ETrue );
        }
    
    CleanupStack::PopAndDestroy( rtpPacket );
    return ts;
    }
    
// -----------------------------------------------------------------------------
// CCRRtpRecordSink::AddPausePacket
// Wrapper for AddPausePacketL().
// -----------------------------------------------------------------------------
//
void CCRRtpRecordSink::AddPausePacket()
    {
    LOG( "CCRRtpRecordSink::AddPausePacket()");

    TRAPD( err, AddPausePacketL() );
    if ( err )
        {
        ForceStopRecording( err );
        }
    }
    
// -----------------------------------------------------------------------------
// CCRRtpRecordSink::AddPausePacketL
// Adds pause packet to the group.
// -----------------------------------------------------------------------------
//
void CCRRtpRecordSink::AddPausePacketL()
    {
    HBufC8* data = CRtpUtil::MakeBytesLC( KMaxTUint );
    AddPacketToGroupL( data->Des(), MRtpFileWriteObserver::ERtpClipPause );
    CleanupStack::PopAndDestroy( data );
    }
    
// -----------------------------------------------------------------------------
// CCRRtpRecordSink::ForceStopRecording
// Stops recording on clip handler and destroys the sink.
// -----------------------------------------------------------------------------
//
void CCRRtpRecordSink::ForceStopRecording( const TInt& aStatus )
    {
    LOG2( "CCRRtpRecordSink::ForceStopRecording(), iGroupMode: %d, aStatus: %d",
                                                   iGroupMode, aStatus );
    iGroupMode = MRtpFileWriteObserver::ESaveIdle;

    if ( iClipHandler )
        {
        iClipHandler->StopRecording( aStatus );
        }
    
    iObserver->ConnectionStatusChange( iOwningSession.SourceChecksum(),
        MCRConnectionObserver::ECRRecordingEnded, aStatus );
    iOwningSession.SinkStops( Id() );
    }

// -----------------------------------------------------------------------------
// CCRRtpRecordSink::ResetGroupVariables
// 
// -----------------------------------------------------------------------------
//
void CCRRtpRecordSink::ResetGroupVariables()
    {
    iGroupSize = KGroupHeaderSize; // Room for group header and packets count
    iPacketsCount = 0;
    iGroupPointer.Zero();
    iOldestTs = KMaxTUint;
    iLatestAudio.Set( NULL, 0 );
    }

//  End of File
