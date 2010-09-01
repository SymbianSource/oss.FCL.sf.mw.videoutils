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
* Description:    A timer.*
*/




#ifndef CRTPTIMER_H
#define CRTPTIMER_H

// INCLUDES
#include <e32base.h>

// CONSTANTS
_LIT( KPaniCRtpTimerError, "RTP Timer panic" );

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class MRtpTimerObserver;

// CLASS DECLARATION

/**
*  A timer for RTP handler.
*
*  @lib RtpClipHandler.lib
*  @since Series 60 3.0
*/
class CRtpTimer : public CTimer
    {

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @param aObs a notifier which informs when the time passed. 
    * @param aPrior a priority of the timer.
    */
    IMPORT_C static CRtpTimer* NewL( 
        MRtpTimerObserver& aObs, 
        const TPriority& aPrior = EPriorityStandard );
    
    /**
    * Destructor.
    */
    IMPORT_C virtual ~CRtpTimer();

private: // Constructors and destructor

    /**
    * C++ parameter constructor.
    * @param aObs The notifier which informs when the time passed 
    * @param aPrior a priority of the timer.
    */
    CRtpTimer( MRtpTimerObserver& aObs, const TPriority& aPrior );

    /**
    * By default Symbian 2nd phase constructor is private.
    */
    void ConstructL();

private: // Functions from base classes

    /**
    * From CActive : Called when request completion event occurs.
    * @since Series 60 3.0
    * @return none
    */
    void RunL();

    /**
    * From CActive : Handles a leave occurring in the request
    *                completion event handler RunL().
    * @since Series 60 3.0
    * @param aError the leave code
    * @return a status of function
    */
    TInt RunError( TInt aError );

private: // Data

    // Observer
    MRtpTimerObserver& iObs;

  };

#endif // CRTPTIMER_H
            
// End of File
