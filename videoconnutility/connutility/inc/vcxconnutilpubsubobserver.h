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
* Description:    interface to notify about PS key changes in vcxconnectionutility*
*/




#ifndef _M_CVCX_CONUTILOBS_H__
#define _M_CVCX_CONUTILOBS_H__

#include <e32def.h>
#include <e32cmn.h>

/**
* Interface is being used inside vcxconnectionutility to notify 
* the implementation class about changes of values in the PS keys.
*/

class MConnUtilPubsubObserver

    {
    public:
        
        /**
         * Method to be called when PS key value changes
         * 
         * @param aKey    key id 
         * @param aValue  new value
         */
        virtual void ValueChangedL(const TUint32& aKey, const TInt&  aValue) = 0;

    };

#endif // _M_CVCX_CONUTILOBS_H__
