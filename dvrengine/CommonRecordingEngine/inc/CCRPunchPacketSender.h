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
* Description:    Class for sending a punch packet*
*/




#ifndef __CCRPUNCHPACKRTSENDER_H
#define __CCRPUNCHPACKRTSENDER_H

// INCLUDES
#include <e32base.h>
#include "CCRSock.h"

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRConnection; 
class CAsyncCallBack;
class CCRRtspPacketSource;

// CLASS DECLARATION

/**
*  Class that punches holes to firewall.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRPunchPacketSender : public CBase, public MCRSockObserver
    {

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @return CCRPunchPacketSender pointer to CCRPunchPacketSender class
    */
    static CCRPunchPacketSender* NewL( RConnection& aConnection, 
                                       RSocketServ& aSockServer, 
                                       TInetAddr& aFromAddr, 
                                       TInetAddr& aRemoteAddr , 
                                       TUint32 aMySSRC,
                                       CCRRtspPacketSource& aOwner );

    /**
    * Destructor.
    */
    virtual ~CCRPunchPacketSender( );

private: // Methods from base classes

    /**
    * From MCRSockObserver.
    * This method is called after some data has been received  from socket.
    * @since Series 60 3.0
    * @param aData is descriptor containing the data received. 
    *        ownership of data is not passed via this call.
    * @return none.
    */
    void DataReceived( TInt aSockId,
                       const TDesC8 &aData );

    /**
    * From MCRSockObserver.
    * This method is called after status of socket changes.
    * @since Series 60 3.0
    * @param aStatus is sock status.
    * @return none.
    */
    void SockStatusChange( TInt aSockId,
                           CCRSock::TCRSockStatus aStatus,
                           TInt aError );

private:

    /**
    * Default constructor.
    * @param aConnection a connection.
    * @param aSockServer a socket server.
    * @param aFromAddr a address.
    * @param aRemoteAddr
    * @param aMySSRC
    * @param aOwner
    */
    CCRPunchPacketSender( RConnection& aConnection, 
                          RSocketServ& aSockServer,
                          TInetAddr& aFromAddr, 
                          TInetAddr& aRemoteAddr , 
                          TUint32 aMySSRC,
                          CCRRtspPacketSource& aOwner ); 
     
    /** 
    * 2nd phase constructor 
    */
    void ConstructL();
       
private: // new methods
    
    /**
    * Method that is called when this instace is finishing business.
    * @since Series 60 3.0
    * @param aSelf
    * @return TInt
    */
    static TInt CleanupCallBack ( TAny* aSelf ); 
    
private: // Data types
    
    /**
    * Connection.
    */
    RConnection& iConnection; 
    
    /**
    * Socket server.
    */
    RSocketServ& iSockServer;
    
    /**
    * Socket 1.
    */
    CCRSock* iSock1;
    
    /**
    * Socket 2
    */
    CCRSock* iSock2;
    
    /**
    * Address.
    */
    TInetAddr iFromAddr;
    
    /**
    * Remote address.
    */
    TInetAddr iRemoteAddr;
    
    /**
    * Sender report.
    */
    TUint32 iMySSRC;
    
    /**
    * Owner.
    */
    CCRRtspPacketSource& iOwner; 
    
    /**
    * Asyncronic cleanup.
    */
    CAsyncCallBack* iCleanUp; 
    
    /**
    * Socket 1 state.
    */
    TBool iSentViaSock1; 
    
    /**
    * Socket 2 state.
    */
    TBool iSentViaSock2;         

    };

#endif // __CCRPUNCHPACKRTSENDER_H

// End of file
