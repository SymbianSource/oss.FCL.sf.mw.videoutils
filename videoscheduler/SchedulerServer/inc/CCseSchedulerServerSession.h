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
* Description:    Interface for Scheduler Server's Session class*
*/




#ifndef __CCSESCHEDULERSERVERSESSION_H
#define __CCSESCHEDULERSERVERSESSION_H

//  INCLUDES
#include <e32base.h>
#include "CseSchedulerServer.pan"   // Panic codes

// CONSTANTS
// None.

// MACROS
// None.

// DATA TYPES
// None
        
// FUNCTION PROTOTYPES
// None.

// FORWARD DECLARATIONS
class CCseSchedulerEngine;
class CCseSchedulerServer;

// CLASS DECLARATION

/**
*  CCseSchedulerSession
*
*  @lib
*/
class CCseSchedulerServerSession : public CSession2
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @Param aClient reference client thread
        * @Param aServer pointer to Server
        * @return CCseSchedulerSession pointer to CCseSchedulerSession class
        */
        static CCseSchedulerServerSession* NewL( CCseSchedulerServer* aServer );

        
        /**
        * Destructor.
        */
        virtual ~CCseSchedulerServerSession();

    public: // New functions   
        
        /**
        * Server
        * @return a pointer to server
        */
        CCseSchedulerServer* Server();
            
        /**
        * Service
        * @param aMessage contains data from the client.
        * @return None
        */
        void ServiceL( const RMessage2& aMessage );
        
    private: // New functions   

        /**
        * Dispatch message
        * @param aMessage contains data from the client.
        * @return None
        */
        void DispatchMessageL( const RMessage2& aMessage );
        
        /**
        * New Object
        * @param aMessage contains data from the client.
        * @return None
        */
        void NewObjectL( const RMessage2& aMessage );
        
        /**
        * Deletes object, can't fail - can panic client
        * @param aHandle handle
        * @return None
        */
        void DeleteObject( TUint aHandle );
        
        /**
        * Counts resources
        * @return Number of resources
        */
        TInt CountResources();

        /**
        * Panics client
        * @param aPanic panic code
        * @return None
        */
        void PanicClient( TCseSchedulerServPanic aPanic ) const;
        
    private:  // Constructors and destructor

        /**
        * C++ default constructor.
        * @Param aClient reference client thread
        */
        CCseSchedulerServerSession();

        /**
        * Symbian 2nd phase constructor.
        * @Param aServer pointer to Server
        */
        void ConstructL( CCseSchedulerServer* aServer );

    private: // Data
        
        /**
        * Object index for this session. Own.
        */
        CObjectIx* iObjects;

        /**
        * Total number of resources allocated
        */
        TInt iResourceCount;
        
    };

#endif // __CCSESCHEDULERSERVERSESSION_H

