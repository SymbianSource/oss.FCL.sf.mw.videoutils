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
* Description:    Header for Scheduler Server class*
*/




#ifndef __CCSESCHEDULERSERVER_H
#define __CCSESCHEDULERSERVER_H

//  INCLUDES
#include <e32base.h>
#include "CseSchedulerServer.pan"   // Panic codes

// CONSTANTS
#define KCseSchedulerMinHeapSize 2*KMinHeapSize
#define KCseSchedulerMaxHeapSize 0x1F0000 

// MACROS
// None

// DATA TYPES
// None

// FUNCTION PROTOTYPES
// None

// FORWARD DECLARATIONS
class CCseSchedulerServerEngine;

// CLASS DECLARATION
/**
*  Server class
*
*  @lib CseSchedulerServer.exe
*/
class CCseSchedulerServer : public CPolicyServer
    {
    public: // Constructors and destructor
        
        /**
        * Two-phased constructor.        
        */
         static CCseSchedulerServer* NewLC( );
        
        /**
        * Destructor.
        */
        virtual ~CCseSchedulerServer();
    
    public: // New functions

        /**
        * Thread function, creates cleanup stack for the thread.
        * @return KErrNone
        */
        static TInt ThreadFunction( );
        
        /**
        * Thread function, the active scheduler is installed and started.
        * @return None.
        */
        static void ThreadFunctionL( );

        /**
        * Panics server.
        * @param aPanic panic code.
        * @return None.
        */
        static void PanicServer( TCseSchedulerServPanic aPanic );
        
        /**
        * Starts the server thread.
        * @return None.
        */
        static TInt StartThread();
        
        /**
        * Dec
        * @return None.
        */
        void Dec();
        
        /**
        * Inc
        * @return None.
        */
        void Inc();
        
        /**
        * Create new Session.
        * @param aVersion server version number.
        * @return CSharableSession pointer to new created session.
        */
        CSession2* NewSessionL( const TVersion &aVersion,
                                const RMessage2& aMessage ) const;

        /**
        * Gets engine object.
        * @return CCseSchedulerServerEngine pointer.
        */
        CCseSchedulerServerEngine* GetEngineObjectL();

		/**
		* Makes check if server is still needed (has schedules on run or active
		* client), if not calls ActiveScheduler::Stop resulting server shutdown.
		*/
		void StopServer();

    protected: // Functions from base classes

        /**
        * From CPolicyServer, called by framework to perform 
        * custom security check for any client messages.
        * 
        * @param aMsg     Message.
        * @param aAction  Action.
        * @param aMissing Security info.
        * @return Result of security check.
        */
        TCustomResult CustomSecurityCheckL( const RMessage2 &aMsg, TInt &aAction, TSecurityInfo &aMissing );

	    /**
		* Starter callback used with CIdle. Used to break call stack.
		* @param    aPtr    Pointer back to CCseSchedulerUserInformer-class
		* @return   Always EFalse.
		*/
		static TInt StarterCallback( TAny* aPtr );
	
		/**
		* Starter callback handler. Checks if server is actually needed (there
		* is client or schedules to be run). If engine is not needed, signals
		* server to be deleted.
		* @return   Always EFalse
		*/
		TInt HandleStarterCallback();
				
   
    private: // Constructors and destructor 
        
        /**
        * C++ default constructor.
        */
        CCseSchedulerServer();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( );
    
    private: // New functions

        /**
        * Deletes scheduler engine.
        * @return None.
        */
        void DeleteSchedulerEngine();

        
    private: // Data    
        /**
        * Object container index. Own.
        */
        CObjectConIx* iContainerIx;

        /**
        * Object container. Own.
        */
        CObjectCon* iObjectCon;
        
        /**
        * Number of sessions
        */
        TInt iSessionCount;

        /**
        * Engine object. Own.
        */
        CCseSchedulerServerEngine* iSchedulerEngine;       
        
        /**
        * CIdle, used to break call stack when scheduler is started. Own.
        */ 
        CPeriodic* iStarterBreaker;
    };

#endif // __CCSESCHEDULERSERVER_H

// End of File