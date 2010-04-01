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
* Description:    Implementation of Scheduler engine client's Service*
*/





// INCLUDE FILES
#include <ipvideo/RCseSchedulerService.h>               // Header fo this class
#include <ipvideo/CCseScheduledProgram.h>               // Represent one schedule in database
#include "CseDebug.h"                           // Debug macros
#include <ipvideo/CseSchedulerClientServerCommon.h>     // Client common methods (server start up etc)
#include <s32mem.h>

// EXTERNAL DATA STRUCTURES
// None.

// EXTERNAL FUNCTION PROTOTYPES  
// None.

// CONSTANTS
// None.

// MACROS
// None.

// LOCAL CONSTANTS AND MACROS
// None.

// MODULE DATA STRUCTURES
// None.

// LOCAL FUNCTION PROTOTYPES
// None.

// FORWARD DECLARATIONS
// None.

// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// RCseSchedulerService::RCseSchedulerService
//
// -----------------------------------------------------------------------------
//
RCseSchedulerService::RCseSchedulerService() : RCseSchedulerServiceBase()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerService::RCseSchedulerService");
    // None
    CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerService::RCseSchedulerService");
    }

// -----------------------------------------------------------------------------
// RCseSchedulerService::~RCseSchedulerService
//
// -----------------------------------------------------------------------------
//
RCseSchedulerService::~RCseSchedulerService()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerService::~RCseSchedulerService");
    CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerService::~RCseSchedulerService");
    }
    
// -----------------------------------------------------------------------------
// RCseSchedulerService::AddScheduleL
//
// -----------------------------------------------------------------------------
//
void RCseSchedulerService::AddScheduleL( CCseScheduledProgram& aData ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerService::AddScheduleL");
    if ( !SubSessionHandle() )
        {
		User::Leave( ECseNoSubsessionHandle );
        }
    // First we to "externalize" (put CCseScheduledProgram-class content to array)
    // information from given scheduled program.
    HBufC8* msg = HBufC8::NewLC( aData.ExternalizeLength() );
	HBufC8* id = HBufC8::NewLC( 4 );
    TPtr8 ptr( msg->Des() );
    TPtr8 idPtr( id->Des() );
    
    RDesWriteStream writeStream;
    CleanupClosePushL( writeStream );
    writeStream.Open( ptr );
    aData.ExternalizeL( writeStream );
    CleanupStack::PopAndDestroy( &writeStream );
    
    TIpcArgs args( &ptr, &idPtr );
    
    // Send array to server side.
    User::LeaveIfError( SendReceive( ECseAddSchedule, args ) );
    
    RDesReadStream readStream;
	CleanupClosePushL( readStream );
    readStream.Open( idPtr );
    aData.SetDbIdentifier( readStream.ReadUint32L() );
    CleanupStack::PopAndDestroy( &readStream );
	
	CleanupStack::PopAndDestroy( id );
	CleanupStack::PopAndDestroy( msg );
	CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerService::AddScheduleL");
    }

// -----------------------------------------------------------------------------
// RCseSchedulerService::RemoveScheduleL
//
// -----------------------------------------------------------------------------
//
void RCseSchedulerService::RemoveScheduleL( const TUint32 aDbIdentifier ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerService::RemoveScheduleL");
    if ( !SubSessionHandle() )
        {
		User::Leave( ECseNoSubsessionHandle );
        }

    // Put integer value to stream
    HBufC8* msg = HBufC8::NewL( 4 );
	CleanupStack::PushL( msg );
    TPtr8 ptr( msg->Des() );
    RDesWriteStream writeStream;
    CleanupClosePushL( writeStream );
    writeStream.Open( ptr );
	writeStream.WriteUint32L( aDbIdentifier );
    CleanupStack::PopAndDestroy( &writeStream );
    
    TIpcArgs args( &ptr );
    
    // Send stream to server side
    User::LeaveIfError( SendReceive( ECseRemoveSchedule, args ) );  
	CleanupStack::PopAndDestroy( msg );
	CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerService::RemoveScheduleL");
    }
    
