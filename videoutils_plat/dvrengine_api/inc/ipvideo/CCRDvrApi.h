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
* Description:    Interface for Common recording engine, the part of API*
*/




#ifndef CCRDVRAPI_H
#define CCRDVRAPI_H

// INCLUDES
#include <ipvideo/CCRApiBase.h>
#include <ipvideo/CRTypeDefs.h>
#include <es_sock.h>

// CONSTANTS
const TReal KRealZero( 0.0 );
const TReal KRealMinusOne( -1.0 );

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRMsgQueueObserver;
class MCREngineObserver;

// CLASS DECLARATION

/**
*  Ip-LiveTV-specific client api that manages service functionality.
*
*  @lib CommonRecordingEngineClient.lib
*  @since Series 60 3.0
*/
class CCRDvrApi : public CCRApiBase
    {

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    */
    IMPORT_C static CCRDvrApi* NewL( MCREngineObserver *aObserver );

    /**
    * Two-phased constructor.
    */
    IMPORT_C static CCRDvrApi* NewL();

    /**
    * Destructor.
    * @return None.
    */
    IMPORT_C virtual ~CCRDvrApi();

public: // New methods

    /**
    * Tells receiving engine which IAP to use
    * @since Series 60 3.0
    * @param aIapId is handle to RConnection that is already open.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */                        
    IMPORT_C TInt SetIap( const TSubConnectionUniqueId& aIapId );

    /**
    * Tells receiving engine to cancel IAP selection.
    * @since Series 60 3.0
    * @param none.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */                        
    IMPORT_C TInt CancelSetIap();

    /**
    * Prepares RTSP stream for player.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aRtspUrl is set of parameters required for rtsp.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt PlayRtspUrl( TUint& aSessionChk,
                               const SCRRtspParams& aRtspUrl );

    /**
    * Prepares DVB-H live stream for player.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aLiveParams is set of parameters required for DVB-H.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt PlayDvbhStream( TUint& aSessionChk,
                                  const SCRLiveParams& aLiveParams );

    /**
    * Changes DVB-H service of live streaming.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aLiveParams is set of parameters required for DVB-H.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt ChangeDvbhService( TUint& aSessionChk,
                                     const SCRLiveParams& aLiveParams );

    /**
    * Prepares RTP clip stream for player.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aRtpFile is set of parameters required for rtsp.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt PlayRtpFile( TUint& aSessionChk,
                               const SCRRtpPlayParams& aRtpFile );

    /**
    * Prepares RTP clip stream for player.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aRtpHandle a open file handle for RTP file.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt PlayRtpFile( TUint& aSessionChk,
                               const RFile& aRtpHandle );

    /**
    * Starts recording for currently active stream.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aRecordParams a parameters for recording.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt RecordCurrentStream( const TUint aSessionChk,
                                       const SCRRecordParams& aRecordParams );

    /**
    * Starts recording for RTSP live stream.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aRtspUrl is set of parameters required for rtsp.
    * @param aRecordParams a parameters for recording.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt RecordRtspStream( TUint& aSessionChk,
                                    const SCRRtspParams& aRtspUrl,
                                    const SCRRecordParams& aRecordParams );

    /**
    * Starts recording for DVB-H live stream.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aLiveParams is set of parameters required for DVB-H.
    * @param aRecordParams a parameters for recording.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt RecordDvbhStream( TUint& aSessionChk,
                                    const SCRLiveParams& aLiveParams,
                                    const SCRRecordParams& aRecordParams );

    /**
    * Pauses/Resumes recording of wanted stream.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aStart a start or end pausing.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt PauseRecordStream( const TUint aSessionChk,
                                     const TBool& aStart );

    /**
    * Stops recording of wanted stream.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt StopRecordStream( const TUint aSessionChk );

    /**
    * Starts time shifting for wanted stream.
    * @since Series 60 3.0
    * @param aTimeShiftChk a session definition checksum.
    * @param aCurrentChk a session definition of existing session.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt StartTimeShift( TUint& aTimeShiftChk,
                                  const TUint aCurrentChk );

    /**
    * Stops time shifting mode.
    * @since Series 60 3.0
    * @param aTimeShiftChk a session definition checksum.
    * @param aCurrentChk a session definition of existing session.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt StopTimeShift( const TUint aTimeShiftChk,
                                 const TUint aCurrentChk );

    /**
    * Method for ordering "play" for packet source.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aStartPos is start position in seconds.
    * @param aEndPos is end position in seconds.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */                        
    IMPORT_C TInt PlayCommand( const TUint aSessionChk,
                               const TReal aStartPos = KRealMinusOne,
                               const TReal aEndPos = KRealMinusOne );

    /**
    * Method for ordering "pause" for packet source.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */                        
    IMPORT_C TInt PauseCommand( const TUint aSessionChk );

    /**
    * Method for ordering "stop" for packet source.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */                        
    IMPORT_C TInt StopCommand( const TUint aSessionChk );

    /**
    * Setter for play position of packet source.		 
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aPosition a postion of RTP playback.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt SetPosition( const TUint aSessionChk,
                               const TInt64 aPosition );

    /**
    * Getter for play position of packet source.		 
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aPosition a postion of RTP playback.
    * @param aDuration a duration of RTP clip.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt GetPosition( const TUint aSessionChk,
                               TInt64& aPosition,
                               TInt64& aDuration );

    /**
    * Method for ordering closing a source and all sinks.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt CloseSession( const TUint aSessionChk );

    /**
    * Prepares rtp file playing as a live source.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt PlayNullSource( TUint& aSessionChk );

    /**
    * Prepares rtsp stream for null sink.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aRtspUrl is set of parameters required for rtsp
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    IMPORT_C TInt PlayRtspUrlToNullSink( TUint& aSessionChk,
                                         const SCRRtspParams& aRtspUrl );

private: // Constructors and destructor

    /**
    * C++ default constructor.
    */
    CCRDvrApi();

    /**
    * Symbian 2nd phase constructor can leave.
    */
    void ConstructL( MCREngineObserver *aObserver );

    /**
    * Symbian 2nd phase constructor can leave.
    */
    void ConstructL();

private: // Data

    /**
    * Queue observer.
    */
    CCRMsgQueueObserver* iQueueObserver;

    };

#endif // CCRDVRAPI_H

// End of File
