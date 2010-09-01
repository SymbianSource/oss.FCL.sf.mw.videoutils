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
* Description:    Class for wrapping RSocket and CActive*
*/




#ifndef CCRSOCK_H
#define CCRSOCK_H

// INCLUDES
#include <in_sock.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KMaxDataSize( 8192 );

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
class CCRSock;
class MCRSockObserver; 

// CLASS DECLARATION

/**
*  Class for receiving data via socket.
*
*  This is solely owned by CCRSock and considered a helper class.
*  Because there may be send and receive operations outstanding
*  at the same time, one RSocket needs 2  CActive instances.
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CCRSockReader : public CActive
    {

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @param aSock is reference to socket object 
    *        that this class instance serves.
    * @since Series 60 3.0
    * @param aConnection a connection.
    * @param aSockServer a socket server.
    * @return CCRSockReader pointer to CCRSockReader class.
    */
    static CCRSockReader* NewL( CCRSock& aSock, 
                                RConnection& aConnection,
                                RSocketServ& aSockServer );
    /**
    * Destructor.
    */
    virtual ~CCRSockReader();
    
public: // New Methods

    /**
    * Method for starting reading.
    * @since Series 60 3.0
    * @param none
    * @return always succeeds
    */
    void IssueRead( void );
    
private: // Constructors and destructors
    
    /**
    * Default constructor
    */
    CCRSockReader( CCRSock& aSock,
                   RConnection& aConnection,
                   RSocketServ& aSockServer );
    /** 
    * 2nd phase constructor 
    */
    void ConstructL();

private: // Methods from base classes

    /** 
    * From CActive.
    * This is the work-horse of this class.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void RunL();
    
    /** 
    * From CActive.
    * This cancels work in progress.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */     
    void DoCancel();
    
    /** 
    * From CActive.
    * This handles errors.
    * @since Series 60 3.0
    * @param none.
    * @return none an status of method.
    */     
    TInt RunError( TInt aError );
    
private: // Data
    
    /**
    * Socket.
    */
    CCRSock& iSock;

    /** 
    * connection to use.
    */
    RConnection& iConnection;
    
    /**
    * Socket server.
    */
    RSocketServ& iSockServer;

#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE ) 
    TUint recvCount; 
    TUint recvBytes; 
#endif // LIVE_TV_FILE_TRACE || LIVE_TV_RDEBUG_TRACE
    };