// -----------------------------------------------------------------------------
// RCseSchedulerService::GetScheduleL
//
// -----------------------------------------------------------------------------
//
void RCseSchedulerService::GetScheduleL( const TUint32 aDbIdentifier,
										 CCseScheduledProgram* aProg ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerService::GetScheduleL");
    if ( !SubSessionHandle() )
        {
		User::Leave( ECseNoSubsessionHandle );
        }
    if ( !aProg )
    	{
    	User::Leave( KErrArgument );
    	}
    // Buffer for DbIdentifier
    HBufC8* msgDbId = HBufC8::NewLC( 4 );
    TPtr8 dbIdPtr( msgDbId->Des() );
    
	// Buffer for incoming message length
    HBufC8* lengthBuff = HBufC8::NewLC( 4 );
    TPtr8 lengthPtr( lengthBuff->Des() );
    TInt length( 0 );
    
    // Initialize DbIdentifier
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
    writeStream.Open( dbIdPtr );
    writeStream.WriteUint32L( aDbIdentifier );
    CleanupStack::PopAndDestroy( &writeStream );
    
    // Set args for IPC
    TIpcArgs args( &dbIdPtr, &lengthPtr );
    
    // Ask for message length
    User::LeaveIfError( SendReceive( ECseGetScheduleLength, args ) ); 
    
    // Read length from the message
    RDesReadStream readStream;
	CleanupClosePushL( readStream );
    readStream.Open( lengthPtr );
	length = readStream.ReadInt32L();
	CleanupStack::PopAndDestroy( &readStream );
	
	// Now ask for the scheduled program information
	HBufC8* programBuffer = HBufC8::NewLC( length );
	TPtr8 programPtr( programBuffer->Des() );
	
	TIpcArgs progArgs( &dbIdPtr, &programPtr );
	User::LeaveIfError( SendReceive( ECseGetSchedule, progArgs ) );
	
	// Internalize (change stream to CCseScheduledProgram-class) stream that we got.
	RDesReadStream readStr;
	CleanupClosePushL( readStr );
	readStr.Open( programPtr );
	aProg->InternalizeL( readStr );
	CleanupStack::PopAndDestroy( &readStr );
	
	CleanupStack::PopAndDestroy( programBuffer );
	CleanupStack::PopAndDestroy( lengthBuff );
	CleanupStack::PopAndDestroy( msgDbId );
	CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerService::GetSchduleL");
    }

// -----------------------------------------------------------------------------
// RCseSchedulerService::GetSchedulesL
//
// -----------------------------------------------------------------------------
//
void RCseSchedulerService::GetSchedulesL( const TInt32 aAppUid, 
						RPointerArray<CCseScheduledProgram>& aArray ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerService::GetSchdulesL");
    if ( !SubSessionHandle() )
        {
		User::Leave( ECseNoSubsessionHandle );
        }
        
    // Buffer for app UID
    HBufC8* msgAppUid = HBufC8::NewLC( 4 );
    TPtr8 appUidPtr( msgAppUid->Des() );
    
	// Buffer for incoming message length
    HBufC8* lengthBuff = HBufC8::NewLC( 4 );
    TPtr8 lengthPtr( lengthBuff->Des() );
    TInt length( 0 );
    
    // Write App Uid
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
    writeStream.Open( appUidPtr );
    writeStream.WriteInt32L( aAppUid );
    CleanupStack::PopAndDestroy( &writeStream );
    
    // Set args for IPC
    TIpcArgs args( &appUidPtr, &lengthPtr );
    
    // Ask for message length
    User::LeaveIfError( SendReceive( ECseGetScheduleArrayLengthByAppUid, args ) ); 
    
    // Read length from the message
    RDesReadStream readStream;
	CleanupClosePushL( readStream );
    readStream.Open( lengthPtr );
	length = readStream.ReadInt32L();
	CleanupStack::PopAndDestroy( &readStream );
	
	// Now ask for the scheduled program information
	HBufC8* programBuffer = HBufC8::NewLC( length );
	TPtr8 programPtr( programBuffer->Des() );
	
	TIpcArgs progArgs( &appUidPtr, &programPtr );
	User::LeaveIfError( SendReceive( ECseGetSchedulesByAppUid, progArgs ) );
	
	RDesReadStream readStr;
	CleanupClosePushL( readStr );
	readStr.Open( programPtr );
	
	// read number of items in stream
	TInt itemCount ( 0 );
	itemCount = readStr.ReadInt32L();
	
	// read all items from stream to array
	for ( TInt i = 0; i < itemCount; i++ )
		{
		// Internalize (change stream to CCseScheduledProgram-class) stream that we got.
		CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
		CleanupStack::PushL( prog );
		prog->InternalizeL( readStr );
		aArray.AppendL( prog );	
		CleanupStack::Pop( prog );
		}
		
	readStr.Close();

	CleanupStack::PopAndDestroy( &readStr );
	CleanupStack::PopAndDestroy( programBuffer );
	CleanupStack::PopAndDestroy( lengthBuff );
	CleanupStack::PopAndDestroy( msgAppUid );
	CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerService::GetSchdulesL");
    }

