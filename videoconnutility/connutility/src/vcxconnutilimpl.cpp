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
* Description:    Class to handle connection creation.*
*/




// INCLUDE FILES
#include <e32cmn.h>
#include <bldvariant.hrh>
#include <commdb.h>            // CMDBSession
#include <commsdattypesv1_1.h> // CCDWAPIPBearerRecord
#include <connectionuiutilities.h> 
#include <cmdestination.h>
#include <cdbcols.h>
#include <commdbconnpref.h>
#include <eikbtgrp.h>
#include <wlancdbcols.h>
#include <connpref.h>
#include <extendedconnpref.h>
#include <mpxlog.h>

#include <ipvideo/vcxconnectionutility.h>
#include <ipvideo/vcxconnutilengineobserver.h>
#include "vcxconnectionutility.hrh"
#include "vcxconnutilimpl.h"
#include "vcxconnutilengine.h"
#include "vcxconnutilpubsub.h"
#include "vcxconnutilwaitsch.h"

// CONSTANTS
_LIT( KConnUtilRoleSema, "__ConnUtillMasterSlaveRoleSwitchSema__ ");
_LIT( KConnUtilConnectionSema, "__ConnUtillConnectionCreateSema__ ");

const TInt KIptvCenRepAPModeAlwaysAsk = 0;
const TInt KConUtilSemaStartupValue = 1;
const TInt KNbrOfNotFoundConnTries = 30;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::NewL()
// -----------------------------------------------------------------------------
//
CVcxConnUtilImpl* CVcxConnUtilImpl::NewL( CVcxConnectionUtility* aUiInterface )
    {
    CVcxConnUtilImpl* self = CVcxConnUtilImpl::NewLC( aUiInterface );  
    CleanupStack::Pop( self ); // self;
    return self;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::NewLC()
// -----------------------------------------------------------------------------
//
CVcxConnUtilImpl* CVcxConnUtilImpl::NewLC( CVcxConnectionUtility* aUiInterface )
    {
    CVcxConnUtilImpl* self = new (ELeave) CVcxConnUtilImpl( aUiInterface );
    CleanupStack::PushL( self );
    self->ConstructL( );
    return self;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::ConstructL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::ConstructL( )
    {
    MPX_DEBUG1("vcxconnutil ## CVcxConnUtilImpl::ConstructL() in");
    
    iWaitHandler = CVcxConnUtilWaitSch::NewL();
    
    iObservers.Reset();
    
    TInt err( KErrNone );
    err = iSemaSwitchRole.CreateGlobal( KConnUtilRoleSema, KConUtilSemaStartupValue );
    if( err == KErrAlreadyExists )
        {
        // semafore exists already, open it
        err = iSemaSwitchRole.OpenGlobal( KConnUtilRoleSema );
        }
    User::LeaveIfError( err );
    
    err = iSemaCreateConn.CreateGlobal( KConnUtilConnectionSema, KConUtilSemaStartupValue );
    if( err == KErrAlreadyExists )
        {
        // semafore exists already, open it
        err = iSemaCreateConn.OpenGlobal( KConnUtilConnectionSema );
        }
    User::LeaveIfError( err );
    
    iEngine = CVcxConnUtilEngine::NewL( this );
        
    iPubsub = CVcxConnUtilPubSub::NewL( this );
            
    // check and set pubsub -values 
    TInt activeIap( 0 );
    User::LeaveIfError( iPubsub->GetValue( EVCxPSIapId, activeIap ) );
    if( activeIap != 0 )
        {
        // check if there really is connection, if not reinit values
        if( !iEngine->IsIapConnected( activeIap ) )
            {
            // no connection, data integrity has been corrupted at some point
            activeIap = 0;
            }
        }
    if( activeIap == 0 ) 
        {
        MPX_DEBUG1("CVcxConnUtilImpl::ConstructL() no connection yet, initing values");
        User::LeaveIfError( iPubsub->SetValue( EVCxPSMasterExists, 0 ) );
        
        User::LeaveIfError( iPubsub->SetValue( EVCxPSIapId, 0 ) );
        
        User::LeaveIfError( iPubsub->SetValue( EVCxPSSnapId, 0 ) );
        
        User::LeaveIfError( iPubsub->SetValue( 
                                 EVCxPSConnectionStatus, EVCxNotConnected ) );
        
        User::LeaveIfError( iPubsub->SetValue( EVCxPSNbrConnInstances, 0 ) );        
        }
    
    MPX_DEBUG1("vcxconnutil ## CVcxConnUtilImpl::ConstructL() out");
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::CVcxNsConnectionUtility()
// -----------------------------------------------------------------------------
//
CVcxConnUtilImpl::CVcxConnUtilImpl( CVcxConnectionUtility* aUiInterface ) :
iUIInterface( aUiInterface )
    {
    }
    
// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::~CVcxConnectionUtility()
// -----------------------------------------------------------------------------
//
CVcxConnUtilImpl::~CVcxConnUtilImpl()
    {   
    MPX_DEBUG1("vcxconnutil ## CVcxConnUtilImpl::~CVcxConnUtilImpl in");
    
    iObservers.Close();
    
    if( iEngine && iPubsub )
        {
        TRAP_IGNORE( DisconnectL() );
        }
    
    delete iWaitHandler;
    
    iSemaSwitchRole.Close();
    iSemaCreateConn.Close();
     
    delete iPubsub;
    delete iEngine;
    
    MPX_DEBUG1("vcxconnutil ## CVcxConnUtilImpl::~CVcxConnUtilImpl out");
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::RegisterObserverL
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::RegisterObserverL( MConnUtilEngineObserver* aObserver )
   {
   MPX_DEBUG1("vcxconnutil ## CVcxConnUtilImpl::RegisterObserverL in");
   iObservers.AppendL( aObserver );
   MPX_DEBUG1("vcxconnutil ## CVcxConnUtilImpl::RegisterObserverL out");
   }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::RemoveObserver
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::RemoveObserver( MConnUtilEngineObserver* aObserver )
   {
   MPX_DEBUG1("Cvcxconnutil ## VcxConnUtilImpl::RemoveObserver in");
   TInt i;
   for ( i = 0; i < iObservers.Count(); i++ )
       {
       if ( aObserver == iObservers[i] )
           {
           iObservers.Remove( i );
           iObservers.Compress();
           break;
           }
       } 
   MPX_DEBUG1("Cvcxconnutil ## VcxConnUtilImpl::RemoveObserver out");
   }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::EngineConnectionStatus
// -----------------------------------------------------------------------------
//
TVCxConnectionStatus CVcxConnUtilImpl::EngineConnectionStatus()
    {
    return iEngine->ConnectionStatus();
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::GetIap
// -----------------------------------------------------------------------------
//
TInt CVcxConnUtilImpl::GetIap( TUint32& aIapId, TBool aSilent )
    {
    MPX_DEBUG1("vcxconnutil ## CVcxConnUtilImpl::GetIap in ");
    aIapId = 0;
    TInt err( KErrNone );
    // assume always new connection
    iNewConnection = ETrue;
    
    switch ( iEngine->ConnectionStatus() )
        {
        case EVCxNotConnected:            
        case EVCxDisconnecting:                    
            iSemaCreateConn.Wait();
            err = KErrNotFound;
            // KErrNotFound from connection creation indicates that we have 
            // some destination with unusable iap(s) in it for we cannot connect to
            // in that case, destination id to db is setted as "always ask" and connection
            // creation will be tried again for 30 times
            for(TInt i = 0; i < KNbrOfNotFoundConnTries && err == KErrNotFound; ++i)
                {
                err = CreateConnection( aSilent );
                MPX_DEBUG2("vcxconnutil ## CVcxConnUtilImpl::GetIap - CreateConnection returned %d ", err );
                }
            iSemaCreateConn.Signal();    
            break;             
        case EVCxConnecting:
            {
            // this instance is connecting, meaning engine is waiting for
            // networking to create connection, we need to wait for the engine 
            // to finish it's waiting
            TRAP( err, WaitL( EVCxPSConnectionStatus ) );
            if( iEngine->ConnectionStatus() != EVCxConnected )
                {
                // if main active object didn't managed to create connection,
                // return error. This active object does not know the actual
                // error so return KErrGeneral
                err = KErrGeneral;
                }
            else
                {
                err = HandleFinalizeConnection();
                }
            }
            break;
        case EVCxRoamingAccepted: // pass throught
        case EVCxRoamingRequest:  // pass throught
            TRAP( err, WaitL( EVCxPSConnectionStatus ) );
        default:
            iNewConnection = EFalse; 
            break;
        }          
        
    if( err == KErrNone )
        {   
        // get iap id if any. If this is the first connection for this 
        // instance, iap id will be fetched later on
        aIapId = iEngine->IapId();       
        }

    MPX_DEBUG2("vcxconnutil ## CVcxConnUtilImpl::GetIap out error: %d ", err );
    return err;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::WapIdFromIapIdL
// -----------------------------------------------------------------------------
//
TUint32 CVcxConnUtilImpl::WapIdFromIapIdL( TUint32 aIapId )
    {
    MPX_DEBUG1("vcxconnutil ## CVcxConnUtilImpl::WapIdFromIapIdL() in ");
    MPX_DEBUG2("vcxconnutil ## CVcxConnUtilImpl::WapIdFromIapIdL() IAP id = %d", aIapId);
    CMDBSession* db = CMDBSession::NewL( CMDBSession::LatestVersion() );
    CleanupStack::PushL( db );
    
    // WapIpBearer table contains the mapping between wap and iap id's.
    CCDWAPIPBearerRecord* wapBearerRecord = 
        static_cast<CCDWAPIPBearerRecord*>( CCDRecordBase::RecordFactoryL( KCDTIdWAPIPBearerRecord ) );
        
    CleanupStack::PushL( wapBearerRecord );
    
    wapBearerRecord->iWAPIAP = aIapId;
    
    TBool found = wapBearerRecord->FindL( *db );
    
    if ( !found )
        {
        MPX_DEBUG1("vcxconnutil ## CVcxConnUtilImpl::WapIdFromIapIdL() Record was not found. Leaving with KErrNotFound.");
        User::Leave(KErrNotFound);
        }

    TUint32 wap = static_cast<TUint32>( wapBearerRecord->iWAPAccessPointId );
    
    CleanupStack::PopAndDestroy( wapBearerRecord );
    CleanupStack::PopAndDestroy( db );
    
    MPX_DEBUG2("vcxconnutil ## CVcxConnUtilImpl::WapIdFromIapIdL() Matching WAP id = %d", wap);
    MPX_DEBUG1("vcxconnutil ## CVcxConnUtilImpl::WapIdFromIapIdL() out ");
    return wap;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::PrepareConnSettings()
// -----------------------------------------------------------------------------
//
TInt CVcxConnUtilImpl::PrepareConnSettings()
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::PrepareConnSettings in");  
    TInt err( KErrNone );
    TInt vcDestinationID( 0 );
    
    if ( !iEngine->QueryConn() )
        {
        TRAP( err, vcDestinationID = 
            iEngine->GetCmmDestinationIdL( CMManager::ESnapPurposeUnknown ) );
        MPX_DEBUG2( "vcxconnutil ## CVcxConnUtilImpl::PrepareConnSettings - destination id %d", vcDestinationID);  
        }
   
    TBool detailsGotten( EFalse );
    TRAP( err, detailsGotten = iEngine->PrepareConnectionDetailsL( vcDestinationID ) );
    if( err == KErrNone && !detailsGotten )
        {
        // not able to resolve connection details, proceed with always ask
        iEngine->ResetConnectionInfo();
        }
    
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::PrepareConnSettings out");  
    return err;            
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::CreateConnection()
// -----------------------------------------------------------------------------
//
TInt CVcxConnUtilImpl::CreateConnection( TBool aSilent )
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::CreateConnectionL in");     
    
    TInt err( KErrNone );

    TInt connStatusPS( EVCxNotConnected );
    
    err = PrepareConnSettings();
    if( err != KErrNone )
        {
        MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::CreateConnectionL error getting connsettings out");
        return err;
        }
    
    err = iPubsub->GetValue( EVCxPSConnectionStatus, connStatusPS );
    if( err != KErrNone )
        {
        MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::CreateConnectionL error getting PS conn status out");
        return err;
        }
    
    // if there is already an active connection created by some other  
    // instance, use that one
    TInt snapId( 0 );
    TBool masterConnect( EFalse );

    if( connStatusPS == EVCxConnected || connStatusPS == EVCxRoamingRequest )
        {        
        err = iPubsub->GetValue( EVCxPSSnapId, snapId );
        if( err != KErrNone )
            {
            MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::CreateConnectionL error getting PS snap id, out");                   
            return err;
            }
        if( snapId == 0 )
            {
            MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::CreateConnectionL not usable snap is in PS");
            // current snap is undefined, set snap id KErrNotFound for 
            // DoCreateConnection to use active iap instead of snap id
            snapId = KErrNotFound;
            }
        }
    else
        {
        masterConnect = ETrue;
        snapId = iEngine->DestinationId();
        }

    err = DoCreateConnection( aSilent, snapId, masterConnect );
    if( err == KErrNotFound && masterConnect )
        {
        // KErrNotFound from connection creation indicates that we have 
        // some destination with unusable iap(s) in it. Enable connection query.
        MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::CreateConnectionL - set connection query");
        iEngine->SetQueryConn( ETrue );
        }
    if( err == KErrNone )
        {
        err = HandleFinalizeConnection();
        }
    
    MPX_DEBUG2( "vcxconnutil ## CVcxConnUtilImpl::CreateConnectionL out (%d)", err);
    return err;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::DoCreateConnection()
// -----------------------------------------------------------------------------
//
TInt CVcxConnUtilImpl::DoCreateConnection(  TBool /*aSilent*/, TInt32 aSnapId, TBool aMasterConnect )
    {
    MPX_DEBUG2( "vcxconnutil ## CVcxConnUtilImpl::DoCreateConnectionL in (snapId=%d)", aSnapId);
    
    TInt err( KErrNone );
    
    if ( aSnapId == KIptvCenRepAPModeAlwaysAsk )
        {  
        //In always ask mode we show always ask dialog
        TCommDbConnPref connPref;
        connPref.SetDialogPreference(  ECommDbDialogPrefPrompt  );
        err = iEngine->StartToConnect( connPref );
        MPX_DEBUG2( "CVcxConnUtilImpl::DoCreateConnectionL connection start always ask err %d", err);
        }
    else if ( aSnapId > KIptvCenRepAPModeAlwaysAsk )
        {
        TConnSnapPref prefs;
        prefs.SetSnap( aSnapId );
        err = iEngine->StartToConnect( prefs, aMasterConnect );
        MPX_DEBUG2( "CVcxConnUtilImpl::DoCreateConnectionL connection start err %d", err);
        }
    else
        {
        TInt iapPS( 0 );
        err = iPubsub->GetValue( EVCxPSIapId, iapPS );
        if( err == KErrNone )
            {
            if( iapPS )
                {
                TCommDbConnPref connPref;
                connPref.SetIapId( iapPS );
                connPref.SetDialogPreference( ECommDbDialogPrefDoNotPrompt );
                err = iEngine->StartToConnect( connPref, EFalse );
                MPX_DEBUG2( "CVcxConnUtilImpl::DoCreateConnectionL connection via iap start err %d", err);
                }
            else
                {
                err = KErrNotFound;
                }            
            }  
        }
    if( err == KErrNone && aMasterConnect )
        {
        err = iEngine->CreateMobility();
        }
    MPX_DEBUG2( "vcxconnutil ## CVcxConnUtilImpl::DoCreateConnectionL out (%d)", err);
    return err;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::HandleFinalizeConnection()
// -----------------------------------------------------------------------------
//
TInt CVcxConnUtilImpl::HandleFinalizeConnection()
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleFinalizeConnection in");
    if( !iNewConnection )
        {
        // if connection is not new for this instance
        // no finalizing required
        MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleFinalizeConnection no new connection, out");
        return KErrNone;
        }
    if( iEngine->ConnectionStatus() != EVCxConnected  )
        {
        // connection not ok, no point to proceed, return error
        MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleFinalizeConnection not connected, out");
        return KErrGeneral;
        }

    iNewConnection = EFalse;
    TRAPD( err, iEngine->FillActiveConnectionDetailsL() );
               
    if( err == KErrNone )
        {                
        TRAP( err, iPubsub->IncCounterPubSubL( EVCxPSNbrConnInstances ) );              
        if( err == KErrNone )
            {                                    
            // master role check and switch if necessary
            if( !iMaster )
                {
                err = CheckAndChangeSlaveToMaster();                        
                }
            }
            if( err == KErrNone && iMaster )
                {
                TRAP( err, SaveConnectionToPubSubL() );
                }
            if( err == KErrNone )
                {    
                // start getting events from pubsub
                iPubsub->StartSubscibers();
                }
         }            
     if( err != KErrNone )
         {
         // some operation above failed, internal error
         // try to disconnect and return error       
         TRAPD( errDisc, DisconnectL() );
         // return latest error if disconnecting fails
         if( errDisc != KErrNone )
             {
             err = errDisc;
             }
         }
     MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleFinalizeConnection out");
     return err;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::DisconnectConnection()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::DisconnectL()
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::Disconnect in");
    TVCxConnectionStatus connStatus = iEngine->ConnectionStatus();
    
    // if this is a master, and it is roaming we need to wait 
    // for roaming to finish
    if( iMaster )
        {
        // because EVCxRoamingAccepted is never saved as
        // connection status to PS,
        // it is safe to wait PS value key change here
        if( connStatus == EVCxRoamingRequest   ||
            connStatus == EVCxRoamingAccepted   )
            {
            MPX_DEBUG1( "CVcxConnUtilImpl::Disconnect master waiting roaming to finish");                        
            WaitL( EVCxPSConnectionStatus );
            MPX_DEBUG1( "CVcxConnUtilImpl::Disconnect master waiting finished");
            connStatus = iEngine->ConnectionStatus();
            }
        }
    else
        {
        // if slave is about to disconnect
        // and it's internal state is roaming
        // we know that it has been responded to 
        // roaming request. Decrease value here
        if( connStatus == EVCxRoamingRequest   ||
            connStatus == EVCxRoamingAccepted  ||
            connStatus == EVCxPendingClientRequest )
            {
            if( connStatus == EVCxRoamingAccepted )
                {
                MPX_DEBUG1( "CVcxConnUtilImpl::Disconnect slave removes it's acceptance");
                iPubsub->DecCounterPubSubL( EVCxPSNbRoamAccepted );
                }
            MPX_DEBUG1( "CVcxConnUtilImpl::Disconnect slave removes it's response");
            iPubsub->DecCounterPubSubL( EVCxPSNbrRoamResp );     
            }
        }
    
    // disconnecting, no longer interested in pubsub events
    iPubsub->CancelValueSubscribers();
    
    if( connStatus == EVCxNotConnected )
        {
        return;
        }
    
    TInt nbrOfConnUsers( 0 );
    User::LeaveIfError( iPubsub->GetValue( EVCxPSNbrConnInstances, nbrOfConnUsers ) );
                    
    if(  nbrOfConnUsers > 0 )
        {
        iPubsub->DecCounterPubSubL( EVCxPSNbrConnInstances );
        }  
    iPubsub->GetValue( EVCxPSNbrConnInstances, nbrOfConnUsers );
    // we're leaving and there are no other instances using connection
    if( nbrOfConnUsers == 0 )
        {
        // no more connection users, reset PS -values   
        User::LeaveIfError( iPubsub->SetValue( EVCxPSIapId,  0 ) );
          
        User::LeaveIfError( iPubsub->SetValue( EVCxPSSnapId, 0 ) );
            
        User::LeaveIfError( iPubsub->SetValue( 
                            EVCxPSConnectionStatus, EVCxNotConnected ) );
            
        User::LeaveIfError( iPubsub->SetValue( 
                            EVCxPSRoamingRequestStatus, EVCxRoamingNotInit ) );
        }
    iEngine->Disconnect();
       
    // cannot be master anymore if not using connection
    if( iMaster )
        {
        iMaster = EFalse;
        User::LeaveIfError( iPubsub->SetValue( EVCxPSMasterExists,  0 ) );  
        }
        
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::Disconnect out");
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::SaveConnectionToPubSubL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::SaveConnectionToPubSubL()
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::SaveConnectionToPubSubL in");
    // publish connection properties
    // IAP id:
    User::LeaveIfError( iPubsub->SetValue( EVCxPSIapId, iEngine->IapId() ) );
    
    // Snap ID:
    User::LeaveIfError( iPubsub->SetValue( EVCxPSSnapId, iEngine->DestinationId() ) );
    
    // connection State
    User::LeaveIfError( iPubsub->SetValue( 
                              EVCxPSConnectionStatus, iEngine->ConnectionStatus() ) );   
    
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::SaveConnectionToPubSubL out");
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::RequestIsRoamingAllowedL()
// -----------------------------------------------------------------------------
//
TBool CVcxConnUtilImpl::RequestIsRoamingAllowedL()
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::RequestIsRoamingAllowedL in");
    TBool okToRoam( ETrue );
    
    // if this instance is master, need to wait for slaves to request their status before proceeding
    if( iMaster )
        {        
        // init pubsubs for roaming request status checking
        User::LeaveIfError( iPubsub->SetValue( EVCxPSNbrRoamResp,    0 ) );
        User::LeaveIfError( iPubsub->SetValue( EVCxPSNbRoamAccepted, 0 ) );
        
        // query own state
        okToRoam = DoRequestClientRoamingL();
        // client might have disconnected
        if( iEngine->ConnectionStatus() == EVCxNotConnected )
            {
            MPX_DEBUG1( "CVcxConnUtilImpl::RequestIsRoamingAllowedL master disconnected, out");
            return EFalse;            
            }
             
        User::LeaveIfError( iPubsub->SetValue( EVCxPSConnectionStatus, EVCxRoamingRequest ) );
        TInt nbrOfConnUsers(0);
        User::LeaveIfError( iPubsub->GetValue( EVCxPSNbrConnInstances, nbrOfConnUsers ) );
        if( okToRoam && nbrOfConnUsers > 1 )
            {
            
            // if we have slaves also using connection, 
            // we need to wait for them to request roaming statuses
            // from their clients also. In this case, the status is being 
            // checked via pubsub EVCxPSRoamingRequestStatus to which master
            // updates after enought resposes are received
            MPX_DEBUG1( "CVcxConnUtilImpl::RequestIsRoamingAllowedL master waiting");
            TRAPD( err, WaitL( EVCxPSRoamingRequestStatus ) );
            if( err != KErrNone )
                {
                MPX_DEBUG2( "CVcxConnUtilImpl::RequestIsRoamingAllowedL master wait leaved %d", err );
                }
            MPX_DEBUG1( "CVcxConnUtilImpl::RequestIsRoamingAllowedL master released");
    
            TInt roamingStatus( EVCxRoamingNotInit );
            User::LeaveIfError( iPubsub->GetValue( EVCxPSRoamingRequestStatus, roamingStatus ) );
            if( EVCxRoamingAllowed != roamingStatus )
                {
                okToRoam = EFalse;            
                }
            }
        }    
    // reset roaming PS value
    User::LeaveIfError( iPubsub->SetValue( EVCxPSRoamingRequestStatus, EVCxRoamingNotInit ) );
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::RequestIsRoamingAllowedL out");
    return okToRoam;
    }
 
// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::DoRequestClientRoamingL()
// -----------------------------------------------------------------------------
//
TBool CVcxConnUtilImpl::DoRequestClientRoamingL()
    {   
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::DoRequestClientRoamingL in" );
    
    TBool okToRoam( ETrue );
    TInt err( KErrNone );

    TVCxConnectionStatus oldState = iEngine->ConnectionStatus();
    iEngine->SetConnectionStatus( EVCxPendingClientRequest );
    for ( TInt i = 0; i < iObservers.Count() && okToRoam; i++ )
        {
        TRAP( err, okToRoam = iObservers[i]->RequestIsRoamingAllowedL() );
        if( err != KErrNone || !okToRoam )
            {
            okToRoam = EFalse;
            break;
            }
        }
    if( iEngine->ConnectionStatus() != EVCxNotConnected )
        {
        // operation path back to connection utility, reset state, in case 
        // client has not diconnected during callback        
        iEngine->SetConnectionStatus( oldState );
        
        if( okToRoam && err == KErrNone )
            {
            // roaming ok for this instance, increase nbr of accepted   
            iPubsub->IncCounterPubSubL( EVCxPSNbRoamAccepted );
            MPX_DEBUG1( "CVcxConnUtilImpl::DoRequestClientRoamingL accepted increased" );
            }
        }
    
    // increase nbr of responses
    iPubsub->IncCounterPubSubL( EVCxPSNbrRoamResp );
        
    MPX_DEBUG2( "CVcxConnUtilImpl::DoRequestClientRoamingL allowed %d out", okToRoam );
    
    return okToRoam;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::IapChanged()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::IapChangedL()
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::IapChanged in");
    if( !iMaster )
        {
        MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::IapChanged not master out");
        return;
        }
    TInt err( KErrNone );
    
    // current information
    TUint32 oldIap = iEngine->IapId();
    TVCxConnectionStatus engineStatus = iEngine->ConnectionStatus();
    
    // resolve connection information from the network middleware
    iEngine->ResetConnectionInfo();
    if( engineStatus != EVCxNotConnected && 
        engineStatus != EVCxError )
        {
        iEngine->FillActiveConnectionDetailsL();                 
        }        
    
    // if there is no active connection or gotten iap id is invalid
    TUint32 iapID = iEngine->IapId();
    if( iapID == 0 || !( iEngine->IsIapConnected( iapID ) ) ) 
        {        
        if( engineStatus != EVCxError )
            {            
            engineStatus = EVCxNotConnected;
            }
        // signal status before disconnect -call to make sure 
        // slaves react in case master's disconnecting fails
        MPX_DEBUG2( "IapChanged not connected notify %d ", engineStatus );
        iPubsub->SetValue( EVCxPSConnectionStatus, engineStatus );
           
        MPX_DEBUG1( "CVcxConnUtilImpl::IapChanged no active iap, diconnect");
        TRAP( err, DisconnectL() );
        if( err != KErrNone )
            {
            MPX_DEBUG2( "CVcxConnUtilImpl::IapChanged no active iap, diconnect leaves %d", err );
            }
        }
    else
        {
        MPX_DEBUG1( "CVcxConnUtilImpl::IapChanged iap connected");
        engineStatus = EVCxConnected;
        iEngine->SetConnectionStatus( EVCxConnected );
        // refresh PS connection state
        SaveConnectionToPubSubL();
        }
                    
    // notify clients about iap change
    if( engineStatus != EVCxError && ( oldIap != iapID || !iapID ) )
        {
        MPX_DEBUG1( "CVcxConnUtilImpl::IapChanged notify observers");  
        NotifyObserversIAPChanged();        
        }
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::IapChanged out");               
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::WaitL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::WaitL( TUint32 aWaitId )
    {
    iWaitHandler->WaitL( aWaitId );
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::EndWait()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::EndWait( TUint32 aWaitId )
    {
    iWaitHandler->EndWait( aWaitId );
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::IsMaster()
// -----------------------------------------------------------------------------
//
TBool CVcxConnUtilImpl::IsMaster()
    {
    return iMaster;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::ValueChangedL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::ValueChangedL( const TUint32& aKey, const TInt&  aValue )
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::ValueChangedL (TInt) in");   
    EndWait( aKey );
    // pubsub key value changed, check and update 
    // functionality as required.
    switch( aKey )
        {
        case EVCxPSMasterExists:
            {
            HandleMasterChangeL();           
            break;
            }              
        case EVCxPSConnectionStatus: 
            {   
            HandleSlaveConnectionStatusL( aValue );
            break;
            }
        case EVCxPSNbrRoamResp:
            {
            HandleRoamingReponsesL( aValue );
            }
            break;
        case EVCxPSRoamingRequestStatus:
            // NOP
            break;
        default:
            User::Leave( KErrNotFound );
            break;      
        } 
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::ValueChangedL (TInt) out");  
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::HandleMasterChangeL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::HandleMasterChangeL()
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleMasterChangeL in"); 
    // if this instance is not connected, don't bother
    // to change. Not connected instance cannot be master
    if( iMaster )
        {
        MPX_DEBUG1( "CVcxConnUtilImpl::HandleMasterChangeL master, out");
        return;
        }
    
    if( iEngine->ConnectionStatus() != EVCxConnected )
        {
        MPX_DEBUG1( "CVcxConnUtilImpl::HandleMasterChangeL not connected, out");
        return;
        }
    
    User::LeaveIfError( CheckAndChangeSlaveToMaster() );

    if( iMaster )
        {                    
        // at this point we need to reinit ex-slave's connection to use 
        // destination (snap) instead of IAP for the ex-slave to be able 
        // to get mobility events. 
        // if original connection was via IAP due "always ask", do nothing
        if( iEngine->DestinationId() )
            {
            // this instance has become master, so it needs to init the mobility object
            if( iEngine->CreateMobility() != KErrNone )
                {
                MPX_DEBUG1( "CVcxConnUtilImpl::HandleMasterChangeL master reinit connection not ok");
                DisconnectL();  
                }
            else
                {
                MPX_DEBUG1( "CVcxConnUtilImpl::HandleMasterChangeL master reinit connection ok");
                iEngine->FillActiveConnectionDetailsL();
                SaveConnectionToPubSubL();
                }
            }  
        }
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleMasterChangeL out"); 
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::CheckAndChangeSlaveToMaster()
// -----------------------------------------------------------------------------
//
TInt CVcxConnUtilImpl::CheckAndChangeSlaveToMaster()
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::CheckAndChangeSlaveToMaster in"); 
    TInt err( KErrNone );  
    TInt status( 0 );
    
    ///////
    // entering critical section
    iSemaSwitchRole.Wait();                
    err = iPubsub->GetValue( EVCxPSMasterExists, status );
    // if master already exists, do nothing
    if( !status && err == KErrNone)
        {
        MPX_DEBUG1( "CVcxConnUtilImpl::CheckAndChangeSlaveToMaster changing master");
        iMaster = ETrue;
        err = iPubsub->SetValue( EVCxPSMasterExists, iMaster );
        }
    iSemaSwitchRole.Signal();
    // Leaving critical section
    ///////
    
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::CheckAndChangeSlaveToMaster out"); 
    return err;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::HandleSlaveConnectionStatusL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::HandleSlaveConnectionStatusL( const TInt& aNewStatus )
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleSlaveConnectionStatus in"); 
    // if master, do nothing
    if( iMaster )
        {
        MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleSlaveConnectionStatus master out"); 
        return;
        }
    // resolve given connection, this structure is needed  
    // to prevent errors in type check
    TVCxConnectionStatus gottenStatus( EVCxNotConnected );
    switch( aNewStatus )
        {
        case EVCxNotConnected: 
            gottenStatus = EVCxNotConnected; 
            break;
        case EVCxConnecting:
            gottenStatus = EVCxConnecting;
            break;
        case EVCxConnected:
            gottenStatus = EVCxConnected;
            break;
        case EVCxDisconnecting:
            gottenStatus = EVCxDisconnecting;
            break;
        case EVCxRoamingRequest:
            gottenStatus = EVCxRoamingRequest;
            break;
        case EVCxRoamingAccepted:
            gottenStatus = EVCxRoamingAccepted;
            break;  
        case EVCxError:
            gottenStatus = EVCxError;
            break;        
        default:
            User::Leave( KErrGeneral );
            break;
        }
    TVCxConnectionStatus currentStatus = iEngine->ConnectionStatus();  
    
    MPX_DEBUG2( "HandleSlaveConnectionStatus gotten %d", gottenStatus );
    MPX_DEBUG2( "HandleSlaveConnectionStatus current %d", currentStatus );
    
    if( gottenStatus == EVCxRoamingRequest && currentStatus == EVCxConnected )
        {
        // if master is requesting roaming, query all external clients  
        // whether we can roam or not  
        MPX_DEBUG1( "CVcxConnUtilImpl::HandleSlaveConnectionStatus slave check Roaming");
        if ( DoRequestClientRoamingL() )
            {
            gottenStatus = EVCxRoamingAccepted;
            }
        // client might have disconnected during roaming. In that case do not change status
        if( iEngine->ConnectionStatus() != EVCxNotConnected )
            {
            // set connection status explicitly to tell slave we're roaming
            iEngine->SetConnectionStatus( gottenStatus );
            }
        }
    
    else if( ( currentStatus == EVCxRoamingRequest  ||
               currentStatus == EVCxRoamingAccepted || 
               currentStatus == EVCxConnected ) &&
               gottenStatus == EVCxConnected )
        {
        // if current status was roaming or
        // master has notified new connection and state
        // has changed to connected, meaning
        // master has succesfully reinitialized preferred connection
        // slave needs to try to reinit connection via new iap if 
        // new iap differs from current
        
        TInt valueFromPS( 0 );
        User::LeaveIfError( iPubsub->GetValue( EVCxPSIapId, valueFromPS ) );
        MPX_DEBUG2( "HandleSlaveConnectionStatus slave iap %d", iEngine->IapId() );
        MPX_DEBUG2( "HandleSlaveConnectionStatus slave PS iap %d", valueFromPS );
        if( valueFromPS != iEngine->IapId() )
            {
            User::LeaveIfError( iPubsub->GetValue( EVCxPSSnapId, valueFromPS ) );
            iEngine->Disconnect();
            if( DoCreateConnection( ETrue, valueFromPS, EFalse ) == KErrNone )
                { 
                // refresh connection details
                iEngine->FillActiveConnectionDetailsL();                  
                MPX_DEBUG1( "CVcxConnUtilImpl::HandleSlaveConnectionStatus slave restarted ok" );                
                }
            else
                {
                MPX_DEBUG1( "CVcxConnUtilImpl::HandleSlaveConnectionStatus slave restarting not ok" );
                DisconnectL();
                gottenStatus = EVCxNotConnected;
                }            
            NotifyObserversIAPChanged();
            }
        iEngine->SetConnectionStatus( gottenStatus );        
        }
    else 
        {
        if( gottenStatus == EVCxNotConnected || 
            gottenStatus == EVCxError )
            {
            // master has notified disconnecting or error for some reason
            DisconnectL();  
            // notify normal disconnect to observers
            if( gottenStatus == EVCxNotConnected )
                {
                NotifyObserversIAPChanged();
                }
            }
        }
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleSlaveConnectionStatus out");
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::ValueChangedL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::HandleRoamingReponsesL( const TInt& aNbrOfResps )
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleRoamingReponsesL in"); 
    if( !iMaster )
        {
        MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleRoamingReponsesL slave out");
        return;
        }
    if( iEngine->ConnectionStatus() !=  EVCxRoamingRequest )
        {
        MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleRoamingReponsesL not roaming");
        return;
        }
    // check if all have been responded and if all agreed on roaming
    TInt nbrOfConUsers( 0 );
    TInt nbrOfAgeed( 0 );
    TInt nbrofResponses( aNbrOfResps );
   
    User::LeaveIfError( iPubsub->GetValue( EVCxPSNbrConnInstances, nbrOfConUsers )  );
    User::LeaveIfError( iPubsub->GetValue( EVCxPSNbRoamAccepted, nbrOfAgeed )  );
    
    if( nbrOfConUsers == nbrofResponses )
        {
        if( nbrofResponses == nbrOfAgeed )
            {
            // every instance has responded and all agrees roaming, 
            // change state for master's main thread to proceed
            MPX_DEBUG1( "CVcxConnUtilImpl::HandleRoamingReponsesL EVCxRoamingAllowed");
            
            User::LeaveIfError( iPubsub->SetValue( 
                              EVCxPSRoamingRequestStatus, EVCxRoamingAllowed ) );
            }
        else
            {
            MPX_DEBUG1( "CVcxConnUtilImpl::HandleRoamingReponsesL EVCxRoamingNotInit");
            User::LeaveIfError( iPubsub->SetValue( 
                                  EVCxPSRoamingRequestStatus, EVCxRoamingNotInit ) );
            }
        }  
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::HandleRoamingReponsesL out"); 
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::NotifyObserversIAPChanged()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::NotifyObserversIAPChanged()
    {
    for ( TInt i = 0; i < iObservers.Count(); i++ )
        {
        MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::NotifyObserversIAPChanged notifying observer"); 
        TRAPD( err, iObservers[i]->IapChangedL() );
        if( err != KErrNone )
            {
            MPX_DEBUG2( "vcxconnutil ## NotifyObserversIAPChanged::IapChanged observer leaved %d", err);               
            }        
        } 
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::DisplayWaitNote()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::DisplayWaitNote(const TDesC& aConnectionName)
     {
     MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::DisplayWaitNote in"); 
     if( iUIInterface )
         {
         iUIInterface->DisplayWaitNote( aConnectionName );
         }
     MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::DisplayWaitNote out");
     }

// -----------------------------------------------------------------------------
// CVcxConnUtilImpl::CloseWaitNote()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilImpl::CloseWaitNote()
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::CloseWaitNote in");
    if( iUIInterface )
         {
         iUIInterface->CloseWaitNote();
         }
    MPX_DEBUG1( "vcxconnutil ## CVcxConnUtilImpl::CloseWaitNote out");
    }

//  End of File  
