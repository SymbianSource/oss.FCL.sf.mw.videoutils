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



#ifndef CIPTVTESTACTIVEWAIT_H
#define CIPTVTESTACTIVEWAIT_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*
*
*
*
*  @since
*/
class CIptvTestActiveWait : public CBase
    {
    public:  // Constructors and destructor
        /**
        * Two-phased constructor.
        */
        IMPORT_C static CIptvTestActiveWait * NewL();

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CIptvTestActiveWait ();


	enum TIptvTestActiveWaitCmd
       	{
       	EActiveWaitStart,
       	EActiveWaitStop
       	};

    public: // New functions

    	/*
    	* To start and stop CActiveSchedulerWait
    	*/

		IMPORT_C void ActiveWait( TIptvTestActiveWaitCmd aActiveWaitCmd );

		/*
		 * Returns ETrue if CActiveSchedulerWait is running.
		 */
		IMPORT_C TBool IsWaiting();

        /*
         * Starts active wait.
         */
        IMPORT_C TInt Start();

        /*
         * Stops active wait.
         */
        IMPORT_C TInt Stop();


    protected: // From base classes

    private:

        /**
        * C++ default constructor.
        */
        CIptvTestActiveWait ();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        IMPORT_C void ConstructL();

    private:    // Data
		CActiveSchedulerWait* iActiveSchedulerWait;
    };


#endif      // CIPTVTESTACTIVEWAIT_H

// End of File