// -----------------------------------------------------------------------------
// RCseSchedulerService::GetOverlappingScheduleL
//
// -----------------------------------------------------------------------------
//
void RCseSchedulerService::GetOverlappingSchedulesL( 
	CCseScheduledProgram& aProgram, 
	RPointerArray<CCseScheduledProgram>& aResultArray )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerService::GetOverlappingSchedulesL");
	if ( !SubSessionHandle() )
        {
		User::Leave( ECseNoSubsessionHandle );
        }

	// Buffer for schedule data
    HBufC8* schedData = HBufC8::NewLC( sizeof( TInt32 ) + // type
									  	sizeof( TInt64 ) + // start time
									   sizeof( TInt64 ) ); // end time
    TPtr8 schedDataPtr( schedData->Des() );
    
	// Buffer for incoming message length
    HBufC8* lengthBuf = HBufC8::NewLC( sizeof( TInt32 ) ); 
    TPtr8 lengthBufPtr( lengthBuf->Des() );
    TUint32 length( 0 );    

    // Schedule type
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
    writeStream.Open( schedDataPtr );
    writeStream.WriteInt32L( aProgram.ScheduleType() );

	// Start time
	TInt64 temp( 0 );
	TUint32 lower( 0 );
	TUint32 upper( 0 );

    // Change TTime to stream format.
	temp = aProgram.StartTime().Int64();
    lower = (0x00000000FFFFFFFFULL) & temp;
    upper = (0x00000000FFFFFFFFULL) & (temp >> 32);
    writeStream.WriteUint32L( lower );
    writeStream.WriteUint32L( upper );

	// End time
	temp = aProgram.EndTime().Int64();
    lower = (0x00000000FFFFFFFFULL) & temp;
    upper = (0x00000000FFFFFFFFULL) & (temp >> 32);
    writeStream.WriteUint32L( lower );
    writeStream.WriteUint32L( upper );

	writeStream.CommitL();
    CleanupStack::PopAndDestroy( &writeStream );
    
    // Set args for IPC
    TIpcArgs args( &schedDataPtr, &lengthBufPtr );
    
    // Ask for db id
    User::LeaveIfError( SendReceive( ECseGetOverlappingSchedulesLength, args ) ); 
    
	// Read length
    RDesReadStream readStream;
	CleanupClosePushL( readStream );
    readStream.Open( lengthBufPtr );
	length = readStream.ReadUint32L();
	CleanupStack::PopAndDestroy( &readStream );

	if ( length > 0 )
		{
		HBufC8* schedulesBuf = HBufC8::NewLC( length + 
									sizeof( TInt32 ) ); // item count as TInt32
		TPtr8 schedulesPtr( schedulesBuf->Des() );

		TIpcArgs schedArgs( &schedDataPtr, &schedulesPtr );

		User::LeaveIfError( SendReceive( ECseGetOverlappingSchedules, schedArgs ) );

		RDesReadStream readStr;
		CleanupClosePushL( readStr );
		readStr.Open( schedulesPtr );
		// read item count
		TInt32 count( readStr.ReadInt32L() );

		// read all items from stream to array
		for ( TInt i = 0; i < count; i++ )
			{
			// Internalize (change stream to CCseScheduledProgram-class) stream that we got.
			CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
			CleanupStack::PushL( prog );
			prog->InternalizeL( readStr );
			aResultArray.AppendL( prog );	
			CleanupStack::Pop( prog );
			}
			
		readStr.Close();
		CleanupStack::PopAndDestroy( &readStr );
		CleanupStack::PopAndDestroy( schedulesBuf );	
		}
	
	CleanupStack::PopAndDestroy( lengthBuf );
	CleanupStack::PopAndDestroy( schedData );
	CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerService::GetOverlappingSchedulesL");
	}

