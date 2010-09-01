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


#ifndef VCXTESTPSOBSERVER_H
#define VCXTESTPSOBSERVER_H

#include <e32def.h>
#include <e32cmn.h>

class MVCXTestPSObserver
    {
public:
    virtual void ValueChangedL( const TUid& aUid, const TUint32& aKey, const TInt&  aValue ) = 0;
    virtual void ValueChangedL( const TUid& aUid, const TUint32& aKey, const TDesC& aValue ) = 0;
    };

#endif // VCXTESTPSOBSERVER_H