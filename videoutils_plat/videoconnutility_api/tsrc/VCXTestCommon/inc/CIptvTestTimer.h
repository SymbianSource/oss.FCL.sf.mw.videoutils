/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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



#ifndef CIPTVTESTTIMER_H
#define CIPTVTESTTIMER_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class MIptvTestTimerObserver;

// CLASS DECLARATION

/**
*
*
*
*
*  @since
*/
class CIptvTestTimer : public CActive
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CIptvTestTimer* NewL(MIptvTestTimerObserver& aObserver, TInt aId);

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CIptvTestTimer();


    public: // New functions
		IMPORT_C void After(TTimeIntervalMicroSeconds32 aInterval);
		IMPORT_C void CancelTimer( );

    protected: //from base classes
	    void RunL();
	    void DoCancel();

    private:

        /**
        * C++ default constructor.
        */
        CIptvTestTimer(MIptvTestTimerObserver& aObserver, TInt aId);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data
	    RTimer iTimer;
	    MIptvTestTimerObserver& iObserver;
	    TInt  iId;
	    TBool iRestartAfterCancel;
	    TTimeIntervalMicroSeconds32 iInterval;
    };


#endif      // CIPTVTESTTIMER_H

// End of File
