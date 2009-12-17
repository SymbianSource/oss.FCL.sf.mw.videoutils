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


//  INCLUDES
#include "CIptvTestEventWaiter.h"
#include "MIptvTestEventWaiterObserver.h"
#include "CIptvTestEvent.h"
#include "VCXTestLog.h"

// -----------------------------------------------------------------------------
// CIptvTestEventWaiter::NewL
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestEventWaiter* CIptvTestEventWaiter::NewL(MIptvTestEventWaiterObserver& aObserver)
{
	VCXLOGLO1(">>>CIptvTestEventWaiter::NewL");
	CIptvTestEventWaiter* self = new (ELeave) CIptvTestEventWaiter(aObserver);

	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	VCXLOGLO1("<<<CIptvTestEventWaiter::NewL");
	return self;
}

// -----------------------------------------------------------------------------
// CIptvTestEventWaiter::~CIptvTestEventWaiter
// -----------------------------------------------------------------------------
//
EXPORT_C CIptvTestEventWaiter::~CIptvTestEventWaiter()
{
	VCXLOGLO1(">>>CIptvTestEventWaiter::~CIptvTestEventWaiter");
   	if( IsAdded() )
        {
        Deque(); // calls also Cancel()
        }

	iTimer.Close();

	Reset();
	VCXLOGLO1("<<<CIptvTestEventWaiter::~CIptvTestEventWaiter");
}

// -----------------------------------------------------------------------------
// CIptvTestEventWaiter::CheckTimeouts
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestEventWaiter::CheckTimeouts()
{
	VCXLOGLO1(">>>CIptvTestEventWaiter::CheckTimeouts");
	TTime now;
	now.UniversalTime();

	TInt i;
	for(i=0; i<iEventQueue.Count();i++)
		{
		CIptvTestEvent* event = iEventQueue[i];
		if( event->HasTimeouted( now ) )
			{
			VCXLOGLO1("CIptvTestEventWaiter:: event timedout");
			iObserver.EventWaiterErrorL( KErrTimedOut, *event );
			VCXLOGLO1("<<<CIptvTestEventWaiter::CheckTimeouts");
			return KErrTimedOut;
			}
		}

	for(i=0; i<iEventDump.Count();i++)
		{
		CIptvTestEvent* event = iEventDump[i];
		if( event->HasTimeouted( now ) )
			{
			VCXLOGLO1("CIptvTestEventWaiter:: event timedout");
			iObserver.EventWaiterErrorL( KErrTimedOut, *event );
			VCXLOGLO1("<<<CIptvTestEventWaiter::CheckTimeouts");
			return KErrTimedOut;
			}
		}

	VCXLOGLO1("<<<CIptvTestEventWaiter::CheckTimeouts");

	return KErrNone;
}

// -----------------------------------------------------------------------------
// CIptvTestEventWaiter::AddEvent
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestEventWaiter::AddEvent(CIptvTestEvent* aEvent, TBool aSequential)
{
	VCXLOGLO1(">>>CIptvTestEventWaiter::AddEvent");

	if(aSequential)
		{
		// First event in queue, activate it
		if(iEventQueue.Count() <= 0)
			{
			aEvent->Activate();
			}
		// Add event
		iEventQueue.Append( aEvent );
		}
	else
		{
		// All events in dump are active
		iEventDump.Append( aEvent );
		aEvent->Activate();
		}

	if( !IsActive() )
		{
		iTimer.After(iStatus, 1000000); // once in second
		SetActive();
		}

	VCXLOGLO1("<<<CIptvTestEventWaiter::AddEvent");
	return KErrNone;
}

