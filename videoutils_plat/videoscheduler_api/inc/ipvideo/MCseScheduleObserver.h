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




#ifndef __MCSESCHEDULEOBSERVER_H__
#define __MCSESCHEDULEOBSERVER_H__

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION
/**
* MCseScheduleObserver
* This class specifies the function to be called when a scheduler
* cause error or completes.
*/
class MCseScheduleObserver
    {
    public: // New functions                
        /**
        * Called when schedule is finished prosessing-
		* @param aDbIdentifier   DbIdentifier of scheduled program that just
		*                        has been processed.
		* @param aCompletionCode Completion code of schedule. KErrNone
        *                        if everything went alright.
        */
        virtual void ScheduleCompletedL( const TUint32 aDbIdentifier,
                                         TInt aCompletionCode ) = 0;
    };

#endif // __MCSESCHEDULEOBSERVER_H__

// End of File
