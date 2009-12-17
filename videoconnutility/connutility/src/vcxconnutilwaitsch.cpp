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
* Description:    CCVcxConnUtilWaitSch class definition file*
*/





#include "vcxconnutilwaitsch.h"


 /**
 * CVcxConnUtilWait is used to wrap CActiveScheduler objects to be used
 * as "wait-type" context within videoconnutility.
 * Each CVcxConnUtilWait object has a wait type -id for whose
 * object maintans an array of CActiveScheduler objects' wait state.
 */
NONSHARABLE_CLASS( CVcxConnUtilWait ) : public CBase
    {
    public: 
        
        /**
         * Destructor.
         */
        ~CVcxConnUtilWait()
            {
            iWaitArray.ResetAndDestroy();
            };
        
        /**
         * Default constructor
         */
        CVcxConnUtilWait( ) {};
        
        /**
         * Wait type id
         */
        TUint32 iType;
        
        /**
         * Flag to indicate wether CActiveScheduler maintained
         * by this object can really be released. In case flag is 
         * false, CActiveScheduler -objects need to be put to wait
         * again right after their release.  
         */
        TBool   iCanStop;
        
        /**
         * Array containing CActiveScheduler maintained
         * by this object  
         */
        RPointerArray < CActiveSchedulerWait > iWaitArray;
    };


// -----------------------------------------------------------------------------
// CVcxConnUtilWaitSch::CVcxConnUtilWaitSch()
// -----------------------------------------------------------------------------
//
CVcxConnUtilWaitSch::CVcxConnUtilWaitSch()
    {
    // No implementation required
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilWaitSch::~CVcxConnUtilWaitSch()
// -----------------------------------------------------------------------------
//
CVcxConnUtilWaitSch::~CVcxConnUtilWaitSch()
    {
    iWaits.ResetAndDestroy();
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilWaitSch::NewLC()
// -----------------------------------------------------------------------------
//
CVcxConnUtilWaitSch* CVcxConnUtilWaitSch::NewLC()
    {
    CVcxConnUtilWaitSch* self = new (ELeave)CVcxConnUtilWaitSch();
    CleanupStack::PushL(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilWaitSch::NewL()
// -----------------------------------------------------------------------------
//
CVcxConnUtilWaitSch* CVcxConnUtilWaitSch::NewL()
    {
    CVcxConnUtilWaitSch* self = CVcxConnUtilWaitSch::NewLC();
    CleanupStack::Pop( self ); // self;
    return self;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilWaitSch::WaitL()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilWaitSch::WaitL( TUint32 aWaitId )
    {
    CVcxConnUtilWait* wait = GetWaitL( aWaitId );
    CActiveSchedulerWait* activeWait = GetActiveSWaitL( wait ); 
        
    while( !wait->iCanStop )
        {
        activeWait->Start();        
        }
    
    TInt index = wait->iWaitArray.FindL( activeWait );
    if( index != KErrNotFound )
        {       
        wait->iWaitArray.Remove( index );
        }
    delete activeWait;
    
    if( !wait->iWaitArray.Count() )
        {
        index = iWaits.FindL( wait );
        if( index != KErrNotFound )
            {            
            iWaits.Remove( index );
            }
        delete wait;
        }
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilWaitSch::EndWait()
// -----------------------------------------------------------------------------
//
void CVcxConnUtilWaitSch::EndWait( TUint32 aWaitId )
    {
    TInt waitCount( 0 );
    TInt asWaitCount( 0 );
    
    waitCount = iWaits.Count();
    
    for( TInt i( 0 ); i < waitCount; ++i )
        {
        iWaits[ i ]->iCanStop = ( aWaitId == iWaits[ i ]->iType );
        
        asWaitCount = iWaits[ i ]->iWaitArray.Count();
        
        for( TInt j( 0 ); j < asWaitCount; ++j )
            {
            if( iWaits[ i ]->iWaitArray[ j ]->IsStarted() )
                {
                iWaits[ i ]->iWaitArray[ j ]->AsyncStop();
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilWaitSch::GetWaitL()
// -----------------------------------------------------------------------------
//
CVcxConnUtilWait* CVcxConnUtilWaitSch::GetWaitL( TUint32 aID )
    {
    CVcxConnUtilWait* wait( 0 );
    TInt count( 0 );
    count = iWaits.Count();
    
    for( TInt i( 0 ); i < count; ++i )
        {
        if( iWaits[ i ]->iType == aID )
            {
            wait = iWaits[ i ];
            break;
            }
        }
    if( !wait )
        {
        wait = new (ELeave) CVcxConnUtilWait();
        CleanupStack::PushL( wait );
        wait->iType = aID;
        iWaits.AppendL( wait );
        CleanupStack::Pop( wait );
        }
    return wait;
    }

// -----------------------------------------------------------------------------
// CVcxConnUtilWaitSch::GetActiveSWaitL()
// -----------------------------------------------------------------------------
//
CActiveSchedulerWait* CVcxConnUtilWaitSch::GetActiveSWaitL( CVcxConnUtilWait* aWait )
    {  
    CActiveSchedulerWait* item( 0 );
    if( aWait )
        {
        TInt count( aWait->iWaitArray.Count() );
        for( TInt i( 0 ); i < count; i++ )
            {
            if( !( aWait->iWaitArray[i]->IsStarted() ) )
                {
                item = aWait->iWaitArray[i];
                break;
                }
            }
        }
    if( !item )
        {
        item = new ( ELeave) CActiveSchedulerWait;
        CleanupStack::PushL( item );
        aWait->iWaitArray.AppendL( item );
        CleanupStack::Pop( item );
        }
    return item;
    }
// End of file

