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
* Description:    Database where to store scheduled events.*
*/




// INCLUDE FILES
#include "CCseScheduleDB.h"         // Header file for this class
#include <ipvideo/CCseScheduledProgram.h>   // Represent one schedule in database
#include <bautils.h>
#include "CseDebug.h"               // Debug macros
#include <babackup.h>

// EXTERNAL DATA STRUCTURES
// None

// EXTERNAL FUNCTION PROTOTYPES  
// None

// CONSTANTS
_LIT( KCseCDiskPrefix,  "c:"                         );
_LIT( KCseSqlGetAll,    "SELECT * FROM ScheduleEvent");
_LIT( KCseSelectFromDb, "SELECT * FROM "             );
_LIT( KCseWhere,        " WHERE "                    );
_LIT( KCseEqual,        " = "                        );
_LIT( KCseOrderBy,      " ORDER BY "                 );
_LIT( KCseAnd,          " AND "                      );
_LIT( KCseLess,         " < "                        );
_LIT( KCseMore,         " > "                        );

static TUint KScheduleNameMaxLength = 255;
static TUint KCseDatbaseVersionNumber = 1;

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
// CCseScheduleDB::CCseScheduleDB()
//
// ---------------------------------------------------------------------------
CCseScheduleDB::CCseScheduleDB() : iDbAccess( ECseDbOpen )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::CCseScheduleDB");
	// Default C++ Constructor		
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::CCseScheduleDB");
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::~CCseScheduleDB()
//
// ---------------------------------------------------------------------------	
CCseScheduleDB::~CCseScheduleDB()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::~CCseScheduleDB");
	       	
	iScheduleDb.Close();
	delete iScheduleFileStore;
	iFsSession.Close(); 
	
	if( iCompactTimer )
        {
        iCompactTimer->Cancel();
        delete iCompactTimer;
        }
    delete iBackupWrapper;
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::~CCseScheduleDB");
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::NewL()
//
// ---------------------------------------------------------------------------	
CCseScheduleDB* CCseScheduleDB::NewL()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::NewL");
    CCseScheduleDB* self = new ( ELeave ) CCseScheduleDB();    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );    
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::NewL");
    
    return self;
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::ConstructL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::ConstructL()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::ConstructL");
    
    User::LeaveIfError( iFsSession.Connect() );
    
    iCompactTimer = CPeriodic::NewL( CActive::EPriorityStandard );
            
    iBackupWrapper = CBaBackupSessionWrapper::NewL();
	
	TRAPD( err, OpenOrCreateDbL() );

    if ( err != KErrNone )
        {
		CSELOGSTRING2_HIGH_LEVEL( "CCseScheduleDB::ConstructL - Opening or creating the database FAILED: %d", err );

        if ( err != KErrNoMemory && 
             err != KErrLocked && 
             err != KErrDisMounted &&
             err != KErrDiskFull &&
             err != KErrNotReady )
            {
            // Delete and recreate database file. Cannot recover other way. 
    		CSELOGSTRING_HIGH_LEVEL( "CCseScheduleDB::ConstructL - deleting database" );
            RemoveDbL();
    		CSELOGSTRING_HIGH_LEVEL( "CCseScheduleDB::ConstructL - recreating database" );
            TRAPD( err, OpenOrCreateDbL() );
            if ( err != KErrNone )
                {
                CSELOGSTRING2_HIGH_LEVEL( "CCseScheduleDB::ConstructL couldnt recreate database (%d), leaving", err );
                User::Leave( err );
                }
            }
        else
            {
            CSELOGSTRING2_HIGH_LEVEL( "CCseScheduleDB::ConstructL leaving (%d)", err );
            User::Leave( err );
            }
        }
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::ConstructL");	
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::RemoveDbL()
// ---------------------------------------------------------------------------
void CCseScheduleDB::RemoveDbL()
    {
    delete iBackupWrapper;
	iBackupWrapper = NULL;
    iScheduleDb.Close();
	delete iScheduleFileStore;
	iScheduleFileStore = NULL;
    if ( BaflUtils::FileExists( iFsSession, iDbFile ) )
        {
        TInt err( iFsSession.Delete( iDbFile ) );
		CSELOGSTRING2_HIGH_LEVEL("CCseScheduleDB:: deleting database file, err = %d", err);
        }
	}


// ---------------------------------------------------------------------------
// CCseScheduleDB::GetApplicationSchedulesL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::GetApplicationSchedulesL( const TInt32 aAppUid,
								RPointerArray<CCseScheduledProgram>& aArray )
	{
	CSELOGSTRING2_HIGH_LEVEL(">>>CCseScheduleDB::GetApplicationSchedulesL - AppUid: %d",
	                          aAppUid);
	
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	// Gets program occurence(s) that are going to happen next
	// from database
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleApplicationUIDCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( aAppUid );
	sqlStatement.Append( KCseOrderBy );
	sqlStatement.Append( KCseScheduleStartTimeCol );

	// Order table based on start time
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	// Get the SQL table indexes.
	TDbColNo keyIndex = colSetOrder->ColNo( KCseScheduleDbKeyCol );
	TDbColNo nameIndex = colSetOrder->ColNo( KCseScheduleNameCol );
	TDbColNo startTimeIndex = colSetOrder->ColNo( KCseScheduleStartTimeCol );
	TDbColNo endTimeIndex = colSetOrder->ColNo( KCseScheduleEndTimeCol );
	TDbColNo applicationUidIdIndex = colSetOrder->ColNo( KCseScheduleApplicationUIDCol );
	TDbColNo plugInUidIndex = colSetOrder->ColNo( KCseSchedulePlugInUIDCol );
	TDbColNo applicationSpecificIndex = colSetOrder->ColNo( KCseScheduleApplicationSpecificCol );
	TDbColNo scheduleTypeIndex = colSetOrder->ColNo( KCseScheduleTypeCol );
	TDbColNo pluginTypeIndex = colSetOrder->ColNo( KCseSchedulePluginTypeCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );
	
	// Loop through the tables
	while ( view.AtRow() )
		{
		// Create new CCseSceduledProgram to be added to array
		CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
		CleanupStack::PushL( prog );			
		
		// Fill created schedule with DB information
		view.GetL();
		prog->SetDbIdentifier( view.ColUint32( keyIndex ) );
		prog->SetName( view.ColDes8( nameIndex ) );
		prog->SetStartTime( view.ColTime( startTimeIndex ) );
		prog->SetEndTime( view.ColTime( endTimeIndex ) );
		prog->SetAppUid( view.ColInt32( applicationUidIdIndex ) );
		prog->SetPluginUid( view.ColInt32( plugInUidIndex ) );
		prog->SetScheduleType( view.ColInt32( scheduleTypeIndex ) );
		prog->SetPluginType( view.ColInt32( pluginTypeIndex ) );
		
		TInt streamLen( 0 );
		streamLen = view.ColLength( applicationSpecificIndex );
	
		HBufC8* appDataBuffer = HBufC8::NewL( streamLen );
		
		CleanupStack::PushL( appDataBuffer );
		
		TPtr8 buffPtr( appDataBuffer->Des() );
		RDbColReadStream readStream;
		readStream.OpenLC( view, applicationSpecificIndex );
		readStream.ReadL( buffPtr, streamLen );
		readStream.Release();
		CleanupStack::PopAndDestroy(); // readStream

		prog->SetApplicationDataL( *appDataBuffer );
		
		CleanupStack::PopAndDestroy( appDataBuffer );
		User::LeaveIfError( aArray.Append( prog ) );
		CleanupStack::Pop( prog );
		view.NextL();
		}
		
	CleanupStack::PopAndDestroy( &view ); // Closes view
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::GetApplicationSchedulesL");	
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::OpenOrCreateDbL()
//
// Create a new database. 
// ---------------------------------------------------------------------------
void CCseScheduleDB::OpenOrCreateDbL()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::OpenOrCreateDbL");
	TBuf<KMaxFileName> path;
	
	// Get path to CSE's private directory
	TInt err( iFsSession.PrivatePath( path ) );
	if ( err != KErrNone )
		{
		CSELOGSTRING2_HIGH_LEVEL("iFsSession.PrivatePath() failed: %d", err );
		User::Leave( err );
		}

	iDbFile.Zero();
	iDbFile.Append( KCseCDiskPrefix );
	iDbFile.Append( path );
	if ( BaflUtils::CheckFolder( iFsSession, iDbFile ) != KErrNone ) 
		{
		err = iFsSession.CreatePrivatePath( EDriveC );
		if ( err != KErrNone )
			{
			CSELOGSTRING2_HIGH_LEVEL( "CreatePrivatePath failed! %d", err );
			User::Leave( err );
			}
		}
	
	iDbFile.Append( KCseScheduleDBName() );
	
	// If file doesn't exist create it
    if ( !BaflUtils::FileExists( iFsSession, iDbFile ) )
        {
        CSELOGSTRING_HIGH_LEVEL("CCseScheduleDB::OpenOrCreateDbL Create New");
        iScheduleDb.Close();
                    
        if( iScheduleFileStore )
            {
            delete iScheduleFileStore;
            iScheduleFileStore = NULL;
            }
            
		iScheduleFileStore = CPermanentFileStore::ReplaceL( iFsSession,
                                                            iDbFile,
                                                            EFileRead|EFileWrite );
		// Set file store type
		iScheduleFileStore->SetTypeL( iScheduleFileStore->Layout() ); 
    	// Create stream object
	    TStreamId id = iScheduleDb.CreateL( iScheduleFileStore );   
		// Keep database id as root of store  
    	iScheduleFileStore->SetRootL( id ); 
		// Complete creation by commiting
    	iScheduleFileStore->CommitL();                              

        CreateDatabaseTableL( iScheduleDb );        
        CreateScheduleTableL( iScheduleDb );
        SetDatabaseVersionL( );
        }
    // Or if it exists then just open it.
	else
		{
		iScheduleDb.Close();
	    delete iScheduleFileStore;
		iScheduleFileStore = NULL;
		iScheduleFileStore = CPermanentFileStore::OpenL( iFsSession, iDbFile,
														EFileRead|EFileWrite );
		iScheduleFileStore->SetTypeL( iScheduleFileStore->Layout() );
		iScheduleDb.OpenL( iScheduleFileStore, iScheduleFileStore->Root() );
    	CheckDbVersionL();    	
		User::LeaveIfError( iScheduleDb.Compact() );
		}
				
	if ( !iBackupWrapper )
	    {
	    iBackupWrapper = CBaBackupSessionWrapper::NewL();
	    }
	    
	iBackupWrapper->RegisterFileL( iDbFile, *this );

    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::OpenOrCreateDbL");
    }
    
