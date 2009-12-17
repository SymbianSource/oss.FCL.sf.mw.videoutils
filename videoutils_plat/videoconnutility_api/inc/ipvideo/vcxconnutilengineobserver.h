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
* Description:    Interface for network event handling*
*/


#ifndef _M_CVCX_CONUTILENGINEOBS_H__
#define _M_CVCX_CONUTILENGINEOBS_H__


// CLASS DECLARATION
/**
* Interface for the clients of vcxconnectionutility to receive and react
* for iap changes and roaming. 
* It is not required for a client to implement this interface. In that case
* vcxconnectionutility always excepts client to allow roaming.
* 
*/

class MConnUtilEngineObserver

    {
    public:

         /**
         * This method is called by the vcxconnectionutility when 
         * it has received a notification about the availability of
         * "better" iaps in the network. This can happen  
         * when WLAN defined in the corresponding destination becomes
         * available or when WLAN carrier is lost and there is a defined
         * 3G/gprs iap (or another available WLAN) in the destination
         * being used.
         *
         * Client does not have to accept the changing of the iap. If the client 
         * returns EFalse notifying it does not want to roam, then the 
         * iap is not changed.
         *
         * This however can result in disconnecting, if the old iap is 
         * no longer available, as happens when WLAN carrier is lost
         *
         * @return TBool  ETrue: roaming accepted, EFalse: Roaming not accepted
         *
         */
        virtual TBool RequestIsRoamingAllowedL() = 0;
        
        /**
         * This method is called by the vcxconnectionutility when 
         * there have been some changes in the iap currently using:
         * 
         * Connection lost: this happens when for example WLAN carrier 
         * is lost and there is no other iap to use or when the corresponding
         * connection is closed by some other application.
         *
         * Iap change by the roaming: if the client has accepted roaming,
         * vcxconnectionutility notifies about new iap is ready to use
         *
         * When client is being called by this method, it can request an
         * iap id by calling CVcxConnectionUtility::GetIap-
         *
         *
         */
        virtual void  IapChangedL() = 0;
    };

#endif // _M_CVCX_CONUTILENGINEOBS_H__
