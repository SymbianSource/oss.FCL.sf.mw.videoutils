/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    Interface for Common recording engine, common part*
*/




#ifndef CCRAPIBASE_H
#define CCRAPIBASE_H

// INCLUDES
#include <e32base.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRServerHandleSingleton;

// CLASS DECLARATION

/**
*  General client api base class that all API classes in 
*  common recording engine API implementations need to inherit.
*
*  @lib CommonRecordingEngineClient.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( CCRApiBase ) : public CBase
    {

protected: // Constructors and destructor

    /**
    * C++ default constructor.
    */
    CCRApiBase();

    /**
    * Destructor.
    * @return None.
    */
    virtual ~CCRApiBase();

    /**
    * Symbian 2nd phase constructor can leave.
    */
    void BaseConstructL();

protected: // data

    CCRServerHandleSingleton* iSingleton;
    
    };

#endif // CCRAPIBASE_H

// End of File
