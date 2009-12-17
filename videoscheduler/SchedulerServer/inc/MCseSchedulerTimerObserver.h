/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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




#ifndef __MCSESCHEDULERTIMEROBSERVER_H__
#define __MCSESCHEDULERTIMEROBSERVER_H__

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION
/**
* MCseSchedulerTimerObserver
* This class specifies the function to be called when a timeout occurs.
* Used in conjunction with CCseSchedulerTimer class.
*/
class MCseSchedulerTimerObserver
    {
    public: // New functions
        /**
		* Called when time is up and plugins should be run.
		* 
		*/
        virtual void RunPluginsL( ) = 0;
        
        /**
		* Informs about time errors (time changes, timer is set towards time already
		* in past),
		* @parms aError timer error code. Most common are:
		*               KErrAbort - Phone time has changed
		*               KErrUnderflow - Time is in the past
		*               KErrOverFlow - Time is too far in the future
		*/
        virtual void TimerErrorL( TInt32 aError ) = 0;
    };

#endif // __MCSESCHEDULERTIMEROBSERVER_H__

// End of File