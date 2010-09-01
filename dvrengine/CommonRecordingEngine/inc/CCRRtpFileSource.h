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




#ifndef CCRRTPFILESOURCE_H
#define CCRRTPFILESOURCE_H

//  INCLUDES
#include "CCRPacketSourceBase.h"
#include <ipvideo/MRtpFileReadObserver.h>
#include <ipvideo/MRtpFileWriteObserver.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CRtpClipHandler;

// CLASS DECLARATION

/**
*  Class that negotiaties RTP stream using RTSP.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRRtpFileSource : public CCRPacketSourceBase,
                         public MRtpFileReadObserver
    {

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @return CCRRtpFileSource pointer to CCRRtpFileSource class
    */
    static CCRRtpFileSource* NewL( const SCRRtpPlayParams& aParams,
                                   CRtpClipHandler*& aClipHandler,
                                   MCRStreamObserver& aSessionObs,
                                   CCRStreamingSession& aOwningSession );

    /**
    * Two-phased constructor.
    * @return CCRRtpFileSource pointer to CCRRtpFileSource class
    */
    static CCRRtpFileSource* NewL( const RFile& aRtpHandle,
                                   CRtpClipHandler*& aClipHandler,
                                   MCRStreamObserver& aSessionObs,
                                   CCRStreamingSession& aOwningSession );

    /**
    * Destructor.
    */
    virtual ~CCRRtpFileSource();

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
    CCRRtpFileSource( CRtpClipHandler*& aClipHandler,
                      MCRStreamObserver& aSessionObs,
                      CCRStreamingSession& aOwningSession );

    /**
    * By default Symbian 2nd phase constructor is private.
    */
    void ConstructL( const SCRRtpPlayParams& aParams );

    /**
    * By default Symbian 2nd phase constructor is private.
    */
    void ConstructL( const RFile& aRtpHandle );

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
    * Setter for current position.
    * @since Series 60 3.0
    * @param aPosition a postion of playback.
    * @return a system wide error code.
    */
    TInt SetPosition( const TInt64 aPosition );
    
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

private: // New methods

    /**
    * Method for requesting more packets to buffer.
    * @since Series 60 3.0
    * @param aForce if true, forces read packets even buffer more than threshold.
    * @return a system wide error code.
    */
    TInt NextClipGroup( const TBool aForce );

    /**
    * Setter for current position of .rtp clip.
    * @since Series 60 3.0
    * @param aPosition a postion of playback.
    * @return a system wide error code.
    */
    TInt RtpPosition( const TUint aPosition );
    
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
    * Clip handler for RTP clip.
    */
    CRtpClipHandler*& iClipHandler;

    /**
    * Session observer.
    */
    MCRStreamObserver& iSessionObs;
    
    /**
    * SDP data.
    */
    HBufC8* iSdp;
    
    /**
    * Time of initialy readed group.
    */
    TUint iInitialTime;

    /**
    * Pause packet sent to sink from a clip.
    */
    TInt iClipPauseSent;

    };

#endif // CCRRTPFILESOURCE_H

// End of file
