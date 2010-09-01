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




// INCLUDE FILES
#include "CCRNullSink.h"
#include "CCRPacketBuffer.h"
#include "CCRStreamingSession.h"
#include "CCRTimer.h"
#include "MCRConnectionObserver.h"
#include "videoserviceutilsLogger.h"

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRNullSink::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//  
CCRNullSink* CCRNullSink::NewL(
    CCRStreamingSession::TCRSinkId aSinkId,
    CCRStreamingSession& aOwningSession )
    {
    CCRNullSink* self = new( ELeave ) CCRNullSink( aSinkId, aOwningSession );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRNullSink::CCRNullSink
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//  
CCRNullSink::CCRNullSink(
    CCRStreamingSession::TCRSinkId aSinkId,
    CCRStreamingSession& aOwningSession )
  : CCRPacketSinkBase( aOwningSession, aSinkId )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRNullSink::ConstructL
// 2nd phase. 
// -----------------------------------------------------------------------------
//  
void CCRNullSink::ConstructL()
    {
    // None
    }
    
// -----------------------------------------------------------------------------
// CCRNullSink::~CCRNullSink
// Destructor.
// -----------------------------------------------------------------------------
//
CCRNullSink::~CCRNullSink()
    {    
    delete iSdp; 
    delete iFlowTimer;
    iBuffer = NULL; // does not delete and it is right thing.   
    }
    
// -----------------------------------------------------------------------------
// CCRNullSink::SetSdpL
// as a side-effect causes parsing of the sdp.
// -----------------------------------------------------------------------------
//  
void CCRNullSink::SetSdpL( const TDesC8& aSdp )
    {
    LOG1( "CCRNullSink::SetSdpL(), aSdp len: %d", aSdp.Length() );
    
    delete iSdp; iSdp = NULL; 
    iSdp = aSdp.AllocL();
    iOwningSession.PlayCommand( KRealZero, KRealMinusOne );
    }

// -----------------------------------------------------------------------------
// CCRNullSink::NewPacketAvailable
//
// -----------------------------------------------------------------------------
//      
void CCRNullSink::NewPacketAvailable()
    {
    if ( iBuffer )
        {
        TPtr8 packet( NULL, 0 );
        MCRPacketSource::TCRPacketStreamId streamId;
        const TInt book( iBuffer->GetStream( iSinkId, streamId ) );
        iBuffer->GetPacket( book, packet ); 
        const TUint8* pointer( &packet[2] );
        TInt seq( BigEndian::Get16( pointer ) );
        LOG2( "CCRNullSink::NewPacketAvailable(), streamId: %u, seq: %u", 
                                                  streamId, seq );
        // Keep buffer size reasonable
        iBuffer->HandleBufferSize();
        
        // Handle flow control
        if ( !iBuffer->ContinousStream() && !iFlowTimer )
            {
            TRAPD( err, iFlowTimer = CCRTimer::NewL( CActive::EPriorityLow, *this ) );
            if ( !err )
                {
                if ( iBuffer->PacketsCount( iSinkId ) > KErrNotFound )
                    {
                    iFlowTimer->After( 0 );
                    }
                else
                    {
                    iFlowTimer->At( 2e6 );
                    }
                }
            else
                {
                delete iFlowTimer; iFlowTimer = NULL;
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRNullSink::TimerExpired
// -----------------------------------------------------------------------------
//
void CCRNullSink::TimerExpired( CCRTimer* /*aTimer*/ )
    {
    if ( iBuffer->PacketsCount( iSinkId ) > KErrNotFound )
        {
        NewPacketAvailable();
        }
    else
        {
        iOwningSession.SourceRestore();
        }
    
    delete iFlowTimer; iFlowTimer = NULL;
    }

// -----------------------------------------------------------------------------
// CCRNullSink::RegisterConnectionObs
// -----------------------------------------------------------------------------
//
void CCRNullSink::RegisterConnectionObs( MCRConnectionObserver* aObserver )
    {
    iObserver = aObserver;
    }

// -----------------------------------------------------------------------------
// CCRNullSink::StatusChanged
// -----------------------------------------------------------------------------
//
void CCRNullSink::StatusChanged( MCRPacketSource::TCRPacketSourceState aNewState )
    {
    if( iObserver )
        {
        TCRTestSinkData data = aNewState==MCRPacketSource::ERtpStateIdle ? ECRTestSinkStateIdle :
                               aNewState==MCRPacketSource::ERtpStateSdpAvailable ? ECRTestSinkStateSdpAvailable :
                               aNewState==MCRPacketSource::ERtpStateSeqAndTSAvailable ? ECRTestSinkStateSeqAndTSAvailable :
                               aNewState==MCRPacketSource::ERtpStatePlaying ? ECRTestSinkStatePlaying :
                               /*aNewState==MCRPacketSource::ERtpStateClosing?*/ ECRTestSinkStateClosing;

        LOG3( "CCRNullSink::StatusChanged: newState=%d -> ECRTestSinkData, checksum=%d, data=%d",
              (TInt)aNewState, (TInt)iOwningSession.SourceChecksum(), (TInt)data );
        iObserver->ConnectionStatusChange( iOwningSession.SourceChecksum(),
                                           MCRConnectionObserver::ECRTestSinkData,
                                           (TInt)data );
        }
    }


//  End of File
