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
* Description:    containd common constants used in the scope of vcxconnectionutility*
*/




#ifndef __VCX_CONNUTILCOMMON_H
#define __VCX_CONNUTILCOMMON_H

/**
 * use UID of the PS worker as a category of the 
 * PS keys used by connectionutility
 */
const TUid KVcxConnUtilPScategory = { 0x2001B2AB };

/**
 * name of the "PS -worker" executable
 */
_LIT( KVcxConnUtilPSWorkerName, "vcxconnutilpsworker" );  

/**
 * name of the semaphore used by the PS worker to signal 
 * PS keys are ready
 */
_LIT( KVcxConnUtilPSSema, "__vcxxonnutilsemaphore__" );    

#endif // __VCX_CONNUTILCOMMON_H
