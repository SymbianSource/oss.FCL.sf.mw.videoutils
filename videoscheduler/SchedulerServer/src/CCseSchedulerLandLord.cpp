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




#include "CCseSchedulerLandLord.h"
#include "CCseSchedulerPluginStarter.h"
#include <ipvideo/CCseScheduledProgram.h>
#include "CCseSchedulerThreadPacket.h"
#include <ipvideo/MCseScheduleObserver.h>   // Observer for informing engine
#include <e32std.h>
#include "CseDebug.h"               // Debug macros
#include "CCseSemaphoreController.h"

_LIT( KCseUniThreadName, "CseThread" );
_LIT( KCseLine,          "-" );

// ============================ MEMBER FUNCTIONS ===============================

// ---------------------------------------------------------------------------
// CCseSchedulerTimer::CCseSchedulerTimer()
//
// ---------------------------------------------------------------------------
CCseSchedulerLandLord::CCseSchedulerLandLord( MCseScheduleObserver* aObserver ) :
                            CActive( CActive::EPriorityStandard ),
                            iThreadRunning( EFalse ),
                            iTakedownRequested( EFalse )

	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::CCseSchedulerLandLord");
	
	// C++ default constructor
	iObserver = aObserver;	
		
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::CCseSchedulerLandLord");
	}

// ---------------------------------------------------------------------------
// CCseSchedulerLandLord::~CCseSchedulerLandLord()
//
// ---------------------------------------------------------------------------	
CCseSchedulerLandLord::~CCseSchedulerLandLord()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::~CCseSchedulerLandLord");

	Cancel();

    delete iShutdownCompleteMonitor;    
    iShutdownCompleteMonitor = NULL;    
    delete iActiveSchedulerWait;
    iActiveSchedulerWait = NULL;    
	iObserver = NULL;
	delete iThreadPacket;
	iThreadPacket = NULL;
	delete iThreadName;
		
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::~CCseSchedulerLandLord");
	}

// ---------------------------------------------------------------------------
// CCseSchedulerLandLord::NewL()
//
// ---------------------------------------------------------------------------	
CCseSchedulerLandLord* CCseSchedulerLandLord::NewL( 
	                        MCseScheduleObserver* aObserver )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::NewL");
	
	// Symbian C++ constructor
	CCseSchedulerLandLord* self = new ( ELeave ) CCseSchedulerLandLord( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( );
    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::NewL");
    return self;
	}
	
// ---------------------------------------------------------------------------
// CCseSchedulerLandLord::ConstructL()
//
// ---------------------------------------------------------------------------
void CCseSchedulerLandLord::ConstructL()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::ConstructL");	

    iActiveSchedulerWait = new (ELeave) CActiveSchedulerWait;    
    CActiveScheduler::Add( this );
  
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::ConstructL");
	}

// ---------------------------------------------------------------------------
// CCseSchedulerLandLord::DoCancel()
//
// ---------------------------------------------------------------------------
void CCseSchedulerLandLord::DoCancel()
	{
	CSELOGSTRING_HIGH_LEVEL( ">>>CCseSchedulerLandLord::DoCancel" );

	if ( iThreadRunning )
	    {
    	CSELOGSTRING_HIGH_LEVEL(
    	    "CCseSchedulerLandLord::DoCancel Canceling thread" );
        
	    // Signal thread to die away!
	    iPluginShutdownSemaphore.Signal();
	
    	iThreadRunning = EFalse;
    	
    	// Complete schedule with KErrAbort
    	TRAP_IGNORE( iObserver->ScheduleCompletedL(
    	    iThreadPacket->Schedule( )->DbIdentifier(), KErrAbort ) );    	
	    }

	CSELOGSTRING_HIGH_LEVEL( "<<<CCseSchedulerLandLord::DoCancel" );
	}

// ---------------------------------------------------------------------------
// CCseSchedulerTimer::RunL()
//
// ---------------------------------------------------------------------------		
void CCseSchedulerLandLord::RunL()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::RunL");
	
    iThreadRunning = EFalse;

    // In case we have not cancelled operation, complete schedule with result code.
    // If we have cancelled it, we're not interested about the result as the
    // schedule has already been removed from the DB
    if( !iTakedownRequested )
        {        
        iObserver->ScheduleCompletedL( iThreadPacket->Schedule( )->DbIdentifier(),
                                       iThreadPacket->ResultCode( ) );
        }
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::RunL");
	}
	
// ---------------------------------------------------------------------------
// CCseSchedulerTimer::SetScheduleL()
//
// ---------------------------------------------------------------------------		
void CCseSchedulerLandLord::SetScheduleL( CCseScheduledProgram& aData )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::SetSchedule");
	
	delete iThreadPacket;
	iThreadPacket = NULL;
	iThreadPacket = CCseSchedulerThreadPacket::NewL();
	iThreadPacket->SetScheduleL( aData );	    

	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::SetSchedule");
	}
		
