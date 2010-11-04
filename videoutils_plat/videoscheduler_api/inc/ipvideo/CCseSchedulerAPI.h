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
* Description: 
*
*/



#ifndef _CCSESCHEDULERAPI_H
#define _CCSESCHEDULERAPI_H

// INCLUDES
#include <e32base.h>
#include <ipvideo/RCseSchedulerClient.h>    // Client common methods (server start up etc)
#include <ipvideo/RCseSchedulerService.h>   // Client common methods (open, close etc)

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
class CCseSchedulerApi : public CBase                          
    {
    public: // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        IMPORT_C static CCseSchedulerApi* NewL();

        /**
        * Destructor.
        * @return None.
        */
        IMPORT_C virtual ~CCseSchedulerApi();

    private: // Constructors and destructor

        /**
        * C++ default constructor.
        */
        CCseSchedulerApi();

        /**
        * Symbian 2nd phase constructor can leave.
        */
        void ConstructL();
        
    public: // New methods
    	/**
        * Adds given schedule to Common Scheduling Engine database. After the call is
        * completed, the parameter contains the DB identifier of the schedule.
        * @param    aData   Schedule to be added to database.
        * @return   Generic symbian error code.
        */
        IMPORT_C TInt AddSchedule( CCseScheduledProgram& aData ) const;
        
        /**
        * Removes schedule from database.
        * @param    aDbIdentifier   Database identifier of the schedule to be removed.
        * @return   Generic symbian error code.
        */
        IMPORT_C TInt RemoveSchedule( const TUint32 aDbIdentifier ) const;
        
        /**
        * Gets schedule from database.
        * @param    aDbIdentifier   Database identifier of the schedule that is wanted
        *                           from the database
        * @param    aProg           Pointer where this scheduled program is get.
        * @return   Generic symbian error codes.
        */
        IMPORT_C TInt GetSchedule( const TUint32 aDbIdentifier,
        						   CCseScheduledProgram* aProg ) const;

		/**
		* Get list of scheduled events from the scheduling engine based on given UID.
		* @param    aAppUid     Application UID.
		* @param    aArray      On return contains items from the database.
		* @return   Generic symbian error code.
		*/
		IMPORT_C TInt GetSchedulesByAppUid( const TInt32 aAppUid, 
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
		IMPORT_C TInt GetOverlappingSchedules( CCseScheduledProgram& aProgram, 
		                                       RPointerArray<CCseScheduledProgram>& aResultArray );
		
		/**
		* Get list of scheduled events from the scheduling engine based on given UID.
		* @param    aPluginUid     Plugin UID.
		* @param    aArray      On return contains items from the database.
		* @return   Generic symbian error code.
		*/                                       
		IMPORT_C TInt GetSchedulesByPluginUid( const TInt32 aPluginUid,
                                      RPointerArray<CCseScheduledProgram>& aArray ) const;

        /**
		* Get list of scheduled events from the scheduling engine based on given schedule type.
		* @param    aScheduleType   Schedule type.
		* @param    aArray          On return contains items from the database.
		* @return   Generic symbian error code.
		*/                                      
		IMPORT_C TInt GetSchedulesByType( const TInt32 aScheduleType, 
                                 RPointerArray<CCseScheduledProgram>& aArray ) const;

        /**
		* Get list of scheduled events from the scheduling engine between given timeframe.
		* @param    aBeginning  Beginning of the time frame
		* @param    aEnd        End of the time frame
		* @return   Generic symbian error code.
		*/                                                                               
		IMPORT_C TInt GetSchedulesByTime( const TTime& aBeginning,
                                 const TTime& aEnd,
                                 RPointerArray<CCseScheduledProgram>& aArray ) const;
                                         

    private: // Data
        /**
        * Scheduler engine - Client
        */
        RCseSchedulerClient iClient;
        
        /**
        * Scheduler engine - Service
        */
        RCseSchedulerService iService;
    };

#endif //_CCSESCHEDULERAPI_H

// End of file
