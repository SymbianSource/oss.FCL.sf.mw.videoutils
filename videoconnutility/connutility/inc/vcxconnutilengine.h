/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    class to handle networking functions in vcxconnectionutility*
*/




#ifndef __CVCX_CONUTILENGINE_H__
#define __CVCX_CONUTILENGINE_H__

#include <e32base.h>

#include <e32std.h>
#include <cmmanager.h>
#include <es_sock.h>
#include <connpref.h>
#include <commdbconnpref.h>
#include <comms-infras/cs_mobility_apiext.h>

#include <cmdestinationext.h>
#include <cmmanagerext.h>
#include <rconnmon.h>
#include "vcxconnectionutility.hrh"

class CActiveCommsMobilityApiExt;
class MConnUtilExtEngineObserver;


// CLASS DECLARATION
/**
* Class defines functionality for handling networking functions in 
* vcxconnectionutility. 
* These are:
* 1. connection creation and disconnecting
* 2. handling and notifying of events from apllication level roaming (ALR)
* 3. handling and notifying of other interesting events from the network:
*  - link layer closing and connection deletion
*  - iap list changing
*
* Class uses MConnUtilEngineObserver interface to inform CVcxConnUtilImpl 
* -class about the events.  
*/

NONSHARABLE_CLASS (  CVcxConnUtilEngine ) : public CActive,
                                            public MMobilityProtocolResp,
                                            public MConnectionMonitorObserver
    {   
    public: // construction / destruction
    
        /**
         * Construction.
         * 
         * @param MConnUtilExtEngineObserver
         * @return The newly constructed CVcxConnUtilEngine
         */
        static CVcxConnUtilEngine* NewL( MConnUtilExtEngineObserver* aObserver );
    
        /**
         * Construction. Object left to cleanupstack
         * 
         * @param MConnUtilExtEngineObserver
         * @return The newly constructed CVcxConnUtilEngine
         */
        static CVcxConnUtilEngine* NewLC( MConnUtilExtEngineObserver* aObserver );
        
        /**
         * destructor
         */
        virtual ~CVcxConnUtilEngine();
        
    private:
            
        /**
         * C++ constructor
         */
        CVcxConnUtilEngine(); 
            
        /**
         * Private 2nd phase construction.
         */
        void ConstructL();  
        
    private: // From CActive
            
        /**
         * From CActive. Connection startup is done asynchronously after 
         * connection cretaion ends, the status value of the connection is 
         * updated whether connection succeed or not
         */  
        void RunL();
        
        /**
         * From CActive. Cancels coonnection creation.
         */
        void DoCancel();

    private: // From MMobilityProtocolResp
    
        /**
         * From MMobilityProtocolResp. We have better carrier available for
         * the active connection.
         * 
         * @param aOldAPInfo    current IAP information
         * @param aNewAPInfo    suggested new IAP information
         * @param aIsUpgrade    ETrue indicates IAP is to be upgraded
         * @param aIsSeamless   ETrue indicates that new connection is seamless
         *                      (new IAP requires dialog for connection)
         */ 
        void PreferredCarrierAvailable( TAccessPointInfo aOldAPInfo,
                                        TAccessPointInfo aNewAPInfo,
                                        TBool aIsUpgrade,
                                        TBool aIsSeamless );
         
        /**
         * From MMobilityProtocolResp. We have a new carrier available for
         * the active connection.
         * 
         * @param aNewAPInfo    IAP information from the new available connection
         * @param aIsSeamless   ETrue indicates that new connection is seamless
         *                      (new IAP requires dialog for connection)
         */ 
        void NewCarrierActive( TAccessPointInfo aNewAPInfo, TBool aIsSeamless );
          
        /**
         * From MMobilityProtocolResp.
         * 
         * @param aError
         */
        void Error( TInt aError );
        
    private: // from MConnectionMonitorObserver
        
        /**
         * @see MConnectionMonitorObserver
         */
        void EventL( const CConnMonEventBase &aConnMonEvent );
    
    public: // new methods

        typedef RArray<TUint32> RDestinationArray;
        
        /**
         * Starts a connection via SNAP
         * 
         * @param aPref         connection information to be used connection startup
         * @return              KErrNone always is async connection and if sync connection
         *                      startup succeed or one of the system wide error codes
         */ 
        TInt StartToConnect( TConnPref& aPref,  TBool aConnAsync = ETrue ); 
        
        /**
        * Disconnects active connection and closes RConnection
        */
        void Disconnect();
  
        /**
         * Method is used to get the name of the destination that matches 
         * given id and to make sure, that destination really have connection
         * methods to use. 
         * 
         * @param aDestinationID destination (SNAP) if from where to fetch infromation
         * 
         * @return TBool true, if connection is ok to start 
         */ 
        TBool PrepareConnectionDetailsL( TInt aDestinationID );
        
        /**
         * Method reads platform's connection settings, and returns a destination id.
         * If aPurpose is setted as ESnapPurposeUnknown, default id is returned,
         * otherwise purpose is used to get correct id 
         * 
         * @param aPurpose if ESnapPurposeUnknown, get default, in all other cases
         *                 try to fetch id based on the purpose
         * 
         * @return TUint32 destination id 
         */ 
        TUint32 GetCmmDestinationIdL( CMManager::TSnapPurpose aPurpose = 
                                          CMManager::ESnapPurposeUnknown );
        
        /**
         * Method checks if there is an active connection for
         * the given iap
         * 
         * @param aIapId   iap id to check
         * 
         * @return TBool   ETrue if connection exists
         * 
         */ 
        TBool IsIapConnected( TUint32 aIapId );
        
        /**
         * Method fills started connection's information: IAP id (iIapId) and IAP name
         * This is needed in case of "always ask" to get the information client requires.
         * 
         */ 
        void FillActiveConnectionDetailsL();        
        
        /**
         * clears iapid, snapid and their names
         * 
         */
        void ResetConnectionInfo();
       
        /**
         * Returns currently fetched iap id
         * 
         * @return TUint32
         */
        TUint32 IapId() const;
        
        /**
         * Returns currently fetched iap id
         * 
         * @return TInt
         */
        TInt DestinationId() const;
        
        /**
         * Returns currently fetched snap name
         * 
         * @return TUint32
         */
         TPtrC DestinationName() const;
                 
        /**
         * Returns current connection status
         * 
         * @return TPSVCConnectionStatus
         */
         TVCxConnectionStatus ConnectionStatus() const;
        
        /**
         * Explicitly sets internal connection status.          
         * 
         * @param TPSVCConnectionStatus
         */
        void SetConnectionStatus( TVCxConnectionStatus aStatus );
               
        /**
         * Method can be called to create mobility object
         * for getting roaming events from the network.          
         * 
         * @return TInt KErrnone if object created or already exists. Otherwise a
         *                       system level error. 
         */
        TInt CreateMobility();

        /**
         * Returns "query connection" status.
         */
        TBool QueryConn();

        /**
         * Set "query connection" status.
		 *
		 * @param aQueryConn 	Value for connection query flag.
         */
        void SetQueryConn( TBool aQueryConn );

        
    private:
        
        /**
         * Internal helper method trying to fetch destination
         * information for the current IAP.
         * 
         */
        void FillDestinationInfoL();
        
    private:
        
        /**
         * Socket Server
         */
        RSocketServ iSocketServer;

        /**
         * Connection
         */
        RConnection iConnection;

        /**
         * Connection monitor
         */
        RConnectionMonitor iConnectionMonitor;
        
        /**
         * Current connection state
         */
        TVCxConnectionStatus iConnectionState;
        
        /**
         * Connection Manager Extensions
         */
        RCmManagerExt  iCmManagerExt;

        /**
         * Destination id.
         */
        TInt iDestinationId;
        
        /**
         * IAP ID.
         */
        TUint32 iIapId;

        /**
         * Destination (snap) Name.
         */
        HBufC* iDestinationName;
              
        /**
         * current connection id
         */
        TUint iCurrentConnectionId;
        
        /**
         * Mobility object to fetch carrier change events 
         */
        CActiveCommsMobilityApiExt* iMobility;
        
        /**
         * array of all available destination ids
         */
        RDestinationArray iDestinationArray;
        
        /**
         * observer to notify about changes in the network 
         */
        MConnUtilExtEngineObserver* iObserver;
        
        /**
         * Value to pass connection result error code from the 
         * async connection creation
         */
        TInt iConnectingResult;
        
        /**
         * If is set to ETrue, connection selection dialog will be displayed.
         * Used when tried SNAP contained invalid IAP. 
         */
        TBool iQueryConn;
        
    };

#endif // __CVCX_CONUTILENGINE_H__
