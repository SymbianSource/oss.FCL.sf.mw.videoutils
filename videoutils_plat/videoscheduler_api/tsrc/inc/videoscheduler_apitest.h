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




#ifndef VIDEOSCHEDULER_APITEST_H
#define VIDEOSCHEDULER_APITEST_H

//  INCLUDES
#include <StifLogger.h>
#include <TestScripterInternal.h>
#include <StifTestModule.h>
#include <TestclassAssert.h>

// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def
#define TEST_CLASS_VERSION_MAJOR 0
#define TEST_CLASS_VERSION_MINOR 0
#define TEST_CLASS_VERSION_BUILD 0

// Logging path
_LIT( KVideoScheduler_ApiTestLogPath, "\\logs\\testframework\\VideoScheduler_ApiTest\\" );
// Log file
_LIT( KVideoScheduler_ApiTestLogFile, "VideoScheduler_ApiTest.txt" );
_LIT( KVideoScheduler_ApiTestLogFileWithTitle, "VideoScheduler_ApiTest_[%S].txt" );

_LIT( KTestModuleName, "VideoScheduler_ApiTest" );
_LIT( KLogLocation, "In %S" );

// FUNCTION PROTOTYPES
//?type ?function_name(?arg_list);

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;
class CVideoScheduler_ApiTest;
class CCseScheduledProgram;
class CCseSchedulerApi;

// DATA TYPES
//enum ?declaration
//typedef ?declaration
//extern ?data_type;

// CLASS DECLARATION

