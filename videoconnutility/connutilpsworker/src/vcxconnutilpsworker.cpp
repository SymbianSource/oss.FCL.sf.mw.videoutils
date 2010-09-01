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
* Description:    executable for defining PS keys used by the connectionutility*
*/




#include <e32base.h>
#include <e32std.h>
#include <e32property.h>

#include "vcxconnutilcommon.h"
#include "vcxconnectionutility.hrh"


// ---------------------------------------------------------------------------
// E32Main()
// Provides the globale entry point function for the platform to start process 
// ---------------------------------------------------------------------------
//
GLDEF_C TInt E32Main()
    {
    // Create cleanup stack
    __UHEAP_MARK;

    // define properties
    RProperty::Define( KVcxConnUtilPScategory, EVCxPSConnectionStatus, RProperty::EInt );
            
    RProperty::Define( KVcxConnUtilPScategory, EVCxPSIapId, RProperty::EInt );
                        
    RProperty::Define( KVcxConnUtilPScategory, EVCxPSSnapId, RProperty::EInt );            
            
    RProperty::Define( KVcxConnUtilPScategory, EVCxPSMasterExists, RProperty::EInt );
            
    RProperty::Define( KVcxConnUtilPScategory, EVCxPSNbrConnInstances, RProperty::EInt );
            
    RProperty::Define( KVcxConnUtilPScategory, EVCxPSRoamingRequestStatus, RProperty::EInt );
            
    RProperty::Define( KVcxConnUtilPScategory, EVCxPSNbRoamAccepted, RProperty::EInt );
            
    RProperty::Define( KVcxConnUtilPScategory, EVCxPSNbrRoamResp, RProperty::EInt );
     
    RSemaphore semaphore;
    if( semaphore.OpenGlobal( KVcxConnUtilPSSema ) == KErrNone )
        {
        // Semaphore opened ok, so someone really needing PS keys exists.
        // signal it to notify PS keys are defined
        semaphore.Signal();
        semaphore.Close();
        }
    __UHEAP_MARKEND;
    return KErrNone;
    }
// end of file
