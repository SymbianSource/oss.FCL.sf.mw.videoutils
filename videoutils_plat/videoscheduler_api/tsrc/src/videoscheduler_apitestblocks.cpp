/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   ?Description*
*/




// [INCLUDE FILES] - do not remove
#include <e32svr.h>
#include <StifParser.h>
#include <Stiftestinterface.h>
#include "videoscheduler_apitest.h"
#include <S32MEM.H>

#include <ipvideo/CCseSchedulerAPI.h>
#include <ipvideo/CCseScheduledProgram.h>

#include "vcxtestlog.h"

// EXTERNAL DATA STRUCTURES
//extern  ?external_data;

// EXTERNAL FUNCTION PROTOTYPES
//extern ?external_function( ?arg_type,?arg_type );

// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def

// LOCAL CONSTANTS AND MACROS
//const ?type ?constant_var = ?constant;
//#define ?macro_name ?macro_def

// MODULE DATA STRUCTURES
//enum ?declaration
//typedef ?declaration

// LOCAL FUNCTION PROTOTYPES
//?type ?function_name( ?arg_type, ?arg_type );

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// ?function_name ?description.
// ?description
// Returns: ?value_1: ?description
//          ?value_n: ?description_line1
//                    ?description_line2
// -----------------------------------------------------------------------------
//
/*
?type ?function_name(
    ?arg_type arg,  // ?description
    ?arg_type arg)  // ?description
    {

    ?code  // ?comment

    // ?comment
    ?code
    }
*/

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Delete
// Delete here all resources allocated and opened from test methods.
// Called from destructor.
// -----------------------------------------------------------------------------
//
void CVideoScheduler_ApiTest::Delete()
    {
    if( iApi )
        {
        for( TInt i=0; i<iAddedProgramIds.Count(); i++ )
            {
            iApi->RemoveSchedule( iAddedProgramIds[i] );
            }
        }
    
    delete iApi;
    iApi = NULL;
    
    delete iProgram;
    iProgram = NULL;
    
    delete iExternalizeBuff;
    iExternalizeBuff = NULL;
    
    iAddedProgramIds.Close();
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::RunMethodL(
    CStifItemParser& aItem )
    {

    static TStifFunctionInfo const KFunctions[] =
        {
        // Copy this line for every implemented function.
        // First string is the function name used in TestScripter script file.
        // Second is the actual implementation member function.
        ENTRY( "API_NewL", CVideoScheduler_ApiTest::API_NewL ),
        ENTRY( "API_AddSchedule", CVideoScheduler_ApiTest::API_AddScheduleL ),
        ENTRY( "API_RemoveSchedule", CVideoScheduler_ApiTest::API_RemoveScheduleL ),
        ENTRY( "RemoveAllSchedules", CVideoScheduler_ApiTest::RemoveAllSchedulesL ),
        ENTRY( "API_GetSchedule", CVideoScheduler_ApiTest::API_GetScheduleL ),
        ENTRY( "API_GetSchedulesByAppUid", CVideoScheduler_ApiTest::API_GetSchedulesByAppUidL ),
        ENTRY( "API_GetOverlappingSchedules", CVideoScheduler_ApiTest::API_GetOverlappingSchedulesL ),
        ENTRY( "API_GetSchedulesByPluginUid", CVideoScheduler_ApiTest::API_GetSchedulesByPluginUidL ),
        ENTRY( "API_GetSchedulesByType", CVideoScheduler_ApiTest::API_GetSchedulesByTypeL ),
        ENTRY( "API_GetSchedulesByTime", CVideoScheduler_ApiTest::API_GetSchedulesByTimeL ),

        ENTRY( "Program_NewL", CVideoScheduler_ApiTest::Program_NewL ),
        ENTRY( "Program_NewLFromProgram", CVideoScheduler_ApiTest::Program_NewLFromProgramL ),
        ENTRY( "Program_Externalize", CVideoScheduler_ApiTest::Program_ExternalizeLL ),
        ENTRY( "Program_Internalize", CVideoScheduler_ApiTest::Program_InternalizeLL ),
        ENTRY( "Program_ExternalizeLength", CVideoScheduler_ApiTest::Program_ExternalizeLengthL ),
        ENTRY( "Program_SetPluginType", CVideoScheduler_ApiTest::Program_SetPluginTypeL ),
        ENTRY( "Program_SetName", CVideoScheduler_ApiTest::Program_SetNameL ),
        ENTRY( "Program_SetStartTime", CVideoScheduler_ApiTest::Program_SetStartTimeL ),
        ENTRY( "Program_SetEndTime", CVideoScheduler_ApiTest::Program_SetEndTimeL ),
        ENTRY( "Program_SetAppUid", CVideoScheduler_ApiTest::Program_SetAppUidL ),
        ENTRY( "Program_SetPluginUid", CVideoScheduler_ApiTest::Program_SetPluginUidL ),
        ENTRY( "Program_SetApplicationData", CVideoScheduler_ApiTest::Program_SetApplicationDataLL ),
        ENTRY( "Program_SetDbIdentifier", CVideoScheduler_ApiTest::Program_SetDbIdentifierL ),
        ENTRY( "Program_SetScheduleType", CVideoScheduler_ApiTest::Program_SetScheduleTypeL ),
        ENTRY( "Program_PluginType", CVideoScheduler_ApiTest::Program_PluginTypeL ),
        ENTRY( "Program_Name", CVideoScheduler_ApiTest::Program_NameL ),
        ENTRY( "Program_StartTime", CVideoScheduler_ApiTest::Program_StartTimeL ),
        ENTRY( "Program_EndTime", CVideoScheduler_ApiTest::Program_EndTimeL ),
        ENTRY( "Program_AppUid", CVideoScheduler_ApiTest::Program_AppUidL ),
        ENTRY( "Program_PluginUid", CVideoScheduler_ApiTest::Program_PluginUidL ),
        ENTRY( "Program_ApplicationData", CVideoScheduler_ApiTest::Program_ApplicationDataL ),
        ENTRY( "Program_DbIdentifier", CVideoScheduler_ApiTest::Program_DbIdentifierL ),
        ENTRY( "Program_ScheduleType", CVideoScheduler_ApiTest::Program_ScheduleTypeL ),
        ENTRY( "Program_DebugDump", CVideoScheduler_ApiTest::Program_DebugDumpL ),

        ENTRY( "Program_SetToFuture", CVideoScheduler_ApiTest::Program_SetToFutureL ),
        // [test cases entries] - Do not remove

        };

    const TInt count = sizeof( KFunctions ) /
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::LogMethod
// Generate log from current method
// -----------------------------------------------------------------------------
//
void CVideoScheduler_ApiTest::LogMethod( TPtrC aMethod )
    {
    // Print to UI
    TBuf< 64 > buffer;
    buffer.Format( KLogLocation, &aMethod );
    TestModuleIf().Printf( 0, KTestModuleName, buffer );
    // Print to log file
    iLog->Log( buffer );
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::API_NewL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::API_NewL( CStifItemParser& aItem )
    {
    LogMethod(_L("API_NewL"));

    TInt err( KErrNone );
    
    iApi = CCseSchedulerApi::NewL();
    
    TInt createProgram( 1 );
    if( aItem.GetNextInt( createProgram ) != KErrNone )
        {
        createProgram = 1;
        }
    
    if( createProgram == 1 )
        {
        iProgram = CCseScheduledProgram::NewL();

        RPointerArray<CCseScheduledProgram> programs;    
        err = iApi->GetSchedulesByAppUid( 0x020202, programs );
        if( err == KErrNone )
            {
            for( TInt i=0; i < programs.Count(); i++ )
                {
                iApi->RemoveSchedule( programs[i]->DbIdentifier() );
                }
            }
        programs.ResetAndDestroy();
        }
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::API_AddScheduleL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::API_AddScheduleL( CStifItemParser& /*aItem*/ )
    {
    LogMethod(_L("API_AddSchedule"));
    VCXLOGLO1("CVideoScheduler_ApiTest::API_AddScheduleL ------>");
    
    TInt err( KErrNone );

    err = iApi->AddSchedule( *iProgram );
    VCXLOGLO2("CVideoScheduler_ApiTest::API_AddScheduleL: DBID after: %d", iProgram->DbIdentifier() );
    iAddedProgramIds.Append( iProgram->DbIdentifier() );
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::API_RemoveScheduleL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::API_RemoveScheduleL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::API_RemoveScheduleL ------>");
    LogMethod(_L("API_RemoveScheduleL"));

    TInt err( KErrNone );
    
    VCXLOGLO2("CVideoScheduler_ApiTest::API_RemoveScheduleL: DBID: %d", iProgram->DbIdentifier() );
    
    TUint32 id = iProgram->DbIdentifier();
    err = iApi->RemoveSchedule( id );
    
    for( TInt i=0; i<iAddedProgramIds.Count(); i++ )
        {
        if( iAddedProgramIds[i] == id )
            {
            iAddedProgramIds.Remove( i );
            break;
            }
        }    

    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::RemoveAllSchedulesL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::RemoveAllSchedulesL( CStifItemParser& /*aItem*/ )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::RemoveAllSchedulesL ------>");
    LogMethod(_L("RemoveAllSchedulesL"));

    TInt err( KErrNone );
    
    while( iAddedProgramIds.Count() )
        {
        VCXLOGLO2("CVideoScheduler_ApiTest::RemoveAllSchedulesL -- remove ID %d", iAddedProgramIds[0]);
        err = iApi->RemoveSchedule( iAddedProgramIds[0] );

        iAddedProgramIds.Remove( 0 );
        
        if( err != KErrNone )
            {
            VCXLOGLO2("CVideoScheduler_ApiTest::RemoveAllSchedulesL -- Failed: %d", err);
            }
        }
    
    VCXLOGLO2("CVideoScheduler_ApiTest::RemoveAllSchedulesL <------ %d", err);
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::API_GetScheduleL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::API_GetScheduleL( CStifItemParser& /*aItem*/ )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::API_GetScheduleL ------>");
    LogMethod(_L("API_GetScheduleL"));

    TInt err( KErrNone );

    CCseScheduledProgram* program = CCseScheduledProgram::NewL();
    CleanupStack::PushL( program );
    
    err = iApi->GetSchedule( iProgram->DbIdentifier(), program );
    
    if( err == KErrNone )
        {
        if( !program )
            {
            VCXLOGLO1("CVideoScheduler_ApiTest::API_GetScheduleL: Error, program is NULL.");
            err = KErrCorrupt;
            }
        else
            {
            err = ComparePrograms( *program, *iProgram );
            }
        }
    
    CleanupStack::PopAndDestroy( program );
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::API_GetSchedulesByAppUidL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::API_GetSchedulesByAppUidL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::API_GetSchedulesByAppUidL ------>");
    LogMethod(_L("API_GetSchedulesByAppUidL"));

    TInt err( KErrNone );
    
    TInt appUid( 0 );
    User::LeaveIfError( aItem.GetNextInt( appUid ) );
    
    TInt expectedCount(0);
    User::LeaveIfError( aItem.GetNextInt( expectedCount ) );

    RPointerArray<CCseScheduledProgram> programs;    
    
    err = iApi->GetSchedulesByAppUid( appUid, programs );

    if( err == KErrNone )
        {
        if( programs.Count() != expectedCount )
            {
            err = KErrCorrupt;
            }
        else 
        if( programs.Count() > 0 )
            {
            delete iProgram;
            iProgram = NULL;
            
            iProgram = programs[0];
            programs.Remove( 0 );
            }
        }

    programs.ResetAndDestroy();
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::API_GetOverlappingSchedulesL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::API_GetOverlappingSchedulesL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::API_GetOverlappingSchedulesL ------>");
    LogMethod(_L("API_GetOverlappingSchedulesL"));

    TInt err( KErrNone );

    TInt expectedCount(0);
    User::LeaveIfError( aItem.GetNextInt( expectedCount ) );
    
    RPointerArray<CCseScheduledProgram> programs;
    
    err = iApi->GetOverlappingSchedules( *iProgram, programs );

    if( err == KErrNone )
        {
        if( programs.Count() != expectedCount )
            {
            err = KErrCorrupt;
            }
        else 
        if( programs.Count() > 0 )
            {
            delete iProgram;
            iProgram = NULL;
            
            iProgram = programs[0];
            programs.Remove( 0 );
            }
        }
    
    programs.ResetAndDestroy();
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::API_GetSchedulesByPluginUidL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::API_GetSchedulesByPluginUidL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::API_GetSchedulesByPluginUidL ------>");
    LogMethod(_L("API_GetSchedulesByPluginUidL"));

    TInt err( KErrNone );

    TInt pluginUid( 0 );
    User::LeaveIfError( aItem.GetNextInt( pluginUid ) ); 
    
    TInt expectedCount(0);
    User::LeaveIfError( aItem.GetNextInt( expectedCount ) );
    
    RPointerArray<CCseScheduledProgram> programs;
    
    err = iApi->GetSchedulesByPluginUid( pluginUid, programs );

    if( err == KErrNone )
        {
        if( programs.Count() != expectedCount )
            {
            err = KErrCorrupt;
            }
        else 
        if( programs.Count() > 0 )
            {
            delete iProgram;
            iProgram = NULL;
            
            iProgram = programs[0];
            programs.Remove( 0 );
            }
        }    
    
    programs.ResetAndDestroy();
    
    return err;
    }


// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::API_GetSchedulesByTypeL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::API_GetSchedulesByTypeL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::API_GetSchedulesByTypeL ------>");
    LogMethod(_L("API_GetSchedulesByTypeL"));

    TInt err( KErrNone );

    TInt type(0);
    User::LeaveIfError( aItem.GetNextInt( type ) );

    TInt expectedCount(0);
    User::LeaveIfError( aItem.GetNextInt( expectedCount ) );
    
    RPointerArray<CCseScheduledProgram> programs;
    
    err = iApi->GetSchedulesByType( type, programs );

    if( err == KErrNone )
        {
        VCXLOGLO2("CVideoScheduler_ApiTest::API_GetSchedulesByTypeL count:    %d", programs.Count());
        VCXLOGLO2("CVideoScheduler_ApiTest::API_GetSchedulesByTypeL expected: %d", expectedCount);
        if( programs.Count() != expectedCount )
            {
            err = KErrCorrupt;
            }
        else 
        if( programs.Count() > 0 )
            {
            delete iProgram;
            iProgram = NULL;
            
            iProgram = programs[0];
            programs.Remove( 0 );
            }        
        }
    
    programs.ResetAndDestroy();
    
    return err;
    }


// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::API_GetSchedulesByTimeL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::API_GetSchedulesByTimeL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::API_GetSchedulesByTimeL ------>");
    LogMethod(_L("API_GetSchedulesByTimeL"));

    TInt err( KErrNone );
    TInt intervalStart( 0 );
    TInt intervalEnd( 0 );
    
    User::LeaveIfError( aItem.GetNextInt( intervalStart ) );
    User::LeaveIfError( aItem.GetNextInt( intervalEnd ) );
    
    TTime startTime( 0 );
    TTime endTime( 0 );
    
    startTime.UniversalTime();
    endTime.UniversalTime();
    
    startTime += TTimeIntervalSeconds( intervalStart );
    endTime += TTimeIntervalSeconds( intervalEnd );
    
    TInt expectedCount(0);
    User::LeaveIfError( aItem.GetNextInt( expectedCount ) );
    
    RPointerArray<CCseScheduledProgram> programs;
    
    err = iApi->GetSchedulesByTime( startTime, endTime, programs );

    if( err == KErrNone )
        {
        if( programs.Count() != expectedCount )
            {
            err = KErrCorrupt;
            }
        else 
        if( programs.Count() > 0 )
            {
            delete iProgram;
            iProgram = NULL;
            
            iProgram = programs[0];
            programs.Remove( 0 );
            }  
        }    
    
    programs.ResetAndDestroy();
    
    return err;
    }


// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_NewL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_NewL( CStifItemParser& /*aItem*/ )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_NewL ------>");
    LogMethod(_L("Program_NewL"));

    TInt err( KErrNone );
    
    iProgram = CCseScheduledProgram::NewL();
    iProgram->SetAppUid( 0x020202 );

    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_NewLFromProgramL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_NewLFromProgramL( CStifItemParser& /*aItem*/ )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_NewLFromProgramL ------>");
    LogMethod(_L("Program_NewLFromProgramL"));

    TInt err( KErrNone );
    
    CCseScheduledProgram* program = iProgram;
    iProgram = NULL;
    
    CleanupStack::PushL( program );
    iProgram = CCseScheduledProgram::NewL( *program );    
    CleanupStack::PopAndDestroy( program );
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_ExternalizeLL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_ExternalizeLL( CStifItemParser& /*aItem*/ )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_ExternalizeLL ------>");
    LogMethod(_L("Program_ExternalizeLL"));

    TInt err( KErrNone );
    
    TUint32 dataSize = iProgram->ExternalizeLength();
    
    delete iExternalizeBuff;
    iExternalizeBuff = NULL;
    iExternalizeBuff = HBufC8::NewL( dataSize );
    TPtr8 buffPtr( iExternalizeBuff->Des() );
    
    RDesWriteStream stream;
    stream.Open( buffPtr );
    iProgram->ExternalizeL( stream );
    stream.CommitL();
    stream.Close();

    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_InternalizeLL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_InternalizeLL( CStifItemParser& /*aItem*/ )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_InternalizeLL ------>");
    LogMethod(_L("Program_InternalizeLL"));

    TInt err( KErrNone );
    
    CCseScheduledProgram* program = CCseScheduledProgram::NewL();
    CleanupStack::PushL( program );
    
    RDesReadStream stream;
    TPtr8 buffPtr( iExternalizeBuff->Des() );
    stream.Open( buffPtr );
    program->InternalizeL( stream );
    stream.Close();
    
    err = ComparePrograms( *program, *iProgram );
    
    CleanupStack::PopAndDestroy( program );

    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_ExternalizeLengthL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_ExternalizeLengthL( CStifItemParser& /*aItem*/ )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_ExternalizeLengthL ------>");
    LogMethod(_L("Program_ExternalizeLengthL"));

    TInt err( KErrNone );
    
    iProgram->ExternalizeLength();

    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_SetPluginTypeL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_SetPluginTypeL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_SetPluginTypeL ------>");
    LogMethod(_L("Program_SetPluginTypeL"));

    TInt err( KErrNone );
    
    TInt pluginType;
    User::LeaveIfError( aItem.GetNextInt( pluginType ) );
    
    iProgram->SetPluginType( pluginType );

    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_SetNameL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_SetNameL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_SetNameL ------>");
    LogMethod(_L("Program_SetNameL"));

    TInt err( KErrNone );
    
    aItem.SetParsingType( CStifItemParser::EQuoteStyleParsing );
    
    TPtrC name;
    User::LeaveIfError( aItem.GetNextString( name ) );
    
    TBuf8<1024> name8;
    name8.Copy( name );
    
    iProgram->SetName( name8 );

    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_SetStartTimeL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_SetStartTimeL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_SetStartTimeL ------>");
    LogMethod(_L("Program_SetStartTimeL"));

    TInt err( KErrNone );
    
    aItem.SetParsingType( CStifItemParser::EQuoteStyleParsing );
    
    TPtrC dayString;
    User::LeaveIfError( aItem.GetNextString( dayString ) );

    TTime time( dayString );
    
    iProgram->SetStartTime( time );
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_SetEndTimeL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_SetEndTimeL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_SetEndTimeL ------>");
    LogMethod(_L("Program_SetEndTimeL"));

    TInt err( KErrNone );
    
    aItem.SetParsingType( CStifItemParser::EQuoteStyleParsing );
    
    TPtrC dayString;
    User::LeaveIfError( aItem.GetNextString( dayString ) );

    TTime time( dayString );
    
    iProgram->SetEndTime( time );    

    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_SetAppUidL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_SetAppUidL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_SetAppUidL ------>");
    LogMethod(_L("Program_SetAppUidL"));

    TInt err( KErrNone );
    
    TInt uid( 0 );
    User::LeaveIfError( aItem.GetNextInt( uid ) );
    
    iProgram->SetAppUid( uid );

    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_SetPluginUidL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_SetPluginUidL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_SetPluginUidL ------>");
    LogMethod(_L("Program_SetPluginUidL"));

    TInt err( KErrNone );
    
    TInt uid( 0 );
    User::LeaveIfError( aItem.GetNextInt( uid ) );
    
    iProgram->SetPluginUid( uid );
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_SetApplicationDataLL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_SetApplicationDataLL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_SetApplicationDataLL ------>");
    LogMethod(_L("Program_SetApplicationDataLL"));

    TInt err( KErrNone );
    
    aItem.SetParsingType( CStifItemParser::EQuoteStyleParsing );
    
    TPtrC data;
    User::LeaveIfError( aItem.GetNextString( data ) );
    
    TBuf8<1024> data8;
    data8.Copy( data );
    
    iProgram->SetApplicationDataL( data8 );
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_SetDbIdentifierL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_SetDbIdentifierL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_SetDbIdentifierL ------>");
    LogMethod(_L("Program_SetDbIdentifierL"));

    TInt err( KErrNone );
    
    TInt id;
    User::LeaveIfError( aItem.GetNextInt( id ) );
    
    iProgram->SetDbIdentifier( id );

    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_SetScheduleTypeL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_SetScheduleTypeL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_SetScheduleTypeL ------>");
    LogMethod(_L("Program_SetScheduleTypeL"));

    TInt err( KErrNone );

    TInt type;
    User::LeaveIfError( aItem.GetNextInt( type ) );
    
    iProgram->SetScheduleType( type );
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_PluginTypeL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_PluginTypeL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_PluginTypeL ------>");
    LogMethod(_L("Program_PluginTypeL"));

    TInt err( KErrNone );
    
    TInt pluginType;
    TInt expectedPluginType;
    User::LeaveIfError( aItem.GetNextInt( expectedPluginType ) );
    
    pluginType = iProgram->PluginType();
    
    if( pluginType != expectedPluginType )
        {
        err = KErrCorrupt;
        }

    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_NameL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_NameL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_NameL ------>");
    LogMethod(_L("Program_NameL"));

    TInt err( KErrNone );
    
    aItem.SetParsingType( CStifItemParser::EQuoteStyleParsing );

    TPtrC expectedName;
    User::LeaveIfError( aItem.GetNextString( expectedName ) );
    
    const TPtrC8 name8 = iProgram->Name(); 
    
    TBuf<1024> name;
    name.Copy( name8 );
    
    if( name != expectedName )
        {
        err = KErrCorrupt;
        }
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_StartTimeL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_StartTimeL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_StartTimeL ------>");
    LogMethod(_L("Program_StartTimeL"));

    TInt err( KErrNone );

    aItem.SetParsingType( CStifItemParser::EQuoteStyleParsing );
    
    TPtrC dayString;
    User::LeaveIfError( aItem.GetNextString( dayString ) );

    TTime expectedTime( dayString );
    
    TTime time = iProgram->StartTime();
    
    if( time != expectedTime )
        {
        err = KErrCorrupt;
        }
        
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_EndTimeL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_EndTimeL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_EndTimeL ------>");
    LogMethod(_L("Program_EndTimeL"));

    TInt err( KErrNone );
    
    aItem.SetParsingType( CStifItemParser::EQuoteStyleParsing );
    
    TPtrC dayString;
    User::LeaveIfError( aItem.GetNextString( dayString ) );

    TTime expectedTime( dayString );
    
    TTime time = iProgram->EndTime();
    
    if( time != expectedTime )
        {
        err = KErrCorrupt;
        }
    
    return err;
    }


// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_SetToFutureL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_SetToFutureL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_SetToFutureL ------>");
    LogMethod(_L("Program_SetToFutureL"));

    TInt err( KErrNone );
    TInt intervalStart( 0 );
    TInt intervalEnd( 0 );
    
    User::LeaveIfError( aItem.GetNextInt( intervalStart ) );
    User::LeaveIfError( aItem.GetNextInt( intervalEnd ) );
    
    TTime startTime( 0 );
    TTime endTime( 0 );
    
    startTime.UniversalTime();
    endTime.UniversalTime();
    
    startTime += TTimeIntervalSeconds( intervalStart );
    endTime += TTimeIntervalSeconds( intervalEnd );
    
    iProgram->SetStartTime( startTime );
    iProgram->SetEndTime( endTime );
    
    VCXLOGLO3("CVideoScheduler_ApiTest::Program_SetToFutureL -- start %d end %d", startTime, endTime);

    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_AppUidL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_AppUidL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_AppUidL ------>");
    LogMethod(_L("Program_AppUidL"));

    TInt err( KErrNone );

    TInt expectedUid;
    User::LeaveIfError( aItem.GetNextInt( expectedUid ) );    
    
    TInt32 uid = iProgram->AppUid();
    
    if( uid != expectedUid )
        {
        err = KErrCorrupt;
        }
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_PluginUidL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_PluginUidL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_PluginUidL ------>");
    LogMethod(_L("Program_PluginUidL"));

    TInt err( KErrNone );

    TInt expectedUid;
    User::LeaveIfError( aItem.GetNextInt( expectedUid ) );    
    
    TInt32 uid = iProgram->PluginUid();
    
    if( uid != expectedUid )
        {
        err = KErrCorrupt;
        }
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_ApplicationDataL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_ApplicationDataL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_ApplicationDataL ------>");
    LogMethod(_L("Program_ApplicationDataL"));

    TInt err( KErrNone );

    aItem.SetParsingType( CStifItemParser::EQuoteStyleParsing );
    
    TPtrC expectedData;
    User::LeaveIfError( aItem.GetNextString( expectedData ) );
    
    const TPtrC8 data8 = iProgram->ApplicationData();
    
    TBuf<1024> data;
    data.Copy( data8 );
    
    if( data != expectedData )
        {
        err = KErrCorrupt;
        }
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_DbIdentifierL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_DbIdentifierL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_DbIdentifierL ------>");
    LogMethod(_L("Program_DbIdentifierL"));

    TInt err( KErrNone );
    
    TInt expectedId;
    User::LeaveIfError( aItem.GetNextInt( expectedId ) );    
   
    TUint32 id = iProgram->DbIdentifier();
   
    if( id != expectedId )
        {
        err = KErrCorrupt;
        }
   
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_ScheduleTypeL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_ScheduleTypeL( CStifItemParser& aItem )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::API_AddScheduleL ------>");
    LogMethod(_L("Program_ScheduleTypeL"));

    TInt err( KErrNone );

    TInt expectedType;
    User::LeaveIfError( aItem.GetNextInt( expectedType ) );    
   
    TInt32 type = iProgram->ScheduleType();
   
    if( type != expectedType )
        {
        err = KErrCorrupt;
        }
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::Program_DebugDumpL
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::Program_DebugDumpL( CStifItemParser& /*aItem*/ )
    {
    VCXLOGLO1("CVideoScheduler_ApiTest::Program_DebugDumpL ------>");
    LogMethod(_L("Program_DebugDumpL"));

    TInt err( KErrNone );

    iProgram->DebugDump();
    
    return err;
    }

// -----------------------------------------------------------------------------
// CVideoScheduler_ApiTest::ComparePrograms
// -----------------------------------------------------------------------------
//
TInt CVideoScheduler_ApiTest::ComparePrograms( CCseScheduledProgram& aProgram1, CCseScheduledProgram& aProgram2 )
    {
    TInt err( KErrNone );
    
    if( aProgram1.DbIdentifier() != aProgram2.DbIdentifier() )
        {
        VCXLOGLO3("CVideoScheduler_ApiTest::ComparePrograms: Error, db identifiers differ: %d vs. %d.", aProgram1.DbIdentifier(), aProgram2.DbIdentifier() );
        err = KErrCorrupt;
        }

    return err;
    }
// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  [End of File] - Do not remove
