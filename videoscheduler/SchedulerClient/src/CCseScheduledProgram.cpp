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
* Description:    This class contains data for scheduled programs.*
*/




// INCLUDES
#include <ipvideo/CCseScheduledProgram.h>   // Header file for this class
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

// ---------------------------------------------------------------------------
// CCseScheduledProgram::CCseScheduledProgram()
//
// ---------------------------------------------------------------------------	
EXPORT_C CCseScheduledProgram::CCseScheduledProgram() :
									iStartTime( 0 ),
									iEndTime( 0 ),									
									iAppUid( 0 ),
									iPluginUid( 0 ),
									iDbIdentifier( 0 ),
									iScheduleType ( ECseOther ),
									iPluginType( ECseUniPlugin )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::CCseScheduledProgram");
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduledProgram::CCseScheduledProgram");
	}
	
// ---------------------------------------------------------------------------
// CCseScheduledProgram::NewL()
//
// ---------------------------------------------------------------------------	
EXPORT_C CCseScheduledProgram* CCseScheduledProgram::NewL( CCseScheduledProgram& aProg)
	{
    CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::NewL");
    
    CCseScheduledProgram* self = new ( ELeave ) CCseScheduledProgram();    
    CleanupStack::PushL( self );
    self->ConstructL();
    self->SetName( aProg.Name() );
    self->SetStartTime( aProg.StartTime() );
    self->SetEndTime( aProg.EndTime() );
    self->SetAppUid( aProg.AppUid() );
    self->SetPluginUid( aProg.PluginUid() );
    self->SetDbIdentifier( aProg.DbIdentifier() );    
    self->SetApplicationDataL( aProg.ApplicationData() );   
    self->SetScheduleType( aProg.ScheduleType() );
    self->SetPluginType( aProg.PluginType() );
    CleanupStack::Pop( self );   
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduledProgram::NewL");
    return self;
	}


// ---------------------------------------------------------------------------
// CCseScheduledProgram::NewL()
//
// ---------------------------------------------------------------------------	
EXPORT_C CCseScheduledProgram* CCseScheduledProgram::NewL()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::NewL");
    
    CCseScheduledProgram* self = new ( ELeave ) CCseScheduledProgram();    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
        
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduledProgram::NewL");
    return self;
	}
	
// ---------------------------------------------------------------------------
// CCseScheduledProgram::ConstructL()
//
// ---------------------------------------------------------------------------	
void CCseScheduledProgram::ConstructL()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::ConstructL");
    iAppData = HBufC8::NewL( 0 );    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduledProgram::ConstructL");
	}
	
// ---------------------------------------------------------------------------
// CCseScheduledProgram::~CCseScheduledProgram()
//
// ---------------------------------------------------------------------------	
EXPORT_C CCseScheduledProgram::~CCseScheduledProgram()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::~CCseScheduledProgram");
    delete iAppData;
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduledProgram::~CCseScheduledProgram");	
	}

// ---------------------------------------------------------------------------
// CCseScheduledProgram::SetScheduleType()
//
// ---------------------------------------------------------------------------	
EXPORT_C void CCseScheduledProgram::SetScheduleType( const TInt32 aScheduleType )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::SetScheduleType");
    iScheduleType = aScheduleType;	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduledProgram::SetScheduleType");	
	}
		
// ---------------------------------------------------------------------------
// CCseScheduledProgram::SetApplicationDataL()
//
// ---------------------------------------------------------------------------	
EXPORT_C void CCseScheduledProgram::SetApplicationDataL( const TDesC8& aAppData )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::SetApplicationData");
    delete iAppData;
	iAppData = NULL;
	iAppData = aAppData.AllocL();
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduledProgram::SetApplicationData");	
	}
	
// ---------------------------------------------------------------------------
// CCseScheduledProgram::ApplicationData()
//
// ---------------------------------------------------------------------------	
EXPORT_C TPtrC8 CCseScheduledProgram::ApplicationData()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::ApplicationData");
    return *iAppData;    
	}
	
