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
* Description:    Implementation Class to handle connection creation.*
*/




#ifndef __VCX_CONNUTILIMPL_H_
#define __VCX_CONNUTILIMPL_H_

// INCLUDES
#include <e32base.h>
#include <e32property.h>

#include <agentdialog.h>  

#include "vcxconnutilpubsubobserver.h"
#include "vcxconnutilextengineobserver.h"

// FORWARD DECLARATIONS
class CRepository;
class CVcxConnUtilEngine;
class CVcxConnUtilPubSub;
class CVcxConnectionUtility;
class CVcxConnUtilWaitSch;


// CLASS DECLARATION

/**
* This class defines operations for handling the networking notifications
* from the engine class, notifications from the PS and provides a functionality
* for those.
*
*/
NONSHARABLE_CLASS( CVcxConnUtilImpl) : public CBase, 
                                       public MConnUtilPubsubObserver,
                                       public MConnUtilExtEngineObserver
    {

    public: // Constructors and destructor
        
        /**
         * Construction.
         * 
         * @param CVcxConnectionUtility* pointer to main interface class for
         *                               for showing dialogs
         * @return The newly constructed CVcxConnUtilEngine
         */
        static CVcxConnUtilImpl* NewL( CVcxConnectionUtility* aUiInterface );
    
        /**
         * Construction. Object left to cleanupstack
         * 
         * @param CVcxConnectionUtility* pointer to main interface class for
         *                               for showing dialogs
         * @return The newly constructed CVcxConnUtilEngine
         */
        static CVcxConnUtilImpl* NewLC( CVcxConnectionUtility* aUiInterface );
        
        /**
         * Destructor.
         */
        virtual ~CVcxConnUtilImpl();

    private:
                
        /**
         * constructor.
         *
         * @param CVcxConnectionUtility* pointer to main interface class for
         *                               for showing dialogs
         */
        CVcxConnUtilImpl( CVcxConnectionUtility* aUiInterface  );

        /**
         * default constructor definition.
         *
         */
        CVcxConnUtilImpl( );
        
        /**
         * Private 2nd phase construction.
         */
        void ConstructL();
                 
    public: // New functions
       
        
        /**
        * Returns open IAP, If connection is not opened it will be created
        *
        * @param aIapId     On return, IAP ID.
        * @param aSilent    If ETrue, tries to open connection silently (without dialogs)
        *                    
        * @return KErrNone or one of the system wide error codes.
        */
        TInt GetIap( TUint32& aIapId, TBool aSilent );

        /**
        * Closes open connection.
        */
        void DisconnectL();
        
        /**
         * Registers observer
         * 
         * @param MConnUtilEngineObserver
         */
        void RegisterObserverL( MConnUtilEngineObserver* aObserver );
        
        /**
         * Removes observer from the array of observers
         * 
         * @param MConnUtilEngineObserver 
         */
        void RemoveObserver( MConnUtilEngineObserver* aObserver );
        
        /**
         * Returns current connection status
         *  
         * @return TVCxConnectionStatus
         */
        TVCxConnectionStatus EngineConnectionStatus();
        
        /**
         * Gets WAP id from IAP id. Leaves with KErrNotFound if no record with given 
         * IAP id is found.
         * 
         * @return WAP id matching the given IAP id.
         */
        TUint32 WapIdFromIapIdL( TUint32 aIapId );

    private: // from MConUtilPubsubObserver
          
        /**
         * Notification about pubsub key (integer) value change 
         *  
         * @param aKey   PS key informing the change
         * @param aValue a new value           
         */
        void ValueChangedL(const TUint32& aKey, const TInt&  aValue);
          
    private: // from MConnUtilExtEngineObserver
         
        /**
         * Engine has gotten mobility event about preferred IAP. Utility must requets permission
         * from it's clients before connection changing can be done.
         *  
         * @return TBool  ETrue if roaming can be allowed from the applicatin.            
         */
        TBool RequestIsRoamingAllowedL();
          
        /**
         * Notification about IAP change due roaming. If this instance
         * is master, it should change the IAP to Pubsub
         *              
         */
        void IapChangedL();
        
        /**
         * Method calls video connection utility's wait scheduler to
         * put current active object to wait with given id.
         * 
         * @param aWaitId  id for the wait
         */
        void WaitL( TUint32 aWaitId );
      
        /**
         * Method calls video connection utility's wait scheduler to
         * end wait for given id
         * 
         * @param aWaitId  id for the wait
         */
        void EndWait( TUint32 aWaitId ); 
        
        /**
         * Method return ETrue, if this instance is  master
         * 
         * @return TBool ETrue if is master, EFalse if not master
         */
        TBool IsMaster();
        
    private: // New functions
        
        /**
         * Method reads Video center's connection related settings
         * from the cenrep. Method also tries to reinit details inside
         * conn util engine, if it fails, "always ask" is saved to VC settings
         *              
         * @return KErrNone or one of the system wide error codes. Method can also leave.
         */
        TInt PrepareConnSettings();
       
        /**
         * Setups the connection creation prefs for "always ask" situation, and calls engine
         * to start connection or calls DoCreateConnectionL for other cases.
         *   
         * @param aSilent    If ETrue, tries to open connection silently (without dialogs) 
         *           
         * @return KErrNone or one of the system wide error codes. Method can also leave.
         */        
        TInt CreateConnection( TBool aSilent );
        
        /**
         * Setups and tries to open connection via SNAP.
         *  
         * @param aSilent         If ETrue, tries to open connection silently (without dialogs) 
         * @param aSnapId         snap (destination) id to use in connection
         * @param aMasterConnect  If true, mobility object needs to be created for receiving events
         *                        from network.  
         *           
         * @return KErrNone or one of the system wide error codes. Method can also leave.
         */   
        TInt DoCreateConnection( TBool aSilent, TInt32 aSnapId, TBool aMasterConnect );       
        
        /**
         * After connection staring is finished by the engine, this method is to be called.
         * Method first checks is connection is ne for this inctance and if it is, 
         * utility's internal connection information is setted up. Method also
         * calls CheckAndChangeSlaveToMaster to setup this inctance to be master if 
         * one does not yet exist. If this inctance is to be master, connection info
         * is updated to PS as well. 
         *   
         *           
         * @return KErrNone if connection data filled ok and we are connected.
         */
        TInt HandleFinalizeConnection();
        
        /**
         * Displays a wait note
         *   
         * @param aConnectionName name of connection to show on the dialog          
         */ 
        void DisplayWaitNote( const TDesC& aConnectionName = KNullDesC );
        
        /**
         * closes a wait note
         *           
         */
        void CloseWaitNote();
        
        /**
         * Method queries all observers and requests roaming status from them.
         * If all observers accept or there is not observers, PS key EVCxPSNbRoamAccepted
         * is increased by one. After method ends, PS key EVCxPSNbrRoamResp is increased by 
         * one to notify that one response is ready.  
         *   
         * @return TBool    ETrue if roaming is allowed        
         */ 
        TBool DoRequestClientRoamingL();
        
        /**
         * Method saves iap id and name, snap id and name and connection state
         * to pubsub.  
         *           
         */ 
        void SaveConnectionToPubSubL();
        
        /**
         * Method handles changing of slave to master when existing 
         * master is diconnecting. If change has occured, reinits
         * the connection to be used via snap instead of IAP
         *            
         */
        void HandleMasterChangeL();
        
        /**
         * Method handles the actual change of this instance to be 
         * master if there is not yet any. 
         * 
         * Changing is protected by semaphore, so only one existing
         * slave instance can check and possible change it's status
         * at the time.  
         * 
         * Note that KErrNone does not indicate succesfull status 
         * chenge. Caller should also check if iMaster -value has changed.
         * 
         * @return TInt  KErrNone or one of the system wide error codes,               
         */
        TInt CheckAndChangeSlaveToMaster();
        
        /**
         * Method changes slave instance connection status
         * based on given value.  
         * 
         * @param aNewStatus new status from master  
         *           
         */
        void HandleSlaveConnectionStatusL( const TInt& aNewStatus );
        
        /**
         * Method checks whether there are enough responses for the roaming 
         * request. If there is sets roaming acceptance status to PS.   
         * 
         * @param aNbrOfResps number of responces received so far  
         *           
         */
        void HandleRoamingReponsesL( const TInt& aNbrOfResps );
        
        /**
         * Method notifies observers about IAP changed event  
         *           
         */
        void NotifyObserversIAPChanged();
                                      
    private: // Data
        
        
        /**
         * Interface for UI related functions for connecting utility:
         * Showing and hiding connection -dialog
         */
        CVcxConnectionUtility* iUIInterface;
        
        /**
         * Connection utility engine. Handles connection creation, disconnecting and
         * connection meta data
         */
        CVcxConnUtilEngine* iEngine; 
        
        /**
         * pubsub engine, listens pubsub keys
         * and notify chenges 
         */
        CVcxConnUtilPubSub* iPubsub;

        /**
         * Wait handler for managing all CactiveSchedulerWaits
         * used by video connection utility
         */
        CVcxConnUtilWaitSch* iWaitHandler;
                               
        /**
         * Array of external network observers. 
         */
        RArray<MConnUtilEngineObserver*> iObservers;
        
        /**
         * Is this instance the master
         */
        TBool iMaster;
                
        /**
         * Semaphore to prevent concurrency in master / slave switch
         */
        RSemaphore iSemaSwitchRole;
        
        /**
         * Semaphore to prevent concurrency connection creation
         */
        RSemaphore iSemaCreateConn;

        /**
         * Used curing connection creation to tell active objects
         * that current connection is new.
         */
        TBool iNewConnection;
    };
#endif // __VCX_CONNUTILIMPL_H_
// End of File
