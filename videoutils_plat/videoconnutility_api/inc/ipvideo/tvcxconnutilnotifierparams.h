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
* Description:    data structure for passing messages between con utility and*
*/





#ifndef TVCXCONNUTILNOTIFIERPARAMS_
#define TVCXCONNUTILNOTIFIERPARAMS_

// Deprecation warning
#warning This header file has been deprecated. Will be removed in one of the next SDK releases.

/**
 * Max lenght of the string passed via message
 */
const TInt KConnUtilMaxMsgLen = 255;

/**
 * implementation UID for the notifier plugin that
 * videoconnutility uses
 */
const TUid KVcxNotifierImplUid = { 0x20016BA5 };

/**
*  TVcxNotifierParams class. Class is used as message between 
*  vcxconnectionutility and notifier plugin. Message is to be sent to notifier,
*  when connection is being created for showing "connecting.." or similar msg and
*  when connection creation is ready.
*
*  When connection is to be created, vcxconnectionutility sends message via through
*  notifier server where it's passed to the notifier plugin with uid of KVcxNotifierImplUid
*  vcxconnectionutility might pass additional message string to be shown in the dialogg
*  with iMsg descriptor.
* 
*  Event can be either:
*  - EConnecting (1), when notifier should show dialog asynchronously
*  - EConnected (2), when vcxconnectionutility wants plugin to close dialog synchronously
*  enums are defined in vcxconnectionutility.hrh
* 
* If plugin wants to nofify about closing of dialog before vcxconnectionutility wants it to be closed.
* it should complete async dialog showing with result of KErrCancel.
*  
*/
class TVcxNotifierParams 
    {
    public:      

        enum TVcxNotifierEventId
            {
            EConnecting = 1,
            EConnected
            };
        
        /**
        * Event.
        */        
        TInt iEvent;

        /**
        * String message.
        */
        TBuf<KConnUtilMaxMsgLen> iMsg;        
    };



#endif // TVCXCONNUTILNOTIFIERPARAMS_
