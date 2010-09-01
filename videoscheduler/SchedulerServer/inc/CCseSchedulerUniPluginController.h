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
* Description:    Header for UniPluginController-class*
*/




#ifndef __CCSESCHEDULERUNIPLUGINCONTROLLER_H
#define __CCSESCHEDULERUNIPLUGINCONTROLLER_H

// INCLUDES
#include "CCseSchedulerPluginControllerBase.h"

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
class CCseSchedulerPluginStarter;
class CCseSchedulerLandLord;

/**
* Controller for plugins that can be only be in run one at the time
*/
class CCseSchedulerUniPluginController : public CCseSchedulerPluginControllerBase								  
    {
    public: // Constructors and destructor
        /**
        * Two-phased constructor.
        * @params aEngine reference to engine
        * @params aPluginUid plugin uid of the plugin this plugin controller is made for
        * @return CCseSchedulerPluginControllerBase pointer to CCseSchedulerPluginControllerBase class
        */
        static CCseSchedulerUniPluginController* NewL( CCseSchedulerServerEngine& aEngine, TInt32 aPluginUid );

        /**
        * Destructor.
        */
        virtual ~CCseSchedulerUniPluginController();
        
        
    public: // from CCseSchedulerPluginControllerBase
    
        void ClearControllerL();
        
        TBool IsControllerActive();
        
        void RunPluginsL() ;
    
        void SetSchedulesL( RPointerArray<CCseScheduledProgram>& aScheduleArray );
    
        void ScheduleCompletedL( const TUint32 aDbIdentifier, TInt aCompletitionCode );
        
        void TimerErrorL( const TInt32 aError );

    private: // Constructors and destructor
        /**
        * C++ default constructor.
        */
        CCseSchedulerUniPluginController( CCseSchedulerServerEngine& aEngine, TInt32 aPluginUid );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        
    private: // New methods
       	/**
        * Handles succesfully completed schedules.
        * @param aDbIdentifier DbIdentifier of schedule just completed
        */
		void ScheduleCompletedSuccesfullyL( const TUint32 aDbIdentifier );
    
        /**
        * Handles failed schedules.
        * @param aDbIdentifier DbIdentifier of schedule just completed
        * @param aCompletitionCode Error code of the completed schedule.
        */
        void HandleScheduleErrorL( const TUint32 aDbIdentifier,
                                   TUint32 aCompletitionCode );
    private: // Data
        
        /**
        * LandLord. Owns data that is given to new thread and acts as observer to thread.
        * Own.
        */  
        CCseSchedulerLandLord* iLandLord;
        
        /**
        * Boolean representing if the LandLord is active or not.
        */  
        TBool iIsLandLordWorking;
    };
    
#endif // __CCSESCHEDULERUNIPLUGINCONTROLLER_H