// ---------------------------------------------------------------------------
// CCseScheduleDB::CheckDbVersionL
//
// ---------------------------------------------------------------------------
void CCseScheduleDB::CheckDbVersionL( )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::CheckDbVersionL");
	
	TInt dbVersion( 0 );	
	
	TRAPD( leave, dbVersion = GetDbVersionL() );
	
	if ( dbVersion != KCseDatbaseVersionNumber || leave != KErrNone )
	    {
    	CSELOGSTRING3_HIGH_LEVEL(
    	    "CCseScheduleDB::CheckDbVersionL Not ok, version: %d, error: %d",
    	    dbVersion, leave );
    	
	    // Destroy the old one
	    iScheduleDb.Close();
	    delete iScheduleFileStore;
	    iScheduleFileStore = NULL;
	    
	    // Create new one
	    iScheduleFileStore = CPermanentFileStore::ReplaceL( iFsSession,
                                                            iDbFile,
                                                            EFileRead|EFileWrite );
                                                        
		// Set file store type
		iScheduleFileStore->SetTypeL( iScheduleFileStore->Layout() ); 
    	// Create stream object
	    TStreamId id = iScheduleDb.CreateL( iScheduleFileStore );   
		// Keep database id as root of store  
    	iScheduleFileStore->SetRootL( id ); 
		// Complete creation by commiting
    	iScheduleFileStore->CommitL();                              

    	CreateDatabaseTableL( iScheduleDb );
        CreateScheduleTableL( iScheduleDb );
	    SetDatabaseVersionL( );
	    }
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::CheckDbVersionL");		
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::GetDbVersionL
//
// ---------------------------------------------------------------------------
TInt CCseScheduleDB::GetDbVersionL( )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::GetDbVersionL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	TInt dbVersion(0);
	
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );
	sqlStatement.Append( KCseDatabaseTable );
		
	// Order table based on start time
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	// Get the next reminder
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	TDbColNo versionIndex = colSetOrder->ColNo( KCseDatabaseVersionCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );
		
	if ( view.AtRow() )
		{				
		view.GetL();
		dbVersion = view.ColInt32( versionIndex );		
		}
	CleanupStack::PopAndDestroy( &view ); // Closes view

	CSELOGSTRING2_HIGH_LEVEL(
	    "<<<CCseScheduleDB::GetDbVersionL, version: %d",
	    dbVersion );
	return dbVersion;
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::CreateScheduleTableL
//
// Creates Schedule table.
// ---------------------------------------------------------------------------
void CCseScheduleDB::CreateScheduleTableL( RDbStoreDatabase& aDatabase ) const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::CreateScheduleTableL");
    
    // Create columns for the database.     
    TDbCol keyCol( KCseScheduleDbKeyCol, EDbColUint32 );
    keyCol.iAttributes = TDbCol::EAutoIncrement;
    
	TDbCol nameCol( KCseScheduleNameCol, EDbColText8, KScheduleNameMaxLength );		
	TDbCol startTimeCol( KCseScheduleStartTimeCol, EDbColDateTime );
	TDbCol endTimeCol( KCseScheduleEndTimeCol, EDbColDateTime );
    TDbCol applicationUidCol( KCseScheduleApplicationUIDCol, EDbColInt32 );   
	TDbCol plugInUidCol( KCseSchedulePlugInUIDCol, EDbColInt32 );    
	TDbCol typeCol( KCseScheduleTypeCol, EDbColInt32 );    	
	TDbCol applicationSpecificCol( KCseScheduleApplicationSpecificCol, EDbColLongBinary );
	TDbCol runCountCol( KCseScheduleRunCountCol, EDbColInt32 );	
	TDbCol stateCol( KCseScheduleStateCol, EDbColInt32 );	
	TDbCol pluginTypeCol( KCseSchedulePluginTypeCol, EDbColInt32 );	
	TDbCol reserved1Col( KCseScheduleReserved1Col, EDbColInt32 );	
	TDbCol reserved2Col( KCseScheduleReserved2Col, EDbColInt32 );	

	// Create column set and add defined columns in to the set
    CDbColSet* scheduleColSet = CDbColSet::NewLC();
    scheduleColSet->AddL( keyCol );
	scheduleColSet->AddL( nameCol );
	scheduleColSet->AddL( startTimeCol );
	scheduleColSet->AddL( endTimeCol );
    scheduleColSet->AddL( applicationUidCol );
    scheduleColSet->AddL( plugInUidCol );
	scheduleColSet->AddL( typeCol );
    scheduleColSet->AddL( applicationSpecificCol );	
    scheduleColSet->AddL( runCountCol );	
    scheduleColSet->AddL( stateCol );
    scheduleColSet->AddL( pluginTypeCol );
    scheduleColSet->AddL( reserved1Col );
    scheduleColSet->AddL( reserved2Col );
    
    // Create new table to the database with created columnset
    User::LeaveIfError( aDatabase.CreateTable( KCseScheduleTable, 
										      *scheduleColSet ) ); 
    
    CleanupStack::PopAndDestroy( scheduleColSet );
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::CreateScheduleTableL");
    }

// ---------------------------------------------------------------------------
// CCseScheduleDB::CreateDatabaseTableL
//
// Creates Database table.
// ---------------------------------------------------------------------------
void CCseScheduleDB::CreateDatabaseTableL( RDbStoreDatabase& aDatabase )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::CreateDatabaseTableL");

    // Create columns for the database.     
    TDbCol versionCol( KCseDatabaseVersionCol, EDbColInt32 );
    TDbCol reserved1Col( KCseDatabaseReserved1Col, EDbColInt32 );
    TDbCol reserved2Col( KCseDatabaseReserved2Col, EDbColInt32 );

	// Create column set and add defined columns in to the set
    CDbColSet* databaseColSet = CDbColSet::NewLC();
    databaseColSet->AddL( versionCol );
    databaseColSet->AddL( reserved1Col );
    databaseColSet->AddL( reserved2Col );
    
    // Create new table to the database with created columnset
    User::LeaveIfError( aDatabase.CreateTable( KCseDatabaseTable, 
										      *databaseColSet ) ); 
    
    CleanupStack::PopAndDestroy( databaseColSet );
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::CreateDatabaseTableL");
    }

