/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   ?Description*
*/


#ifndef VCXCONNUTILTESTCOMMON_H_
#define VCXCONNUTILTESTCOMMON_H_

// Category for PS keys used by tests.
const TUid KVCXConnUtilTestPScategory = { 0x101FB3E3 };

const TInt KVCXConnUtilTestExeGlobalTesterCount = 1;

// Set this to 1 to inform all the tester exe's for shutdown.
const TInt KVCXConnUtilTestExeGlobalShutdownKey = 2;

/**
 *  P&S key for commands, created by tester exe which adds it's own process id to this.
 */
const TUint KVCXConnUtilTestExePsKeyCmd = 10; // Observed for changes by tester exe.

/**
 *  Parameters for the commands, created by tester exe which adds it's own process id to these.
 */
const TUint KVCXConnUtilTestExePsKeyCmdIntParam1 = 11;
const TUint KVCXConnUtilTestExePsKeyCmdIntParam2 = 12;
const TUint KVCXConnUtilTestExePsKeyCmdDescParam1 = 13;

/**
 *  P&S keys for responses, created by tester exe which adds it's own process id to these.
 */
// This is sent after tester exe has received the cmd. 
const TUint KVCXConnUtilTestExePsKeyResponseAck = 21; 
// Will contain command which sent the repsonse.
const TUint KVCXConnUtilTestExePsKeyResponseCmd = 22;
// Will contain parameter for the repsonse.
const TUint KVCXConnUtilTestExePsKeyResponseParam = 23;
// Symbian error code for the response. 
const TUint KVCXConnUtilTestExePsKeyResponseError = 24;

/**
 * List of commands to tester exe.
 */
enum TVcxConnUtilCommand
    {
    EVcxConnUtilCommandShutdown, // Params: none
    EVcxConnUtilCommandSetName, // Params: desc for name
    EVcxConnUtilCommandGetIap, // Params: int1 = silent, int2 = timed call
    EVcxConnUtilCommandGetWapIdForIap, // Params: int1 = IAP ID
    EVcxConnUtilCommandDisconnect, // Params: none
    EVcxConnUtilCommandSetRoamingAllowed, // Params: int1 = the value
    EVcxConnUtilCommandSetRoamingLeaveAtRoamingRequest, // Params: int1 = the value
    EVcxConnUtilCommandSetDelayBeforeRoamingRequest, // Params: int1 = the value
    };

#endif /*VCXCONNUTILTESTCOMMON_H_*/
