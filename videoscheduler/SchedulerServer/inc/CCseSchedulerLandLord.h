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
* Description:    Header file for CCseSchedulerThreadCrossroads-class.*
*/




#ifndef _CCSESCHEDULERLANDLORD_H
#define _CCSESCHEDULERLANDLORD_H

//  INCLUDES
#include <e32base.h>
#include "MCseSemaphoreSignalObserver.h"

// CONSTANTS
#define KThreadNameSize     19

// MACROS
// None

// DATA TYPES
// None

// FUNCTION PROTOTYPES
// None

// FORWARD DECLARATIONS
class MCseScheduleObserver;
class RThread;
class CCseScheduledProgram;
class CCseSchedulerThreadPacket;
class CCseSemaphoreController;
class CActiveSchedulerWait;

/**
* LandLord controlling shared data that is given to thread. After plugin thread is started,
* nothing should be accessed before thread has died.
*
* Also observes thread when it dies.
*/
class CCseSchedulerLandLord : public CActive,
                              public MCseSemaphoreSignalObserver
                              
{
	public:	// Constructors and destructors
		/**
        * Destructor.
        */
		virtual ~CCseSchedulerLandLord();
		
		/**
        * Two-phased constructor.
        * @param    aObserver   A pointer back to plugin controller.
        */
		static CCseSchedulerLandLord* NewL( MCseScheduleObserver* aObserver );

	private: // Constructors and destructors
	 	/**
        * By default Symbian 2nd phase constructor is private.
        */
		void ConstructL();
		
		/**
        * C++ default constructor.
		* @param    aObserver   A pointer back to plugin controller.
        */
		CCseSchedulerLandLord( MCseScheduleObserver* aObserver );	
		
    public:
        /**
        * Set schedule to LandLord.
        * @param aData  CCseScheduledProgram to be set.
        */
        void SetScheduleL( CCseScheduledProgram& aData );
        
        /**
        * Returns ThreadPacket of the LandLord
        * @return pointer to ThreadPacket
        */
        CCseSchedulerThreadPacket* ThreadPacket( );
		
		/**
        * Run plugin. Creates thread where the CCseSchedulerPluginStarter is run
        * and sets itself observing when thread dies.        
        */
		void RunPluginL( );
		
		/**
         * Clears landlord. Uses semaphore to signal plugin thread that thread needs
         * to be taken down.
         * Synchronous.        
         */
		void ClearL();
		
		/**
        * Returns ETrue if LandLord is working (plugin is running).        
        */
		TBool IsWorking();
		
	private: // CActive
		/**
        * RunL
        * @return   None
        */	
		void RunL();
		
		/**
        * RunError
        * @return   
        */
		TInt RunError( TInt aError );
		
		/**
        * DoCancel
        * @return   None
        */
		void DoCancel();
    
	private:  // from MCseSemaphoreSignalObserver
        void SemaphoreSignalled( );   
		
    private:    // New methods
        /**
        * Generates unique thread name to internal iThreadName-buffer.
        */
        void GenerateThreadNameL();
        
        /**
         * Generates semaphores to signal thread shutdown and when shutdown
         * is completed.
         */
        void GenerateSemaphoresL( );
        
        /**
         * Starts or stops active wait. Used when thread needs to be taken down.
         */
        void ActiveWait( TBool aStart );     
		
	private: // Data
		/**
		* Pointer back to plugin controller. Own.
		*/
		MCseScheduleObserver* iObserver;
		
		/**
		* ThreadPacket. Contains data to be given plugin thread when run. Own.
		*/
		CCseSchedulerThreadPacket* iThreadPacket;
		
		/**
		* Thread where plugin is run.
		*/
		RThread iThread;
		
		/**
		* Unique thread name. Own.
		*/
		HBufC* iThreadName;
		
		/**
		* ETrue if thread is running, EFalse if not.
		*/
		TBool iThreadRunning;
		
		/**
		* Semaphore signalled when plugin is wanted to take down.
		*/
		RSemaphore iPluginShutdownSemaphore;

     	/**
		* Semaphore signalled when plugin has been taken down.
		*/		
		RSemaphore iPluginShutdownCompleteSemaphore;
		
		/**
		* ActiveSchedulerWait. Own.
		*/
		CActiveSchedulerWait* iActiveSchedulerWait;
		
		/**
		* Checks once a while if plugin shutdown has been completed.
		* Own.
		*/
		CCseSemaphoreController* iShutdownCompleteMonitor;
		
		/**
         * ETrue if shutdown has been signalled to CCSeSchedulerPluginStarter.
         * EFalse if not.
         */
		TBool iTakedownRequested;
};

#endif //_CCSESCHEDULERLANDLORD_H