// ---------------------------------------------------------------------------
// CCseScheduleDB::SetDatabaseVersionL
//
// Creates Database table.
// ---------------------------------------------------------------------------
void CCseScheduleDB::SetDatabaseVersionL( )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::SetDatabaseVersionL");
    
    iSqlSchedule.Zero(); 
    iSqlSchedule.Append( KCseSelectFromDb );
    iSqlSchedule.Append( KCseDatabaseTable );
    
    RDbView view;
    CleanupClosePushL( view );
    
    // Prepare DB for update
    User::LeaveIfError(view.Prepare( iScheduleDb,
    					TDbQuery( iSqlSchedule ),
    					TDbWindow::EUnlimited,
    					RDbView::EInsertOnly ));    
    view.InsertL();
    
    // Create colomn set (row) to be added
    CDbColSet* databaseColSet = view.ColSetL();
    CleanupStack::PushL( databaseColSet );

    // Fill row with Schedule information
	view.SetColL( databaseColSet->ColNo( KCseDatabaseVersionCol ),
										 KCseDatbaseVersionNumber );
    CleanupStack::PopAndDestroy( databaseColSet );
    view.PutL();
	CleanupStack::PopAndDestroy( &view ); // closes view
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::SetDatabaseVersionL");
    }

// ---------------------------------------------------------------------------
// CCseScheduleDB::AddScheduleL()
//
// ---------------------------------------------------------------------------
void CCseScheduleDB::AddScheduleL( CCseScheduledProgram& aData )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::AddScheduleL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	iSqlSchedule.Zero();
	iSqlSchedule.Append( KCseSqlGetAll );
    RDbView view;
    CleanupClosePushL( view );

    // Prepare DB for update
    User::LeaveIfError(view.Prepare( iScheduleDb,
    					TDbQuery( iSqlSchedule ),
    					TDbWindow::EUnlimited,
    					RDbView::EInsertOnly ));    
    view.InsertL();
    
    // Create colomn set (row) to be added
    CDbColSet* scheduleColSet = view.ColSetL();
    CleanupStack::PushL( scheduleColSet );

    // Fill row with Schedule information
	view.SetColL( scheduleColSet->ColNo( KCseScheduleNameCol ),
										aData.Name() );

	view.SetColL( scheduleColSet->ColNo( KCseScheduleStartTimeCol ),
										aData.StartTime() );

	view.SetColL( scheduleColSet->ColNo( KCseScheduleEndTimeCol ),
										aData.EndTime() );

	view.SetColL( scheduleColSet->ColNo( KCseScheduleApplicationUIDCol ), 
										aData.AppUid() );

	view.SetColL( scheduleColSet->ColNo( KCseSchedulePlugInUIDCol ), 
										 aData.PluginUid() );	

	view.SetColL( scheduleColSet->ColNo( KCseScheduleTypeCol ), 
										 aData.ScheduleType() );	
	
	view.SetColL( scheduleColSet->ColNo( KCseScheduleRunCountCol ), 
										 0 );
										 
    view.SetColL( scheduleColSet->ColNo( KCseScheduleStateCol ), 
										 ECseWaiting );
										 
    view.SetColL( scheduleColSet->ColNo( KCseSchedulePluginTypeCol ), 
										 aData.PluginType() );
										 
	// Description is long one, it needs to be handled by stream
	RDbColWriteStream write;
	write.OpenLC( view, scheduleColSet->ColNo( KCseScheduleApplicationSpecificCol ) );
	write.WriteL( aData.ApplicationData() );
	write.CommitL();
	CleanupStack::PopAndDestroy(); // write
	
	TDbColNo keyColumnNo = scheduleColSet->ColNo( KCseScheduleDbKeyCol );
    CleanupStack::PopAndDestroy( scheduleColSet );
    view.PutL();

    // Get new program key
    aData.SetDbIdentifier( view.ColUint32( keyColumnNo ) );
    
    CleanupStack::PopAndDestroy( &view ); // closes view
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::AddScheduleL");
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::RemoveScheduleL()
//
// ---------------------------------------------------------------------------
void CCseScheduleDB::RemoveScheduleL( const TUint32 aDbIdentifier )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::RemoveScheduleL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	// Create SQL string to find given schedule from DB
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleDbKeyCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( aDbIdentifier );
	
	User::LeaveIfError( iScheduleDb.Begin() );
	
	// Prepare DB to update
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
									  TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	view.FirstL();

	TInt count( 0 );

    // If schedule found, remove it.
	while ( view.AtRow() )
		{
		view.DeleteL();
		count++;
		view.NextL();	
		}		
	
	CleanupStack::PopAndDestroy( &view ); // closes view
	User::LeaveIfError( iScheduleDb.Commit() );
	    	
    CSELOGSTRING3_HIGH_LEVEL("---- %d Items for identifier %d removed from DB", count, aDbIdentifier);
	if ( count == 0 )
		{
		User::Leave( KErrNotFound );	
		}
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::RemoveScheduleL");
	}
	
// ---------------------------------------------------------------------------
// CCseScheduleDB::GetNextScheduleTimeL()
//
// ---------------------------------------------------------------------------	
TTime CCseScheduleDB::GetNextScheduleTimeL()
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::GetNextScheduleTimeL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	// Create SQL satring to get schedule occurence(s) that are
	// going to happen next from database.
	TTime nextScheduleTime( 0 );
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleStateCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( ECseWaiting );
	sqlStatement.Append( KCseOrderBy );
	sqlStatement.Append( KCseScheduleStartTimeCol );
		
	// Order table based on start time
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	// Get the next schedule
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	TDbColNo startTimeIndex = colSetOrder->ColNo( KCseScheduleStartTimeCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );
	
	// Schedule found get its start time.
	if ( view.AtRow() )	
		{		
		view.GetL();				
		nextScheduleTime = view.ColTime( startTimeIndex );
		}
		
	CleanupStack::PopAndDestroy( &view ); // Closes view
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::GetNextScheduleTimeL");
	
	return nextScheduleTime;
	}
	
	

// ---------------------------------------------------------------------------
// CCseScheduleDB::FillScheduleArrayByTimeL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::FillScheduleArrayByTimeL( const TTime& aScheduleTime, 
					RPointerArray<CCseScheduledProgram>& aNextScheduleArray )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::FillScheduleArrayByTimeL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	// Gets program occurence(s) that are going to happen next
	// from database
	TBuf<100> startTimeBuf;
	
	_LIT( KDateTimeFormat,"# %1%*D/%2%*M/%3%*Y %H:%T:%S.%C#" ); 

	aScheduleTime.FormatL( startTimeBuf, KDateTimeFormat );
		
	// Create SQL string to get all schedules to be happen at given time
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleStartTimeCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.Append( startTimeBuf );
		
	// Order table based on start time
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	// Get the next reminder
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	// Get column indexes
	TDbColNo keyIndex = colSetOrder->ColNo( KCseScheduleDbKeyCol );
	TDbColNo nameIndex = colSetOrder->ColNo( KCseScheduleNameCol );
	TDbColNo startTimeIndex = colSetOrder->ColNo( KCseScheduleStartTimeCol );
	TDbColNo endTimeIndex = colSetOrder->ColNo( KCseScheduleEndTimeCol );
	TDbColNo applicationUidIdIndex = colSetOrder->ColNo( KCseScheduleApplicationUIDCol );
	TDbColNo plugInUidIndex = colSetOrder->ColNo( KCseSchedulePlugInUIDCol );
	TDbColNo typeIndex = colSetOrder->ColNo( KCseScheduleTypeCol );
	TDbColNo applicationSpecificIndex = colSetOrder->ColNo( KCseScheduleApplicationSpecificCol );
	TDbColNo pluginTypeIndex = colSetOrder->ColNo( KCseSchedulePluginTypeCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );
	
	// Loop through all found schedules.
	while ( view.AtRow() )
		{
		// Create new schedule to be added to array
		CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
		CleanupStack::PushL( prog );			
		view.GetL();
		
		// Copy information from DB to schedule.
		prog->SetDbIdentifier( view.ColUint32( keyIndex ) );
		prog->SetName( view.ColDes8( nameIndex ) );
		prog->SetStartTime( view.ColTime( startTimeIndex ) );
		prog->SetEndTime( view.ColTime( endTimeIndex ) );
		prog->SetAppUid( view.ColInt32( applicationUidIdIndex ) );
		prog->SetPluginUid( view.ColInt32( plugInUidIndex ) );
		prog->SetScheduleType( CCseScheduledProgram::TCseScheduleType( 
		                       view.ColInt32( typeIndex ) ) );
        prog->SetPluginType( CCseScheduledProgram::TCsePluginType( 
							   view.ColInt32( pluginTypeIndex ) ) );
		TInt streamLen( view.ColLength( applicationSpecificIndex ) );
	
		HBufC8* appDataBuffer = HBufC8::NewL( streamLen );
		CleanupStack::PushL( appDataBuffer );
		
		TPtr8 buffPtr( appDataBuffer->Des() );
		RDbColReadStream readStream;
		readStream.OpenLC( view, applicationSpecificIndex );
		readStream.ReadL( buffPtr, streamLen );
		readStream.Release();
		CleanupStack::PopAndDestroy(); // readStream;

		prog->SetApplicationDataL( *appDataBuffer );
		
		CleanupStack::PopAndDestroy( appDataBuffer );
		User::LeaveIfError( aNextScheduleArray.Append( prog ) );
		CleanupStack::Pop( prog );
		
		// And next schedule...
		view.NextL();
		}
		
	CleanupStack::PopAndDestroy( &view ); // Closes view
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::FillScheduleArrayByTimeL");
	}


