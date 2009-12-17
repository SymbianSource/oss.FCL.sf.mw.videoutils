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
* Description:    Class for wrapping RConnection*
*/




#ifndef __CCRCONNECTION_H
#define __CCRCONNECTION_H

// INCLUDES
#include "MCRConnectionObserver.h"
#include <e32base.h>
#include <es_sock.h>
#include <CommDbConnPref.h>
#include <es_enum.h>
#include <rconnmon.h>

// CONSTANTS
/** Max Bandwidth constants -- from Helix Player, R1_Mobile_4_0_Factory.cfg : GRPS */
const TInt KCRBandwidthGPRS( 40200 );
/** Max Bandwidth constants -- from Helix Player, R1_Mobile_4_0_Factory.cfg : EGRPS */
const TInt KCRBandwidthEdgeGPRS( 89600 );
/** Max Bandwidth constants -- from Helix Player, R1_Mobile_4_0_Factory.cfg : 3G */
const TInt KCRBandwidthWCDMA( 384000 );
/** Max Bandwidth constants -- from Helix Player, R1_Mobile_4_0_Factory.cfg : WLAN */
const TInt KCRBandwidthWLAN( 1432572 );
/** Max Bandwidth constants -- from Helix Player, R1_Mobile_4_0_Factory.cfg : LAN */
const TInt KCRBandwidthLAN( 3000000 );

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRConnection;

// CLASS DECLARATION

/**
* RConnection wrapper
*/
class CCRConnection : public CActive,
                      public MConnectionMonitorObserver
    {

public: // Data types

    /**
    * Enumeration for different connection stages
    */
    enum TConnectionState
        {
        EIdle,          /**< Connection idle */
        EFindingAP,     /**< Search access point */
        EFindingBearer, /**< Finding bearer */
        EOpen,          /**< Opening connection */
        EConnecting     /**< Connecting */
        };

    /** 
    * Heuristics for the connection, ie, characteristics from the
    * connection discovered during run-time and that may affect it's usage.
    */
    enum TConnectionHeuristic
        {
        /**
        * Connection is unable to stream RTP over UDP,
        * most likely due to firewall blocking.
        */
        EUdpStreamingBlocked = 0
        };

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @param aSockServer 
    * @return CCRConnection pointer to CCRConnection class
    */
    static CCRConnection* NewL( RSocketServ& aSockServer );

    /**
    * Destructor.
    */
    virtual ~CCRConnection( );

public: // New methods
	/**
    * Method for starting and attaching a connection.
    * @since Series 60 3.0
    * @param aConnectionId Connection id.
    * @return none.
    */
    void Attach( TUint aConnectionId ); 

    /**
    * Method for getting the rconnection to use. This is useful only
    * when connection state is open. 
    * @since Series 60 3.0
    * @param none.
    * @return reference to RConnection.
    */
    RConnection& Connection( void ); 
    
    /**
    * Method for getting state of the connection.
    * @since Series 60 3.0
    * @param none.
    * @return connection state.
    */
    TConnectionState State( void ) const;

    /**
    * Gets bearer for current connection.
    * @since Series 60 3.0
    * @param none.
    * @return bearer type
    */
    TConnMonBearerType BearerType() const;

    /**
    * Gets (estimated) maximum available connection bandwidth.
    * @since Series 60 3.0
    * @param none.
    * @return bandwidth in bit/s, zero if unkown or connection not available.
    */
    TInt MaximumBandwidth();

    /**
    * Check if bearer is considered 3G, WLAN or similar
    * @since Series 60 3.0
    * @param none.
    * @return TBool
    */
    TBool IsBearerWLANor3G( TConnMonBearerType aBearer );

    /**
    * Add observer for connection status.
    * @since Series 60 3.0
    * @param aObserver new observer
    * @return TInt
    */
    TInt RegisterObserver( MCRConnectionObserver* aObserver );

    /**
    * remove observer for connection status.
    * @since Series 60 3.0
    * @param aObserver observer to be removed.
    * @return TInt.
    */
    TInt UnregisterObserver( MCRConnectionObserver* aObserver );

    /** 
    * Retrives a heuristic associated with the connection.
    * @since Series 60 3.0
    * @param aHeuristic 
    * @return value associated with the heuristic.
    */
    TBool GetHeuristic( TConnectionHeuristic aHeuristic );

    /** 
    * Sets a heuristic associated with the connection
    * @since Series 60 3.0
    * @param aHeuristic
    * @param aValue value associated with the heuristic.
    * @return none.
    */
    void SetHeuristic( TConnectionHeuristic aHeuristic,
                       TBool aValue );

private: // Methods from base classes
    
    /**
    * From CActive.
    * Cancel protocol implementation.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void DoCancel();

    /**
    * From CActive.
    * Outstanding request completed.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void RunL();

    /**
    * From CActive.
    * Handle error.
    * @since Series 60 3.0
    * @param aError Error code.
    * @return KErrNone.
    */
    TInt RunError( TInt aError );

    /**
    * From MConnectionMonitorObserver.
    * Receives event notifications from connection monitor,
    * specifically for bearer changes.
    * @since Series 60 3.0
    * @param aConnMonEvent
    * @return none.
    */
    void EventL( const CConnMonEventBase& aConnMonEvent );

private: // New methods
    
    /**
    * Method for sending connection error events to message queue.
    * @since Series 60 3.0
    * @param aError is Error code.
    * @return none
    */
    void SendConnectionErrorToQueue( TInt aError ); 

	/**
	* Close internal RConnection iConnection.
    * @since Series 60 3.0
    * @param none.
    * @return none
	*/
	void CloseRConnection();
    
    /**
    * Finds access point.
    * @since Series 60 3.0
    * @param none.
    * @return none
    */
    TInt FindApL();
    
    /**
    * Finds bearer.
    * @since Series 60 3.0
    * @param none.
    * @return none
    */
    void FindBearerL();
    
    /**
    * Notify UI about connection.
    * @since Series 60 3.0
    * @param none.
    * @return none
    */
    void NotificationL();
    
private: // Constructors and destructors
    
    /**
    * default constructor
    * @param aSockServer a reference to socket server.
    */
    CCRConnection( RSocketServ& aSockServer );
     
    /** 
    * 2nd phase constructor 
    */
    void ConstructL();

private: // Data

    /**
    * Socker server.
    */
    RSocketServ& iSockServer;

    /**
    * Connection.
    */
    RConnection iConnection;
    
    /**
    * Observers.
    */
    RPointerArray<MCRConnectionObserver> iObservers;
    
    /**
    * Current state.
    */
    TConnectionState iState; 
    
    /**
    * Connection preference.
    */
    TCommDbConnPref iConnPref;
    
    /**
    * variable for getting notification when interface goes up or down
    */
    TNifProgressBuf iNotification;

    /**
    * Connection id to be used
    */
    TUint iCurrentConnectionId;

    /**
    * Connection monitor
    */
    RConnectionMonitor iConMon;

    /**
    * How many active connections were found by connection monitor
    */
    TUint iConnectionCount;

    /**
    * Heuristics for connection
    */
    TUint32 iHeuristics;

    /**
    * Bearer type for current connection
    */
    TConnMonBearerType iBearerType;

    /**
    * Bearer type detected after change
    */
    TConnMonBearerType iNewBearerType;
    
    /**
    * Whether we have connection progress notification pending or not
    */
    TBool iConMonProgressNotifyPending ; 

    };

#endif // __CCRCONNECTION_H

// End of file
