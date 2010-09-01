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
* Description:    Engine part of the engine. Engineered to keep count*
*/




#ifndef CCRENGINE_H
#define CCRENGINE_H

//  INCLUDES
#include "CCRConnection.h"
#include <ipvideo/CRTypeDefs.h>
#include <e32base.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None
    
// FORWARD DECLARATIONS
class CAsyncCallBack;
class CCRSession;
class CCRStreamingSession;
class CCRClientInformer;
class CRtpClipHandler;

// CLASS DECLARATION

/**
*  Server's bookkeeping part: knows what clients there are and thru which
*  client sessions, what streaming sessions there are and passes commands
*  to those sessions and sinks. 
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCREngine : public CObject,
                  public MCRConnectionObserver
    {

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @return CCREngine pointer to CCREngine class
    */
    static CCREngine* NewL( void );

    /**
    * Destructor.
    */
    virtual ~CCREngine();

public: // New functions

    /**
    * Does the general message handling.
    * @since Series 60 3.0
    * @param aMessage contains data from the client.
    * @return none.
    */
    void GeneralServiceL( const RMessage2& aMessage );

    /**
    * Method that streams call to ask for resignation. 
    * @since Series 60 3.0
    * @param aSession is pointer to streamingsession that wants to quit.
    * @return none.
    */
    void SessionStop( CCRStreamingSession* aSession ); 
    
private: // Constructors and destructor

    /**
    * C++ default constructor.
    */
    CCREngine( void );

    /**
    * By default Symbian 2nd phase constructor is private.
    */
    void ConstructL();
            
public: // Methods from base classes

    /**
    * From MCRConnectionObserver.
    * receives notifications of connection status changes,
    * used to clear heuristics on streaming capabilities.
    * @since Series 60 3.0
    * @param aSessionId a session id.
    * @param aStatus is the new status of the connection.
    * @param aErr contains error code if new status is failure status.
    * @return none.
    */
    void ConnectionStatusChange( TInt aSessionId,
                                 TCRConnectionStatus aStatus,
                                 TInt aErr );
    
