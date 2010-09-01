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
* Description:    Class for all rtp packet sources*
*/



#ifndef MCRPACKRTSOURCE_H
#define MCRPACKRTSOURCE_H

// INCLUDES
#include <e32def.h>
#include <e32cmn.h>

// CONSTANTS
const TReal KRealZero( 0.0 ); 
const TReal KRealMinusOne( -1.0 ); 

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRPacketBuffer;

// CLASS DECLARATION

/**
* Interface for packet sources
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class MCRPacketSource
    {

public: // Data types

    /**
    * Enum for indicating what stream an individual rtp packet belongs to.
    */
    enum TCRPacketStreamId
        {
        EAudioStream = 0,           /**< This rtp stream carries audio content data */
        EAudioControlStream = 1,    /**< This rtp stream carries audio control data */
        EVideoStream = 2,           /**< This rtp stream carries video content data */
        EVideoControlStream = 3,    /**< This rtp stream carries video control data */
        ESubTitleStream = 4,        /**< This rtp stream carries sub title content data */
        ESubTitleControlStream = 5, /**< This rtp stream carries sub title control data */
        EDisContinousStream = 6,    /**< This is info of discontinous point in stream */
        EStreamEndTag = 7,          /**< This is info of end point of stream */
        EStreamIdCount              /**< Number of different sources */
        };    

    /** 
    * Enum for indicating rtp packet source state.
    */
    enum TCRPacketSourceState
        {
        ERtpStateIdle = 0,          /**< Nothing started yet */
        ERtpStateSdpAvailable,      /**< SDP received */
        ERtpStateSeqAndTSAvailable, /**< Should be ready for prime-time */
        ERtpStateSetupRepply,       /**< Setup repply received */
        ERtpStatePlaying,           /**< Stream is playing */
        ERtpStateClosing,           /**< Stream is about to end */
        ERtpStateCount              /**< Count number of different states */
        };
            
public: // New methods

    /**
    * Method for acquiring sdp. 
    * @since Series 60 3.0
    * @param aSdp is string pointer that will be .Set() to contain
    *        the sdp. If no sdp is available no .Set() will occur.
    * @return KErrNotReady if no sdp available. 
    */
    virtual TInt GetSdp( TPtrC8& aSdp ) = 0;

    /**
    * Method for acquiring (almost) up-to-date sequence and ts numbers.
    * @since Series 60 3.0
    * @param aAudioSeq is reference to TUint that will be set by this
    *        method to contain latest available sequence number for
    *        audio stream being received via this packet source. 
    * @param aAudioTS rtp timestamp for audio
    * @param aVideoSeq rtp seq for video. If no video,
             value will not be touched.
    * @param aVideoTS rtp ts for video.
    * @return KErrNone if data available. 
    */
    virtual TInt SeqAndTS( TUint& aAudioSeq,
                           TUint& aAudioTS,
                           TUint& aVideoSeq,
                           TUint& aVideoTS ) = 0; 

    /** 
    * Method for ordering "play" for packet source.
    * @since Series 60 3.0
    * @param aStartPos is start position in sends.
    * @param aEndPos is play end position in seconds.
    * @return a system wide error code.
    */         
    virtual TInt Play( const TReal& aStartPos,
                       const TReal& aEndPos ) = 0;

    /** 
    * Method for ordering "pause" for packet source.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */         
    virtual TInt Pause() = 0;
                
    /** 
    * Method for stopping play from source.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    virtual TInt Stop() = 0;
        
    /**
    * Method for getting range of stream. If no range/duration
    * is available this method will set the return values to
    * 0.0,-1.0 and that may be quite normal state live streams. 
    * Also, depending on type of source this data may not be always
    * available, for instance rtsp sources know the range after
    * play command has been issued but not before. 
    * @since Series 60 3.0
    * @param aLower is where to start from. If no value available, 
    *        value of aLower must be set to 0
    * @param aUpper is where to stop. Negative values mean eternity.
    * @return none
    */      
    virtual void GetRange( TReal& aLower,
                           TReal& aUpper ) = 0; 
    
    /**
    * Post action after source initialized.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    virtual void PostActionL() = 0;

    /**
    * Method for requesting more packets to buffer.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    virtual void Restore() = 0;

    /**
    * Setter for current position.
    * @since Series 60 3.0
    * @param aPosition a postion of playback.
    * @return a system wide error code.
    */
    virtual TInt SetPosition( const TInt64 aPosition ) = 0;
    
    /**
    * Getter for current position.
    * @since Series 60 3.0
    * @param aPosition a postion of playback.
    * @param aDuration a duration of playback.
    * @return a system wide error code.
    */
    virtual TInt GetPosition( TInt64& aPosition,
                              TInt64& aDuration ) = 0;
    
    };

#endif // MCRPACKRTSOURCE_H

//  End of File
