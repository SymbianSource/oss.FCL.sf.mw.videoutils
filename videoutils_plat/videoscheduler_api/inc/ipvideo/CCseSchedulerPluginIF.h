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
* Description:    Common interface for scheduler plugins.*
*/




#ifndef CCSESCHEDULERPLUGINIF_H
#define CCSESCHEDULERPLUGINIF_H

#include <e32std.h>
#include <e32base.h>
class CCseScheduledProgram;
class MCsePluginObserver;

class CCseSchedulerPluginIF : public CBase
    {
public:
    /**
    * Construction method.
    * @param    aUid    Ecom implementation uid of the used plugin.
    * @return   None
    */
    static CCseSchedulerPluginIF* NewL( const TUid& aUid );

    /**
    * Runs given task in plugin.
    * @param    aProg       Schedule to be run in plugin.
    * @param    aObserver   Pointer back to scheduling engine. Called
    *                       when schedule is completed or error occurs.
    * @return   None
    */	
	virtual void RunTaskL( CCseScheduledProgram& aProg,
						   MCsePluginObserver* aObserver ) = 0;	
	
    /**
    * Return the ECom implementation uid of this plugin.
    * @return ECom implementation uid.
    */
    TUid ImplementationUid() const;
    
    /**
    * Destructor
    */    
    virtual ~CCseSchedulerPluginIF();
    
protected:    
    /**
    * Instance identifier key. When instance of an
    * implementation is created by ECOM framework, the
    * framework will assign UID for it. The UID is used in
    * destructor to notify framework that this instance is
    * being destroyed and resources can be released.
    */    
    TUid iDtorIDKey;

    /**
    * Ecom implementation uid
    */
    TUid iImplementationUid;
    };



#include <ecom/ecom.h>
#include <ipvideo/CCseSchedulerPluginIF.inl>

#endif //CCSESCHEDULERPLUGINIF_H
