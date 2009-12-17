/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*
*/


#ifndef VCXTESTMESSAGEWAIT_H_
#define VCXTESTMESSAGEWAIT_H_

//  INCLUDES
#include <e32base.h>
#include "MIptvTestTimerObserver.h"

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION
class CIptvTestActiveWait;
class CIptvTestTimer;

/**
* MVCXTestMessageWaitObserver
*/
class MVCXTestMessageWaitObserver
    {
    public:
    
	    /**
	    * Called when message queue empties or there's timeout.
	    */
    	virtual void MessageWaitComplete( TInt aError ) = 0;
    
    };

/**
* CVCXTestMessageWait
*/
class CVCXTestMessageWait : public CBase, public MIptvTestTimerObserver
    {
    public:  // Constructors and destructor
        /**
        * Two-phased constructor.
        */
        IMPORT_C static CVCXTestMessageWait * NewL( MVCXTestMessageWaitObserver* aObserver );

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CVCXTestMessageWait ();


    public: // New functions

		/**
		* Resets already received messages, wait queue and cancels previous receiving.  
		*/		
		IMPORT_C void Reset();

        /**
        * Resets only already received messages.  
        */      
        IMPORT_C void ResetReceivedMessages();
        
		/**
		* Adds message to the wait queue.
		*/
		IMPORT_C void AddMessage( TInt32 aMsg );
		
		/**
		* Waits for a message. Calls observer callback when message is received. This will reset 
		* previously added messages. 
		*/		
		IMPORT_C void WaitForMessageL( TInt32 aMsg, TInt aTimeoutSeconds, TBool aDoBlock );
		
		/**
		* Waits for the added messages. Calls observer callback when messages have been received.
		*/		
		IMPORT_C void WaitForAllL( TInt aTimeoutSeconds, TBool aDoBlock );
		
        /**
        * Waits until messages have not been received for the specified seconds. 
        */      
        IMPORT_C void CoolDownL( TInt aSeconds );		

		/**
		* Checks wait queue for the message.
		*/		
		IMPORT_C void ReceiveMessage( TInt32 aMsg, TInt aError );
		
    public: // From base classes.
        
        void TimerComplete( TInt aTimerId, TInt aError );
    
    private:

        /**
        * C++ default constructor.
        */
        CVCXTestMessageWait( MVCXTestMessageWaitObserver* aObserver );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        IMPORT_C void ConstructL();

    private:    // Data
		
		MVCXTestMessageWaitObserver* iObserver;
		
	    CIptvTestActiveWait* iActiveWaitBlocking;
	    
	    CIptvTestTimer* iTimeoutTimer;
	    
	    // Messages which are received while waited messages array is empty.
	    RArray<TInt32> iReceivedMessages;
	    
	    RArray<TInt32> iWaitedMessages;
	  
	    TInt iTimeoutSeconds;
	    
	    TBool iWaitStarted;
	    
	    TBool iCoolingDown;
	    
	    TInt iError;
    };

#endif // VCXTESTMESSAGEWAIT_H_

// End of File
