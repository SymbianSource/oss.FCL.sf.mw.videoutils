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
* Description:    Call back observer for clip manager.*
*/




#ifndef MRTPCLIPREPAIROBSERVER_H
#define MRTPCLIPREPAIROBSERVER_H

// INCLUDES
#include <e32def.h>

// CONSTANTS
// None.

// MACROS
// None.

// DATA TYPES
// None.

// FORWARD DECLARATIONS
// None.

// CLASS DECLARATION

/**
*  A RTP clip repair observer.
*  
*  @lib DvrRtpClipHandler.lib
*  @since Series 60 3.0
*/
class MRtpClipRepairObserver
    {

public: // New functions
    
    /**
    * Called when clip repairing completes.
    * @since Series 60 3.0
    * @param aStatus a status of repairing.
    * @return None.
    */
    virtual void RtpClipRepaired( const TInt aStatus ) = 0;
  
    };


#endif // MRTPCLIPREPAIROBSERVER_H

// End of File
