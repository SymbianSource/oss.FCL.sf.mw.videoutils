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
* Description:    Header for PluginStarter-class*
*/



#ifndef __CCSESCHEDULERPLUGINSTARTER_H
#define __CCSESCHEDULERPLUGINSTARTER_H

// INCLUDES
#include <e32base.h>
#include <ipvideo/MCsePluginObserver.h>
#include "MCseSemaphoreSignalObserver.h"

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FUNCTION PROTOTYPES
// None

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION
class CCseSchedulerThreadPacket;
class CCseSchedulerPluginIF;
class CCseSchedulerLandLord;
class CCseSemaphoreController;

/**
* Handles running and shutting down one plugin. Runs in seperate thread than rest of the SchedulerEngine
*/
class CCseSchedulerPluginStarter : public CBase,
                                   public MCseSemaphoreSignalObserver,
                                   public MCsePluginObserver
    {
    public: // Constructors and destructor
        /**
        * Two-phased constructor.
        * @param    aThreadPacket   Thread packet containing all the needed information to plugin
        *                           to be run.
        * @return CCseSchedulerPluginStarter pointer to CCseSchedulerPluginStarter class
        */
        static CCseSchedulerPluginStarter* NewL( CCseSchedulerThreadPacket* aThreadPacket );

        /**
        * Destructor.
        */
        virtual ~CCseSchedulerPluginStarter();
        
    public: // from MCsePluginObserver-class
        void PluginCompleted( TInt aCompletitionValue );
        
    public: // from MCseSemaphoreSignalObserver-class
        void SemaphoreSignalled( );        
        
    public: // New methods
        /**
        * Creates plugin and runs schedule in it.
        * @param    aParam    Any pointer that actually holds CCseThreadPacket
        * @return   KErrNone if succesful, otherwise common Symbian error codes.
        */
        static TInt StartThread( TAny* aParam );

    private: // Constructors and destructor
        /**
        * C++ default constructor.
        */
        CCseSchedulerPluginStarter( CCseSchedulerThreadPacket* aThreadPacket );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        
    private:  // New methods
		/**
        * Called from actual StartThread. Creates CCseSchedulerPluginStarter.
        * @param    aParam    Any pointer that actually holds CCseThreadPacket        
        */
        static void DoStartThreadL( TAny* aParam );        
        
        /**
        * Creates plugin based on schedule and runs schedule in it.
        */
        void RunPluginL( );
        
        /**
		* Plugin complete callback used with CIdle. Used to break call stack.
		* @param    aPtr    Pointer back to CCseSchedulerPluginStarter-class
		* @return   Always  EFalse.
		*/
		static TInt PluginCompleteCallback( TAny* aPtr );

		/**
		* Plugin complete callback handler. Stops active scheduler, sets completition value
		* and runs down the plugin.		
		* @return   Always EFalse
		*/
		TInt HandlePluginCompleteCallback();
        
   private:  // Data
        /**
        * Thread packet. Includes all needed information for plugin and the completiotion value.
        * Own.
        */
        CCseSchedulerThreadPacket* iThreadPacket;
        
        /**
        * Plugin in run. Own.
        */
        CCseSchedulerPluginIF* iPlugin;
        
        /**
        * CIdle, used to break call stack when plugin completes. Own.
        */ 
        CIdle* iCompletitionBreaker;
        
        /**
        * Bool flagging if plugin is wanted to be shot down
        */ 
        TBool iPluginRdyToTakedown;
        
        /**
        * Semaphore monitor. Check once a while if shutdown semaphore is signalled.
        */ 
        CCseSemaphoreController* iSemaphoreMonitor;
    };
    
#endif // __CCSESCHEDULERPLUGINSTARTER_H