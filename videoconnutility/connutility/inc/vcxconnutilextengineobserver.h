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
* Description:    extended engine observer interface for video connection utility internal use*
*/




#ifndef __VCXCONNUTILEXTENGINEOBSERVER_H_
#define __VCXCONNUTILEXTENGINEOBSERVER_H_

#include <ipvideo/vcxconnutilengineobserver.h>
/**
* Interface is being used inside vcxconnectionutility to notify 
* IAP changes and request for waits
*/

class MConnUtilExtEngineObserver : public MConnUtilEngineObserver

    {
    public:
        
        /**
         * Method is called, when observable request observer to
         * put it's current active object to wait until EndWait
         * for same wait id is called
         * 
         * @param aWaitId    wait id 
         */
        virtual void WaitL( TUint32 aWaitId ) = 0;
        
        /**
         * Method is called, when observable request observer to
         * end wait for given id
         * 
         * @param aWaitId    wait id 
         */
        virtual void EndWait( TUint32 aWaitId ) = 0;
        
        /**
         * Method is called, when observable needs to know wether
         * this instance is the master instance
         * 
         * @return TBool    ETrue is master, EFalse not a master 
         */
        virtual TBool IsMaster() = 0;

    };


#endif /*VCXCONNUTILEXTENGINEOBSERVER_H_*/
