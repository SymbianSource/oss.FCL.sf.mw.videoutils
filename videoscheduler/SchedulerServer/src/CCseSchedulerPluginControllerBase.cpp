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
* Description:    Base class for plugin controllers*
*/





// INCLUDE FILES
#include "CCseSchedulerPluginControllerBase.h"
#include "CseDebug.h"                           // Debug macros
#include <ipvideo/CCseScheduledProgram.h>
#include "CCseSchedulerTimer.h"
#include "CCseSchedulerServerEngine.h"


// EXTERNAL DATA STRUCTURES
// None.

// EXTERNAL FUNCTION PROTOTYPES  
// None.

// CONSTANTS
// None

// MACROS
// None.

// LOCAL CONSTANTS AND MACROS
// None.

// MODULE DATA STRUCTURES
// None.

// LOCAL FUNCTION PROTOTYPES
// None.


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::CCseSchedulerPluginControllerBase
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCseSchedulerPluginControllerBase::CCseSchedulerPluginControllerBase( MCsePluginControllerObserver& aEngine,
                                                                      TInt32 aPluginUid ) :
                                                                      iEngine( aEngine ),
                                                                      iPluginUid( aPluginUid )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginControllerBase::CCseSchedulerPluginControllerBase");
            
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginControllerBase::CCseSchedulerPluginControllerBase");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCseSchedulerPluginControllerBase* CCseSchedulerPluginControllerBase::NewL( MCsePluginControllerObserver& aEngine,
                                                                      TInt32 aPluginUid )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginControllerBase::NewL");
    
    CCseSchedulerPluginControllerBase* self = new( ELeave ) CCseSchedulerPluginControllerBase( aEngine, aPluginUid );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginControllerBase::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginControllerBase::ConstructL()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginControllerBase::ConstructL");
    
    iTimer = CCseSchedulerTimer::NewL( this );    
               
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginControllerBase::ConstructL");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::~CCseSchedulerPluginControllerBase
// Destructor.
// -----------------------------------------------------------------------------
//
CCseSchedulerPluginControllerBase::~CCseSchedulerPluginControllerBase()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginControllerBase::~CCseSchedulerPluginControllerBase");
    
    delete iTimer;    
    iScheduleArray.ResetAndDestroy();
		
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginControllerBase::~CCseSchedulerPluginControllerBase");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::SetSchedulesL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginControllerBase::SetSchedulesL( RPointerArray<CCseScheduledProgram>& aScheduleArray )
    {
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginControllerBase::SetSchedules");

    //  The aScheduleArray parameter contains schedules which are either before
    //  or at the same time as the existing ones in the iScheduleArray.
    //  Just destroy the old ones and replace with new ones.
    //  Also cancel the timer because the schedule to run may be different

    iScheduleArray.ResetAndDestroy();
    iTimer->Cancel();

	// Go through all given schedules and copy them our array
	for( TInt i = 0; i < aScheduleArray.Count(); i++)
	    {
        CCseScheduledProgram* schedule =
                CCseScheduledProgram::NewL(*aScheduleArray[i] );
        CleanupStack::PushL( schedule );

	    iScheduleArray.AppendL( schedule );

	    CleanupStack::Pop( schedule );
	    }

    //  If we have schedules, set the timer
    if (iScheduleArray.Count() > 0)
        {
        //  All the schedules have same time
        iTimer->SetTimer( iScheduleArray[0]->StartTime() );
        }

	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginControllerBase::SetSchedules");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::GetPluginUid
//
// -----------------------------------------------------------------------------
//
TInt32 CCseSchedulerPluginControllerBase::PluginUid( )
    {
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginControllerBase::PluginUid");
	CSELOGSTRING2_HIGH_LEVEL("<<<CCseSchedulerPluginControllerBase::PluginUid: UID = %d",
	                             iPluginUid);
    return iPluginUid;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::RunPluginsL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginControllerBase::RunPluginsL( )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginControllerBase::RunPluginsL");
    CSELOGSTRING_HIGH_LEVEL(
        "CCseSchedulerPluginControllerBase::RunPluginsL - Base class, do nothing");
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginControllerBase::RunPluginsL");
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::IsObsolote
//
// -----------------------------------------------------------------------------
//
TBool CCseSchedulerPluginControllerBase::IsObsolete() const
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginControllerBase::IsObsolete");
    
    TBool ret( EFalse );
    
    // If we have no schedules in array we declare ourselves as "obsolete".
    if( iScheduleArray.Count() == 0 )
        {
        ret = ETrue;
        }
    
	CSELOGSTRING2_HIGH_LEVEL("<<<CCseSchedulerPluginControllerBase::IsObsolete: %d", ret);
	
	return ret;
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::ScheduleCompleted
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginControllerBase::ScheduleCompletedL( const TUint32 /*aDbIdentifier*/,
                                                            TInt /*aCompletitionCode*/ )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::ScheduleCompleted");	
	CSELOGSTRING_HIGH_LEVEL(
	    "CCseSchedulerUniPluginController::ScheduledEventCompleted - WTF LEIK BSAE KLASS");
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::ScheduleCompleted");
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::TimerErrorL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginControllerBase::TimerErrorL( const TInt32 aError )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerPluginControllerBase::TimerErrorL");
	
	if ( aError == KErrAbort )
		{
		 // System time changed. Make sanity check and start timer again.
		CSELOGSTRING_HIGH_LEVEL(
		    ">>>CCseSchedulerPluginControllerBase::TimerErrorL, system time has changed");
		if ( iScheduleArray.Count() > 0 )
			{
			TTime now;
			now.UniversalTime();
			// Time still in the future
			if ( now < iScheduleArray[0]->StartTime() )
				{
				iTimer->SetTimer( iScheduleArray[0]->StartTime() );
				}
			else
				{
				RunPluginsL();
				}
			}
		return; 
		}

	if ( aError == KErrUnderflow )
		{
		 // Scheduled time is in the past, pass it to notifier. It should decide what to do
		CSELOGSTRING_HIGH_LEVEL(
		    ">>>CCseSchedulerPluginControllerBase::TimerErrorL, scheduled time is in the past");
		RunPluginsL();
		return;
		}
			
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerPluginControllerBase::TimerErrorL");
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::DoCleanUp
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginControllerBase::DoCleanUp( )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::DoCleanUp");	
	CSELOGSTRING_HIGH_LEVEL(
	    "CCseSchedulerUniPluginController::DoCleanUp - Base class");
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::DoCleanUp");
	}

// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::SetAsObsolete
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginControllerBase::SetToBeCleared( TBool aToBeCleared )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::SetToBeCleared");	
	
	iToBeCleared = aToBeCleared;
	    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::SetToBeCleared");
	}


// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::ClearController
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerPluginControllerBase::ClearControllerL( )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::ClearControllerL");	

    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::ClearControllerL - Baseclass, nothing here");	

	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::ClearControllerL");
	}



// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::ClearPlugin
//
// -----------------------------------------------------------------------------
//
TBool CCseSchedulerPluginControllerBase::IsToBeCleared( )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::IsToBeCleared");	

    CSELOGSTRING2_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::IsToBeCleared: %d", iToBeCleared );	
    return iToBeCleared;	
	}

// -----------------------------------------------------------------------------
// CCseSchedulerPluginControllerBase::IsControllerActive
//
// -----------------------------------------------------------------------------
//
TBool CCseSchedulerPluginControllerBase::IsControllerActive( )
    {   
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::IsControllerActive");  

    CSELOGSTRING2_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::IsControllerActive: %d", iToBeCleared );  
    return ETrue;    
    }
