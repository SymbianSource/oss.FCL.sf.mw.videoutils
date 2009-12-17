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


#ifndef VCXCONNUTILTESTER_H
#define VCXCONNUTILTESTER_H

#include <e32base.h>
#include "VCXConnUtilTester.h"

#include "vcxconnutilengineobserver.h"
#include "../../../../../videoconnutility/inc/vcxconnectionutility.hrh"
#include "MIptvTestTimerObserver.h"

class CIptvTestTimer;
class CIptvTestActiveWait;
class CVcxConnectionUtility;

/**
* Video connection utility tester class.
*/
class CVCXConnUtilTester : public CBase,
                           public MConnUtilEngineObserver,
                           public MIptvTestTimerObserver
    {
    public:

        /**
        * Two-phased constructor.
        */
        static CVCXConnUtilTester* NewL();

        /**
        * C++ default constructor.
        */
        CVCXConnUtilTester();

        /**
        * Destructor.
        */
        ~CVCXConnUtilTester();

    public: // Public new functions

        /**
         * ConnUtil GetIap
         *
         * @return an error code.
         */
        TInt GetIap( TUint32& aIapId, TBool aSilent, TBool aIsTimed );

        /**
         * ConnUtil WapIdFromIapIdL
         */
        TUint32 WapIdFromIapIdL( const TInt32& aIapId );
        
        /**
         * ConnUtil Disconnect
         *
         * @return an error code.
         */
        TInt Disconnect();

    private: // Constructors and destructor

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private: // Functions from base classes
        /**
         * From MConnUtilEngineObserver.
         */
        TBool RequestIsRoamingAllowedL();

        /**
         * From MConnUtilEngineObserver.
         */
        void  IapChangedL();

        /**
        * From MIptvTestTimerObserver Handles timer completion
        * @since
        * @param aTimerId
        * @param aError
        */
        void TimerComplete(TInt aTimerId, TInt aError);

    private: // Data

        CVcxConnectionUtility* iConnUtil;

    public: // Data
        TBool iIsRoamingAllowed;

        TBool iLeaveAtRoamingRequest;

        TInt iDelaySecondsAtRoamingRequest;

        CIptvTestTimer* iTimer;

        CIptvTestActiveWait* iWait;

        // This will show in function traces.
        TBuf<256> iName;
    };

#endif // VCXCONNUTILTESTER_H
