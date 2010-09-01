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
* Description:    Wrapper for CTimer*
*/




#ifndef CCRTIMER_H
#define CCRTIMER_H

// INCLUDES
#include <e32base.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class MCRTimerObserver;

/**
*  CCRTimer
*
*  This class will notify an object after a specified timeout.
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRTimer : public CTimer
    {

public: // Constructors and destructors

    /**
    * Two-phased constructor.
    * Creates a CCRTimer object using two phase construction,
    * and returns a pointer to the created object.
    * @since Series 60 3.0
    * @param aPriority Priority to use for this timer.
    * @param aTimeOutNotify Object to notify of timeout event.
    * @return A pointer to the created instance of CCRTimer.
    */
    IMPORT_C static CCRTimer* NewL( const TInt aPriority,
                                    MCRTimerObserver& aTimerObserver );

    /**
    * Two-phased constructor.
    * Creates a CCRTimer object using two phase construction,
    * and returns a pointer to the created object.
    * @since Series 60 3.0
    * @param aPriority Priority to use for this timer.
    * @param aTimerObserver Object to notify of timeout event.
    * @return A pointer to the created instance of CCRTimer.
    */
    IMPORT_C static CCRTimer* NewLC( const TInt aPriority,
                                     MCRTimerObserver& aTimerObserver );

    /**
    * Destructor.
    * Destroys the object and release all memory objects.
    */
    virtual IMPORT_C ~CCRTimer();

protected: // Functions from base classes

    /**
    * From CActive.
    * Called when operation completes.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    IMPORT_C void RunL();

private: // Constructors and destructors

    /**
    * C++ default constructor.
    * Performs the first phase of two phase construction.
    * @since Series 60 3.0
    * @param aPriority Priority to use for this timer.
    * @param aTimeOutNotify An observer to notify.
    * @return none.
    */
    IMPORT_C CCRTimer( const TInt aPriority,
                       MCRTimerObserver& aTimerObserver );

    /**
    * ConstructL.
    * 2nd phase constructor.
    */
    IMPORT_C void ConstructL();

private: // Data
    
    /**
    * iNotify, the observer for this objects events.
    */
    MCRTimerObserver& iObserver;
    
    };

#endif // CCRTIMER_H

//  End of File
