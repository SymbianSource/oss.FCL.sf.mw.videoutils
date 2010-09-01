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
* Description:    Packet sink where XPS gets the streaming.*
*/



#ifndef CCRXPSSINK_H
#define CCRXPSSINK_H

// INCLUDES
#include "CCRPacketSinkBase.h"
#include "MCRTimerObserver.h"
#include "VideoServiceUtilsConf.hrh"
#include <CXPSPktSinkObserver.h>
#include <RtpDef.h>
#include "videoserviceutilsLogger.h"
#ifdef VIA_FEA_IPTV_USE_IPDC
#include <H264Mpeg4GenrToFileFormat.h>
#endif // VIA_FEA_IPTV_USE_IPDC

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRPacketBuffer;
class CXPSPacketSink;
class CCRStreamingSession;
class CRtpTsConverter;
class CRtpPacket;
class CCRTimer;

// CLASS DECLARATION

/**
*  Packet sink that does not forward packets. Good for testing. 
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRXpsSink : public CCRPacketSinkBase,
                   public MCRTimerObserver,
                   public MXPSPktSinkObserver
    {

public: // Constructors and destructors   
    
    /**
    * Two-phased constructor.
    * @return CCRXpsSink pointer. 
    */
    static CCRXpsSink* NewL( CCRStreamingSession::TCRSinkId aSinkId,
                             CCRStreamingSession& aOwningSession );
    
    /**
    * Destructor 
    */
    virtual ~CCRXpsSink();
    
protected: // Constructors and destructors

    /**
    * By default default constructor is private
    * @param aConnection is the IAP representative that we need to use
    * @param aSockServer is the socketserver to use when opening socks
    * @param aSinkId that this class needs to use when reporting progress back to owner
    * @param aOwningSession is the streaming session that owns this instance
    */
    CCRXpsSink( CCRStreamingSession::TCRSinkId aSinkId,
                CCRStreamingSession& aOwningSession );

    /**
    * Second phase of 2-phased construction
    */
    void ConstructL(); 

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
    * From MCRTimerObserver.
    * The function to be called when a timeout occurs.
    * @since Series 60 3.0
    * @param aTimer a pointer to timer.
    * @return none.
    */
    void TimerExpired( CCRTimer* aTimer );

    /**
    * From MXPSPktSinkObserver.
    * API to inform for resumption of packet supply.
    * @since Series 60 3.0
    * @param uStreamId a stream id.
    * @return none.
    */
    void RestorePacketSupply( TUint uStreamId );

private: // New methods

    /**
    * Restores sink for new packets.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void RestoreSink();

    /**
    * Sends one RTP packet to a player.
    * @since Series 60 3.0
    * @param none.
    * @return ETrue if packet was sent, EFalse if not.
    */
    TBool SendPacket();

    /**
    * Adjust time stamp and enque audio packet to player.
    * @since Series 60 3.0
    * @param aPacket a packet payload to send.
    * @return a system wide error code of sending.
    */
    TInt SendAudioPacket( const TDesC8& aPacket );

    /**
    * Adjust time stamp and enque video packet to player.
    * @since Series 60 3.0
    * @param aPacket a packet payload to send.
    * @return a system wide error code of sending.
    */
    TInt SendVideoPacket( const TDesC8& aPacket );

    /**
    * Adjust time stamp and enque sub title packet to player.
    * @since Series 60 3.0
    * @param aPacket a packet payload to send.
    * @return a system wide error code of sending.
    */
    TInt SendTitlePacket( const TDesC8& aPacket );

    /**
    * Sends packet to the player.
    * @since Series 60 3.0
    * @param aStreamId a stream id of packet.
    * @return a system wide error code of sending.
    */
    TInt EnqueuePacket( const TUint aStreamId );
    
    /**
     * Serach if buffer contains control stream packets.
     * If not, in non continous stream ask more packets to buffer.  
     * 
     * @since S60 v3.0
     * @return none.
     */
    void SearchForControlStreamPackets();

    /**
     * Checks if the buffer contains control stream packets. This is needed
     * if we receive packets for stream that has not yet received any control
     * stream packets, so that we do not have to discard the packets. If any
     * control stream packets are found for a stream that has not been 
     * initiated, then that control stream packet is used to initiate that
     * stream.
     * 
     * @since S60 v3.0
     * @return ETrue if a control stream packet was found, EFalse othervise.
     */
    TBool CheckBufferForControlStreamPackets();

    /**
    * Starts flow timer.
    * @since Series 60 3.0
    * @param aInterval a timer interval.
    * @return none.
    */
    void StartTimer( const TInt& aInterval );
    
    /**
    * Starts flow timer.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void StopTimer();
    
#ifdef VIA_FEA_IPTV_USE_IPDC
    /**
    * Serach for fmtp string.
    * @since Series 60 3.0
    * @param aSdpData a SDP data.
    * @return smtp string from SDP.
    */
    HBufC8* FindFmtpLC( const TDesC8& aSdpData );

#endif // VIA_FEA_IPTV_USE_IPDC

private: // Data

    /**
    * XPS packet sink.
    */
    CXPSPacketSink* iPacketSink;
    
    /**
    * Status of XPS buffer.
    */
    TInt iWaitPlayer;
    
    /**
    * Requested packet after overflow.
    */
    TInt iRequested;

    /**
    * Status for XPS reset.
    */
    TBool iXpsResetOk;
    
    /**
    * Status of range.
    */
    TBool iRangeKnown;

    /**
    * Flow control timer.
    */
    CCRTimer* iFlowTimer;
    
    /**
    * Audio stream id for XPS.
    */
    TInt iAudioStreamId;
    
    /**
    * Audio stream id for XPS.
    */
    TInt iVideoStreamId;
    
    /**
    * Sub title stream id for XPS.
    */
    TInt iTitleStreamId;
    
    /**
    * TS converter for audio.
    */
    CRtpTsConverter* iAudioConv;

    /**
    * TS converter for video.
    */
    CRtpTsConverter* iVideoConv;
    
    /**
    * TS converter for video.
    */
    CRtpTsConverter* iTitleConv;
    
    /**
    * RTP packet to enqueue.
    */
    CRtpPacket* iRtpPacket;
    
#ifdef VIA_FEA_IPTV_USE_IPDC
    /**
    * Depacketizer for video stream (ISMACryp).
    */
    CH264Mpeg4GenrToFileformat * iVideoDepacketizer;

#endif // VIA_FEA_IPTV_USE_IPDC

#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    /**
    * Debug purposes.
    */
    TInt iLogXps;

#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE

    };

#endif // CCRXPSSINK_H

//  End of File