// ---------------------------------------------------------------------------
// CCseScheduleDB::GetScheduleByDbIdentifierL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::GetScheduleByDbIdentifierL( const TUint32 aDbIdentifier, 
								           CCseScheduledProgram* aSchedule )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::GetScheduleByDbIdentifierL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	if ( !aSchedule )
		{
		return; 
		}		
	
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleDbKeyCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( aDbIdentifier );
		
	// Order table based on start time
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	// Get the next reminder
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	TDbColNo keyIndex = colSetOrder->ColNo( KCseScheduleDbKeyCol );
	TDbColNo nameIndex = colSetOrder->ColNo( KCseScheduleNameCol );
	TDbColNo startTimeIndex = colSetOrder->ColNo( KCseScheduleStartTimeCol );
	TDbColNo endTimeIndex = colSetOrder->ColNo( KCseScheduleEndTimeCol );
	TDbColNo applicationUidIdIndex = colSetOrder->ColNo( KCseScheduleApplicationUIDCol );
	TDbColNo plugInUidIndex = colSetOrder->ColNo( KCseSchedulePlugInUIDCol );
	TDbColNo typeIndex = colSetOrder->ColNo( KCseScheduleTypeCol );
	TDbColNo applicationSpecificIndex = colSetOrder->ColNo( KCseScheduleApplicationSpecificCol );
	TDbColNo pluginTypeIndex = colSetOrder->ColNo( KCseSchedulePluginTypeCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );
		
	while ( view.AtRow() )
		{				
		view.GetL();
		aSchedule->SetDbIdentifier( view.ColUint32( keyIndex ) );
		aSchedule->SetName( view.ColDes8( nameIndex ) );
		aSchedule->SetStartTime( view.ColTime( startTimeIndex ) );
		aSchedule->SetEndTime( view.ColTime( endTimeIndex ) );
		aSchedule->SetAppUid( view.ColInt32( applicationUidIdIndex ) );
		aSchedule->SetPluginUid( view.ColInt32( plugInUidIndex ) );
		
		aSchedule->SetScheduleType( CCseScheduledProgram::TCseScheduleType( 
							   view.ColInt32( typeIndex ) ) );
        aSchedule->SetPluginType( CCseScheduledProgram::TCsePluginType( 
							   view.ColInt32( pluginTypeIndex ) ) );

		TInt streamLen( 0 );
		streamLen = view.ColLength( applicationSpecificIndex );
	
		HBufC8* appDataBuffer = HBufC8::NewL( streamLen );
		CleanupStack::PushL( appDataBuffer );
		
		TPtr8 buffPtr( appDataBuffer->Des() );
		RDbColReadStream readStream;
		readStream.OpenLC( view, applicationSpecificIndex ); // |->1
		readStream.ReadL(buffPtr, streamLen);
		readStream.Release();
		CleanupStack::PopAndDestroy( ); // 1<-| Popping pointer to Col that we dont have here
		TPtrC8 ptr8( appDataBuffer->Des() );
		aSchedule->SetApplicationDataL( *appDataBuffer );
		CleanupStack::PopAndDestroy( appDataBuffer );
		view.NextL();
		}
		
	CleanupStack::PopAndDestroy( &view ); // Closes view
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::GetScheduleByDbIdentifierL");
	}


