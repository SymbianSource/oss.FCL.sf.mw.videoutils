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
* Description:    Packet sink where no-one gets the streaming*
*/



#ifndef CCRRTPRECORDSINK_H
#define CCRRTPRECORDSINK_H

// INCLUDES
#include "CCRPacketSinkBase.h"
#include <ipvideo/MRtpFileWriteObserver.h>
#include "MCRTimerObserver.h"
#include <CXPSPktSinkObserver.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRPacketBuffer;
class CCRStreamingSession;
class MCRConnectionObserver;
class CRtpClipHandler;
class CRtpTsConverter;

// CLASS DECLARATION

/**
*  Packet sink that does not forward packets. Good for testing. 
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRRtpRecordSink : public CCRPacketSinkBase,
                         public MRtpFileWriteObserver
    {

public: // Constructors and destructors   
    
    /**
    * Two-phased constructor.
    * @param aRecordParams a recording parameters.
    * @param aSinkId that this class needs to use when reporting progress back to owner.
    * @param aOwningSession is the streaming session that owns this instance.
    * @param aObserver a pointer to connection observer.
    * @param aClipHandler a reference pointer to .rtp clip handler.
    * @return CCRRtpRecordSink pointer. 
    */
    static CCRRtpRecordSink* NewL( const SCRRecordParams& aRecordParams,
                                   CCRStreamingSession::TCRSinkId aSinkId,
                                   CCRStreamingSession& aOwningSession,
                                   MCRConnectionObserver* aObserver,
                                   CRtpClipHandler*& aClipHandler );
    
    /**
    * Destructor 
    */
    virtual ~CCRRtpRecordSink();
    
protected: // Constructors and destructors

    /**
    * By default default constructor is private
    * @param aSinkId that this class needs to use when reporting progress back to owner.
    * @param aObserver a pointer to connection observer.
    * @param aClipHandler a reference pointer to .rtp clip handler.
    * @param aOwningSession is the streaming session that owns this instance.
    */
    CCRRtpRecordSink( CCRStreamingSession::TCRSinkId aSinkId,
                      CCRStreamingSession& aOwningSession,
                      MCRConnectionObserver* aObserver,
                      CRtpClipHandler*& aClipHandler );

    /**
    * Second phase of 2-phased construction
    * @param aRecordParams a recording parameters.
    */
    void ConstructL( const SCRRecordParams& aRecordParams ); 

private: // Methods from base classes
    
    /**
    * From CCRPacketSinkBase.
    * Method for setting the sdp in use.
    * @since Series 60 3.0
    * @param aSdp is the new sdp.
    * @return none.
    */
    void SetSdpL( const TDesC8& aSdp );
    
    /**
    * From CCRPacketSinkBase.
    * Adds packet to the sink.
    * @since Series 60 3.0
    * @return none.
    */
    void NewPacketAvailable();
    
    /**
    * From CCRPacketSinkBase.
    * Buffer reset info for the sink.
    * @since Series 60 3.0
    * @return none.
    */
    void BufferResetDone();
    
    /**
    * From CCRPacketSinkBase.
    * Method for pause action for sink.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    TInt Pause();

    /**
    * From CCRPacketSinkBase.
    * Method for restore action for sink.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    TInt Restore();

    /**
    * From CCRPacketSinkBase.
    * Method for stopping action for sink.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void Stop();

    /**
    * From MRtpFileWriteObserver.
    * Group saved indication.
    * @since Series 60 3.0
    * @return none.
    */
    void GroupSaved();

    /**
    * From MRtpFileWriteObserver.
    * Clip handler status of recording.
    * @since Series 60 3.0
    * @param aStatus a status of file writing.
    * @return none.
    */
    void WriteStatus( const TInt aStatus );

private: // New methods

    /**
    * Adds RTP packet(s) to a group.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void AddToGroup();

    /**
    * Adds one RTP packet to a group.
    * @since Series 60 3.0
    * @param aPacket a packet data.
    * @param aType a packet type.
    * @return none.
    */
    void AddPacketToGroupL( const TDesC8& aPacket,
                            const MRtpFileWriteObserver::TRtpType& aType );

    /**
    * Saves one RTP packet group to a clip.
    * @since Series 60 3.0
    * @param aAction a saving action.
    * @return none.
    */
    void SaveGroup( MRtpFileWriteObserver::TRtpSaveAction aAction );

    /**
    * Saves one RTP packet group to a clip.
    * @since Series 60 3.0
    * @param aAction a saving action.
    * @return none.
    */
    void SaveGroupL( MRtpFileWriteObserver::TRtpSaveAction aAction );

    /**
    * Converts stream id to old RTP propriatary format packet type.
    * @since Series 60 3.0
    * @param aType a packet type from a clip.
    * @param aStream a stream id to update.
    * @return true if packet valid, otherwise false.
    */
    TBool StreamToType( const MCRPacketSource::TCRPacketStreamId& aStream,
                        MRtpFileWriteObserver::TRtpType& aType );

    /**
    * Handles TS delta calculation from audio packets.
    * @since Series 60 3.0
    * @param aPacket a packet data.
    * @return a TS of packet.
    */
    TUint TsFromPacketL( const TDesC8& aPacket );

    /**
    * Wrapper for AddPausePacketL().
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void AddPausePacket();

    /**
    * Adds pause packet to group buffer.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void AddPausePacketL();

    /**
    * Resets packets group variables.
    * @since Series 60 3.0
    * @param aStatus a reason for recording end.
    * @return none.
    */
    void ForceStopRecording( const TInt& aStatus );

    /**
    * Resets packets group variables.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void ResetGroupVariables();

private: // Data

    /**
    * Packets group buffer.
    */
    HBufC8* iGroupBuffer;
    
    /**
    * Connection status observer.
    */
    MCRConnectionObserver* iObserver;
    
    /**
    * Clip handler for RTP clip.
    */
    CRtpClipHandler*& iClipHandler;

    /**
    * Packets group pointer.
    */
    TPtr8 iGroupPointer;
    
    /**
    * Current group size in bytes.
    */
    TInt iGroupSize;
    
    /**
    * Current group packets count.
    */
    TInt iPacketsCount;
    
    /**
    * Wanted group size.
    */
    TInt iWantedGroup;
    
    /**
    * TS converter for audio.
    */
    CRtpTsConverter* iAudioConv;

    /**
    * Time stamp of oldest audio packet.
    */
    TUint iOldestTs;
    
    /**
    * Latest audio packet.
    */
    TPtrC8 iLatestAudio;
    
    /**
    * Record mode, can be either normal of time shift.
    */
    MRtpFileWriteObserver:: TRtpSaveAction iSaveMode;
    
    /**
    * Next packet(s) mode, can be: normal, pause, end.
    */
    MRtpFileWriteObserver:: TRtpSaveAction iGroupMode;
    
    /**
    * Recording parameters.
    */
    MRtpFileWriteObserver::SRtpRecParams iRecParams;
    
    };

#endif // CCRRTPRECORDSINK_H

//  End of File
