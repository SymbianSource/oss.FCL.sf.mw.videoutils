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
* Description:    Hold rtp packets waiting to be sent*
*/




// INCLUDE FILES
#include "CCRPacketBuffer.h"
#include <e32cmn.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
// None.

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRPacketBuffer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCRPacketBuffer* CCRPacketBuffer::NewL( const TInt aMaxPackets )
    {
    CCRPacketBuffer* self = new( ELeave ) CCRPacketBuffer( aMaxPackets );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::CCRPacketBuffer
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCRPacketBuffer::CCRPacketBuffer( const TInt aMaxPackets )
  : iMaxPackets( aMaxPackets ),
    iContinousStream( EFalse ),
    iMoreComing( EFalse )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRPacketBuffer::ConstructL()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::~CCRPacketBuffer
// Destructor.
// -----------------------------------------------------------------------------
//
CCRPacketBuffer::~CCRPacketBuffer()
    {
    LOG( "CCRPacketBuffer::~CCRPacketBuffer" );
    
    iBuffer.ResetAndDestroy(); 
    iBookKeeping.Close();
    iSinkArray.Close();
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::AddSink
// 
// -----------------------------------------------------------------------------
//   
TInt CCRPacketBuffer::AddSink( CCRPacketSinkBase* aSink ) 
    {
    LOG( "CCRPacketBuffer::AddSink() in" );
    
    // Check if sink exist already?
    TBool exist( EFalse );
    for ( TInt i( iBookKeeping.Count() - 1 ); i >= 0 && !exist; i-- )
        {
        exist = ( aSink->Id() == iBookKeeping[i].iId );
        }
    
    // If not, add sink to list
    TInt err( KErrNone );
    if ( !exist )
        {
        SBookKeeping book;
        book.iId = aSink->Id();
        book.iIndex = KErrNotFound;
        err = iBookKeeping.Append( book );
        if ( !err )
            {
            err = iSinkArray.Append( aSink );
            if ( err )
                {
                // Remove last from book keeping, because sink append failed
                LOG1( "CCRPacketBuffer::AddSink(), Sink append error: %d", err );
                iBookKeeping.Remove( iBookKeeping.Count() - 1 );
                }
            }
        }

    LOG3( "CCRPacketBuffer::AddSink() out, err: %d, iSinkArray count: %d, iBookKeeping count: %d",
                                           err, iSinkArray.Count(), iBookKeeping.Count() );
    return err;
    }
    
// -----------------------------------------------------------------------------
// CCRPacketBuffer::AddPacket
// Method for add.
// -----------------------------------------------------------------------------
//  
void CCRPacketBuffer::AddPacket( 
    const MCRPacketSource::TCRPacketStreamId& aStream,
    const TDesC8& aHeader,
    const TDesC8& aPacket )
    {
    // Room
    VerifyRoom();

    // Add packet
    const TInt total( KStreamTypeBytesLength + 
                      aHeader.Length() + aPacket.Length() );
    HBufC8* packet = HBufC8::New( total );
    if ( packet )
        {
        TPtr8 ptr( packet->Des() );
        ptr.SetLength( KStreamTypeBytesLength );
        ptr[KStreamTypeBytePoint] = ( TUint8 )( aStream );
        ptr.Append( aHeader );
        ptr.Append( aPacket );
        if ( iBuffer.Insert( packet, 0 ) )
            {
            delete packet;
            }
        else
            {
            PacketToBookKeeping();
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::AddPacket
// Method for add.
// -----------------------------------------------------------------------------
//  
void CCRPacketBuffer::AddPacket(
    const MCRPacketSource::TCRPacketStreamId& aStream,
    const TDesC8& aPacket )
    {
    // Room
    VerifyRoom();

    // Add packet
    const TInt total( KStreamTypeBytesLength + aPacket.Length() );
    HBufC8* packet = HBufC8::New( total );
    if ( packet )
        {
        TPtr8 ptr( packet->Des() );
        ptr.SetLength( KStreamTypeBytesLength );
        ptr[KStreamTypeBytePoint] = ( TUint8 )( aStream );
        ptr.Append( aPacket );
        if ( iBuffer.Insert( packet, 0 ) )
            {
            delete packet;
            }
        else
            {
            PacketToBookKeeping();
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::ContinousStream
// Method for set buffer estimate when it ends.
// -----------------------------------------------------------------------------
//  
void CCRPacketBuffer::ContinousStream( const TBool aState )
    {
    LOG1( "CCRPacketBuffer::ContinousStream(), aState: %d", aState );
    iContinousStream = aState;
    }
    
// -----------------------------------------------------------------------------
// CCRPacketBuffer::ContinousStream
// Method for get the time when buffer should end.
// -----------------------------------------------------------------------------
//  
TBool CCRPacketBuffer::ContinousStream()
    {
    return iContinousStream;
    }
    
// -----------------------------------------------------------------------------
// CCRPacketBuffer:::MoreComing
// Method for set more coming state.
// -----------------------------------------------------------------------------
//  
void CCRPacketBuffer::MoreComing( const TBool aState )
    {
    iMoreComing = aState;
    }
    
// -----------------------------------------------------------------------------
// CCRPacketBuffer:::MoreComing
// Method for get more coming state.
// -----------------------------------------------------------------------------
//  
TBool CCRPacketBuffer::MoreComing()
    {
    return iMoreComing;
    }
    
// -----------------------------------------------------------------------------
// CCRPacketBuffer::GetStream
// -----------------------------------------------------------------------------
//
TInt CCRPacketBuffer::GetStream(
    const CCRStreamingSession::TCRSinkId& aId,
    MCRPacketSource::TCRPacketStreamId& aStreamId )
    {
    return GetStream( aId, 0, aStreamId );
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::GetStream
// -----------------------------------------------------------------------------
//
TInt CCRPacketBuffer::GetStream(
    const CCRStreamingSession::TCRSinkId& aId,
    const TInt aOffset,
    MCRPacketSource::TCRPacketStreamId& aStreamId )
    {
    const TInt bookKeeping( GetBookKeeping( aId ) );
    if ( bookKeeping > KErrNotFound  )
        {
        const TInt index( GetBufferIndex( bookKeeping ) - aOffset );
        if ( index > KErrNotFound && index < iBuffer.Count() )
            {
            aStreamId = MCRPacketSource::TCRPacketStreamId( 
                        iBuffer[index]->Des()[KStreamTypeBytePoint] );
            }
        }
    
    return bookKeeping;    
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::GetPacket
// Method for remove
// -----------------------------------------------------------------------------
//
void CCRPacketBuffer::GetPacket(
    const CCRStreamingSession::TCRSinkId& aId,
    TPtr8& aReturnedData )
    {
    const TInt bookKeeping( GetBookKeeping( aId ) );
    GetPacket( bookKeeping, aReturnedData );
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::GetPacket
// Method for remove
// -----------------------------------------------------------------------------
//
void CCRPacketBuffer::GetPacket( const TInt aBookKeeping, TPtr8& aReturnedData )
    {
    PeekPacket( aBookKeeping, aReturnedData, 0 );
    
    if ( aReturnedData.Ptr() || aReturnedData.Length() > 0 )
        {
        // One packet used
        iBookKeeping[aBookKeeping].iIndex--;        
        }
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::PeekPacket
// Method for peeking
// -----------------------------------------------------------------------------
//
void CCRPacketBuffer::PeekPacket( 
    const CCRStreamingSession::TCRSinkId& aId,
    TPtr8& aReturnedData, 
    const TInt aOffset )
    {
    const TInt bookKeeping( GetBookKeeping( aId ) );
    PeekPacket( bookKeeping, aReturnedData, aOffset );
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::PeekPacket
// Method for peeking
// -----------------------------------------------------------------------------
//
void CCRPacketBuffer::PeekPacket( 
    const TInt aBookKeeping, 
    TPtr8& aReturnedData, 
    const TInt aOffset )
    {
    aReturnedData.Set( NULL, 0, 0 ); 
    
    if ( aBookKeeping > KErrNotFound && aBookKeeping < iBookKeeping.Count() )
        {
        const TInt index( GetBufferIndex( aBookKeeping ) - aOffset );
        if ( index > KErrNotFound && index < iBuffer.Count() )
            {
            // Data
            aReturnedData.Set( 
                iBuffer[index]->Des().MidTPtr( KStreamTypeBytesLength ) );
            }
        }    
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::PacketsCount
// Method for asking count of packet available.
// -----------------------------------------------------------------------------
//   
TInt CCRPacketBuffer::PacketsCount( const CCRStreamingSession::TCRSinkId& aId )
    {
    const TInt bookKeeping( GetBookKeeping( aId ) );
    if ( bookKeeping > KErrNotFound )
        {
        return iBookKeeping[bookKeeping].iIndex;
        }
    
    return KErrNotFound;
    }
    
// -----------------------------------------------------------------------------
// CCRPacketBuffer::PacketsMinCount
// Method for asking minimum count of packet available in any sink.
// -----------------------------------------------------------------------------
//   
TInt CCRPacketBuffer::PacketsMinCount()
    {
    TInt ret( KMaxTInt );
    for ( TInt i( iBookKeeping.Count() - 1 ); i >= 0 ; i-- )
        {
        if ( iBookKeeping[i].iIndex < ret )
            {
            ret = iBookKeeping[i].iIndex;
            }
        }
    
    return ret;
    }
    
// -----------------------------------------------------------------------------
// CCRPacketBuffer::HandleBufferSize
// Removes packets which are used in all book keepings.
// -----------------------------------------------------------------------------
//   
void CCRPacketBuffer::HandleBufferSize()
    {
    // Find maximum index
    TInt max( KErrNotFound );
    for ( TInt i( 0 ); i < iBookKeeping.Count(); i++ )
        {
        max = Max( max, iBookKeeping[i].iIndex );
        }
    
    // Delete used packets
    const TInt oldCount( iBuffer.Count() );
    for ( TInt i( oldCount - 1 ); ( i > max ) && ( i > KErrNotFound ); i-- )
        {
        delete iBuffer[i];
        iBuffer.Remove( i );
        }
    
    // Compress if packets deleted
    if ( iBuffer.Count() < oldCount )
        {
        iBuffer.Compress();
        }
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::AdjustBuffer
// Drops packets to 25% of buffers max size. 
// -----------------------------------------------------------------------------
//
void CCRPacketBuffer::AdjustBuffer()
    {
    const TInt limit( iMaxPackets / 4 );
    for ( TInt i( iBookKeeping.Count() - 1 ); i >= 0 ; i-- )
        {
        if ( iBookKeeping[i].iIndex > limit )
        	{
            LOG3( "CCRPacketBuffer::AdjustBuffer(), book: %d, index: %d, limit: %d",
               i, iBookKeeping[i].iIndex, limit );
            iBookKeeping[i].iIndex = limit;
        	}
        }
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::ResetBuffer
// 
// -----------------------------------------------------------------------------
//   
void CCRPacketBuffer::ResetBuffer()
    {
    // Reset book keeping
    for ( TInt i( iBookKeeping.Count() - 1 ); i >= 0 ; i-- )
        {
        LOG2( "CCRPacketBuffer::ResetBuffer(), book: %d, index: %d",
            i, iBookKeeping[i].iIndex ); 
        iBookKeeping[i].iIndex = KErrNotFound;
        }
    
    // Notify sinks
    for ( TInt i( iSinkArray.Count() - 1 ); i >= 0; i-- )
        {
        iSinkArray[i]->BufferResetDone();
        }

    // Reset items
    iBuffer.ResetAndDestroy();
    }
    
// -----------------------------------------------------------------------------
// CCRPacketBuffer::RemoveSink
// 
// -----------------------------------------------------------------------------
//   
TInt CCRPacketBuffer::RemoveSink( CCRPacketSinkBase* aSink ) 
    {
    // Sink
    for ( TInt i( iSinkArray.Count() - 1 ); i >= 0; i-- )
        {
        if ( iSinkArray[i] == aSink ) 
            {
            iSinkArray.Remove( i );
            LOG1( "CCRPacketBuffer::RemoveSink(), removed Sink: %d", i );
            }   
        }

    // Book keeping
    for ( TInt i( iBookKeeping.Count() - 1 ); i >= 0; i-- )
        {
        if ( iBookKeeping[i].iId == aSink->Id() ) 
            {
            iBookKeeping.Remove( i ); 
            LOG1( "CCRPacketBuffer::RemoveSink(), removed Book keeping: %d", i );
            }   
        }
    
    return iSinkArray.Count();         
    }
    
// -----------------------------------------------------------------------------
// CCRPacketBuffer::VerifyRoom()
// Verify maximum packets in buffer. Will drop packets count to 1/4 if maximum
// size reached. 
// -----------------------------------------------------------------------------
//  
void CCRPacketBuffer::VerifyRoom()
    {
    TInt count( iBuffer.Count() );
    if ( count >= iMaxPackets )
        {
        LOG1( "CCRPacketBuffer::VerifyRoom(), Buffer full ! count: %d", count );
        AdjustBuffer();
        
        // Make sure memory not run out because of asyncronous packets deleting
        if ( count >= ( iMaxPackets * 2 ) )
            {
            ResetBuffer();
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRPacketBuffer::GetBookKeeping
// Updates book keeping index basing on sink id.
// -----------------------------------------------------------------------------
//
TInt CCRPacketBuffer::GetBookKeeping( const CCRStreamingSession::TCRSinkId& aId )
    {
    for ( TInt i( 0 ); i < iBookKeeping.Count(); i++ )
        {
        if ( iBookKeeping[i].iId == aId )
            {
            return i;
            }
        }
    
    return KErrNotFound;
    }
    
// -----------------------------------------------------------------------------
// CCRPacketBuffer::GetBufferIndex
// Getter for buffer index from book keeping.
// -----------------------------------------------------------------------------
//
TInt CCRPacketBuffer::GetBufferIndex( const TInt aBookKeeping )
    {
    if ( aBookKeeping > KErrNotFound && aBookKeeping < iBookKeeping.Count() )
        {
        return iBookKeeping[aBookKeeping].iIndex;
        }
    
    return KErrNotFound;
    }
    
// -----------------------------------------------------------------------------
// CCRPacketBuffer::PacketToBookKeeping
// 
// -----------------------------------------------------------------------------
//   
void CCRPacketBuffer::PacketToBookKeeping()
    {
    // New packet to book keeping
    for ( TInt i( 0 ); i < iBookKeeping.Count(); i++ )
        {
        iBookKeeping[i].iIndex++;
        }

    // New packet available
    for ( TInt i( 0 ); i < iSinkArray.Count(); i++ )
        {
        iSinkArray[i]->NewPacketAvailable();
        }
    }

//  End of File
