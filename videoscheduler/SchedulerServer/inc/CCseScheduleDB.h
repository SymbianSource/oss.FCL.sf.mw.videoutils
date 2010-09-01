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
* Description:    header file for CCseScheduleDB class*
*/



#ifndef _CCSESCHEDULEDB_H
#define _CCSESCHEDULEDB_H

#include <e32base.h>
#include <d32dbms.h>    // RDbStoreDatabase
#include <f32file.h>    // RFs
#include <s32file.h>
#include <babackup.h>

// CONSTANTS
const int KCseCustomSqlLength               = 256;
const int KCseDbCompactTimeout              = 30000000;

// Constant for filename definition
_LIT( KCseScheduleDBName, "CseSchedules.db" );
// Database table
_LIT( KCseDatabaseTable,                    "CseDatabase" );
_LIT( KCseDatabaseVersionCol,               "DbVersion" );
_LIT( KCseDatabaseReserved1Col,             "DbReserved1" );
_LIT( KCseDatabaseReserved2Col,             "DbReserved2" );

// Program table
_LIT( KCseScheduleTable, 					"ScheduleEvent" );
_LIT( KCseScheduleDbKeyCol, 				"Key" );
_LIT( KCseScheduleNameCol,					"Name" );
_LIT( KCseScheduleStartTimeCol,				"StartTime" );
_LIT( KCseScheduleEndTimeCol,				"EndTime" );
_LIT( KCseScheduleApplicationUIDCol, 		"ApplicationUID" );
_LIT( KCseSchedulePlugInUIDCol,    			"PlugInUID" );
_LIT( KCseScheduleTypeCol,					"Type" );
_LIT( KCseScheduleApplicationSpecificCol,	"ApplicationSpecificData" );
_LIT( KCseScheduleRunCountCol,              "ScheduleRunCount" );
_LIT( KCseScheduleStateCol,                 "ScheduleState" );
_LIT( KCseSchedulePluginTypeCol,            "PluginType" );
_LIT( KCseScheduleReserved1Col,             "ScheduleReserved1" );
_LIT( KCseScheduleReserved2Col,             "ScheduleReserved2" );

class CCseScheduledProgram;

/**  
* Version of CleanupXxxxPushL() that can be used to 'ResetAndDestroy'
* the supplied object if the clean-up stack unwinds.
*/
template <class T>
class CleanupResetAndDestroy
    {
    public:
        inline static void PushL( T& aRef );
    private:
        static void ResetAndDestroy( TAny *aPtr );
    };
    
template <class T>
    inline void CleanupResetAndDestroyPushL( T& aRef );

template <class T>
inline void CleanupResetAndDestroy<T>::PushL( T& aRef )
     {
     CleanupStack::PushL( TCleanupItem( &ResetAndDestroy, &aRef ) );
     }

template <class T>
void CleanupResetAndDestroy<T>::ResetAndDestroy( TAny *aPtr )
    {
    ( static_cast<T*>( aPtr ) )->ResetAndDestroy();
    }
    
template <class T>
inline void CleanupResetAndDestroyPushL( T& aRef )
    {
    CleanupResetAndDestroy<T>::PushL( aRef );
    }
    
    
