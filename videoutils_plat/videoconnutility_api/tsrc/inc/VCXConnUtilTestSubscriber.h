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
* Description:   Class to handle subscribtions from PS*
*/


#ifndef VCXCONNUTILTESTSUBSCRIBER_H
#define VCXCONNUTILTESTSUBSCRIBER_H

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		// For RTimer, link against: euser.lib

#include <e32property.h>

class MVCXConnUtilTestPSObserver;

class CVCXConnUtilTestSubscriber : public CActive
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
        static CVCXConnUtilTestSubscriber* NewL( const TUid aUid,
                                             const TUint32 aKey,
                                             RProperty::TType aType,
                                             MVCXConnUtilTestPSObserver* aObserver );

        /**
         * Destructor.
         *
         */
        virtual ~CVCXConnUtilTestSubscriber();

        /**
         * Returns a pubsub value from this property
         *
         *@param &aValue value to fetch
         *
         *@return TInt KErrNone or some system level error code
         */
        TInt Get( TInt& aValue );

        /**
         * Returns a pubsub value from this property
         *
         *@param &aValue value tofetch
         *
         *@return TInt KErrNone or some system level error code
         */
        TInt Get( TDes& aValue );

        /**
         * Sets a pubsub value from this property
         *
         *@param aValue value to set
         *
         *@return TInt KErrNone or some system level error code
         */
        TInt Set( TInt& aValue );

        /**
         * Sets a pubsub value from this property
         *
         *@param aValue value to set
         *
         *@return TInt KErrNone or some system level error code
         */
        TInt Set( const TDesC& aValue );

        /**
         * Start subscribing if not already started
         */
        void Start();

        /**
         * Activate safe wait object to wait modifications
         */
        void WaitChangeL();

        /**
         * Explicitly ends waiting
         */
        void EndWait();

    private:  // constuct / destruct

        /**
         * Constructor
         *
         */
        CVCXConnUtilTestSubscriber( const TUid aUid,
                                const TUint32 aKey,
                                RProperty::TType aType,
                                MVCXConnUtilTestPSObserver* aObserver );

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
        MVCXConnUtilTestPSObserver* iObserver;
    };

#endif // VCXCONNUTILTESTSUBSCRIBER_H
