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
* Description:    Timer for the notifier*
*/




// INCLUDE FILES
#include "CCseSchedulerThreadPacket.h"
#include <ipvideo/CCseScheduledProgram.h>   // Represent one schedule in database
#include <e32std.h>
#include "CseDebug.h"               // Debug macros


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
// ---------------------------------------------------------------------------
// CCseSchedulerThreadPacket::CCseSchedulerThreadPacket()
//
// ---------------------------------------------------------------------------
CCseSchedulerThreadPacket::CCseSchedulerThreadPacket( ) :
                           iResultCode( KErrGeneral )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerThreadPacket::CCseSchedulerThreadPacket");
			
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::CCseSchedulerLandLord");
	}

// ---------------------------------------------------------------------------
// CCseSchedulerThreadPacket::~CCseSchedulerThreadPacket()
//
// ---------------------------------------------------------------------------	
CCseSchedulerThreadPacket::~CCseSchedulerThreadPacket()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerThreadPacket::~CCseSchedulerThreadPacket");

    delete iSchedule;
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerThreadPacket::~CCseSchedulerThreadPacket");
	}

// ---------------------------------------------------------------------------
// CCseSchedulerThreadPacket::NewL()
//
// ---------------------------------------------------------------------------	
CCseSchedulerThreadPacket* CCseSchedulerThreadPacket::NewL()
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerThreadPacket::NewL");
	
	// Symbian C++ constructor
	CCseSchedulerThreadPacket* self = new ( ELeave ) CCseSchedulerThreadPacket();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerThreadPacket::NewL");
    return self;
	}
	
// ---------------------------------------------------------------------------
// CCseSchedulerThreadPacket::ConstructL()
//
// ---------------------------------------------------------------------------
void CCseSchedulerThreadPacket::ConstructL()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerThreadPacket::ConstructL");	

  
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerThreadPacket::ConstructL");
	}

// ---------------------------------------------------------------------------
// CCseSchedulerThreadPacket::SetScheduleL()
//
// ---------------------------------------------------------------------------		
void CCseSchedulerThreadPacket::SetScheduleL( CCseScheduledProgram& aData )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerThreadPacket::SetSchedule");

    delete iSchedule;
    iSchedule = NULL;
    iSchedule = CCseScheduledProgram::NewL( aData );

	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerThreadPacket::SetSchedule");
	}
	
// ---------------------------------------------------------------------------
// CCseSchedulerThreadPacket::SetResultCode()
//
// ---------------------------------------------------------------------------		
void CCseSchedulerThreadPacket::SetResultCode( TInt32 aResultCode )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerThreadPacket::SetResultCode");

    iResultCode = aResultCode;

	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerThreadPacket::SetResultCode");
	}	

// ---------------------------------------------------------------------------
// CCseSchedulerThreadPacket::Schedule()
//
// ---------------------------------------------------------------------------		
CCseScheduledProgram* CCseSchedulerThreadPacket::Schedule( )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerThreadPacket::Schedule");
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerThreadPacket::Schedule");

    return iSchedule;	
	}

// ---------------------------------------------------------------------------
// CCseSchedulerThreadPacket::ResultCode()
//
// ---------------------------------------------------------------------------		
TInt32 CCseSchedulerThreadPacket::ResultCode( )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerThreadPacket::ResultCode");
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerThreadPacket::ResultCode");
    return iResultCode;	
	}    
	
// ---------------------------------------------------------------------------
// CCseSchedulerThreadPacket::ShutdownSemaphore()
//
// ---------------------------------------------------------------------------		
RSemaphore& CCseSchedulerThreadPacket::ShutdownSemaphore( )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerThreadPacket::ShutdownSemaphore");
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerThreadPacket::ShutdownSemaphore");
    return iShutdownSemaphore;	
	}   
	
// ---------------------------------------------------------------------------
// CCseSchedulerThreadPacket::ShutdownCompleteSemaphore()
//
// ---------------------------------------------------------------------------		
RSemaphore& CCseSchedulerThreadPacket::ShutdownCompleteSemaphore( )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>ShutdownCompleteSemaphore::ShutdownCompleteSemaphore");
    CSELOGSTRING_HIGH_LEVEL("<<<ShutdownCompleteSemaphore::ShutdownCompleteSemaphore");
    return iShutdownCompleteSemaphore;	
	}

// ---------------------------------------------------------------------------
// CCseSchedulerThreadPacket::ShutdownSemaphore()
//
// ---------------------------------------------------------------------------		
void CCseSchedulerThreadPacket::SetShutdownSemaphore( RSemaphore& aSemaphore )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerThreadPacket::SetShutdownSemaphore");
	iShutdownSemaphore = aSemaphore;
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerThreadPacket::SetShutdownSemaphore");
	}
	
// ---------------------------------------------------------------------------
// CCseSchedulerThreadPacket::ShutdownCompleteSemaphore()
//
// ---------------------------------------------------------------------------		
void CCseSchedulerThreadPacket::SetShutdownCompleteSemaphore( RSemaphore& aSemaphore )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>ShutdownCompleteSemaphore::SetShutdownCompleteSemaphore");
    iShutdownCompleteSemaphore = aSemaphore;
    CSELOGSTRING_HIGH_LEVEL("<<<ShutdownCompleteSemaphore::SetShutdownCompleteSemaphore");    
	}
// End of file
