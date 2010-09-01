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
* Description:    Class that reads RTP packets from propriatary file format.*
*/




// INCLUDE FILES
#include "CCRRtpFileSource.h"
#include <ipvideo/CRtpClipHandler.h>
#include "CCRPacketBuffer.h"
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KGroupsCountPoint( 0 );
const TInt KBufferThesholdCount( 20 );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRRtpFileSource::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCRRtpFileSource* CCRRtpFileSource::NewL(
    const SCRRtpPlayParams& aParams,
    CRtpClipHandler*& aClipHandler,
    MCRStreamObserver& aSessionObs,
    CCRStreamingSession& aOwningSession )
    {
    CCRRtpFileSource* self = new( ELeave )
        CCRRtpFileSource( aClipHandler, aSessionObs, aOwningSession );
    CleanupStack::PushL( self );
    self->ConstructL( aParams );
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCRRtpFileSource* CCRRtpFileSource::NewL(
    const RFile& aRtpHandle,
    CRtpClipHandler*& aClipHandler,
    MCRStreamObserver& aSessionObs,
    CCRStreamingSession& aOwningSession )
    {
    CCRRtpFileSource* self = new( ELeave )
        CCRRtpFileSource( aClipHandler, aSessionObs, aOwningSession );
    CleanupStack::PushL( self );
    self->ConstructL( aRtpHandle );
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::CCRRtpFileSource
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCRRtpFileSource::CCRRtpFileSource(
    CRtpClipHandler*& aClipHandler,
    MCRStreamObserver& aSessionObs,
    CCRStreamingSession& aOwningSession )
  : CCRPacketSourceBase( aOwningSession, CCRStreamingSession::ECRRtpSourceId ),
    iClipHandler( aClipHandler ),
    iSessionObs( aSessionObs ),
    iInitialTime( KMaxTUint ),
    iClipPauseSent( 0 )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRRtpFileSource::ConstructL( const SCRRtpPlayParams& aParams )
    {
    const TBool timeShift( 
        aParams.iFileName.Find( KDvrTimeShiftFile ) > KErrNotFound );
    LOG1( "CCRRtpFileSource::ConstructL() in, timeShift: %d", timeShift );

    // RTP clip handler
    User::LeaveIfNull( iClipHandler );
    iClipHandler->RegisterReadObserver( this );
    iClipHandler->StartPlayBackL( aParams, timeShift );
    
    LOG( "CCRRtpFileSource::ConstructL() out" );
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRRtpFileSource::ConstructL( const RFile& aRtpHandle )
    {
    LOG( "CCRRtpFileSource::ConstructL() in" );

    User::LeaveIfNull( iClipHandler );
    iClipHandler->RegisterReadObserver( this );
    iClipHandler->StartPlayBackL( aRtpHandle );
    
    LOG( "CCRRtpFileSource::ConstructL() out" );
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::~CCRRtpFileSource
// Destructor.
// -----------------------------------------------------------------------------
//
CCRRtpFileSource::~CCRRtpFileSource()
    {
    LOG( "CCRRtpFileSource::~CCRRtpFileSource()" );

    if ( iClipHandler )
        {
        iClipHandler->StopPlayBack( KErrNone, 0 );
        }

    delete iSdp;
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::GetSdp
// -----------------------------------------------------------------------------
//
TInt CCRRtpFileSource::GetSdp( TPtrC8& aSdp )
    {
    if ( iSdp )
        {
        aSdp.Set( iSdp->Des() );
        return KErrNone;
        }

    return KErrNotReady;
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::SetBuffer
// -----------------------------------------------------------------------------
//
void CCRRtpFileSource::SetBuffer( CCRPacketBuffer* aBuffer )
    {
    iBuffer = aBuffer;
    iBuffer->ContinousStream( EFalse );
    iBuffer->MoreComing( EFalse );
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::PostActionL
// -----------------------------------------------------------------------------
//
void CCRRtpFileSource::PostActionL()
    {
    LOG( "CCRRtpFileSource::PostActionL(), SDP will be handled !" );

    // SDP
    if ( iClipHandler )
        {
        iSdp = iClipHandler->GetClipSdpL();
        }

    // Notify that SDP available
    iSessionObs.StatusChanged( MCRPacketSource::ERtpStateSdpAvailable );
    delete iSdp; iSdp = NULL;
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::Restore
// -----------------------------------------------------------------------------
//
void CCRRtpFileSource::Restore()
    {
    const TInt err( NextClipGroup( ETrue ) );
    if ( err && err != KErrEof )
        {
        LOG1( "CCRRtpFileSource::Restore(), NextClipGroup() err: %d", err );
        iSessionObs.StatusChanged( MCRPacketSource::ERtpStateClosing );
        }
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::Play
//
// -----------------------------------------------------------------------------
//      
TInt CCRRtpFileSource::Play( const TReal& aStartPos, const TReal& aEndPos )
    {
    LOG3( "CCRRtpFileSource::Play(), aStartPos: %f, aEndPos: %f, iClipPauseSent: %d", 
                                     aStartPos, aEndPos, iClipPauseSent );
    // Play for player?
    if ( aStartPos == KRealZero && aEndPos == KRealZero )
        {
        iInitialTime = KMaxTUint;
        return NextClipGroup( EFalse );
        }
    
    // Loading started in player?
    if ( aStartPos == KRealMinusOne && aEndPos == KRealMinusOne )
        {
        const TBool pauseSent( iClipPauseSent > 0 );
        if ( pauseSent )
            {
            iClipPauseSent--;
            iBuffer->ResetBuffer();
            }

        // Read more from clip
        TInt err( NextClipGroup( EFalse ) );
        if ( !err && pauseSent )
            {
            iInitialTime = KMaxTUint;
            err = ECRStreamPauseHanling;
            LOG( "CCRRtpFileSource::Play(), ECRStreamPauseHanling" );
            }
        else
            {
            if ( err == KErrEof )
                {
                err = KErrNone;
                if ( iClipHandler )
                    {
                    TRAP( err, iClipHandler->SetSeekPointL( 0 ) );
                    }
                if ( !err )
                    {
                    err = ECRStreamEndHandling;
                    LOG( "CCRRtpFileSource::Play(), ECRStreamEndHandling" );
                    }
                }
            }
        
        return err;
        }
    
    return KErrCompletion;
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::Stop
// -----------------------------------------------------------------------------
//
TInt CCRRtpFileSource::Stop()
    {
    LOG( "CCRRtpFileSource::Stop()" );

    return RtpPosition( 0 );
    }
    
// -----------------------------------------------------------------------------
// CCRRtpFileSource::SetPosition
//
// -----------------------------------------------------------------------------
//      
TInt CCRRtpFileSource::SetPosition( const TInt64 aPosition )
    {
    TInt err( RtpPosition( TUint( aPosition / KSiKilo ) ) );
    if ( !err )
        {
        err = NextClipGroup( EFalse );
        }
    
    return err;
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::GetPosition
//
// -----------------------------------------------------------------------------
//      
TInt CCRRtpFileSource::GetPosition( TInt64& aPosition, TInt64& aDuration )
    {
    if ( iBuffer && iClipHandler )
        {
        if ( iInitialTime != KMaxTUint )
            {
            aPosition += TInt64( iInitialTime ) * KSiKilo;
            }
        else
            {
            LOG( "CCRRtpFileSource::GetPosition(), iInitialTime not valid !" );
            }
        
        aDuration = TInt64( iClipHandler->GetCurrentLength() ) * KSiKilo;
#ifdef CR_ALL_LOGS
        LOG2( "CCRRtpFileSource::GetPosition(), aPosition: %u, aDuration: %u", 
            ( TUint )( aPosition / KSiKilo ), ( TUint )( aDuration / KSiKilo ) );
#endif // CR_ALL_LOGS
        return KErrNone;
        }

    return KErrCompletion;
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::GroupReadedL
// Adds packets to the buffer when finished asyncronous group reading.
// -----------------------------------------------------------------------------
//
void CCRRtpFileSource::GroupReadedL(
    const TDesC8& aGroup,
    const TUint aGroupTime,
    const TBool aLastGroup )
    {
    // Group time
    if ( iInitialTime == KMaxTUint )
        {
        iInitialTime = aGroupTime;
        }
    
    // Data valid?
    TInt point( KGroupsCountPoint + KPacketsCountBytes );
    const TInt total( aGroup.Length() );
    if ( point > total ) 
        {
        LOG( "CCRRtpFileSource::GroupReadedL(), No Packets Total Count !" );
        User::Leave( KErrCorrupt );
        }

    // Packets total count (PTC)
    const TInt totalCount( CRtpUtil::GetValueL(
                           aGroup.Mid( KGroupsCountPoint, KPacketsCountBytes ) ) );
    if ( totalCount > 0 )
        {
        iBuffer->MoreComing( ETrue );
        }
    
    // Loop all packets
    for ( TInt i( 0 ); i < totalCount; i++ )
        {
        // Corrupted?
        if ( ( point + KPacketSizeBytesLen ) > total )
            {
            LOG( "CCRRtpFileSource::GroupReadedL(), No Packets Size !" );
            User::Leave( KErrCorrupt );
            }

        // Packet total Size (PTS)
        TInt packetSize( CRtpUtil::GetValueL( 
                         aGroup.Mid( point, KPacketSizeBytesLen ) ) );
        // Corrupted?
        if ( packetSize <= 0 || ( point + packetSize ) > total )
            {
            LOG( "CCRRtpFileSource::GroupReadedL(), No Packets Payload !" );
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
        LOG3( "CCRRtpFileSource::GroupReadedL(), type: %d, packet: %d, seq: %d", 
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
                iBuffer->MoreComing( EFalse );
                if ( aLastGroup && stream != MCRPacketSource::EStreamEndTag )
                    {
                    LOG( "CCRRtpFileSource::GroupReadedL(), Misses last group from clip !" );
                    stream = MCRPacketSource::EStreamEndTag;
                    }
                }
            
            // Packet to buffer
            iBuffer->AddPacket( stream, packet );
            }
        
        point+= packetSize;
        }
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::ReadStatus
// -----------------------------------------------------------------------------
//
void CCRRtpFileSource::ReadStatus( TInt aStatus  )
    {
    LOG1( "CCRRtpFileSource::ReadStatus(), aStatus: %d", aStatus );

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
// CCRRtpFileSource::NextClipGroup
// -----------------------------------------------------------------------------
//
TInt CCRRtpFileSource::NextClipGroup( const TBool aForce )
    {
    if ( iBuffer && iClipHandler )
        {
        if ( aForce || iBuffer->PacketsMinCount() < KBufferThesholdCount )
            {
            TRAPD( err, iClipHandler->NextClipGroupL() );
            return err;
            }
        
        return KErrNone;
        }
    
    return KErrNotReady;
    }

// -----------------------------------------------------------------------------
// CCRRtpFileSource::RtpPosition
// -----------------------------------------------------------------------------
//
TInt CCRRtpFileSource::RtpPosition( const TUint aPosition )
    {
    LOG1( "CCRRtpFileSource::RtpPosition(), aPosition: %d", aPosition ); 

    TInt err( KErrCompletion );
    if ( iBuffer && iClipHandler )
        {
        TRAP( err, iClipHandler->SetSeekPointL( aPosition ) );
        if ( !err )
            {
            iInitialTime = KMaxTUint;
            iBuffer->ResetBuffer();
            }
        }
    
    return err;
    }
    
// -----------------------------------------------------------------------------
// CCRRtpFileSource::TypeToStream
// -----------------------------------------------------------------------------
//
TBool CCRRtpFileSource::TypeToStream(
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
            LOG( "CCRRtpFileSource::TypeToStream(), ERtpClipPause" );
            iClipPauseSent = ETrue;
            aStream = MCRPacketSource::EDisContinousStream;
            break;
        
        case MRtpFileWriteObserver::ERtpClipEnd:
            LOG( "CCRRtpFileSource::TypeToStream(), ERtpClipEnd" );
            aStream = MCRPacketSource::EStreamEndTag;
            break;

        default:
            LOG1( "CCRRtpFileSource::TypeToStream(), Default case, aType: %d",
                                                                   aType );
            return EFalse;
        }
    
    return ETrue;
    }
    
//  End of File
