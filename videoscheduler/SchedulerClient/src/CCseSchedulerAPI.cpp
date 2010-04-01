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
* Description:    Implementation of Scheduler server's client api*
*/



// INCLUDE FILES
#include <ipvideo/CCseSchedulerAPI.h>                   // Header file for this class
#include <ipvideo/CCseScheduledProgram.h>               // Represent one schedule in database
#include <ipvideo/CseSchedulerClientServerCommon.h>     // Common defines for client and server
#include <ipvideo/RCseSchedulerClient.h>                // Client common methods (server start up etc)
#include "CseDebug.h"                           // Debug macros
#include <s32mem.h>

// EXTERNAL DATA STRUCTURES
// None

// EXTERNAL FUNCTION PROTOTYPES  
// None

// CONSTANTS
// None

// MACROS
// None

// LOCAL CONSTANTS AND MACROS
// None

// MODULE DATA STRUCTURES
// None

// LOCAL FUNCTION PROTOTYPES
// None

// FORWARD DECLARATIONS
// None

// ============================ MEMBER FUNCTIONS ===============================
// FUNCTION PROTOTYPES

// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CCseSchedulerApi::CCseSchedulerApi
//
// -----------------------------------------------------------------------------
//
CCseSchedulerApi::CCseSchedulerApi()  
    { 
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerApi::CCseSchedulerApi");

    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerApi::CCseSchedulerApi");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerApi::NewL
// Static two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CCseSchedulerApi* CCseSchedulerApi::NewL()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerApi::NewL");
    CCseSchedulerApi* self = new ( ELeave ) CCseSchedulerApi();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self ); 
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerApi::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerApi::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCseSchedulerApi::ConstructL()
    {    
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerApi::ConstructL");
    User::LeaveIfError( iClient.Connect() );
    User::LeaveIfError( iService.Open( iClient ) );
        
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerApi::ConstructL");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerApi::~CCseSchedulerApi
//
// -----------------------------------------------------------------------------
//
EXPORT_C CCseSchedulerApi::~CCseSchedulerApi()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerApi::~CCseSchedulerApi");
    iService.Close();
    iClient.Close();
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerApi::~CCseSchedulerApi");
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerApi::AddSchedule
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCseSchedulerApi::AddSchedule( CCseScheduledProgram& aData ) const
    {    
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerApi::AddSchedule");
    TRAPD( err, iService.AddScheduleL( aData ) );
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerApi::AddSchedule");
	return err;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerApi::RemoveSchedule
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCseSchedulerApi::RemoveSchedule( const TUint32 aDbIdentifier ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerApi::RemoveSchedule");
    TRAPD( err, iService.RemoveScheduleL( aDbIdentifier ) );
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerApi::RemoveSchedule");
	return err;
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerApi::GetSchedule
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCseSchedulerApi::GetSchedule( const TUint32 aDbIdentifier,
											  CCseScheduledProgram* aProg ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerApi::GetSchedule");
    TRAPD( err, iService.GetScheduleL( aDbIdentifier, aProg ) );
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerApi::GetSchedule");
	return err;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerApi::GetSchedulesByAppUid
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCseSchedulerApi::GetSchedulesByAppUid( const TInt32 aAppUid, 
					RPointerArray<CCseScheduledProgram>& aArray ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerApi::GetSchedulesByAppUid");
    TRAPD( err, iService.GetSchedulesL( aAppUid, aArray ) );
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerApi::GetSchedulesByAppUid");
	return err;
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerApi::GetSchedulesByPluginUid
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCseSchedulerApi::GetSchedulesByPluginUid( const TInt32 aPluginUid,
                                                         RPointerArray<CCseScheduledProgram>& aArray ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerApi::GetSchedulesByPluginUid");
    TRAPD( err, iService.GetSchedulesByPluginUidL( aPluginUid, aArray ) );
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerApi::GetSchedulesByPluginUid");
	return err;
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerApi::GetSchedulesByScheduleType
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCseSchedulerApi::GetSchedulesByType( const TInt32 aType, 
                                                    RPointerArray<CCseScheduledProgram>& aArray ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerApi::GetSchedulesByType");
    TRAPD( err, iService.GetSchedulesByTypeL( aType, aArray ) );
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerApi::GetSchedulesByType");
	return err;
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerApi::GetSchedulesByTime
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCseSchedulerApi::GetSchedulesByTime( const TTime& aBeginning,
                                                    const TTime& aEnd,
                                                    RPointerArray<CCseScheduledProgram>& aArray ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerApi::GetSchedulesByTime");
    TRAPD( err, iService.GetSchedulesByTimeL( aBeginning, aEnd, aArray ) );
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerApi::GetSchedulesByTime");
	return err;
    }    
    

// -----------------------------------------------------------------------------
// CCseSchedulerApi::GetOverlappingSchedule
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCseSchedulerApi::GetOverlappingSchedules( CCseScheduledProgram& aProgram,
                                                         RPointerArray<CCseScheduledProgram>& aResultArray )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerApi::GetOverlappingSchedules");
	TRAPD( err, iService.GetOverlappingSchedulesL( aProgram, aResultArray ) );
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerApi::GetOverlappingSchedules");
	return err;
	}

// End of File