// -----------------------------------------------------------------------------
// RCseSchedulerService::GetSchedulesByPluginUidL
//
// -----------------------------------------------------------------------------
//
void RCseSchedulerService::GetSchedulesByPluginUidL( const TInt32 aPluginUid, 
						                             RPointerArray<CCseScheduledProgram>& aArray ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerService::GetSchedulesByPluginUidL");
    if ( !SubSessionHandle() )
        {
		User::Leave( ECseNoSubsessionHandle );
        }
        
    // Buffer for app UID
    HBufC8* msgPluginUid = HBufC8::NewLC( 4 );
    TPtr8 pluginUidPtr( msgPluginUid->Des() );
    
	// Buffer for incoming message length
    HBufC8* lengthBuff = HBufC8::NewLC( 4 );
    TPtr8 lengthPtr( lengthBuff->Des() );
    TInt length( 0 );
    
    // Write App Uid
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
    writeStream.Open( pluginUidPtr );
    writeStream.WriteInt32L( aPluginUid );
    CleanupStack::PopAndDestroy( &writeStream );
    
    // Set args for IPC
    TIpcArgs args( &pluginUidPtr, &lengthPtr );
    
    // Ask for message length
    User::LeaveIfError( SendReceive( ECseGetScheduleArrayLengthByPluginUid, args ) );
    
    // Read length from the message
    RDesReadStream readStream;
	CleanupClosePushL( readStream );
    readStream.Open( lengthPtr );
	length = readStream.ReadInt32L();
	CleanupStack::PopAndDestroy( &readStream );
	
	// Now ask for the scheduled program information
	HBufC8* programBuffer = HBufC8::NewLC( length );
	TPtr8 programPtr( programBuffer->Des() );
	
	TIpcArgs progArgs( &pluginUidPtr, &programPtr );
	User::LeaveIfError( SendReceive( ECseGetSchedulesByPluginUid, progArgs ) );
	
	RDesReadStream readStr;
	CleanupClosePushL( readStr );
	readStr.Open( programPtr );
	
	// read number of items in stream
	TInt itemCount ( 0 );
	itemCount = readStr.ReadInt32L();
	
	// read all items from stream to array
	for ( TInt i = 0; i < itemCount; i++ )
		{
		// Internalize (change stream to CCseScheduledProgram-class) stream that we got.
		CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
		CleanupStack::PushL( prog );
		prog->InternalizeL( readStr );
		aArray.AppendL( prog );	
		CleanupStack::Pop( prog );
		}
		
	readStr.Close();

	CleanupStack::PopAndDestroy( &readStr );
	CleanupStack::PopAndDestroy( programBuffer );
	CleanupStack::PopAndDestroy( lengthBuff );
	CleanupStack::PopAndDestroy( msgPluginUid );
	CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerService::GetSchedulesByPluginUidL");    
    }

// -----------------------------------------------------------------------------
// RCseSchedulerService::GetSchedulesByTypeL
//
// -----------------------------------------------------------------------------
//
void RCseSchedulerService::GetSchedulesByTypeL( const TInt32 aType, 
						                        RPointerArray<CCseScheduledProgram>& aArray ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerService::GetSchedulesByTypeL");
    if ( !SubSessionHandle() )
        {
		User::Leave( ECseNoSubsessionHandle );
        }
        
    // Buffer for app UID
    HBufC8* msgScheduleType = HBufC8::NewLC( 4 );
    TPtr8 typePtr( msgScheduleType->Des() );
    
	// Buffer for incoming message length
    HBufC8* lengthBuff = HBufC8::NewLC( 4 );
    TPtr8 lengthPtr( lengthBuff->Des() );
    TInt length( 0 );
    
    // Write App Uid
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
    writeStream.Open( typePtr );
    writeStream.WriteInt32L( aType );
    CleanupStack::PopAndDestroy( &writeStream );
    
    // Set args for IPC
    TIpcArgs args( &typePtr, &lengthPtr );
    
    // Ask for message length
    User::LeaveIfError( SendReceive( ECseGetScheduleArrayLengthByType, args ) );
    
    // Read length from the message
    RDesReadStream readStream;
	CleanupClosePushL( readStream );
    readStream.Open( lengthPtr );
	length = readStream.ReadInt32L();
	CleanupStack::PopAndDestroy( &readStream );
	
	// Now ask for the scheduled program information
	HBufC8* programBuffer = HBufC8::NewLC( length );
	TPtr8 programPtr( programBuffer->Des() );
	
	TIpcArgs progArgs( &typePtr, &programPtr );
	User::LeaveIfError( SendReceive( ECseGetSchedulesByType, progArgs ) );
	
	RDesReadStream readStr;
	CleanupClosePushL( readStr );
	readStr.Open( programPtr );
	
	// read number of items in stream
	TInt itemCount ( 0 );
	itemCount = readStr.ReadInt32L();
	
	// read all items from stream to array
	for ( TInt i = 0; i < itemCount; i++ )
		{
		// Internalize (change stream to CCseScheduledProgram-class) stream that we got.
		CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
		CleanupStack::PushL( prog );
		prog->InternalizeL( readStr );
		aArray.AppendL( prog );	
		CleanupStack::Pop( prog );
		}
		
	readStr.Close();

	CleanupStack::PopAndDestroy( &readStr );
	CleanupStack::PopAndDestroy( programBuffer );
	CleanupStack::PopAndDestroy( lengthBuff );
	CleanupStack::PopAndDestroy( msgScheduleType );
	CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerService::GetSchedulesByTypeL");    
    }