// -----------------------------------------------------------------------------
// CCseScheduledProgram::InternalizeL
// -----------------------------------------------------------------------------
//
EXPORT_C void CCseScheduledProgram::InternalizeL( RReadStream& aStream )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::InternalizeL");
	TUint32 lower( 0 );
	TUint32 upper( 0 );
	TInt32 buffLen( 0 );	

	// Name
	buffLen = aStream.ReadInt32L();
	aStream.ReadL( iName, buffLen );

	// Start time
    lower = aStream.ReadUint32L();
    upper = aStream.ReadUint32L();
    TInt64 time( 0 );
    time = (TInt64) lower;    
    TInt64 longUpper = (TInt64) upper;
    longUpper = longUpper << 32;
    longUpper &= (0xFFFFFFFF00000000ULL);    
    time |= longUpper;
    iStartTime = time;  
        
	// End time
	lower = aStream.ReadUint32L();
    upper = aStream.ReadUint32L();
    time = (TInt64) lower;    
    longUpper = (TInt64) upper;
    longUpper = longUpper << 32;
    longUpper &= (0xFFFFFFFF00000000ULL);    
    time |= longUpper;
    iEndTime = time;  

	// Application UID
	iAppUid = aStream.ReadInt32L();
	
	// Notifier plug-in UID
	iPluginUid = aStream.ReadInt32L();
	
	// Db identifier
	iDbIdentifier = aStream.ReadUint32L();
	
	// Schedule type
	iScheduleType = aStream.ReadInt32L();
	
	// Plugin type
	iPluginType = aStream.ReadInt32L();
		
	// Application data length
	buffLen = aStream.ReadInt32L();
	
	// Application data
	delete iAppData;
	iAppData = NULL;
	iAppData = HBufC8::NewL( buffLen );
	TPtr8 ptr( iAppData->Des() );
	aStream.ReadL( ptr, buffLen );
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduledProgram::InternalizeL");	
	}
	
// -----------------------------------------------------------------------------
// CCseScheduledProgram::ExternalizeL
// -----------------------------------------------------------------------------
//
EXPORT_C void CCseScheduledProgram::ExternalizeL(RWriteStream& aStream)
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::ExternalizeL");
	TInt64 temp( 0 );
	TUint32 lower( 0 );
	TUint32 upper( 0 );
	TInt32 buffLen( 0 );

	// Name
	buffLen = iName.Length();
	aStream.WriteInt32L( buffLen );
	aStream.WriteL( iName );
	// Start time
	temp = iStartTime.Int64();
    lower = (0x00000000FFFFFFFFULL) & temp;
    upper = (0x00000000FFFFFFFFULL) & (temp >> 32);
    aStream.WriteUint32L( lower );
    aStream.WriteUint32L( upper );
    
    // End time
	temp = iEndTime.Int64();
    lower = (0x00000000FFFFFFFFULL) & temp;
    upper = (0x00000000FFFFFFFFULL) & (temp >> 32);
    aStream.WriteUint32L( lower );
    aStream.WriteUint32L( upper );
    
	// Application UID
	aStream.WriteInt32L( iAppUid );
	
	// Notifier plug-in UID
	aStream.WriteInt32L( iPluginUid );
	
	// DbIdentifier
	aStream.WriteUint32L( iDbIdentifier );
	
	// Schedule type	
	aStream.WriteInt32L( iScheduleType );
	
	// Schedule type	
	aStream.WriteInt32L( iPluginType );
	
	// Application data length
	buffLen = iAppData->Length();
	aStream.WriteInt32L( buffLen );
	
	// Application data
	aStream.WriteL( *iAppData );
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduledProgram::ExternalizeL");	
	}


