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


#ifndef MIPTVUTILCONNECIONTOBSERVER_H
#define MIPTVUTILCONNECIONTOBSERVER_H

// INCLUDES

#include <e32std.h>

// CLASS DECLARATION

class MTestUtilConnectionObserver
    {
public:

    virtual void ConnectionCreated() = 0;
    virtual void ConnectionClosed() = 0;
    virtual void ConnectionFailed() = 0;
    virtual void ConnectionAlreadyExists() = 0;
    virtual void ConnectionTimeout() = 0;
    };

#endif // IPTVUTILCONNECIONTOBSERVER_H