// -----------------------------------------------------------------------------
// CIptvTestEventWaiter::EventHappened
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestEventWaiter::EventHappened(CIptvTestEvent* aEvent, CIptvTestEvent** aQueuedEvent)
{
	VCXLOGLO1(">>>CIptvTestEventWaiter::EventHappened");
	TBool eventFound = EFalse;

	// Check first from the queue.

	VCXLOGLO1("Event queue:");

	for(TInt i=0; i<iEventQueue.Count();i++)
		{
		CIptvTestEvent* event = iEventQueue[i];
		TBuf<32> name;
		event->GetName(name);
		VCXLOGLO3(" queue %d: %S, %d", i, &name);
		}

	//CheckTimeouts();

	if(iEventQueue.Count() > 0)
		{
		CIptvTestEvent* event = iEventQueue[0];
		TBuf<32> name;
		event->GetName(name);
		VCXLOGLO2(" - %S", &name);

		if( !eventFound && event->Compare( *aEvent ) )
			{
			iEventQueue.Remove( 0 );
			if(iEventQueue.Count() > 0)
				{
				iEventQueue[0]->Activate();
				}
			iPastEvents.Append( event );
			eventFound = ETrue;

			*aQueuedEvent = event;

			VCXLOGLO1("*Match*");
			}
		}

	// Check if the event is in the dump, meaning it can happen any time.

	VCXLOGLO1("Event dump:");

	if(!eventFound)
		{
		for(TInt i=0; i<iEventDump.Count();i++)
			{
			CIptvTestEvent* event = iEventDump[i];

			TBuf<32> name;
			event->GetName(name);
			VCXLOGLO2(" - %S", &name);

			if( !eventFound && event->Compare( *aEvent ) )
				{
				iEventDump.Remove( i );
				iPastEvents.Append( event );
				eventFound = ETrue;
				*aQueuedEvent = event;
				VCXLOGLO1("*Match*");
				}
			}
		}

	// Event was found and there's no more events in queues. Message observer that all events are validated.

	if( eventFound && iEventQueue.Count() == 0 && iEventDump.Count() == 0 )
		{
		iObserver.EventWaiterAllEventsValidated();
		}

	if( !eventFound )
		{
		VCXLOGLO1("CIptvTestEventWaiter:: Event not found from queues.");

		if(iEventQueue.Count() > 0)
			{
			CIptvTestEvent* event = iEventQueue[0];
			*aQueuedEvent = event;
			}

		VCXLOGLO1("<<<CIptvTestEventWaiter::EventHappened");
		return KErrNotFound;
		}

	VCXLOGLO1("<<<CIptvTestEventWaiter::EventHappened");
	return KErrNone;
}

// -----------------------------------------------------------------------------
// CIptvTestEventWaiter::Reset
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestEventWaiter::Reset()
{
	VCXLOGLO1(">>>CIptvTestEventWaiter::Reset");
	iEventQueue.ResetAndDestroy();
	iEventDump.ResetAndDestroy();
	iPastEvents.ResetAndDestroy();
	VCXLOGLO1("<<<CIptvTestEventWaiter::Reset");
	return KErrNone;
}


// -----------------------------------------------------------------------------
// CIptvTestEventWaiter::GetEventCount
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestEventWaiter::GetEventCount()
{
	VCXLOGLO1(">>>CIptvTestEventWaiter::GetEventCount");
	TInt count = 0;
	count += iEventQueue.Count();
	count += iEventDump.Count();
	VCXLOGLO2("<<<CIptvTestEventWaiter:: events in queue: %d", count);

	VCXLOGLO1("<<<CIptvTestEventWaiter::GetEventCount");
	return count;
}

// -----------------------------------------------------------------------------
// CIptvTestEventWaiter::CIptvTestEventWaiter
// -----------------------------------------------------------------------------
//
CIptvTestEventWaiter::CIptvTestEventWaiter(MIptvTestEventWaiterObserver& aObserver)
 : CActive(CActive::EPriorityStandard), iObserver(aObserver)
{

}

// -----------------------------------------------------------------------------
// CIptvTestEventWaiter::ConstructL
// -----------------------------------------------------------------------------
//
void CIptvTestEventWaiter::ConstructL()
{
	VCXLOGLO1(">>>CIptvTestEventWaiter::ConstructL");
	User::LeaveIfError(iTimer.CreateLocal());
	CActiveScheduler::Add(this);
	VCXLOGLO1("<<<CIptvTestEventWaiter::ConstructL");
}

// ---------------------------------------------------------------------------
// CTestUtilConnectionWaiter::RunL()
//
// Handle request completion events
// ---------------------------------------------------------------------------
//
void CIptvTestEventWaiter::RunL()
	{
 	//VCXLOGLO1(">>>CIptvTestEventWaiter::RunL");

//    TInt statusCode = iStatus.Int();

	CheckTimeouts();

	// Start timer again if waiting for events

	if(iEventQueue.Count() > 0 || iEventDump.Count() > 0)
		{

		if( !IsActive() )
			{
			iTimer.After(iStatus, 1000000); // once in second
			SetActive();
			}
		}


    //VCXLOGLO1("<<<CIptvTestEventWaiter::RunL");
	}

// ---------------------------------------------------------------------------
// CIptvTestEventWaiter::DoCancel()
//
// Cancels ongoing requests
// ---------------------------------------------------------------------------
//
void CIptvTestEventWaiter::DoCancel()
	{
	VCXLOGLO1(">>>CIptvTestEventWaiter::DoCancel");
	iTimer.Cancel();
	VCXLOGLO1("<<<CIptvTestEventWaiter::DoCancel");
    }


// End of File
