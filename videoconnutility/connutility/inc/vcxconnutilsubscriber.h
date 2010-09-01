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
* Description:    Class to handle subscribtions from PS*
*/




#ifndef __VCX_CONNUTILSUBSCRI_H
#define __VCX_CONNUTILSUBSCRI_H

#include <e32base.h>	
#include <e32property.h>

class MConnUtilPubsubObserver;

/**
* This class defines operations listening modifications for the
* property defined.
* Notifies changes throught MConnUtilPubsubObserver interface. 
*
*/ 
NONSHARABLE_CLASS ( CVcxConnUtilSubscriber ) : public CActive
    {

    public: // constuct / destruct
    
        /**
         * Construction.
         * 
         * @param aUid      category for the property
         * @param aKey      key id for the property
         * @param aType     type of the key (at the moment only int is supported)
         * @param aObserver pointer to observer to who to notify about changes
         * 
         * @return The newly constructed CVcxConnUtilPubSub
         */
        static CVcxConnUtilSubscriber* NewL( const TUid aUid, 
                                             const TUint32 aKey,
                                             RProperty::TType aType,
                                             MConnUtilPubsubObserver* aObserver );
        
        /**
         * Destructor.
         *
         */
        virtual ~CVcxConnUtilSubscriber();
        
        /**
         * Returns a reference to property this 
         * subscriber listens to.
         * 
         * @return RProperty
         */
        RProperty& Property();
              
        /**
         * Start subscribing if not already started
         */
        void Start();
        
        
    private:  // constuct
        
        /**
         * Constructor
         *
         */
        CVcxConnUtilSubscriber( const TUid aUid, 
                                const TUint32 aKey, 
                                RProperty::TType aType,
                                MConnUtilPubsubObserver* aObserver );
        
        /**
         * Symbian 2nd phase construction
         *   
         */
        void ConstructL();
     
    private: // from CActive
        
        /**
         * From CActive. Calles when value subscribed to changes 
         * 
         */  
        void RunL();
        
        /**
         * From CActive. Cancels subscribtion
         * 
         */
        void DoCancel();
        
    private:
        
        /**
         * ETrue if object is initialized and subscribtion starts
         */
        TBool                    iInitialized;
        
        /**
         * Property to subscribe
         */
        RProperty                iProperty;
        
        /**
         * Category uid for the property
         */
        const TUid               iUid;
        
        /**
         * Key id for the property
         */
        const TUint32            iKey;
        
        /**
         * type of the property
         */
        RProperty::TType         iKeyType;
                
        /**
         * Observer
         */
        MConnUtilPubsubObserver* iObserver; 
    };      

#endif // __VCX_CONNUTILSUBSCRI_H
