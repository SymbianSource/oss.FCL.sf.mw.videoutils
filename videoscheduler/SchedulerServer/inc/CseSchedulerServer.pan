/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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





#ifndef __CSESCHEDULERSERVER_PAN__
#define __CSESCHEDULERSERVER_PAN__

/** CseEngineServer panic codes */
enum TCseSchedulerServPanic
    {
    ECsePanicBadRequest = 1,
    ECsePanicBadDescriptor,
    ECreateTrapCleanup,
    ECseCreateTrapCleanup,
    ECseSrvCreateServer,
    ECsePanicBadSubSessionHandle,
	ECsePanicCouldNotCreatePrivateDir
    };

#endif // __CSESCHEDULERSERVER_PAN__

// End of File