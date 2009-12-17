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
* Description:    scheduling engine client/server common header.*
*/





#ifndef __CSESCHEDULERCLIENTSERVERCOMMON_H__
#define __CSESCHEDULERCLIENTSERVERCOMMON_H__

// INCLUDE FILES
#include <e32base.h>

// CONSTANTS
_LIT( KCseSchedulerServerName,             "CseSchedulerEngineServer" ); // Server name
_LIT( KCseSchedulerServerSemaphoreName, "CseSchedulerServerSemaphore" );
_LIT( KCseSchedulerServerFileName,           "CseSchedulerServer.exe" );


// The server version. A version must be specified when
// creating a session with the server.
const TUint KCseServMajorVersionNumber=0;
const TUint KCseServMinorVersionNumber=1;
const TUint KCseServBuildVersionNumber=1;


// Enum for different server requests
typedef enum
	{
	ECseRequestBase = 0,
	ECseAddSchedule,
	ECseRemoveSchedule,
	ECseGetSchedules,
	ECseGetSchedule,
	ECseGetScheduleLength,
	ECseGetSchedulesByAppUid,
	ECseGetScheduleArrayLengthByAppUid,
	ECseServCreateSubSession,
	ECseServCloseSubSession,
	ECseServCloseSession,
	ECseGetOverlappingSchedulesLength,
	ECseGetOverlappingSchedules,
	ECseGetScheduleArrayLengthByPluginUid,
	ECseGetSchedulesByPluginUid,
	ECseGetScheduleArrayLengthByType,
	ECseGetSchedulesByType,
	ECseGetScheduleArrayLengthByTimeframe,
	ECseGetSchedulesByTimeframe,
	ECseRequestLast
	} MCseSchedulerServerRequests;

// Enum for different server responses	
typedef enum
	{
	ECseNoSubsessionHandle = 0x100,
	} MCseSchedulerRetValues;


#endif // __CSESCHEDULERCLIENTSERVERCOMMON_H__

// End of file