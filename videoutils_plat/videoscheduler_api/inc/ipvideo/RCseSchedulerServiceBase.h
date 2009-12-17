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
* Description:    Interface for Cse Scheduler engine Client's ServiceBase class.*
*/





#ifndef __RCSESCHEDULERSERVICEBASE_H
#define __RCSESCHEDULERSERVICEBASE_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS
// None.

// MACROS
// None.

// DATA TYPES
// None.

// FUNCTION PROTOTYPES
// None.

// FORWARD DECLARATIONS
class RCseSchedulerClient;

// CLASS DECLARATION

/**
*  Base class for services. This represents a client-side sub-session 
*  and has a corresponding sub-session object on the server-side.
*
*  @lib CseSchedulerClient.dll
*/
class RCseSchedulerServiceBase : public RSubSessionBase
    {
    public:
        /**
        * C++ default constructor.
        */
        RCseSchedulerServiceBase();

        /**
        * Destructor.
        * @return None.
        */
        virtual ~RCseSchedulerServiceBase();
        
        /**
        * Open server.
        * @param aServer 
        * @return KErrNone if successful, otherwise one of the system-wide error codes.
        */
        TInt Open( RCseSchedulerClient& aClient );

        /**
        * Close server.
        * @return None
        */
        void Close();

        /**
        * Uses user panic to panic client.
        * @param aFault Panic reason.
        * @return None.
        */
        void PanicClient( TInt aFault ) const;

        /**
        * Sessionhandle
        */
        inline RCseSchedulerClient& SessionHandle() const { return *iClient; }

    private:

        /**
        * Pointer to client. Own.
        */
        RCseSchedulerClient* iClient; 

    };

#endif // __RCSESCHEDULERSERVICEBASE_H

// End of File


