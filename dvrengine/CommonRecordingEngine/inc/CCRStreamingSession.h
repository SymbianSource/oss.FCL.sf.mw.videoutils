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
* Description:    Streaming session, eg one url, one channel...*
*/




#ifndef _CCRSTREAMINGSESSION_
#define _CCRSTREAMINGSESSION_

// INCLUDES
#include <ipvideo/CRTypeDefs.h>
#include "MCRStreamObserver.h"
#include <e32base.h>
#include <es_sock.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRPacketBuffer;
class CAsyncCallBack;
class CCRConnection;
class CCRPacketSourceBase;
class CCRPacketSinkBase;
class CRtpClipHandler;
class CCREngine;
class MCRPacketSource;

// CLASS DECLARATION

/**
*  Streaming session.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/

class CCRStreamingSession : public CBase,
                            public MCRStreamObserver
    {

public: // Datatype definitions
    
    enum TCRSinkId
        {
        ECRRtspSinkId = 7000,
        ECRXpsSinkId,
        ECR3gpRecSinkId,
        ECRRtpRecSinkId,
        ECRNullSinkId
        };

    enum TCRSourceId
        {
        ECRRtspSourceId = 7100,
        ECRDvbhSourceId,
        ECRRtpSourceId,
        ECRNullSourceId
        };

public: // Constructors and destructors

    /**
    * Two-phased constructor.
    * @return CCRStreamingSession pointer. 
    */
    static CCRStreamingSession* NewL( RSocketServ& aSockServer,
                                      CCRConnection* aConnection,
                                      CCREngine& aEngine );

    /**
    * Destructor 
    */
    virtual ~CCRStreamingSession();

public: // New methods

    /**
    * Method used to start a new RTSP session with parameters.
    * @since Series 60 3.0
    * @param aParams specifies the connection addr.
    * @param aClipHandler a reference pointer to RTP clip handler.
    * @param aSessionDefinition a session definition string.
    * @return none.
    */
    void OpenSourceL( const SCRRtspParams& aParams,
                      const TDesC& aSessionDefinition );

    /**
    * Method used to start a new RTP session with parameters.
    * @since Series 60 3.0
    * @param aParams specifies the RTP clip parameters.
    * @param aClipHandler a reference pointer to RTP clip handler.
    * @param aSessionDefinition a session definition string.
    * @return none.
    */
    void OpenSourceL( const SCRRtpPlayParams& aParams,
                      CRtpClipHandler*& aClipHandler,
                      const TDesC& aSessionDefinition );

    /**
    * Method used to start a new RTP session with parameters.
    * @since Series 60 3.0
    * @param aRtpHandle a open file handle for RTP file.
    * @param aParams specifies the RTP clip parameters.
    * @param aSessionDefinition a session definition string.
    * @return none.
    */
    void OpenSourceL( const RFile& aRtpHandle,
                      CRtpClipHandler*& aClipHandler,
                      const TDesC& aSessionDefinition );

    /**
    * Method used to start a new DVB-H session with parameters.
    * @since Series 60 3.0
    * @param aLiveParams specifies DVB-H live parameters.
    * @param aSessionDefinition a session definition string.
    * @return none.
    */
    void OpenSourceL( const SCRLiveParams& aLiveParams,
                      const TDesC& aSessionDefinition );

    /**
    * Method used to start a new Null source session.
    * @since Series 60 3.0
    * @param aSessionDefinition a session definition string.
    * @return none.
    */
    void OpenSourceL( const TDesC& aSessionDefinition );

    /**
    * Calculates source definition checksum: URL, file name etc.
    * @since Series 60 3.0
    * @param aName a pointer to defination name.
    * @return none.
    */
    TUint SourceDefinition( const TDesC& aName );

    /**
    * Getter for source definition checksum.
    * @since Series 60 3.0
    * @param none.
    * @return a value of source definition checksum.
    */
    TUint SourceChecksum();

    /**
    * Method that creates a set of buffers depending on
    * number of rtp streams to handle.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void CreateAndSetBufferL();

    /**
    * Method that creates a packet sink that forwards 
    * packets to rop plugin for rendering.
    * @since Series 60 3.0
    * @param aLoopbackPort a RTSP port for loopback socket.
    * @return none.
    */
    void CreateRtspSinkL( const TInt& aLoopbackPort );

    /**
    * Creates XPS sink for passing packets to Helix player.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void CreateXpsSinkL();

    /**
    * Creates RTP clip sink for saving packets to a clip.
    * @since Series 60 3.0
    * @param aRecordParams a recording parameters.
    * @return none.
    */
    void Create3gpRecordSinkL( const SCRRecordParams& aRecordParams );

    /**
    * Creates RTP clip sink for saving packets to a clip.
    * @since Series 60 3.0
    * @param aRecordParams a recording parameters.
    * @param aClipHandler a reference pointer to RTP clip handler.
    * @return none.
    */
    void CreateRtpRecordSinkL( const SCRRecordParams& aRecordParams,
                               CRtpClipHandler*& aClipHandler );

    /**
    * Post action after source initialized.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void PostActionL();

    /**
    * Play command for current source.
    * @since Series 60 3.0
    * @param aStartPos is start position in seconds.
    * @param aEndPos is end position in seconds.
    * @return a system wide error code.
    */
    TInt PlayCommand( const TReal& aStartPos,
                      const TReal& aEndPos );

    /**
    * Pause command for current source.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    TInt PauseCommand();

    /**
    * Stop command for current source.
    * @since Series 60 3.0
    * @param none.
    * @return a system wide error code.
    */
    TInt StopCommand();

    /**
    * Setter for source position.
    * @since Series 60 3.0
    * @param aPosition a current play position of the clip.
    * @return a system wide error code.
    */
    TInt SetPosition( const TInt64 aPosition );

    /**
    * Getter for source position.
    * @since Series 60 3.0
    * @param aPosition a current play position of the clip.
    * @param aDuration a duration of the clip.
    * @return a system wide error code.
    */
    TInt GetPosition( TInt64& aPosition,
                      TInt64& aDuration );

    /**
    * Pause command for wanted sink.
    * @since Series 60 3.0
    * @param aSinkId identifies the sink.
    * @return a system wide error code.
    */
    TInt PauseCommand( const TCRSinkId& aSinkId );

    /**
    * Restore command for wanted sink.
    * @since Series 60 3.0
    * @param aSinkId identifies the sink.
    * @return a system wide error code.
    */
    TInt RestoreCommand( const TCRSinkId& aSinkId );

    /**
    * Stop command for wanted sink.
    * @since Series 60 3.0
    * @param aSinkId identifies the sink.
    * @return a system wide error code.
    */
    TInt StopCommand( const TCRSinkId& aSinkId );

    /**
    * Method to transfer ownership of existing sink.
    * @since Series 60 3.0
    * @param aSinkId identifies the sink.
    * @param aTargetSession identifies the sink.
    * @return a system wide error code.
    */
    TInt TransferSink( const TCRSinkId& aSinkId,
                       CCRStreamingSession& aTargetSession );

    /**
    * Method to add new sink to session.
    * @since Series 60 3.0
    * @param aSink a pointer to the sink to add.
    * @return a system wide error code.
    */
    TInt AddNewSink( CCRPacketSinkBase* aSink );

    /**
    * Veryfies if source or any sink on session uses RTP clip handler.
    * @since Series 60 3.0
    * @param none.
    * @return true if clip handler user, otherwise false.
    */
    TBool ClipHandlerUsed();

    /**
    * Method that creates a packet sink that forwards.
    * packets to bit bucket.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void CreateNullSinkL();

    /**
    * Sets SDP and buffer for sinks. 
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void InitializeSinks();

    /**
    * Set timestamp and sequence numbers from source to sinks.
    * If Range is available, sets it too.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void SetSeqAndTs();

    /**
    * Method that sink calls to communicate that it wants to quit its business.
    * It is now our responsibility to delete the sink after buffers have been
    * notified about the situation.
    * @since Series 60 3.0
    * @param aSinkId identifies the sink.
    * @return none.
    */
    void SinkStops( const TCRSinkId& aSinkId );

    /**
    * Method that source uses to communicate that it wants more packets.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void SourceRestore();

    /**
    * Method that source uses to communicate that it wants to be deleted.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void SourceStop();

private: // Constructors and destructors

    /**
    * by default default constructor is private
    * @param aSockServer is the socketserver to use when opening socks.
    * @param aConnection 
    * @param aEngine 
    */
    CCRStreamingSession( RSocketServ& aSockServer,
                         CCRConnection* aConnection,
                         CCREngine& aEngine );

    /**
    * second phase of 2-phased construction
    */
    void ConstructL();