// ---------------------------------------------------------------------------
// CCseScheduleDB::GetOverlappingSchedulesL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::GetOverlappingSchedulesL( const TInt32 aType,
                                               const TTime& aStartTime,
                                               const TTime& aEndTime,
                                               RPointerArray<CCseScheduledProgram>& aResultArray
                                               )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::GetOverlappingSchedulesL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	TBuf<100> startTimeBuf;
	_LIT( KDateTimeFormat,"# %1%*D/%2%*M/%3%*Y %H:%T:%S#" ); 
	aStartTime.FormatL( startTimeBuf, KDateTimeFormat );
	TBuf<100> endTimeBuf;
	aEndTime.FormatL( endTimeBuf, KDateTimeFormat );  

    // Create SQL string to find all overlapping schedules from database between given
    // timeframe.
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleTypeCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( aType );
	sqlStatement.Append( KCseAnd );
	sqlStatement.Append( KCseScheduleStartTimeCol );
	sqlStatement.Append( KCseLess );
	sqlStatement.Append( endTimeBuf );
	sqlStatement.Append( KCseAnd );
	sqlStatement.Append( KCseScheduleEndTimeCol );
	sqlStatement.Append( KCseMore );
	sqlStatement.Append( startTimeBuf );	
	sqlStatement.Append( KCseOrderBy );
	sqlStatement.Append( KCseScheduleStartTimeCol );

    // Order table		
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	// Move to the first row
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	// Get colon indexes.
	TDbColNo keyIndex = colSetOrder->ColNo( KCseScheduleDbKeyCol );
	TDbColNo nameIndex = colSetOrder->ColNo( KCseScheduleNameCol );
	TDbColNo startTimeIndex = colSetOrder->ColNo( KCseScheduleStartTimeCol );
	TDbColNo endTimeIndex = colSetOrder->ColNo( KCseScheduleEndTimeCol );
	TDbColNo applicationUidIdIndex = colSetOrder->ColNo( KCseScheduleApplicationUIDCol );
	TDbColNo plugInUidIndex = colSetOrder->ColNo( KCseSchedulePlugInUIDCol );
	TDbColNo typeIndex = colSetOrder->ColNo( KCseScheduleTypeCol );
	TDbColNo applicationSpecificIndex = colSetOrder->ColNo( KCseScheduleApplicationSpecificCol );
	TDbColNo pluginTypeIndex = colSetOrder->ColNo( KCseSchedulePluginTypeCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );
	
	// Loop through all found schedules.
	while ( view.AtRow() )
		{
		// Create new schedule to be added to array.
		CCseScheduledProgram* schedule = CCseScheduledProgram::NewL();
		CleanupStack::PushL( schedule );	
		view.GetL();
		
		// Set schedule information.
		schedule->SetDbIdentifier( view.ColUint32( keyIndex ) );
		schedule->SetName( view.ColDes8( nameIndex ) );
		schedule->SetStartTime( view.ColTime( startTimeIndex ) );
		schedule->SetEndTime( view.ColTime( endTimeIndex ) );
		schedule->SetAppUid( view.ColInt32( applicationUidIdIndex ) );
		schedule->SetPluginUid( view.ColInt32( plugInUidIndex ) );
		
		schedule->SetScheduleType( CCseScheduledProgram::TCseScheduleType( 
							   view.ColInt32( typeIndex ) ) );
        schedule->SetPluginType( CCseScheduledProgram::TCsePluginType( 
							   view.ColInt32( pluginTypeIndex ) ) );

		TInt streamLen( 0 );
		streamLen = view.ColLength( applicationSpecificIndex );
	
		HBufC8* appDataBuffer = HBufC8::NewL( streamLen );
		CleanupStack::PushL( appDataBuffer );
		
		TPtr8 buffPtr( appDataBuffer->Des() );
		RDbColReadStream readStream; // |->1
		readStream.OpenLC( view, applicationSpecificIndex );
		readStream.ReadL(buffPtr, streamLen);
		readStream.Release();
		CleanupStack::PopAndDestroy( ); //  1<-| Popping pointer to Col that we dont have here
		TPtrC8 ptr8( appDataBuffer->Des() );
		schedule->SetApplicationDataL( *appDataBuffer );
		CleanupStack::PopAndDestroy( appDataBuffer );
		if ( schedule->StartTime() < aEndTime && schedule->EndTime() > aStartTime )
			{
			aResultArray.AppendL( schedule );
			CleanupStack::Pop( schedule );	
			}
		else
			{
			CleanupStack::PopAndDestroy( schedule );
			schedule = NULL;
			}
		
		// Next shcedule...
		view.NextL();
		}
	CleanupStack::PopAndDestroy( &view ); // Closes view
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::GetOverlappingSchedulesL");
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::IncreaseRunCountL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::IncreaseRunCountL( const TUint32 aDbIdentifier )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::IncreaseRunCountL");
	LeaveIfDbLockedL();
	ResetCompactTimer();

    // Create SQL string to find given schedule
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleDbKeyCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( aDbIdentifier );
		
	// Order table based on DB identifier
	RDbView readView;
	CleanupClosePushL( readView );
    
    User::LeaveIfError( readView.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( readView.EvaluateAll() );
    	
	// Get the next reminder
	readView.FirstL();
	
	// First we read existint run count
	CDbColSet* readColSet = readView.ColSetL();
	CleanupStack::PushL( readColSet );		
	TDbColNo runCountCol = readColSet->ColNo( KCseScheduleRunCountCol );	
	CleanupStack::PopAndDestroy( readColSet );
	TInt32 runCount( 0 );
	if ( readView.AtRow() )
		{				
		readView.GetL();
		runCount = readView.ColInt32( runCountCol );
		runCount++;		
		}
	// Close read view
	CleanupStack::PopAndDestroy( &readView );

	// Update run count
	if ( runCount > 0 )
		{
		// Open write view
        RDbView writeView;
		CleanupClosePushL( writeView );    
        User::LeaveIfError( writeView.Prepare( iScheduleDb,
    				        TDbQuery( sqlStatement ),
    					    TDbWindow::EUnlimited,
    					    RDbView::EUpdatable ));
    	User::LeaveIfError( writeView.EvaluateAll() );
    	
    	writeView.FirstL();
    	CDbColSet* writeColSet = writeView.ColSetL();
		CleanupStack::PushL( writeColSet );		
		TDbColNo runCountColNo = writeColSet->ColNo( KCseScheduleRunCountCol );	
		CleanupStack::PopAndDestroy( writeColSet );

    	writeView.UpdateL();
    	
    	// If we found (as we should) schedule again we update its value.
    	if ( writeView.AtRow() )
    	    {    	        
	        writeView.SetColL( runCountColNo, runCount );
    	    }
	    
	    writeView.PutL();
        
        CleanupStack::PopAndDestroy( &writeView );
		}	
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::IncreaseRunCountL");
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::GetRunCountL()
//
// ---------------------------------------------------------------------------	
TInt32 CCseScheduleDB::GetRunCountL( const TUint32 aDbIdentifier )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::GetRunCountL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	TInt32 runCount( 0 );

    // Create SQL string to find given schedule	
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleDbKeyCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( aDbIdentifier );
		
	// Order table based on start time
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	// Get the next reminder
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
		
	TDbColNo runCountCol = colSetOrder->ColNo( KCseScheduleRunCountCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );

    // If we found schedule get its run count.
	if ( view.AtRow() )
		{				
		view.GetL();
		
		runCount = view.ColInt32( runCountCol );
		}
		
	CleanupStack::PopAndDestroy( &view ); // Closes view
	
	CSELOGSTRING2_HIGH_LEVEL("<<<CCseScheduleDB::GetRunCountL: runCount=%d", runCount);
	
	return runCount;
	}
	
// ---------------------------------------------------------------------------
// CCseScheduleDB::SetScheduleStateL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::SetScheduleStateL( const TUint32 aDbIdentifier,
                                        const TInt32 aState )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::SetScheduleStateL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
    // Create SQL string to find given schedule	
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleDbKeyCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( aDbIdentifier );
		
	// Open write view
    RDbView writeView;
	CleanupClosePushL( writeView );    
    User::LeaveIfError( writeView.Prepare( iScheduleDb,
				        TDbQuery( sqlStatement ),
					    TDbWindow::EUnlimited,
					    RDbView::EUpdatable ));
	User::LeaveIfError( writeView.EvaluateAll() );
	
	writeView.FirstL();
	CDbColSet* writeColSet = writeView.ColSetL();
	CleanupStack::PushL( writeColSet );		
	TDbColNo stateColNo = writeColSet->ColNo( KCseScheduleStateCol );	
	CleanupStack::PopAndDestroy( writeColSet );

	writeView.UpdateL();
	
	// If we found schedule, we change the column value.
	if ( writeView.AtRow() )
	    {    	        
        writeView.SetColL( stateColNo, aState );
	    }
    
    writeView.PutL();
    CleanupStack::PopAndDestroy( &writeView );
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::SetScheduleStateL");
	}
	
// ---------------------------------------------------------------------------
// CCseScheduleDB::GetScheduleStateL()
//
// ---------------------------------------------------------------------------	
TInt32 CCseScheduleDB::GetScheduleStateL( const TUint32 aDbIdentifier )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::GetScheduleStateL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	// Create SQL string to find given schedule	
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleDbKeyCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( aDbIdentifier );
		
	// Order table based on DB identifier
	RDbView readView;
	CleanupClosePushL( readView );    
    User::LeaveIfError( readView.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( readView.EvaluateAll() );    		
    	
	// Get the reminder
	readView.FirstL();
	
	CDbColSet* readColSet = readView.ColSetL();
	CleanupStack::PushL( readColSet );		
	TDbColNo stateCol = readColSet->ColNo( KCseScheduleStateCol );	
	CleanupStack::PopAndDestroy( readColSet );
	TInt32 state( KErrNotFound );
	
    // If we found schedule, get the schedule state.
	if ( readView.AtRow() )
		{				
		readView.GetL();
		state = readView.ColInt32( stateCol );
		}
	// Close read view
	CleanupStack::PopAndDestroy( &readView );
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::GetScheduleStateL");
	
	return state;
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::StartupSchedulesL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::StartupSchedulesL( RPointerArray<CCseScheduledProgram>& aScheduleArray )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::StartupSchedulesL");
	LeaveIfDbLockedL();
	ResetCompactTimer();

	// Create SQL string to find wanted schedules.
	// At startup phase we are first interested for
	// those schedules that were on run when phone was
	// shutdown (eg runcount is bigger than 0).	
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleRunCountCol );
	sqlStatement.Append( KCseMore );
	sqlStatement.AppendNum( 0 );

    // Order table
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	// Get the first reminder
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	// Column indexes
	TDbColNo keyIndex = colSetOrder->ColNo( KCseScheduleDbKeyCol );
	TDbColNo nameIndex = colSetOrder->ColNo( KCseScheduleNameCol );
	TDbColNo startTimeIndex = colSetOrder->ColNo( KCseScheduleStartTimeCol );
	TDbColNo endTimeIndex = colSetOrder->ColNo( KCseScheduleEndTimeCol );
	TDbColNo applicationUidIdIndex = colSetOrder->ColNo( KCseScheduleApplicationUIDCol );
	TDbColNo plugInUidIndex = colSetOrder->ColNo( KCseSchedulePlugInUIDCol );
	TDbColNo typeIndex = colSetOrder->ColNo( KCseScheduleTypeCol );
	TDbColNo applicationSpecificIndex = colSetOrder->ColNo( KCseScheduleApplicationSpecificCol );
	TDbColNo pluginTypeIndex = colSetOrder->ColNo( KCseSchedulePluginTypeCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );

    // Loop through the found schedules.		
	while ( view.AtRow() )
		{
		// Create CseScheduledProgram to be added to string.
		CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
		CleanupStack::PushL( prog );			
		view.GetL();
		
		// Set schedule information.
		prog->SetDbIdentifier( view.ColUint32( keyIndex ) );
		prog->SetName( view.ColDes8( nameIndex ) );
		prog->SetStartTime( view.ColTime( startTimeIndex ) );
		prog->SetEndTime( view.ColTime( endTimeIndex ) );
		prog->SetAppUid( view.ColInt32( applicationUidIdIndex ) );
		prog->SetPluginUid( view.ColInt32( plugInUidIndex ) );
		
		prog->SetScheduleType( CCseScheduledProgram::TCseScheduleType( 
							   view.ColInt32( typeIndex ) ) );
        prog->SetPluginType( CCseScheduledProgram::TCsePluginType( 
							   view.ColInt32( pluginTypeIndex ) ) );
							   
		TInt streamLen( view.ColLength( applicationSpecificIndex ) );
	
		HBufC8* appDataBuffer = HBufC8::NewL( streamLen );
		CleanupStack::PushL( appDataBuffer );
		
		TPtr8 buffPtr( appDataBuffer->Des() );
		RDbColReadStream readStream;
		readStream.OpenLC( view, applicationSpecificIndex ); // |->1
		readStream.ReadL( buffPtr, streamLen );
		readStream.Release();
		CleanupStack::PopAndDestroy( ); // 1<-| Popping pointer to Col that we dont have here

		prog->SetApplicationDataL( *appDataBuffer );
		
		CleanupStack::PopAndDestroy( appDataBuffer );
		User::LeaveIfError( aScheduleArray.Append( prog ) );
		CleanupStack::Pop( prog );
		
		// Next shcedule
		view.NextL();
		}
		
	CleanupStack::PopAndDestroy( &view ); // Closes view
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::StartupSchedulesL");
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::ResetCompactTimer()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::ResetCompactTimer()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::ResetCompactTimer");
    
    // If iCompatTimer is not running start it.
    if( !iCompactTimer )
        {
        TRAPD(err, iCompactTimer = CPeriodic::NewL( CActive::EPriorityStandard ));
        
        if(err != KErrNone )
            {
            CSELOGSTRING2_HIGH_LEVEL("CCseScheduleDB::ResetCompactTimer - Can't create iCompactTimer: %d", err);
            }
        }
        
    // If iCompactTimer is running cancel it and start over again.
    if( iCompactTimer )
        {            
        iCompactTimer->Cancel();
        
        iCompactTimer->Start( KCseDbCompactTimeout,
                              KCseDbCompactTimeout, 
                              TCallBack( CompactCallback, this ) );
        }
        
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::ResetCompactTimer");
    }

