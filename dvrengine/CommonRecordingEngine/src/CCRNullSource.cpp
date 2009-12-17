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
* Description:    Class that reads packets from a .rtp clip for testing purposes.*
*/




// INCLUDE FILES
#include "CCRNullSource.h"
#include <ipvideo/CRtpClipHandler.h>
#include "CCRPacketBuffer.h"
#include "CRtpTimer.h"
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KGroupsCountPoint( 0 );
const TInt KBufferThesholdCount( 20 );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRNullSource::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCRNullSource* CCRNullSource::NewL(
    const TDesC& aClipName,
    MCRStreamObserver& aSessionObs,
    CCRStreamingSession& aOwningSession )
    {
    CCRNullSource* self = new( ELeave )
        CCRNullSource( aSessionObs, aOwningSession );
    CleanupStack::PushL( self );
    self->ConstructL( aClipName );
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CCRNullSource::CCRNullSource
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCRNullSource::CCRNullSource(
    MCRStreamObserver& aSessionObs,
    CCRStreamingSession& aOwningSession )
  : CCRPacketSourceBase( aOwningSession, CCRStreamingSession::ECRNullSourceId ),
    iSessionObs( aSessionObs ),
    iGroupTime( KMaxTUint )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRNullSource::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRNullSource::ConstructL( const TDesC& aClipName )
    {
    LOG1( "CCRNullSource::ConstructL() in, aClipName: %S", &aClipName );

    iClipHandler = CRtpClipHandler::NewL();
    iClipHandler->RegisterReadObserver( this );

    // Start playback
    SCRRtpPlayParams params;
    params.iFileName = aClipName;
    iClipHandler->StartPlayBackL( params );
    
    LOG( "CCRNullSource::ConstructL() out" );
    }

// -----------------------------------------------------------------------------
// CCRNullSource::~CCRNullSource
// Destructor.
// -----------------------------------------------------------------------------
//
CCRNullSource::~CCRNullSource()
    {
    LOG( "CCRNullSource::~CCRNullSource()" );

    if ( iClipHandler )
        {
        iClipHandler->StopPlayBack( KErrNone, 0 );
        }
    
    delete iClipHandler; iClipHandler = NULL;
    delete iFlowTimer;
    delete iSdp;
    }

// -----------------------------------------------------------------------------
// CCRNullSource::GetSdp
// -----------------------------------------------------------------------------
//
TInt CCRNullSource::GetSdp( TPtrC8& aSdp )
    {
    if ( iSdp )
        {
        aSdp.Set( iSdp->Des() );
        return KErrNone;
        }

    return KErrNotReady;
    }

// -----------------------------------------------------------------------------
// CCRNullSource::SetBuffer
// -----------------------------------------------------------------------------
//
void CCRNullSource::SetBuffer( CCRPacketBuffer* aBuffer )
    {
    iBuffer = aBuffer;
    iBuffer->ContinousStream( EFalse );
    iBuffer->MoreComing( EFalse );
    }

// -----------------------------------------------------------------------------
// CCRNullSource::PostActionL
// -----------------------------------------------------------------------------
//
void CCRNullSource::PostActionL()
    {
    LOG( "CCRNullSource::PostActionL(), SDP will be handled !" );

    // SDP
    iSdp = iClipHandler->GetClipSdpL();

    // Notify that SDP available
    iSessionObs.StatusChanged( MCRPacketSource::ERtpStateSdpAvailable );
    delete iSdp; iSdp = NULL;
    }

// -----------------------------------------------------------------------------
// CCRNullSource::Restore
// -----------------------------------------------------------------------------
//
void CCRNullSource::Restore()
    {
    delete iFlowTimer; iFlowTimer = NULL;
    const TInt err( NextClipGroup() );
    if ( err )
        {
        LOG1( "CCRNullSource::Restore(), NextClipGroup() err: %d", err );
        iSessionObs.StatusChanged( MCRPacketSource::ERtpStateClosing );
        }
    }

// -----------------------------------------------------------------------------
// CCRNullSource::Play
//
// -----------------------------------------------------------------------------
//      
TInt CCRNullSource::Play( const TReal& aStartPos, const TReal& aEndPos )
    {
    LOG2( "CCRNullSource::Play(), aStartPos: %f, aEndPos: %f", 
                                  aStartPos, aEndPos );

    if ( aStartPos == KRealZero && aEndPos == KRealZero )
        {
        Restore();
        }
    
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CCRNullSource::Stop
// -----------------------------------------------------------------------------
//
TInt CCRNullSource::Stop()
    {
    iClipHandler->StopPlayBack( KErrNone, 0 );
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CCRNullSource::GetPosition
//
// -----------------------------------------------------------------------------
//      
TInt CCRNullSource::GetPosition( TInt64& aPosition, TInt64& aDuration )
    {
    if ( iBuffer )
        {
        if ( iGroupTime != KMaxTUint )
            {
            aPosition += TInt64( iGroupTime ) * KSiKilo;
            }
        
        aDuration = TInt64( iClipHandler->GetCurrentLength() ) * KSiKilo;
#ifdef CR_ALL_LOGS
        LOG2( "CCRNullSource::GetPosition(), aPosition: %u, aDuration: %u", 
               ( TUint )( aPosition / KSiKilo ), ( TUint )( aDuration / KSiKilo ) );
#endif // CR_ALL_LOGS
        return KErrNone;
        }

    return KErrCompletion;
    }

// -----------------------------------------------------------------------------
// CCRNullSource::GroupReadedL
// Adds packets to the buffer when finished asyncronous group reading.
// -----------------------------------------------------------------------------
//
void CCRNullSource::GroupReadedL(
    const TDesC8& aGroup,
    const TUint aGroupTime,
    const TBool aLastGroup )
    {
    // Group time
    if ( iGroupTime == KMaxTUint )
        {
        iGroupTime = aGroupTime;
        }
    
    // Data valid?
    TInt point( KGroupsCountPoint + KPacketsCountBytes );
    const TInt total( aGroup.Length() );
    if ( point > total ) 
        {
        LOG( "CCRNullSource::GroupReadedL(), No Packets Total Count !" );
        User::Leave( KErrCorrupt );
        }

    // Packets total count (PTC)
    const TInt totalCount( CRtpUtil::GetValueL(
                           aGroup.Mid( KGroupsCountPoint, KPacketsCountBytes ) ) );
    if ( totalCount > 0 )
        {
        iBuffer->ContinousStream( ETrue );
        }
    
    // Loop all packets
    for ( TInt i( 0 ); i < totalCount; i++ )
        {
        // Corrupted?
        if ( ( point + KPacketSizeBytesLen ) > total )
            {
            LOG( "CCRNullSource::GroupReadedL(), No Packets Size !" );
            User::Leave( KErrCorrupt );
            }

        // Packet total Size (PTS)
        TInt packetSize( CRtpUtil::GetValueL( 
                         aGroup.Mid( point, KPacketSizeBytesLen ) ) );
        // Corrupted?
        if ( packetSize <= 0 || ( point + packetSize ) > total )
            {
            LOG( "CCRNullSource::GroupReadedL(), No Packets Payload !" );
            User::Leave( KErrCorrupt );
            }
        
        // Packet type
        point += KPacketSizeBytesLen;
        const MRtpFileWriteObserver::TRtpType type( 
            ( MRtpFileWriteObserver::TRtpType )( aGroup[point] ) );
        point += KPacketTypeBytesLen;
        packetSize -= ( KPacketSizeBytesLen + KPacketTypeBytesLen );

        // Insert packet to the buffer
        const TPtrC8 packet( aGroup.Mid( point, packetSize ) );

#ifdef CR_ALL_LOGS
        const TUint8* pointer( &packet[2] );
        TInt seq( BigEndian::Get16( pointer ) );
        LOG3( "CCRNullSource::GroupReadedL(), type: %d, packet: %d, seq: %d", 
                                              type, packet.Length(), seq );
        //RFileLogger::WriteFormat( _L( "livetv" ), _L( "play.log" ), EFileLoggingModeAppend, 
        //    _L( "GroupReadedL(), type: %d, packet: %d, seq: %d" ), type, packet.Length(), seq );
#endif // CR_ALL_LOGS

        MCRPacketSource::TCRPacketStreamId stream( MCRPacketSource::EStreamIdCount );
        if ( TypeToStream( type, stream ) )
            {
            // Last packet in group?
            if ( i >= ( totalCount - 1 ) )
                {
                iBuffer->ContinousStream( EFalse );
                if ( aLastGroup && stream != MCRPacketSource::EStreamEndTag )
                    {
                    LOG( "CCRNullSource::GroupReadedL(), Misses last group from clip !" );
                    stream = MCRPacketSource::EStreamEndTag;
                    }
                }
            
            // Packet to buffer
            iBuffer->AddPacket( stream, packet );
            }
        
        point+= packetSize;
        }
    
    if ( !iFlowTimer )
        {
        iFlowTimer = CRtpTimer::NewL( *this );
        iFlowTimer->After( KNormalRecGroupLength * KSiKilo );
        }
    }

// -----------------------------------------------------------------------------
// CCRNullSource::ReadStatus
// -----------------------------------------------------------------------------
//
void CCRNullSource::ReadStatus( TInt aStatus  )
    {
    LOG1( "CCRNullSource::ReadStatus(), aStatus: %d", aStatus );

    switch ( aStatus )
        {
        case MRtpFileReadObserver::ERtpTimeShifTEnd:
            break;
        
        default:
            iSessionObs.StatusChanged( MCRPacketSource::ERtpStateClosing );
            break;
        }
    }

// -----------------------------------------------------------------------------
// CCRNullSource::TimerEventL
// Internal timer call this when triggered.
// -----------------------------------------------------------------------------
//
void CCRNullSource::TimerEventL()
    {
    User::LeaveIfError( NextClipGroup() );
    delete iFlowTimer; iFlowTimer = NULL;
    }

// -----------------------------------------------------------------------------
// CCRNullSource::TimerError
// Internal timer call this when TimerEventL() leaves.
// -----------------------------------------------------------------------------
//
void CCRNullSource::TimerError( const TInt aError )
    {
    LOG1( "CCRNullSource::TimerError(), TimerEventL() leaved: %d", aError );
    ( void )aError; // Prevent compiler warning

    delete iFlowTimer; iFlowTimer = NULL;
    iSessionObs.StatusChanged( MCRPacketSource::ERtpStateClosing );
    }

// -----------------------------------------------------------------------------
// CCRNullSource::NextClipGroup
// -----------------------------------------------------------------------------
//
TInt CCRNullSource::NextClipGroup()
    {
    if ( iBuffer && iClipHandler )
        {
        if ( iBuffer->PacketsMinCount() < KBufferThesholdCount )
            {
            TRAPD( err, iClipHandler->NextClipGroupL() );
            return err;
            }
        
        return KErrNone;
        }
    
    return KErrNotReady;
    }

// -----------------------------------------------------------------------------
// CCRNullSource::TypeToStream
// -----------------------------------------------------------------------------
//
TBool CCRNullSource::TypeToStream(
    const MRtpFileWriteObserver::TRtpType& aType,
    MCRPacketSource::TCRPacketStreamId& aStream )
    {
    switch ( aType )
        {
        case MRtpFileWriteObserver::ERtpAudio:
            aStream = MCRPacketSource::EAudioStream;
            break;

        case MRtpFileWriteObserver::ERtcpAudio:
            aStream = MCRPacketSource::EAudioControlStream;
            break;

        case MRtpFileWriteObserver::ERtpVideo:
            aStream = MCRPacketSource::EVideoStream;
            break;

        case MRtpFileWriteObserver::ERtcpVideo:
            aStream = MCRPacketSource::EVideoControlStream;
            break;

        case MRtpFileWriteObserver::ERtpSubTitle:
            aStream = MCRPacketSource::ESubTitleStream;
            break;

        case MRtpFileWriteObserver::ERtcpSubTitle:
            aStream = MCRPacketSource::ESubTitleControlStream;
            break;

        case MRtpFileWriteObserver::ERtpClipPause:
            LOG( "CCRNullSource::TypeToStream(), ERtpClipPause" );
            aStream = MCRPacketSource::EDisContinousStream;
            break;
        
        case MRtpFileWriteObserver::ERtpClipEnd:
            LOG( "CCRNullSource::TypeToStream(), ERtpClipEnd" );
            aStream = MCRPacketSource::EStreamEndTag;
            break;

        default:
            LOG1( "CCRNullSource::TypeToStream(), Default case, aType: %d",
                                                                aType );
            return EFalse;
        }
    
    return ETrue;
    }
    
//  End of File
