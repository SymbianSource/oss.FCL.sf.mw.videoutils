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
* Description:    Actual intelligence of scheduler server*
*/





// INCLUDE FILES
#include <s32mem.h>
#include <e32def.h>
#include <e32const.h>
#include "CCseSchedulerServerEngine.h"      // Common methods for server
#include <ipvideo/CseSchedulerClientServerCommon.h> // Common defines for client and server
#include <ipvideo/CCseScheduledProgram.h>           // Represent one schedule in database
#include "CseDebug.h"                       // Debug macros
#include "CCseScheduleDB.h"                 // Schedule database
#include "CCseSchedulerTimer.h"             // Schedule timer for determining schedule times
#include <ipvideo/CCseSchedulerPluginIF.h>          // Scheduler plugin interface
#include "CCseSchedulerUniPluginController.h"
#include "CCseSchedulerMultiPluginController.h"
#include "CCseSchedulerServer.h"


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
// CCseSchedulerServerEngine::CCseSchedulerServerEngine
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCseSchedulerServerEngine::CCseSchedulerServerEngine( CCseSchedulerServer& aServer ) :
    iServer( aServer )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::CCseSchedulerServerEngine");
        
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::CCseSchedulerServerEngine");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCseSchedulerServerEngine* CCseSchedulerServerEngine::NewL( CCseSchedulerServer& aServer ) 
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::NewL");
    
    CCseSchedulerServerEngine* self = new( ELeave ) CCseSchedulerServerEngine( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::NewL");
    return self;
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::ConstructL()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::ConstructL");
        
    iDb = CCseScheduleDB::NewL();
    iCompletitionBreaker = CIdle::NewL( CActive::EPriorityLow );
    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::ConstructL");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::~CCseSchedulerServerEngine
// Destructor.
// -----------------------------------------------------------------------------
//
CCseSchedulerServerEngine::~CCseSchedulerServerEngine()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::~CCseSchedulerServerEngine");
    
    delete iDb;
    	
	iPluginControllerArray.ResetAndDestroy();
	iPluginControllerArray.Close();
    delete iCompletitionBreaker;    
		
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::~CCseSchedulerServerEngine");
    }


// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GeneralServiceL
//
// Schedule operations
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GeneralServiceL( const RMessage2& aMessage )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GeneralService");
	
	switch( aMessage.Function() )
		{
		case ECseAddSchedule:
			{
			AddScheduleL( aMessage );
			break; 		
			}
		case ECseRemoveSchedule:
			{
			RemoveScheduleL( aMessage );
			break;
			}
		case ECseGetSchedules:
			{
			GetSchedulesL( aMessage );	
			break;
			}
		case ECseGetSchedule:
			{
			GetScheduleL( aMessage );
			break;
			}
		case ECseGetScheduleLength:
			{
			GetScheduleLengthL( aMessage );
			break;
			}
		case ECseGetSchedulesByAppUid:
			{
			GetSchedulesByAppUidL( aMessage );
			break;
			}
		case ECseGetScheduleArrayLengthByAppUid:
			{
			GetScheduleArrayLengthByAppUidL( aMessage );
			break;
			}
		case ECseGetOverlappingSchedulesLength:
			{
			GetOverlappingSchedulesLengthL( aMessage );
			break;
			}
		case ECseGetOverlappingSchedules:
			{
			GetOverlappingSchedulesL( aMessage );
			break;
			}			
		case ECseGetScheduleArrayLengthByPluginUid:
			{
			GetScheduleArrayLengthByPluginUidL( aMessage );
			break;
			}
		case ECseGetSchedulesByPluginUid:
			{
			GetSchedulesByPluginUidL( aMessage );
			break;
			}
		case ECseGetScheduleArrayLengthByType:
			{
			GetScheduleArrayLengthByTypeL( aMessage );
			break;
			}
		case ECseGetSchedulesByType:
			{
			GetSchedulesByTypeL( aMessage );
			break;
			}
		case ECseGetScheduleArrayLengthByTimeframe:		     
			{
			GetScheduleArrayLengthByTimeframeL( aMessage );
			break;
			}
		case ECseGetSchedulesByTimeframe:
			{
			GetSchedulesByTimeframeL( aMessage );
			break;
			}
			
		default:
			{
			aMessage.Complete( KErrNotSupported ); 
		    break; 
			}
		}
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::GeneralService");
	}

	
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::PanicClient
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::PanicClient( TCseSchedulerServPanic aPanic,
											 const RMessage2& aMessage ) const
    {    
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::PanicClient");
    
    // Panic the client on server side
    _LIT( KTxtClientFault, "CseSchedulerServer" );
    aMessage.Panic( KTxtClientFault, aPanic );
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::PanicClient");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::AddScheduleL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::AddScheduleL( const RMessage2& aMessage )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::AddScheduleL");
	TInt ret( KErrNone );
	TBuf8<4> id;
	
	// Initialize buffers...
	CCseScheduledProgram* schedule = CCseScheduledProgram::NewL();
	CleanupStack::PushL( schedule );
	HBufC8* msg = HBufC8::NewL( aMessage.GetDesLengthL( 0 ) );
	CleanupStack::PushL( msg );
	TPtr8 ptr( msg->Des() );
	
	// Read schedule as a stream format to buffer
	aMessage.ReadL( 0, ptr, 0 );
			
	RDesReadStream readStream;
	CleanupClosePushL( readStream );
	readStream.Open( ptr );
	
	// Change stream formatted schedule to "CseScheduledProgram"-format
	schedule->InternalizeL( readStream );
	
	CleanupStack::PopAndDestroy( &readStream );
	
	// Add it to DB	
	iDb->AddScheduleL( *schedule );
	
	// Make the completition value
	id.FillZ();
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
	writeStream.Open( id );
	writeStream.WriteUint32L( schedule->DbIdentifier() );
	writeStream.CommitL();
	CleanupStack::PopAndDestroy( &writeStream );
	
	aMessage.WriteL( 1, id, 0 );

    RescheduleByPluginL( schedule->PluginUid() );

	CleanupStack::PopAndDestroy( msg );
	CleanupStack::PopAndDestroy( schedule );

    aMessage.Complete( ret );    
    CSELOGSTRING2_HIGH_LEVEL("<<<CCseSchedulerServerEngine::AddScheduleL, ret=%d", ret);
	}

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::RemoveScheduleL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::RemoveScheduleL( const RMessage2& aMessage )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::RemoveScheduleL");
	
	TUint32 dbIdentifier( 0 );
	TBuf8<4> idBuff;
	TInt32 pluginUid( 0 );
	
	idBuff.FillZ();
	
	// Read db identifier from aMessager
	User::LeaveIfError( aMessage.Read( 0, idBuff ) );
	
	RDesReadStream readStream;
	CleanupClosePushL( readStream );
	readStream.Open( idBuff );
	dbIdentifier = readStream.ReadUint32L();
	CleanupStack::PopAndDestroy( &readStream );
	
	// Remove schedule
	// First check the plugin UID of removed schedule
	CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
	CleanupStack::PushL( prog );
	iDb->GetScheduleByDbIdentifierL( dbIdentifier, prog );
	pluginUid = prog->PluginUid( );
	// Now remove it from DB
	iDb->RemoveScheduleL( dbIdentifier );	
	RescheduleByPluginL( pluginUid );
	
	// Use completition callback to remove those controllers that aren't needed anymore
	if( !iCompletitionBreaker->IsActive() )
        {            
        iCompletitionBreaker->Start(TCallBack( CompletitionCallback, this ));
        }
    else
        {
        CSELOGSTRING_HIGH_LEVEL(
            "CCseSchedulerServerEngine::ScheduleCompletedSuccesfully - Breaker already active");
        }	
	
	CleanupStack::PopAndDestroy( prog );
	aMessage.Complete( KErrNone );	
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::RemoveScheduleL");
	}

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetScheduleL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetScheduleL( const RMessage2& aMessage )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetScheduleL");
	TUint32 dbIdentifier( 0 );
	TBuf8<4> idBuff;
	
	idBuff.FillZ();
	
	// Read db identifier from aMessage
	TInt err( aMessage.Read( 0, idBuff ) );
		
	if ( err == KErrNone )
		{
		// Create new db identifier to be passed to client
		CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
		CleanupStack::PushL( prog );

		RDesReadStream readStream;
		CleanupClosePushL( readStream );
		readStream.Open( idBuff );
		dbIdentifier = readStream.ReadUint32L();
		CleanupStack::PopAndDestroy( &readStream );
		
		// Get schedule
		iDb->GetScheduleByDbIdentifierL( dbIdentifier, prog );
		
		TInt length( prog->ExternalizeLength() );
		
		// Externalize schedule to stream format
		HBufC8* retBuff = HBufC8::NewL( length );
		CleanupStack::PushL( retBuff );
		TPtr8 retPtr( retBuff->Des() );
		RDesWriteStream writeStream;
		CleanupClosePushL( writeStream );
		writeStream.Open( retPtr );
		prog->ExternalizeL( writeStream );
		writeStream.CommitL();
		CleanupStack::PopAndDestroy( &writeStream );
		
		User::LeaveIfError( aMessage.Write( 1, retPtr ) );
		CleanupStack::PopAndDestroy( retBuff );
		CleanupStack::PopAndDestroy( prog );
		}
	
	aMessage.Complete( err );
	CSELOGSTRING2_HIGH_LEVEL("<<<CCseSchedulerServerEngine::RemoveScheduleL, err=%d", err);
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetScheduleLengthL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetScheduleLengthL( const RMessage2& aMessage )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetScheduleLengthL");
	TUint32 dbIdentifier( 0 );
	TBuf8<4> idBuff;
	TBuf8<4> lenBuff;
	TInt length( 0 );
	CCseScheduledProgram* schedule = CCseScheduledProgram::NewL();
	CleanupStack::PushL( schedule );
	idBuff.FillZ();
	
	// Read db identifier from aMessage
	User::LeaveIfError( aMessage.Read( 0, idBuff ) );
	
	RDesReadStream readStream;
	CleanupClosePushL( readStream );
	readStream.Open( idBuff );
	dbIdentifier = readStream.ReadUint32L();
	CleanupStack::PopAndDestroy( &readStream );
	
	// Get schedule
	iDb->GetScheduleByDbIdentifierL( dbIdentifier, schedule );
	
	// Get schedule length
	length = schedule->ExternalizeLength();
	
	lenBuff.FillZ();
	
	// Write length in stream so we can pass it to client
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
	writeStream.Open( lenBuff );
    writeStream.WriteInt32L( length );
	writeStream.CommitL();
    CleanupStack::PopAndDestroy( &writeStream );
    
    User::LeaveIfError( aMessage.Write( 1, lenBuff ) );
    
    CleanupStack::PopAndDestroy( schedule );
	
	aMessage.Complete( KErrNone );
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::GetScheduleLengthL");
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetScheduleLengthL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetScheduleArrayLengthByAppUidL( 
											const RMessage2& aMessage )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetScheduleArrayLengthByAppUidL");
	TInt32 appUid( 0 );
	TBuf8<4> idBuff;
	TBuf8<4> lenBuff;
	TInt length( 0 );
	idBuff.FillZ();
	
	// Read application uid from aMessage
	User::LeaveIfError( aMessage.Read( 0, idBuff ) );
	
	RDesReadStream readStream;
	CleanupClosePushL( readStream );
	readStream.Open( idBuff );
	appUid = readStream.ReadInt32L();
	CleanupStack::PopAndDestroy( &readStream );

    // Get schedules from database
	RPointerArray<CCseScheduledProgram> progArray;
	CleanupResetAndDestroyPushL( progArray );
	iDb->GetApplicationSchedulesL( appUid, progArray );
	
	// Calculate total length
	// Number of items in array
	length = length + 4; // Size of TInt32
	// Data from array
	for ( TInt i = 0 ; i < progArray.Count(); i++ )
		{
		length = length + progArray[i]->ExternalizeLength();
		}
	CleanupStack::PopAndDestroy( &progArray ); // closes progArray
	
	lenBuff.FillZ();
	
	// Change length value to stream so that we can send it back to client.
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
	writeStream.Open( lenBuff );
    writeStream.WriteInt32L( length );
    CleanupStack::PopAndDestroy( &writeStream );
    
    User::LeaveIfError( aMessage.Write( 1, lenBuff ) );
    
  	aMessage.Complete( KErrNone );	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::GetScheduleArrayLengthByAppUidL");
	}

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetSchedulesByAppUidL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetSchedulesByAppUidL( const RMessage2& aMessage )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetSchedulesByAppUidL");
	TInt32 appUid( 0 );
	TBuf8<4> idBuff;
	idBuff.FillZ();
	TInt32 numberOfElems( 0 );
	TInt length( 0 );
	
	// Read application UID from aMessage
	TInt err( aMessage.Read( 0, idBuff ) );
		
	if ( err == KErrNone )
		{
		RPointerArray<CCseScheduledProgram> progArray;
		CleanupResetAndDestroyPushL( progArray );
		RDesReadStream readStream;
		CleanupClosePushL( readStream );
		readStream.Open( idBuff );
		appUid = readStream.ReadInt32L();
		CleanupStack::PopAndDestroy( &readStream );
		
		// Get schedules from database
		iDb->GetApplicationSchedulesL( appUid, progArray );
		
		// Calculate total length
		// Number of items in array
		length = length + 4; // Size of TInt32
		// Data from array
		for ( TInt i = 0 ; i < progArray.Count(); i++ )
			{		
			length += progArray[i]->ExternalizeLength();
			}
		
		// Allocate write stream
		HBufC8* retBuff = HBufC8::NewL( length );
		CleanupStack::PushL( retBuff );
		TPtr8 retPtr( retBuff->Des() );
		RDesWriteStream writeStream;
		CleanupClosePushL( writeStream );
		writeStream.Open( retPtr );
		
		// Fill stream
		// Write number elems
		numberOfElems = progArray.Count();
		writeStream.WriteInt32L( numberOfElems );		
		// Add elems one by one
		for ( TInt i = 0; i < progArray.Count(); i++ )
			{
			progArray[i]->ExternalizeL( writeStream );
			}
		writeStream.CommitL();
		
		CleanupStack::PopAndDestroy( &writeStream );
		
		User::LeaveIfError( aMessage.Write( 1, retPtr ) );	
		
		CleanupStack::PopAndDestroy( retBuff );
		CleanupStack::PopAndDestroy( &progArray );
		}

	aMessage.Complete( err );
	CSELOGSTRING2_HIGH_LEVEL("<<<CCseSchedulerServerEngine::GetSchedulesByAppUidL", err);
	}

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetSchedulesL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetSchedulesL( const RMessage2& aMessage )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetSchedulesL");
	aMessage.Complete( KErrNotSupported );
	}
    
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetOverlappingSchedulesLengthL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetOverlappingSchedulesLengthL( 
											const RMessage2& aMessage )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetOverlappingSchedulesLengthL");
	TInt32 scheduleType( 0 );
	TBuf8<20> idBuff;
	TBuf8<sizeof( TUint32 )> retBuf; // contains length
	
	TUint32 length( 0 );
	
	idBuff.FillZ();
	
	// Read schedule type from aMessage
	User::LeaveIfError( aMessage.Read( 0, idBuff ) );
	
	RDesReadStream readStream;
	CleanupClosePushL( readStream );
	readStream.Open( idBuff );
	scheduleType = readStream.ReadInt32L();

	TUint32 lower( 0 );
	TUint32 upper( 0 );
	
	// Read start time from the aMessage
    lower = readStream.ReadUint32L();
    upper = readStream.ReadUint32L();
    TInt64 time( 0 );
    time = (TInt64) lower;    
    TInt64 longUpper = (TInt64) upper;
    longUpper = longUpper << 32;
    longUpper &= (0xFFFFFFFF00000000ULL);    
    time |= longUpper;
    TInt64 startTime = time;  
        
	// Read end time from the aMessage
	lower = readStream.ReadUint32L();
    upper = readStream.ReadUint32L();
    time = (TInt64) lower;    
    longUpper = (TInt64) upper;
    longUpper = longUpper << 32;
    longUpper &= (0xFFFFFFFF00000000ULL);    
    time |= longUpper;
    TInt64 endTime = time;  

	CleanupStack::PopAndDestroy( &readStream );

	RPointerArray<CCseScheduledProgram> schedules;
	CleanupResetAndDestroyPushL( schedules );
	
	// Get overlapping schedules
	iDb->GetOverlappingSchedulesL( scheduleType, startTime, endTime, schedules );
	
	// Count length one by one
	for ( TInt i = 0; i < schedules.Count(); i++ )
		{
		length += schedules[i]->ExternalizeLength();
		}
	
	CleanupStack::PopAndDestroy( &schedules );	

	retBuf.FillZ();
	
	
	// Write length to stream so we can pass it to client
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
	writeStream.Open( retBuf );
    writeStream.WriteInt32L( length );
	writeStream.CommitL();
    CleanupStack::PopAndDestroy( &writeStream );
    
    User::LeaveIfError( aMessage.Write( 1, retBuf ) );

	aMessage.Complete( KErrNone );
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::GetOverlappingSchedulesLengthL");
	}


// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetOverlappingSchedulesL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetOverlappingSchedulesL( 
											const RMessage2& aMessage )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetOverlappingSchedulesL");
	TInt32 scheduleType( 0 );
	TBuf8<20> idBuff;
	
	idBuff.FillZ();
	
	// Read schedule type from aMessage
	User::LeaveIfError( aMessage.Read( 0, idBuff ) );
	
	RDesReadStream readStream;
	CleanupClosePushL( readStream );
	readStream.Open( idBuff );
	scheduleType = readStream.ReadInt32L();

	TUint32 lower( 0 );
	TUint32 upper( 0 );
	
	// Read start time from aMessage
    lower = readStream.ReadUint32L();
    upper = readStream.ReadUint32L();
    TInt64 time( 0 );
    time = (TInt64) lower;    
    TInt64 longUpper = (TInt64) upper;
    longUpper = longUpper << 32;
    longUpper &= (0xFFFFFFFF00000000ULL);    
    time |= longUpper;
    TInt64 startTime = time;  
        
	//Read end time from aMessage
	lower = readStream.ReadUint32L();
    upper = readStream.ReadUint32L();
    time = (TInt64) lower;    
    longUpper = (TInt64) upper;
    longUpper = longUpper << 32;
    longUpper &= (0xFFFFFFFF00000000ULL);    
    time |= longUpper;
    TInt64 endTime = time;  

	CleanupStack::PopAndDestroy( &readStream );

	RPointerArray<CCseScheduledProgram> schedules;
	CleanupResetAndDestroyPushL( schedules );
	
	// Get overlapping schedules from database
	iDb->GetOverlappingSchedulesL( scheduleType, startTime, endTime, schedules );
	
	TUint32 length( 0 );

	length += sizeof( TInt32 ); // item count added to buffer first
	
	// Length of the schedules needed for transfer stream
	for ( TInt i = 0; i < schedules.Count(); i++ )
		{
		length += schedules[i]->ExternalizeLength();
		}
	
	HBufC8* retBuf = HBufC8::NewLC( length );
	TPtr8 retBufPtr( retBuf->Des() );

	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
	writeStream.Open( retBufPtr );
	// write item count to buffer
    writeStream.WriteInt32L( schedules.Count() );
	
	// write all items to buffer
	for ( TInt j = 0; j < schedules.Count(); j++ )
		{
		schedules[j]->ExternalizeL( writeStream );
		}
	writeStream.CommitL();
    CleanupStack::PopAndDestroy( &writeStream );

	User::LeaveIfError( aMessage.Write( 1, retBufPtr ) );

    CleanupStack::PopAndDestroy( retBuf );
	CleanupStack::PopAndDestroy( &schedules );	

	aMessage.Complete( KErrNone );
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::GetOverlappingSchedulesL");
	}



// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::RescheduleL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::RescheduleL()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::Reschedule");
			
	RArray<TInt32> uidArray;
	
	iDb->GetPluginsL( uidArray );
	
	if( uidArray.Count() > 0 )
	    {
	    for(TInt i = 0; i < uidArray.Count(); i++)
	        {
	        TTime runTime( iDb->GetNextScheduleTimeByPluginL( uidArray[i] ) );
	        
	        // Search for existing plugin controller for this schedule
	        RPointerArray<CCseScheduledProgram> schedules;
	        CleanupResetAndDestroyPushL( schedules ); 	        
	        iDb->GetSchedulesByTimeAndPluginL( runTime, uidArray[i], schedules );
	        
	        if( schedules.Count() > 0 )
	            {	        
    	        TBool controllerFound( EFalse );
    	        
    	        // Plugin controller might existCheck if the plugin controller already exists.
    	        for( TInt ii = 0; ii < iPluginControllerArray.Count(); ii++ )
    	            {
    	            if( schedules[0]->PluginUid() == iPluginControllerArray[ii]->PluginUid() )
    	                {
    	                iPluginControllerArray[ii]->SetSchedulesL( schedules );
    	                controllerFound = ETrue;
    	                break;	                
    	                }	                
    	            }
    	        
    	        // It didn't so we create new one
    	        if( controllerFound == EFalse )
    	            {                            
                    if( schedules[0]->PluginType() == CCseScheduledProgram::ECseUniPlugin )
                        {
                        CSELOGSTRING_HIGH_LEVEL(
                            "CCseSchedulerServerEngine::Reschedule: Create new uniplugin controller");
                        // Create new PluginController
                        iPluginControllerArray.AppendL( CCseSchedulerUniPluginController::NewL( *this, schedules[0]->PluginUid() ) );
                        // Add schedules to just created controller
                        iPluginControllerArray[iPluginControllerArray.Count()-1]->SetSchedulesL(schedules);
                        }
                    else if ( schedules[0]->PluginType() == CCseScheduledProgram::ECseMultiPlugin )
                        {
                        CSELOGSTRING_HIGH_LEVEL(
                        "CCseSchedulerServerEngine::Reschedule: Create new multiplugin controller");
                        // Create new PluginController
                        iPluginControllerArray.AppendL( CCseSchedulerMultiPluginController::NewL( *this, schedules[0]->PluginUid() ) );
                        // Add schedules to just created controller
                        iPluginControllerArray[iPluginControllerArray.Count()-1]->SetSchedulesL(schedules);
                        }
                    else
                        {
                        // WTF?!?!
                        CSELOGSTRING_HIGH_LEVEL(
                            "CCseSchedulerServerEngine::Reschedule: Plugin something completely different");
                        }
    	            }
	            }
	        // Delete schedule array.
	        CleanupStack::PopAndDestroy( &schedules );
	        }
	    }
	    
	uidArray.Reset();
	uidArray.Close();
	
    // Stop server if we dont have any clients and there is no more
	// reason for us to live (no schedules active).
	iServer.StopServer();
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::Reschedule out");
	}

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::IsSchedulerActive()
//
// -----------------------------------------------------------------------------
//
TBool CCseSchedulerServerEngine::IsSchedulerActive() const
    {
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::IsSchedulerActive");
	
	TBool ret(EFalse);
	
	if( iPluginControllerArray.Count() > 0 )
        {
        ret = ETrue;
        }
	
    CSELOGSTRING2_HIGH_LEVEL("<<<CCseSchedulerServerEngine::IsSchedulerActive: %d", ret);
	return ret;
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::Close()
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::Close()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::IsSchedulerActive");
	
    while( AccessCount() >= 1 )
        {
        Dec();
        }
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::IsSchedulerActive");
	}

	
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetScheduleArrayLengthByPluginUidL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetScheduleArrayLengthByPluginUidL( const RMessage2& aMessage )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetScheduleArrayLengthByPluginUidL");
	TInt32 pluginUid( 0 );
	TBuf8<4> idBuff;
	TBuf8<4> lenBuff;
	TInt length( 0 );
	idBuff.FillZ();
	
	// Read application uid from aMessage
	User::LeaveIfError( aMessage.Read( 0, idBuff ) );
	
	RDesReadStream readStream;
	CleanupClosePushL( readStream );
	readStream.Open( idBuff );
	pluginUid = readStream.ReadInt32L();
	CleanupStack::PopAndDestroy( &readStream );

    // Get schedules from database
	RPointerArray<CCseScheduledProgram> progArray;
	CleanupResetAndDestroyPushL( progArray );	
	iDb->GetSchedulesByPluginL( pluginUid, progArray);
	
	// Calculate total length
	// Number of items in array
	length = length + 4; // Size of TInt32
	// Data from array
	for ( TInt i = 0 ; i < progArray.Count(); i++ )
		{
		length = length + progArray[i]->ExternalizeLength();
		}
	CleanupStack::PopAndDestroy( &progArray ); // closes progArray
	
	lenBuff.FillZ();
	
	// Change length value to stream so that we can send it back to client.
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
	writeStream.Open( lenBuff );
    writeStream.WriteInt32L( length );
    CleanupStack::PopAndDestroy( &writeStream );
    
    User::LeaveIfError( aMessage.Write( 1, lenBuff ) );
    
  	aMessage.Complete( KErrNone );	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::GetScheduleArrayLengthByPluginUidL");
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetSchedulesByPluginUidL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetSchedulesByPluginUidL( const RMessage2& aMessage )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetSchedulesByPluginUidL");
	TInt32 pluginUid( 0 );
	TBuf8<4> idBuff;
	idBuff.FillZ();
	TInt32 numberOfElems( 0 );
	TInt length( 0 );
	
	// Read application UID from aMessage
	TInt err( aMessage.Read( 0, idBuff ) );
		
	if ( err == KErrNone )
		{
		RPointerArray<CCseScheduledProgram> progArray;
		CleanupResetAndDestroyPushL( progArray );
		RDesReadStream readStream;
		CleanupClosePushL( readStream );
		readStream.Open( idBuff );
		pluginUid = readStream.ReadInt32L();
		CleanupStack::PopAndDestroy( &readStream );
		
		// Get schedules from database
		iDb->GetSchedulesByPluginL( pluginUid, progArray );
		
		// Calculate total length
		// Number of items in array
		length = length + 4; // Size of TInt32
		// Data from array
		for ( TInt i = 0 ; i < progArray.Count(); i++ )
			{		
			length += progArray[i]->ExternalizeLength();
			}
		
		// Allocate write stream
		HBufC8* retBuff = HBufC8::NewL( length );
		CleanupStack::PushL( retBuff );
		TPtr8 retPtr( retBuff->Des() );
		RDesWriteStream writeStream;
		CleanupClosePushL( writeStream );
		writeStream.Open( retPtr );
		
		// Fill stream
		// Write number elems
		numberOfElems = progArray.Count();
		writeStream.WriteInt32L( numberOfElems );		
		// Add elems one by one
		for ( TInt i = 0; i < progArray.Count(); i++ )
			{
			progArray[i]->ExternalizeL( writeStream );
			}
		writeStream.CommitL();
		
		CleanupStack::PopAndDestroy( &writeStream );
		
		User::LeaveIfError( aMessage.Write( 1, retPtr ) );	
		
		CleanupStack::PopAndDestroy( retBuff );
		CleanupStack::PopAndDestroy( &progArray );
		}

	aMessage.Complete( err );
	CSELOGSTRING2_HIGH_LEVEL("<<<CCseSchedulerServerEngine::GetSchedulesByPluginUidL", err);
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetScheduleArrayLengthByTypeL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetScheduleArrayLengthByTypeL( const RMessage2& aMessage )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetScheduleArrayLengthByTypeL");
	TInt32 type( 0 );
	TBuf8<4> idBuff;
	TBuf8<4> lenBuff;
	TInt length( 0 );
	idBuff.FillZ();
	
	// Read application uid from aMessage
	User::LeaveIfError( aMessage.Read( 0, idBuff ) );
	
	RDesReadStream readStream;
	CleanupClosePushL( readStream );
	readStream.Open( idBuff );
	type = readStream.ReadInt32L();
	CleanupStack::PopAndDestroy( &readStream );

    // Get schedules from database
	RPointerArray<CCseScheduledProgram> progArray;
	CleanupResetAndDestroyPushL( progArray );	
	iDb->GetSchedulesByTypeL( type, progArray);
	
	// Calculate total length
	// Number of items in array
	length = length + 4; // Size of TInt32
	// Data from array
	for ( TInt i = 0 ; i < progArray.Count(); i++ )
		{
		length = length + progArray[i]->ExternalizeLength();
		}
	CleanupStack::PopAndDestroy( &progArray ); // closes progArray
	
	lenBuff.FillZ();
	
	// Change length value to stream so that we can send it back to client.
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
	writeStream.Open( lenBuff );
    writeStream.WriteInt32L( length );
    CleanupStack::PopAndDestroy( &writeStream );
    
    User::LeaveIfError( aMessage.Write( 1, lenBuff ) );
    
  	aMessage.Complete( KErrNone );	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::GetScheduleArrayLengthByTypeL");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetSchedulesByTypeL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetSchedulesByTypeL( const RMessage2& aMessage )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetSchedulesByTypeL");
	TInt32 type( 0 );
	TBuf8<4> idBuff;
	idBuff.FillZ();
	TInt32 numberOfElems( 0 );
	TInt length( 0 );
	
	// Read application UID from aMessage
	TInt err( aMessage.Read( 0, idBuff ) );
		
	if ( err == KErrNone )
		{
		RPointerArray<CCseScheduledProgram> progArray;
		CleanupResetAndDestroyPushL( progArray );
		RDesReadStream readStream;
		CleanupClosePushL( readStream );
		readStream.Open( idBuff );
		type = readStream.ReadInt32L();
		CleanupStack::PopAndDestroy( &readStream );
		
		// Get schedules from database
		iDb->GetSchedulesByTypeL( type, progArray );
		
		// Calculate total length
		// Number of items in array
		length = length + 4; // Size of TInt32
		// Data from array
		for ( TInt i = 0 ; i < progArray.Count(); i++ )
			{		
			length += progArray[i]->ExternalizeLength();
			}
		
		// Allocate write stream
		HBufC8* retBuff = HBufC8::NewL( length );
		CleanupStack::PushL( retBuff );
		TPtr8 retPtr( retBuff->Des() );
		RDesWriteStream writeStream;
		CleanupClosePushL( writeStream );
		writeStream.Open( retPtr );
		
		// Fill stream
		// Write number elems
		numberOfElems = progArray.Count();
		writeStream.WriteInt32L( numberOfElems );		
		// Add elems one by one
		for ( TInt i = 0; i < progArray.Count(); i++ )
			{
			progArray[i]->ExternalizeL( writeStream );
			}
		writeStream.CommitL();
		
		CleanupStack::PopAndDestroy( &writeStream );
		
		User::LeaveIfError( aMessage.Write( 1, retPtr ) );	
		
		CleanupStack::PopAndDestroy( retBuff );
		CleanupStack::PopAndDestroy( &progArray );
		}

	aMessage.Complete( err );
	CSELOGSTRING2_HIGH_LEVEL("<<<CCseSchedulerServerEngine::GetSchedulesByTypeL", err);
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetScheduleArrayLengthByTimeframeL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetScheduleArrayLengthByTimeframeL( const RMessage2& aMessage )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetScheduleArrayLengthByTimeframeL");	
	TBuf8<20> idBuff;
	TBuf8<sizeof( TUint32 )> retBuf; // contains length
	
	TUint32 length( 0 );
	
	idBuff.FillZ();
	
	// Read schedule type from aMessage
	User::LeaveIfError( aMessage.Read( 0, idBuff ) );
	
	RDesReadStream readStream;
	CleanupClosePushL( readStream );
	readStream.Open( idBuff );

	TUint32 lower( 0 );
	TUint32 upper( 0 );
	
	// Read start time from the aMessage
    lower = readStream.ReadUint32L();
    upper = readStream.ReadUint32L();
    TInt64 time( 0 );
    time = (TInt64) lower;    
    TInt64 longUpper = (TInt64) upper;
    longUpper = longUpper << 32;
    longUpper &= (0xFFFFFFFF00000000ULL);    
    time |= longUpper;
    TInt64 beginning = time;  
        
	// Read end time from the aMessage
	lower = readStream.ReadUint32L();
    upper = readStream.ReadUint32L();
    time = (TInt64) lower;    
    longUpper = (TInt64) upper;
    longUpper = longUpper << 32;
    longUpper &= (0xFFFFFFFF00000000ULL);    
    time |= longUpper;
    TInt64 end = time;  

	CleanupStack::PopAndDestroy( &readStream );

	RPointerArray<CCseScheduledProgram> schedules;
	CleanupResetAndDestroyPushL( schedules );
	
	// Get overlapping schedules
	iDb->GetSchedulesByTimeframeL( beginning, end, schedules );
	
	// Count length one by one
	for ( TInt i = 0; i < schedules.Count(); i++ )
		{
		length += schedules[i]->ExternalizeLength();
		}
	
	CleanupStack::PopAndDestroy( &schedules );	

	retBuf.FillZ();
	
	
	// Write length to stream so we can pass it to client
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
	writeStream.Open( retBuf );
    writeStream.WriteInt32L( length );
	writeStream.CommitL();
    CleanupStack::PopAndDestroy( &writeStream );
    
    User::LeaveIfError( aMessage.Write( 1, retBuf ) );

	aMessage.Complete( KErrNone );
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::GetScheduleArrayLengthByTimeframeL");	
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::GetSchedulesByTimeframeL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::GetSchedulesByTimeframeL( const RMessage2& aMessage )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::GetSchedulesByTimeframeL");
	TBuf8<20> idBuff;
	
	idBuff.FillZ();
	
	// Read schedule type from aMessage
	User::LeaveIfError( aMessage.Read( 0, idBuff ) );
	
	RDesReadStream readStream;
	CleanupClosePushL( readStream );
	readStream.Open( idBuff );

	TUint32 lower( 0 );
	TUint32 upper( 0 );
	
	// Read start time from aMessage
    lower = readStream.ReadUint32L();
    upper = readStream.ReadUint32L();
    TInt64 time( 0 );
    time = (TInt64) lower;    
    TInt64 longUpper = (TInt64) upper;
    longUpper = longUpper << 32;
    longUpper &= (0xFFFFFFFF00000000ULL);    
    time |= longUpper;
    TInt64 beginning = time;  
        
	//Read end time from aMessage
	lower = readStream.ReadUint32L();
    upper = readStream.ReadUint32L();
    time = (TInt64) lower;    
    longUpper = (TInt64) upper;
    longUpper = longUpper << 32;
    longUpper &= (0xFFFFFFFF00000000ULL);    
    time |= longUpper;
    TInt64 end = time;  

	CleanupStack::PopAndDestroy( &readStream );

	RPointerArray<CCseScheduledProgram> schedules;
	CleanupResetAndDestroyPushL( schedules );
	
	// Get overlapping schedules from database
	iDb->GetSchedulesByTimeframeL( beginning, end, schedules );
	
	TUint32 length( 0 );

	length += sizeof( TInt32 ); // item count added to buffer first
	
	// Length of the schedules needed for transfer stream
	for ( TInt i = 0; i < schedules.Count(); i++ )
		{
		length += schedules[i]->ExternalizeLength();
		}
	
	HBufC8* retBuf = HBufC8::NewLC( length );
	TPtr8 retBufPtr( retBuf->Des() );

	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
	writeStream.Open( retBufPtr );
	// write item count to buffer
    writeStream.WriteInt32L( schedules.Count() );
	
	// write all items to buffer
	for ( TInt j = 0; j < schedules.Count(); j++ )
		{
		schedules[j]->ExternalizeL( writeStream );
		}
	writeStream.CommitL();
    CleanupStack::PopAndDestroy( &writeStream );

	User::LeaveIfError( aMessage.Write( 1, retBufPtr ) );

    CleanupStack::PopAndDestroy( retBuf );
	CleanupStack::PopAndDestroy( &schedules );	

	aMessage.Complete( KErrNone );
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::GetSchedulesByTimeframeL");
    }

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::ScheduleCompletedSuccesfullyL()
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::ScheduleCompletedSuccesfullyL( TUint32 aDbIdentifier )
	{		
	CSELOGSTRING2_HIGH_LEVEL(
	    ">>>CCseSchedulerServerEngine::ScheduleCompletedSuccesfully: aDbIdentifier: %d",
	    aDbIdentifier);
	    
    TInt32 pluginUid( 0 );    
    
    // First get the plugin UID of removed schedule
	CCseScheduledProgram* prog = CCseScheduledProgram::NewL() ;
	CleanupStack::PushL( prog );
	iDb->GetScheduleByDbIdentifierL( aDbIdentifier, prog );
	pluginUid = prog->PluginUid( );		
	CleanupStack::PopAndDestroy( prog );

    // Remove completed schedule from DB
    TRAPD( err, iDb->RemoveScheduleL( aDbIdentifier ) );
    if ( err != KErrNone )
    	{
    	CSELOGSTRING2_HIGH_LEVEL(
    	    "CCseSchedulerServerEngine::ScheduleCompletedSuccesfully - iDb->RemoveScheduleL() LEAVED: %d",
    	     err);
    	}

    // Reschedule completed plugin    	
    if( pluginUid != 0 )
        {
        // We have plugin Uid, reschedule that controller
        RescheduleByPluginL( pluginUid );
        }
    else
        {
        // No plugin uid found, reschedule all.
        RescheduleL();
        }

    if( !iCompletitionBreaker->IsActive() )
        {            
        iCompletitionBreaker->Start(TCallBack( CompletitionCallback, this ));
        }
    else
        {
        CSELOGSTRING_HIGH_LEVEL(
            "CCseSchedulerServerEngine::ScheduleCompletedSuccesfully - Breaker already active");
        }
   
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::ScheduleCompletedSuccesfully");
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::ScheduleCompletedWithError()
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::ScheduleCompletedWithErrorL( 
    TUint32 aDbIdentifier,
    TUint32 aErrorCode )
    {	
	CSELOGSTRING3_HIGH_LEVEL(
	    ">>>CCseSchedulerServerEngine::ScheduleCompletedWithError: aDbIdentifier: %d, aError %d",
	    aDbIdentifier,
	    aErrorCode);
	TInt32 pluginUid( 0 );
	    
    // First get the plugin UID of removed schedule
	CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
	CleanupStack::PushL( prog );
	iDb->GetScheduleByDbIdentifierL( aDbIdentifier, prog );
	pluginUid = prog->PluginUid( );		
	CleanupStack::PopAndDestroy( prog );	
 
	// Something strange happened while executing schedule. Remove it from database.	
	TRAPD( err, iDb->RemoveScheduleL( aDbIdentifier ) );
	if ( err != KErrNone )
		{
		CSELOGSTRING2_HIGH_LEVEL(
		    ">>>CCseSchedulerServerEngine::ScheduleCompletedWithError: iDb->RemoveScheduleL Leaved: %d",
		    err);
		}

	// Reschedule completed plugin    	
    if( pluginUid == 0 )
        {
        // We have plugin Uid, reschedule that controller
        RescheduleByPluginL( pluginUid );
        }
    else
        {
        // No plugin uid found, reschedule all.
        RescheduleL();
        }

#if CSE_LOGGING_METHOD == 0 
    ( void )aErrorCode;
#endif
    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::ScheduleCompletedWithError");
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::IncreaseScheduleRunCount( )
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::IncreaseScheduleRunCountL( TUint32 aDbIdentifier )
	{	
	CSELOGSTRING2_HIGH_LEVEL(
	    ">>>CCseSchedulerServerEngine::IncreaseScheduleRunCountL: aDbIdentifier: %d",
	    aDbIdentifier );
 
    TRAPD( err, iDb->IncreaseRunCountL( aDbIdentifier ) );
    
    if (err != KErrNone )
        {
        CSELOGSTRING2_HIGH_LEVEL(
            "CCseSchedulerServerEngine::IncreaseScheduleRunCount - IncreaseRunCount failed: %d. Removing from DB", err);
        
        // IncreaseRunCount leaved. It is crucial for us that IncreaseRunCount passes.
        // We use it to prevent scheduling engine to run certain malicious software.
        iDb->RemoveScheduleL( aDbIdentifier );
        }
            
    
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::IncreaseScheduleRunCountL");
	}
	    
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::IsAllowedToRun( )
//
// -----------------------------------------------------------------------------
//
TBool CCseSchedulerServerEngine::IsAllowedToRun( TUint32 aDbIdentifier )
	{	
	CSELOGSTRING2_HIGH_LEVEL(
	    ">>>CCseSchedulerServerEngine::IsAllowedToRun: aDbIdentifier: %d",
	    aDbIdentifier );
 
    TBool ret( EFalse );
    TInt32 runCount( 0 );
 
    TRAPD( err, runCount = iDb->GetRunCountL( aDbIdentifier ) );
    
    if ( err == KErrNone && runCount < 3 )
        {
        // Set true if check went ok and there is only less than three tries to run schedule        
        ret = ETrue;
        }
                
	CSELOGSTRING2_HIGH_LEVEL(
	    "<<<CCseSchedulerServerEngine::IsAllowedToRun: %d", ret);
	return ret;
	}

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::ScheduleNotValid( )
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::ScheduleNotValid( TUint32 aDbIdentifier )
	{	
	CSELOGSTRING2_HIGH_LEVEL(
	    ">>>CCseSchedulerServerEngine::ScheduleNotValid: aDbIdentifier: %d",
	    aDbIdentifier );

    TRAP_IGNORE( iDb->RemoveScheduleL( aDbIdentifier ) );    

	CSELOGSTRING_HIGH_LEVEL(
	    "<<<CCseSchedulerServerEngine::ScheduleNotValid");	
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::CompletitionCallback
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerServerEngine::CompletitionCallback( TAny* aPtr )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::CompletitionCallback");
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::CompletitionCallback");
	return static_cast<CCseSchedulerServerEngine*>( aPtr )->HandleCompletitionCallback();
	}

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::HandleCompletitionCallback
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerServerEngine::HandleCompletitionCallback() 
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::HandleCompletitionCallback ");
	    
    // Reschedule has been made to relevant plugin controllers.
    // Go through controllers and remove obsolote ones
    for( TInt i = 0; i < iPluginControllerArray.Count(); i++ )
        {
        // Do cleanup on controller
        iPluginControllerArray[i]->DoCleanUp( );
        
        if( iPluginControllerArray[i]->IsObsolete() )
            {
            // Delete it and remove from controller array
            delete iPluginControllerArray[i];
            iPluginControllerArray.Remove(i);            
            }
        }
        
    iServer.StopServer();        
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::HandleCompletitionCallback ");
	return EFalse;
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::RequestReschedule
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::RequestReschedule( ) 
	{
	CSELOGSTRING_HIGH_LEVEL(">>>RequestReschedule::HandleCompletitionCallback ");

    TRAP_IGNORE( RescheduleL() );

	CSELOGSTRING_HIGH_LEVEL("<<<RequestReschedule::HandleCompletitionCallback ");	
	}

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::RescheduleByPluginL
//
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::RescheduleByPluginL( TInt32 aPluginUid )
	{
	CSELOGSTRING2_HIGH_LEVEL(
	    ">>>CCseSchedulerServerEngine::RescheduleByPluginL - Plugin Uid: %d", aPluginUid);

    TTime runTime( iDb->GetNextScheduleTimeByPluginL( aPluginUid ) );
    TBool controllerFound( EFalse );
    
    // Search for existing plugin controller for this schedule
    RPointerArray<CCseScheduledProgram> schedules;
    CleanupResetAndDestroyPushL( schedules );
    iDb->GetSchedulesByTimeAndPluginL( runTime, aPluginUid, schedules );
    
    // Search plugin controller that needs rescheduling
    for( TInt i = 0; i < iPluginControllerArray.Count(); i++ )
        {
        if( aPluginUid == iPluginControllerArray[i]->PluginUid() )
            {
            if( schedules.Count() > 0 )
                {                    
                iPluginControllerArray[i]->SetSchedulesL( schedules );
                }
            else
                {
                 // Remove controller if controller found but
                 // there are no schedules for it
                
                // Check if controller can be removed right away or should it be done
                // after callstack breaker
                if( iPluginControllerArray[i]->IsControllerActive() )
                    {
                    CSELOGSTRING_HIGH_LEVEL(
                        "CCseSchedulerServerEngine::RescheduleByPluginL - Plugin found but no schedules, set controller to be removed");
                    iPluginControllerArray[i]->SetToBeCleared( ETrue );
                    iCompletitionBreaker->Start(TCallBack( CleanerCallback, this ));
                    }
                else
                    {
                    CSELOGSTRING_HIGH_LEVEL(
                        "CCseSchedulerServerEngine::RescheduleByPluginL - Plugin found but no schedules, remove controller");
                    delete iPluginControllerArray[i];
                    iPluginControllerArray.Remove( i );
                    }
                }
                
            controllerFound = ETrue;
            break;
            }        
        }
        
    if( controllerFound == EFalse )
        {
        // Create new controller if schedules found
        if( schedules.Count() > 0 )
            {                    
            if( schedules[0]->PluginType() == CCseScheduledProgram::ECseUniPlugin )
                {
                CSELOGSTRING_HIGH_LEVEL(
                    "CCseSchedulerServerEngine::RescheduleByPluginL: Create new uniplugin controller");
                // Create new PluginController
                iPluginControllerArray.AppendL( CCseSchedulerUniPluginController::NewL( *this, schedules[0]->PluginUid() ) );
                // Add schedules to just created controller
                iPluginControllerArray[iPluginControllerArray.Count()-1]->SetSchedulesL(schedules);
                }
            else if ( schedules[0]->PluginType() == CCseScheduledProgram::ECseMultiPlugin )
                {
                CSELOGSTRING_HIGH_LEVEL(
                "CCseSchedulerServerEngine::RescheduleByPluginL: Create new multiplugin controller");
                // Create new PluginController
                iPluginControllerArray.AppendL( CCseSchedulerMultiPluginController::NewL( *this, schedules[0]->PluginUid() ) );
                // Add schedules to just created controller
                iPluginControllerArray[iPluginControllerArray.Count()-1]->SetSchedulesL(schedules);
                }
            else
                {
                // WTF?!?!
                CSELOGSTRING_HIGH_LEVEL(
                    "CCseSchedulerServerEngine::RescheduleByPluginL: Plugin something completely different");
                }
            }
        }    
        
    // Delete schedule array.
    CleanupStack::PopAndDestroy( &schedules );
    
    // Stop server if we dont have any clients and there is no more
	// reason for us to live (no schedules active).
	iServer.StopServer();
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::RescheduleByPluginL out");
	}
	
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::ClearingDone()
// -----------------------------------------------------------------------------
//
void CCseSchedulerServerEngine::ClearingDone()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::ClearingDone");

    // Use completition callback to remove those controllers that aren't needed anymore
	if( !iCompletitionBreaker->IsActive() )
        {            
        iCompletitionBreaker->Start(TCallBack( CompletitionCallback, this ));
        }
    else
        {
        CSELOGSTRING_HIGH_LEVEL(
            "CCseSchedulerServerEngine::ClearingDone - Breaker already active");
        }	
        
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::ClearingDone");
    }
    
// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::CleanerCallback
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerServerEngine::CleanerCallback( TAny* aPtr )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::CleanerCallback");
	static_cast<CCseSchedulerServerEngine*>( aPtr )->HandleCleanerCallback();
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::CleanerCallback");
	return EFalse;
	}

// -----------------------------------------------------------------------------
// CCseSchedulerServerEngine::HandleCompletitionCallback
// -----------------------------------------------------------------------------
//
TInt CCseSchedulerServerEngine::HandleCleanerCallback() 
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseSchedulerServerEngine::HandleCleanerCallback ");
	
    // go through plugins and clear
    for( TInt i = iPluginControllerArray.Count(); i > 0 ; i--)
        {
        if( iPluginControllerArray[i-1]->IsToBeCleared() )
            {
            // If ClearControllerL -call leaves it means that we couldn't signal
            // plugin to take himself down. In such cases we leak memory, but we
            // still need to keep going so we do not screw all the rest schedules.
            TRAP_IGNORE( iPluginControllerArray[i-1]->ClearControllerL());
            delete iPluginControllerArray[i-1];
            iPluginControllerArray.Remove(i-1);            
            }
        }

    iServer.StopServer();    
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseSchedulerServerEngine::HandleCleanerCallback ");
	return EFalse;
	}

