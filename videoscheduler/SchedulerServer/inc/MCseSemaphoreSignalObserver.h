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
* Description:    Observer to inform plugin starter that it is time shutdown*
*/




#ifndef __MCSESEMAPHORESIGNALOBSERVER_H__
#define __MCSESEMAPHORESIGNALOBSERVER_H__

// CLASS DECLARATION
/**
* MCseSemaphoreSignalObserver
* Observer interface to be called once observed semaphore is signalled
*/
class MCseSemaphoreSignalObserver
    {
    public: // New functions                
        /**
        * Called when observed semaphore is signalled.        
        */
        virtual void SemaphoreSignalled( ) = 0;        
    };

#endif // __MCSESEMAPHORESIGNALOBSERVER_H__

// End of File
