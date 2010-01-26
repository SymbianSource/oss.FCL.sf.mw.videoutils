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
* Description:    Interface for CseScheduler Client's Service class*
*/



#ifndef __RCSESCHEDULERSERVICE_H__
#define __RCSESCHEDULERSERVICE_H__

// INCLUDES
#include <ipvideo/rcseschedulerservicebase.h>   // Client common methods (open, close etc)

// CONSTANTS
// None.

// MACROS
// None.

// DATA TYPES
// None.

// FUNCTION PROTOTYPES
// None.

// FORWARD DECLARATIONS
class CCseScheduledProgram;

// CLASS DECLARATION    
/**
*  General client api that manages service functionality.
*
*  @lib CseSchedulerClient.lib
*/
class RCseSchedulerService : public RCseSchedulerServiceBase
    {
    public: // Constructors and destructor
        
        /**
        * Constructor.
        * @return None.
        */
        RCseSchedulerService();

        /**
        * Destructor.
        * @return None.
        */
        virtual ~RCseSchedulerService();
    
    public: // New methods
    	/**
        * Adds given schedule to Common Scheduling Engine database. After the call is
        * completed, the parameter contains the DB identifier of the schedule.
        * @param    aData   Schedule to be added to database.        
        */
        void AddScheduleL( CCseScheduledProgram& aData ) const;
        
        /**
        * Removes schedule from database.
        * @param    aDbIdentifier  Database identifier of the schedule to be removed.
        * @return Generic symbian error code.
        */
        void RemoveScheduleL( const TUint32 aDbIdentifier ) const;
        
        /**
        * Gets schedule from database.
        * @param    aDbIdentifier   Database identifier of the schedule that is wanted
        *                           from the database
        * @param    aProg           Pointer where this scheduled program is get.
        * @return   Generic symbian error codes.
        */
        void GetScheduleL( const TUint32 aDbIdentifier,
        				  CCseScheduledProgram* aProg ) const;

		/**
		* Get list of scheduled events from the scheduling engine based on given UID.
		* @param    aAppUid     Application UID.
		* @param    aArray      On return contains items from the database.
		* @return   Generic symbian error code.
		*/
		void GetSchedulesL( const TInt32 aAppUid, 
							RPointerArray<CCseScheduledProgram>& aArray ) const;

		/**
		* Get overlapping schedule (if any) from the scheduling engine.
		* Uses schedule type, start and end times to find overalapping 
		* schedules from the Commone Scheduling Engine database.
		* @param    aProgram        Schedule to used for finding overlapping schedules.
		* @param    aResultArray    On return, contains array of overlapping 
		* 		                    schedules, empty if none found.
		* @return Generic symbian error code.
		*/
		void GetOverlappingSchedulesL( CCseScheduledProgram& aProgram, 
					RPointerArray<CCseScheduledProgram>& aResultArray );
		
		/**
		* Get list of scheduled events from the scheduling engine based on given plugin UID.
		* @param    aPluginUid     Plugin UID.
		* @param    aArray      On return contains items from the database.
		* @return   Generic symbian error code.
		*/			
		void GetSchedulesByPluginUidL( const TInt32 aPluginUid, 
					                   RPointerArray<CCseScheduledProgram>& aArray ) const;
        
        /**
		* Get list of scheduled events from the scheduling engine based on given schedule type.
		* @param    aType       Schedule type.
		* @param    aArray      On return contains items from the database.
		* @return   Generic symbian error code.
		*/			
        void GetSchedulesByTypeL( const TInt32 aType, 
			                      RPointerArray<CCseScheduledProgram>& aArray ) const;
        
        /**
		* Get list of scheduled events from the scheduling engine between given timeframe.
		* @param    aBeginning  Beginning of the timeframe.
		* @param    aEnd        End of the timeframe.
		* @param    aArray      On return contains items from the database.
		* @return   Generic symbian error code.
		*/						                    
        void GetSchedulesByTimeL( const TTime& aBegining, 
                                  const TTime& aEnd,
			                      RPointerArray<CCseScheduledProgram>& aArray ) const;
    };

#endif //__RCSESCHEDULERSERVICE_H__

// End of File

