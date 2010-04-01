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
* Description:    Base class for uni plugin controllers*
*/





// INCLUDE FILES
#include "CCseSchedulerUniPluginController.h"
#include "CseDebug.h"                           // Debug macros
#include "CCseSchedulerPluginStarter.h"
#include "CCseSchedulerLandLord.h"
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
// CCseSchedulerUniPluginController::CCseSchedulerUniPluginController
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCseSchedulerUniPluginController::CCseSchedulerUniPluginController (
                                        CCseSchedulerServerEngine& aEngine,
                                        TInt32 aPluginUid ) :
                                        CCseSchedulerPluginControllerBase( aEngine, aPluginUid ),                                        
                                        iLandLord( NULL ),
                                        iIsLandLordWorking( EFalse )
    {
    CSELOGSTRING_HIGH_LEVEL(
        ">>>CCseSchedulerUniPluginController::CCseSchedulerUniPluginController");
        
    CSELOGSTRING_HIGH_LEVEL(
        "<<<CCseSchedulerUniPluginController::CCseSchedulerUniPluginController");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerUniPluginController::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCseSchedulerUniPluginController* CCseSchedulerUniPluginController::NewL(
                                        CCseSchedulerServerEngine& aEngine,
                                        TInt32 aPluginUid )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::NewL");
    
    CCseSchedulerUniPluginController* self =
            new( ELeave ) CCseSchedulerUniPluginController( aEngine, aPluginUid );
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerUniPluginController::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCseSchedulerUniPluginController::ConstructL()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::ConstructL");
    
    CCseSchedulerPluginControllerBase::ConstructL();
       
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::ConstructL");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerUniPluginController::~CCseSchedulerUniPluginController
// Destructor.
// -----------------------------------------------------------------------------
//
CCseSchedulerUniPluginController::~CCseSchedulerUniPluginController()
    {
    CSELOGSTRING_HIGH_LEVEL(
        ">>>CCseSchedulerUniPluginController::~CCseSchedulerUniPluginController");
    delete iLandLord;
    iLandLord = NULL;
    CSELOGSTRING_HIGH_LEVEL(
        "<<<CCseSchedulerUniPluginController::~CCseSchedulerUniPluginController");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerUniPluginController::RunPluginsL
// 
// -----------------------------------------------------------------------------
//
void CCseSchedulerUniPluginController::RunPluginsL()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::RunPluginsL");

    // Is there something to be run?!?
    if(iScheduleArray.Count() >0 )
        {
        // Check if there is security reasons for this schedule why this
        // schedule shouldn't be run
        if( iEngine.IsAllowedToRun( iScheduleArray[0]->DbIdentifier() ) )
            {
            // Check if LandLord is still working. If he is, we just fall through
            // because schedule will be kicked when current plugin completes
            if( iIsLandLordWorking == EFalse )
                {                    
                // When we come here in UniPluginController it means that there isn't any
                // threads running on LandLord side. Scrap the old if it exists and create new one.
                if ( iLandLord )
                    {
                    delete iLandLord;
                    iLandLord = NULL;
                    }
                
                iLandLord = CCseSchedulerLandLord::NewL( this );
                
                iLandLord->SetScheduleL( *iScheduleArray[0] );
                
                iEngine.IncreaseScheduleRunCountL( iScheduleArray[0]->DbIdentifier() );
                
                iIsLandLordWorking = ETrue;
                
                iLandLord->RunPluginL();
                }
            }
        else
            {
            // Schedule cannot be run anymore due to security reasons
            // Signal engine to remove schedule from DB
            iEngine.ScheduleNotValid( iScheduleArray[0]->DbIdentifier() );
            
            // Remove schedule own list
            delete iScheduleArray[0];
            iScheduleArray.Remove( 0 );
            
            // Call ourselves again.
            RunPluginsL();
            }            
        }
    else
        {
        // No schedules...!?!? REEEEESCHEDULE!!!
        iEngine.RequestReschedule();
        }

    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::RunPluginsL");
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerUniPluginController::SetSchedulesL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerUniPluginController::SetSchedulesL(
                                            RPointerArray<CCseScheduledProgram>& aScheduleArray )
    {
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::SetSchedules");

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
    if (iScheduleArray.Count() > 0  && !iIsLandLordWorking)
        {
        //  All the schedules have same time
        iTimer->SetTimer( iScheduleArray[0]->StartTime() );
        }

	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::SetSchedules");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerUniPluginController::ScheduledEventCompleted
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerUniPluginController::ScheduleCompletedL( const TUint32 aDbIdentifier,
                                                           TInt aCompletitionCode )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::ScheduleCompletedL");
    
    if ( aCompletitionCode == KErrNone )
        {
        ScheduleCompletedSuccesfullyL( aDbIdentifier );
        }
    else
        {        
        HandleScheduleErrorL( aDbIdentifier, aCompletitionCode );
        }    
     
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::ScheduleCompletedL");
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerUniPluginController::ScheduleCompletedSuccesfullyL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerUniPluginController::ScheduleCompletedSuccesfullyL( const TUint32 aDbIdentifier )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::ScheduleCompletedSuccesfullyL");
	
	// Remove completed schedule from list
    for ( TInt i = iScheduleArray.Count(); i > 0; i--)
        {
        if( iScheduleArray[i-1]->DbIdentifier() == aDbIdentifier )
            {
            delete iScheduleArray[i-1];
            iScheduleArray.Remove( i-1 );                
            }
        }

    // After all this LandLord has finished everything. He may rest now    
    iIsLandLordWorking = EFalse;            
        
    // Notify engine about completition.
    iEngine.ScheduleCompletedSuccesfullyL( aDbIdentifier );
    
    // If there is still schedule(s), set timer for it
    if( iScheduleArray.Count() > 0 )
        {
        iTimer->SetTimer( iScheduleArray[0]->StartTime() );
        }    
    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::ScheduleCompletedSuccesfullyL");	
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerUniPluginController::HandleScheduleErrorL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerUniPluginController::HandleScheduleErrorL( const TUint32 aDbIdentifier,
                                                            TUint32 aCompletitionCode )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::HandleScheduleErrorL");
	
    // Remove completed schedule from list
    for ( TInt i = iScheduleArray.Count(); i > 0; i--)
        {
        if( iScheduleArray[i-1]->DbIdentifier() == aDbIdentifier )
            {
            delete iScheduleArray[i-1];
            iScheduleArray.Remove( i-1 );                
            }
        }

    // After all this LandLord has finished everything. He may rest now    
    iIsLandLordWorking = EFalse;            
	
	// Here we could try to handle plugin errors. Problem is that scheduler doesn't
	// know anything about schedule it is running and so there cannot be tailored
	// error handling for each plugin. In the end, we just complete schedule
	// to engine with error code (ends up removing schedule from DB).
	iEngine.ScheduleCompletedWithErrorL( aDbIdentifier, aCompletitionCode );
	
	// If there is still schedule(s), set timer for it
    if( iScheduleArray.Count() > 0 )
        {
        iTimer->SetTimer( iScheduleArray[0]->StartTime() );
        }

	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::HandleScheduleErrorL");
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerUniPluginController::TimerErrorL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerUniPluginController::TimerErrorL( const TInt32 aError )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::TimerErrorL");
	
	if ( aError == KErrAbort )
		{
		 // System time changed. Make sanity check and start timer again.
		CSELOGSTRING_HIGH_LEVEL(
		    ">>>CCseSchedulerUniPluginController::TimerErrorL, system time has changed");
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
				// If schedule is in the past let the "normal" route decide when
				// it is he's her time to be run
				RunPluginsL();
				}
			}
		return; 
		}

	if ( aError == KErrUnderflow )
		{
		 // Scheduled time is in the past, pass it to notifier. It should decide what to do
		CSELOGSTRING_HIGH_LEVEL(
		    ">>>CCseSchedulerServerEngine::TimerErrorL, scheduled time is in the past");
		RunPluginsL();
		return;
		}
			
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::TimerErrorL");
	}

// -----------------------------------------------------------------------------
// CCseSchedulerUniPluginController::ClearController
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerUniPluginController::ClearControllerL( )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::ClearControllerL");	

    iLandLord->ClearL();
        
    iScheduleArray.ResetAndDestroy();
        
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::ClearControllerL");
	}

// -----------------------------------------------------------------------------
// CCseSchedulerUniPluginController::ClearController
//
// -----------------------------------------------------------------------------
//
TBool CCseSchedulerUniPluginController::IsControllerActive( )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerUniPluginController::IsControllerActive");
    
    CSELOGSTRING2_HIGH_LEVEL("<<<CCseSchedulerUniPluginController::IsControllerActive: %d",
                            iIsLandLordWorking);
    
    return iIsLandLordWorking;
    }

// End of file.
