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
* Description:    Session part of the client/server paradigm.*
*/




#ifndef __CCRSESSION_H
#define __CCRSESSION_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS
// None.

// MACROS
// None.

// DATA TYPES
enum TCRPanic
    {
    ECRPanicServiceHandle,
    ECRPanicBadDescriptor,
    ECRPanicInvalidRequestType,
    ECRPanicRequestAsyncTwice,
    ECRPanicBadSubSessionHandle,
    ECRPanicZeroLengthDes,
    ECRPanicDesLengthNegative,
    ECRPanicNullPtrArray,
    ECRPanicNullHandle,
    ECRPanicHandleNotOpen,
    ECRPanicIndexOutOfRange,
    ECRPanicHandleNotClosed,
    ECRPanicBadRequest,
    ECRPanicPanicBadName
    };

// FORWARD DECLARATIONS
class CCREngine;
class CCRServer;

// CLASS DECLARATION

/**
*  CCRSession is the server side session that client applications 
*  talk with. This merely passes commands to CCREngine
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRSession : public CSession2
    {

public: // Constructors and destructor
    
    /**
    * Two-phased constructor.
    * @Param aClient reference client thread
    * @Param aServer pointer to Server
    * @return CCRSession pointer to CCRSession class
    */
    static CCRSession* NewL( CCRServer* aServer );
    
    /**
    * Destructor.
    */
    virtual ~CCRSession();

public: // New methods   
    
    /**
    * Server.
    * @since Series 60 3.0
    * @return a pointer to server.
    */
    inline CCRServer* Server() const
        { 
        return REINTERPRET_CAST( CCRServer*, 
               CONST_CAST( CServer2*, CSession2::Server() ) );
        }

    /**
    * Service.
    * @since Series 60 3.0
    * @param aMessage contains data from the client.
    * @return None.
    */
    void ServiceL( const RMessage2& aMessage );
    
private: // New methods   

    /**
    * Dispatch message.
    * @since Series 60 3.0
    * @param aMessage contains data from the client.
    * @return None.
    */
    void DispatchMessageL( const RMessage2& aMessage );
    
    /**
    * New Object.
    * @since Series 60 3.0
    * @param aMessage contains data from the client.
    * @return None.
    */
    void NewObjectL( const RMessage2& aMessage );
    
    /**
    * Deletes object, can't fail - can panic client.
    * @since Series 60 3.0
    * @param aHandle handle.
    * @return None.
    */
    void DeleteObject( TUint aHandle );
    
    /**
    * Counts resources
    * @return Number of resources
    */
    TInt CountResources();

    /**
    * Panics client.
    * @since Series 60 3.0
    * @param aPanic panic code.
    * @return None.
    */
    void PanicClient( TInt aPanic ) const;
    
private: // Constructors and destructor

    /**
    * C++ default constructor.
    * @Param aClient reference client thread.
    */
    CCRSession();
    
    /**
    * Symbian 2nd phase constructor.
    * @Param aServer pointer to Server.
    */
    void ConstructL( CCRServer* aServer );

private: // Data
    
    /**
    * Object index for this session.
    */
    CObjectIx* iObjects;

    /**
    * Total number of resources allocated.
    */
    TInt iResourceCount;
    
    };

#endif // __CCRSESSION_H

//  End of File