/**
* Encapsulates a socket in easy-to-use way
*/
class CCRSock : public CActive
    {

public: // Data types

    /**
     * Enum for communicating what is going on with this socket 
     */
    enum TCRSockStatus
        {
        EInitNeeded = 1000,
        EIdle,       /**< Nothing in progress */
        EResolving,   /**< Finding out addr to connect to */
        EConnecting,  /**< Addr found but no connection yet */
        ESending,     /**< Request to send has been issued but not yet completed */
        EListening,   /**< Request to wait for incoming connection is not yet compl. */     
        EFailed       /**< Something went wrong */
        };

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @param aObserver is reference to object that 
    *        gets notified when something happens
    * @param aSockId is a number that this class instance
    *        will keep, do nothing with, and us the same 
    *        number when reporting statuses back to observer.
    *        motivation here is that same observer may have
    *        several instances of this class all reporting
    *        via same method. 
    * @param aProtoTCP ETrue for TCP socket, EFalse for UDP
    * @param aIssueRead ETrue to automatically receive data from
    *        socket and return to observer through DataReceived.
    *        EFalse to let user handle receiving data.
    * @return CCRSock pointer to CCRSock class
    */
    static CCRSock* NewL( MCRSockObserver& aObserver,
                          TInt aSockId,
                          RConnection& aConnection,
                          RSocketServ& aSockServer,
                          TBool aProtoTCP,
                          TBool aIssueRead );

    /**
    * Destructor.
    */
    virtual ~CCRSock();

public: // New methods

    /**
    * method for causing the socket to connect to remote addr.
    * @since Series 60 3.0
    * @param aAddr is the addr to connect to 
    * @param aPort is ip port number to connect to
    * @param aLocalPort specifies the local port to bind to. If 0 random
    *        port is selected
    * @return KErrNone is returned if connection is all right
    */
    TInt ConnectSock( const TDesC& aAddr,
                      TUint aPort,
                      TInt aLocalPort = 0 );

    /**
    * method for causing the socket to connect to remote addr without
    * doing DNS lookup. 
    * @since Series 60 3.0
    * @param aAddr is the addr to connect to 
    * @param aLocalPort specifies the local port to bind to. If 0 random
    *        port is selected       
    * @return KErrNone is returned if connection is all right
    */
    TInt ConnectSock( const TSockAddr& aAddr, 
                      TInt aLocalPort = 0);
                      
    /**
    * method for causing the socket to start listening at part.
    * @since Series 60 3.0
    * @param aPort is the port to listen to 
    * @return KErrNone is returned if connection is all right
    */
    TInt ListenPort( TUint aPort );
    
    /**
    * method for joining a multicast group
    * @since Series 60 3.0
    * @param aGruopAddr IPv6 address of the group to join
    * @return KErrNone on success
    */
    TInt JoinGroup( const TInetAddr& aGroupAddr );
    
    /**
    * method for sending data over the sock
    * @since Series 60 3.0
    * @param aData is the data that is sent over sock
    * @return none, succeeds always, if something goes wrong, it does it in async way
    */      
    void SendData( const TDesC8& aDataThatIsSentOverSocket );
        
    /**
    * Method for asking the status: what is going on 
    * @since Series 60 3.0
    * @param none.
    * @return socket status.
    */
    CCRSock::TCRSockStatus SockStatus( void ) const;

    /** 
    * Helper class may frobnicate our private parts:
    */
    friend class CCRSockReader;
    
    /** 
    * Method for getting the addr this socket is connected to in the other end
    * @since Series 60 3.0
    * @param none.
    * @return the addr
    */
    TInetAddr ConnectedAddr( void );

    /** 
    * Method for getting the addr this socket is connected to in the local end
    * @since Series 60 3.0
    * @param none.
    * @return the addr
    */
    TInetAddr LocalAddr( void );
    
    /** 
    * Method for setting the "where to send addr" and this is applicable for UDP socks
    * @param aAddr is the new addr.
    * @return none
    */
    void SetToAddr( const TInetAddr &aAddr );

    /**
    * Gets reference underlying Symbian socket implementation. To be used with caution.
    * @since Series 60 3.0
    * @param none.
    * @return reference to underlying ES_SOCK socket
    */
    RSocket& Socket();

private: // Constructors and destructors

    /**
    * default constructor
    */
    CCRSock( MCRSockObserver& aObserver,
             TInt aSockId,
             RConnection& aConnection,
             RSocketServ& aSockServer,
             TBool aProtoTCP,
             TBool aIssueRead );

    /** 
    * 2nd phase constructor 
    */
    void ConstructL();
       
private: // Methods from base classes

    /** 
    * From CActive.
    * This is the work-horse of this class.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */
    void RunL();
    
    /** 
    * From CActive.
    * This cancels work in progress.
    * @since Series 60 3.0
    * @param none.
    * @return none.
    */     
    void DoCancel();
    
    /** 
    * From CActive.
    * This handles errors.
    * @since Series 60 3.0
    * @param none.
    * @return none an status of method.
    */     
    TInt RunError( TInt aError );

private: // New methods

    /**
    * Handles send buffer.
    */
    void CopySendData( const TDesC8& aData );

    /**
    * Performs cleanup.
    */
    void CleanUp();
    
private: // Data types
    
    /**
    * This is used to read/write.
    */
    RSocket iSocket;
    
    /**
    * status for above socket.
    */
    TBool iIsiSocketOpen;
    
    /**
    * This is used to listen.
    * Not used when we use this class for outgoing connection.
    */
    RSocket iListenSocket;  
    
    /**
    * Status for above socket.
    */
    TBool iIsiListenSocketOpen;
    
    /**
    * This is used to find out addr by name.
    */
    RHostResolver iResolver;       
    /**
    * This is needed to get hold of RSocket.
    */
    RSocketServ& iSockServer;
    
    /**
    * This is where we connect to.
    */
    TNameEntry iHostAddress;
    
    /**
    * this is our internal status.
    */
    TCRSockStatus iSockStatus;
    
    /**
    * Our observer.
    */
    MCRSockObserver& iObserver;
    
    /**
    * Our own internal id.
    */
    const TInt iSockId;
    
    /**
    * This tells whether we're about to connect via udp or tcp.
    */
    TBool iProtoTCP;
    
    /**
    * This tells whether receiving data from socket is handled by CCRSockReader or used.
    */
    TBool iIssueRead;
    
    /**
    * This tells the port we're about to connect.
    */
    TUint iPort;
    
    /**
    * This tells the port we're binding locally.
    */
    TUint iLocalPort;
    
    /**
    * This is where we keep the data received.
    */
    HBufC8* iReceivedDataBuf;
    
    /**
    * Pointer to received data buffer.
    */
    TPtr8 iReceivedData;
    
    /**
    * This is where we keep the data being sent.
    */
    HBufC8* iSentDataBuf;

    /**
    * Pointer to send data buffer.
    */
    TPtr8 iSentData;  
    
    /**
    * This tells how much data we got.
    */
    TSockXfrLength iReceivedDataLen;
    
    /**
    * This tells how much data we sent.
    */
    TSockXfrLength iSentDataLen;        
    
    /** 
    * This tells where the packet was received from.
    */
    TInetAddr iFromAddr;
    
    /**
    * This tells if we've been listening or receiving in the past
    */
    TBool iWasListening;
    
    /**
    * This is instance of a helper class doing the reading part.
    */
    CCRSockReader *iReader;
    
    /**
    * This tells where to send UDP packets.
    */
    TInetAddr iToAddr;
    
    /**
    * Connection to use.
    */
    RConnection& iConnection;
        
#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE ) 
    TUint sendCount; 
    TUint sendBytes; 
#endif // LIVE_TV_FILE_TRACE || LIVE_TV_RDEBUG_TRACE

    };

/**
* Class for live tv socket "client" e.g. the user of class CCRSock.
*/
class MCRSockObserver
    {

public:

    /**
    * This method is called after some data has been received from socket.
    * @since Series 60 3.0
    * @param aData is descriptor containing the data received. 
    *        ownership of data is not passed via this call. 
    * @return none.
    */
    virtual void DataReceived( TInt aSockId,
                               const TDesC8& aData ) = 0;

    /**
    * This method is called after status of socket changes.
    * @since Series 60 3.0
    * @param aSockId a socket id.
    * @param aStatus is sock status.
    * @param aError a error code.
    * @return none
    */
    virtual void SockStatusChange( TInt aSockId,
                                   CCRSock::TCRSockStatus aStatus,
                                   TInt aError ) = 0;
    };

#endif // CCRSOCK_H

// End of file

