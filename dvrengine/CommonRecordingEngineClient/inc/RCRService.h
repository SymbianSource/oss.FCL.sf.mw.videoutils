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
* Description:    Interface for RC Client's Service class*
*/




#ifndef RCRSERVICE_H
#define RCRSERVICE_H

// INCLUDES
#include "RCRServiceBase.h"
#include <es_sock.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class RCRClient;

// CLASS DECLARATION

/**
*  This is the actual client API that passes commands
*  to server side. CCRDvrApi and other classes visible
*  to client application are merely wrappers to this
*  class. If CCRDvrApi has method foo, it will just
*  call method foo if this class here and this will in 
*  turn say something like return SendReceive( ECRDoFooInServer, args );
*  This needs to have implementations of all the engine
*  methods of all client classes.
*
*  @lib CommonRecordingEngineClient.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( RCRService ) : public RCRServiceBase
    {
    
public: // Constructors and destructor
    
    /**
    * Constructor.
    */
    RCRService();

public: // New methods

	/**
	* Tells receiving engine which IAP to use.
    * @since Series 60 3.0
	* @param aIapId is the internet access point id. 
    * @return none.
	*/						  
	TInt SetIap( const TSubConnectionUniqueId& aIapId );

	/**
	* Tells receiving engine to cancel IAP selection.
    * @since Series 60 3.0
	*/						  
	TInt CancelSetIap();

    /**
    * Prepares live IPTV PTSP stream.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aRtspParams parameter set for rtsp.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt PlayRtspUrl( TUint& aSessionChk,
                      const SCRRtspParams& aRtspParams );

    /**
    * Prepares DVB-H live stream.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aLiveParams parameter set for DVB-H.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt PlayDvbhStream( TUint& aSessionChk,
                         const SCRLiveParams& aLiveParams );

    /**
    * Changes DVB-H live stream service.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aLiveParams parameter set for DVB-H.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt ChangeDvbhService( TUint& aSessionChk,
                            const SCRLiveParams& aLiveParams );

    /**
    * Prepares clip RTP stream from a name.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aRtpFile parameter set for rtsp.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt PlayRtpFile( TUint& aSessionChk,
                      const SCRRtpPlayParams& aRtpFile );

    /**
    * Prepares clip RTP stream from a file handle.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aRtpHandle a open file handle for RTP file.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt PlayRtpFile( TUint& aSessionChk,
                      const RFile& aRtpHandle );

    /**
    * Starts currently active stream recording.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aRecordParams a parameters for recording.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt RecordCurrentStream( const TUint aSessionChk,
                              const SCRRecordParams& aRecordParams );
    
    /**
    * Starts recording for RTSP live stream.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aRtspUrl is set of parameters required for rtsp.
    * @param aRecordParams a parameters for recording.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt RecordRtspStream( TUint& aSessionChk,
                           const SCRRtspParams& aRtspUrl,
                           const SCRRecordParams& aRecordParams );
    
    /**
    * Starts DVB-H recording.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aLiveParams is set of parameters required for DVB-H.
    * @param aRecordParams a parameters for recording.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt RecordDvbhStream( TUint& aSessionChk,
                           const SCRLiveParams& aLiveParams,
                           const SCRRecordParams& aRecordParams );
    
    /**
    * Pauses/Resumes recording.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aStart a start or end pausing.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt PauseRecordStream( const TUint aSessionChk,
                            const TBool& aStart );

    /**
    * Stops recording.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt StopRecordStream( const TUint aSessionChk );

    /**
    * Starts time shift mode.
    * @since Series 60 3.0
    * @param aTimeShiftChk a session definition checksum.
    * @param aSessionChk a session definition of existing session.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt StartTimeShift( TUint& aTimeShiftChk,
                         const TUint aSessionChk );

    /**
    * Stops time shift mode.
    * @since Series 60 3.0
    * @param aTimeShiftChk a session definition checksum.
    * @param aCurrentChk a session definition of existing session.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt StopTimeShift( const TUint aTimeShiftChk,
                        const TUint aCurrentChk );

    /**
    * Method for ordering "play" for packet source.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aStartPos is start position in seconds.
    * @param aEndPos is end position in seconds.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt PlayCommand( const TUint aSessionChk,
                      const TReal& aStartPos,
                      const TReal& aEndPos );

    /**
    * Method for ordering "pause" for packet source.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt PauseCommand( const TUint aSessionChk );

    /**
    * Method for ordering "stop" for packet source.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt StopCommand( const TUint aSessionChk );

    /**
    * Setter for play position of packet source.		 
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aPosition a postion of RTP playback.
    * @param aDuration a duration of RTP clip.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt SetPosition( const TUint aSessionChk,
                      const TInt64 aPosition );

    /**
    * Getter for play position of packet source.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aPosition a postion of RTP playback.
    * @param aDuration a duration of RTP clip.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt GetPosition( const TUint aSessionChk,
                      TInt64& aPosition,
                      TInt64& aDuration );

    /**
    * Method for ordering closing a source and all sinks.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if ok, otherwise system-wide error codes.
    */
    TInt CloseSession( const TUint aSessionChk );

    /**
    * Prepares rtp file playing as a live source.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt PlayNullSource( TUint& aSessionChk );

    /**
    * Prepares live IPTV PTSP stream to null sink.
    * @since Series 60 3.0
    * @param aSessionChk a session definition checksum.
    * @param aRtspParams a active live parameters.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide  error codes.
    */
    TInt PlayRtspUrlToNullSink( TUint& aSessionChk,
                                const SCRRtspParams& aRtspParams );
    };

#endif // RCRSERVICE_H

// End of File
