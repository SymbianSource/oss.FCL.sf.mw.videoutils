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
* Description:    Common interface for sinks*
*/




#ifndef CCRPACKETSINKBASE_H
#define CCRPACKETSINKBASE_H

// INCLUDES
#include <e32base.h>
#include "CCRStreamingSession.h"

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRPacketBuffer;

// CLASS DECLARATION

/**
*  Interface to sink.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRPacketSinkBase : public CBase
    {

public: // Constructors and destructors

    /**
    * Destructor
    */
    virtual ~CCRPacketSinkBase(); 

public: // New methods

    /**
    * Connects the given buffer with this sink.
    * @since Series 60 3.0
    * @param aBuffer is the buffer to connect with.
    * @param aBufId a id of the buffer.
    * @return none.
    */
    void SetBuffer( CCRPacketBuffer* aBuffer );
    
    /**
    * Method for setting the sdp in use.
    * @since Series 60 3.0
    * @param aSdp is the new sdp.
    * @return none.
    */
    virtual void SetSdpL( const TDesC8& aSdp ) = 0;
    
    /**
    * Adds packet to sink
    * @since Series 60 3.0
    * @return none.
    */
    virtual void NewPacketAvailable() = 0;
    
    /**
    * Buffer reset info for the sink.
    * @since Series 60 3.0
    * @return none.
    */
    virtual void BufferResetDone();
    
    /**
    * Method that source uses to communicate its status.
    * @since Series 60 3.0
    * @param aNewState is the new state of the source.
    * @return none.
    */
    virtual void StatusChanged( MCRPacketSource::TCRPacketSourceState aNewState );
    
    /**
    * Method that owner of sink may utilitze to distinguish between sinks.
    * @since Series 60 3.0
    * @param none.
    * @return Id that may have been set. 
    */
    virtual CCRStreamingSession::TCRSinkId Id( void ) const; 
    
    /**
    * Method for pause action for sink.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    virtual TInt Pause();

    /**
    * Method for restore action for sink.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    virtual TInt Restore();

    /**
    * Method for stopping action for sink.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    virtual void Stop();

    /**
    * method for setting initial seq+ts _before_ the stream is played.
    * @since Series 60 3.0
    * @param aAudioSeq is the initial rtp seq number for audio packets.
    * @param aAudioTS  is the initial rtp timestamp number for audio packets.
    * @param aVideoSeq is the initial rtp seq number for video packets.
    * @param aVideoTS  is the initial rtp timestamp number for video packets.
    * @return none.
    */
    virtual void SetSeqAndTS( TUint& aAudioSeq,
                              TUint& aAudioTS,
                              TUint& aVideoSeq,
                              TUint& aVideoTS );

    /**
    * method for setting play range before the stream is played.
    * @param aLower is where clip range begins, in seconds.
    * @param aUpper is where clip range ends, usually clip end, in seconds.
    * @return none.
    */
    virtual void SetRange( TReal aLower,
                           TReal aUpper );
                      
protected: // Constructors and destructors

    /**
    * Default constructor 
    */
    CCRPacketSinkBase( CCRStreamingSession& aSession,
                       CCRStreamingSession::TCRSinkId aSinkId ); 

protected: // Data
    
    /** 
    * Pointer to streaming session that owns us.
    */
    CCRStreamingSession& iOwningSession;                     
    
    /**
    * Buffer used by sinks.
    */
    CCRPacketBuffer* iBuffer;

    /**
    * Sink id, set by owner.
    */
    const CCRStreamingSession::TCRSinkId iSinkId; 
    
    };

#endif // CCRPACKETSINKBASE_H

//  End of File
