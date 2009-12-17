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




#ifndef CCRNULLSINK_H
#define CCRNULLSINK_H

// INCLUDES
#include "CCRPacketSinkBase.h"
#include "CCRPacketBuffer.h"
#include "MCRTimerObserver.h"
#include <e32base.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRConnection; 
class RSocketServ; 
class CCRStreamingSession; 
class CCRTimer;
class MCRConnectionObserver;

// CLASS DECLARATION

/**
*  Packet sink that does not forward packets. Good for testing. 
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRNullSink : public CCRPacketSinkBase,
                    public MCRTimerObserver
    {

public: // Constructors and destructors
    
    /**
    * Two-phased constructor.
    * @return CCRNullSink pointer. 
    */
    static CCRNullSink* NewL( CCRStreamingSession::TCRSinkId aSinkId,
                              CCRStreamingSession& aOwningSession );
    
    /**
    * Destructor 
    */
    virtual ~CCRNullSink();

public: // Public methods, testing purpose
    /**
    * Register a test observer to receive streaming information
    * @param aObserver pointer to observer
    */
    void RegisterConnectionObs( MCRConnectionObserver* aObserver );

    /**
    * Method that source uses to communicate its status.
    * @since Series 60 3.0
    * @param aNewState is the new state of the source.
    * @return none.
    */
    virtual void StatusChanged( MCRPacketSource::TCRPacketSourceState aNewState );

protected: // Constructors and destructors

    /**
    * By default default constructor is private
    * @param aSinkId that this class needs to use when reporting progress back to owner
    * @param aOwningSession is the streaming session that owns this instance
    */
    CCRNullSink( CCRStreamingSession::TCRSinkId aSinkId,
                 CCRStreamingSession& aOwningSession );

    /**
    * Second phase of 2-phased construction
    */
    void ConstructL(); 

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
    * From MCRTimerObserver.
    * The function to be called when a timeout occurs.
    * @since Series 60 3.0
    * @param aTimer a pointer to timer.
    * @return none.
    */
    void TimerExpired( CCRTimer* aTimer );

private: // Data

    /**
    * SDP in use.
    */
    HBufC8* iSdp; 

    /**
    * Flow control timer.
    */
    CCRTimer* iFlowTimer;
    
    /**
    * when TBool in iPacketPendingInBuffer is ETrue  it means that the buffer
    * in iBuffer having same array index as said TBool has a packet.
    * waiting to be sent to rop
    */ 
    TBool iPacketPendingInBuffer; 

    /**
    * Test client observer
    */
    MCRConnectionObserver* iObserver;
    };

#endif // CCRNULLSINK_H

//  End of File
