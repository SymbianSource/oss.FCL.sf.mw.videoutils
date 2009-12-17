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
* Description:    Observer for CCRTimer*
*/




#ifndef CCRTIMEROBSERVER_H
#define CCRTIMEROBSERVER_H

// INCLUDES
// None

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRTimer;

// CLASS DECLARATION

/**
*  This class specifies the function to be called when a timeout occurs.
*  Used in conjunction with CCRTimer class.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class MCRTimerObserver
    {

public:

    /**
    * The function to be called when a timeout occurs.
    * @since Series 60 3.0
    * @param aTimer a pointer to timer.
    * @return none.
    */
    virtual void TimerExpired( CCRTimer* aTimer ) = 0;
    
    };

#endif // CCRTIMEROBSERVER_H

//  End of File