// -----------------------------------------------------------------------------
// CCseScheduledProgram::ExternalizeLength
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCseScheduledProgram::ExternalizeLength( )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::ExternalizeLength");
	TInt retValue( 0 );
		retValue =   sizeof( iStartTime )
				    +sizeof( iEndTime )
				    +sizeof( iAppUid )
				    +sizeof( iPluginUid )
				    +sizeof( iDbIdentifier )
				    +sizeof( iScheduleType )
				    +sizeof( iPluginType )
				    +sizeof( TInt32 )         // Application data length
				    +sizeof( TInt32 )         // Name length
				    +iName.Size( )			  // Name
				    +iAppData->Size( );	      // Application data size
		
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduledProgram::ExternalizeLength");
	return retValue;
	}

// -----------------------------------------------------------------------------
// CCseScheduledProgram::SetPluginType
// -----------------------------------------------------------------------------
//
EXPORT_C void CCseScheduledProgram::SetPluginType( TInt32 aPluginType )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::SetPluginType");
	iPluginType = aPluginType;
	}

// -----------------------------------------------------------------------------
// CCseScheduledProgram::SetName
// -----------------------------------------------------------------------------
//
EXPORT_C void CCseScheduledProgram::SetName( const TDesC8& aName )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::SetName");
	iName = aName;
	}

// -----------------------------------------------------------------------------
// CCseScheduledProgram::SetDbIdentifier
// -----------------------------------------------------------------------------
//
EXPORT_C void CCseScheduledProgram::SetDbIdentifier( const TUint32 aDbIdentifier )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::SetDbIdentifier");
	iDbIdentifier = aDbIdentifier;
	}
	
// -----------------------------------------------------------------------------
// CCseScheduledProgram::SetStartTime
// -----------------------------------------------------------------------------
//
EXPORT_C void CCseScheduledProgram::SetStartTime( const TTime& aStartTime )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::SetStartTime");
	iStartTime = aStartTime;
	}

// -----------------------------------------------------------------------------
// CCseScheduledProgram::SetEndTime
// -----------------------------------------------------------------------------
//
EXPORT_C void CCseScheduledProgram::SetEndTime( const TTime& aEndTime )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::SetEndTime");
	iEndTime = aEndTime;
	}

// -----------------------------------------------------------------------------
// CCseScheduledProgram::SetAppUid
// -----------------------------------------------------------------------------
//
EXPORT_C void CCseScheduledProgram::SetAppUid( const TInt32 aAppUid )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::SetAppUid");
	iAppUid = aAppUid;
	}

// -----------------------------------------------------------------------------
// CCseScheduledProgram::SetPluginUid
// -----------------------------------------------------------------------------
//
EXPORT_C void CCseScheduledProgram::SetPluginUid( const TInt32 aPluginUid )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::SetPluginUid");
	iPluginUid = aPluginUid;
	}


// Getters for the instance data
// -----------------------------------------------------------------------------
// CCseScheduledProgram::GetPluginType
// -----------------------------------------------------------------------------
//
EXPORT_C TInt32 CCseScheduledProgram::PluginType() const
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::PluginType");
	return iPluginType;
	}

// -----------------------------------------------------------------------------
// CCseScheduledProgram::Name
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC8 CCseScheduledProgram::Name() const
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::Name");
	return iName;
	}

// -----------------------------------------------------------------------------
// CCseScheduledProgram::DbIdentifier
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CCseScheduledProgram::DbIdentifier() const
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::DbIdentifier");
	return iDbIdentifier;
	}

// -----------------------------------------------------------------------------
// CCseScheduledProgram::StartTime
// -----------------------------------------------------------------------------
//
EXPORT_C TTime& CCseScheduledProgram::StartTime()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::SetDbIdentifier");
	return iStartTime;
	}

// -----------------------------------------------------------------------------
// CCseScheduledProgram::EndTime
// -----------------------------------------------------------------------------
//
EXPORT_C TTime& CCseScheduledProgram::EndTime()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::EndTime");
	return iEndTime;
	}


