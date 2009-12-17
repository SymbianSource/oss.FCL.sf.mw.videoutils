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
* Description:    Sends message to client with message queue.*
*/




#ifndef CCRCLIENTINFORMER_H
#define CCRCLIENTINFORMER_H

// INCLUDES
#include <e32base.h>
#include <ipvideo/CRTypeDefs.h>

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
*  Packet sink that does not forward packets. Good for testing. 
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRClientInformer : public CBase
    {

public: // Constructors and destructors

    /**
    * Two-phased constructor.
    * @return CCRClientInformer pointer. 
    */
    static CCRClientInformer* NewL( void );
    
    /**
    * Destructor 
    */
    virtual ~CCRClientInformer( );
    
public: // New methods

    /**
    * Send a message thrue the message queue.
    * @since Series 60 3.0
    * @param aQueueName a name of queue.
    * @param aInfo a info of the message.
    * @return a system wide error code.
    */
    TInt SendMessage( const TDes& aQueueName,
                      SCRQueueEntry& aInfo );
    
private: // Constructors and destructors
    
    /**
    * By default default constructor is private
    */
    CCRClientInformer();
    
    };

#endif // CCRCLIENTINFORMER_H

//  End of File
