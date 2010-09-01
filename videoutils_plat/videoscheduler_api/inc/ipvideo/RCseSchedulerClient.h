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
* Description:    Interface for Cse Scheduler engine Client class*
*/





#ifndef _RCSESCHEDULERCLIENT_H
#define _RCSESCHEDULERCLIENT_H

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
// None.

// CLASS DECLARATION

/**
*  This is the client-side interface through which communication 
*  with the server is channeled.
*
*  @lib CseSchedulerClient.dll
*/
class RCseSchedulerClient : public RSessionBase
    {
    public: // Constructors and destructor

        /**
        * C++ default constructor.
        * @return None
        */
        RCseSchedulerClient();

        /**
        * Destructor.
        * @return None.
        */
        virtual ~RCseSchedulerClient();

    public: // New functions

        /**
        * Connect to the  server.
        * @return KErrNone if successful, otherwise another of the system-wide 
        *         error codes.
        */
        TInt Connect();

        /**
        * Close the session.
        * @return None
        */
        void Close();

        /**
        * Version
        * @return version number
        */
        TVersion Version() const;

    private: // Data

    };

#endif // _RCSESCHEDULERCLIENT_H

// End of File


