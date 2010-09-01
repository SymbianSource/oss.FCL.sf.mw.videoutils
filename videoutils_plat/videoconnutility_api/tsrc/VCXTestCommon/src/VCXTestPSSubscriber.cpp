/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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


#include "VCXTestLog.h"
#include "VCXTestPSSubscriber.h"
#include "VCXTestPSObserver.h"

const TInt KMaxStrLenght( 100 );

// -----------------------------------------------------------------------------
// CVCXTestPSSubscriber::CVCXTestPSSubscriber()
// -----------------------------------------------------------------------------
//
CVCXTestPSSubscriber::CVCXTestPSSubscriber( const TUid aUid,
                                                const TUint32 aKey,
                                                RProperty::TType aType,
                                                MVCXTestPSObserver* aObserver ) :
    CActive( EPriorityStandard ),
    iUid( aUid ),
    iKey( aKey ),
    iKeyType(aType),
    iObserver( aObserver )
    {
        // NOP
    }

// -----------------------------------------------------------------------------
// CVCXTestPSSubscriber::NewL()
// -----------------------------------------------------------------------------
//
EXPORT_C CVCXTestPSSubscriber* CVCXTestPSSubscriber::NewL( const TUid aUid,
                                                      const TUint32 aKey,
                                                      RProperty::TType aType,
                                                      MVCXTestPSObserver* aObserver )
    {
    CVCXTestPSSubscriber* self =
                      new( ELeave ) CVCXTestPSSubscriber( aUid, aKey, aType, aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CVCXTestPSSubscriber::ConstructL()
// -----------------------------------------------------------------------------
//
void CVCXTestPSSubscriber::ConstructL()
    {
    iInitialized = EFalse;
    User::LeaveIfError( iProperty.Attach( iUid, iKey ) );
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CVCXTestPSSubscriber::~CVCXTestPSSubscriber()
// -----------------------------------------------------------------------------
//
EXPORT_C CVCXTestPSSubscriber::~CVCXTestPSSubscriber()
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
// CVCXTestPSSubscriber::Get()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestPSSubscriber::Get( TInt& aValue )
    {
    return iProperty.Get( aValue );
    }

// -----------------------------------------------------------------------------
// CVCXTestPSSubscriber::Get()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestPSSubscriber::Get( TDes& aValue )
    {
    return iProperty.Get( aValue );
    }

// -----------------------------------------------------------------------------
// CVCXTestPSSubscriber::Set()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestPSSubscriber::Set( TInt& aValue )
    {
    return iProperty.Set( aValue );
    }

// -----------------------------------------------------------------------------
// CVCXTestPSSubscriber::Set()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CVCXTestPSSubscriber::Set( const TDesC& aValue )
    {
    return iProperty.Set( aValue );
    }

// -----------------------------------------------------------------------------
// CVCXTestPSSubscriber::Start()
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestPSSubscriber::Start()
    {
    if( !IsActive() )
        {
        iProperty.Subscribe( iStatus );
        SetActive();
        iInitialized = ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CVCXTestPSSubscriber::WaitChange()
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestPSSubscriber::WaitChangeL()
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
// CVCXTestPSSubscriber::EndWait()
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestPSSubscriber::EndWait()
    {
    if ( iSafeWait && iSafeWait->IsStarted() )
        {
        iSafeWait->AsyncStop();
        }
    }

// -----------------------------------------------------------------------------
// CVCXTestPSSubscriber::DoCancel()
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestPSSubscriber::DoCancel()
    {
    if( IsActive() )
        {
        iProperty.Cancel();
        }
    iInitialized = EFalse;
    }

// -----------------------------------------------------------------------------
// CVCXTestPSSubscriber::RunL()
// -----------------------------------------------------------------------------
//
void CVCXTestPSSubscriber::RunL()
    {
    VCXLOGLO1(">>>CVCXTestPSSubscriber::RunL");
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
    VCXLOGLO1("<<<CVCXTestPSSubscriber::RunL");
  }

//  End of File
