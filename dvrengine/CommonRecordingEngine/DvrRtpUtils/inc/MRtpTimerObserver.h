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
* Description:    A timer observer.*
*/




#ifndef MRTPTIMEROBSERVER_H
#define MRTPTIMEROBSERVER_H

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
*  A timer observer.
*
*  @lib RtpClipHandler.lib
*  @since Series 60 3.0
*/
class MRtpTimerObserver
    {

public: // New functions
    
    /**
    * Called when timer completion event occurs
    * @since Series 60 3.0
    * @return none
    */
    virtual void TimerEventL() = 0;

    /**
    * Handles a leave occurring in the request
    * completion event handler RunL().
    * @since Series 60 3.0
    * @param aError the leave code.
    * @return None.
    */
    virtual void TimerError( const TInt aError ) = 0;

    };

#endif // MRTPTIMEROBSERVER_H
            
// End of File
