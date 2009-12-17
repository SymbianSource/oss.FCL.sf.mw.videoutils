/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    Streaming observer definition for streaming session.*
*/




#ifndef _MCRSTREAMOBSERVER_H
#define _MCRSTREAMOBSERVER_H

// INCLUDES
#include "MCRPacketSource.h"

/**
*  Stream observer.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class MCRStreamObserver
    {

public: // New methods
    
    /**
    * Source status indication.
    * @since Series 60 3.0
    * @param aStatus
    * @return none.
    */
    virtual void StatusChanged( MCRPacketSource::TCRPacketSourceState aStatus ) = 0;
    
    };
    
#endif // _MCRSTREAMOBSERVER_H

//  End of File