// -----------------------------------------------------------------------------
// RCseSchedulerService::GetSchedulesByTimeL
//
// -----------------------------------------------------------------------------
//
void RCseSchedulerService::GetSchedulesByTimeL( const TTime& aBeginning, 
                                                const TTime& aEnd,
						                        RPointerArray<CCseScheduledProgram>& aArray ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>RCseSchedulerService::GetSchedulesByTimeL");
	if ( !SubSessionHandle() )
        {
		User::Leave( ECseNoSubsessionHandle );
        }

	// Buffer for schedule data
    HBufC8* timeframe = HBufC8::NewLC( sizeof( TInt64 ) +  // start time
									   sizeof( TInt64 ) ); // end time
									   
    TPtr8 timeframePtr( timeframe->Des() );
    
	// Buffer for incoming message length
    HBufC8* lengthBuf = HBufC8::NewLC( sizeof( TInt32 ) ); 
    TPtr8 lengthBufPtr( lengthBuf->Des() );
    TUint32 length( 0 );    

    // Schedule type
	RDesWriteStream writeStream;
	CleanupClosePushL( writeStream );
    writeStream.Open( timeframePtr );
    
	// Start time
	TInt64 temp( 0 );
	TUint32 lower( 0 );
	TUint32 upper( 0 );

    // Change TTime to stream format.
	temp = aBeginning.Int64();
    lower = (0x00000000FFFFFFFFULL) & temp;
    upper = (0x00000000FFFFFFFFULL) & (temp >> 32);
    writeStream.WriteUint32L( lower );
    writeStream.WriteUint32L( upper );

	// End time
	temp = aEnd.Int64();
    lower = (0x00000000FFFFFFFFULL) & temp;
    upper = (0x00000000FFFFFFFFULL) & (temp >> 32);
    writeStream.WriteUint32L( lower );
    writeStream.WriteUint32L( upper );

	writeStream.CommitL();
    CleanupStack::PopAndDestroy( &writeStream );
    
    // Set args for IPC
    TIpcArgs args( &timeframePtr, &lengthBufPtr );
    
    // Ask for db id
    User::LeaveIfError( SendReceive( ECseGetScheduleArrayLengthByTimeframe, args ) ); 
    
	// Read length
    RDesReadStream readStream;
	CleanupClosePushL( readStream );
    readStream.Open( lengthBufPtr );
	length = readStream.ReadUint32L();
	CleanupStack::PopAndDestroy( &readStream );

	if ( length > 0 )
		{
		HBufC8* schedulesBuf = HBufC8::NewLC( length + 
									          sizeof( TInt32 ) ); // item count as TInt32
		TPtr8 schedulesPtr( schedulesBuf->Des() );

		TIpcArgs schedArgs( &timeframePtr, &schedulesPtr );

		User::LeaveIfError( SendReceive( ECseGetSchedulesByTimeframe, schedArgs ) );

		RDesReadStream readStr;
		CleanupClosePushL( readStr );
		readStr.Open( schedulesPtr );
		// read item count
		TInt32 count( readStr.ReadInt32L() );

		// read all items from stream to array
		for ( TInt i = 0; i < count; i++ )
			{
			// Internalize (change stream to CCseScheduledProgram-class) stream that we got.
			CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
			CleanupStack::PushL( prog );
			prog->InternalizeL( readStr );
			aArray.AppendL( prog );	
			CleanupStack::Pop( prog );
			}
			
		readStr.Close();
		CleanupStack::PopAndDestroy( &readStr );
		CleanupStack::PopAndDestroy( schedulesBuf );	
		}
	
	CleanupStack::PopAndDestroy( lengthBuf );
	CleanupStack::PopAndDestroy( timeframe );
	CSELOGSTRING_HIGH_LEVEL("<<<RCseSchedulerService::GetSchedulesByTimeL");    
    }



// End of File
