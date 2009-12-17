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
* Description:    Handles tcp/udp socket*
*/



 
// INCLUDE FILES
#include "CCRSock.h"
#include "videoserviceutilsLogger.h"

// CONSTANTS
_LIT( KCRSockLocalhost, "127.0.0.1" );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRSock::CCRSock
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCRSock::CCRSock(
    MCRSockObserver& aObserver,
    TInt aSockId, 
    RConnection& aConnection,
    RSocketServ& aSockServer, 
    TBool aProtoTCP,
    TBool aIssueRead )
  : CActive( CActive::EPriorityStandard ),
    iSockServer( aSockServer ),
    iSockStatus( CCRSock::EInitNeeded ),
    iObserver( aObserver ),
    iSockId( aSockId ),
    iProtoTCP( aProtoTCP ),
    iIssueRead( aIssueRead ),
    iReceivedData( NULL, 0 ),
    iSentData( NULL, 0 ),
    iConnection( aConnection )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRSock::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCRSock* CCRSock::NewL(
    MCRSockObserver& aObserver,
    TInt aSockId, 
    RConnection& aConnection,
    RSocketServ& aSockServer,
    TBool aProtoTCP,
    TBool aIssueRead )
    {
    CCRSock* self = new( ELeave ) CCRSock( aObserver, aSockId, aConnection,
                                           aSockServer, aProtoTCP, aIssueRead );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRSock::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRSock::ConstructL()
    {
    LOG( "CCRSock::ConstructL() in" );
    
    iReceivedDataBuf = HBufC8::NewL( KMaxDataSize );
    iReceivedData.Set( iReceivedDataBuf->Des() );
    iSentDataBuf = HBufC8::NewL( KMaxDataSize );
    iSentData.Set( iSentDataBuf->Des() );
    
    // Add self to active scheduler
    CActiveScheduler::Add( this );
    if ( iIssueRead )
        {
        iReader = CCRSockReader::NewL( *this, iConnection, iSockServer );
        }
    
    iToAddr.SetPort( 0 );
    
    LOG( "CCRSock::ConstructL() out" );
    }

// -----------------------------------------------------------------------------
// CCRSock::~CCRSock
// Destructor.
// -----------------------------------------------------------------------------
//
CCRSock::~CCRSock()
    {
    LOG( "CCRSock::~CCRSock()" );
    
    CleanUp();
    delete iReader;
    delete iSentDataBuf;
    delete iReceivedDataBuf;
    }


// -----------------------------------------------------------------------------
// CCRSock::RunL
// "Brain"
// -----------------------------------------------------------------------------
//
void CCRSock::RunL() 
    {
    TInt err( KErrNone ); 

    if ( iStatus == KErrEof && iWasListening )
        {
        iSocket.Close();
        err = iSocket.Open( iSockServer );
        if ( err == KErrNone )
            {   
            LOG1( "CCRSock::RunL(), reopening sock: %d for listen", iSockId );
            iIsiSocketOpen = ETrue; 
            iListenSocket.Accept( iSocket, iStatus );
            iSockStatus = CCRSock::EListening;              
            SetActive(); 
            }
        else
            {
            LOG2( "CCRSock::RunL(), iSocket.Open FAILED id: %d err: %d",
                                                        iSockId, err );
            }
        }
    else
        {
        switch ( iSockStatus )
            {
        case EResolving: // in connection, this is usually 1st time to come to RunL
            if ( iStatus == KErrNone )
                { // host name found
                iHostAddress().iAddr.SetPort( iPort );
                iSocket.Close();
                err = iSocket.Open( iSockServer, 
                                   KAfInet, 
                                   iProtoTCP ? KSockStream : KSockDatagram, 
                                   iProtoTCP ? KProtocolInetTcp : KProtocolInetUdp,
                                   iConnection )                ;
                if ( err )
                    {
                    iSockStatus = CCRSock::EFailed;
                    iObserver.SockStatusChange( iSockId, iSockStatus, err );
                    iResolver.Close();
                    LOG2( "CCRSock::RunL(), iSockId: %d, err: %d", iSockId, err );
                    }
                else
                    {
                    iIsiSocketOpen = ETrue; 
                    if ( iLocalPort > 0 )
                        {
                        TInetAddr bindAddr( KInetAddrAny, iLocalPort );
                        err = iSocket.Bind( bindAddr ); 
                        if ( err != KErrNone ) 
                            {
                            LOG2( "CCRSock::ConnectSock(), Bind FAILED, Id: %d, err %d", iSockId, err ); 
                            }
                        }
                    
                    LOG2( "CCRSock::RunL(), iSockId: %d, port: %d",
                                            iSockId, iHostAddress().iAddr.Port() );
                    iSocket.Connect( iHostAddress().iAddr, iStatus );
                    iToAddr = iHostAddress().iAddr;
                    err = iSocket.SetOpt( KSOBlockingIO, KSOLSocket);
                    if ( err != KErrNone )
                        {
                        LOG1( "CCRSock::RunL(), iSocket.SetOpt FAILED: %d", err );
                        }
                    iSockStatus = CCRSock::EConnecting;
                    iObserver.SockStatusChange( iSockId, iSockStatus, err );
                    SetActive();
                    iResolver.Close();
                    }
                }
            else
                { // resolving not ok
                iSockStatus = CCRSock::EFailed;
                iObserver.SockStatusChange( iSockId, iSockStatus, iStatus.Int() );
                iResolver.Close();
                }
            break;
        
        case EConnecting:
            if ( iStatus == KErrNone ) // success
                {
                iSockStatus = CCRSock::EIdle;
                // next action is up to user, don't do SetActive here.
                LOG1( "CCRSock::RunL(), iSockId: %d", iSockId );
                    
                if ( iIssueRead && iReader && ( !iReader->IsActive() ) ) 
                    {
                    iReader->IssueRead();
                    }
                }
            else
                {
                iSockStatus = CCRSock::EFailed;
                iObserver.SockStatusChange( iSockId, iSockStatus, iStatus.Int() );
                CleanUp(); /* close everything */
                }       
            iObserver.SockStatusChange( iSockId, iSockStatus, iStatus.Int() );
            break;
        
        case ESending:
            // send has been finished,somehow:
            if ( iStatus == KErrNone ) // success
                {
#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE ) 
                sendBytes += iSentDataLen();
                sendCount ++;
                if ( ( sendCount % 50 ) == 0 )
                    {
                    LOG3( "CCRSock::RunL(), sendCount: %d, sendBytes: %d, iSockId: %d",
                                            sendCount, sendBytes, iSockId );
                    }
#endif                  
                iSockStatus = CCRSock::EIdle;       
                // next action is up to user, don't do SetActive here.
                }
            else
                {
                iSockStatus = CCRSock::EFailed;
                CleanUp(); /* close everything */
                }
            iObserver.SockStatusChange( iSockId, iSockStatus, iStatus.Int() );
            break;
        
        case EListening:
            if ( iStatus == KErrNone ) // success, da zocket is open
                {
                iSockStatus = CCRSock::EIdle;       
                if ( iIssueRead && iReader && ( !iReader->IsActive() ) ) 
                    {
                    iReader->IssueRead();               
                    }
                }
            else
                {
                iSockStatus = CCRSock::EFailed;
                CleanUp(); /* close everything */
                }
            iObserver.SockStatusChange( iSockId, iSockStatus, iStatus.Int() );
            break;                                                              
        
        default:
            __ASSERT_DEBUG( 1==2, User::Panic( _L("CRRTP"), KErrArgument) );
            break; /* this should not happend? */       
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRSock::DoCancel
// Cancels pending actions
// -----------------------------------------------------------------------------
//
void CCRSock::DoCancel() 
    {
    LOG( "CCRSock::DoCancel() in" );
    if ( iIsiSocketOpen )
        {
        iSocket.CancelAll();
        }
    if ( iIsiListenSocketOpen )
        {
        iListenSocket.CancelAll(); 
        }
    
    LOG( "CCRSock::DoCancel() out" );
    }

// -----------------------------------------------------------------------------
// CCRSock::CopySendData
// Handles send buffer size.
// -----------------------------------------------------------------------------
//
void CCRSock::CopySendData( const TDesC8& aData ) 
    {
    if ( aData.Length() > iSentData.MaxLength() )
        {
        // Alloc more than 8k
        delete iSentDataBuf; iSentDataBuf = NULL;
        iSentDataBuf = HBufC8::New( aData.Length() );
        iSentData.Set( iSentDataBuf->Des() );
        }
    else
        {
        if ( iSentData.MaxLength() > KMaxDataSize &&
             aData.Length() <= KMaxDataSize  )
            {
            // Back to 8k if not more needed
            delete iSentDataBuf; iSentDataBuf = NULL;
            iSentDataBuf = HBufC8::New( KMaxDataSize );
            iSentData.Set( iSentDataBuf->Des() );
            }
        }
    
    iSentData.Copy( aData );
    }
    
// -----------------------------------------------------------------------------
// CCRSock::CleanUp
// Performs cleanup
// -----------------------------------------------------------------------------
//
void CCRSock::CleanUp() 
    {
    LOG( "CCRSock::CleanUp() in" );
    Cancel();

    iResolver.Close();
    iSocket.Close();
    iListenSocket.Close(); 
    iSockStatus = CCRSock::EInitNeeded;

    iIsiSocketOpen = EFalse;    
    iIsiListenSocketOpen = EFalse;
    LOG( "CCRSock::CleanUp() out" );
    }

// -----------------------------------------------------------------------------
// CCRSock::RunError
// Q: Is anything wrong
// A: Thanks for asking. About everything. 
// -----------------------------------------------------------------------------
//
TInt CCRSock::RunError( TInt aError )
    {
    LOG1( "CCRSock::RunError(), aError: %d", aError );
    ( void )aError; // Prevent compiler warning

    return KErrNone;        
    }
    
// -----------------------------------------------------------------------------
// CCRSock::ConnectSock
// Initiates connection to remote addr. 
// -----------------------------------------------------------------------------
//
TInt CCRSock::ConnectSock(
    const TDesC& aAddr,
    TUint aPort,
    TInt aLocalPort ) 
    {
    LOG( "CCRSock::ConnectSock()" );
    
    TInt retval( KErrNone );
    if ( IsActive() ) 
        {
        retval = KErrInUse;
        }
    else
        {
        iWasListening = EFalse;
        iPort = aPort;
        iLocalPort = aLocalPort; 
        if ( aAddr.Compare( KCRSockLocalhost() ) != 0 ) 
            {
            iResolver.Close();
            if ( (retval = iResolver.Open( iSockServer, KAfInet, 
                                           KProtocolInetTcp, iConnection) ) == KErrNone )
                {
                iResolver.GetByName( aAddr, iHostAddress, iStatus ); 
                iSockStatus = CCRSock::EResolving;
                SetActive();
                }
            else
                {
                LOG2( "CCRSock::ConnectSock(), Resolver.Open id: %d, err: %d",
                                                        iSockId, retval ); 
                iSockStatus = CCRSock::EFailed;             
                }
            }
        else
            { // localhost, no need to resolve
            iHostAddress().iAddr.SetPort( iPort );
            iSocket.Close();
            retval = iSocket.Open( iSockServer, 
                                  KAfInet, 
                                  iProtoTCP ? KSockStream : KSockDatagram, 
                                  iProtoTCP ? KProtocolInetTcp : KProtocolInetUdp,
                                  iConnection )             ;
            if ( retval )
                {
                LOG2( "CCRSock::ConnectSock(), Socket.Open id: %d, err: %d",
                                                      iSockId, retval );
                iSockStatus = CCRSock::EFailed;
                }
            else
                {
                iIsiSocketOpen = ETrue;
                iSockStatus = CCRSock::EConnecting;
                if ( aLocalPort > 0 )
                    {
                    TInetAddr bindAddr( KInetAddrAny, aLocalPort );
                    TInt err( iSocket.Bind( bindAddr ) );
                    if ( err != KErrNone )
                        {
                        LOG2( "CCRSock::ConnectSock(), Bind FAILED iSockId: %d, err: %d",
                                                                   iSockId, err );
                        }
                    }
                iToAddr = TInetAddr( KInetAddrLoop, aPort );
                LOG2( "CCRSock::ConnectSock(), iSockId %d port %d",
                                               iSockId, aPort );
                iSocket.Connect( iToAddr, iStatus );
                SetActive();
                if ( iProtoTCP ) 
                    {
                    retval = iSocket.SetOpt( KSOBlockingIO, KSOLSocket );
                    }
                iObserver.SockStatusChange( iSockId, iSockStatus, retval );
                }
            }
        }
    
    LOG1( "CCRSock::ConnectSock(), retVal: %d", retval );
    return retval;
    }

// -----------------------------------------------------------------------------
// CCRSock::ConnectSock
// Initiates connection to remote addr without resolving. 
// -----------------------------------------------------------------------------
//
TInt CCRSock::ConnectSock(
    const TSockAddr& aAddr,
    TInt aLocalPort ) 
    {
    LOG( "CCRSock::ConnectSock(), no dns" );
    
    TInt retval( KErrNone );
    if ( IsActive() ) 
        {
        retval = KErrInUse;
        }
    else
        {
        iWasListening = EFalse;
        iPort = aAddr.Port();
        iLocalPort = aLocalPort; 
        iHostAddress().iAddr = aAddr; 
        iSocket.Close();
        retval = iSocket.Open( iSockServer, 
                              KAfInet, 
                              iProtoTCP ? KSockStream : KSockDatagram, 
                              iProtoTCP ? KProtocolInetTcp : KProtocolInetUdp,
                              iConnection ) ;       
        if ( retval )
            {
            LOG2( "CCRSock::ConnectSock(), Socket.Open id: %d, err: %d",
                                                  iSockId, retval );
            iSockStatus = CCRSock::EFailed;
            }
        else
            {
            iIsiSocketOpen = ETrue;
            iSockStatus = CCRSock::EConnecting;
            if ( aLocalPort > 0 )
                {
                TInetAddr bindAddr( KInetAddrAny, aLocalPort );
                TInt err( iSocket.Bind( bindAddr ) );
                if ( err != KErrNone )
                    {
                    LOG2( "CCRSock::ConnectSock(), Bind FAILED id: %d err: %d",
                                                               iSockId, err );
                    }
                }
            iToAddr = aAddr; 
            LOG2( "CCRSock::ConnectSock(), id: %d, port: %d", iSockId, iPort );
            iSocket.Connect( iToAddr, iStatus );
            SetActive();
            if ( iProtoTCP ) 
                {
                retval = iSocket.SetOpt( KSOBlockingIO, KSOLSocket );
                }
            iObserver.SockStatusChange( iSockId, iSockStatus, retval );
            }
        }
    
    LOG1( "CCRSock::ConnectSock(), retVal: %d", retval );
    return retval;
    }

// -----------------------------------------------------------------------------
// CCRSock::ListenPort
// Starts listening to port. Synchronous. 
// -----------------------------------------------------------------------------
//
TInt CCRSock::ListenPort( TUint aPort )
    {
    LOG1( "CCRSock::ListenPort(), aPort: %d", aPort );
    
    TInt retval( KErrNone );
    if ( IsActive() ) 
        {
        return KErrInUse;
        }
    if ( iSockStatus != CCRSock::EInitNeeded    )
        {
        return KErrNotReady;
        }
    
    iHostAddress().iAddr.SetPort( iPort );
    iWasListening = ETrue; 
    if ( iProtoTCP )
        { 
        iListenSocket.Close();
        if ( ( retval = iListenSocket.Open( iSockServer, KAfInet,
               KSockStream, KProtocolInetTcp, iConnection ) ) == KErrNone )
            {
            iIsiListenSocketOpen = ETrue; 
            TInetAddr listenAddr( KInetAddrAny, aPort );
            LOG2( "CCRSock::ListenPort(), id: %d, port: %d", iSockId,(TInt)aPort);            
            retval = iListenSocket.Bind( listenAddr );
            if ( retval == KErrNone )
                {
                retval = iListenSocket.Listen( 5 );
                if ( retval == KErrNone )
                    {
                    iSocket.Close();
                    retval = iSocket.Open( iSockServer );
                    if ( retval == KErrNone )
                        {   
                        iIsiSocketOpen = ETrue;
                        iListenSocket.Accept( iSocket, iStatus );
                        iSockStatus = CCRSock::EListening;
                        SetActive();
                        }
                    else
                        {
                        LOG1( "CCRSock::ListenPort(), iSocket.Open FAILED retval: %d", retval );
                        }
                    }
                else
                    {
                    LOG1( "CCRSock::ListenPort(), iListenSock.Listen FAILED retval: %d", retval );
                    }
                }
            else
                {
                LOG2( "CCRSock::ListenPort() iListenSocket.Bind FAILED Id: %d, retval: %d", iSockId, retval);
                }
            }
        else
            {
            LOG2( "ListenSocket.Open id: %d, err: %d", iSockId, retval );
            }   
        }
    else
        {
        // for UDP things are different: just open, bind 
        iSocket.Close();
        if ( ( retval = iSocket.Open( iSockServer, 
                                      KAfInet, 
                                      KSockDatagram, 
                                      KProtocolInetUdp,
                                      iConnection ) ) != KErrNone )
            {
            iSockStatus = CCRSock::EFailed;
            LOG2( "CCRSock::ListenPort(), UDPSocket.Open id: %d, err: %d", iSockId, retval );
            }
        else
            {
            TInetAddr listenAddr( KInetAddrAny, aPort );
            retval = iSocket.Bind( listenAddr );
            if ( retval == KErrNone )
                {
                LOG2( "CCRSock::ListenPort(), udp: %d ok, id: %d", aPort,iSockId );
                iSockStatus = CCRSock::EIdle;
                iIsiSocketOpen = ETrue;
                }
            else
                {
                LOG2( "CCRSock::ListenPort(), UDPSocket.Bind FAILED id: %d, retval: %d", iSockId, retval ); 
                iSockStatus = CCRSock::EFailed;
                }
            if ( iIssueRead && iReader && ( !iReader->IsActive() ) ) 
                {
                iReader->IssueRead();                   
                }
            }
        }
    
    LOG1( "CCRSock::ListenPort(), retval: %d", retval );
    return retval; 
    }

// -----------------------------------------------------------------------------
// CCRSock::JoinGroup
// Joins a multicast group. Synchronous.
// -----------------------------------------------------------------------------
//
TInt CCRSock::JoinGroup( const TInetAddr& aGroupAddr )
    {
    LOG( "CCRSock::JoinGroup()" );
    
    TPckgBuf<TIp6Mreq> request;
    request().iAddr = aGroupAddr.Ip6Address();
    request().iInterface = 0;
    return iSocket.SetOpt( KSoIp6JoinGroup, KSolInetIp, request );
    }

// -----------------------------------------------------------------------------
// CCRSock::SendData
// Initiates async data sending
// -----------------------------------------------------------------------------
//
void CCRSock::SendData( const TDesC8& aDataThatIsSentOverSocket )
    {
#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE ) 
    if ( iProtoTCP && aDataThatIsSentOverSocket.Length() &&
         aDataThatIsSentOverSocket[0] != ( TUint8 )( '$' ) )
        {   
        LOG2("CCRSock::SendData(), id: %d, len: %d", 
                                   iSockId, aDataThatIsSentOverSocket.Length() ); 
        TChar c;
        TName d;
        for ( TInt i( 0 ); i < aDataThatIsSentOverSocket.Length(); i++ )
            {
            c = aDataThatIsSentOverSocket[i]; 
            d.Append( c ); 
            if ( ( i > 0 ) && ( i % 80 ) == 0 )
                {
                LOG1( ">%S<", &d );             
                d.Zero(); 
                }
            }
        
        LOG1( ">%S<", &d );
        }
#endif

    // Data to socket
    if ( !IsActive() )
        {
        CopySendData( aDataThatIsSentOverSocket );
        if ( iProtoTCP )
            {       
            iSocket.Write( iSentData, iStatus );
            iSockStatus = CCRSock::ESending;
            SetActive();
            }
        else
            {
            if ( iToAddr.Port() != 0 ) 
                {
                iSocket.SendTo( iSentData, iToAddr, 0, iStatus, iSentDataLen );
                iSockStatus = CCRSock::ESending;                
                SetActive();
                }
            else
                {
                LOG1( "CCRSock::SendData(), Discarding send, id: %d" ,iSockId );
                }
            }
        }
    else
        {
        LOG2( "CCRSock::SendData(), id: %d, Already active, Dumped packet, len: %d" ,
            iSockId, aDataThatIsSentOverSocket.Length() );
        }
    }
        
// -----------------------------------------------------------------------------
// CCRSock::SockStatus
// returns status
// -----------------------------------------------------------------------------
//
CCRSock::TCRSockStatus CCRSock::SockStatus() const
    {   
    return iSockStatus; 
    }

// -----------------------------------------------------------------------------
// CCRSock::ConnectedAddr
// returns endpoint addr of this sock
// -----------------------------------------------------------------------------
//
TInetAddr CCRSock::ConnectedAddr( void )
    {
    TInetAddr addr;
    iSocket.RemoteName( addr );
    return addr;
    }

// -----------------------------------------------------------------------------
// CCRSock::LocalAddr
// returns local addr of this sock
// -----------------------------------------------------------------------------
//
TInetAddr CCRSock::LocalAddr( void )
    {
    TInetAddr addr;
    iSocket.LocalName( addr );
    return addr;
    }

// -----------------------------------------------------------------------------
// CCRSock::SetToAddr
// sets "to" addr of this sock
// -----------------------------------------------------------------------------
//
void CCRSock::SetToAddr( const TInetAddr &aAddr )
    {
    LOG( "CCRSock::SetToAddr() in" );
    iToAddr = aAddr;    

#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE ) 
    TName an_addr;
    iToAddr.Output( an_addr ); 
    LOG3( "CCRSock::SetToAddr(), id: %d, addr: %S, port: %d", iSockId, &an_addr, aAddr.Port() );
#endif
    
    }

// -----------------------------------------------------------------------------
// CCRSock::Socket
// -----------------------------------------------------------------------------
RSocket& CCRSock::Socket()
    {
    return iSocket;
    }

// -----------------------------------------------------------------------------
// ----------- here begins implementation of "SockReader" helper class----------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//

// -----------------------------------------------------------------------------
// CCRSockReader::NewL
// Construction startpoint
// -----------------------------------------------------------------------------
//
CCRSockReader* CCRSockReader::NewL(
    CCRSock& aSock, 
    RConnection& aConnection, RSocketServ& aSockServer )
    {
    CCRSockReader* self = new ( ELeave ) CCRSockReader( 
        aSock, aConnection, aSockServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;    
    }
// -----------------------------------------------------------------------------
// CCRSockReader::CCRSockReader
// Default constructor
// -----------------------------------------------------------------------------
//
CCRSockReader::CCRSockReader(
      CCRSock& aSock, RConnection& aConnection,
      RSocketServ& aSockServer ) 
    : CActive( EPriorityStandard ),
      iSock( aSock ),
      iConnection( aConnection ),
      iSockServer( aSockServer )
    {
    }
// -----------------------------------------------------------------------------
// CCRSockReader::ConstructL
// Actual constructor
// -----------------------------------------------------------------------------
//
void CCRSockReader::ConstructL() 
    {
    LOG( "CCRSockReader::ConstructL()" );
    
    // Add self to active scheduler
    CActiveScheduler::Add( this );  
    }
    
// -----------------------------------------------------------------------------
// CCRSock::~CCRSockReader
// Destructor
// -----------------------------------------------------------------------------
//
CCRSockReader::~CCRSockReader() 
    {
    LOG( "CCRSockReader::~CCRSockReader()" );
    Cancel();
    }
    
// -----------------------------------------------------------------------------
// CCRSockReader::RunL
// Work-horse
// -----------------------------------------------------------------------------
//
void CCRSockReader::RunL()
    {
#if defined ( LIVE_TV_FILE_TRACE ) || defined ( LIVE_TV_RDEBUG_TRACE ) 
    if ( iSock.iProtoTCP && iStatus.Int() != KErrNone )
        {
        LOG2( "CCRSockReader::RunL(), id: %d, status: %d", iSock.iSockId, iStatus.Int() );
        }
#endif
    
    switch ( iStatus.Int() )    
        {
        case KErrNone:
            {
#if defined( LIVE_TV_FILE_TRACE ) || defined( LIVE_TV_RDEBUG_TRACE )
            if ( !iSock.iProtoTCP ) 
                {
                recvBytes += iSock.iReceivedData.Length();
                recvCount ++;
                if ( ( recvCount % 50 ) == 0 )
                    {
                    LOG3( "CCRSockReader::RunL(), recvCount: %d, recvBytes: %d, id: %d",
                                                  recvCount, recvBytes, iSock.iSockId );
                    TName an_addr;
                    iSock.iFromAddr.Output( an_addr );
                    TInt a_byte2 = iSock.iReceivedData[2];
                    TInt a_byte3 = iSock.iReceivedData[3];          
                    LOG3( "CCRSockReader::RunL(), Addr %S, port: %d, last seq: %d",
                           &an_addr, iSock.iFromAddr.Port(), ( a_byte2 * 255 ) + a_byte3 );
                    }
                }
#endif // LIVE_TV_FILE_TRACE || LIVE_TV_RDEBUG_TRACE
            
            iSock.iObserver.DataReceived( iSock.iSockId, iSock.iReceivedData );
            IssueRead();
            }
            break;

        default: // error cases
            {
            LOG2( "CCRSockReader::RunL(), id: %d, status: %d", iSock.iSockId, iStatus.Int() );
            iSock.iSockStatus = CCRSock::EFailed;
            iSock.iObserver.SockStatusChange(
                iSock.iSockId, iSock.iSockStatus, iStatus.Int() );
            }
            break;       
        }   
    }

// -----------------------------------------------------------------------------
// CCRSockReader::IssueRead
// Asks for more data
// -----------------------------------------------------------------------------
//  
void CCRSockReader::IssueRead()
    {
    if ( IsActive() )
        {
        LOG( "CCRSockReader::IssueRead(), IsActive! return" );
        return;
        }
    
    iSock.iReceivedData.Zero();
    if ( iSock.iProtoTCP )
        {       
        iSock.iSocket.RecvOneOrMore( iSock.iReceivedData, 0, iStatus, 
                                     iSock.iReceivedDataLen );
        }
    else
        {
        iSock.iSocket.RecvFrom( iSock.iReceivedData, iSock.iFromAddr, 0, iStatus );
        }

    SetActive();
    }   
    
// -----------------------------------------------------------------------------
// CCRSockReader::DoCancel
// Cancels outstanding operations
// -----------------------------------------------------------------------------
//
void CCRSockReader::DoCancel() 
    {
    LOG( "CCRSockReader::DoCancel()" );
    // CCRSock::DoCancel() has already called CancelAll to socket so no need to do it here
    }
// -----------------------------------------------------------------------------
// CCRSockReader::RunError
// If anything goes wrong
// -----------------------------------------------------------------------------
//
TInt CCRSockReader::RunError( TInt aError )
    {
    LOG1( "CCRSockReader::RunError(), aError: %d, return KErrNone", aError  );
    ( void )aError; // Prevent compiler warning

    return KErrNone;    
    }

// -----------------------------------------------------------------------------
// ----------- here ends implementation of "SockReader" helper class----------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//
//  End of File

