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
* Description:   Class to handle subscribtions from PS*
*/


#include "VCXTestLog.h"
#include "VCXConnUtilTestSubscriber.h"
#include "VCXConnUtilTestPSObserver.h"

const TInt KMaxStrLenght( 100 );

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::CVCXConnUtilTestSubscriber()
// -----------------------------------------------------------------------------
//
CVCXConnUtilTestSubscriber::CVCXConnUtilTestSubscriber( const TUid aUid,
                                                const TUint32 aKey,
                                                RProperty::TType aType,
                                                MVCXConnUtilTestPSObserver* aObserver ) :
    CActive( EPriorityStandard ),
    iUid( aUid ),
    iKey( aKey ),
    iKeyType(aType),
    iObserver( aObserver )
    {
        // NOP
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::NewL()
// -----------------------------------------------------------------------------
//
CVCXConnUtilTestSubscriber* CVCXConnUtilTestSubscriber::NewL( const TUid aUid,
                                                      const TUint32 aKey,
                                                      RProperty::TType aType,
                                                      MVCXConnUtilTestPSObserver* aObserver )
    {
    CVCXConnUtilTestSubscriber* self =
                      new( ELeave ) CVCXConnUtilTestSubscriber( aUid, aKey, aType, aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::ConstructL()
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTestSubscriber::ConstructL()
    {
    iInitialized = EFalse;
    User::LeaveIfError( iProperty.Attach( iUid, iKey ) );
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::~CVCXConnUtilTestSubscriber()
// -----------------------------------------------------------------------------
//
CVCXConnUtilTestSubscriber::~CVCXConnUtilTestSubscriber()
    {
    if( IsActive() )
        {
        Cancel();
        }
    iProperty.Close();

    delete iSafeWait;
    iSafeWait = NULL;
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::Get()
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTestSubscriber::Get( TInt& aValue )
    {
    return iProperty.Get( aValue );
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::Get()
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTestSubscriber::Get( TDes& aValue )
    {
    return iProperty.Get( aValue );
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::Set()
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTestSubscriber::Set( TInt& aValue )
    {
    return iProperty.Set( aValue );
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::Set()
// -----------------------------------------------------------------------------
//
TInt CVCXConnUtilTestSubscriber::Set( const TDesC& aValue )
    {
    return iProperty.Set( aValue );
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::Start()
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTestSubscriber::Start()
    {
    if( !IsActive() )
        {
        iProperty.Subscribe( iStatus );
        SetActive();
        iInitialized = ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::WaitChange()
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTestSubscriber::WaitChangeL()
    {
    if( !iSafeWait )
        {
        iSafeWait = new ( ELeave ) CActiveSchedulerWait;
        }
    if ( iSafeWait && !iSafeWait->IsStarted() )
        {
        iSafeWait->Start();
        }
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::EndWait()
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTestSubscriber::EndWait()
    {
    if ( iSafeWait && iSafeWait->IsStarted() )
        {
        iSafeWait->AsyncStop();
        }
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::DoCancel()
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTestSubscriber::DoCancel()
    {
    if( IsActive() )
        {
        iProperty.Cancel();
        }
    iInitialized = EFalse;
    }

// -----------------------------------------------------------------------------
// CVCXConnUtilTestSubscriber::RunL()
// -----------------------------------------------------------------------------
//
void CVCXConnUtilTestSubscriber::RunL()
    {
    VCXLOGLO1(">>>CVCXConnUtilTestSubscriber::RunL");
    // resubscribe before processing new
    // value to prevent missing updates
    iProperty.Subscribe( iStatus );
    SetActive();

    if( iInitialized )
        {
        TBuf< KMaxStrLenght > strValue;
        TInt intValue;

        if( iKeyType == RProperty::EInt )
            {
            // int type changed
            if( iProperty.Get( intValue ) == KErrNone && iObserver )
                {
                iObserver->ValueChangedL( iUid, iKey, intValue );
                }
            }
        else if( iKeyType == RProperty::EText )
            {
            if( iProperty.Get( strValue ) == KErrNone && iObserver )
                {
                iObserver->ValueChangedL( iUid, iKey, strValue );
                }
            }
        }
    EndWait();
    iInitialized = ETrue;
    VCXLOGLO1("<<<CVCXConnUtilTestSubscriber::RunL");
  }

//  End of File