/**
*  CVideoScheduler_ApiTest test class for STIF Test Framework TestScripter.
*  ?other_description_lines
*
*  @lib ?library
*  @since ?Series60_version
*/
NONSHARABLE_CLASS(CVideoScheduler_ApiTest) : public CScriptBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CVideoScheduler_ApiTest* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~CVideoScheduler_ApiTest();

    public: // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );
        
        /**
         * CompareProgramsL 
         * @since ?Series60_version
         * @param aProgram1
         * @param aProgram2 
         * @return Symbian OS error code.
         */
        TInt ComparePrograms( CCseScheduledProgram& aProgram1, CCseScheduledProgram& aProgram2 );

        /**
        * LogMethod
        * @since ?Series60_version
        * @param aMethod Name of the method where this function is called
        * @return Nothing
        */        
        void LogMethod( TPtrC aMethod );

    public: // Functions from base classes

        /**
        * From CScriptBase Runs a script line.
        * @since ?Series60_version
        * @param aItem Script line containing method name and parameters
        * @return Symbian OS error code
        */
        virtual TInt RunMethodL( CStifItemParser& aItem );

    protected:  // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    protected:  // Functions from base classes

        /**
        * From ?base_class ?member_description
        */
        //?type ?member_function();

    private:

        /**
        * C++ default constructor.
        */
        CVideoScheduler_ApiTest( CTestModuleIf& aTestModuleIf );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        
        // Prohibit copy constructor if not deriving from CBase.
        // ?classname( const ?classname& );
        // Prohibit assigment operator if not deriving from CBase.
        // ?classname& operator=( const ?classname& );

        /**
        * Frees all resources allocated from test methods.
        * @since ?Series60_version
        */
        void Delete();

        /**
        * Test methods are listed below.
        *
        * Methods starting with API_ test CCseSchedulerAPI class.
        *
        * Methods starting with Program_ test CCseScheduledProgram class. Test class has member
        * instance of this class and it's used when CCseSchedulerAPI methods need the program data.
        *
        * Default app uid for programs is 0x020202, all these will be removed 
        * when API_NewL is called without parameter 0. 
        */

        /**
        * API_NewL test method.
        * Creates also CCseScheduledProgram if parameter is not 0.  
        * 
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt API_NewL( CStifItemParser& aItem );
        
        /**
        * API_AddSchedule test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt API_AddScheduleL( CStifItemParser& aItem );

        /**
        * API_RemoveSchedule test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt API_RemoveScheduleL( CStifItemParser& aItem );

        /**
        * API_RemoveSchedule test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt RemoveAllSchedulesL( CStifItemParser& aItem );
        
        /**
        * API_GetSchedule test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt API_GetScheduleL( CStifItemParser& aItem );

        /**
        * API_GetSchedulesByAppUid test method.
        * First schedule is copied to member variable.
        * 
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt API_GetSchedulesByAppUidL( CStifItemParser& aItem );

        /**
        * API_GetOverlappingSchedules test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt API_GetOverlappingSchedulesL( CStifItemParser& aItem );

        /**
        * API_GetSchedulesByPluginUidL test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt API_GetSchedulesByPluginUidL( CStifItemParser& aItem );
        
        /**
        * API_GetSchedulesByTypeL test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt API_GetSchedulesByTypeL( CStifItemParser& aItem );
        
        /**
        * API_GetSchedulesByTimeL test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt API_GetSchedulesByTimeL( CStifItemParser& aItem );
        
        /**
        * Program_NewL test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_NewL( CStifItemParser& aItem );

        /**
        * Program_NewLFromProgramL test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_NewLFromProgramL( CStifItemParser& aItem );
        
        /**
        * Program_ExternalizeL test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_ExternalizeLL( CStifItemParser& aItem );

        /**
        * Program_InternalizeL test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_InternalizeLL( CStifItemParser& aItem );

        /**
        * Program_ExternalizeLength test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_ExternalizeLengthL( CStifItemParser& aItem );

        /**
        * Program_SetPluginType test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_SetPluginTypeL( CStifItemParser& aItem );

        /**
        * Program_SetName test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_SetNameL( CStifItemParser& aItem );

        /**
        * Program_SetStartTime test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_SetStartTimeL( CStifItemParser& aItem );

        /**
        * Program_SetEndTime test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_SetEndTimeL( CStifItemParser& aItem );

        /**
        * Program_SetAppUid test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_SetAppUidL( CStifItemParser& aItem );

        /**
        * Program_SetPluginUid test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_SetPluginUidL( CStifItemParser& aItem );

        /**
        * Program_SetApplicationDataL test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_SetApplicationDataLL( CStifItemParser& aItem );

        /**
        * Program_SetDbIdentifier test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_SetDbIdentifierL( CStifItemParser& aItem );

        /**
        * Program_SetScheduleType test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_SetScheduleTypeL( CStifItemParser& aItem );

        /**
        * Program_PluginType test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_PluginTypeL( CStifItemParser& aItem );

        /**
        * Program_Name test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_NameL( CStifItemParser& aItem );

        /**
        * Program_StartTime test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_StartTimeL( CStifItemParser& aItem );

        /**
        * Program_EndTime test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_EndTimeL( CStifItemParser& aItem );

        /**
        * Program_EndTime test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_SetToFutureL( CStifItemParser& aItem );
        
        /**
        * Program_AppUid test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_AppUidL( CStifItemParser& aItem );

        /**
        * Program_PluginUid test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_PluginUidL( CStifItemParser& aItem );

        /**
        * Program_ApplicationData test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_ApplicationDataL( CStifItemParser& aItem );

        /**
        * Program_DbIdentifier test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_DbIdentifierL( CStifItemParser& aItem );

        /**
        * Program_ScheduleType test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_ScheduleTypeL( CStifItemParser& aItem );

        /**
        * Program_DebugDump test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt Program_DebugDumpL( CStifItemParser& aItem );

        /**
         * Method used to log version of test class
         */
        void SendTestClassVersion();

        //ADD NEW METHOD DEC HERE
        //[TestMethods] - Do not remove

    public:     // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    protected:  // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    private:    // Data

        // ?one_line_short_description_of_data
        //?data_declaration;

        // Reserved pointer for future extension
        //TAny* iReserved;

    public:     // Friend classes
        //?friend_class_declaration;
    protected:  // Friend classes
        //?friend_class_declaration;
    private:    // Friend classes
        //?friend_class_declaration;

        CCseSchedulerApi* iApi;
        CCseScheduledProgram* iProgram;
        HBufC8* iExternalizeBuff;
        RArray<TUint32> iAddedProgramIds;
    };

#endif      // VIDEOSCHEDULER_APITEST_H

// End of File
