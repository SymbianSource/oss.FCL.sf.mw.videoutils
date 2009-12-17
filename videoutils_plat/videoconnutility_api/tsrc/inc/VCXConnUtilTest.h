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


#ifndef VCXCONNUTILTEST_H
#define VCXCONNUTILTEST_H

//  INCLUDES
#include <StifLogger.h>
#include <TestScripterInternal.h>
#include <StifTestModule.h>

#include "MIptvTestTimerObserver.h"

#include "vcxconnutilengineobserver.h"
#include "VCXConnUtilTestPSObserver.h"
#include "../../../../videoconnutility/inc/vcxconnectionutility.hrh"
#include "../../../../videoconnutility/inc/vcxconnutilcommon.h"

// CONSTANTS
const TInt KVcxConnUtilTestModeSubscriber   = 0;
const TInt KVcxConnUtilTestModeConnectivity = 1;
const TInt KVcxConnUtilTestModeBoth         = 2; // Default mode, P&S is subscribed and connection creation is allowed

const TInt KTimeoutTimerId = 0;

// MACROS

// Logging path
_LIT( KVCXConnUtilTestLogPath, "\\logs\\testframework\\VCXConnUtilTest\\" );
// Log file
_LIT( KVCXConnUtilTestLogFile, "VCXConnUtilTest.txt" );
_LIT( KVCXConnUtilTestLogFileWithTitle, "VCXConnUtilTest_[%S].txt" );

// FUNCTION PROTOTYPES
//?type ?function_name(?arg_list);

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;
class CVCXConnUtilTest;
class CVCXTestCommon;
class CVCXConnUtilTestSubscriber;
class CIptvTestTimer;
class CIptvTestActiveWait;

// DATA TYPES
//enum ?declaration
//typedef ?declaration
//extern ?data_type;

// CLASS DECLARATION

class TVcxConnTestPSProperty
    {
    public:
        TVcxConnTestPSProperty( )
            {
            iIsInteger = EFalse;
            }
        TVcxConnTestPSProperty( TInt aProperty, TInt aValue )
            {
            iProperty = aProperty;
            iIntegerValue = aValue;
            iIsInteger = ETrue;
            }
        TVcxConnTestPSProperty( TInt aProperty, TDesC& aValue )
            {
            iProperty = aProperty;
            iStringValue = aValue;
            iIsInteger = EFalse;
            }
        ~TVcxConnTestPSProperty( )
            {

            }
    public:
        TBool iIsInteger;
        TInt iProperty;
        TInt iIntegerValue;
        TBufC<256> iStringValue;
    };