/**
* Schedule database.
*/
class CCseScheduleDB : public CBase,
                       public MBackupObserver
{
	public:	// Constructors and destructors	
		virtual ~CCseScheduleDB();
		
		/**
        * Two-phased constructor.
        */
		IMPORT_C static CCseScheduleDB* NewL();
		
		/**
        * Enum for schedule state
        */
		enum TCseScheduleState
            {
            ECseWaiting = 0,
            ECseRunning            
            };
        
        /**
        * Enum for database access
        */    
        enum TCseDatabaseAccess
            {
            ECseDbOpen = 0,
            ECseDbLocked            
            };
            
    public: // Methods from MBackupObserver
        void ChangeFileLockL(const TDesC &aFileName, TFileLockFlags aFlags);
		
	public: // New methods
		/**
		* Adds given schedule to database. Method will leave with
		* KErrLocked -error code if Backup/Restore is going on.
		* @param	aData schedule to be added to database.
		* @return	None
		*/
		void AddScheduleL( CCseScheduledProgram& aData );
		
		/**
		* Removes schedule from database. Method will leave with
		* KErrLocked -error code if Backup/Restore is going on.
		* @param	aData schedule to be removed from database.
		* @return	None
		*/
		void RemoveScheduleL( const TUint32 aDbIdentifier );
		
		/**
		* Finds start time of the schedule that is happening next from the database.
		* Method will leave with KErrLocked -error code if Backup/Restore is going on.
		* @return	Start time of the next schedule.
		*/
		TTime GetNextScheduleTimeL();
		
		/**
		* Fills parameter array with schedules that have same starting time as
		* paramter time. Method will leave with KErrLocked -error code if
		* Backup/Restore is going on.
		* @param	aSchdeduleTime      Start time of the schedules fetched.
		* @param	aNextScheduleArray  Array containing schedules after call
		*                               is completed.
		* @return	None
		*/
		void FillScheduleArrayByTimeL( const TTime& aScheduleTime, 
			 		RPointerArray<CCseScheduledProgram>& aNextScheduleArray );
		
		/**
		* Gets the given schedule from database. Method will leave with
		* KErrLocked -error code if Backup/Restore is going on.
		* @param	aDbIdentifier       Database identifier of the schedule fetched.
		* @param	aSchedule           Pointer where schedule is stored when call
		*                               is completed.
		* @return	None
		*/	 						 
		void GetScheduleByDbIdentifierL( const TUint32 aDbIdentifier, 
                                         CCseScheduledProgram* aSchedule );
	
		/**
		* Get application specific schedules from db. Method will leave with
		* KErrLocked -error code if Backup/Restore is going on.
		* @param    aAppUid         Application UID.
		* @param    aScheduleArray  Array where to put scheduled events.
		* @return	None
		*/
		void GetApplicationSchedulesL( const TInt32 aAppUid, 
                                       RPointerArray<CCseScheduledProgram>& aArray );

		/**
		* Get overlapping schedules. Method will leave with
		* KErrLocked -error code if Backup/Restore is going on.
		* @param    aType           Schedule type (one from 
		* 		                    CCseScheduledProgram::TCseScheduleType)
		* @param    aStartTime      Start time
		* @param    aEndTime        End Time
		* @param    aResultArray    On return, contains pointers to overlapping schedules
		*               		    empty if none found.
		* @return   None.
		*/
		void GetOverlappingSchedulesL( const TInt32 aType, 
			                           const TTime& aStartTime,
			                           const TTime& aEndTime,
			                           RPointerArray<CCseScheduledProgram>& aResultArray );
			
		/**
		* Increases runcount of given schedule in DB. Method will leave with
		* KErrLocked -error code if Backup/Restore is going on.
		* @param    aDbIdentifier   Database identifier of the schedule thats
		*                           runcount is to be incremented		
		*/
		void IncreaseRunCountL( const TUint32 aDbIdentifier );
		
		/**
		* Gets the runcount of give schedule. Method will leave with
		* KErrLocked -error code if Backup/Restore is going on.
		* @param    aDbIdentifier   DbIdentifier of the schedule whos runcount is fetched.
		* @return   Run count of the schedule.
		*/
		TInt32 GetRunCountL( const TUint32 aDbIdentifier );
		
		/**
		* Gets the state of give schedule. Method will leave with
		* KErrLocked -error code if Backup/Restore is going on.
		* @param    aDbIdentifier   DbIdentifier of the schedule whos state is fetched.
		* @return   Schedule state (one of the CCseScheduledDB::TCseScheduleState)
		*/
		TInt32 GetScheduleStateL( const TUint32 aDbIdentifier );
		
		/**
		* Sets the state of give schedule. Method will leave with
		* KErrLocked -error code if Backup/Restore is going on.
		* @param    aDbIdentifier   DbIdentifier of the schedule whos state is fetched.
		* @param    aState          State to be set.
		* @return   None
		*/
		void SetScheduleStateL( const TUint32 aDbIdentifier,
                                const TInt32 aState );
        
        /**
		* Gets the startup schedules from the DB. When phone is set on, we try to run all
		* schdeulus that were on run when the phone was set off (run count is bigger than 0).
		* Method will leave with KErrLocked -error code if Backup/Restore is going on.
		* @param    aResultArray   Array where schedules that were on run are stored.
		* @return   None
		*/
        void StartupSchedulesL( RPointerArray<CCseScheduledProgram>& aResultArray );
        
        /**
		* Get plugin specific schedules from db. Method will leave with
		* KErrLocked -error code if Backup/Restore is going on.
		* @param    aPluginUid      Application UID.
		* @param    aScheduleArray  Array where to put scheduled events.
		* @return	None
		*/
        void GetSchedulesByPluginL( const TInt32 aPluginUid,
						            RPointerArray<CCseScheduledProgram>& aArray );

        /**
		* Get plugin specific schedules of specific type from db. Method will leave with
		* KErrLocked -error code if Backup/Restore is going on.
		* @param    aType           Schedule type
		* @param    aScheduleArray  Array where to put scheduled events.
		* @return	None
		*/
        void GetSchedulesByTypeL( const TInt32 aType,
						          RPointerArray<CCseScheduledProgram>& aArray );
						          
        /**
		* Get schedules in given timeframe. Method will leave with
		* KErrLocked -error code if Backup/Restore is going on.	
		* @param    aBeginning      Start time of timeframe
		* @param    aEnd            End Time of timeframe
		* @param    aResultArray    On return, contains pointers to overlapping schedules
		*               		    empty if none found.
		* @return   None.
		*/
		void GetSchedulesByTimeframeL( const TTime& aStartTime,
			                           const TTime& aEndTime,
			                           RPointerArray<CCseScheduledProgram>& aResultArray );
			                           
        /**
		* Gets all the uid's of different plugins that are in DB.
		* KErrLocked -error code if Backup/Restore is going on.	
		* @param    aPlugins    On return, contains all the different plugin uid's in
		*               		db. Empty if none found.
		*/
		void GetPluginsL( RArray<TInt32>& aPlugins );
		
		/**
		* Get the next happening time of schedule from database based on plugin uid.
		* KErrLocked -error code if Backup/Restore is going on.	
		* @param    aPluginUid  Plugin uid in which to base the search
	    * @return   Next start time of the schedule of given plugin Uid.
		*/
		TTime GetNextScheduleTimeByPluginL( TInt32 aPluginUid );
		
		/**
		* Gets all the schedules that are going to happen at the given time based on plugin uid.
		* KErrLocked -error code if Backup/Restore is going on.	
		* @param    aScheduleTime       Start time in which to base the search
		* @param    aPluginUid          Plugin uid in which to base the search
		* @param    aNextScheduleArray  On return, contains pointers to schedules.
		*/
		void GetSchedulesByTimeAndPluginL( 
		                         const TTime& aScheduleTime, 
                                 TInt32 aPluginUid,
					             RPointerArray<CCseScheduledProgram>& aNextScheduleArray );


	private:
	    /**
        * C++ default constructor.        
        */
		CCseScheduleDB();
	
		/**
        * By default Symbian 2nd phase constructor is private.
        */
		void ConstructL();

	private: // Methods        
        /**
        * Checks the db version from db. If it is older than current one, destroy it
        * and create new one.
        * @return	None
        */
        void CheckDbVersionL();
        
        /**        
        * Open DB or create new database if DB file is not found. 
        * @return	None
        */
        void OpenOrCreateDbL();
            	
    	/**
		* Create schedule table.
		* @param	aDatabase where to create schedule table.
		* @return	None
		*/
    	void CreateScheduleTableL( RDbStoreDatabase& aDatabase ) const;
    	
    	/**
 		* Create database table.
 		* @param	aDatabase where to create database table.
 		* @return	None
 		*/
     	void CreateDatabaseTableL( RDbStoreDatabase& aDatabase );
     	
     	/**
  		* Gets the database version number
  		* @param	None
  		* @return	None
  		*/
      	TInt GetDbVersionL(  );
      	
     	/**
  		* Set hardcoded database version to database. Used after creating new database.
  		* @param	None
  		* @return	None
  		*/
      	void SetDatabaseVersionL();

        /**
        * Removes existing database.
        * @return   None
        */
		void RemoveDbL();
		
		/**
        * Static call back for CPeriodic.
        * @param    aThis   Pointer to CCseScheduleDB so that we can
        *                   direct call back to class. 
        * @return   Always TFalse
        */
		static TInt CompactCallback( TAny* aThis );
		
		/**
        * We want to compact database 30 seconds after last database operation.
        * this resets 30 sec counter.
        * @return   None
        */
		void ResetCompactTimer( );
		
		/**
        * Compacts database.
        * @return   None
        */
		void CompactDb();
		
		/**
        * Closed Db file and sets iDbAccess to ECseDbLocked.
        * @return   None
        */
		void CloseDbFile();
		
		/**
        * Closed Db file and sets iDbAccess to ECseDbOpen.
        * @return   None
        */
		void OpenDbFileL();
		
		/**
        * Leaves with KErrLocked if iDbAccess is ECseDbLocked.
        * @return   None
        */
		void LeaveIfDbLockedL() const;

	private: // Data
		/**
		* File server session is connected in construction and closed in
		* destruction
		*/
    	RFs iFsSession;
    	
    	/**
    	* File name of the database which to handle
    	*/    	
		TFileName iDbFile;
		
		/**
		* For database operations
		*/
        RDbStoreDatabase iScheduleDb;
        
        /**
        * For creating and opening services database file. Own.
        */
        CFileStore* iScheduleFileStore;

        /**
        * Timer used in compacting database. Own.
        */
		CPeriodic* iCompactTimer;  
        
        /**
        * SQL string for SQL calls
        */
        TBuf<KCseCustomSqlLength> iSqlSchedule;
        
        /**
        * Status is database accessible
        */
        TCseDatabaseAccess iDbAccess;
        
        /**
        * Wrapper to get notification if backup/restore is
        * about to happen. Own.
        */
        CBaBackupSessionWrapper* iBackupWrapper;
};
#endif _CCSESCHEDULEDB_H