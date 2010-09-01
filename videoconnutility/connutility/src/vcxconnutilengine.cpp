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
* Description: 
*
*/




#include <mpxlog.h>
#include <commdb.h>
#include <basched.h>
#include <cmmanagerext.h>
#include <extendedconnpref.h>

#include "vcxconnutilextengineobserver.h"
#include "vcxconnutilengine.h"

// CONSTANTS
_LIT( KCommDBIapId, "IAP\\Id" );


// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::NewL()
// -----------------------------------------------------------------------------
//
CVcxConnUtilEngine* CVcxConnUtilEngine::NewL( MConnUtilExtEngineObserver* aObserver )
    {
    CVcxConnUtilEngine* self = CVcxConnUtilEngine::NewLC( aObserver );  
    CleanupStack::Pop( self ); // self;
    return self;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::NewLC()
// -----------------------------------------------------------------------------
//
CVcxConnUtilEngine* CVcxConnUtilEngine::NewLC( MConnUtilExtEngineObserver* aObserver )
    {
    CVcxConnUtilEngine* self = new (ELeave) CVcxConnUtilEngine();
    CleanupStack::PushL( self );
    self->iObserver = aObserver;
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::CVcxNsConnUtilEngine()
// -----------------------------------------------------------------------------
//
CVcxConnUtilEngine::CVcxConnUtilEngine( ) : 
    CActive( EPriorityStandard ),     
    iConnectionState( EVCxNotConnected )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::ConstructL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilEngine::ConstructL()
    {
    MPX_DEBUG1("vcxconnutil  ## CVcxConnUtilEngine::ConstructL() in");
       
    iCmManagerExt.OpenL();
        
    User::LeaveIfError( iSocketServer.Connect() );
           
    User::LeaveIfError( iConnectionMonitor.ConnectL() );    
    
    User::LeaveIfError( iConnectionMonitor.NotifyEventL( *this ) );
        
    // get all destination ids
    iCmManagerExt.AllDestinationsL( iDestinationArray );
    
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::ConstructL() out" ); 
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::~CVcxNsConnUtilEngine()
// -----------------------------------------------------------------------------
//
CVcxConnUtilEngine::~CVcxConnUtilEngine()
    {
    
    if( iMobility )
        {
        // make sure observers are not being called
        // after receiving Error() from Cancel()
        iConnectionState = EVCxDisconnecting;
        iMobility->Cancel();
        delete iMobility;
        }
    
    if( IsActive() )
        {
        Cancel();
        }
    
    iConnection.Close();
        
    iSocketServer.Close();
      
    iCmManagerExt.Close();
    
    iConnectionMonitor.Close();
       
    delete iDestinationName;
    
    iDestinationArray.Close();
   
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::StartToConnect()
// -----------------------------------------------------------------------------
//
TInt CVcxConnUtilEngine::StartToConnect( TConnPref& aPref, TBool aConnAsync )
    {
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::StartToConnect with TConnSnapPref in" );
        
    TInt err( KErrNone );
    if( EVCxConnected == iConnectionState  ||
        EVCxConnecting == iConnectionState )
        {
        return err;
        }
    iConnectionState = EVCxConnecting;  
    err = iConnection.Open( iSocketServer );
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::StartToConnect connection opened" );
    if( err != KErrNone )
        {
        return err;
        }   

    if( !IsActive() )
        {
        if( aConnAsync )
            {
            MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::StartToConnect connecting async" );
            
            iConnection.Start( aPref, iStatus );
            SetActive();
            TRAP( err, iObserver->WaitL( EVCxPSConnectionStatus ) );
            if( err == KErrNone )
                {
                // save possible error code from async 
                // connection creation
                err = iConnectingResult;
                }
            else if( err == KLeaveExit )
                {
                // app is closing, active scheduler is about to be closed.
                // cancel connection waiting.
                if( IsActive() )
                    {
                    Cancel();
                    }
                }
            }
        else
            {
            MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::StartToConnect connecting sync" );
            err = iConnection.Start();
            if( err == KErrNone )
                {
                iConnectionState = EVCxConnected;
                }
            }
        }
    
    if( iConnectionState != EVCxConnected )
        {
        iIapId = 0;
        }

    MPX_DEBUG2( "vcxconnutil  ## CVcxConnUtilEngine::StartToConnect with TConnSnapPref err: %d  out", err );
    return err;  
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::CreateMobility()
// -----------------------------------------------------------------------------
//
TInt CVcxConnUtilEngine::CreateMobility()
    {
    TInt err( KErrNone );
    if( iMobility )
        {
        iMobility->Cancel();        
        delete iMobility;
        iMobility = 0;
        }
    if( !iMobility  )
        {        
        TRAP( err, iMobility = CActiveCommsMobilityApiExt::NewL( iConnection, *this ) );
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::Disconnect()
// -----------------------------------------------------------------------------
//
void  CVcxConnUtilEngine::Disconnect()
    {
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::Disconnect in");
    if( iConnectionState != EVCxNotConnected )
        { 
        MPX_DEBUG1( "CVcxConnUtilEngine::Disconnect closing connection");
        iConnectionState = EVCxDisconnecting;
        if( iMobility )
            {
            iMobility->Cancel();
            delete iMobility;
            iMobility = NULL;
            }               

        iConnection.Close();
        iConnectionState = EVCxNotConnected;
        
        }      
    if( IsActive() )
        {
        Cancel();
        }
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::Disconnect out");
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::PreferredCarrierAvailable()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilEngine::PreferredCarrierAvailable( TAccessPointInfo aOldAPInfo ,
                                                      TAccessPointInfo aNewAPInfo,
                                                      TBool /*aIsUpgrade*/,
                                                      TBool aIsSeamless )
    {
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::PreferredCarrierAvailable in");
    MPX_DEBUG2( "PreferredCarrierAvailable oldiap: %d", aOldAPInfo.AccessPoint() );
    MPX_DEBUG2( "PreferredCarrierAvailable newiap: %d", aNewAPInfo.AccessPoint() );
    MPX_DEBUG2( "PreferredCarrierAvailable seamless: %d", aIsSeamless );
    
    
    if ( !aIsSeamless && 
         aOldAPInfo.AccessPoint() > 0 &&
         aOldAPInfo.AccessPoint() != aNewAPInfo.AccessPoint() )             
        {                
        if( iConnectionState == EVCxRoamingAccepted )
            {
            // in this case we have a roaming ongoing at the
            // but we have not yet received NewCarrierActive
            // so networking has found yet better usable iap.
            // since everyone have allready accepted roaming we
            // can migrate it right away
            MPX_DEBUG1( "CVcxConnUtilEngine::PreferredCarrierAvailable  migrated due EVCxRoamingAccepted state" ); 
            iMobility->MigrateToPreferredCarrier();
            }
        else if( iConnectionState != EVCxRoamingRequest )
            {
            TBool allowRoam( EFalse );
            iConnectionState = EVCxRoamingRequest;
            
            TRAPD( err, allowRoam = iObserver->RequestIsRoamingAllowedL() );
            
            // there is a possibility, that client has disconnected during request
            // in that case, do nothing
            if( iConnectionState == EVCxNotConnected )
                {                
                return;
                }
            
            if( err == KErrNone && allowRoam )
                {
            
                MPX_DEBUG1( "CVcxConnUtilEngine::PreferredCarrierAvailable migrated" ); 
  
                iMobility->MigrateToPreferredCarrier();
                iConnectionState = EVCxRoamingAccepted;
                }
            else
                {
                MPX_DEBUG1( "CVcxConnUtilEngine::PreferredCarrierAvailable ignored" );
                iMobility->IgnorePreferredCarrier();
                iConnectionState = EVCxConnected;
                // reset IAP / SNAP info to the pubsub
                TRAPD( err, iObserver->IapChangedL() );
                if( err != KErrNone )
                    {
                    // internal error, let's try to resolve connection
                    iConnectionState = EVCxError;
                    TRAP( err, iObserver->IapChangedL() );                    
                    }
                }
            }
       else
            {
            MPX_DEBUG1( "CVcxConnUtilEngine::PreferredCarrierAvailable Allready roaming" ); 
            }
        }
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::PreferredCarrierAvailable out" );
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::NewCarrierActive()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilEngine::NewCarrierActive( TAccessPointInfo /*aNewAPInfo*/, TBool aIsSeamless )
    {
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::NewCarrierActive in");
     
    if ( aIsSeamless )
        {
        // It is Seamless. E.g. Mobile IP enabled.
        MPX_DEBUG1( "CVcxConnUtilEngine::NewCarrierActive seamless");
        iConnectionState = EVCxConnected;
        }
    else
        {
        MPX_DEBUG1( "CVcxConnUtilEngine::NewCarrierActive accepting new carrier");
         
        // We accept the new IAP / SNAP.
        iMobility->NewCarrierAccepted();
        iConnectionState = EVCxConnected;
        // change new IAP / SNAP info to the pubsub
        TRAPD( err, iObserver->IapChangedL() );
        if( err != KErrNone )
            {                    
            // internal error, let's try to resolve connection
            iMobility->NewCarrierRejected();
            iConnectionState = EVCxError;
            TRAP( err, iObserver->IapChangedL() ); 
            }
        }
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::NewCarrierActive out" );
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::EventL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilEngine::EventL( const CConnMonEventBase &aConnMonEvent )
    {
    TInt eventType = aConnMonEvent.EventType();
        
    TInt connectionId = aConnMonEvent.ConnectionId();

    // for other events:
    // if not master instance or event does not concern
    // current connection or we are not connected, do not 
    // need to handle events 
    if( !iObserver->IsMaster() || 
        connectionId != iCurrentConnectionId ||
        iConnectionState != EVCxConnected )
        {
        return;
        }
  
    switch(eventType)
        {               
        case EConnMonConnectionStatusChange:
            {     
            const CConnMonConnectionStatusChange* event = 
                            static_cast<const CConnMonConnectionStatusChange*>( &aConnMonEvent );

            if( event->ConnectionStatus() == KLinkLayerClosed )
                {
                MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::EventL: KLinkLayerClosed ");
                iObserver->IapChangedL();
                }                
            }
            break;
        case EConnMonDeleteConnection:
            {
            iObserver->IapChangedL();
            }
            break;
        default:
            break;
        }
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::Error()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilEngine::Error( TInt aError  )
    {
    MPX_DEBUG2( "vcxconnutil  ## CVcxConnUtilEngine::Error %d in", aError );
    if( aError != KErrNone )
        {
        // Error method is also called, when we call Cancel -method for iMobility
        // during disconnecting or during deallocation. Those are not concidered
        // as "errors" in the connection utility, so they do not require any
        // further handling
        if( iConnectionState != EVCxDisconnecting )
            {
            iConnectionState = EVCxError;
            TRAP_IGNORE( iObserver->IapChangedL() ); 
            }
        
        }
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::Error out" );
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::GetCmmDestinationIdL()
// -----------------------------------------------------------------------------
//
TUint32 CVcxConnUtilEngine::GetCmmDestinationIdL( CMManager::TSnapPurpose aPurpose )
    {
    TUint32 destinationId( 0 );
    // get default destination id
    TCmDefConnValue defConnection;
    iCmManagerExt.ReadDefConnL( defConnection );

    // if default is setted to be destination, get it's ID         
    if( defConnection.iType == ECmDefConnDestination && 
        aPurpose == CMManager::ESnapPurposeUnknown )
        {
        destinationId = defConnection.iId;
        }
    else if( defConnection.iType == ECmDefConnConnectionMethod ||
             aPurpose != CMManager::ESnapPurposeUnknown )
        {
        // default connection is connection method,
        // search for the destination of it
        RArray<TUint32> destinations;
        CleanupClosePushL( destinations );            
        iCmManagerExt.AllDestinationsL(destinations);
            
        RCmDestinationExt destination;
        RCmConnectionMethodExt connectionMethod;
        TInt exists( KErrNotFound );
        TInt count = destinations.Count();
        for( TInt i = 0; i < count && exists != KErrNone; i++ )
            {
            destination = iCmManagerExt.DestinationL( destinations[ i ] );
            CleanupClosePushL( destination ); 
            
            // look for default destination id
            if( aPurpose == CMManager::ESnapPurposeUnknown )
                {
                // check if connection method belongs to destination
                TRAP( exists, 
                  connectionMethod = destination.ConnectionMethodByIDL( 
                                                           defConnection.iId ) );
                if( exists == KErrNone )
                    {
                    connectionMethod.Close();
                    // correct destination found
                    destinationId = destination.Id();                   
                    }                
                }  
            // look for destination id based on purpose
            else
                {
                if( destination.MetadataL( CMManager::ESnapMetadataPurpose ) == 
                                            aPurpose )
                    {
                    // found, save destination id and end search
                    destinationId =  destination.Id();
                    exists = KErrNone;
                    }
                }
            CleanupStack::PopAndDestroy( &destination ); 
            }
        CleanupStack::PopAndDestroy( &destinations );
        }
    return destinationId;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::PrepareConnectionDetailsL()
// -----------------------------------------------------------------------------
//
TBool CVcxConnUtilEngine::PrepareConnectionDetailsL( TInt aDestinationID )
    {
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::PrepareConnectionDetailsL in");
    TBool connectionOk( EFalse );
    
    ResetConnectionInfo();
 
    if( aDestinationID )
        {
     
        MPX_DEBUG2( "CVcxConnUtilEngine::PrepareConnectionDetailsL destinationID: %d", aDestinationID);
        iDestinationId = aDestinationID;
        RCmDestinationExt destination = iCmManagerExt.DestinationL( aDestinationID );
        CleanupClosePushL(destination);
        
        iDestinationName = destination.NameLC();        
        CleanupStack::Pop( iDestinationName );
        
        MPX_DEBUG2( "CVcxConnUtilEngine::PrepareConnectionDetailsL destination name: %S", iDestinationName);
                
        TInt cmCount = destination.ConnectionMethodCount();
        MPX_DEBUG2( "CVcxConnUtilEngine::PrepareConnectionDetailsL method count: %d", cmCount);
        if( cmCount )
            {
            connectionOk = ETrue;
            }
        CleanupStack::PopAndDestroy( &destination );
        }
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::PrepareConnectionDetailsL out");
    return connectionOk;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::IsIapConnected()
// -----------------------------------------------------------------------------
//
TBool CVcxConnUtilEngine::IsIapConnected( TUint32 aIapId )
    {  
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::IsIapConnected in");
    TBool connected( EFalse );
    TRAPD( err, connected = 
      iCmManagerExt.GetConnectionMethodInfoBoolL( aIapId, CMManager::ECmConnected ) )
    
    if( err != KErrNone )
        {
        connected = EFalse;
        }
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::IsIapConnected out");
    return connected;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::FillActiveConnectionDetailsL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilEngine::FillActiveConnectionDetailsL() 
    {
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::FillActiveConnectionDetailsL in");
    // fill in connection information from the latest connection
    TRequestStatus status;
    TUint connectionCount(0);    
    iConnectionMonitor.GetConnectionCount(connectionCount, status);
    User::WaitForRequest( status );
    User::LeaveIfError(status.Int());
             
    if (connectionCount)
        {
        // fill in IAP Id
        iConnection.GetIntSetting( KCommDBIapId, iIapId );       
        MPX_DEBUG2( "CVcxConnUtilEngine::FillActiveConnectionDetailsL iap id: %d", iIapId );
        
        // get current connection id
        TBool found( EFalse );
        TUint subConnCount( 0 );
        for ( TInt i ( 1 ); i <= connectionCount && !found; i ++ )
            {
            TUint connectionId; 
            User::LeaveIfError( iConnectionMonitor.GetConnectionInfo( i, 
                                                                      connectionId, 
                                                                      subConnCount ) );          
            TUint iapIdToCheck;                                              
            iConnectionMonitor.GetUintAttribute( 
                                connectionId, 0, KIAPId, iapIdToCheck, status );
            // Note: Codescanner generates here pointless warning. We need to get data synchronously.
            User::WaitForRequest ( status ); 
            User::LeaveIfError( status.Int() );
            
            if ( iapIdToCheck == iIapId ) 
                { 
                // found 
                iCurrentConnectionId = connectionId; 
                found = ETrue;
                }                
            }
                      
        if( !iDestinationId )
            {
            // try to fill destination info if not yet gotten
            FillDestinationInfoL();
            }     
        }  
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::FillActiveConnectionDetailsL out");
    }
  
// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::IapId()
// -----------------------------------------------------------------------------
//
TUint32 CVcxConnUtilEngine::IapId() const
    {
    MPX_DEBUG2( "vcxconnutil  ## CVcxConnUtilEngine::IapId %d", iIapId );

    return iIapId;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::DestinationId()
// -----------------------------------------------------------------------------
//
TInt CVcxConnUtilEngine::DestinationId() const
    {
    return iDestinationId;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::ConnectionStatus()
// -----------------------------------------------------------------------------
//
TVCxConnectionStatus CVcxConnUtilEngine::ConnectionStatus() const
    {
    MPX_DEBUG2( "vcxconnutil  ## CVcxConnUtilEngine::ConnectionStatus %d", iConnectionState );
    return iConnectionState;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::SetConnectionStatus()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilEngine::SetConnectionStatus( TVCxConnectionStatus aStatus )
    {
    iConnectionState = aStatus;
    }


// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::DestinationName()
// -----------------------------------------------------------------------------
//
TPtrC CVcxConnUtilEngine::DestinationName() const
    {
    if ( iDestinationName )
        {
        return iDestinationName->Des();
        } 
    return TPtrC();
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::ResetConnectionInfo()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilEngine::ResetConnectionInfo()
    {
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::ResetConnectionInfo in");
    iIapId = 0;
    delete iDestinationName;
    iDestinationName = 0;
    iDestinationId = 0;
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::ResetConnectionInfo out");
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::FillDestinationInfoL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilEngine::FillDestinationInfoL( )
    {
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::FillDestinationInfoL in");
    TInt count = iDestinationArray.Count();
    delete iDestinationName;
    iDestinationName = 0;
    iDestinationId = 0;
    
    TBool found( EFalse );
    for( TUint destIndex = 0; destIndex < count && !found; ++destIndex )
        {
        RCmDestinationExt destination = 
            iCmManagerExt.DestinationL( iDestinationArray[ destIndex ] );
        
        CleanupClosePushL( destination );
        TInt cmCount = destination.ConnectionMethodCount();

        for ( TUint methIndex = 0; methIndex < cmCount && !found; ++methIndex )
            {
            RCmConnectionMethodExt connectionMethod = destination.ConnectionMethodL( methIndex );
            CleanupClosePushL( connectionMethod );
            if( iIapId == connectionMethod.GetIntAttributeL( CMManager::ECmIapId ) )
                {
                // found, save data
                iDestinationId = iDestinationArray[ destIndex ];
                MPX_DEBUG2( "CVcxConnUtilEngine::FillDestinationInfoL destination found: %d", iDestinationId );
                iDestinationName = destination.NameLC();                
                CleanupStack::Pop( iDestinationName );
                found = ETrue;
                MPX_DEBUG2( "CVcxConnUtilEngine::FillDestinationInfoL snap name: %S", iDestinationName );
                }
            CleanupStack::PopAndDestroy( &connectionMethod );
            }
        CleanupStack::PopAndDestroy( &destination );
        }
    MPX_DEBUG1( "vcxconnutil  ## CVcxConnUtilEngine::FillDestinationInfoL out");
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::DoCancel()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilEngine::DoCancel()
    {
   
    if( iConnectionState == EVCxConnecting )
        {
        MPX_FUNC( "CVcxConnUtilEngine::DoCancel disconnect");
        Disconnect();
        }
    iConnectingResult = KErrCancel;
    iObserver->EndWait( EVCxPSConnectionStatus );
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::RunL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilEngine::RunL()
    {
    if ( iStatus.Int() != KErrNone )
        {
        iConnectingResult = iStatus.Int();
        iConnectionState = EVCxNotConnected;
        }
    else
        {
        iConnectingResult = KErrNone;
        if( iConnectionState == EVCxConnecting )
            {                            
            iConnectionState = EVCxConnected;
            }
        }
    iObserver->EndWait( EVCxPSConnectionStatus );
       
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::QueryConn()
// -----------------------------------------------------------------------------
//
TBool CVcxConnUtilEngine::QueryConn()
    {
    return iQueryConn;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilEngine::SetQueryConn()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilEngine::SetQueryConn( TBool aQueryConn )
    {
    iQueryConn = aQueryConn;
    }

//  End of File
