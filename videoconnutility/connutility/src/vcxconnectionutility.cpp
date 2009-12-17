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
#include <e32std.h>
#include <mpxlog.h>

#include <ipvideo/vcxconnectionutility.h>
#include <ipvideo/tvcxconnutilnotifierparams.h>
#include "vcxconnectionutility.hrh"
#include "vcxconnutilimpl.h"

// CONSTANTS 
const TInt KVCxConnUtilAppUid =  0x20016BA1;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CVcxConnectionUtility::InstanceL()
// -----------------------------------------------------------------------------
//
EXPORT_C CVcxConnectionUtility* CVcxConnectionUtility::InstanceL()
    {
    MPX_DEBUG1("vcxconnutil ## CVcxConnectionUtility::InstanceL() in");
    
    CVcxConnectionUtility* self = NULL;
    TAny* instance = UserSvr::DllTls( KVCxConnUtilAppUid );
    
    if ( !instance )
        {
        self = new( ELeave ) CVcxConnectionUtility;
        CleanupStack::PushL( self );
        self->ConstructL();
        User::LeaveIfError( UserSvr::DllSetTls( KVCxConnUtilAppUid, self ) );
        CleanupStack::Pop( self );
        }
    else
        {
        self = reinterpret_cast<CVcxConnectionUtility*>( instance );
        }
    self->iReferenceCount++;
       
    MPX_DEBUG1("vcxconnutil ## CVcxConnectionUtility::InstanceL() out");
    
    return self;

    }

// -----------------------------------------------------------------------------
// CVcxConnectionUtility::DecreaseReferenceCount()
// -----------------------------------------------------------------------------
//
EXPORT_C void CVcxConnectionUtility::DecreaseReferenceCount()
    {
    TAny* singleton = UserSvr::DllTls( KVCxConnUtilAppUid );

    if ( singleton == this )
        {
        iReferenceCount--;
        if( iReferenceCount == 0 )
            {
            UserSvr::DllFreeTls( KVCxConnUtilAppUid );
            delete this;
            }
        }
    }

// -----------------------------------------------------------------------------
// CVcxConnectionUtility::ConstructL()
// -----------------------------------------------------------------------------
//
void CVcxConnectionUtility::ConstructL()
    {
    MPX_DEBUG1("vcxconnutil ## CVcxNsConnectionUtility::ConstructL() in");
        
    iUtilImpl = CVcxConnUtilImpl::NewL( this );
     
    MPX_DEBUG1("vcxconnutil ## CVcxNsConnectionUtility::ConstructL() out");
    }

// -----------------------------------------------------------------------------
// CVcxConnectionUtility::CVcxNsConnectionUtility()
// -----------------------------------------------------------------------------
//
CVcxConnectionUtility::CVcxConnectionUtility( ) :
CActive( EPriorityStandard )
    {
    CActiveScheduler::Add(this); 
    }
    
// -----------------------------------------------------------------------------
// CVcxConnectionUtility::~CVcxConnectionUtility()
// -----------------------------------------------------------------------------
//
CVcxConnectionUtility::~CVcxConnectionUtility()
    {   
    MPX_DEBUG1("vcxconnutil ## CVcxConnectionUtility::~CConnectionManager in");
    CloseWaitNote(); 
  
    delete iUtilImpl;   
    iNotifier.Close();
    
    MPX_DEBUG1("vcxconnutil ## CVcxConnectionUtility::~CConnectionManager out");
    }

// -----------------------------------------------------------------------------
// CVcxConnectionUtility::RegisterObserverL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVcxConnectionUtility::RegisterObserverL( MConnUtilEngineObserver* aObserver )
   {
   iUtilImpl->RegisterObserverL( aObserver );
   }

// -----------------------------------------------------------------------------
// CVcxConnectionUtility::RemoveObserver
// -----------------------------------------------------------------------------
//
EXPORT_C void CVcxConnectionUtility::RemoveObserver( MConnUtilEngineObserver* aObserver )
   {
   iUtilImpl->RemoveObserver( aObserver );
   }