/**
*  CVCXConnUtilTest test class for STIF Test Framework TestScripter.
*/
NONSHARABLE_CLASS(CVCXConnUtilTest) : public CScriptBase,
                                      public MVCXConnUtilTestPSObserver,
                                      public MIptvTestTimerObserver
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CVCXConnUtilTest* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~CVCXConnUtilTest();

    public: // New functions

    public: // Functions from base classes

        /**
        * From CScriptBase Runs a script line.
        * @since ?Series60_version
        * @param aItem Script line containing method name and parameters
        * @return Symbian OS error code
        */
        virtual TInt RunMethodL( CStifItemParser& aItem );

        /**
         * From MVCXConnUtilTestPSObserver, called when P&S key changes.
         */
        void ValueChangedL( const TUid& aUid, const TUint32& aKey, const TInt&  aValue );

        /**
         * From MVCXConnUtilTestPSObserver, called when P&S key changes.
         */
        void ValueChangedL( const TUid& aUid, const TUint32& aKey, const TDesC& aValue );

        /**
        * From MIptvTestTimerObserver Handles timer completion
        * @since
        * @param aTimerId
        * @param aError
        */
        void TimerComplete(TInt aTimerId, TInt aError);

    private: // New functions

        /**
         * Gets name string for property.
         */
        void GetPropertyNameL( TInt aProperty, TDes& aPropertyName );

        /**
         * Gets connection name into a string.
         */
        void GetConnectionStatusL( TInt aConnectionStatus, TDes& aStatusString );

        /**
         * Checks that the tester process is still alive. Leaves if it's dead. 
         */
        void TesterExeAliveL();
        
        void CreateTesterProcessL();

    protected:  // New functions

    private:

        /**
        * C++ default constructor.
        */
        CVCXConnUtilTest( CTestModuleIf& aTestModuleIf );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Frees all resources allocated from test methods.
        * @since ?Series60_version
        */
        void Delete();

        /**
        * Test methods are listed below.
        */

        /**
         * Tries to close all tester exe's, should be called in start of each case before call 
         * to any CreateL.
         */
        virtual TInt PrepareCaseL( CStifItemParser& aItem );

        /**
         * Commands the tester exe to shutdown checks for any error codes for the process exit.
         * Should be called in end of each test case before deleting test module. 
         */
        virtual TInt ShutdownTesterL( CStifItemParser& aItem );
        
        /**
        * Initializes the test module.
        * @param mode, If KVcxConnUtilTestModeNormal
        *                 - Doesn't subscribe ConnUtil P&S keys.
        *                 - Main use connectivity.
        *
        *              If KVcxConnUtilTestModeSubscribe
        *                 - Subscribes ConnUtil P&S keys and keeps track of the changes.
        *                 - Doesn't create instance of ConnUtil, can't be used for connectivity.
        * @param caseId.
        */
        virtual TInt CreateL( CStifItemParser& aItem );

        /**
         * Calls CVcxConnectionUtility::GetIap which creates connection.
         * Can only be used when test module is created in KVcxConnUtilTestModeNormal mode.
         */
        virtual TInt GetIapL( CStifItemParser& aItem );
        
        /**
         * Calls CVcxConnectionUtility::WapIdFromIapIdL for the defined IAP.
         * @param iapName, if IAP is not found then invalid IAP ID is used.
         */
        virtual TInt WapIdFromIapIdL( CStifItemParser& aItem );

        /**
         * Calls CVcxConnectionUtility::Disconnect which disconnects active connection.
         * Can only be used when test module is created in KVcxConnUtilTestModeNormal mode.
         */
        virtual TInt DisconnectL( CStifItemParser& aItem );

        /**
         * Calls CVcxConnectionUtility::Disconnect which disconnects active connection.
         * Can be used always.
         */
        virtual TInt CheckPropertyL( CStifItemParser& aItem );

        /**
         * Sets roaming flag which is returned to ConnUtil upon request.
         * Can only be used when test module is created in KVcxConnUtilTestModeNormal mode.
         */
        virtual TInt SetRoamingAllowedL( CStifItemParser& aItem );

        /**
         * Sets flag for test module to cause User::Leave when it gets roaming request.
         * Can only be used when test module is created in KVcxConnUtilTestModeNormal mode.
         */
        virtual TInt SetLeaveAtRoamingRequestL( CStifItemParser& aItem );

        /**
         * Sets delay before roaming request is answered.
         * Can only be used when test module is created in KVcxConnUtilTestModeNormal mode.
         */
        virtual TInt SetDelayBeforeRoamingRequestL( CStifItemParser& aItem );

        /**
         * Gets the values for ConnUtil P&S properties and prints 'em to debug ouput.
         * Can be used always.
         */
        virtual TInt PrintConnUtilDetails( CStifItemParser& aItem );

        /**
         * Adds property and value to queue of waited P&S changes.
         * @param property
         * @param value
         * Can only be used when test module is created in KVcxConnUtilTestModeSubscriber mode.
         */
        virtual TInt AddWaitedPropertyL( CStifItemParser& aItem );

        /**
         * Adds property that is causes error signal while there's properties in queue
         * added with AddWaitedProperty.
         * @param property
         * @param value
         * Can only be used when test module is created in KVcxConnUtilTestModeSubscriber mode.
         */
        virtual TInt AddNotAllowedPropertyL( CStifItemParser& aItem );

        /**
         * Resets the queue of expected property changes.
         * Can only be used when test module is created in KVcxConnUtilTestModeSubscriber mode.
         */
        virtual TInt ResetWaitedPropertiesL( CStifItemParser& aItem );

        /**
         * After call to this when queue for waited properties is empty the test script will be signaled.
         * Can only be used when test module is created in KVcxConnUtilTestModeSubscriber mode.
         */
        virtual TInt WaitForPropertyQueueL( CStifItemParser& aItem );

    private:    // Data

        CVCXTestCommon* iTestCommon;

        // KVcxConnUtilTestModeSubscriber or KVcxConnUtilTestModeNormal
        TInt iMode;

        CVCXConnUtilTestSubscriber* iConnectionStatusSubscriber;
        CVCXConnUtilTestSubscriber* iIapIdSubscriber;
        CVCXConnUtilTestSubscriber* iSnapIdSubscriber;
        CVCXConnUtilTestSubscriber* iMasterExistsSubscriber;
        CVCXConnUtilTestSubscriber* iNbrConnInstancesSubscriber;
        CVCXConnUtilTestSubscriber* iRoamingRequestStatusSubscriber;
        CVCXConnUtilTestSubscriber* iNbrRoamRespSubscriber;
        CVCXConnUtilTestSubscriber* iNbRoamAcceptedSubscriber;

        // Array of P&S changes to properties of VcxConnUtil
        RArray<TVcxConnTestPSProperty> iWaitedPSChanges;

        RArray<TVcxConnTestPSProperty> iNotAllowedPSChanges;

        TBool iWaitingForEmptyPropertyQueue;

        // Timeout timer
        CIptvTestTimer* iTimeoutTimer;

        CIptvTestActiveWait* iWait;

        // Is wait for connection request enabled.
        TInt iWaitTimedRequest;

        // This will show in function traces.
        TBuf<256> iName;
        
        // Testexe
        RProcess iProcess;
        
        TInt iPSKeyBase;
        
        CVCXConnUtilTestSubscriber* iTesterGetAckResponseSubscriber;        
        CVCXConnUtilTestSubscriber* iTesterResponseSubscriber;        
        
        CVCXConnUtilTestSubscriber* iTesterCountSubscriber;
        
        TBool iShutdownCalled;
        
        TBool iTesterCountChangedAlready;
    };

#endif      // VCXCONNUTILTEST_H

// End of File
