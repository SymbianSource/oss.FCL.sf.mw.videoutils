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
* Description:    Observer to inform common scheduling engine when plugin is*
*/




#ifndef __MCSEPLUGINOBSERVER_H__
#define __MCSEPLUGINOBSERVER_H__

// CLASS DECLARATION
/**
* MCsePluginObserver
* This class specifies the function to be called when a plugin
* cause error or completes.
*/
class MCsePluginObserver
    {
    public: // New functions                
        /**
        * Called when plugin is finihed progressing        
        * @param    aCompletionCode Completion code of plugin. KErrNone
        *                           if everything went alright.        
        */
        virtual void PluginCompleted( TInt aCompletionCode ) = 0;        
    };

#endif // __MCSEPLUGINOBSERVER_H__

// End of File