// -----------------------------------------------------------------------------
// CVcxConnectionUtility::GetIap
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVcxConnectionUtility::GetIap( TUint32& aIapId, TBool aSilent )
    {
    MPX_DEBUG1("vcxconnutil ## CVcxConnectionUtility::GetIap in ");
    TInt err( KErrNone );
    
    err = iUtilImpl->GetIap( aIapId, aSilent );
    
    MPX_DEBUG2("vcxconnutil ## CVcxConnectionUtility::GetIap out error: %d ", err );
    return err;
    }

// -----------------------------------------------------------------------------
// CVcxConnectionUtility::WapIdFromIapIdL
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CVcxConnectionUtility::WapIdFromIapIdL( TUint32 aIapId )
    {
    MPX_DEBUG1("vcxconnutil ## CVcxConnectionUtility::WapIdFromIapIdL in ");
    TUint32 wapId = iUtilImpl->WapIdFromIapIdL( aIapId );
    MPX_DEBUG1("vcxconnutil ## CVcxConnectionUtility::WapIdFromIapIdL out ");
    return wapId;
    }

// -----------------------------------------------------------------------------
// CVcxConnectionUtility::Disconnect()
// -----------------------------------------------------------------------------
//
EXPORT_C void CVcxConnectionUtility::DisconnectL()
    {
    MPX_DEBUG1( "vcxconnutil ## CVcxConnectionUtility::Disconnect in");
    if( iReferenceCount <= 1 )
        {
        iUtilImpl->DisconnectL();
        }
    MPX_DEBUG1( "vcxconnutil ## CVcxConnectionUtility::Disconnect out");
    }


void CVcxConnectionUtility::RunL()
    {
    TInt err = iStatus.Int();
    if ( iStatus.Int() == KErrCancel && 
         iUtilImpl->EngineConnectionStatus() == EVCxConnecting )
        {
        // This implifies that user has cancelled connecting dialog,
        // if currently connecting, disconnect
        TRAP_IGNORE( iUtilImpl->DisconnectL() );       
        }
    }


void CVcxConnectionUtility::DoCancel()
    {
    // complete request if we're active.
    // othervise app blocks
    if( IsActive() )
        {
        TRequestStatus* s = &iStatus;
        User::RequestComplete( s, KErrNone );    
        }
    }

// -----------------------------------------------------------------------------
// CVcxConnectionUtility::DisplayWaitNote()
// -----------------------------------------------------------------------------
//
void CVcxConnectionUtility::DisplayWaitNote(const TDesC& aConnectionName)
     {
     if( !IsActive() )
         {
         TVcxNotifierParams value;
         value.iEvent = TVcxNotifierParams::EConnecting;            
         
         value.iMsg = aConnectionName;
        
         TPckg<TVcxNotifierParams> param( value );                                   
        
         TPckgBuf<TBool> resultPckg; // Output parameters
        
         // get connection to the notify server         
         iNotifier.Close();
         
         if( iNotifier.Connect() == KErrNone )
             {        
             iStatus = KRequestPending;
             iNotifier.StartNotifierAndGetResponse( iStatus, KVcxNotifierImplUid, 
                                                     param, resultPckg );        
             SetActive();
             }
         }
     }

// -----------------------------------------------------------------------------
// CVcxConnectionUtility::CloseWaitNote()
// -----------------------------------------------------------------------------
//
void CVcxConnectionUtility::CloseWaitNote()
    {
    
    if( IsActive() )
        {
        Cancel();
        }
    
    TVcxNotifierParams value;
    value.iEvent = TVcxNotifierParams::EConnected;             
    value.iMsg.Zero();
    
    TPckg<TVcxNotifierParams> param( value );                                   

    TPckgBuf<TBool> resultPckg; 

    iNotifier.Close();
    
    if( iNotifier.Connect() == KErrNone )
        {

        TRequestStatus stat;
        iNotifier.StartNotifierAndGetResponse( stat, KVcxNotifierImplUid, param, resultPckg );
    
        User::WaitForRequest( stat );
    
        iNotifier.Close();
        }
    }

//  End of File  
