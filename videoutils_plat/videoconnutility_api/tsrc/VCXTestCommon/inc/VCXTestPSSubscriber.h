/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*
*/


#ifndef VCXTESTPSSUBSCRIBER_H
#define VCXTESTPSSUBSCRIBER_H

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		// For RTimer, link against: euser.lib

#include <e32property.h>

class MVCXTestPSObserver;

class CVCXTestPSSubscriber : public CActive
    {
    public:

        /**
         * Construction.
         *
         * @param aUid      category for the property
         * @param aKey      key id for the property
         * @param aType     type of the key (at the moment only int and text are supported)
         * @param aObserver pointer to observer to who to notify about changes
         *
         * @return The newly constructed CVcxConnUtilPubSub
         */
        IMPORT_C static CVCXTestPSSubscriber* NewL( const TUid aUid,
                                             const TUint32 aKey,
                                             RProperty::TType aType,
                                             MVCXTestPSObserver* aObserver );

        /**
         * Destructor.
         *
         */
        IMPORT_C virtual ~CVCXTestPSSubscriber();

        /**
         * Returns a pubsub value from this property
         *
         *@param &aValue value to fetch
         *
         *@return TInt KErrNone or some system level error code
         */
        IMPORT_C TInt Get( TInt& aValue );

        /**
         * Returns a pubsub value from this property
         *
         *@param &aValue value tofetch
         *
         *@return TInt KErrNone or some system level error code
         */
        IMPORT_C TInt Get( TDes& aValue );

        /**
         * Sets a pubsub value from this property
         *
         *@param aValue value to set
         *
         *@return TInt KErrNone or some system level error code
         */
        IMPORT_C TInt Set( TInt& aValue );

        /**
         * Sets a pubsub value from this property
         *
         *@param aValue value to set
         *
         *@return TInt KErrNone or some system level error code
         */
        IMPORT_C TInt Set( const TDesC& aValue );

        /**
         * Start subscribing if not already started
         */
        IMPORT_C void Start();

        /**
         * Activate safe wait object to wait modifications
         */
        IMPORT_C void WaitChangeL();

        /**
         * Explicitly ends waiting
         */
        IMPORT_C void EndWait();

    private:  // constuct / destruct

        /**
         * Constructor
         *
         */
        CVCXTestPSSubscriber( const TUid aUid,
                                const TUint32 aKey,
                                RProperty::TType aType,
                                MVCXTestPSObserver* aObserver );

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
         * Waiter object for data change
         */
        CActiveSchedulerWait* iSafeWait;

        /**
         * Observer
         */
        MVCXTestPSObserver* iObserver;
    };

#endif // VCXTESTPSSUBSCRIBER_H