private: // New methods

    /**
    * Verifies needed cababilities of DVR client user.
    * @since Series 60 3.0
    * @param aMessage contains data from the client.
    * @return None.
    */
    void VerifyCapabilitiesL( const RMessage2& aMessage );

    /**
    * Sets connection.
    * @since Series 60 3.0
    * @param aIap ?.
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    TInt SetConnectionId( const SCRRtspIapParams& aIap );
    
    /**
    * Cancel connetion set.
    * @since Series 60 3.0
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    TInt CancelSetConnectionId();
    
    /**
    * Starts RTSP URL streaming.
    * @since Series 60 3.0
    * @param aRtspParams a RTPS params.
    * @return a session definition checksum.
    */
    TUint PlayRtspUrlL( const SCRRtspParams& aRtspParams );

    /**
    * Starts DVB-H live streaming.
    * @since Series 60 3.0
    * @param aLiveParams is set of parameters required for DVB-H.
    * @return a session definition checksum.
    */
    TUint PlayDvbhLiveL( const SCRLiveParams& aLiveParams );

    /**
    * Change service (channel) of DVB-H live stream.
    * @since Series 60 3.0
    * @param aCurrentSessionChk a current service session checksum.
    * @param aLiveParams is set of parameters required for DVB-H.
    * @return a session definition checksum.
    */
    TUint ChangeDvbhServiceL( const TUint aSessionChk,
                              const SCRLiveParams& aLiveParams );

    /**
    * Starts RTP file format playing.
    * @since Series 60 3.0
    * @param aRtpParams a RTP clip play params.
    * @return a session definition checksum.
    */
    TUint PlayRtpFileL( const SCRRtpPlayParams& aRtpParams );

    /**
    * Starts RTP file format playing.
    * @since Series 60 3.0
    * @param aFileHandle a open file handle for RTP file.
    * @return a session definition checksum.
    */
    TUint PlayRtpFileL( const RFile& aFileHandle );

    /**
    * Starts recording of current stream.
    * @since Series 60 3.0
    * @param aSessionChk a session defination checksum.
    * @param aRecordParams a recording parameters.
    * @return none.
    */
    void RecordCurrentStreamL( const TUint aSessionChk,
                               const SCRRecordParams& aRecordParams );
    
    /**
    * Starts recording of RTSP stream.
    * @since Series 60 3.0
    * @param aRtspUrl is set of parameters required for rtsp.
    * @param aRecordParams a recording parameters.
    * @return a session definition checksum.
    */
    TUint RecordRtspStreamL( const SCRRtspParams& aRtspUrl,
                             const SCRRecordParams& aRecordParams );
    
    /**
    * Starts recording of DVB-H stream.
    * @since Series 60 3.0
    * @param aLiveParams is set of parameters required for DVB-H.
    * @param aRecordParams a recording parameters.
    * @return a session definition checksum.
    */
    TUint RecordDvbhStreamL( const SCRLiveParams& aLiveParams,
                             const SCRRecordParams& aRecordParams );
    
    /**
    * Pauses/Resumes current recording.
    * @since Series 60 3.0
    * @param aSessionChk a session defination checksum.
    * @param aStart a start or end pausing.
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    TInt PauseRecordStream( const TUint aSessionChk,
                            const TBool& aStart );

    /**
    * Stops current recording.
    * @since Series 60 3.0
    * @param aSessionChk a session defination checksum.
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    TInt StopRecordStream( const TUint aSessionChk );

    /**
    * Starts time shifting.
    * @since Series 60 3.0
    * @param aSessionChk a session defination checksum.
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    TInt StartTimeShiftL( const TUint aSessionChk );

    /**
    * Stops time shifting.
    * @since Series 60 3.0
    * @param aTimeShiftChk a session definition checksum.
    * @param aCurrentChk a session definition of existing session.
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    TInt StopTimeShift( const TUint aTimeShiftChk,
                        const TUint aCurrentChk );

    /**
    * Play command for wanted source.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aStartPos is start position in seconds.
    * @param aEndPos is end position in seconds.
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    TInt PlayCommand( const TUint aSessionChk,
                      const TReal aStartPos,
                      const TReal aEndPos );

    /**
    * Pause command for wanted source.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    TInt PauseCommand( const TUint aSessionChk );

    /**
    * Stop command for wanted source.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    TInt StopCommand( const TUint aSessionChk );

    /**
    * Setter for source position.
    * @since Series 60 3.0
    * @param aSessionChk a session defination checksum.
    * @param aPosition a current play position of the clip.
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    TInt SetPosition( const TUint aSessionChk,
                      const TInt64 aPosition );

    /**
    * Getter for source position.
    * @since Series 60 3.0
    * @param aSessionChk a session defination checksum.
    * @param aPosition a current play position of the clip.
    * @param aDuration a duration of the clip.
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    TInt GetPosition( const TUint aSessionChk,
                      TInt64& aPosition,
                      TInt64& aDuration );

    /**
    * Closes wanted source and all sinks.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    TInt CloseSession( const TUint aSessionChk );

    /**
    * Emulates live source from a .rtp clip.
    * @since Series 60 3.0
    * @param none.
    * @return a session definition checksum.
    */
    TUint PlayNullSourceL();
    
    /**
    * Sends RTSP stream packets to null sink.
    * @since Series 60 3.0
    * @param aRtspParams a RTPS params.
    * @return a session definition checksum.
    */
    TUint PlayRtspUrlToNullSinkL( const SCRRtspParams& aRtspParams );
    
    /**
    * Creates connection.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void CreateConnectionL( void ); 
    
    /**
    * Creates RTP clip handler.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void CreateClipHandlerL( void ); 
    
    /**
    * Verifies if RTSP session with params is already available.
    * @since Series 60 3.0
    * @param aRtspParams a RTPS params.
    * @return an index to sessions array if session found,
              otherwise KErrNotFound.
    */
    TInt VerifyRtspSessionL( const SCRRtspParams& aRtspParams );

    /**
    * Verifies if DVB-H session with params is already available.
    * @since Series 60 3.0
    * @param aLiveParams is set of parameters required for DVB-H.
    * @return an index to sessions array if session found,
              otherwise KErrNotFound.
    */
    TInt VerifyDvbhSessionL( const SCRLiveParams& aLiveParams );

    /**
    * Verifies if DVB-H session with params is already available.
    * @since Series 60 3.0
    * @param aRtpParams a RTP clip play params.
    * @return an index to sessions array if session found,
              otherwise KErrNotFound.
    */
    TInt VerifyRtpFileSessionL( const SCRRtpPlayParams& aRtpParams );

    /**
    * Verifies if session with defination name is available.
    * @since Series 60 3.0
    * @param aFileHandle a open file handle for RTP file.
    * @return an index to sessions array if names match,
              otherwise KErrNotFound.
    */
    TInt VerifyRtpFileSessionL( const RFile& aFileHandle );

    /**
    * Verifies if session with defination name is available.
    * @since Series 60 3.0
    * @param aName a definition name of the session.
    * @return an index to sessions array if names match,
              otherwise KErrNotFound.
    */
    TInt VerifySession( const TDesC& aName );

    /**
    * Verifies if session with definition checksum is available.
    * @since Series 60 3.0
    * @param aSessionChk a session checksum to verify for.
    * @return an index to sessions array if names match,
              otherwise KErrNotFound.
    */
    TInt VerifySession( const TUint aSessionChk );

    /**
    * Creates recording sin base on file format.
    * @since Series 60 3.0
    * @param aSessionIndex a session index.
    * @param aRecordParams a recording parameters.
    * @return none.
    */
    void CreateRecordingSinkL( const TInt aSessionIndex,
                               const SCRRecordParams& aRecordParams ); 
    
    /**
    * Static callback called via CAsyncCallBack to initiate cleaning of a sessions.
    * @since Series 60 3.0
    * @param aThis ?.
    * @return KErrNone if succesful, otherwise system wide error code.
    */
    static TInt SessionStopCallBack( TAny* aThis ); 
    
    /**
    * Method for cleaning a sessions. Called from StreamStopCallBack.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void DoSessionStop( void ); 
    
    /**
    * Method for deleting RTP clip handler if it is not used in any session.
    * @since Series 60 3.0
    * @param aCurrentSessionIndex a index of active session.
    * @return none.
    */
    void DeleteClipHandler( const TInt aCurrentSessionIndex ); 
    
    /**
    * Reads non unicode descriptor buffer to client address space.
    * @since Series 60 3.0
    * @param aMessage an object which encapsulates a client request.
    * @param aParam a index value identifying the argument.
    * @param aDes a buffer to be read.
    * @return None.
    */
    void Read( const RMessage2& aMessage, 
               const TInt& aParam,
               TDes8& aDes );

    /**
    * Reads unicode descriptor buffer to client address space.
    * @since Series 60 3.0
    * @param aMessage an object which encapsulates a client request.
    * @param aParam a index value identifying the argument.
    * @param aDes a buffer to be read.
    * @return None.
    */
    void Read( const RMessage2& aMessage, 
               const TInt& aParam,
               TDes16& aDes );

    /**
    * Writes non unicode descriptor buffer to client address space.
    * @since Series 60 3.0
    * @param aMessage an object which encapsulates a client request.
    * @param aParam a index value identifying the argument.
    * @param aDes a data to be written.
    * @return None.
    */
    void Write( const RMessage2& aMessage, 
                const TInt& aParam,
                const TDesC8& aDes );

    /**
    * Writes unicode descriptor buffer to client address space.
    * @since Series 60 3.0
    * @param aMessage an object which encapsulates a client request.
    * @param aPtr a pointer to client address space.
    * @param aDes a data to be written.
    * @return None.
    */
    void Write( const RMessage2& aMessage, 
                const TInt& aParam,
                const TDesC16& aDes );

    /**
    * Method for panicing a client.
    * @since Series 60 3.0
    * @param aPanic is the panic reason, essentially a symbian error code.
    * @aMessage is the message causing the panic.
    * @return none.
    */
    void PanicClient( TInt aPanic, 
                      const RMessage2& aMessage );

private: // Data    
    
    /**
    * Name of the queue.
    */
    HBufC* iQueueName;
    
    /**
    * Callstack-cutter for cleanup purpose.
    */
    CAsyncCallBack* iCleanUp;
    
    /**
    * Connection manager.
    */
    CCRConnection* iConnection;
    
    /**
    * Clip handler for RTP clip.
    */
    CRtpClipHandler* iClipHandler;

    /**
    * Sends messages to client's message queue
    */
    CCRClientInformer* iInformer;

    /**
    * Socket server.
    */
    RSocketServ iSockServer;
    
    /**
    * Streamong sessions.
    */
    RPointerArray<CCRStreamingSession> iSessions;
    
    /**
    * Array of sessions that have declared that they want to be deleted
    */
    RPointerArray<CCRStreamingSession> iSessionsToDelete; 
    
    /**
    * Loopback port.
    */
    TInt iLoopbackPort;
    
    };

#endif // CCRENGINE_H

// End of file

