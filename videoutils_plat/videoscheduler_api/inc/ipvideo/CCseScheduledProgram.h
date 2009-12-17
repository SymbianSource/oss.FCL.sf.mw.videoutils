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
* Description:    This class contains data for scheduled programs.*
*/


#ifndef __CCSESCHEDULEDPROGRAM_H__
#define __CCSESCHEDULEDPROGRAM_H__

#include <e32def.h>
#include <e32base.h>
#include <s32strm.h>

// CONSTANTS
const TInt KCseNameMaxLength = 255;  // Based on SymbianDB column max length

/**
* Scheduled program data.
*/
class CCseScheduledProgram : public CBase
	{
	public: // Constructors and destructors
    	/**
        * Two-phased constructor.
        */            
		IMPORT_C static CCseScheduledProgram* NewL();
		
		/**
        * Two-phased copy constructor.
        */
		IMPORT_C static CCseScheduledProgram* NewL( CCseScheduledProgram& aProg );
		
		/**
        * Destructor.
        * @return None.
        */
		IMPORT_C virtual ~CCseScheduledProgram();
		
		
		// Enums
		// Schedule type
        enum TCseScheduleType
            {
            ECseReminder = 0,
            ECseRecording,
            ECseProgramGuideUpdate,
            ECseScheduleDownload,
            ECseOther
            };
        
        // Plugin type:	   
	    enum TCsePluginType
            {
            // Only one of this kind of plugin can be running at same time
            ECseUniPlugin = 1,
            // Several of this kind of plugin can be running same time
            ECseMultiPlugin            
            };
    public: // New methods
        /**
        * Externalizes class content to stream.
        * @param aStream Writestream where to write content
        * @return None.
        */
		IMPORT_C void ExternalizeL( RWriteStream& aStream );
		
		/**
        * Internalizes class content from stream.
        * @param aStream Readstream from where to read content
        * @return None.
        */ 
		IMPORT_C void InternalizeL( RReadStream& aStream );
		
		/**
        * Calculates externalization length.
        * @return Length of the stream needed when class is externalized.
        */
		IMPORT_C TInt ExternalizeLength();
		
	public:	// Setters
	
	    /**
        * Sets new plugin type for schedule.
        * EUniPlugin for plugins that can have only one running at a given time.
        * EMultiPlugin for plugins that can have several running at a given time
        * @param aPluginType New plugin type
        */
		IMPORT_C void SetPluginType( TInt32 aPluginType );		
		
		/**
        * Sets new schedule name.
        * @param aName New name of the schedule
        */
		IMPORT_C void SetName( const TDesC8& aName );		
		
		/**
        * Sets new schedule start time.
        * @param aStartTime New start time of the schedule
        */
		IMPORT_C void SetStartTime( const TTime& aStartTime );
		
		/**
        * Sets new schedule end time.
        * @param aEndTime New end time of the schedule
        */
		IMPORT_C void SetEndTime( const TTime& aEndTime );		
		
		/**
        * Sets new application uid for schedule.
        * @param aAppUid New application uid of the schedule
        */
		IMPORT_C void SetAppUid( const TInt32 aAppUid );
		
		/**
        * Sets new plugin uid for schedule
        * @param aPluginUid New plugin uid of the schedule
        */
		IMPORT_C void SetPluginUid( const TInt32 aPluginUid );
		
		/**
        * Sets new application data for schedule
        * @param aAppData New application data of the schedule
        */
		IMPORT_C void SetApplicationDataL( const TDesC8& aAppData );
		
		/**
        * Sets new database identifier for the schedule. Note that this should only
        * be set by Common Scheduling Engine, not the application using it.
        * @param aDbIdentifier New database identifier of the schedule
        */
		IMPORT_C void SetDbIdentifier( const TUint32 aDbIdentifier );
				
		/**
        * Sets new schedule for the schedule.
        * @param aScheduleType New schedule type of the schedule
        */
		IMPORT_C void SetScheduleType( const TInt32 aScheduleType );

		// Getters
		/**
        * Get the type of scheduler plugin
        * EUniPlugin for plugins that can have only one running at a given time.
        * EMultiPlugin for plugins that can have several running at a given time
        * @return Schedule plugin type.
        */
		IMPORT_C TInt32 PluginType() const;
		
		/**
        * Get the name of the schedule.
        * @return Schedule name.
        */
		IMPORT_C TPtrC8 Name() const;
		
		/**
        * Gets the start time of the schedule.
        * @return Start time.
        */
		IMPORT_C TTime& StartTime();
		
		/**
        * Gets the end time of the schedule.
        * @return End time.
        */
		IMPORT_C TTime& EndTime();		
		
		/**
        * Gets the application uid of the schedule
        * @return Application uid.
        */
		IMPORT_C TInt32 AppUid() const;
		
		/**
        * Gets the plugin uid of the schedule.
        * @return Plugin uid.
        */
		IMPORT_C TInt32 PluginUid() const;
		
		/**
        * Gets the application data of the schedule.
        * @return Application data.
        */
		IMPORT_C TPtrC8 ApplicationData();
		
		/**
        * Gets the database identifier of the schedule.
        * @return Database identifier
        */
		IMPORT_C TUint32 DbIdentifier() const;
				
		/**
        * Gets the schedule type of the schedule.
        * @return Schedule type
        */
		IMPORT_C TInt32 ScheduleType() const;

        /**
        * Dumps content of schedule to debug trace
        * @return   None
        */
        IMPORT_C void DebugDump() const;
	
	protected: // Constructors
		/**
		*	Default constructor
		*/
		EXPORT_C CCseScheduledProgram();
		
		/**
        * Symbian 2nd phase constructor can leave.
        */
		void ConstructL();

	protected: // Data			
		/**
		* Name
		* Name of the scheduled program, e.g. name of the TV program.		
		*/
		TBuf8<KCseNameMaxLength>	iName;

		/**
		* Start time
		* Start time when the scheduled program is run (passed to plugin which is defined here).
		*/
		TTime					iStartTime;	

		/**
		* End time
		* End time for schedules that take longer time to complete, for example recording of the TV program.
		* If schedule is "instant" (e.g. just notification to user) start time and end time should be the same.
		*/
		TTime					iEndTime;
		
		/**
		* Application UID
		* This is the application uid of the program that has added this schedule. Can be used to retrieve
		* all schedules of certain application.
		*/
		TInt32					iAppUid;

		/**
		* Plugin UID
		* UID of the plugin where this scheduled program is passed when the time is right.
		*/
		TInt32					iPluginUid;
		
		/**
		* Application Data
		* Application specific data for the plugin. Common scheduling Engine has no knowledge what
		* this data contains.
		*/
		HBufC8*					iAppData;
		
		/**
		* DB identifier
		* Used to separate schedules from each other in database. Should be only added by Common
		* scheduling engine, not by application using it.
		*/
		TUint32					iDbIdentifier;
		
		/**
		* Schedule type
		* Type of this schedule, e.g. reminder, recording...
		*/		
		TInt32                  iScheduleType;
		
		/**
		* Schedule plugin type
		* Type of the plugin where this schedule will be run
		*/		
		TInt32                  iPluginType;
	};
	
#endif //__CCSESCHEDULEDPROGRAM_H__

// End of file
