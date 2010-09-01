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
* Description:    Singleton to hold server session*
*/




#ifndef CCRSERVERHADLESINGLETON_H
#define CCRSERVERHADLESINGLETON_H

// INCLUDES
#include "RCRService.h"
#include "RCRClient.h"
#include <e32base.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRAPIBase;

// CLASS DECLARATION

/**
*  Singleton class to store client handles. Instance of this class is kept
*  in TLS and a reference count is maintained. This is to ensure that if a
*  process creates multiple CCRDvrApi instances, they will all share the
*  same session to CommonRecordingEngine.
*
*  @lib CommonRecordingEngineClient.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( CCRServerHandleSingleton ) : public CBase
    {

    /**
    * CCRApiBase and CCRDvrApi are the only classes 
    * that should have access to this class.
    */
    friend class CCRApiBase;
    friend class CCRDvrApi;

private: // Constructors and destructors

    /**
    * Private constructor to prevent direct instantiation. InstanceL must be used.
    */
    CCRServerHandleSingleton();

    /**
    * Second phase of 2-phased construction
    */
    void ConstructL();

    /**
    * Private destructor to prevent direct deletion. Release must be used.
    */
    ~CCRServerHandleSingleton();

private: // New methods

    /**
    * Returns the singleton instance. If it does not yet exist, it is created. 
    * If it does, a reference count is incremented
    * @since Series 60 3.0
    * @param none.
    * @return Instance to singleton class.
    */
    static CCRServerHandleSingleton* InstanceL();

    /**
    * Releases the singleton instance. If reference count reaches zero, the
    * instance is destroyed and client connection closed.
    */
    static void Release();  

    /**
    * Returns a reference to the service handle
    * @since Series 60 3.0
    * @param none.
    * @return Reference to RCRService
    */
    inline RCRService& Service() { return iService; }

private: // Data

    /**
    * Amount of references to this instance.
    * Instance will be deleted when this reaches zero.
    */
    TInt iReferenceCount;

    /**
    * Handle to Recording Engine client.
    */
    RCRClient iClient;

    /**
    * Handle to Recording Engine services.
    */
    RCRService iService;

    };

#endif // CCRSERVERHADLESINGLETON_H

//  End of File
