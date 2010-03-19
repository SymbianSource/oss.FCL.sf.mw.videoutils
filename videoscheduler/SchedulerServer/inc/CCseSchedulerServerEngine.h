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
* Description:    Interface for Scheduler Server's Engine class*
*/




#ifndef __CCSESCHEDULERSERVERENGINE_H
#define __CCSESCHEDULERSERVERENGINE_H

//  INCLUDES
#include <ipvideo/mcsescheduleobserver.h>   // Observer so that CCseSchedulerTimer can inform engine.
#include "MCsePluginControllerObserver.h"   // Observer so that CCseSchedulerTimer can inform engine.
#include "CseSchedulerServer.pan"   // Panic codes


// CONSTANTS
// Usually we have only one scheduled event at a time so we set granularity to 1, but
// in some rare cases we migth have more so we have to use array.
const int KCseScheduleGranularity      		= 1;

// MACROS
// None

// DATA TYPES
// None

// FUNCTION PROTOTYPES
// None

// FORWARD DECLARATIONS
class CCseScheduleDB;
class CCseSchedulerTimer;
class CCseScheduledProgram;
class CCseSchedulerServerSession;
class CCseSchedulerServer;
class CCseSchedulerPluginControllerBase;


// CLASS DECLARATION
// None

/**
* Server's "brain". Controls all subsession messages to current object for 
* futher prosessing. Manages message list thru the reguest handler. Reads from 
* the client address space and writes back to client address space. Controls 
* sub objet request(s).
*/
class CCseSchedulerServerEngine : public CObject,
								  public MCsePluginControllerObserver
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param Reference to scheduler server
        * @return CCseSchedulerEngine pointer to CCseSchedulerEngine class
        */
        static CCseSchedulerServerEngine* NewL( CCseSchedulerServer& aServer );

        /**
        * Destructor.
        */
        virtual ~CCseSchedulerServerEngine();
        
    public: // From MCsePluginControllerObserver        
    	
    	void ScheduleCompletedSuccesfullyL( const TUint32 aDbIdentifier );
    	    	
    	void ScheduleCompletedWithErrorL( const TUint32 aDbIdentifier, TUint32 aCompletionCode );
    	    	
    	void IncreaseScheduleRunCountL( const TUint32 aDbIdentifier );
    	    	
    	TBool IsAllowedToRun( TUint32 aDbIdentifier );
    	    	
        void ScheduleNotValid( const TUint32 aDbIdentifier );
                
        void RequestReschedule( );
                
        void ClearingDone();
   
   public: // New methods     
        /**
		* Plugin complete callback used with CIdle. Used to break call stack.
		* @param    aPtr    Pointer back to CCseSchedulerUserInformer-class
		* @return   Always EFalse.
		*/
		static TInt CompletitionCallback( TAny* aPtr );

		/**
		* Schedule complete callback handler. Removes schedule from "on run list",
		* informs engine that schedule has comlete and checks if the plugin where
		* schedule was run still needed.
		* @return   Always EFalse
		*/
		TInt HandleCompletitionCallback();
		
		/**
         * Cleaner callback used with CIdle. Used to break call stack.         
         * @param    aPtr    Pointer back to CCseSchedulerUserInformer-class
         * @return   Always EFalse.
         */
		static TInt CleanerCallback( TAny* aPtr );
	
		/**
		 * Cleaner callback handler. Goes through all plugin contollers and clears them
		 * (shutdowns all threads running plugins).		 
		 * @return   Always EFalse		 
		 */
		TInt HandleCleanerCallback();

    public: // From CObject
        /**
        * Overwrites CObjects Close. Sets the AccesCount to zero.        
        * @return   None
        */
        void Close();        

    public: // New functions
    
        /**
        * Does the general message handling.
        * @param aMessage contains data from the client.
        * @return None.
        */
        void GeneralServiceL( const RMessage2& aMessage );

        /**
        * Service cancellation.
        * @param aMessage contains data from the client.
        * @return None.
        */
        void CancelService( const RMessage2& aMessage );       
        
        /**
        * Checks if scheduler is active atm (schedules in run or waitint)        
        * @return ETrue if active and EFalse if not.
        */
        TBool IsSchedulerActive() const;
        
        /**
        * Reschedules engine (get new "next schedule" from database and
        * resets timer).       
        * @return   None.
        */
		void RescheduleL();
		
		/**
        * Reschedules engine (get new "next schedule" from database and
        * resets timer).       
        * @return   None.
        */
		void RescheduleByPluginL( TInt32 aPluginUid );
		
    private: // Constructors and destructor

        /**
        * C++ default constructor.
        * @param Reference to scheduler server.
        */
        CCseSchedulerServerEngine( CCseSchedulerServer& aServer );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();                
    

    private: // New functions  
        /**
        * Panic the client on server side.        
        * @param    aPanic      Our panic code.
        * @param    aMessage    A message from client
        * @return   None.
        */
        void PanicClient( TCseSchedulerServPanic aPanic, 
                          const RMessage2& aMessage ) const;
        
        /**
        * Adds schedule to database
        * @param    aMessage    A message from client
        * @return   None.
        */                  
        void AddScheduleL( const RMessage2& aMessage );
        
        /**
        * Removes schedule from database
        * @param    aMessage    A message from client
        * @return   None.
        */        
        void RemoveScheduleL( const RMessage2& aMessage );
        
        /**
        * Gets all schedules from database
        * @param    aMessage    A message from client
        * @return   None.
        */
        void GetSchedulesL( const RMessage2& aMessage );
        
        /**
        * Adds schedule to database
        * @param    aMessage    A message from client
        * @return   None.
        */
        void GetScheduleL( const RMessage2& aMessage );
        
        /**
        * Gets externalize length of one specific schedule
        * @param    aMessage    A message from client
        * @return   None.
        */
        void GetScheduleLengthL( const RMessage2& aMessage );
        
        /**
        * Gets schedules based on application UID.
        * @param    aMessage    A message from client
        * @return   None.
        */
        void GetSchedulesByAppUidL( const RMessage2& aMessage );
	    
	    /**
        * Gets externalize length of the schedules of one application UID
        * @param    aMessage    A message from client
        * @return   None.
        */
		void GetScheduleArrayLengthByAppUidL( const RMessage2& aMessage );

		/**
        * Gets externalize length of the overlapping schedules
        * @param    aMessage    A message from client
        * @return   None.
        */
		void GetOverlappingSchedulesLengthL( const RMessage2& aMessage );

	    /**
        * Gets overlapping schedules of given timeframe
        * @param    aMessage    A message from client
        * @return   None.
        */
		void GetOverlappingSchedulesL( const RMessage2& aMessage );
		
		/**
        * Gets startup schedules from database (schedules that were on run when
        * phone was shut down)
        * @return None.
        */
		void EngineSchedulesStartup();
		
		/**
        * Gets externalize length of the schedules of one plugin UID
        * @param    aMessage    A message from client
        * @return   None.
        */
        void GetScheduleArrayLengthByPluginUidL( const RMessage2& aMessage );
        
        /**
        * Gets schedules based on plugin UID.
        * @param    aMessage    A message from client
        * @return   None.
        */
        void GetSchedulesByPluginUidL( const RMessage2& aMessage );
    
        /**
        * Gets externalize length of the schedules of one type.
        * @param    aMessage    A message from client
        * @return   None.
        */
        void GetScheduleArrayLengthByTypeL( const RMessage2& aMessage );

        /**
        * Gets schedules based on schedule type.
        * @param    aMessage    A message from client
        * @return   None.
        */
        void GetSchedulesByTypeL( const RMessage2& aMessage );

        /**
        * Gets externalize length of the schedules of specific timeframe.
        * @param    aMessage    A message from client
        * @return   None.
        */
        void GetScheduleArrayLengthByTimeframeL( const RMessage2& aMessage );
    
        /**
        * Gets schedules between given timeframe.
        * @param    aMessage    A message from client
        * @return   None.
        */
        void GetSchedulesByTimeframeL( const RMessage2& aMessage );
		
    private:    // Data
        /**
        * DB class. Own.
        */
        CCseScheduleDB* iDb;

        /**
		* Array holding all plugin controllers.
		*/
        RPointerArray<CCseSchedulerPluginControllerBase> iPluginControllerArray;
        
        /**
        * CIdle, used to break call stack when schedule completes. Own.
        */ 
        CIdle* iCompletitionBreaker;
        
        /**
        * Reference to scheduler server
        */ 
        CCseSchedulerServer& iServer;
    };

#endif // __CCSESCHEDULERSERVERENGINE_H

// End of file

