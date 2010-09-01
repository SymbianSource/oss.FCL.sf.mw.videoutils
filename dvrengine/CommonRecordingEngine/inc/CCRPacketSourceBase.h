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
* Description:    Common interface for sources*
*/



#ifndef MCRPACKETSOURCEBASE_H
#define MCRPACKETSOURCEBASE_H

// INCLUDES
#include "CCRStreamingSession.h"
#include "MCRPacketSource.h"
#include "MCRConnectionObserver.h"

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION
// None

/**
* Interface to source.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRPacketSourceBase : public CBase,
                            public MCRPacketSource
    {

public: // Constructors and destructors

    /**
    * Destructor
    */
    virtual ~CCRPacketSourceBase(); 

public: // Methods from base classes

	/**
    * From MCRPacketSource.
    * Method for setting buffers to packet source.
	* @since Series 60 3.0
	* @param aBuffer is buffer to be set.
	* @return none.
	*/
	virtual void SetBuffer( CCRPacketBuffer* aBuffer );

    /**
    * From MCRPacketSource.
    * Method for acquiring sdp.
    * @since Series 60 3.0
    * @param aSdp is string pointer that will be .Set() to contain the sdp.
    *        If no sdp is available no .Set() will occur.
    * @return KErrNotReady if no sdp available.
    */
    virtual TInt GetSdp( TPtrC8& aSdp ) = 0;

    /**
    * From MCRPacketSource.
    * Method for acquiring (almost) up-to-date sequence and ts numbers.
    * @since Series 60 3.0
    * @param aAudioSeq is reference to TUint that will be set by this
    *        method to contain latest available sequence number for
    *        audio stream being received via this packet source.
    * @param aAudioTS rtp timestamp for audio.
    * @param aVideoSeq rtp seq for video. If no video, value will not be touched.
    * @param aVideoTS rtp ts for video.
    * @return KErrNone if data available.
    */
    virtual TInt SeqAndTS( TUint& aAudioSeq,
                           TUint& aAudioTS,
                           TUint& aVideoSeq,
                           TUint& aVideoTS );

    /**
    * From MCRPacketSource.
    * Method for ordering "play" for packet source.
    * @since Series 60 3.0
    * @param aStartPos is start position in seconds.
    * @param aEndPos is end position in seconds.
    * @return a system wide error code.
    */
    virtual TInt Play( const TReal& aStartPos,
                       const TReal& aEndPos );

    /**
    * From MCRPacketSource.
    * Method for pausing play from source.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    virtual TInt Pause();

    /**
    * From MCRPacketSource.
    * Method for stopping play from source.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    virtual TInt Stop();

    /**
    * From MCRPacketSource.
    * Setter for current position.
    * @since Series 60 3.0
    * @param aPosition a postion of playback.
    * @return a system wide error code.
    */
    virtual TInt SetPosition( const TInt64 aPosition );
    
    /**
    * From MCRPacketSource.
    * Getter for current position.
    * @since Series 60 3.0
    * @param aPosition a postion of playback.
    * @param aDuration a duration of playback.
    * @return a system wide error code.
    */
    virtual TInt GetPosition( TInt64& aPosition,
                              TInt64& aDuration );
    
    /**
    * From MCRPacketSource.
    * Method for getting range of stream. If no range/duration
    * is available this method will set the return values to
    * 0.0,-1.0 and that may be quite normal state live streams.
    * @since Series 60 3.0
    * @param aLower is where to start from.
             If no value available, value of aLower must be set to 0.
    * @param aUpper is where to stop. Negative values mean eternity.
    * @return none.
    */
    virtual void GetRange( TReal& aLower,
                           TReal& aUpper );

    /**
    * From MCRPacketSource.
    * Post action after source initialized.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    virtual void PostActionL();

    /**
    * From MCRPacketSource.
    * Method for requesting more packets to buffer.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    virtual void Restore();

public: // New methods

    /**
    * Method that owner of source may utilitze to distinguish
    * between sources
    * @since Series 60 3.0
    * @param none.
    * @return Id that may have been set. 
    */
    virtual CCRStreamingSession::TCRSourceId Id( void ) const; 
    
    /**
    * Registers connection observer.
    * @since Series 60 3.0
    * @param aObserver aObserver.
    * @return none.
    */
    virtual void RegisterConnectionObs( MCRConnectionObserver* aObserver );

protected: // Constructors and destructors

    /**
    * Default constructor 
    */
    CCRPacketSourceBase( CCRStreamingSession& aSession,
                         CCRStreamingSession::TCRSourceId aSourceId ); 

protected: // Data
    
    /** 
    * Pointer to streaming session that owns us.
    */
    CCRStreamingSession& iOwningSession;                     
    
    /**
    * Buffer used by sources.
    */
    CCRPacketBuffer* iBuffer;
     
    /**
    * Source id, set by owner
    */
    const CCRStreamingSession::TCRSourceId iSourceId; 
    
    };

#endif // MCRPACKETSOURCEBASE_H

//  End of File