// ---------------------------------------------------------------------------
// CCseScheduleDB::CompactCallback()
//
// ---------------------------------------------------------------------------
TInt CCseScheduleDB::CompactCallback( TAny* aThis )
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::CompactCallback");
    
    // Direct call to class method.
    static_cast<CCseScheduleDB*>( aThis )->CompactDb();
    
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::CompactCallback");
    return EFalse;    
    }

// ---------------------------------------------------------------------------
// CCseScheduleDB::CompactDb()
//
// ---------------------------------------------------------------------------    
void CCseScheduleDB::CompactDb()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::CompactDb");
    // Compact database

    if( iDbAccess != ECseDbLocked )
        {
        TInt err( iScheduleDb.Compact() );
        if ( err != KErrNone )
            {
            CSELOGSTRING2_HIGH_LEVEL("iScheduleDb.Compact() failed: %d", err);    
            }
        }
    
    // Cancel timer if it is running    
	if ( iCompactTimer )
		{
		iCompactTimer->Cancel();
    	delete iCompactTimer;
    	iCompactTimer = NULL;    	
		}
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::CompactDb");
    }

// ---------------------------------------------------------------------------
// CCseScheduleDB::ChangeFileLockL()
//
// ---------------------------------------------------------------------------    
//
void CCseScheduleDB::ChangeFileLockL(
    const TDesC& /*aFileName*/,
    TFileLockFlags aFlags )
    {
    CSELOGSTRING_HIGH_LEVEL( ">>>CCseScheduleDB::ChangeFileLockL" );
    
    switch ( aFlags )
        {
        case MBackupObserver::EReleaseLockReadOnly:
        case MBackupObserver::EReleaseLockNoAccess:
            {
            // Backup and restore starting, close the db
            CloseDbFile();
            
            // Cancel timer if it is running    
            if ( iCompactTimer )
                {
                iCompactTimer->Cancel();
                delete iCompactTimer;
                iCompactTimer = NULL;       
                }
            }
            break;
            
        default:
            {
            OpenDbFileL();
            }
            break;                
        }

    CSELOGSTRING_HIGH_LEVEL( "<<<CCseScheduleDB::ChangeFileLockL" );
    }

// ---------------------------------------------------------------------------
// CCseScheduleDB::CloseDbFile()
//
// ---------------------------------------------------------------------------    
void CCseScheduleDB::CloseDbFile()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::CloseDbFile");
    
    iDbAccess = ECseDbLocked;
    iScheduleDb.Close();
    delete iScheduleFileStore;
    iScheduleFileStore = NULL;    
        
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::CloseDbFile");
    }
	
// ---------------------------------------------------------------------------
// CCseScheduleDB::OpenDbFileL()
//
// ---------------------------------------------------------------------------    
void CCseScheduleDB::OpenDbFileL()
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::OpenDbFileL");
    
    iScheduleDb.Close();
    if( iScheduleFileStore )
        {
        delete iScheduleFileStore;
        iScheduleFileStore = NULL;
        }
	iScheduleFileStore = CPermanentFileStore::OpenL( iFsSession, iDbFile,
												     EFileRead|EFileWrite );
	iScheduleFileStore->SetTypeL( iScheduleFileStore->Layout() );	
    iScheduleDb.OpenL( iScheduleFileStore, iScheduleFileStore->Root() );
	User::LeaveIfError( iScheduleDb.Compact() );    
	
	iDbAccess = ECseDbOpen;
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::OpenDbFileL");
    }

// ---------------------------------------------------------------------------
// CCseScheduleDB::LeaveIfDbLocked()
//
// ---------------------------------------------------------------------------    
void CCseScheduleDB::LeaveIfDbLockedL() const
    {
    CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::LeaveIfDbLocked");
    
    if(iDbAccess == ECseDbLocked)
        {
        CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::LeaveIfDbLocked - Leave, DB locked!!");
        User::Leave( KErrLocked );
        }
        
    CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::LeaveIfDbLocked");
    }

// ---------------------------------------------------------------------------
// CCseScheduleDB::GetSchedulesByPluginL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::GetSchedulesByPluginL( const TInt32 aPluginUid,
								            RPointerArray<CCseScheduledProgram>& aArray )
	{
	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::GetSchedulesByPluginL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	// Gets program occurence(s) that are going to happen next
	// from database
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseSchedulePlugInUIDCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( aPluginUid );
	sqlStatement.Append( KCseOrderBy );
	sqlStatement.Append( KCseScheduleStartTimeCol );

	// Order table based on start time
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	// Get the SQL table indexes.
	TDbColNo keyIndex = colSetOrder->ColNo( KCseScheduleDbKeyCol );
	TDbColNo nameIndex = colSetOrder->ColNo( KCseScheduleNameCol );
	TDbColNo startTimeIndex = colSetOrder->ColNo( KCseScheduleStartTimeCol );
	TDbColNo endTimeIndex = colSetOrder->ColNo( KCseScheduleEndTimeCol );
	TDbColNo applicationUidIdIndex = colSetOrder->ColNo( KCseScheduleApplicationUIDCol );
	TDbColNo plugInUidIndex = colSetOrder->ColNo( KCseSchedulePlugInUIDCol );
	TDbColNo applicationSpecificIndex = colSetOrder->ColNo( KCseScheduleApplicationSpecificCol );
	TDbColNo scheduleTypeIndex = colSetOrder->ColNo( KCseScheduleTypeCol );
	TDbColNo pluginTypeIndex = colSetOrder->ColNo( KCseSchedulePluginTypeCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );
	
	// Loop through the tables
	while ( view.AtRow() )
		{
		// Create new CCseSceduledProgram to be added to array
		CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
		CleanupStack::PushL( prog );			
		
		// Fill created schedule with DB information
		view.GetL();
		prog->SetDbIdentifier( view.ColUint32( keyIndex ) );
		prog->SetName( view.ColDes8( nameIndex ) );
		prog->SetStartTime( view.ColTime( startTimeIndex ) );
		prog->SetEndTime( view.ColTime( endTimeIndex ) );
		prog->SetAppUid( view.ColInt32( applicationUidIdIndex ) );
		prog->SetPluginUid( view.ColInt32( plugInUidIndex ) );
		prog->SetScheduleType( view.ColInt32( scheduleTypeIndex ) );
		prog->SetPluginType( view.ColInt32( pluginTypeIndex ) );
		
		TInt streamLen( 0 );
		streamLen = view.ColLength( applicationSpecificIndex );
	
		HBufC8* appDataBuffer = HBufC8::NewL( streamLen );
		
		CleanupStack::PushL( appDataBuffer );
		
		TPtr8 buffPtr( appDataBuffer->Des() );
		RDbColReadStream readStream;
		readStream.OpenLC( view, applicationSpecificIndex ); // |->1
		readStream.ReadL( buffPtr, streamLen );
		readStream.Release();
		CleanupStack::PopAndDestroy( ); // 1<-| Popping pointer to Col that we dont have here

		prog->SetApplicationDataL( *appDataBuffer );
		
		CleanupStack::PopAndDestroy( appDataBuffer );
		User::LeaveIfError( aArray.Append( prog ) );
		CleanupStack::Pop( prog );
		view.NextL();
		}
		
	CleanupStack::PopAndDestroy( &view ); // Closes view
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::GetSchedulesByPluginL");	
	}
	