// -----------------------------------------------------------------------------
// CCseScheduledProgram::ScheduleType
// -----------------------------------------------------------------------------
//
EXPORT_C TInt32 CCseScheduledProgram::ScheduleType() const
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::ScheduleType");
	return iScheduleType;
	}

// -----------------------------------------------------------------------------
// CCseScheduledProgram::AppUid
// -----------------------------------------------------------------------------
//
EXPORT_C TInt32 CCseScheduledProgram::AppUid() const
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::Appuid");
	return iAppUid;
	}

// -----------------------------------------------------------------------------
// CCseScheduledProgram::PluginUid
// -----------------------------------------------------------------------------
//
EXPORT_C TInt32 CCseScheduledProgram::PluginUid() const
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::PluginUid");
	return iPluginUid;
	}


// -----------------------------------------------------------------------------
// CCseScheduledProgram::DebugDump
// -----------------------------------------------------------------------------
//
EXPORT_C void CCseScheduledProgram::DebugDump() const
	{	
#ifdef _DEBUG	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduledProgram::DebugDump");
	
	// Write schedule name
	// Change 8-bit buffer to 16-bit.
	HBufC* writeBuff = NULL;
	TRAPD( err, writeBuff = HBufC::NewL( iName.Length() ) );
    if(err == KErrNone)
        {            
        TPtr writePtr = writeBuff->Des();
        writePtr.Copy( iName );
        CSELOGSTRING2_HIGH_LEVEL("Schedule name:                    %S", writeBuff);
        }
    else
        {    
        CSELOGSTRING2_HIGH_LEVEL("Schedule name:                    %S", &iName);
        }
        
    // Start time
	TBuf<100> startTimeBuf;
	_LIT( KDateTimeFormat,"%1%*D/%2%*M/%3%*Y %H:%T:%S" );
	TRAP( err, iStartTime.FormatL( startTimeBuf, KDateTimeFormat ) );
	if ( KErrNone == err )
	    {
    	CSELOGSTRING2_HIGH_LEVEL("Schedule start time:              %S", &startTimeBuf);
	    }
	
	// End time
	TBuf<100> endTimeBuf;	
	TRAP( err, iEndTime.FormatL( endTimeBuf, KDateTimeFormat ) );
	if ( KErrNone == err )
	    {
    	CSELOGSTRING2_HIGH_LEVEL("Schedule end time:                %S", &endTimeBuf);
	    }
	
	// Application UID
	CSELOGSTRING2_HIGH_LEVEL("Schedule application uid:         0x%x", (unsigned)iAppUid);
	
	// Plugin UID
	CSELOGSTRING2_HIGH_LEVEL("Schedule plugin uid:              0x%x", (unsigned)iPluginUid);
		
	// Application data
	// Write only length of data
	CSELOGSTRING2_HIGH_LEVEL("Schedule application data length: %d", iAppData->Length() );
	
	// DB identifier
	CSELOGSTRING2_HIGH_LEVEL("Schedule DB identifier:           %x", iDbIdentifier);
	
	// Schedule type
	switch(iScheduleType)
	    {
	        case ECseReminder:
	        CSELOGSTRING_HIGH_LEVEL("Schedule type:                    ECseReminder");
	        break;
	        
	        case ECseRecording:
	        CSELOGSTRING_HIGH_LEVEL("Schedule type:                    ECseRecording");
	        break;
	        
	        case ECseProgramGuideUpdate:
	        CSELOGSTRING_HIGH_LEVEL("Schedule type:                    ECseProgramGuideUpdate");
	        break;
	        
	        case ECseScheduleDownload:
	        CSELOGSTRING_HIGH_LEVEL("Schedule type:                    ECseScheduleDownload");
	        break;
	        
	        case ECseOther:
	        CSELOGSTRING_HIGH_LEVEL("Schedule type:                    ECseOther");
	        break;
	        
	        default:
	        break;	        
	    }
	delete writeBuff;
    writeBuff = NULL;

	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduledProgram::DebugDump");
#endif
	}


// End of file