private: // Functions from base classes

    /**
    * From MCRStreamObserver.
    * Source status indication.
    * @since Series 60 3.0
    * @param aStatus 
    * @return none.
    */
    void StatusChanged( MCRPacketSource::TCRPacketSourceState aStatus );

private: // New methods

    /**
    * Static callback called via CAsyncCallBack to initiate cleaning of a sink
    * @since Series 60 3.0
    * @param aThis a pointer to owner of call back.
    * @return none.
    */
    static TInt SinkStopCallBack( TAny* aThis );

    /**
    * Method for cleaning a sink. Called from SinkStopCallBack
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void DoSinkStop( void );

    /**
    * Method that deletes a packet sink.
    * @since Series 60 3.0
    * @param aSinkId identifies the sink.
    * @return none.
    */
    void DeleteSink( const TCRSinkId& aSinkId );

public: // Data

    /**
    * Socket server.
    */
    RSocketServ& iSockServer;

    /**
    * Connections.
    */
    CCRConnection* iConnection;

    /**
    * Reference to engine object owning this instance.
    */
    CCREngine& iEngine;

    /**
    * Callback for cutting the call stack in cleanup.
    */
    CAsyncCallBack *iCleanUp;

    /**
    * Packet sources.
    */
    CCRPacketSourceBase* iSource;

    /**
    * Source definition checksum.
    */
    TUint iSourceChecksum;

    /**
    * Packet buffer.
    */
    CCRPacketBuffer* iBuffer;

    /**
    * Packet sinks.
    */
    RPointerArray<CCRPacketSinkBase> iSinks;

    /**
    * Array of sinks that have declared that they want to be deleted.
    */
    RArray<TCRSinkId> iSinksToDelete;

};

#endif // _CCRSTREAMINGSESSION_

//  End of File
