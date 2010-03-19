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
* Description:    Header for PluginControllerBase-class*
*/




#ifndef __CCSESCHEDULERPLUGINCONTROLLERBASE_H
#define __CCSESCHEDULERPLUGINCONTROLLERBASE_H

// INCLUDES
#include <e32base.h>
#include <ipvideo/mcsepluginobserver.h>
#include <ipvideo/mcsescheduleobserver.h>
#include "MCseSchedulerTimerObserver.h"
#include "MCsePluginControllerObserver.h"

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
class CCseScheduledProgram;
class CCseSchedulerTimer;
class CCseSchedulerServerEngine;

/**
* Base class for plugin controllers. Handles all plugin related stuff except scheduling logic.
*/
class CCseSchedulerPluginControllerBase : public CBase,                                          
                                          public MCseScheduleObserver,
                                          public MCseSchedulerTimerObserver
    {
    public: // Constructors and destructor
        /**
        * Two-phased constructor.
        * @params aEngine reference to engine
        * @params aPluginUid plugin uid of the plugin this plugin controller is made for
        * @return CCseSchedulerPluginControllerBase pointer to CCseSchedulerPluginControllerBase class
        */
        static CCseSchedulerPluginControllerBase* NewL( MCsePluginControllerObserver& aEngine, TInt32 aPluginUid );

        /**
        * Destructor.
        */
        virtual ~CCseSchedulerPluginControllerBase();

    protected:
        /**
        * C++ default constructor.
        */
        CCseSchedulerPluginControllerBase( MCsePluginControllerObserver& aEngine, TInt32 aPluginUid );
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();           
    
    public: // from MCseScheduleObserver-class     
        /**
        * Plugins uses this method to inform common scheduler engine that
        * schedule is handled.
		* @param aDbIdentifier DbIdentifier of scheduled program that just
		*                      has been processed.
        */
        virtual void ScheduleCompletedL( const TUint32 aDbIdentifier, TInt aCompletitionCode );

    public: // from MCseSchedulerTimerObserver-class        
        /**
        * Plugins uses this method to inform common scheduler engine that
        * schedule is handled.
		* @param aDbIdentifier DbIdentifier of scheduled program that just
		*                      has been processed.
        */
        virtual void TimerErrorL( const TInt32 aError );
        
        /**
        * Timer is up, run the Plugins!!!
        */
        virtual void RunPluginsL( );

    public: // New methods
    
        /**
         * Sets plugin to be cleared after callstack break.
         * @param aToBeCleared ETrue if controller needs to be cleared.
         */
        void SetToBeCleared( TBool aToBeCleared );
        
        /**
         * Clears controller: clears (takes down all threads running for the plugin)
         * all landlords. 
         */
        virtual void ClearControllerL();
        
        /**
        * Cleans the controller from LandLord(s) that aren't needed anymore.
        */
        virtual void DoCleanUp( );
     
        /**
        * Sets schedules to controller. More precise information is found from Uni and Multi
        * controller classes.
        * @params aScheduleArrat    Array of schedules to be set into controller. All given
        *                           schedules should have same start time.
        */
        virtual void SetSchedulesL( RPointerArray<CCseScheduledProgram>& aScheduleArray );
                
        /**
        * Gets plugin id. This Uid of the plugin going to be run when time is right. Can be
        * used, for example, to find right kind of controller to a schedule.
        * @return plugin Uid of the plugin used to run the schedule.
        */
        TInt32 PluginUid( );        
        
        /**
        * Checks if the plugin is obsolete (There are no more schedules to be run).
        * @return ETrue if obsolete, EFalse if not.
        */
        TBool IsObsolete() const;

        /**
        * Checks if the plugin is set to be cleared.
        * @return ETrue if set, EFalse if not.
        */
        TBool IsToBeCleared();
        
        /**
        * Checks if there are active plugins in controller.
        * @return ETrue if active plugins found, EFalse otherwise
        */
        virtual TBool IsControllerActive();
        
    protected: //data        
        /**
        * Array containing schedules to be run. Own.
        */
        RPointerArray<CCseScheduledProgram> iScheduleArray;
        
         /**
        * Timer class. Own.
        */
        CCseSchedulerTimer* iTimer;
        
        /**
        * Reference to engine
        */
        MCsePluginControllerObserver& iEngine;
        
        /**
        * Uid of the plugin contrelled by this controller.
        */        
        TInt32 iPluginUid;
        
        /**
        * Boolean indicating if this plugin controller is to cleared
        * (plugin running is deleted).
        */        
        TBool iToBeCleared;
    };
    
#endif // __CCSESCHEDULERPLUGINCONTROLLERBASE_H
