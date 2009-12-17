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

#include "vcxconnutilcommon.h"
#include "vcxconnectionutility.hrh"
#include "vcxconnutilpubsub.h"
#include "vcxconnutilsubscriber.h"
#include "vcxconnutilpubsubobserver.h"

// CONSTANTS
_LIT( KConnUtilCounterSema, "__VcxConnUtillCounterSema__ ");
const TInt KSemaStartupValue = 1;

// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::NewL()
// -----------------------------------------------------------------------------
//
CVcxConnUtilPubSub* CVcxConnUtilPubSub::NewL( MConnUtilPubsubObserver *aObserver )
    {
    CVcxConnUtilPubSub* self = CVcxConnUtilPubSub::NewLC( aObserver );	
    CleanupStack::Pop( self ); 
    return self;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::NewLC()
// -----------------------------------------------------------------------------
//
CVcxConnUtilPubSub* CVcxConnUtilPubSub::NewLC( MConnUtilPubsubObserver *aObserver  )
    {
    CVcxConnUtilPubSub* self = new (ELeave) CVcxConnUtilPubSub( );
    CleanupStack::PushL( self );
    self->ConstructL( aObserver );
    return self;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::CVcxNsConnUtilEngine()
// -----------------------------------------------------------------------------
//
CVcxConnUtilPubSub::CVcxConnUtilPubSub( )
    {
    // NOP
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::ConstructL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilPubSub::ConstructL( MConnUtilPubsubObserver *aObserver )
    {
    MPX_DEBUG1("vcxconnutil ## CVcxConnUtilPubSub::ConstructL() in");

    TInt status( KErrNone );
    // semafore for protecting P&S -counter value writing
    status = iSemaCounter.CreateGlobal( KConnUtilCounterSema, KSemaStartupValue );
    if( status == KErrAlreadyExists )
        {
        // if semafore exists already, just open handle to existing
        status = iSemaCounter.OpenGlobal( KConnUtilCounterSema );
        }
    User::LeaveIfError( status );

    User::LeaveIfError( 
            iPropertyCurrentIAPId.Attach( KVcxConnUtilPScategory, 
                                          EVCxPSIapId ) );
    
    User::LeaveIfError( 
            iPropertyCurrentSNAPId.Attach( KVcxConnUtilPScategory, 
                                           EVCxPSSnapId ) );
    
    User::LeaveIfError( 
            iPropertyNbrOfInst.Attach( KVcxConnUtilPScategory, 
                                       EVCxPSNbrConnInstances ) );
    
    User::LeaveIfError(
            iPropertyAccepted.Attach( KVcxConnUtilPScategory, 
                                      EVCxPSNbRoamAccepted ) );
 
    iConnectionStatePS = 
        CVcxConnUtilSubscriber::NewL( KVcxConnUtilPScategory, 
                                      EVCxPSConnectionStatus, 
                                      RProperty::EInt, aObserver );
    
    iMasterExistPS =     
        CVcxConnUtilSubscriber::NewL( KVcxConnUtilPScategory, 
                                      EVCxPSMasterExists, 
                                      RProperty::EInt, aObserver ); 
    
    iRoamingReqStatusPS = 
        CVcxConnUtilSubscriber::NewL( KVcxConnUtilPScategory, 
                                      EVCxPSRoamingRequestStatus, 
                                      RProperty::EInt, aObserver );  
    
    iNbrInstanceResp = 
        CVcxConnUtilSubscriber::NewL( KVcxConnUtilPScategory, 
                                      EVCxPSNbrRoamResp, 
                                      RProperty::EInt, aObserver );
        
    if( GetValue( EVCxPSIapId, status ) == KErrNotFound )
        {
        MPX_DEBUG1("vcxconnutil ## CVcxConnUtilPubSub::ConstructL() no properties");
        
        // define properties
        DefinePropertiesL();           
        }
    
    MPX_DEBUG1("vcxconnutil ## CVcxConnUtilPubSub::ConstructL() out");
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::~CVcxConnUtilPubSub()
// -----------------------------------------------------------------------------
//   
CVcxConnUtilPubSub::~CVcxConnUtilPubSub()
    {
    iSemaCounter.Close();	
    	
    iPropertyCurrentIAPId.Close();
    iPropertyCurrentSNAPId.Close();
    iPropertyNbrOfInst.Close();
    iPropertyAccepted.Close();
    
    delete iConnectionStatePS;
    delete iMasterExistPS;
    delete iRoamingReqStatusPS;
    delete iNbrInstanceResp;
    
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::DefinePropertiesL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilPubSub::DefinePropertiesL()
    {
    MPX_DEBUG1("vcxconnutil ## CVcxConnUtilPubSub::DefinePropertiesL() in");
    TInt result( KErrNone );
    RSemaphore semaphore;
    result = semaphore.CreateGlobal( KVcxConnUtilPSSema, 0 );
    if( result != KErrNone && result != KErrAlreadyExists )
        {
        // kernel cannot create semaphore, or there is no existing
        User::LeaveIfError( result );
        }
    if( result == KErrAlreadyExists )
        {
        // semaphore exist. Some other process is already defining PS
        // open semaphore and wait for it to finish
        User::LeaveIfError( semaphore.OpenGlobal( KVcxConnUtilPSSema ) );
        semaphore.Wait();
        // check if PS keys do exists
        TInt value( 0 );
        if( GetValue( EVCxPSIapId, value ) == KErrNotFound )
            {
            // other proces has failed to define PS, 
            // this process will try it
            result = KErrNone;
            }                
        }
    if( result == KErrNone )
        {
        const TUidType createPSUid( KNullUid, KNullUid, KVcxConnUtilPScategory );
       
        RProcess createPS;
        result = createPS.Create( KVcxConnUtilPSWorkerName(), KNullDesC(), createPSUid );
        if( result == KErrNone )
            {
            createPS.Resume();
            createPS.Close();
            semaphore.Wait();
            // init default values for keys, whose default differs from 0
            SetValue( EVCxPSRoamingRequestStatus, EVCxRoamingNotInit );
            SetValue( EVCxPSConnectionStatus, EVCxNotConnected );         
            }
        }
    // need to signal here for other possible
    // processes to proceed
    semaphore.Signal();
    semaphore.Close();  
    if( result != KErrNone && result != KErrAlreadyExists )
        {
        // all other erros cause leave: worker process creation fails, or
        // kernel refuses to open existing semphore (== platform problem)
        User::LeaveIfError( result );
        }
    MPX_DEBUG1("vcxconnutil ## CVcxConnUtilPubSub::DefinePropertiesL() out");
    }


RProperty* CVcxConnUtilPubSub::GetProperty( TUint32 aKey )
    {
    switch( aKey )
        {
        case EVCxPSConnectionStatus:
            return &( iConnectionStatePS->Property() );
            
        case EVCxPSIapId:
            return &iPropertyCurrentIAPId;
            
        case EVCxPSSnapId:
            return &iPropertyCurrentSNAPId;
            
        case EVCxPSMasterExists:
            return &( iMasterExistPS->Property() );
            
        case EVCxPSNbrConnInstances:
            return &iPropertyNbrOfInst;
            
        case EVCxPSRoamingRequestStatus:
            return &( iRoamingReqStatusPS->Property() );
            
        case EVCxPSNbrRoamResp:    
            return &( iNbrInstanceResp->Property() );
            
        case EVCxPSNbRoamAccepted:    
            return &iPropertyAccepted;
            
        default:
            break;
        }
    return 0;
    }

CVcxConnUtilSubscriber* CVcxConnUtilPubSub::GetSubscriber( TUint32 aKey )
    {
    switch( aKey )
        {  
        case EVCxPSConnectionStatus:
            return iConnectionStatePS;
            
        case EVCxPSMasterExists:
            return iMasterExistPS;
            
        case EVCxPSRoamingRequestStatus:
            return iRoamingReqStatusPS;
            
        case EVCxPSNbrRoamResp:    
            return iNbrInstanceResp;   
            
        default:
            break;
        }
        return 0;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::IncCounterPubSubL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilPubSub::IncCounterPubSubL( TUint32 aKey )
    {
    const TInt KIncValue( 1 );
    DoPSCounterIncrementL( aKey, KIncValue );   
    }
  
// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::DecNbrOfConnUsersPubSub()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilPubSub::DecCounterPubSubL( TUint32 aKey )
    {
    const TInt KDecValue( -1 );
    DoPSCounterIncrementL( aKey, KDecValue );      
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::DoPSCounterIncrementL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilPubSub::DoPSCounterIncrementL( TUint32 aKey, TInt aCalcFactor )
    {
    if( aKey == EVCxPSNbrConnInstances || 
        aKey == EVCxPSNbrRoamResp      ||
        aKey == EVCxPSNbRoamAccepted )
        {
        RProperty* property = GetProperty( aKey );
        if( property )
            {
             TInt value( 0 );
             TInt err( KErrNone );
             // entering critical section
             iSemaCounter.Wait();
             err = property->Get( value );
             if( err == KErrNone )
                 {
                 value += aCalcFactor;
                 err = property->Set( value ); 
                 }
             // leaving critical section 
             iSemaCounter.Signal();
             User::LeaveIfError( err );  
             }
         }	    	
    }
    
// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::SetValue()
// -----------------------------------------------------------------------------
//
TInt CVcxConnUtilPubSub::SetValue( TUint32 aKey, TInt aValue )
    {
    TInt err( KErrNone );
    RProperty* property = GetProperty( aKey );
    if( property )
        {
        err = property->Set( aValue );
        }
    return err;
    }
 
// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::GetValue()
// -----------------------------------------------------------------------------
//
TInt CVcxConnUtilPubSub::GetValue( TUint32 aKey, TInt& aValue )
    {
    TInt err( KErrNone );
    RProperty* property = GetProperty( aKey );
    if( property )
        {
        err = property->Get( aValue );
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::CancelValueSubscribers()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilPubSub::StartSubscibers( TUint32 aKey )
    {
    if( !aKey )
        {
        iConnectionStatePS->Start();
        iMasterExistPS->Start();
        iRoamingReqStatusPS->Start();
        iNbrInstanceResp->Start();
        }
    else
        {
        CVcxConnUtilSubscriber* subscriber = GetSubscriber( aKey );
        if( subscriber )
            {
            subscriber->Start();
            }
        }
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilPubSub::CancelValueSubscribers()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilPubSub::CancelValueSubscribers( TUint32 aKey )
    {
    if( !aKey )
        {
        iConnectionStatePS->Cancel();
        iMasterExistPS->Cancel();
        iRoamingReqStatusPS->Cancel();
        iNbrInstanceResp->Cancel();
        }
    else
        {
        CVcxConnUtilSubscriber* subscriber = GetSubscriber( aKey );
        if( subscriber )
            {
            subscriber->Cancel();
            }
        }
    }
// end of file
