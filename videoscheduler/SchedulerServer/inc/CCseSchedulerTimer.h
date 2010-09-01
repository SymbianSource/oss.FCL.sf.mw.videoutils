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
* Description:    Header file for CCSeSchdulerTimer-class.*
*/




#ifndef _CCSESCHEDULERTIMER_H
#define _CCSESCHEDULERTIMER_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS
// None
// MACROS
// None

// DATA TYPES
// None

// FUNCTION PROTOTYPES
// None

// FORWARD DECLARATIONS
class MCseSchedulerTimerObserver;
class CCseSchedulerPluginIF;

/**
* Timer for determining when schedules should be run.
*/
class CCseSchedulerTimer : public CTimer
{
	public:	// Constructors and destructors
		/**
        * Destructor.
        */
		virtual ~CCseSchedulerTimer();
		
		/**
        * Two-phased constructor.
        * @param    aObserver   A pointer back to server engine through observer
        */
		static CCseSchedulerTimer* NewL( MCseSchedulerTimerObserver* aObserver );

	public:	// New methods
		/**
        * Sets timer to fire on given time
        * @param    aTime   Time when timer is set to fire
        * @return   None
        */
		void SetTimer( const TTime& aTime );

	private: // Constructors and destructors
	 	/**
        * By default Symbian 2nd phase constructor is private.
        */
		void ConstructL();
		
		/**
        * C++ default constructor.
		* @param    aObserver   A pointer back to server engine through observer.
        */
		CCseSchedulerTimer( MCseSchedulerTimerObserver* aObserver );		
		
	private: // Functions from base classes
		/**
        * RunL
        * @return   None
        */	
		void RunL();
		
		/**
        * RunError
        * @return   See CTimer documentation.
        */
		TInt RunError( TInt aError );
		
		/**
        * DoCancel
        * @return   None
        */
		void DoCancel();
		
	private: // Data
		/**
		* Pointer back to scheduler engine. Not own.
		*/
		MCseSchedulerTimerObserver* iObserver;
};

#endif //_CCSESCHEDULERTIMER_H