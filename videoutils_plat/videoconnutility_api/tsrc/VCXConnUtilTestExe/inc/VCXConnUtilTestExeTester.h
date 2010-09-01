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
* Description: 
*
*/


#ifndef VCXCONNUTILTESTEXETESTER_H_
#define VCXCONNUTILTESTEXETESTER_H_


//  INCLUDES
#include <e32base.h>
#include <e32std.h>
#include <e32msgqueue.h>
#include "VCXConnUtilTestPSObserver.h"

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION
class CVCXConnUtilTestSubscriber;
class CVCXConnUtilTester;

/**
* Observer PS key and when it changes to 1 sends msg to queue to stop the tester exe.
*  @since
*/
class CVCXConnUtilTestExeTester : public CBase, public MVCXConnUtilTestPSObserver
    {
    public:  // Constructors and destructor
        /**
        * Two-phased constructor.
        */
        static CVCXConnUtilTestExeTester * NewL( TUint aProcessId );

        /**
        * Destructor.
        */
        virtual ~CVCXConnUtilTestExeTester();

    public: // New functions

        TBool Running() { return iRunning; };
        
    protected: // From base classes

        void ValueChangedL( const TUid& aUid, const TUint32& aKey, const TInt& aValue );
        
        void ValueChangedL( const TUid& aUid, const TUint32& aKey, const TDesC& aValue );
        
    private:

        /**
        * C++ default constructor.
        */
        CVCXConnUtilTestExeTester();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( TUint aProcessId );

    private:    // Data
        TBuf<256> iName;
        
        CVCXConnUtilTester* iTester;
        
        CVCXConnUtilTestSubscriber* iGlobalShutdownSubcriber;
        
        CVCXConnUtilTestSubscriber* iCmdSubcriber;
        
        TBool iRunning;
        
        TInt iPSKeyBase;
        
        TInt iAckCount;
    };

#endif // VCXCONNUTILTESTEXETESTER_H_

// End of File
