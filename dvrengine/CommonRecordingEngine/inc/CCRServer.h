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
* Description:    Server class in client/server paradigm of symbian.*
*/




#ifndef __CCRSERVER_H
#define __CCRSERVER_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES

// FORWARD DECLARATIONS
class CCREngine;

// CLASS DECLARATION

/**
*  Server class.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRServer : public CPolicyServer
    {

public: // Constructors and destructor
    
    /**
    * Two-phased constructor.
    * @param aSemaphore a void pointer to semaphore.
    */
    static CCRServer* NewLC();
    
    /**
    * Destructor.
    */
    virtual ~CCRServer();

public: // New functions

    /**
    * Thread function, creates cleanup stack for the thread.
    * @since Series 60 3.0    
    * @return KErrNone
    */    
    static TInt ThreadFunction( );
    
    /**
    * Thread function, the active scheduler is installed and started.
    * @since Series 60 3.0
    * @return None.
    */    
    static void ThreadFunctionL( );
    
    /**
    * Signals client that server is started.
    * @since Series 60 3.0
    * @return None.
    */
    static void SignalClientL();
    
    /**
    * Panics server.
    * @since Series 60 3.0
    * @param aPanic panic code.
    * @return None.
    */
    static void PanicServer( TInt aPanic );
    
    /**
    * Starts the server thread.
    * @since Series 60 3.0
    * @return None.
    */
    static TInt StartThread();
    
    /**
    * Dec
    * @since Series 60 3.0
    * @return None.
    */
    void Dec();
    
    /**
    * Inc
    * @since Series 60 3.0
    * @return None.
    */
    void Inc();
    
    /**
    * Create new Session.
    * @since Series 60 3.0
    * @param aVersion server version number.
    * @return CSharableSession pointer to new created session.
    */
    CSession2* NewSessionL( const TVersion &aVersion,
                            const RMessage2& aMessage ) const;

    /**
    * Gets engine object.
    * @since Series 60 3.0
    * @return CCREngine pointer.
    */
    CCREngine* GetEngineObjectL();

protected:

// From CPolicyServer

    /**
     * Called by framework to perform custom security check for any
     * client messages.
     *
     * @param aMsg     Message.
     * @param aAction  Action.
     * @param aMissing Security info.
     * @return Result of security check.
     */
    TCustomResult CustomSecurityCheckL(
        const RMessage2 &aMsg,
        TInt &aAction,
        TSecurityInfo &aMissing );

private: // Constructors and destructor 
    
    /**
    * C++ default constructor.
    */
    CCRServer();

    /**
    * By default Symbian 2nd phase constructor is private.    
    */    
    void ConstructL( );

private: // Methods from base classes

    /**
    * Stops the server thread.
    * @since Series 60 3.0
    * @return None.
    */
    void StopServer();

private: // New methods

    /**
    * Deletes CR engine.
    * @since Series 60 3.0
    * @return None.
    */
    void DeleteRtpEngine();
    
private: // Data

    /**
    * Object container index.
    */
    CObjectConIx* iContainerIx;

    /**
    * Object container.
    */
    CObjectCon* iObjectCon;
    
    /**
    * CR engine object.
    */
    CCREngine* iEngine;
        
    /**
    * Number of sessions.
    */
    TInt iSessionCount;

    };

#endif // __CCRSERVER_H

// End of File
