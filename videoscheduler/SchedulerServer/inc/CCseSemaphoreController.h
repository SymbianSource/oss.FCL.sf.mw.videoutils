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
* Description:    Semaphore controller. Checks once a while if given semaphore is*
*/




#ifndef __CCSESEMAPHORECONTROLLER_H
#define __CCSESEMAPHORECONTROLLER_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS
// None.

// MACROS
// None.

// DATA TYPES
// None
        
// FUNCTION PROTOTYPES
// None.

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION
class RSemaphore;
class MCseSemaphoreSignalObserver;

/**
*  CCseSemaphoreController
*
*  @lib
*/
class CCseSemaphoreController : public CTimer
    {
    	public:	// Constructors and destructors
		/**
        * Destructor.
        */
		virtual ~CCseSemaphoreController();
		
		/**
        * Two-phased constructor.
        * @param    aSemaphore  Semaphore to be followed.
        * @param    aObserver   A pointer back to server engine through observer.
        */
		static CCseSemaphoreController* NewL( RSemaphore& aSemaphore,
                                              MCseSemaphoreSignalObserver& aObserver );
      		
        /**
        * Starts checking if semaphore is signalled. Default interval is 5000ms.
        */
        void Start( );
        
        /**
        * Stops checking semaphore.
        */
        void Stop( );
        
	private: // Constructors and destructors
	 	/**
        * By default Symbian 2nd phase constructor is private.
        */
		void ConstructL();
		
		/**
        * C++ default constructor.
		* @param    aSemaphore  Semaphore to be followed.
        * @param    aObserver   A pointer back to server engine through observer.
        */
		CCseSemaphoreController( RSemaphore& aSemaphore,
                                 MCseSemaphoreSignalObserver& aObserver  );
		
	private: // Functions from base classes
		/**
        * RunL        
        * @return   None
        */	
        void RunL();                                           
        
        /**
        * Sets new interval how often semaphore signal is checked. Doesn't restart
        * checking automatically.
        * @param aInterval  Interval how often semaphore signal is checked in milliseconds.
        */
        void SetInterval( TUint aInterval );
		
	private: // data
	    
	    /**
        * Semaphore under observation.
        */
	    RSemaphore& iSemaphore;
	    
	    /**
        * Reference to observer.
        */
	    MCseSemaphoreSignalObserver& iObserver;
        
        /**
        * Interval how often semaphore signal is checked in milliseconds.
        * Default is 5000 ms.
        */
	    TUint iInterval;
    };


#endif // __CCSESEMAPHORECONTROLLER_H
