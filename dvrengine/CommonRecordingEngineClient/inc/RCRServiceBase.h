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
* Description:    Interface for CR Client's ServiceBase class.*
*/




#ifndef RCRSERVICEBASE_H
#define RCRSERVICEBASE_H

//  INCLUDES
#include <e32base.h>
#include <ipvideo/CRTypeDefs.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class RCRClient;

// CLASS DECLARATION

/**
*  Base class for services. This represents a client-side sub-session 
*  and has a corresponding sub-session object on the server-side.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( RCRServiceBase ) : public RSubSessionBase
    {

public: // Constructors and destructors

    /**
    * C++ default constructor.
    */
    RCRServiceBase::RCRServiceBase( );

    /**
    * Open server.
    * @since Series 60 3.0
    * @param aClient a reference to the client.
    * @return KErrNone if successful, otherwise one of the system-wide error codes.
    */
    TInt Open( RCRClient& aClient );

    /**
    * Close server.
    * @since Series 60 3.0
    * @param none.
    * @return None
    */
    void Close();

    /**
    * Uses user panic to panic client.
    * @since Series 60 3.0
    * @param aFault Panic reason.
    * @return None.
    */
    void PanicClient( TInt aFault );

    /**
    * Getter for sessionhandle.
    * @since Series 60 3.0
    * @param none.
    * @return None.
    */
    inline RCRClient& SessionHandle() { return *iClient; }

private: // Data

    /**
    * Pointer to client. Not owned.
    */
    RCRClient* iClient; 

    };

#endif // RCRSERVICEBASE_H

// End of File