// ---------------------------------------------------------------------------
// CCseSchedulerTimer::ThreadPacket()
//
// ---------------------------------------------------------------------------		
CCseSchedulerThreadPacket* CCseSchedulerLandLord::ThreadPacket( )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::Schedule");
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::Schedule");
   
    return iThreadPacket;
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerLandLord::RunError
// From CActive, called when RunL leaves.
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerLandLord::RunError( TInt /*aError*/ )
    {
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::RunError");	
    // ATM there isn't leaving code in RunL so we just cancel timer if it is active.    
    Cancel();
    // Return KErrNone to avoid crash.
	return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerLandLord::GenerateThreadName
// Generates "unique" name for thread
// -----------------------------------------------------------------------------
//
void CCseSchedulerLandLord::GenerateThreadNameL( )
    {
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::GenerateThreadName");	

    delete iThreadName;
    iThreadName = NULL;
    iThreadName = KCseUniThreadName().AllocL();
    iThreadName = iThreadName->ReAllocL(
                                iThreadName->Size() +   // Length of original name.
                                sizeof( TUint32 ) * 2 + // Space for Plugin Uid and DbIdentifier
                                2 );                    // Two "-" as a separator between
                                                        // Uid and identifier

    // Separator                                                        
    iThreadName->Des( ).Append( KCseLine );
    
    // Plugin uid
    iThreadName->Des( ).AppendNum( iThreadPacket->Schedule()->PluginUid() );
    
    // Separator
    iThreadName->Des( ).Append( KCseLine );
    
    // Finally schedule identifier to make thread name unique
    iThreadName->Des( ).AppendNum( iThreadPacket->Schedule()->DbIdentifier() );

	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::GenerateThreadName");	
    }

// -----------------------------------------------------------------------------
// CCseSchedulerLandLord::GenerateSemahoresL
// Generates semaphores for shutdown signalling
// -----------------------------------------------------------------------------
//
void CCseSchedulerLandLord::GenerateSemaphoresL( )
    {
    iPluginShutdownSemaphore.CreateLocal( 0 );
    iThreadPacket->SetShutdownSemaphore( iPluginShutdownSemaphore );
    
    iPluginShutdownCompleteSemaphore.CreateLocal( 0 );
    iThreadPacket->SetShutdownCompleteSemaphore( iPluginShutdownCompleteSemaphore );
    }
	
// -----------------------------------------------------------------------------
// CCseSchedulerLandLord::RunPluginL
// From CActive, called when RunL leaves.
// -----------------------------------------------------------------------------
//
void CCseSchedulerLandLord::RunPluginL( )
    {
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::RunPluginL");	
    TInt error( KErrNone );
    
    // Create semaphores for shutdown signalling
    GenerateSemaphoresL();
    
    // First we create unique thread name
    GenerateThreadNameL( );
    
    // Create thread where to run schedule
    error = iThread.Create( *iThreadName,
                             CCseSchedulerPluginStarter::StartThread,
                             KDefaultStackSize,
                             NULL, // uses caller thread's heap
                             iThreadPacket, // Schedule
                             EOwnerThread );
    
    if( error == KErrNone )
        {
        // Creation succesfull, start running it and logon on it.
        CSELOGSTRING_HIGH_LEVEL(
            "CCseSchedulerLandLord::RunPluginL - Thread creation succesfull");
        iThread.Resume();
        iThread.Logon( iStatus );
        SetActive();
        iThreadRunning = ETrue;
        }
    else
        {
        CSELOGSTRING2_HIGH_LEVEL(
            "CCseSchedulerLandLord::RunPluginL - Thread creation FAILED: %d",
             error);
        }

    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::RunPluginL");    
    } 
    
// -----------------------------------------------------------------------------
// CCseSchedulerLandLord::ActiveWait
//
// -----------------------------------------------------------------------------
//	
void CCseSchedulerLandLord::ActiveWait( TBool aStart )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::ActiveWait()");    
    if (aStart)
        {
	    if(!iActiveSchedulerWait->IsStarted())
	        {
	        CSELOGSTRING_HIGH_LEVEL("CCseSchedulerLandLord::ActiveWait - Wait start");    
	        iActiveSchedulerWait->Start();		
	        }
        }
    else
        {
		if(iActiveSchedulerWait->IsStarted())
			{
			CSELOGSTRING_HIGH_LEVEL("CCseSchedulerLandLord::ActiveWait - Wait stop");    
			iActiveSchedulerWait->AsyncStop();		
			}
        }
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::ActiveWait()");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerLandLord::ActiveWait
//
// -----------------------------------------------------------------------------
//	    
void CCseSchedulerLandLord::SemaphoreSignalled()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::ShutdownPlugin()"); 
    
    ActiveWait( EFalse );
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::ShutdownPlugin()"); 
    }
    
// ---------------------------------------------------------------------------
// CCseSchedulerLandLord::ClearL()
//
// ---------------------------------------------------------------------------
void CCseSchedulerLandLord::ClearL()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::ClearL");	

	if( iThreadRunning )
	    {
	    // Flag the takedown. We're cancelling the operation, so we have
	    // no interest about result code as the schedule has already
	    // been removed from the DB.
	    iTakedownRequested = ETrue;
	    
    	// Signal thread to die away!
        iPluginShutdownSemaphore.Signal();
            
        if( !iShutdownCompleteMonitor )
            {
            CSELOGSTRING_HIGH_LEVEL("CCseSchedulerLandLord::ClearL - New semaphore monitor" );
            iShutdownCompleteMonitor = CCseSemaphoreController::NewL( iPluginShutdownCompleteSemaphore, *this );
            }
        
        CSELOGSTRING_HIGH_LEVEL("CCseSchedulerLandLord::ClearL - StartMonitor" );    
        iShutdownCompleteMonitor->Start();    
        
        ActiveWait( ETrue );     
            
        iThreadRunning = EFalse;	
	    }
  
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerLandLord::ClearL");
	}

// -----------------------------------------------------------------------------
// CCseSchedulerLandLord::ActiveWait
//
// -----------------------------------------------------------------------------
//      
TBool CCseSchedulerLandLord::IsWorking()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerLandLord::IsWorking()"); 
    
    CSELOGSTRING2_HIGH_LEVEL("<<<CCseSchedulerLandLord::IsWorking(): %d", iThreadRunning);
    
    return iThreadRunning;
    }

// End of file
