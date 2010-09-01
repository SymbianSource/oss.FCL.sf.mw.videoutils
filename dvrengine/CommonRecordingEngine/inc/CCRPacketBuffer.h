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
* Description:    Ring buffer for storing packets from source.*
*/




#ifndef _CCRPACKETBUFFER_H
#define _CCRPACKETBUFFER_H

// INCLUDES
#include "MCRPacketSource.h"
#include "CCRPacketSinkBase.h"
#include <e32base.h>
#include <e32cmn.h>

// CONSTANTS
const TInt KStreamTypeBytePoint( 0 );
const TInt KStreamTypeBytesLength( 1 );

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRPacketSinkBase;

// CLASS DECLARATION

/**
*  Ring buffer where to store RTP packets.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRPacketBuffer : public CBase
    {

public: // Constructors and destructors

    /** 
    * Allocation method for buffer. 
    * @param aMaxPackets a maximum packets count in buffer.
    * @return buffer id.
    */
    static CCRPacketBuffer* NewL( const TInt aMaxPackets );
    
    /**
    * De-allocation method.
    */
    ~CCRPacketBuffer();

public: // New methods

    /**
    * Method for setting sink for buffer
    * @since Series 60 3.0
    * @param aSink is the sink to set
    * @return KErrNone if ok, otherwise system-wide error codes.
    */       
    TInt AddSink( CCRPacketSinkBase* aSink );

    /**
    * Method for adding a packet to buf. Succeeds always.
    * If memory allocation fails this silently fails.
    * @since Series 60 3.0
    * @param aStream a type of packet.
    * @param aHeader a pointer to packet header.
    * @param aPacket a pointer to packet payload.
    * @return none
    */
    void AddPacket( const MCRPacketSource::TCRPacketStreamId& aStream,
                    const TDesC8& aHeader,
                    const TDesC8& aPacket );
    
    /**
    * Method for adding a packet to buf.
    * If memory allocation fails this silently fails.
    * @since Series 60 3.0
    * @param aStream a type of packet.
    * @param aPacket is pointer to data that will be stored to buf.
    * @return none
    */
    void AddPacket( const MCRPacketSource::TCRPacketStreamId& aStream,
                    const TDesC8& aPacket );
    
    /**
    * Setter for buffer's flow control mode.
    * @since Series 60 3.0
    * @param aState a state to set.
    * @return none
    */
    void ContinousStream( const TBool aState );
    
    /**
    * Getter for buffer's flow control mode.
    * @since Series 60 3.0
    * @return true if continous stream,
              false if needs flow control.
    */
    TBool ContinousStream();
    
    /**
    * Setter for buffer's flow control state.
    * @since Series 60 3.0
    * @param aState a state to set.
    * @return none
    */
    void MoreComing( const TBool aState );

    /**
    * Getter for buffer's flow control state.
    * @since Series 60 3.0
    * @return more coming state.
    */
    TBool MoreComing();
    
    /**
    * Getter for stream of next packet.
    * @since Series 60 3.0
    * @param aId a id of requester.
    * @param a stream of the packet.
    * @return a book keeping index of sink.
    */
    TInt GetStream( const CCRStreamingSession::TCRSinkId& aId,
        		    MCRPacketSource::TCRPacketStreamId& aStreamId );

    /**
     * Getter for stream of packet in given offset.
     * @since Series 60 3.0
     * @param aId a id of requester.
     * @param a stream of the packet.
     * @return a book keeping index of sink.
     */
     TInt GetStream( const CCRStreamingSession::TCRSinkId& aId,
                     const TInt aOffset,
                     MCRPacketSource::TCRPacketStreamId& aStreamId );

    /**
    * Method for taking packet from a buffer.
    * @since Series 60 3.0
    * @param aId a id of requester.
    * @param aReturnedData is pointer where the data is read.
    *        NULL if nothing found.
    * @return none.
    */
    void GetPacket( const CCRStreamingSession::TCRSinkId& aId,
                    TPtr8& aReturnedData );
        
    /**
    * Method for taking packet from a buffer.
    * @since Series 60 3.0
    * @param aBookKeeping a book keeping index.
    * @param aReturnedData is pointer where the data is read.
    *        NULL if nothing found.
    * @return none.
    */
    void GetPacket( const TInt aBookKeeping,
                    TPtr8& aReturnedData );
    
    /**
    * Method for peeking at a packet in particular offset. This does not
    * set the packet to be removed from the buffer.
    * 
    * @since S60 v3.0
    * @param aId a id of requester.
    * @param aReturnedData is pointer where the data is read.
    *        NULL if nothing found.
    * @param aOffset offset of the packet to be peeked. If zero, the next
    *        packet in the buffer is peeked.
    */
    void PeekPacket( const CCRStreamingSession::TCRSinkId& aId,
                     TPtr8& aReturnedData,
                     const TInt aOffset );
    
    /**
    * Method for peeking at a packet in particular offset. This does not
    * set the packet to be removed from the buffer.
    * 
    * @since S60 v3.0
    * @param aBookKeeping a book keeping index.
    * @param aReturnedData is pointer where the data is read.
    *        NULL if nothing found.
    * @param aOffset offset of the packet to be peeked. If zero, the next
    *        packet in the buffer is peeked.
    */
    void PeekPacket( const TInt aBookKeeping,
                     TPtr8& aReturnedData, 
                     const TInt aOffset );
    
    /**
    * Method for asking how much packets there are in buf for certain sink.
    * @since Series 60 3.0
    * @param aId a id of requester.
    * @return number of packets in buffer.
    */       
    TInt PacketsCount( const CCRStreamingSession::TCRSinkId& aId ); 
    
    /**
    * Method for asking how much packets there are in buf in any sink.
    * @since Series 60 3.0
    * @param none.
    * @return minimum number of packets in buffer.
    */       
    TInt PacketsMinCount(); 
    
    /**
    * Method for removing a sink.
    * @since Series 60 3.0
    * @param aSink is the sink to delete
    * @return number of sinks remaining
    */       
    TInt RemoveSink( CCRPacketSinkBase* aSink );

    /**
    * Method for handle buffer wrapping.
    * @since Series 60 3.0
    * @return None.
    */       
    void HandleBufferSize();
    
    /**
    * Method for handle buffer size adjust.
    * @since Series 60 3.0
    * @return None.
    */       
    void AdjustBuffer();
     
    /**
    * Method for handle buffer resetting.
    * @since Series 60 3.0
    * @return None.
    */       
    void ResetBuffer();
    
