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
* Description:    Class to handle connection creation.*
*/


#ifndef __VCX_CONNUTIL_H_
#define __VCX_CONNUTIL_H_

// Deprecation warning
#warning This header file has been deprecated. Will be removed in one of the next SDK releases.

// INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS
class CVcxConnUtilImpl;
class MConnUtilEngineObserver;


// CLASS DECLARATION
/**
* Class is the main interface class for the vcxconnectionutility.dll.
* By using this class, client can fetch iap ID and maintain an active connection
* for that iap.
* Usage:
*  @code
*  #include <vcxconnectionutility.h>
*  #include <vcxconnutilengineobserver.h> // for receiving events from ALR and iap changes
*
*  // Instantiation of connectionutility is done by Instance()
*  // every Instance call increments internal reference count by 
*  // one. When reference count is decremented to 0 deallocation
*  // occurs. Client should make sure that it calls as many
*  // DecreaseReferenceCount() as it calls InstanceL()
*
*  CVcxConnectionUtility* conUtil = CVcxConnectionUtility::InstanceL();
* 
*  // in case client is interested in events from ALR or from iap changes,
*  // client must implement MConnUtilEngineObserver interface and register
*  // as observer. See MConnUtilEngineObserver documentation for details 
*  conUtil->RegisterObserverL( this );
* 
*  // getting the best possible iap is handled by the GetIap
*  TInt err( KErrNone );
*  TUint32 iapId( 0 );
*  err = conUtil->GetIap( iapId, EFalse );
*  // in case client does not want the "connecting" dialog  // shown, aSilent
*  // parameter should be ETrue     
*
*  // client can try to explicitly disconnect connection, but if there are other references 
*  // connection is not closed.
*  // suggestion is to maintain connection at all time as long as vcxconnectionutility 
*  // is being used and just react to notifications 
*  conUtil->Disconnect();
*  
*  // when connectionutility is no longer needed, client must call DecreaseReferenceCount()
*  // if there are no more references left, memory allocated for the library is deallocated
*  // and possible active connection is closed
*  // if client has registered as observer, it should deregister first 
*  conUtil->RemoveObserver();
*  conUtil->DecreaseReferenceCount();  
*  conutil = NULL;
* 
*  @endcode
* 
*
* @lib vcxconnectionutility.dll
*/
NONSHARABLE_CLASS( CVcxConnectionUtility) : public CActive
    {

    public: // Constructors and destructor

        /**
         * returns instance of the singleton object. This increases
         * local reference count by one, so utility members should
         * not be called through this.
         *
         * @return Pointer to CVcxConnectionUtility object.
         */
        IMPORT_C static CVcxConnectionUtility* InstanceL();
        
        /**
        * Remove the reference to this connectionutility instance.
        * 
        * This should be called when client does not need 
        * utility anymore.
        *
        * Decreases the reference count by one. When
        * all references are removed, the ui engine is destructed.
        */
        IMPORT_C void DecreaseReferenceCount();

    private:
        /**
         * Destructor.
         */
        virtual ~CVcxConnectionUtility();
        
        /**
         * constructor.
         *
         */
        CVcxConnectionUtility( );

        /**
         * Private 2nd phase construction.
         */
        void ConstructL();
        
        /**
         * From CActive, handles an active object’s request completion event
         */
        void RunL();

        /**
         * From CActive, implements cancellation of an outstanding request.
         * This function is called as part of the active object’s Cancel().
         */
        void DoCancel();  
                 
    public: // New functions
       
        
        /**
        * Returns open IAP, if connection is not opened it is created
        *
        * @param aIapId     On return, IAP ID.
        * @param aSilent    If ETrue, tries to open connection silently (without dialogs)
        *                   In the 'Always ask' mode query dialog is always shown.
        *                    
        * @return KErrNone or one of the system wide error codes.
        */
        IMPORT_C TInt GetIap( TUint32& aIapId, TBool aSilent );

        /**
         * Gets WAP id from IAP id. Leaves with KErrNotFound if no record with given 
         * IAP id is found.
         * 
         * @param aIapId IAP id to match.
         * @return WAP id matching the given IAP id.
         */
        IMPORT_C TUint32 WapIdFromIapIdL( TUint32 aIapId );

        /**
        * Closes open connection.
        */
        IMPORT_C void DisconnectL();
        
        /**
         * Registers observer
         * 
         * @param MConnUtilEngineObserver
         */
        IMPORT_C void RegisterObserverL( MConnUtilEngineObserver* aObserver );
        
        /**
         * Removes observer from the array of observers
         * 
         * @param MConnUtilEngineObserver 
         */
        IMPORT_C void RemoveObserver( MConnUtilEngineObserver* aObserver );
           
        /**
         * Displays a wait note. Method calls notifier to show 
         * dialog and sets this active object active. If user cancels
         * dialog by pressing cancel, RunL is being called with KErrCancel.
         *   
         * @param aConnectionName name of connection to show on the dialog          
         */ 
        void DisplayWaitNote( const TDesC& aConnectionName = KNullDesC );
        
        /**
         * closes a wait note
         *           
         */
        void CloseWaitNote();

                                                                                
    private: // Data
        
        /**
         * Count of references to this object.
         */ 
        TInt iReferenceCount;
        
        /**
         * Utility implementation object
         */
        CVcxConnUtilImpl* iUtilImpl;
                        
        /**
         * Notifier used for showing connecting dialogs
         */
        RNotifier iNotifier; 
                                            
    };
#endif // __VCXNS_CONNUTIL_H_
// End of File
