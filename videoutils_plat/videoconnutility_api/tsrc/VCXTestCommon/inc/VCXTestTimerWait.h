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


#ifndef VCXTIMERWAIT_H
#define VCXTIMERWAIT_H

//  INCLUDES
#include <e32base.h>
#include "VCXTestLog.h"
#include "MIptvTestTimerObserver.h"

// FORWARD DECLARATIONS
class CIptvTestTimer;
class CIptvTestActiveWait;

// DATA TYPES

// CLASS DECLARATION

/**
 * Timer waiter class.
 */
class CVCXTestTimerWait : public CBase, public MIptvTestTimerObserver
    {
    public:
        IMPORT_C static CVCXTestTimerWait* NewL();

        IMPORT_C ~CVCXTestTimerWait();

        IMPORT_C void WaitL( TUint32 aMicroSeconds );

    public:
        void TimerComplete(TInt aTimerId, TInt aError);

    private:

        IMPORT_C void ConstructL();

        CVCXTestTimerWait();

    private:
        CIptvTestTimer* iTimer;
        CIptvTestActiveWait* iWait;

    };

#endif // VCXTIMERWAIT_H