private: // Constructors and destructor
    
    /**
    * Second pat of construction 
    */
    void ConstructL();
    
    /**
    * Default constructor is private.
    * @param aMaxPackets a maximum packets count in buffer.
    */
    CCRPacketBuffer( const TInt aMaxPackets );
    
private: // New methods

    /**
    * Method for handling maximum packets in buffer.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void VerifyRoom();
        
    /**
    * Method for taking book keeping index from sink id.
    * @since Series 60 3.0
    * @param aId a id of requester.
    * @return a book keeping index of sink.
    */
    TInt GetBookKeeping( const CCRStreamingSession::TCRSinkId& aId );
        
    /**
    * Method for taking book keeping index from sink id.
    * @since Series 60 3.0
    * @param aBookKeeping a book keeping index.
    * @return an buffer index base on sink.
    */
    TInt GetBufferIndex( const TInt aBookKeeping );
        
    /**
    * Method for update book keeping of the packets.
    * @since Series 60 3.0
    * @param aDataLength a data length of packet.
    * @return None.
    */       
    void PacketToBookKeeping();

private: // Data types

    // Book keeping array
    class SBookKeeping 
        {

    public: // Data

        /**
        * Sink id.
        */
        CCRStreamingSession::TCRSinkId iId;
        
        /**
        * Buffer index.
        */
        TInt iIndex;

        };

private: // Data
    
    /**
    * Maximum memory usage.
    */
    const TInt iMaxPackets;
    
    /**
    * Flow control mode.
    */
    TBool iContinousStream;
    
    /**
    * Flow control state.
    */
    TBool iMoreComing;
    
    /**
    * Buffer to keep packets.
    */
    RPointerArray<HBufC8> iBuffer;

    /**
    * Keeps track of point of each user.
    */
    RArray<SBookKeeping> iBookKeeping; 
    
    /**
    * Zero or more packet sinks that are fed via this buffer.
    */
    RPointerArray<CCRPacketSinkBase> iSinkArray;

    };

#endif // _CCRPACKETBUFFER_H