// ---------------------------------------------------------------------------
// CCseScheduleDB::GetSchedulesByTypeL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::GetSchedulesByTypeL( const TInt32 aType,
								          RPointerArray<CCseScheduledProgram>& aArray )
	{	
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::GetSchedulesByPluginL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	// Gets program occurence(s) that are going to happen next
	// from database
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleTypeCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( aType );
	sqlStatement.Append( KCseOrderBy );
	sqlStatement.Append( KCseScheduleStartTimeCol );

	// Order table based on start time
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	// Get the SQL table indexes.
	TDbColNo keyIndex = colSetOrder->ColNo( KCseScheduleDbKeyCol );
	TDbColNo nameIndex = colSetOrder->ColNo( KCseScheduleNameCol );
	TDbColNo startTimeIndex = colSetOrder->ColNo( KCseScheduleStartTimeCol );
	TDbColNo endTimeIndex = colSetOrder->ColNo( KCseScheduleEndTimeCol );
	TDbColNo applicationUidIdIndex = colSetOrder->ColNo( KCseScheduleApplicationUIDCol );
	TDbColNo plugInUidIndex = colSetOrder->ColNo( KCseSchedulePlugInUIDCol );
	TDbColNo applicationSpecificIndex = colSetOrder->ColNo( KCseScheduleApplicationSpecificCol );
	TDbColNo scheduleTypeIndex = colSetOrder->ColNo( KCseScheduleTypeCol );
	TDbColNo pluginTypeIndex = colSetOrder->ColNo( KCseSchedulePluginTypeCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );
	
	// Loop through the tables
	while ( view.AtRow() )
		{
		// Create new CCseSceduledProgram to be added to array
		CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
		CleanupStack::PushL( prog );			
		
		// Fill created schedule with DB information
		view.GetL();
		prog->SetDbIdentifier( view.ColUint32( keyIndex ) );
		prog->SetName( view.ColDes8( nameIndex ) );
		prog->SetStartTime( view.ColTime( startTimeIndex ) );
		prog->SetEndTime( view.ColTime( endTimeIndex ) );
		prog->SetAppUid( view.ColInt32( applicationUidIdIndex ) );
		prog->SetPluginUid( view.ColInt32( plugInUidIndex ) );
		prog->SetScheduleType( view.ColInt32( scheduleTypeIndex ) );
		prog->SetPluginType( view.ColInt32( pluginTypeIndex ) );
		
		TInt streamLen( 0 );
		streamLen = view.ColLength( applicationSpecificIndex );
	
		HBufC8* appDataBuffer = HBufC8::NewL( streamLen );
		
		CleanupStack::PushL( appDataBuffer );
		
		TPtr8 buffPtr( appDataBuffer->Des() );
		RDbColReadStream readStream;
		readStream.OpenLC( view, applicationSpecificIndex ); // |-> 1
		readStream.ReadL( buffPtr, streamLen );
		readStream.Release();
		CleanupStack::PopAndDestroy( ); // 1 <-| Popping pointer to Col that we dont have here

		prog->SetApplicationDataL( *appDataBuffer );
		
		CleanupStack::PopAndDestroy( appDataBuffer );
		User::LeaveIfError( aArray.Append( prog ) );
		CleanupStack::Pop( prog );
		view.NextL();
		}
		
	CleanupStack::PopAndDestroy( &view ); // Closes view
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::GetSchedulesByPluginL");	
	}
	
// ---------------------------------------------------------------------------
// CCseScheduleDB::GetSchedulesByTimeframeL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::GetSchedulesByTimeframeL( const TTime& aBeginning,
                                               const TTime& aEnd,
                                               RPointerArray<CCseScheduledProgram>& aResultArray
                                               )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::GetSchedulesByTimeframeL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	TBuf<100> startTimeBuf;
	_LIT( KDateTimeFormat,"# %1%*D/%2%*M/%3%*Y %H:%T:%S#" ); 
	aBeginning.FormatL( startTimeBuf, KDateTimeFormat );
	TBuf<100> endTimeBuf;
	aEnd.FormatL( endTimeBuf, KDateTimeFormat );  

    // Create SQL string to find all overlapping schedules from database between given
    // timeframe.
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleStartTimeCol );
	sqlStatement.Append( KCseLess );
	sqlStatement.Append( endTimeBuf );
	sqlStatement.Append( KCseAnd );
	sqlStatement.Append( KCseScheduleEndTimeCol );
	sqlStatement.Append( KCseMore );
	sqlStatement.Append( startTimeBuf );	
	sqlStatement.Append( KCseOrderBy );
	sqlStatement.Append( KCseScheduleStartTimeCol );

    // Order table		
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	// Move to the first row
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	// Get colon indexes.
	TDbColNo keyIndex = colSetOrder->ColNo( KCseScheduleDbKeyCol );
	TDbColNo nameIndex = colSetOrder->ColNo( KCseScheduleNameCol );
	TDbColNo startTimeIndex = colSetOrder->ColNo( KCseScheduleStartTimeCol );
	TDbColNo endTimeIndex = colSetOrder->ColNo( KCseScheduleEndTimeCol );
	TDbColNo applicationUidIdIndex = colSetOrder->ColNo( KCseScheduleApplicationUIDCol );
	TDbColNo plugInUidIndex = colSetOrder->ColNo( KCseSchedulePlugInUIDCol );
	TDbColNo typeIndex = colSetOrder->ColNo( KCseScheduleTypeCol );
	TDbColNo applicationSpecificIndex = colSetOrder->ColNo( KCseScheduleApplicationSpecificCol );
	TDbColNo pluginTypeIndex = colSetOrder->ColNo( KCseSchedulePluginTypeCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );
	
	// Loop through all found schedules.
	while ( view.AtRow() )
		{
		// Create new schedule to be added to array.
		CCseScheduledProgram* schedule = CCseScheduledProgram::NewL();
		CleanupStack::PushL( schedule );	
		view.GetL();
		
		// Set schedule information.
		schedule->SetDbIdentifier( view.ColUint32( keyIndex ) );
		schedule->SetName( view.ColDes8( nameIndex ) );
		schedule->SetStartTime( view.ColTime( startTimeIndex ) );
		schedule->SetEndTime( view.ColTime( endTimeIndex ) );
		schedule->SetAppUid( view.ColInt32( applicationUidIdIndex ) );
		schedule->SetPluginUid( view.ColInt32( plugInUidIndex ) );
		
		schedule->SetScheduleType( CCseScheduledProgram::TCseScheduleType( 
							   view.ColInt32( typeIndex ) ) );
        schedule->SetPluginType( CCseScheduledProgram::TCseScheduleType( 
							   view.ColInt32( pluginTypeIndex ) ) );

		TInt streamLen( 0 );
		streamLen = view.ColLength( applicationSpecificIndex );
	
		HBufC8* appDataBuffer = HBufC8::NewL( streamLen );
		CleanupStack::PushL( appDataBuffer );
		
		TPtr8 buffPtr( appDataBuffer->Des() );
		RDbColReadStream readStream;
		readStream.OpenLC( view, applicationSpecificIndex ); // |-> 1
		readStream.ReadL(buffPtr, streamLen);
		readStream.Release();
		CleanupStack::PopAndDestroy( ); // 1 <-| Popping pointer to Col that we dont have here
		TPtrC8 ptr8( appDataBuffer->Des() );
		schedule->SetApplicationDataL( *appDataBuffer );
		CleanupStack::PopAndDestroy( appDataBuffer );
		
		aResultArray.AppendL( schedule );
		CleanupStack::Pop( schedule );	
	
		// Next shcedule...
		view.NextL();
		}
	CleanupStack::PopAndDestroy( &view ); // Closes view
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::GetSchedulesByTimeframeL");
	}
	
