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




// INCLUDE FILES
#include "CCRClientInformer.h"
#include <e32msgqueue.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRClientInformer::NewL
// -----------------------------------------------------------------------------
//
CCRClientInformer* CCRClientInformer::NewL()
    {
    CCRClientInformer* self = new( ELeave ) CCRClientInformer();    
    return self;
    }

// -----------------------------------------------------------------------------
// CCRClientInformer::CCRClientInformer
// -----------------------------------------------------------------------------    
//
CCRClientInformer::CCRClientInformer()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRClientInformer::~CCRClientInformer
// -----------------------------------------------------------------------------    
//
CCRClientInformer::~CCRClientInformer()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRClientInformer::SendMessage
// -----------------------------------------------------------------------------    
//
TInt CCRClientInformer::SendMessage(
    const TDes& aQueueName,
    SCRQueueEntry& aInfo )
    {
    LIVE_TV_TRACE2( _L( "CCRClientInformer::SendMessage: aInfo.iErr:%d" ), aInfo.iErr );

    RMsgQueue<SCRQueueEntry> queue;
    TInt err( queue.OpenGlobal( aQueueName ) );
    if ( err == KErrNone ) 
        {       
        err = queue.Send( aInfo );
        if ( err != KErrNone ) 
            {
            LIVE_TV_TRACE2(_L("CCRClientInformer::SendMessage() queue.Send() FAILED: %d"), err);
            }
            
        queue.Close();
        }
    else
        {
        LIVE_TV_TRACE2(_L("CCRClientInformer::SendMessage() queue.OpenGlobal() FAILED: %d"), err);                 
        }
        
    return err;
    }
    
//  End of File
