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



#ifndef CIPTVTESTEVENTWAITER_H
#define CIPTVTESTEVENTWAITER_H

//  INCLUDES
#include <e32base.h>
#include <e32cmn.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class MIptvTestEventWaiterObserver;
class CIptvTestEvent;

// CLASS DECLARATION

/**
*
* CIptvTestEventWaiter USAGE:
*
* Implement callbacks from MIptvTestEventWaiterObserver.
* Create your own event class inherited from CIptvTestEvent.
* Create instance.
*
* Add events that are expected to happen with CIptvTestEventWaiter::AddEvent method.
*  - Two queues: sequential events and others
*
* When event happen, call CIptvTestEventWaiter::EventHappened(event)
*  - This will compare the event to the ones in queues with CIptvTestEvent::Compare() method.
*    First tries to find not sequential events and then last sequential event in queue.
*  - If event is not found or it's timedout MIptvTestEventWaiterObserver::EventWaiterErrorL is called.
*
*  @since
*/

class CIptvTestEventWaiter : public CActive
	{
	public:
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CIptvTestEventWaiter* NewL(MIptvTestEventWaiterObserver& aObserver);

        /**
        * Destructor.
        */
		IMPORT_C virtual ~CIptvTestEventWaiter();

	public: // New functions

		/**
		* Checks if events have timeouted
		*/
		IMPORT_C TInt CheckTimeouts();

		/**
		* Will add an event to queue and also check all events if they have timeouted
		*/
		IMPORT_C TInt AddEvent(CIptvTestEvent* aEvent, TBool aSequential);

		/**
		* Will check the event against the events in queue
		* If event is found it is returned in aQueuedEvent
		* If event is not found the first event in event queue is returned in aQueuedEvent
		*/
		IMPORT_C TInt EventHappened(CIptvTestEvent* aEvent, CIptvTestEvent** aQueuedEvent);

		/**
		* Resets the event waiter system. All events are deleted.
		*/
		IMPORT_C TInt Reset();

		/**
		* Returns the amount of events left in queues
		*/
		IMPORT_C TInt GetEventCount();

    protected: //from base classes

		// from CActive

	    /*
	     * RunL()
	     *
	     * Handle request completion events
	     *
	     */
	    void RunL();

	    /*
	     * DoCancel()
	     *
	     * Cancel any outstanding requests
	     *
	     */
	    void DoCancel();

    private:

        /**
        * C++ default constructor.
        */
        CIptvTestEventWaiter(MIptvTestEventWaiterObserver& aObserver);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data

	private:

		// Events which should happen sequential
		RPointerArray<CIptvTestEvent> iEventQueue;

		// Events which have no particular order where they happen
		RPointerArray<CIptvTestEvent> iEventDump;

		RPointerArray<CIptvTestEvent> iPastEvents;

		MIptvTestEventWaiterObserver& iObserver;

		RTimer iTimer;
	};

#endif // CIPTVTESTEVENTWAITER_H

// End of File
