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
* Description:    Wrap rconnection*
*/




// INCLUDE FILES
#include "CCRConnection.h"
#include <e32msgqueue.h>
#include <ipvideo/CRTypeDefs.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
// None.

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRConnection::CCRConnection
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCRConnection::CCRConnection( RSocketServ& aSockServer )
  : CActive( EPriorityStandard ), 
    iSockServer( aSockServer ),
    iState( CCRConnection::EIdle )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRConnection::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCRConnection* CCRConnection::NewL( RSocketServ& aSockServer )
    {
    CCRConnection* self = new( ELeave ) CCRConnection( aSockServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRConnection::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRConnection::ConstructL()
    {
    CActiveScheduler::Add( this );
    User::LeaveIfError( iConnection.Open( iSockServer ) );
    User::LeaveIfError( iConMon.ConnectL() );

    // Request bearer changes events from RConnectionMonitor
    User::LeaveIfError( iConMon.NotifyEventL( *this ) );
    }

// -----------------------------------------------------------------------------
// CCRConnection::~CCRConnection
// Destructor.
// -----------------------------------------------------------------------------
//
CCRConnection::~CCRConnection()
    {
    LOG( "CCRConnection::~CCRConnection()" );

    if ( iConMonProgressNotifyPending && iConnection.SubSessionHandle() )
        {
        LOG( "~CCRConnection CancelProgressNotification" );
        iConnection.CancelProgressNotification();
        }
    
    Cancel();
    iConMon.CancelNotifications();
    iConMon.Close();
    CloseRConnection();
    iObservers.Close();
    }

// -----------------------------------------------------------------------------
// CCRConnection::Start
// -----------------------------------------------------------------------------
//      
void CCRConnection::Attach( TUint aConnectionId ) 
    {
    LOG1( "CCRConnection::Attach: aConnectionId: %u", aConnectionId );
    Cancel();
    iCurrentConnectionId = aConnectionId;
    iConMon.GetConnectionCount( iConnectionCount, iStatus );
    SetActive();
    iState = CCRConnection::EFindingAP;
    }

// -----------------------------------------------------------------------------
// CCRConnection::DoCancel
// If we need to cancel.
// -----------------------------------------------------------------------------
//
void CCRConnection::DoCancel()
    {
    LOG( "CCRConnection::DoCancel" );
    
    if ( iState == CCRConnection::EFindingAP )
        {
        LOG( "CCRConnection::DoCancel in EFindingAP" );
        iConMon.CancelAsyncRequest( EConnMonGetConnectionCount );
        }
    else if ( iState == CCRConnection::EFindingBearer )
        {
        LOG( "CCRConnection::DoCancel in EFindingBearer" ); 
        iConMon.CancelAsyncRequest( EConnMonGetIntAttribute );
        }
    else if ( iState == CCRConnection::EOpen && iConnection.SubSessionHandle() )
        {
        LOG( "CCRConnection::DoCancel in EOpen" );
        iConMonProgressNotifyPending = EFalse;
        iConnection.CancelProgressNotification();
        }
    else if ( iState == CCRConnection::EConnecting )
        {
        LOG( "CCRConnection::DoCancel in EConnecting" );
        // How to cancel a RConnection::Start?
        }
    
    LOG( "CCRConnection::DoCancel() out" );
    }

// -----------------------------------------------------------------------------
// CCRConnection::RunL
// Request succesful completion.
// -----------------------------------------------------------------------------
//
void CCRConnection::RunL()
    {
    LOG2( "CCRConnection::RunL(), iState: %d, iStatus: %d", 
        ( TInt )iState, iStatus.Int() );
    
    // Not pending any longer
    iConMonProgressNotifyPending = EFalse;
    
    // Handle state
    TInt err( KErrNone );   
    if ( iStatus.Int() == KErrNone )
        {
        if (  iState == CCRConnection::EFindingAP )
            {
            err = FindApL();
            }
    	else if ( iState == CCRConnection::EFindingBearer )
            {
            FindBearerL();
            }
        else if ( iState == CCRConnection::EOpen )
            {
            NotificationL();
            }
        }
    else
        {
        SendConnectionErrorToQueue( iStatus.Int() );
        }

    // Verify status
    if ( err )
        {
        LOG1( "CCRConnection::RunL(), err: %d", err );  
        SendConnectionErrorToQueue( err );
        }
    }

// -----------------------------------------------------------------------------
// CCRConnection::SendConnectionErrorToQueue
// -----------------------------------------------------------------------------
//
void CCRConnection::SendConnectionErrorToQueue( TInt aError )
    {
    LOG1( "CCRConnection::SendConnectionErrorToQueue() aError: %d", aError );
    
    iState = CCRConnection::EIdle;
    iCurrentConnectionId = 0;
    iBearerType = EBearerUnknown;
	CloseRConnection();
    MCRConnectionObserver::TCRConnectionStatus status;
    status = MCRConnectionObserver::ECRConnectionError;
    
    for ( TInt i( 0 ); i < iObservers.Count(); i++ )
        {
        iObservers[i]->ConnectionStatusChange( 0, status, aError );
        }
    }

// -----------------------------------------------------------------------------
// CCRConnection::RunError
// If anything goes wrong.
// -----------------------------------------------------------------------------
//
TInt CCRConnection::RunError( TInt aError )
    {
    LOG1( "CCRConnection::RunError: aError %d", aError );
    ( void )aError; // Prevent compiler warning
    iState = CCRConnection::EIdle;
    iCurrentConnectionId = 0;
    iBearerType = EBearerUnknown;
    CloseRConnection();
    return KErrNone; 
    }

// -----------------------------------------------------------------------------
// CCRConnection::CloseRConnection
// -----------------------------------------------------------------------------
//
void CCRConnection::CloseRConnection()
    {
    LOG( "CCRConnection::CloseRConnection()" );

    if ( iConnection.SubSessionHandle() )
    	{
    	iConnection.Close();
    	}
    }

// -----------------------------------------------------------------------------
// CCRConnection::Connection
// Returns the connection.
// -----------------------------------------------------------------------------
//
RConnection& CCRConnection::Connection ( void ) 
    {
    return iConnection; 
    }
    
// -----------------------------------------------------------------------------
// CCRConnection::State
// Returns state of connection.
// -----------------------------------------------------------------------------
//
CCRConnection::TConnectionState CCRConnection::State( void ) const 
    {
    return iState;  
    }

// -----------------------------------------------------------------------------
// CCRConnection::BearerType
// -----------------------------------------------------------------------------
//
TConnMonBearerType CCRConnection::BearerType() const
    {
    return iBearerType;
    }

// -----------------------------------------------------------------------------
// CCRConnection::RegisterObserver
// -----------------------------------------------------------------------------
//
TInt CCRConnection::RegisterObserver( MCRConnectionObserver* aObserver )
    {
    LOG( "CCRConnection::RegisterObserver" );
    return iObservers.Append( aObserver );
    }

// -----------------------------------------------------------------------------
// CCRConnection::UnregisterObserver
// -----------------------------------------------------------------------------
//
TInt CCRConnection::UnregisterObserver( MCRConnectionObserver* aObserver )
    {
    LOG( "CCRConnection::UnregisterObserver" );
    TInt pos = iObservers.Find( aObserver );
    if ( pos < KErrNone ) 
        {
        return pos;
        }

    iObservers.Remove( pos );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CCRConnection::GetHeuristic
// -----------------------------------------------------------------------------
//
TBool CCRConnection::GetHeuristic( TConnectionHeuristic aHeuristic )
    {
    TInt bit( 1 << ( TInt )aHeuristic );
    return ( TBool )( iHeuristics & bit );
    }

// -----------------------------------------------------------------------------
// CCRConnection::SetHeuristic
// -----------------------------------------------------------------------------
//
void CCRConnection::SetHeuristic( TConnectionHeuristic aHeuristic, TBool aValue )
    {
    TInt mask( 1 << ( TInt )aHeuristic );
    if ( aValue )
        {
        iHeuristics |= mask;
        }
    else
        {
        iHeuristics &= ~mask;
        }
    }

// -----------------------------------------------------------------------------
// CCRConnection::EventL
// -----------------------------------------------------------------------------
//
void CCRConnection::EventL( const CConnMonEventBase& aEvent )
    {
	// bearer change events
    if( aEvent.EventType()==EConnMonBearerChange && iState==EOpen )
        {
        // IMPORTANT: EConnMonBearerChange event report changes in *some* connection, not
        // necessarly ours and aEvent.ConnectionId() doest *not* contain plain 'connection id',
        // it has 'bearer id'. So make a new bearertype query to make sure it's ours.
        LOG2( "CCRConnection::EventL: bearer changed, id=%d, bearer=%d", 
            aEvent.ConnectionId(), ( ( CConnMonBearerChange* )( &aEvent) )->Bearer() );

        // Cancel ongoing requests
        if ( IsActive() )
            {
            Cancel();
            }

        iState = CCRConnection::EFindingBearer;
        iConMon.GetIntAttribute( iCurrentConnectionId, 0, KBearer,
                                 ( TInt& )iNewBearerType, iStatus );
		SetActive();
        }
    // other unhandled events
    else
        {
        LOG2( "CCRConnection::EventL: unknown event=%d, connection=%d",
                              aEvent.EventType(), aEvent.ConnectionId() );
        }
    }

// -----------------------------------------------------------------------------
// CCRConnection::IsBearerWLANor3G
// -----------------------------------------------------------------------------
//
TBool CCRConnection::IsBearerWLANor3G( TConnMonBearerType aBearer )
    {
    return aBearer == EBearerWCDMA    ||  // from CIptvNetworkEngine::IsBearer3GOrWLAN,
           aBearer == EBearerWLAN     ||  // EBearerLAN is returned by emulator
           aBearer == EBearerCDMA2000 ||
           aBearer == EBearerLAN;
    }

// -----------------------------------------------------------------------------
// CCRConnection::MaximumBandwidth
// -----------------------------------------------------------------------------
//
TInt CCRConnection::MaximumBandwidth()
    {
    // Determine bandwidth based on bearer or from ConnectionMonitor attributes
    // (KMaximumBitrateDownlink, KGuaranteedBitrateDownlink) when QoS is supported
    TConnMonBearerType bearer = BearerType();
    TInt bandwidth( 0 );

    switch( bearer )
        {
        case EBearerGPRS:
            bandwidth = KCRBandwidthGPRS;
            break;
        
        case EBearerEdgeGPRS:
            bandwidth = KCRBandwidthEdgeGPRS;
            break;
        
        case EBearerWCDMA:
            bandwidth = KCRBandwidthWCDMA;
            break;
        
        case EBearerWLAN:
            bandwidth = KCRBandwidthWLAN;
            break;
        
        case EBearerLAN:
            bandwidth = KCRBandwidthLAN;
            break;
        
        default:
            // None
            break;
        }

    return bandwidth;
    }

// -----------------------------------------------------------------------------
// CCRConnection::FindApL
// Request succesful completion.
// -----------------------------------------------------------------------------
//
TInt CCRConnection::FindApL()
    {
    CloseRConnection();
    TInt err( iConnection.Open( iSockServer ) );
    if ( err )
        {
        LOG1( "CCRConnection::FindApL(), Open iConnection err: %d", err );
        return err;
        }
    
    // Connections
    TUint foundConnections( 0 );
    err = iConnection.EnumerateConnections( foundConnections );
    if ( err )
        {
        LOG1( "CCRConnection::FindApL(), EnumerateConnections fail: %d", err );
        return err;
        }

    // Active connection found
    TUint conId( 0 ); // connection id
    TUint subId( 0 ); // subconnection id
    TUint conToUse( 0 );
    for ( TInt i( 1 ); i <= foundConnections && !err; i++ )
        {
        err = iConMon.GetConnectionInfo( i, conId, subId );
        if ( !err && conId == iCurrentConnectionId ) 
            {
            conToUse = i;   
            i = foundConnections + 1; // break from loop
            }
        }
    if ( err )
        {
        LOG1( "CCRConnection::FindApL() GetConnectionInfo loop err: %d", err );
        return err;
        }
    
    // Connection info
    TPckgBuf<TConnectionInfo> info;
    err = iConnection.GetConnectionInfo( conToUse, info );
    if ( err )
        {
        LOG1( "CCRConnection::FindApL(), GetConnectionInfo 2 fail: %d", err );
        return err;
        }

    // Attach
    err = iConnection.Attach( info, RConnection::EAttachTypeNormal );
    if ( err )
        {
        LOG1( "CCRConnection::FindApL(), Attach failed: %d", err );
        }
    
    // Bearer type
    iState = CCRConnection::EFindingBearer;
    iBearerType = EBearerUnknown;
    iConMon.GetIntAttribute( 
        iCurrentConnectionId, 0, KBearer, ( TInt& )iNewBearerType, iStatus );
    SetActive();
    return err;
    }

// -----------------------------------------------------------------------------
// CCRConnection::FindBearerL
// Request succesful completion.
// -----------------------------------------------------------------------------
//
void CCRConnection::FindBearerL()
    {
    LOG2( "CCRConnection::FindBearerL(), iCurrentConnectionId: %d, iNewBearerType: %d",
        iCurrentConnectionId, iNewBearerType );

    iState = CCRConnection::EOpen;
    TBool was3g( IsBearerWLANor3G( iBearerType ) );
    TBool is3g( IsBearerWLANor3G( iNewBearerType ) );
    TBool genChanged( was3g^is3g );
    TBool justConnected( iBearerType == EBearerUnknown );
    iBearerType = iNewBearerType;

    // Notify if connection has just gone up
    if ( justConnected )
        {
        // Broadcast IapUp&Running for observers
        MCRConnectionObserver::TCRConnectionStatus status;
        status = MCRConnectionObserver::ECRAttachCompleted;
        for ( TInt i( 0 ); i < iObservers.Count(); i++ )
            {
            iObservers[i]->ConnectionStatusChange( 0, status, iStatus.Int() );
            }
        }

    // Notify if connection has just gone up or generation changed
    if ( genChanged )
        {
        // Broadcast IapUp&Running for observers
        MCRConnectionObserver::TCRConnectionStatus status;
        status = MCRConnectionObserver::ECRBearerChanged;
        for( TInt i=0 ; i<iObservers.Count() ; i++ )
            {
            iObservers[i]->ConnectionStatusChange( 0, status, iStatus.Int() );
            }
        }
    if ( iConnection.SubSessionHandle() )
        {
        // Request connection progress notifications from RConnection
        iConMonProgressNotifyPending = ETrue;
        iConnection.ProgressNotification( iNotification, iStatus );
        SetActive();    
        }
    }

// -----------------------------------------------------------------------------
// CCRConnection::NotificationL
// Request succesful completion.
// -----------------------------------------------------------------------------
//
void CCRConnection::NotificationL()
    {
    TNifProgress notification = iNotification();
    LOG2( "CCRConnection::NotificationL stage: %d err: %d",
        ( TInt )( notification.iStage ), ( TInt )( notification.iError ) );

    if ( notification.iError != KErrNone ) 
        {
        // Notify UI
        iState = CCRConnection::EIdle;
    
        MCRConnectionObserver::TCRConnectionStatus status;
        status = MCRConnectionObserver::ECRIapDown;
        for ( TInt i( 0 ); i < iObservers.Count(); i++ )
            {
            iObservers[i]->ConnectionStatusChange( 0, status, notification.iError );
            }
    
        if ( iConnection.SubSessionHandle() )
            {
            iConnection.CancelProgressNotification();
            iConMonProgressNotifyPending = EFalse;
            CloseRConnection();
            }
        
        iCurrentConnectionId = 0;
        iBearerType = EBearerUnknown;   
        }
    else
        {
        switch ( notification.iStage ) 
            {
            case KLinkLayerOpen:
                // this means we're open
                break;
    
            case KConfigDaemonStartingDeregistration:
            case KConfigDaemonUnloading:
            case KConfigDaemonUnloaded:
            case KLinkLayerClosed:
                // And we're closed or closing
                break;
    
            default:
                // Do nothing in rest of the cases
                break;
            }
    
        if ( iConnection.SubSessionHandle() )
            {
            iConnection.ProgressNotification( iNotification, iStatus );
            iConMonProgressNotifyPending = ETrue;
            SetActive();    
            }
        }
    }

//  End of File