// ---------------------------------------------------------------------------
// CCseScheduleDB::GetPluginsL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::GetPluginsL( RArray<TInt32>& aPlugins )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::GetPluginsL");
	LeaveIfDbLockedL();
	ResetCompactTimer();	
	
	// Gets program occurence(s) that are going to happen next
	// from database
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseOrderBy );
	sqlStatement.Append( KCseSchedulePlugInUIDCol );
	
	// Order table based on start time
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	// Get the SQL table indexes.	
	TDbColNo pluginUidIndex = colSetOrder->ColNo( KCseSchedulePlugInUIDCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );
	
	TInt32 currentPluginUid( 0 );
	TInt32 newPluginUid( 0 );
	
	// Loop through the tables
	while ( view.AtRow() )
	    {	    
	    view.GetL();
	    
	    // Get plugin Uid from row	        
		newPluginUid = view.ColInt32( pluginUidIndex );
		
		// If it is different that the current plugin uid add it to
		// array and set new "current" uid.
		if( newPluginUid != currentPluginUid )
		    {
		    // Query results are sorted by plugin uid
		    // so if uid changes we can add it to array
		    // without fearing that we will have
		    // duplicates.		    
		    aPlugins.Append( newPluginUid );
		    currentPluginUid = newPluginUid;
		    }		
		
		view.NextL();
		}
		
    view.Close();		
	CleanupStack::PopAndDestroy( &view ); // Closes view	
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::GetPluginsL");
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::GetNextScheduleTimeByPluginL()
//
// ---------------------------------------------------------------------------	
TTime CCseScheduleDB::GetNextScheduleTimeByPluginL( TInt32 aPluginUid )
	{
	CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::GetNextScheduleTimeByPluginL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	// Create SQL satring to get schedule occurence(s) that are
	// going to happen next from database.
	TTime nextScheduleTime( 0 );
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleStateCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( ECseWaiting );
	sqlStatement.Append( KCseAnd );
	sqlStatement.Append( KCseSchedulePlugInUIDCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( aPluginUid );
	sqlStatement.Append( KCseOrderBy );
	sqlStatement.Append( KCseScheduleStartTimeCol );
		
	// Order table based on start time
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	// Get the next schedule
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	TDbColNo startTimeIndex = colSetOrder->ColNo( KCseScheduleStartTimeCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );
	
	// Schedule found get its start time.
	if ( view.AtRow() )	
		{		
		view.GetL();				
		nextScheduleTime = view.ColTime( startTimeIndex );
		}
		
	CleanupStack::PopAndDestroy( &view ); // Closes view
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::GetNextScheduleTimeByPluginL");
	
	return nextScheduleTime;
	}

// ---------------------------------------------------------------------------
// CCseScheduleDB::GetSchedulesByTimeAndPluginL()
//
// ---------------------------------------------------------------------------	
void CCseScheduleDB::GetSchedulesByTimeAndPluginL(
                                     const TTime& aScheduleTime, 
                                     TInt32 aPluginUid,
					                 RPointerArray<CCseScheduledProgram>& aNextScheduleArray )
	{
    CSELOGSTRING_HIGH_LEVEL(">>>CCseScheduleDB::GetSchedulesByTimeAndPluginL");
	LeaveIfDbLockedL();
	ResetCompactTimer();
	
	// Gets program occurence(s) that are going to happen next
	// from database
	TBuf<100> startTimeBuf;
	
	_LIT( KDateTimeFormat,"# %1%*D/%2%*M/%3%*Y %H:%T:%S.%C#" ); 

	aScheduleTime.FormatL( startTimeBuf, KDateTimeFormat );
		
	// Create SQL string to get all schedules to be happen at given time
	TBuf<KCseCustomSqlLength> sqlStatement;
	sqlStatement.Append( KCseSelectFromDb );	
	sqlStatement.Append( KCseScheduleTable );
	sqlStatement.Append( KCseWhere );
	sqlStatement.Append( KCseScheduleStartTimeCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.Append( startTimeBuf );
	sqlStatement.Append( KCseAnd );
	sqlStatement.Append( KCseSchedulePlugInUIDCol );
	sqlStatement.Append( KCseEqual );
	sqlStatement.AppendNum( aPluginUid );
		
	// Order table based on start time
	RDbView view;
	CleanupClosePushL( view );
	User::LeaveIfError( view.Prepare( iScheduleDb, 
						TDbQuery( sqlStatement ) ) );
	User::LeaveIfError( view.EvaluateAll() );
	
	// Get the next reminder
	view.FirstL();
	
	CDbColSet* colSetOrder = view.ColSetL();
	CleanupStack::PushL( colSetOrder );
	
	// Get column indexes
	TDbColNo keyIndex = colSetOrder->ColNo( KCseScheduleDbKeyCol );
	TDbColNo nameIndex = colSetOrder->ColNo( KCseScheduleNameCol );
	TDbColNo startTimeIndex = colSetOrder->ColNo( KCseScheduleStartTimeCol );
	TDbColNo endTimeIndex = colSetOrder->ColNo( KCseScheduleEndTimeCol );
	TDbColNo applicationUidIdIndex = colSetOrder->ColNo( KCseScheduleApplicationUIDCol );
	TDbColNo plugInUidIndex = colSetOrder->ColNo( KCseSchedulePlugInUIDCol );
	TDbColNo typeIndex = colSetOrder->ColNo( KCseScheduleTypeCol );
	TDbColNo applicationSpecificIndex = colSetOrder->ColNo( KCseScheduleApplicationSpecificCol );
	TDbColNo pluginTypeIndex = colSetOrder->ColNo( KCseSchedulePluginTypeCol );
	
	CleanupStack::PopAndDestroy( colSetOrder );
	
	// Loop through all found schedules.
	while ( view.AtRow() )
		{
		// Create new schedule to be added to array
		CCseScheduledProgram* prog = CCseScheduledProgram::NewL();
		CleanupStack::PushL( prog );			
		view.GetL();
		
		// Copy information from DB to schedule.
		prog->SetDbIdentifier( view.ColUint32( keyIndex ) );
		prog->SetName( view.ColDes8( nameIndex ) );
		prog->SetStartTime( view.ColTime( startTimeIndex ) );
		prog->SetEndTime( view.ColTime( endTimeIndex ) );
		prog->SetAppUid( view.ColInt32( applicationUidIdIndex ) );
		prog->SetPluginUid( view.ColInt32( plugInUidIndex ) );
		
		prog->SetScheduleType( CCseScheduledProgram::TCseScheduleType( 
							   view.ColInt32( typeIndex ) ) );
        prog->SetPluginType( view.ColInt32( pluginTypeIndex ) );
		TInt streamLen( view.ColLength( applicationSpecificIndex ) );
	
		HBufC8* appDataBuffer = HBufC8::NewL( streamLen );
		CleanupStack::PushL( appDataBuffer );
		
		TPtr8 buffPtr( appDataBuffer->Des() );
		RDbColReadStream readStream;
		readStream.OpenLC( view, applicationSpecificIndex ); // |-> 1
		readStream.ReadL( buffPtr, streamLen );
		readStream.Release();
		CleanupStack::PopAndDestroy(); // 1 <-|

		prog->SetApplicationDataL( *appDataBuffer );
		
		CleanupStack::PopAndDestroy( appDataBuffer );
		User::LeaveIfError( aNextScheduleArray.Append( prog ) );
		CleanupStack::Pop( prog );
		
		// And next schedule...
		view.NextL();
		}
		
	CleanupStack::PopAndDestroy( &view ); // Closes view
	
	CSELOGSTRING_HIGH_LEVEL("<<<CCseScheduleDB::GetSchedulesByTimeAndPluginL");
	}


// End of file.
