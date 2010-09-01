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
* Description:    Class to handle subscribtions from PS*
*/



#include "vcxconnutilsubscriber.h"
#include "vcxconnutilpubsubobserver.h"


// -----------------------------------------------------------------------------
// CVcxConnUtilSubscriber::CVcxConnUtilSubscriber()
// -----------------------------------------------------------------------------
//
CVcxConnUtilSubscriber::CVcxConnUtilSubscriber( const TUid aUid, 
                                                const TUint32 aKey, 
                                                RProperty::TType aType,
                                                MConnUtilPubsubObserver* aObserver ) :
    CActive( EPriorityStandard ), 
    iUid( aUid ),
    iKey( aKey ), 
    iKeyType(aType), 
    iObserver( aObserver )
    {
        // NOP
    }
 
// -----------------------------------------------------------------------------
// CVcxConnUtilSubscriber::NewL()
// -----------------------------------------------------------------------------
//
CVcxConnUtilSubscriber* CVcxConnUtilSubscriber::NewL( const TUid aUid, 
                                                      const TUint32 aKey,
                                                      RProperty::TType aType, 
                                                      MConnUtilPubsubObserver* aObserver )
    {
    CVcxConnUtilSubscriber* self =
                      new( ELeave ) CVcxConnUtilSubscriber( aUid, aKey, aType, aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
 
// -----------------------------------------------------------------------------
// CVcxConnUtilSubscriber::ConstructL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilSubscriber::ConstructL()
    {
    iInitialized = EFalse;
    User::LeaveIfError( iProperty.Attach( iUid, iKey ) );
    CActiveScheduler::Add( this );    
    }
 
// -----------------------------------------------------------------------------
// CVcxConnUtilSubscriber::~CVcxConnUtilSubscriber()
// -----------------------------------------------------------------------------
//
CVcxConnUtilSubscriber::~CVcxConnUtilSubscriber()
    {
    if( IsActive() )
        {
        Cancel();
        }
    iProperty.Close();
    
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilSubscriber::Property()
// -----------------------------------------------------------------------------
//
RProperty& CVcxConnUtilSubscriber::Property()
    {
    return iProperty;
    }
  
// -----------------------------------------------------------------------------
// CVcxConnUtilSubscriber::Start()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilSubscriber::Start()
    {
    if( !IsActive() )
        {
        iProperty.Subscribe( iStatus );
        SetActive();
        iInitialized = ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilSubscriber::DoCancel()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilSubscriber::DoCancel()
    {
    if( IsActive() )
        {
        iProperty.Cancel();
        }   
    iInitialized = EFalse;
    }
 
// -----------------------------------------------------------------------------
// CVcxConnUtilSubscriber::RunL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilSubscriber::RunL()
    {
    // resubscribe before processing new 
    // value to prevent missing updates
    iProperty.Subscribe( iStatus );
    SetActive();

    if( iInitialized )
        {           
        if(iKeyType == RProperty::EInt )
            {
            TInt intValue;
            // int type changed
            if( iProperty.Get( intValue ) == KErrNone && iObserver )
                {                
                TRAP_IGNORE( iObserver->ValueChangedL( iKey, intValue ) );
                }
            }
        }
    iInitialized = ETrue;
  }
// end of file
