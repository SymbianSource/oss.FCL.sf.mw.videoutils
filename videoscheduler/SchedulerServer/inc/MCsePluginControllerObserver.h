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
* Description:    Observer to inform common scheduling engine when plugin is*
*/




#ifndef __MCSEPLUGINCONTROLLEROBSERVER_H__
#define __MCSEPLUGINCONTROLLEROBSERVER_H__

class CCseSchedulerPluginControllerBase;

// CLASS DECLARATION
/**
* MCsePluginControllerObserver
* This class offers plugin controller some of the engine's utilities.
*/
class MCsePluginControllerObserver
    {
    public: // New functions                
        /**
        * Method to inform succesfully completed schedules.
        * @param    aDbIdentifier   Identifier of the schedule that has
        *                           completed.
        */
        virtual void ScheduleCompletedSuccesfullyL( const TUint32 aDbIdentifier ) = 0;
        
        /**
        * Method to inform failed schedules.
        * @param    aDbIdentifier   Identifier of the schedule that has
        *                           completed.
        */
        virtual void ScheduleCompletedWithErrorL( const TUint32 aDbIdentifier,
                                                  TUint32 aErrorCode ) = 0;
        
        /**
        * Orders engine to increase run count of given shedule.
        * @param    aDbIdentifier   Identifier of the schedule who's run
        *                           needs to be incremented.
        */                                                 
        virtual void IncreaseScheduleRunCountL( const TUint32 aDbIdentifier ) = 0;
        
        /**
        * Asks from engine if given schedule can be run
        * @param    aDbIdentifier   Identifier of the schedule to be checked.
        * @return boolean value representing if schedule can be run or not.
        */
        virtual TBool IsAllowedToRun( const TUint32 aDbIdentifier ) = 0;
        
        /**
        * Reports not valid schedule to engine (schedule cannot be run anymore).
        * @param    aDbIdentifier   Reported schedules identifier.        
        */
        virtual void ScheduleNotValid( const TUint32 aDbIdentifier ) = 0;
        
        /**
        * Requests rescheduling for plugin controllers        
        */
        virtual void RequestReschedule( ) = 0;
        
        /**
         * Signals when plugin controller clearing is done.        
         */
        virtual void ClearingDone( ) = 0;
    };

#endif // __MCSEPLUGINCONTROLLEROBSERVER_H__

// End of File
