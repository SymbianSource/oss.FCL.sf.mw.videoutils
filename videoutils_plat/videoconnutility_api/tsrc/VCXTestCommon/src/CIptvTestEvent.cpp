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


//  INCLUDES
#include "CIptvTestEvent.h"
#include "VCXTestLog.h"

EXPORT_C CIptvTestEvent::~CIptvTestEvent()
    {
    }

EXPORT_C TBool CIptvTestEvent::HasTimeouted(TTime& aTimeNow)
    {
	if(!iActive)
		{
		return EFalse;
		}
	TTimeIntervalSeconds secondsAlive;
	aTimeNow.SecondsFrom(iActivationTime, secondsAlive);

	VCXLOGLO3("CIptvTestEvent:: '%S' seconds to timeout: %d", &iName, iTimeoutSeconds - secondsAlive.Int() );

	if(secondsAlive.Int() >= iTimeoutSeconds)
		{
		return ETrue;
		}
	return EFalse;
    }

EXPORT_C CIptvTestEvent::CIptvTestEvent( const TDesC& aName, TInt aTimeoutSeconds)
    {
	iName = _L("Noname Event");
	iName = aName;
	iTimeoutSeconds = aTimeoutSeconds;
	iActive = EFalse;
    }

void CIptvTestEvent::ConstructL()
    {
    }

EXPORT_C void CIptvTestEvent::GetName(TDes& aName)
	{
	aName = iName;
	}


EXPORT_C void CIptvTestEvent::Activate()
	{
	iActive = ETrue;
	iActivationTime.UniversalTime();
	VCXLOGLO2("ACTIVATED %S", &iName);
	}

EXPORT_C void CIptvTestEvent::Deactivate()
	{
	iActive = EFalse;
	VCXLOGLO2("DEACTIVATED %S", &iName);
	}


// End of File
