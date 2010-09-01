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
* Description:    CSE inlines.*
*/



#ifndef CIPTVSCHEDULERPLUGIN_INL
#define CIPTVSCHEDULERPLUGIN_INL

// INLINES

/**
* Contructor
*/
inline CCseSchedulerPluginIF* CCseSchedulerPluginIF::NewL( const TUid& aUid  )
    {
    TAny* ext = REComSession::CreateImplementationL( aUid, _FOFF( CCseSchedulerPluginIF, iDtorIDKey ));

    CCseSchedulerPluginIF* result = reinterpret_cast< CCseSchedulerPluginIF* >( ext );
    result->iImplementationUid = aUid;  // set the plugin's implementation uid
    
    return result;
    }


/**
* Destructor
*/
inline CCseSchedulerPluginIF::~CCseSchedulerPluginIF()
    {
    // Inform the ECOM framework that this specific instance of the
    // interface has been destroyed.
    REComSession::DestroyedImplementation( iDtorIDKey );
    }
/**
* ImplementationUid
*/
inline TUid CCseSchedulerPluginIF::ImplementationUid() const 
    { 
    return iImplementationUid; 
    }

#endif
