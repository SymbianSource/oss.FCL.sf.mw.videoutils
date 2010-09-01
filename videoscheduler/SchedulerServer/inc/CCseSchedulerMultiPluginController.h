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
* Description:    Header for MultiPluginController-class*
*/




#ifndef __CCSESCHEDULERMULTIPLUGINCONTROLLER_H
#define __CCSESCHEDULERMULTIPLUGINCONTROLLER_H

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
* Controller for plugins that can be several in run at same time
*/
class CCseSchedulerMultiPluginController : public CCseSchedulerPluginControllerBase								  
    {
    public: // Constructors and destructor
        /**
        * Two-phased constructor.
        * @params aEngine reference to engine
        * @params aPluginUid plugin uid of the plugin this plugin controller is made for
        * @return CCseSchedulerPluginControllerBase pointer to CCseSchedulerPluginControllerBase class
        */
        static CCseSchedulerMultiPluginController* NewL( CCseSchedulerServerEngine& aEngine, TInt32 aPluginUid );

        /**
        * Destructor.
        */
        virtual ~CCseSchedulerMultiPluginController();
    
    public: // from CCseSchedulerPluginControllerBase
        void ClearControllerL();

        TBool IsControllerActive();
        
        void RunPluginsL() ;
                
        void DoCleanUp() ;
                
        void ScheduleCompletedL( const TUint32 aDbIdentifier, TInt aCompletitionCode );

    private: // Constructors and destructor
        /**
        * C++ default constructor.
        */
        CCseSchedulerMultiPluginController( CCseSchedulerServerEngine& aEngine, TInt32 aPluginUid );

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
        * LandLords. Owns data that is given to new thread and acts as observer to thread.
        * Own.
        */  
        RPointerArray<CCseSchedulerLandLord> iLandLords;        
    };
    
#endif // __CCSESCHEDULERMULTIPLUGINCONTROLLER_H
