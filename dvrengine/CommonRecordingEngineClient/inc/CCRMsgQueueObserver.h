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
* Description:    Common recording engine message queue observer*
*/




#ifndef CCRMSGQUEUEOBSERVER_H
#define CCRMSGQUEUEOBSERVER_H

// INCLUDES
#include <e32base.h>
#include <e32msgqueue.h>
#include <ipvideo/MCREngineObserver.h>
#include <ipvideo/CRTypeDefs.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class MCREngineObserver;

// CLASS DECLARATION

/**
*  Common recording engine message queue observer.
*  Uses Symbian OS class RMsgQueue.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( CCRMsgQueueObserver ) : public CActive
    {            

public:  // Constructors and destructor

    /**
    * Two-phased constructor.
    */
    static CCRMsgQueueObserver* NewL( );

    /**
    * Virtual destructor.
    */
    virtual ~CCRMsgQueueObserver();

public: // New methods

    /**
    * Method that adds a view to message queue's observer list.
    * @since Series 60 3.0
    * @param aObserver is the object that will be notified
    * @return none.
    */
    void AddMsgQueueObserverL( MCREngineObserver* aObserver );

    /** 
    * Sets session id to listen to.
    * @since Series 60 3.0
    * @param aSessionId session id
    * @return none.
    */
    void SetSessionId( TInt aSessionId );

private: // Constructors

    /**
    * C++ default constructor.
    */
    CCRMsgQueueObserver( );

    /**
    * By default Symbian 2nd phase constructor is private.
    */
    void ConstructL();

private: // Methods from base classes

    /**
    * From CActive.
    * Handles an active object’s request completion event.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void RunL();

    /**
    * From CActive.
    * Called when outstanding request is cancelled.
    * This function is called as part of the active object’s Cancel().
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void DoCancel();

    /**
    * From CActive.
    * Handles a leave occurring in the request completion  event handler RunL().
    * @since Series 60 3.0
    * @param aError Error code.
    * @return none.
    */
    TInt RunError( TInt aError );

private: // Data

    /**
    * Actual message queue. Or handle to kernel-side object.
    */
    RMsgQueue<SCRQueueEntry> iQueue; 

    /**
    * This is data-area for the message received from kernel side.
    */
    SCRQueueEntry iReceivedMessage; 

    /**
    * Observers are views.
    */
    RPointerArray<MCREngineObserver> iObservers;

    /** 
    * Session id to listen to
    */
    TInt iSessionId;

    };

#endif // CCRMSGQUEUEOBSERVER_H

// End of File
