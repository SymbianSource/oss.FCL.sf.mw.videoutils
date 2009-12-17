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




#ifndef CCRNULLSOURCE_H
#define CCRNULLSOURCE_H

//  INCLUDES
#include "CCRPacketSourceBase.h"
#include <ipvideo/MRtpFileReadObserver.h>
#include <ipvideo/MRtpFileWriteObserver.h>
#include "MRtpTimerObserver.h"

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CRtpClipHandler;
class CRtpTimer;

// CLASS DECLARATION

/**
*  Class that negotiaties RTP stream using RTSP.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRNullSource : public CCRPacketSourceBase,
                      public MRtpFileReadObserver,
                      public MRtpTimerObserver
    {

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @return CCRNullSource pointer to CCRNullSource class
    */
    static CCRNullSource* NewL( const TDesC& aClipName,
                                MCRStreamObserver& aSessionObs,
                                CCRStreamingSession& aOwningSession );

    /**
    * Destructor.
    */
    virtual ~CCRNullSource();

public: // New functions

    /**
    * 
    * @since Series 60 3.0
    * @param none. 
    * @return none.
    */
    //void PunchPacketsSent();

private: // Constructors and destructor

    /**
    * C++ default constructor.
    * @param aSessionObs a session observer.
    * @param aOwningSession a owning session.
    */
    CCRNullSource( MCRStreamObserver& aSessionObs,
                   CCRStreamingSession& aOwningSession );

    /**
    * By default Symbian 2nd phase constructor is private.
    */
    void ConstructL( const TDesC& aClipName );

private: // Methods from base classes

    /**
    * From CCRPacketSourceBase.
    * Method for acquiring sdp.
    * @since Series 60 3.0
    * @param aSdp is string pointer that will be .Set() to contain the sdp.
    *        If no sdp is available no .Set() will occur.
    * @return KErrNotReady if no sdp available.
    */
    TInt GetSdp( TPtrC8& aSdp );

    /**
    * From CCRPacketSourceBase.
    * Method for setting buffers to packet source
    * @since Series 60 3.0
    * @param aBuffer is buffer to be set.
    * @return none.
    */
    void SetBuffer( CCRPacketBuffer* aBuffer );

    /**
    * From CCRPacketSourceBase.
    * Post action after source initialized.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void PostActionL();

    /**
    * From CCRPacketSourceBase.
    * Method for requesting more packets to buffer.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void Restore();

    /**
    * From CCRPacketSourceBase.
    * Method for ordering "play" for packet source.
    * @since Series 60 3.0
    * @param aStartPos is start position in seconds.
    * @param aEndPos is end position in seconds.
    * @return a system wide error code.
    */
    TInt Play( const TReal& aStartPos,
               const TReal& aEndPos );

    /**
    * From CCRPacketSourceBase.
    * Method for stopping play from source.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    TInt Stop();

    /**
    * From CCRPacketSourceBase.
    * Getter for current position.
    * @since Series 60 3.0
    * @param aPosition a postion of playback.
    * @param aDuration a duration of playback.
    * @return a system wide error code.
    */
    TInt GetPosition( TInt64& aPosition,
                      TInt64& aDuration );
    
    /**
    * From MRtpFileReadObserver.
    * Called when RTP packets group readed from a file.
    * @since Series 60 3.0
    * @param aGroup a RTP data readed from a file.
    * @param aGroupTime a group time in milli seconds.
    * @param aLastGroup a indication of last group in clip.
    * @return none.
    */
    void GroupReadedL( const TDesC8& aGroup,
                       const TUint aGroupTime,
                       const TBool aLastGroup );

    /**
    * From MRtpFileReadObserver.
    * File handler status of playback.
    * @since Series 60 3.0
    * @param aStatus a status of file reading.
    * @return none.
    */
    void ReadStatus( TInt aStatus );

    /**
    * From MRtpTimerObserver.
    * Called when timer completion event occurs
    * @since Series 60 3.0
    * @return none
    */
    void TimerEventL();

    /**
    * From MRtpTimerObserver.
    * Handles a leave occurring in the request
    * completion event handler RunL().
    * @since Series 60 3.0
    * @param aError the leave code.
    * @return None.
    */
    void TimerError( const TInt aError );

private: // New methods

    /**
    * Method for requesting more packets to buffer.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    TInt NextClipGroup();

    /**
    * Converts old RTP propriatary format packet type to stream id.
    * @since Series 60 3.0
    * @param aType a packet type from a clip.
    * @param aStream a stream id to update.
    * @return true if packet valid, otherwise false.
    */
    TBool TypeToStream( const MRtpFileWriteObserver::TRtpType& aType,
                        MCRPacketSource::TCRPacketStreamId& aStream );

private:  // Data

    /**
    * Session observer.
    */
    MCRStreamObserver& iSessionObs;
    
    /**
    * RTP clip handler.
    */
    CRtpClipHandler* iClipHandler;

    /**
    * SDP data.
    */
    HBufC8* iSdp;
    
    /**
    * Packets flow timer.
    */
    CRtpTimer* iFlowTimer;
    
    /**
    * Time of last readed group.
    */
    TUint iGroupTime;

    };

#endif // CCRNULLSOURCE_H

// End of file
