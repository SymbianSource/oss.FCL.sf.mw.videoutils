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


#ifndef MIPTVTESTEVENTWAITEROBSERVER_H
#define MIPTVTESTEVENTWAITEROBSERVER_H

// INCLUDES

#include <e32def.h>

// FORWARD DECLARATIONS
class CIptvTestEvent;

// CLASS DECLARATION

class MIptvTestEventWaiterObserver
    {
    public:
		/**
		* Which event caused an error and why
		*/
		virtual void EventWaiterErrorL(TInt aError, CIptvTestEvent& aEvent) = 0;

		/**
		*
		*/
		virtual void EventWaiterAllEventsValidated() = 0;

    };

#endif  // MIPTVTESTEVENTWAITEROBSERVER_H

// End of File
