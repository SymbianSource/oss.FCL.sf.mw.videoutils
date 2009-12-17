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
* Description:    Pubsub handler class for vcxconnectionutility*
*/




#ifndef __CVCX_CONUTILPUBSUB_H__
#define __CVCX_CONUTILPUBSUB_H__

#include <e32base.h>
#include <e32std.h>
#include <e32property.h>


class CVcxConnUtilSubscriber;
class MConnUtilPubsubObserver;

/**
* This class defines operations for handling the pub sub keys and their values.
*
*/
NONSHARABLE_CLASS( CVcxConnUtilPubSub ) : public CBase
    {	
    public: // construction / destruction
    
        /**
         * Construction.
         * 
         * @return The newly constructed CVcxConnUtilPubSub
         */
        static CVcxConnUtilPubSub* NewL( MConnUtilPubsubObserver *aObserver );
    
        /**
         * Construction. Object left to cleanupstack
         * 
         * @return The newly constructed CVcxConnUtilPubSub
         */
        static CVcxConnUtilPubSub* NewLC( MConnUtilPubsubObserver *aObserver );
        
        /**
         * destructor
         */
        virtual ~CVcxConnUtilPubSub();
                        
    private:
            
        /**
         * C++ constructor
         */
        CVcxConnUtilPubSub(); 
            
        /**
         * Private 2nd phase construction.
         *
         * During instantiation, the availibility of the PS keys is being checked. In case
         * keys does not exists, they are being created by starting an external executable,
         * vcxconnutilpsworker, whose UID is being used as the unique category of the PS keys.
         * This kind of solution is needed to make sure, that category is always unique and 
         * the UID of the category is always known beforehand in vcxconnectionutility library 
         * 
         * @param aObserver observer to be passed subscribers
         */
        void ConstructL( MConnUtilPubsubObserver *aObserver );  
        
            
    public: // new functions
        
        /**
         * Increases a "counter" pub sub value by one.
         * If given key is not concidered as a counter,
         * nothing is done
         *  
         * @param aKey    PS key whose value to increase
         */ 
        void IncCounterPubSubL( TUint32 aKey );
        
        /**
         * Decreases a "counter" pub sub value by one
         * If given key is not concidered as a counter,
         * nothing is done
         *  
         * @param aKey    PS key whose value to decrease
         */ 
        void DecCounterPubSubL( TUint32 aKey ); 
        
        /**
         * Saves given value to the defined PS key.
         * 
         * @param aKey    PS to where to save the value
         * @param aValue  Value to save
         * 
         * @return TInt   KErrNone is succees, KerrNotFound if provided key is
         *                not in the scope of connection util or another 
         *                system level error code
         *  
         */ 
        TInt SetValue( TUint32 aKey, TInt aValue );
        
        /**
         * Get value from  defined PS key.
         * 
         * @param aKey    PS from where to fetch value
         * @param aValue  
         * 
         * @return TInt   KErrNone is succees, KerrNotFound if provided key is
         *                not in the scope of connection util or another 
         *                system level error code
         *  
         */ 
        TInt GetValue( TUint32 aKey, TInt& aValue );
        
        /**
         * Start subscribers. If using deafult value, starts all.
         * Otherwise just starts the one defined as aKey
         * 
         * @param aKey    key value of subscriber to start 
         */
        void StartSubscibers( TUint32 aKey = 0 );
        
        /**
         * Cancels subscribers. If using deafult value, cancels all.
         * Otherwise just cancels the one defined as aKey
         * 
         * @param aKey    key value of subscriber to cancel 
         *  
         */
        void CancelValueSubscribers( TUint32 aKey = 0 );
        
   private:

       /**
        * Method starts a vcxconnutilpsworker -process
        * to create PS keys for connectionutility and 
        * waits for the process to finish before returning.
        * 
        * If another process has already started worker, method
        * waits for it to finish.
        * 
        * Method leaves if existing semaphore cannot be opened or
        * creating of process fails.
        *  
        */
        void DefinePropertiesL();
        
        /**
         * Internal helper method to get a pointer to RProperty
         * defined by the aKey -value.
         * In case aKey refers to a property that does not exists,
         * a null pointer is returned
         * 
         * @param aKey a key value for property wanted
         * 
         * @return RProperty* pointer to property or NULL in case 
         *                    property is not found 
         */
        RProperty* GetProperty( TUint32 aKey );
        
        /**
         * Internal helper method to get a pointer to Subscriber
         * defined by the aKey -value.
         * In case aKey refers to a Subscriber that does not exists,
         * a null pointer is returned
         * 
         * @param aKey a key value for subscriber wanted
         * 
         * @return CVcxConnUtilSubscriber* pointer to Subscriber or NULL in case 
         *                                 property is not found 
         */
        CVcxConnUtilSubscriber* GetSubscriber( TUint32 aKey );
        
         /**
         * Internal helper method to handle actual incrementation or 
         * decrementation (based on the calfactor) of the PS key concidered 
         * to be a counter value in connectionutility's context.
         * 
         * @leave if PS Get or Set fails
         * 
         * @param aKey a key value for subscriber wanted
         * 
         * @param aCalcFactor a value to be added to the PS key value
         */
        void DoPSCounterIncrementL( TUint32 aKey, TInt aCalcFactor );
       
   private:

        /**        
         * P&S Current IAP's Id
         */
        RProperty iPropertyCurrentIAPId;

        /**
         * P&S Current Destination's Id
         */
        RProperty iPropertyCurrentSNAPId;
                       
        /**
         * P&S telling how many instances are using connection
         */
        RProperty iPropertyNbrOfInst;
        
        /**
         * P&S telling how many instances 
         * have accepted roaming
         */
        RProperty iPropertyAccepted;
    
        /**
         * P&S telling Connection state property.
         * Object contains PS property for connection state -key and
         * subcribe -listener for that value changes
         */
        CVcxConnUtilSubscriber* iConnectionStatePS;
                
        /**
         * P&S telling if we currently have master instance or not
         * Object contains PS property for masterexist -key and
         * subcribe -listener for that value changes
         */
        CVcxConnUtilSubscriber* iMasterExistPS;
        
        /**
         * P&S telling the roaming request status from
         * the instances
         */
        CVcxConnUtilSubscriber* iRoamingReqStatusPS;
        
        /**
         * P&S telling the amount of connection users responded
         * roaming request
         */
        CVcxConnUtilSubscriber* iNbrInstanceResp;
        
        /**
         * Semaphore to protect critical counter -PS 
         * increment and decrement operations.          
         */
         RSemaphore iSemaCounter;
        
    };

#endif // __CVCX_CONUTILPUBSUB_H__
