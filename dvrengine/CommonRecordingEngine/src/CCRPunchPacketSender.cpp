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
* Description:    Firewall/nat box puncher*
*/




// INCLUDE FILES
#include "CCRPunchPacketSender.h"
#include "CCRRtspPacketSource.h"
#include "videoserviceutilsLogger.h"

// CONSTANTS
// None.

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRPunchPacketSender::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCRPunchPacketSender* CCRPunchPacketSender::NewL(
    RConnection& aConnection, 
    RSocketServ& aSockServer,
    TInetAddr& aFromAddr, 
    TInetAddr& aRemoteAddr , 
    TUint32 aMySSRC,
    CCRRtspPacketSource& aOwner )
    {
    CCRPunchPacketSender* self = new( ELeave ) CCRPunchPacketSender(
        aConnection, aSockServer, aFromAddr, aRemoteAddr, aMySSRC, aOwner );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRPunchPacketSender::CCRPunchPacketSender
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCRPunchPacketSender::CCRPunchPacketSender( 
    RConnection& aConnection, 
    RSocketServ& aSockServer,
    TInetAddr& aFromAddr, 
    TInetAddr& aRemoteAddr , 
    TUint32 aMySSRC,
    CCRRtspPacketSource& aOwner )
  : iConnection( aConnection ),
    iSockServer( aSockServer ),
    iFromAddr( aFromAddr ),
    iRemoteAddr( aRemoteAddr ),
    iMySSRC( aMySSRC ),
    iOwner( aOwner )
    {  
    // None
    }

// -----------------------------------------------------------------------------
// CCRPunchPacketSender::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRPunchPacketSender::ConstructL()
    {
    iSock1 = CCRSock::NewL( *this, 1, iConnection, iSockServer, EFalse, EFalse );
    User::LeaveIfError( iSock1->ConnectSock( iRemoteAddr, iFromAddr.Port() ) );
    iSock2 = CCRSock::NewL( *this, 2, iConnection, iSockServer, EFalse, EFalse );
    TInetAddr remoteAddr2 = iRemoteAddr; 
    remoteAddr2.SetPort ( iRemoteAddr.Port() + 1 ); 
    User::LeaveIfError( iSock2->ConnectSock( remoteAddr2, iFromAddr.Port() + 1 ) );
    iCleanUp = new ( ELeave ) CAsyncCallBack( CActive::EPriorityStandard ) ; 
    }

// -----------------------------------------------------------------------------
// CCRPunchPacketSender::~CCRPunchPacketSender
// Destructor.
// -----------------------------------------------------------------------------
//
CCRPunchPacketSender::~CCRPunchPacketSender()
    {
    LOG( "CCRPunchPacketSender::~CCRPunchPacketSender" );
    
    delete iSock1;
    delete iSock2; 
    delete iCleanUp; 
    }

// -----------------------------------------------------------------------------
// CCRPunchPacketSender::DataReceived
//
// This is called when data is received from socket.
// -----------------------------------------------------------------------------
//
void CCRPunchPacketSender::DataReceived( TInt /*aSockId*/, const TDesC8& /*aData*/ ) 
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRPunchPacketSender::SockStatusChange
//
// This is called when socket status changes.
// -----------------------------------------------------------------------------
//
void CCRPunchPacketSender::SockStatusChange(
    TInt aSockId,
    CCRSock::TCRSockStatus aStatus,
    TInt aError )  
    {
    if ( aStatus == CCRSock::EFailed )
        {
        LOG3( "CCRPunchPacketSender::SockStatusChange(), aSockId: id: %d, aStatus: %d, aError: %d",
            aSockId, ( TInt )aStatus, aError );
        iOwner.SockStatusChange( aSockId, aStatus, aError );
        }
    else if ( aStatus == CCRSock::EIdle )
        {       
        if ( iSentViaSock2 && iSentViaSock1 )
            {
            
            if ( !iCleanUp->IsActive() ) 
                {
                TCallBack cb( CleanupCallBack, this );
                iCleanUp->Set( cb );
                iCleanUp->CallBack();
                }               
            }
        else
            {
            // here send
            TDesC8* packet = NULL;
            if ( iMySSRC ) 
                {
                // construct valid packet only if we have SSRC
                TBuf8<8> receiverReport; 
                receiverReport.Zero(); 
                receiverReport.AppendFill( 0x0, 7 ); 
                TUint8 *rrPtr = const_cast<TUint8 *>( receiverReport.PtrZ() );
                rrPtr[0] = 0x80; // version and count
                rrPtr[1] = 0xC9; // packet type 201 = rr
                rrPtr[2] = 0x00; // packet len high bits = 0 
                rrPtr[3] = 0x01; // packet len low bits = 1 e.g. len = 1
                BigEndian::Put32( ( TUint8* )( &rrPtr[4] ), iMySSRC );
                packet = &receiverReport;
                }
            else
                {
                // Atleast construct a kind-of valid packet.
                TBuf8<12> appPacket; 
                appPacket.Zero(); 
                appPacket.AppendFill( 0x0, 11 ); 
                TUint8 *rrPtr = const_cast<TUint8 *>( appPacket.PtrZ() );
                rrPtr[0] = 0x80; // version and subtype
                rrPtr[1] = 0xCC; // packet type 204 = APP
                rrPtr[2] = 0x00; // packet len high bits = 0 
                rrPtr[3] = 0x01; // packet len low bits = 1 e.g. len = 1
                // this is not a valid SSRC
                BigEndian::Put32( ( TUint8* )( &rrPtr[4] ), iMySSRC );
                rrPtr[8] = 0x44; // ASCII: D
                rrPtr[9] = 0x56; // ASCII: V
                rrPtr[10] = 0x52; // ASCII: R
                rrPtr[11] = 0x45; // ASCII: E
                packet = &appPacket;
                }
            if ( aSockId == 1 && iSock1 && !iSentViaSock1 )
                {
                iSock1->SendData( *packet ); 
                iSentViaSock1 = ETrue;
                }
            else if ( aSockId == 2 && iSock2 && !iSentViaSock2 )
                {
                iSock2->SendData( *packet ); 
                iSentViaSock2 = ETrue;            
                }
            else
                {
                // None
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// CCRPunchPacketSender::CleanupCallBack()
//-----------------------------------------------------------------------------
TInt CCRPunchPacketSender::CleanupCallBack( TAny* aSelf ) 
    {
    LOG( "CCRPunchPacketSender::CleanupCallBack()" );

    CCRPunchPacketSender* self = static_cast<CCRPunchPacketSender*>( aSelf );  
    delete self->iSock1; self->iSock1 = NULL; 
    delete self->iSock2; self->iSock2 = NULL; 
    self->iOwner.PunchPacketsSent( self );
    return KErrNone; 
    }

//  End of File
