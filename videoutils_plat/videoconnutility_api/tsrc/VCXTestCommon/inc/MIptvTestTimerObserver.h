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


#ifndef MIPTVTESTTIMEROBSERVER_H
#define MIPTVTESTTIMEROBSERVER_H

// INCLUDES

#include <e32def.h>

// FORWARD DECLARATIONS


// CLASS DECLARATION

class MIptvTestTimerObserver
    {
    public:
		virtual void TimerComplete(TInt aTimerId, TInt aError) = 0;

    };

#endif  // MIPTVTESTTIMEROBSERVER_H

// End of File
