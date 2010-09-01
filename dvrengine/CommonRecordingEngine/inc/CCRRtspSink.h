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
* Description:    Packet sink where real one player gets media stream.*
*/




#ifndef CCRRTSPSINK_H
#define CCRRTSPSINK_H

// INCLUDES
#include "CCRSock.h"
#include "CCRRtspCommand.h"
#include "CCRRtspResponse.h"
#include "CCRPacketBuffer.h"
#include "CCRStreamingSession.h"
#include "CCRPacketSinkBase.h"
#include <e32base.h>

// CONSTANTS
// None.

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CDvrSdpParser; 
class CCRConnection; 
class RSocketServ; 
class CCRStreamingSession; 

// CLASS DECLARATION
/**
*  Packet sink that forwards stream to rop plugin via rtsp.
*  Implements rtsp server. 
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
 */
class CCRRtspSink : public CCRPacketSinkBase,
                    public MCRSockObserver
    {

public:

    /**
    * Enum for 5 socket to use to play to rop plugin.
    */
    enum TCRROPSockId
        {
        EROPControl = 0, /**< control sock, rtsp in tcp spoken here */
        EROPVideoSend1,  /**< RTP video payload */
        EROPVideoSend2,  /**< RTCP video */
        EROPAudioSend1,  /**< RTP audio payload */
        EROPAudioSend2,  /**< RTCP audio */
        EROPMaxSockets   /**< Just max value, no real socket assigned for this */   
        };

public: // Constructors and destructors   

    /**
    * Two-phased constructor.
    * @param aConnection a connection rerefrence.
    * @param aSockServer a socket server refrence.
    * @param aSinkId a id of this sink.
    * @param aOwningSession a reference to owning session.
    * @param aLoopbackPort a RTSP port for loopback socket.
    * @return CCRRtspSink pointer.
    */
    static CCRRtspSink* NewL( CCRConnection& aConnection,
                              RSocketServ& aSockServer,
                              CCRStreamingSession::TCRSinkId aSinkId,
                              const TInt& aLoopbackPort,
                              CCRStreamingSession& aOwningSession );
    
    /**
     * Destructor 
     */
    virtual ~CCRRtspSink();
    
private: // Methods from base classes

    /**
    * From CCRPacketSinkBase.
    * Connects the given buffer with this sink.
    * @since Series 60 3.0
    * @param aBuffer is the buffer to connect with.
    * @return none.
    */
    void SetBuffer( CCRPacketBuffer* aBuffer );
    
    /**
    * From CCRPacketSinkBase.
    * Adds packet to the sink.
    * @since Series 60 3.0
    * @return none.
    */
    void NewPacketAvailable();
    
    /**
    * From CCRPacketSinkBase.
    * Method for setting the sdp in use
    * @since Series 60 3.0
    * @param aSdp is the new sdp
    * @return none
    */
    void SetSdpL( const TDesC8& aSdp );
    
    /**
    * From CCRPacketSinkBase.
    * Method for setting initial seq+ts _before_ the stream is played.
    * @since Series 60 3.0
    * @param aAudioSeq is the initial rtp seq number for audio packets.
    * @param aAudioTS  is the initial rtp timestamp number for audio packets.
    * @param aVideoSeq is the initial rtp seq number for video packets.
    * @param aVideoTS  is the initial rtp timestamp number for video packets.
    * @return none.
    */
    void SetSeqAndTS( TUint& aAudioSeq,
                      TUint& aAudioTS,
                      TUint& aVideoSeq,
                      TUint& aVideoTS );

    /**
    * From CCRPacketSinkBase.
    * method for setting play range before the stream is played.
    * @since Series 60 3.0
    * @param aLower is where clip range begins, in seconds.
    * @param aUpper is where clip range ends, usually clip end, in seconds.
    * @return none.
    */
    void SetRange( TReal aLower,
                   TReal aUpper );    
    
    /**
    * From CCRPacketSinkBase.
    * Method that source uses to communicate its status.
    * @since Series 60 3.0
    * @param aNewState is the new state of the source.
    * @return none.
    */
    void StatusChanged( MCRPacketSource::TCRPacketSourceState aNewState );
    
    /**
    * From MCRSockObserver.
    * This method is called after some data has been received from socket.
    * @since Series 60 3.0
    * @param aData is descriptor containing the data received. 
    *        ownership of data is not passed via this call. 
    * @return none.
    */
    void DataReceived( TInt aSockId, const TDesC8 &aData );

    /**
    * From MCRSockObserver.
    * This method is called after status of socket changes.
    * @param aSockId 
    * @param aStatus is sock status.
    * @param aError
    * @return none.
    */
    void SockStatusChange( TInt aSockId,
                           CCRSock::TCRSockStatus aStatus,
                           TInt aError );
    
public: // Constructors and destructors   

    /**
    * By default default constructor is private
    * @param aConnection is the IAP representative that we need to use
    * @param aSockServer is the socketserver to use when opening socks
    * @param aSinkId that this class needs to use when reporting progress back to owner
    * @param aOwningSession is the streaming session that owns this instance
    */
    CCRRtspSink( CCRConnection& aConnection,
                 RSocketServ& aSockServer,
                 CCRStreamingSession::TCRSinkId aSinkId,
                 CCRStreamingSession& aOwningSession );

    /**
    * second phase of 2-phased construction
    * @param aLoopbackPort a RTSP port for loopback socket.
    */
    void ConstructL( const TInt& aLoopbackPort ); 

protected: // New methods

    /**
    * Method that produces sdp for rop plugin consumption. 
    * Needs to have iSDP and parser. 
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void ProduceSDPForRopL(); 

    /**
    * Method for doing something with commands rop has sent to us
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void ProcessRtspCommandL();  

    /**
    * Method for doing something after rop has said something to use.
    * @since Series 60 3.0
    * @param aLastCommand is the last command rop has sent
    * @return none.
    */
    void ProduceRtspReplyL( CCRRtspCommand::TCommand aLastCommand );  
    
    /**
    * Method for replying to "options" command.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void ReplyToOptionsL();

    /**
    * Method for replying to "describe" command.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void ReplyToDescribeL();

    /**
    * Method for replying to "setup" command
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void ReplyToSetupL();

    /**
    * Method for replying to "play" command, reply is sent to rop.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void ReplyToPlayL();

    /**
    * Method for replying to "pause" command.
    * @since Series 60 3.0
    * @param symbian error code telling if pausing was ok.
    * @return none.
    */
    void ReplyToPauseL( TInt aErrorCode );

    /**
    * Method for replying to "teardown" command.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void ReplyToTearDownL();

private: // New methods

    /**
    * Getter for socket sink id basing on stream.
    * @since Series 60 3.0
    * @param aStreamId a stream id of packet.
    * @return a socket sink id.
    */
    TCRROPSockId SocketFromStream( MCRPacketSource::TCRPacketStreamId aStreamId );
    
    /**
    * Appends string to the buffer.
    * @since Series 60 3.0
    * @param aBuffer a buffer where to add text.
    * @param aTxt a string to append.
    * @return none.
    */
    void AppendL( HBufC8*& aBuffer,
                  const TDesC8& aTxt );

    /**
    * Appends integer value as text to the buffer.
    * @since Series 60 3.0
    * @param aBuffer a buffer where to add text.
    * @param aNum a numerical value to append.
    * @return none.
    */
    void AppendNumL( HBufC8*& aBuffer,
                     const TInt aNum );

    /**
    * Appends formatted text to the buffer.
    * @since Series 60 3.0
    * @param aBuffer a buffer where to add text.
    * @param aFmt a format string to use.
    * @return none.
    */
    void AppendFormatL( HBufC8*& aBuffer,
                        TRefByValue<const TDesC8> aFmt, ... );

    /**
    * Sends control data to the player.
    * @since Series 60 3.0
    * @param none. 
    * @return none.
    */
    void SendControlData();

    /**
    * This meyhod is called after received data from EROPControl socket.
    * @since Series 60 3.1
    * @param aData is descriptor containing the data received. 
    */
    void HandleReceivedEROPControlL( const TDesC8& aData );

private: // Data

    /**
    * RConnection wrapper we use for sockets. no ownership. 
    */
    CCRConnection& iConnection;

    /**
    * Socket server we use for sockets. no ownership. 
    */
    RSocketServ& iSockServer;

    /**
    * When TBool in iPacketPendingInBuffer is ETrue it means that the buffer
    * in iBuffers having same array index. As said TBool has a packet 
    * waiting to be sent to rop.
    */ 
    TBool iPacketPendingInBuffer;
    
    /**
    * Have our own sdp parser for parsing the sdp.
    */
    CDvrSdpParser* iSdpParser;

    /**
    * SDP to give to rop.
    */
    HBufC8* iSdpForRop;

    /**
    * Sockets for speaking with rop.
    */
    CCRSock* iSockArr[EROPMaxSockets]; 
    
    /** 
    * Stage in rtsp discussions with rop plugin.
    */
    TCRRTSPStage iStage; 
    
    /** 
    * Stage for setup repply received.
    */
    TInt iSetupReceived;
    
    /**
    * Pointer to data sent to us via socket from rop.
    */
    HBufC8* iReceivedData;
    
    /** 
    * Pointers to commands sent by rop in various stages.
    */
    CCRRtspCommand *iCommands[CCRRtspCommand::ERTSPCommandNOCOMMAND];
    
    /** 
    * Pointer to string sent to rop.
    */
    HBufC8* iRopResponse; 

    /** 
    * Audio seq to begin with.
    */
    TUint32 iAudioSeq;
    
    /**
    * Audio TS to begin with.
    */
    TUint32 iAudioTS;
    
    /**
    * Video seq to begin with.
    */  
    TUint32 iVideoSeq;
    
    /**
    * Video TS to begin with
    */
    TUint32 iVideoTS;
    
    /**
    * TBool telling if above 4 things (seq+ts*2) has been seen.
    */
    TBool iSeqAndTSSet;
    
    /**
    * Range begin pos in play command header.
    */
    TReal iLowerRange; 
    
    /**
    * Range end pos in play command header.
    */
    TReal iUpperRange; 
    
    };

#endif // CCRRTSPSINK_H
