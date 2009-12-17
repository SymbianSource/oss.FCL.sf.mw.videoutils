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
* Description:    Header file for CCseSchedulerThreadPacket-class.*
*/



#ifndef _CCSESCHEDULERTHREADPACKET_H
#define _CCSESCHEDULERTHREADPACKET_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS
// None
// MACROS
// None

// DATA TYPES
// None

// FUNCTION PROTOTYPES
// None

// FORWARD DECLARATIONS
class CCseScheduledProgram;

/**
* Class holding CCseScheduledProgram and ThreadResultCode. This is given to
* separate plugin thread and should not be accessed before thread dies.
*/
class CCseSchedulerThreadPacket : public CBase
{
	public:	// Constructors and destructors
		/**
        * Destructor.
        */
		virtual ~CCseSchedulerThreadPacket();
		
		/**
        * Two-phased constructor.
        * @param    aObserver   A pointer back to server engine through observer
        */
		static CCseSchedulerThreadPacket* NewL(  );

	private: // Constructors and destructors
	 	/**
        * By default Symbian 2nd phase constructor is private.
        */
		void ConstructL();
		
		/**
        * C++ default constructor.
		* @param    aObserver   A pointer back to server engine through observer.
        */
		CCseSchedulerThreadPacket(  );	
		
    public: // New methods
        /**
        * Sets schedule to thread packet. Overwrites existing one.
        * @param    aData   Scheduled program to be set        
        */
        void SetScheduleL( CCseScheduledProgram& aData );
        
        /**
        * Gets schedule from thread packet.
        * @return   Pointer to scheduled program.
        */
        CCseScheduledProgram* Schedule( );
        
        /**
        * Sets threads result code. Should be set before thread completes.
        * Defaults to KErrGeneral.
        * @param    aResultCode   New result code. Overwrites existing one.
        */
        void SetResultCode( TInt32 aResultCode);
        
        /**
        * Gets threads result code. 
        * @return Threads result code. Defaulted to KErrGeneral if not set.
        */
        TInt32 ResultCode( );
        
        /**
        * Sets semaphore used to signal plugin shutdown request.
        * @param    aSemaphore   Semaphore used to signal shutdown
        */
        void SetShutdownSemaphore( RSemaphore& aSemaphore );
        
        /**
        * Sets semaphore used to signal when plugin shutdown has completed.
        * @param    aSemaphore   Semaphore used to signal shutdown
        */
        void SetShutdownCompleteSemaphore( RSemaphore& aSemaphore );
        
        /**
        * Gets semaphore used to signal plugin shutdown request.
        * @return RSemaphore Semaphore used to signal plugin shutdown request.
        */
        RSemaphore& ShutdownSemaphore();
        
        /**
        * Gets semaphore used to signal when plugin shutdown has completed.
        * @return RSemaphore Semaphore used to signal when plugin shutdown has completed.
        */
        RSemaphore& ShutdownCompleteSemaphore();
		
	private: // Data
	    /**
	    * Scheduled program to be run in the plugin thread. Own.
	    */
		CCseScheduledProgram* iSchedule;
		
		/**
	    * Plugin completition code after the thread dies.
	    */
		TInt32                iResultCode;
		
		/**
	    * Semaphore signalled when plugin shutdown is wanted while plugin is running.
	    */
		RSemaphore            iShutdownSemaphore;
		
		/**
	    * Semaphore signalled when plugin shutdown is completed.
	    */
		RSemaphore            iShutdownCompleteSemaphore;
};

#endif //_CCSESCHEDULERTHREADPACKET_H