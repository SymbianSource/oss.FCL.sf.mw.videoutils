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
* Description:    Base class for multi plugin controllers*
*/





// INCLUDE FILES
#include "CCseSchedulerMultiPluginController.h"
#include "CseDebug.h"                           // Debug macros
#include "CCseSchedulerPluginStarter.h"
#include "CCseSchedulerLandLord.h"
#include <ipvideo/CCseScheduledProgram.h>
#include "CCseSchedulerTimer.h"
#include "CCseSchedulerServerEngine.h"
#include "CCseSchedulerThreadPacket.h"

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
// CCseSchedulerPluginControllerBase::CCseSchedulerMultiPluginController
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCseSchedulerMultiPluginController::CCseSchedulerMultiPluginController(
                                        CCseSchedulerServerEngine& aEngine,
                                        TInt32 aPluginUid ) :
                                        CCseSchedulerPluginControllerBase( aEngine, aPluginUid )

    {
    CSELOGSTRING_HIGH_LEVEL(
        ">>>CCseSchedulerMultiPluginController::CCseSchedulerMultiPluginController");
        
    CSELOGSTRING_HIGH_LEVEL(
        "<<<CCseSchedulerMultiPluginController::CCseSchedulerMultiPluginController");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerMultiPluginController::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCseSchedulerMultiPluginController* CCseSchedulerMultiPluginController::NewL( 
                                        CCseSchedulerServerEngine& aEngine,
                                        TInt32 aPluginUid )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerMultiPluginController::NewL");
    
    CCseSchedulerMultiPluginController* self = 
            new( ELeave ) CCseSchedulerMultiPluginController( aEngine, aPluginUid );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerMultiPluginController::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerMultiPluginController::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCseSchedulerMultiPluginController::ConstructL()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerMultiPluginController::ConstructL");

    CCseSchedulerPluginControllerBase::ConstructL();
       
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerMultiPluginController::ConstructL");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerMultiPluginController::~CCseSchedulerMultiPluginController
// Destructor.
// -----------------------------------------------------------------------------
//
CCseSchedulerMultiPluginController::~CCseSchedulerMultiPluginController()
    {
    CSELOGSTRING_HIGH_LEVEL(
        ">>>CCseSchedulerMultiPluginController::~CCseSchedulerMultiPluginController");
    
    iLandLords.ResetAndDestroy( );
    iLandLords.Close();    
		
    CSELOGSTRING_HIGH_LEVEL(
        "<<<CCseSchedulerMultiPluginController::~CCseSchedulerMultiPluginController");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerMultiPluginController::RunPluginsL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerMultiPluginController::RunPluginsL()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerMultiPluginController::RunPluginsL");
    TBool scheduleAlreadyRunning( EFalse );
    
    if(iScheduleArray.Count() >0 )
        {
        // Kick those schedules running that aren't already running
        for( TInt i = 0; i < iScheduleArray.Count(); i++)
            {
            for ( TInt ii = 0; ii < iLandLords.Count(); ii++ )
                {
                if( iScheduleArray[i]->DbIdentifier() ==
                        iLandLords[ii]->ThreadPacket()->Schedule()->DbIdentifier() )
                    {
                    scheduleAlreadyRunning = ETrue;
                    }                
                }
            if( !scheduleAlreadyRunning )
                {
                if( iEngine.IsAllowedToRun( iScheduleArray[i]->DbIdentifier() ) )
                    {                    
                    // Create new LandLord for each schedule to be run
                    CCseSchedulerLandLord* landLord = CCseSchedulerLandLord::NewL( this );
                                                  
                    CleanupStack::PushL( landLord );                    
                                                                                    
                    // Set schedule for LandLord
                    landLord->SetScheduleL( *iScheduleArray[i] );
                    
                    // Increase run count
                    iEngine.IncreaseScheduleRunCountL( iScheduleArray[0]->DbIdentifier() );
                    
                    // Order the LandLord to run plugin
                    landLord->RunPluginL();
                    
                    CleanupStack::Pop( landLord );
                    
                    // Add created landLord to array. Destroyed later when landLord
                    // is not needed anymore.
                    iLandLords.AppendL( landLord );
                    
                    CleanupStack::Pop( landLord );
                    }        
                else
                    {
                    // Schedule cannot be run anymore due to security reasons
                    // Signal engine to remove schedule from DB
                    iEngine.ScheduleNotValid( iScheduleArray[i]->DbIdentifier() );
                    
                    // Remove schedule own list
                    delete iScheduleArray[i];
                    iScheduleArray.Remove( i );
                    i--;
                    }
                }
            }
        if( iLandLords.Count() == 0 )
            {
            // We've gone through all our schedules and still no LandLords.
            // We need to request new schedules.
            iEngine.RequestReschedule(); 
            }
        }
    else
        {
        // No schedules...!?!? REEEEESCHEDULE!!!
        iEngine.RequestReschedule();
        }     
        
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerMultiPluginController::RunPluginsL");
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerMultiPluginController::ScheduleCompletedL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerMultiPluginController::ScheduleCompletedL( const TUint32 aDbIdentifier,
                                                             TInt aCompletitionCode )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerMultiPluginController::ScheduleCompletedL");

    if ( aCompletitionCode == KErrNone )
        {
        ScheduleCompletedSuccesfullyL( aDbIdentifier );            
        }
    else
        {
        HandleScheduleErrorL( aDbIdentifier, aCompletitionCode );
        }    
 
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerMultiPluginController::ScheduleCompletedL");
	}

// -----------------------------------------------------------------------------
// CCseSchedulerUniPluginController::ScheduleCompletedSuccesfullyL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerMultiPluginController::ScheduleCompletedSuccesfullyL( const TUint32 aDbIdentifier )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerMultiPluginController::ScheduleCompletedSuccesfullyL");

    // Remove schedule from list
    for ( TInt i = 0; i < iScheduleArray.Count(); i ++)
        {
        if( iScheduleArray[i]->DbIdentifier() == aDbIdentifier )
            {
            delete iScheduleArray[i];
            iScheduleArray.Remove( i );                
            }
        }
        
    // Notify engine about completition.
    iEngine.ScheduleCompletedSuccesfullyL( aDbIdentifier );
    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerMultiPluginController::ScheduleCompletedSuccesfullyL");	
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerMultiPluginController::HandleScheduleErrorl
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerMultiPluginController::HandleScheduleErrorL( const TUint32 aDbIdentifier,
                                                              TUint32 aCompletitionCode )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerMultiPluginController::HandleScheduleError");
	
	// Here we could try to handle plugin errors. Problem is that scheduler doesn't
	// know anything about schedule it is running and so there cannot be tailored
	// error handling for each plugin. In the end, we just complete schedule
	// to engine with error code (ends up removing schedule from DB).
	
	// Remove schedule from list
    for ( TInt i = 0; i < iScheduleArray.Count(); i ++)
        {
        if( iScheduleArray[i]->DbIdentifier() == aDbIdentifier )
            {
            delete iScheduleArray[i];
            iScheduleArray.Remove( i ); 
            break;               
            }
        }
	
	iEngine.ScheduleCompletedWithErrorL( aDbIdentifier, aCompletitionCode );

	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerMultiPluginController::HandleScheduleError");
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerMultiPluginController::DoCleanUp::DoCleanUp
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerMultiPluginController::DoCleanUp( )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerMultiPluginController::DoCleanUp");
    TBool scheduleFound( EFalse );

    // Check all LandLords. If we found that there is no schedules related to 
    // one we destroy it.
    for (TInt i = iLandLords.Count(); i > 0; i--)
        {
        for ( TInt ii = 0; ii < iScheduleArray.Count(); ii++ )
            {
            // Compare LandLords DbIdentifier with the ones in schedules
            if( iLandLords[i-1]->ThreadPacket()->Schedule()->DbIdentifier() ==
                    iScheduleArray[ii]->DbIdentifier() )
                {
                scheduleFound = ETrue;
                break;
                }            
            }
        // Remove LandLord if there is no schedule for it.
        if( !scheduleFound )
            {
            delete iLandLords[i-1];
            iLandLords.Remove(i-1);
            }
        scheduleFound = EFalse;
        }
   
    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerMultiPluginController::DoCleanUp");	
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerMultiPluginController::ClearPlugin
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerMultiPluginController::ClearControllerL( )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerMultiPluginController::ClearControllerL");
	for( TInt i = 0 ; i < iLandLords.Count() ; i++ )
	    {
	    iLandLords[i]->ClearL();
	    }
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerMultiPluginController::ClearControllerL");
	}	

// -----------------------------------------------------------------------------
// CCseSchedulerMultiPluginController::IsControllerActive
//
// -----------------------------------------------------------------------------
//
TBool CCseSchedulerMultiPluginController::IsControllerActive( )
    {
    TBool retVal( EFalse );
    
    for( TInt i = 0; i < iLandLords.Count(); i++ )
        {
        if( iLandLords[i]->IsWorking() )
            {
            retVal = ETrue ;
            break;
            }            
        }
    return retVal;
    }   

// End of file.
