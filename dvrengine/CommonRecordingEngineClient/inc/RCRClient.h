/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    Interface for RC Client class*
*/




#ifndef RCRCLIENT_H
#define RCRCLIENT_H

// INCLUDES
#include <e32base.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION

/**
*  This is the client-side interface through which communication 
*  with the server is channelled.
*
*  @lib CommonRecordingEngineClient.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( RCRClient ) : public RSessionBase
    {

public: // Constructors and destructor

    /**
    * C++ default constructor.
    * @param none.
    * @return none.
    */
    RCRClient();

public: // New methods

    /**
    * Connects to the  server.
    * @since Series 60 3.0
    * @param none.
    * @return KErrNone if successful,
    *         otherwise another of the system-wide error code.
    */
    TInt Connect();

    /**
    * Closes the session.
    * @since Series 60 3.0
    * @param none.
    * @return None
    */
    void Close();

    /**
    * Version.
    * @since Series 60 3.0
    * @param none.
    * @return version number.
    */
    TVersion Version() const;

    };

#endif // RCRCLIENT_H

// End of File
