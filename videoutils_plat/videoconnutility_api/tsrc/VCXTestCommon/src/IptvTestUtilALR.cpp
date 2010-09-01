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


// INCLUDE FILES

#include <centralrepository.h>

#include "IptvTestUtilALR.h"
#include "VCXTestLog.h"
#include "TestUtilConnection.h"
#include "CIptvTestActiveWait.h"
#include "TestUtilConnectionWaiter.h"
#include "VCXTestConstants.h"
#include <cmpluginpacketdatadef.h>

// ========================== MEMBER FUNCTIONS ===============================

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::NewL()
//
// Constructs CIptvTestUtilALR object
// ---------------------------------------------------------------------------
//
EXPORT_C CIptvTestUtilALR* CIptvTestUtilALR::NewL()
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::NewL");
    CIptvTestUtilALR* self = NewLC();
    CleanupStack::Pop(self);
    VCXLOGLO1("<<<CIptvIptvTestUtilALR::NewL");
    return self;
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::NewLC()
//
// Constructs CIptvTestUtilALR object
// ---------------------------------------------------------------------------
//
EXPORT_C CIptvTestUtilALR* CIptvTestUtilALR::NewLC()
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::NewLC");
    CIptvTestUtilALR* self = new (ELeave) CIptvTestUtilALR();
    CleanupStack::PushL(self);
    self->ConstructL();
    VCXLOGLO1("<<<CIptvIptvTestUtilALR::NewLC");
    return self;
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::~CIptvTestUtilALR()
//
// Destructor
// ---------------------------------------------------------------------------
//
EXPORT_C CIptvTestUtilALR::~CIptvTestUtilALR()
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::~CIptvIptvTestUtilALR");

	iCmManager.Close();

    delete iActiveWait;
    iActiveWait = NULL;

    VCXLOGLO1("<<<CIptvIptvTestUtilALR::~CIptvIptvTestUtilALR");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::ConstructL()
//
// Second phase constructor
// ---------------------------------------------------------------------------
//
void CIptvTestUtilALR::ConstructL()
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::ConstructL");

    iCmManager.OpenL();

    iActiveWait = CIptvTestActiveWait::NewL();

    VCXLOGLO1("<<<CIptvIptvTestUtilALR::ConstructL");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::CIptvTestUtilALR()
//
// Constructor
// ---------------------------------------------------------------------------
//
CIptvTestUtilALR::CIptvTestUtilALR()
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::CIptvIptvTestUtilALR");

    VCXLOGLO1("<<<CIptvIptvTestUtilALR::CIptvIptvTestUtilALR");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::RemoveUsedDestinationCenRepL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CIptvTestUtilALR::RemoveUsedDestinationCenRepL()
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::RemoveUsedDestinationCenRep");

    CRepository* cenRep = CRepository::NewLC( VCXTEST_KIptvAlrCenRepUid );
    User::LeaveIfError( cenRep->Delete( VCXTEST_KIptvCenRepUsedSnapIdKey ) );
    CleanupStack::PopAndDestroy( cenRep );

   	VCXLOGLO1("<<<CIptvIptvTestUtilALR::RemoveUsedDestinationCenRep");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::SetUsedDestinationL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CIptvTestUtilALR::SetUsedDestinationL( TDesC& aDestinationName )
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::SetUsedDestinationL");

    VCXLOGLO2("CIptvIptvTestUtilALR:: dest name: %S", &aDestinationName);

    TInt destId(0);

    if( aDestinationName.Compare( _L("ALWAYS_ASK") ) != KErrNone )
        {
        destId = GetDestinationIdL( aDestinationName );
        }
    VCXLOGLO2("CIptvIptvTestUtilALR:: dest id: %d", destId);
    
    TCmDefConnValue defaultConnection;
    defaultConnection.iType = ECmDefConnDestination;
    defaultConnection.iId = destId;
    iCmManager.WriteDefConnL( defaultConnection );

   	VCXLOGLO1("<<<CIptvIptvTestUtilALR::SetUsedDestinationL");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::CreateDestinationL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CIptvTestUtilALR::CreateDestinationL( TDesC& aDestinationName )
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::CreateDestinationL");

    VCXLOGLO2("CIptvIptvTestUtilALR:: dest name: %S", &aDestinationName);

	RCmDestinationExt dest;
	TRAPD( err, dest = iCmManager.CreateDestinationL( aDestinationName ) );
	VCXLOGLO2("CIptvIptvTestUtilALR::CreateDestinationL: CreateDestinationL returned: %d", err);
	User::LeaveIfError( err );
	
	CleanupClosePushL( dest );
	TRAP( err, dest.SetHiddenL( EFalse ) );
	VCXLOGLO2("CIptvIptvTestUtilALR::CreateDestinationL: SetHiddenL returned: %d", err);
	User::LeaveIfError( err );
	
	// This leaves sometimes with KErrLocked, try again in that case few times. 
	for( TInt retry = 0; retry < 5; retry++ )
	    {
	    TRAP( err, dest.UpdateL() );
	    VCXLOGLO2("CIptvIptvTestUtilALR::CreateDestinationL: UpdateL returned: %d", err);
	    if( err != KErrLocked )
	        {
	        User::LeaveIfError( err );
	        break; // All ok.
	        }
	    User::After( 1000000 );
	    }

	CleanupStack::PopAndDestroy( &dest );

   	VCXLOGLO1("<<<CIptvIptvTestUtilALR::CreateDestinationL");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::DeleteDestinationL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CIptvTestUtilALR::DeleteDestinationL( TDesC& aDestinationName )
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::DeleteDestinationL");

    VCXLOGLO2("CIptvIptvTestUtilALR:: dest name: '%S'", &aDestinationName);

    TInt destId = GetDestinationIdL( aDestinationName );
    RCmDestinationExt destination = iCmManager.DestinationL( destId );
    CleanupClosePushL( destination );

    TRAP_IGNORE( TerminateDestinationConnectionsL( aDestinationName ) );

    // Delete methods first.
    for(TInt e=destination.ConnectionMethodCount()-1; e>=0 ; e--)
        {
        RCmConnectionMethodExt method = destination.ConnectionMethodL( e );
        CleanupClosePushL( method );
        VCXLOGLO2( "CIptvIptvTestUtilALR:: deleting method: %d", e );

        destination.DeleteConnectionMethodL( method  );
        destination.UpdateL();
        CleanupStack::PopAndDestroy( &method );
        }

    VCXLOGLO2( "CIptvIptvTestUtilALR:: deleting destination id: %d", destId );
    destination.DeleteLD();
    
    CleanupStack::PopAndDestroy( &destination );

   	VCXLOGLO1("<<<CIptvIptvTestUtilALR::DeleteDestinationL");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::CopyMethodL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CIptvTestUtilALR::CopyMethodL( TDesC& aMethodName, TDesC& aNewMethodName, TDesC& aNewDestinationName )
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::CopyMethodL");

    TBuf<256> srcMethodName( aMethodName );

	VCXLOGLO2( "CIptvIptvTestUtilALR:: srcMethodName: %S", &srcMethodName );
	VCXLOGLO2( "CIptvIptvTestUtilALR:: aNewMethodName: %S", &aNewMethodName );
	VCXLOGLO2( "CIptvIptvTestUtilALR:: aNewDestinationName: %S", &aNewDestinationName );
	
	// Find target destination.
    TInt targetDestId = GetDestinationIdL( aNewDestinationName );
    RCmDestinationExt targetDestination = iCmManager.DestinationL( targetDestId );
    CleanupClosePushL( targetDestination );

    // Check if target has destination with the name already.
    for(TInt e=0; e<targetDestination.ConnectionMethodCount(); e++)
    	{
    	RCmConnectionMethodExt method = targetDestination.ConnectionMethodL( e );
    	CleanupClosePushL( method );

    	HBufC* methodName = method.GetStringAttributeL( CMManager::ECmName );
    	CleanupStack::PushL( methodName );

    	VCXLOGLO2( "CIptvIptvTestUtilALR:: method: '%S'", methodName );
    	if( MethodMatchesL( srcMethodName, *methodName, ETrue ) )
    	    {
			VCXLOGLO1( "CIptvIptvTestUtilALR:: Target destination already has method with the name.");
        	CleanupStack::PopAndDestroy( methodName );
    		CleanupStack::PopAndDestroy( &method );
    		CleanupStack::PopAndDestroy( &targetDestination );
            VCXLOGLO1("<<<CIptvIptvTestUtilALR::CopyMethodL");
            return;
    		}
    	CleanupStack::PopAndDestroy( methodName );
		CleanupStack::PopAndDestroy( &method );
    	}

	// Find method from destinations and create copy of it.
	TBool found( EFalse );
	TBool foundExact( EFalse );
    TUint32 foundDestId( 0 );
    TInt foundIapIndex( 0 );

	RArray<TUint32> destIds(5);
	iCmManager.AllDestinationsL( destIds );
	VCXLOGLO2( "CIptvIptvTestUtilALR:: dest Count: %d", destIds.Count() );
	
	for(TInt i=0; i<destIds.Count(); i++ )
	    {
	    RCmDestinationExt sourceDestination = iCmManager.DestinationL( destIds[i] );
	    CleanupClosePushL( sourceDestination );

	    HBufC* name = sourceDestination.NameLC();
	    VCXLOGLO2( "CIptvIptvTestUtilALR:: dest: '%S'", name );
		TInt methodCount = sourceDestination.ConnectionMethodCount();
		VCXLOGLO2( "CIptvIptvTestUtilALR:: methodCount: %d", methodCount );

	    for(TInt e=0; e<sourceDestination.ConnectionMethodCount(); e++)
	    	{
	    	RCmConnectionMethodExt sourceMethod = sourceDestination.ConnectionMethodL( e );
	    	CleanupClosePushL( sourceMethod );

	    	HBufC* methodName = sourceMethod.GetStringAttributeL( CMManager::ECmName );
	    	CleanupStack::PushL( methodName );
	    	
	    	TInt iapId = sourceMethod.GetIntAttributeL( CMManager::ECmIapId );
	    	VCXLOGLO3( "CIptvIptvTestUtilALR:: method: '%S', id: %d", methodName, iapId );

            // Check for exact match.
            if( MethodMatchesL( srcMethodName, *methodName, ETrue ) )
                {
                VCXLOGLO1( "CIptvIptvTestUtilALR:: exact match.");
                foundExact = ETrue;
                foundDestId = destIds[i];
                foundIapIndex = e;
                }

	    	// No exact match found yet, check if its any of the alternative iaps.
	    	if( !foundExact && MethodMatchesL( srcMethodName, *methodName, EFalse ) )
	    	    {
	    	    VCXLOGLO1( "CIptvIptvTestUtilALR:: alt match.");
	    	    found = ETrue;
                foundDestId = destIds[i];
                foundIapIndex = e;
	    	    }

	    	CleanupStack::PopAndDestroy( methodName );
    		CleanupStack::PopAndDestroy( &sourceMethod );
    		
    		// No need to search further.
    		if( foundExact )
    		    {
    		    break;
    		    }
	    	}

        CleanupStack::PopAndDestroy( name );
        CleanupStack::PopAndDestroy( &sourceDestination );

		// No need to search further.
		if( foundExact )
		    {
		    break;
		    }        
	    }

    // Iap found, create copy of it.
    if( foundExact || found )
        {
        VCXLOGLO1( "CIptvIptvTestUtilALR:: Creating copy of the method." );
        
        RCmDestinationExt sourceDestination = iCmManager.DestinationL( foundDestId );
	    CleanupClosePushL( sourceDestination );
	    
	    VCXLOGLO1( "CIptvIptvTestUtilALR:: source dest ok." );

    	RCmConnectionMethodExt sourceMethod = sourceDestination.ConnectionMethodL( foundIapIndex );
    	CleanupClosePushL( sourceMethod );
    	
    	VCXLOGLO1( "CIptvIptvTestUtilALR:: source method ok." );
    	
        HBufC* sourceMethodName = sourceMethod.GetStringAttributeL( CMManager::ECmName );
        CleanupStack::PushL( sourceMethodName );
        
        VCXLOGLO1( "CIptvIptvTestUtilALR:: source method name ok." );

		RCmConnectionMethodExt newMethod = sourceMethod.CreateCopyL();
		CleanupClosePushL( newMethod );
		
		VCXLOGLO1( "CIptvIptvTestUtilALR:: copy ok." );
		
		// If method names are same rename source method because there can't be two with same name.
        if( *sourceMethodName == aNewMethodName )
            {
            VCXLOGLO1( "CIptvIptvTestUtilALR:: Method names are same, renaming source method." );
            HBufC* newName = HBufC::NewL( sourceMethodName->Length() + 5);
            newName->Des().Append( *sourceMethodName );
            newName->Des().Append( _L("_old") );
            sourceMethod.SetStringAttributeL( CMManager::ECmName, *newName );
            delete newName;
            sourceMethod.UpdateL();            
            }
		
		// Set name for new method.
        newMethod.SetStringAttributeL( CMManager::ECmName, aNewMethodName );
		// Disable dialog asking permission.
		newMethod.SetIntAttributeL( CMManager::ECmSeamlessnessLevel, CMManager::ESeamlessnessShowprogress );
		
		VCXLOGLO1( "CIptvIptvTestUtilALR:: new method updated." );
		
		targetDestination.AddConnectionMethodL( newMethod );
		targetDestination.UpdateL();
		
		VCXLOGLO1( "CIptvIptvTestUtilALR:: method added to target dest." );
		
		CleanupStack::PopAndDestroy( &newMethod );
		CleanupStack::PopAndDestroy( sourceMethodName );
		CleanupStack::PopAndDestroy( &sourceMethod );
		CleanupStack::PopAndDestroy( &sourceDestination );
        }
	// No iap found, search from uncategorized IAPs.
    else
		{
		RArray<TUint32> cmarray;
		TRAPD( err, iCmManager.ConnectionMethodL( cmarray, EFalse, EFalse, EFalse ) );
		if( err != KErrNone )
		    {
		    VCXLOGLO2( "CIptvIptvTestUtilALR:: could not get uncategorized IAPs. err: %d", err );
		    cmarray.Reset();
		    cmarray.Close();
		    }

		VCXLOGLO2( "CIptvIptvTestUtilALR:: uncategorized IAP count: %d", cmarray.Count() );

		for( TInt e = 0; e < cmarray.Count(); e++ )
            {
            RCmConnectionMethodExt sourceMethod = iCmManager.ConnectionMethodL( cmarray[e] );
            CleanupClosePushL( sourceMethod );

            HBufC* methodName = sourceMethod.GetStringAttributeL( CMManager::ECmName );
            CleanupStack::PushL( methodName );
            
            TInt iapId = sourceMethod.GetIntAttributeL( CMManager::ECmIapId );
            VCXLOGLO3( "CIptvIptvTestUtilALR:: method: '%S', id: %d", methodName, iapId );

            // Check for exact match.
            if( MethodMatchesL( srcMethodName, *methodName, ETrue ) )
                {
                VCXLOGLO1( "CIptvIptvTestUtilALR:: exact match.");
                foundExact = ETrue;
                foundIapIndex = e;
                }

	    	// No exact match found yet, check if its any of the alternative iaps.
	    	if( !foundExact && MethodMatchesL( srcMethodName, *methodName, EFalse ) )
	    	    {
	    	    VCXLOGLO1( "CIptvIptvTestUtilALR:: alt match.");
	    	    found = ETrue;
                foundIapIndex = e;
	    	    }

            CleanupStack::PopAndDestroy( methodName );
            CleanupStack::PopAndDestroy( &sourceMethod );

    		// No need to search further.
    		if( foundExact )
    		    {
    		    break;
    		    }
            }

        // Found IAP from uncategorized dest.
        if( found || foundExact )
            {
            VCXLOGLO1( "CIptvIptvTestUtilALR:: Creating copy of the method." );

            RCmConnectionMethodExt sourceMethod = iCmManager.ConnectionMethodL( cmarray[foundIapIndex] );
            CleanupClosePushL( sourceMethod );

            HBufC* sourceMethodName = sourceMethod.GetStringAttributeL( CMManager::ECmName );
            CleanupStack::PushL( sourceMethodName );
            
            RCmConnectionMethodExt newMethod = sourceMethod.CreateCopyL();
            VCXLOGLO1( "CIptvIptvTestUtilALR:: New Method ok." );
            CleanupClosePushL( newMethod );
            
            // If method names are same rename source method because there can't be two with same name.
            if( *sourceMethodName == aNewMethodName )
                {
                VCXLOGLO1( "CIptvIptvTestUtilALR:: Method names are same, renaming source method." );
                HBufC* newName = HBufC::NewL( sourceMethodName->Length() + 5);
                newName->Des().Append( *sourceMethodName );
                newName->Des().Append( _L("_old") );
                sourceMethod.SetStringAttributeL( CMManager::ECmName, *newName );
                delete newName;
                sourceMethod.UpdateL();
                }            
            
            // Set name for new method.
            newMethod.SetStringAttributeL( CMManager::ECmName, aNewMethodName );
            // Disable dialog asking permission.
            newMethod.SetIntAttributeL( CMManager::ECmSeamlessnessLevel, CMManager::ESeamlessnessShowprogress );
            
            targetDestination.AddConnectionMethodL( newMethod );
            targetDestination.UpdateL();

            CleanupStack::PopAndDestroy( &newMethod );            
            CleanupStack::PopAndDestroy( sourceMethodName );
            CleanupStack::PopAndDestroy( &sourceMethod );            
            }

        cmarray.Reset();
        cmarray.Close();
		}

	destIds.Reset();
	destIds.Close();

	PrintDestinationL( targetDestination );
	CleanupStack::PopAndDestroy( &targetDestination );
	
	if( !foundExact && !found )
	    {
	    VCXLOGLO2( "CIptvIptvTestUtilALR:: source method %S not found!", &srcMethodName );
#ifdef __WINSCW__
	    VCXLOGLO1( "CIptvIptvTestUtilALR:: EMULATOR BUILD, check that iap is not uncategorized." );
#endif
	    User::Leave(KErrNotFound);
	    }	

   	VCXLOGLO1("<<<CIptvIptvTestUtilALR::CopyMethodL");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::DeleteMethodL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CIptvTestUtilALR::DeleteMethodL( TDesC& aDestinationName, TDesC& aMethodName )
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::DeleteMethodL");

	VCXLOGLO2( "CIptvIptvTestUtilALR:: aDestinationName: %S", &aDestinationName );
	VCXLOGLO2( "CIptvIptvTestUtilALR:: aMethodName: %S", &aMethodName );

    TInt destId = GetDestinationIdL( aDestinationName );
    RCmDestinationExt destination = iCmManager.DestinationL( destId );
    CleanupClosePushL( destination );

	TBool found( EFalse );

	// Find method and delete it.
	for(TInt e=0; e< destination.ConnectionMethodCount(); e++)
    	{
    	RCmConnectionMethodExt method = destination.ConnectionMethodL( e );
    	CleanupClosePushL( method );

    	HBufC* methodName = method.GetStringAttributeL( CMManager::ECmName );
        CleanupStack::PushL( methodName );
        TInt iapId = method.GetIntAttributeL( CMManager::ECmIapId );
        VCXLOGLO3( "CIptvIptvTestUtilALR:: method: '%S', id: %d", methodName, iapId );

    	if( methodName->Compare( aMethodName ) == 0 )
    		{
			VCXLOGLO1( "CIptvIptvTestUtilALR:: Match. Deleting." );
			found = ETrue;
			destination.DeleteConnectionMethodL( method  );
			destination.UpdateL();
    		}
    	CleanupStack::PopAndDestroy( methodName );
    	CleanupStack::PopAndDestroy( &method );
    	}

	if(!found)
		{
		VCXLOGLO1( "CIptvIptvTestUtilALR:: method not found!");
		User::Leave(KErrNotFound);
		}

	PrintDestinationL( destination );
	CleanupStack::PopAndDestroy( &destination );

   	VCXLOGLO1("<<<CIptvIptvTestUtilALR::DeleteMethodL");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::SetMethodPriorityL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CIptvTestUtilALR::SetMethodPriorityL( TDesC& aDestinationName, TDesC& aMethodName, TUint32 aPriority )
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::SetMethodPriorityL");

	VCXLOGLO2( "CIptvIptvTestUtilALR:: aDestinationName: %S", &aDestinationName );
	VCXLOGLO2( "CIptvIptvTestUtilALR:: aMethodName: %S", &aMethodName );
	VCXLOGLO2( "CIptvIptvTestUtilALR:: aPriority: %d", aPriority );

    TInt destId = GetDestinationIdL( aDestinationName );
    RCmDestinationExt destination = iCmManager.DestinationL( destId );
    CleanupClosePushL( destination );

	TBool found( EFalse );

	if( aPriority > destination.ConnectionMethodCount() )
		{
		VCXLOGLO2( "CIptvIptvTestUtilALR:: Priority must be less than there's methods in the destination, count: %d!", destination.ConnectionMethodCount() );
		User::Leave( KErrCorrupt );
		}

	// No need to set priority here.
	if( destination.ConnectionMethodCount() <= 1 )
	    {
	    CleanupStack::PopAndDestroy( &destination );
	    VCXLOGLO1("<<<CIptvIptvTestUtilALR::SetMethodPriorityL");
	    return;
	    }

	// Find the method and set priority
	for(TInt e=0; e < destination.ConnectionMethodCount(); e++)
    	{
    	VCXLOGLO1( "CIptvIptvTestUtilALR:: Get method." );
    	RCmConnectionMethodExt method = destination.ConnectionMethodL( e );
    	CleanupClosePushL( method );

    	HBufC* methodName = method.GetStringAttributeL( CMManager::ECmName );
        CleanupStack::PushL( methodName );
        VCXLOGLO1( "CIptvIptvTestUtilALR:: Get iap id." );
        TInt iapId = method.GetIntAttributeL( CMManager::ECmIapId );
        VCXLOGLO3( "CIptvIptvTestUtilALR:: method: '%S', id: %d", methodName, iapId );

    	if( methodName->Compare( aMethodName ) == 0 )
    		{
			VCXLOGLO1( "CIptvIptvTestUtilALR:: Match. Setting priority." );
			found = ETrue;
			destination.ModifyPriorityL( method, aPriority );
			destination.UpdateL();
    		}
    	CleanupStack::PopAndDestroy( methodName );
    	CleanupStack::PopAndDestroy( &method );
    	}

	if(!found)
		{
		VCXLOGLO1( "CIptvIptvTestUtilALR:: method not found!");
		User::Leave(KErrNotFound);
		}

	PrintDestinationL( destination );
	CleanupStack::PopAndDestroy( &destination );

   	VCXLOGLO1("<<<CIptvIptvTestUtilALR::SetMethodPriorityL");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::SetMethodStringAttributeL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CIptvTestUtilALR::SetMethodStringAttributeL( TDesC& aDestinationName, TDesC& aMethodName, TInt aAttribute, TDesC& aAttributeValue )
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::SetMethodStringAttributeL");

    VCXLOGLO2( "CIptvIptvTestUtilALR:: aDestinationName: %S", &aDestinationName );
    VCXLOGLO2( "CIptvIptvTestUtilALR:: aMethodName: %S", &aMethodName );
    VCXLOGLO2( "CIptvIptvTestUtilALR:: aAttribute: %d", aAttribute );
    VCXLOGLO2( "CIptvIptvTestUtilALR:: aAttributeValue: %S", &aAttributeValue );

    TInt destId = GetDestinationIdL( aDestinationName );
    RCmDestinationExt destination = iCmManager.DestinationL( destId );
    CleanupClosePushL( destination );

    TBool found( EFalse );

    // Find the method and set attribute
    for(TInt e=0; e < destination.ConnectionMethodCount(); e++)
        {
        RCmConnectionMethodExt method = destination.ConnectionMethodL( e );
        CleanupClosePushL( method );

        HBufC* methodName = method.GetStringAttributeL( CMManager::ECmName );
        CleanupStack::PushL( methodName );
        TInt iapId = method.GetIntAttributeL( CMManager::ECmIapId );
        VCXLOGLO3( "CIptvIptvTestUtilALR:: method: '%S', id: %d", methodName, iapId );

        if( MethodMatchesL( aMethodName, *methodName, EFalse ) )
            {
            VCXLOGLO1( "CIptvIptvTestUtilALR:: Match. Setting attribute." );
            found = ETrue;

            method.SetStringAttributeL( aAttribute, aAttributeValue );
            method.UpdateL();
            }
            
        CleanupStack::PopAndDestroy( methodName );
        CleanupStack::PopAndDestroy( &method );
        }

    if(!found)
        {
        VCXLOGLO1( "CIptvIptvTestUtilALR:: method not found!");
        User::Leave(KErrNotFound);
        }

    CleanupStack::PopAndDestroy( &destination );

    VCXLOGLO1("<<<CIptvIptvTestUtilALR::SetMethodStringAttributeL");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::SetMethodIntAttributeL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CIptvTestUtilALR::SetMethodIntAttributeL( TDesC& aDestinationName, TDesC& aMethodName, TInt aAttribute, TInt aAttributeValue )
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::SetMethodIntAttributeL");

    VCXLOGLO2( "CIptvIptvTestUtilALR:: aDestinationName: %S", &aDestinationName );
    VCXLOGLO2( "CIptvIptvTestUtilALR:: aMethodName: %S", &aMethodName );
    VCXLOGLO2( "CIptvIptvTestUtilALR:: aAttribute: %d", aAttribute );
    VCXLOGLO2( "CIptvIptvTestUtilALR:: aAttributeValue: %d", aAttributeValue );

    TInt destId = GetDestinationIdL( aDestinationName );
    RCmDestinationExt destination = iCmManager.DestinationL( destId );
    CleanupClosePushL( destination );

    TBool found( EFalse );

    // Find the method and set attribute
    for(TInt e=0; e < destination.ConnectionMethodCount(); e++)
        {
        RCmConnectionMethodExt method = destination.ConnectionMethodL( e );
        CleanupClosePushL( method );

        HBufC* methodName = method.GetStringAttributeL( CMManager::ECmName );
        CleanupStack::PushL( methodName );
        TInt iapId = method.GetIntAttributeL( CMManager::ECmIapId );
        VCXLOGLO3( "CIptvIptvTestUtilALR:: method: '%S', id: %d", methodName, iapId );

        if( MethodMatchesL( aMethodName, *methodName, EFalse ) )
            {
            VCXLOGLO1( "CIptvIptvTestUtilALR:: Match. Setting attribute." );
            found = ETrue;

            method.SetIntAttributeL( aAttribute, aAttributeValue );
            method.UpdateL();
            }
            
        CleanupStack::PopAndDestroy( methodName );
        CleanupStack::PopAndDestroy( &method );
        }

    if(!found)
        {
        VCXLOGLO1( "CIptvIptvTestUtilALR:: method not found!");
        User::Leave(KErrNotFound);
        }

    CleanupStack::PopAndDestroy( &destination );

    VCXLOGLO1("<<<CIptvIptvTestUtilALR::SetMethodIntAttributeL");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::SetMethodBoolAttributeL
// ---------------------------------------------------------------------------
//
EXPORT_C void CIptvTestUtilALR::SetMethodBoolAttributeL( TDesC& aDestinationName, TDesC& aMethodName, TInt aAttribute, TBool aAttributeValue )
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::SetMethodBoolAttributeL");

    VCXLOGLO2( "CIptvIptvTestUtilALR:: aDestinationName: %S", &aDestinationName );
    VCXLOGLO2( "CIptvIptvTestUtilALR:: aMethodName: %S", &aMethodName );
    VCXLOGLO2( "CIptvIptvTestUtilALR:: aAttribute: %d", aAttribute );
    VCXLOGLO2( "CIptvIptvTestUtilALR:: aAttributeValue: %d", &aAttributeValue );

    TInt destId = GetDestinationIdL( aDestinationName );
    RCmDestinationExt destination = iCmManager.DestinationL( destId );
    CleanupClosePushL( destination );

    TBool found = EFalse;

    // Find the method and set attribute
    for(TInt e=0; e < destination.ConnectionMethodCount(); e++)
        {
        RCmConnectionMethodExt method = destination.ConnectionMethodL( e );
        CleanupClosePushL( method );

        HBufC* methodName = method.GetStringAttributeL( CMManager::ECmName );
        CleanupStack::PushL( methodName );
        TInt iapId = method.GetIntAttributeL( CMManager::ECmIapId );
        VCXLOGLO3( "CIptvIptvTestUtilALR:: method: '%S', id: %d", methodName, iapId );

        if( MethodMatchesL( aMethodName, *methodName, EFalse ) )
            {
            VCXLOGLO1( "CIptvIptvTestUtilALR:: Match. Setting attribute." );
            found = ETrue;

            method.SetBoolAttributeL( aAttribute, aAttributeValue );
            method.UpdateL();
            }
            
        CleanupStack::PopAndDestroy( methodName );
        CleanupStack::PopAndDestroy( &method );
        }

    if(!found)
        {
        VCXLOGLO1( "CIptvIptvTestUtilALR:: method not found!");
        User::Leave(KErrNotFound);
        }

    CleanupStack::PopAndDestroy( &destination );

    VCXLOGLO1("<<<CIptvIptvTestUtilALR::SetMethodBoolAttributeL");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::GetDestinationIdL()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestUtilALR::GetDestinationIdL( TDesC& aDestinationName )
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::GetDestinationIdL");

    VCXLOGLO2("CIptvIptvTestUtilALR:: dest name: '%S'", &aDestinationName);

    RArray<TUint32> destIds(5);

    iCmManager.AllDestinationsL( destIds );
    VCXLOGLO2( "CIptvIptvTestUtilALR:: dest Count: %d", destIds.Count() );

    TInt foundDestId(0);

    for(TInt i=0; i<destIds.Count(); i++ )
        {
        RCmDestinationExt dest = iCmManager.DestinationL( destIds[i] );
        CleanupClosePushL( dest );
        HBufC* name = dest.NameLC();
        VCXLOGLO3( "CIptvIptvTestUtilALR:: dest: '%S', id: %d", name, destIds[i] );
        if( name->Compare( aDestinationName ) == 0 )
            {
            foundDestId = destIds[i];
            VCXLOGLO2( "CIptvIptvTestUtilALR:: found destination id: %d", destIds[i] );
            CleanupStack::PopAndDestroy( name );
            CleanupStack::PopAndDestroy( &dest );
            break;
            }
        else
            {
            CleanupStack::PopAndDestroy( name );
            CleanupStack::PopAndDestroy( &dest );
            }
        }

    if( foundDestId == 0 )
        {
        VCXLOGLO1( "CIptvIptvTestUtilALR:: destination not found!");
        User::Leave( KErrNotFound );
        }

    destIds.Reset();
    destIds.Close();

    VCXLOGLO1("<<<CIptvIptvTestUtilALR::GetDestinationIdL");
    return foundDestId;
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::TerminateDestinationConnectionsL
// ---------------------------------------------------------------------------
//
EXPORT_C void CIptvTestUtilALR::TerminateDestinationConnectionsL( TDesC& aDestinationName )
    {
    VCXLOGLO1(">>>CIptvIptvTestUtilALR::TerminateDestinationConnectionsL");

    VCXLOGLO2( "CIptvIptvTestUtilALR:: aDestinationName: %S", &aDestinationName );

    TInt destId = GetDestinationIdL( aDestinationName );
    RCmDestinationExt destination = iCmManager.DestinationL( destId );
    CleanupClosePushL( destination );

    // Check if there's active connections for the methods
    for(TInt e=0; e < destination.ConnectionMethodCount(); e++)
        {
        RCmConnectionMethodExt method = destination.ConnectionMethodL( e );
        CleanupClosePushL( method );

        HBufC* methodName = method.GetStringAttributeL( CMManager::ECmName );
        CleanupStack::PushL( methodName );
        TInt iapId = method.GetIntAttributeL( CMManager::ECmIapId );
        VCXLOGLO3( "CIptvIptvTestUtilALR:: method: '%S', id: %d", methodName, iapId );

        CTestUtilConnectionWaiter* connectionWaiter = CTestUtilConnectionWaiter::NewL( this );
        CleanupStack::PushL( connectionWaiter );

        // Terminate active connection
        if( connectionWaiter->IsConnectionActive( iapId ) )
            {
            CTestUtilConnection* connectionUtil = CTestUtilConnection::NewL( this );
            CleanupStack::PushL( connectionUtil );
            connectionUtil->SetConnectionPreferences( KCommDbBearerUnknown, iapId );
            connectionUtil->AttachL();

            connectionUtil->TerminateConnectionL();
            connectionWaiter->WaitUntilConnectionIsClosed( iapId );
            iActiveWait->Start();

            CleanupStack::PopAndDestroy( connectionUtil );
            }
        CleanupStack::PopAndDestroy( connectionWaiter );
        CleanupStack::PopAndDestroy( methodName );
        CleanupStack::PopAndDestroy( &method );
        }

    CleanupStack::PopAndDestroy( &destination );

    VCXLOGLO1("<<<CIptvIptvTestUtilALR::TerminateDestinationConnectionsL");
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::GetDefaultIap
// ---------------------------------------------------------------------------
//
EXPORT_C TUint32 CIptvTestUtilALR::GetDefaultIap()
    {
    VCXLOGLO1(">>>CIptvTestUtilALR::GetDefaultIap");
    RSocketServ ss;
    ss.Connect();
    
    RConnection conn;
    conn.Open( ss );

    TCommDbConnPref prefs;
    prefs.SetDialogPreference( ECommDbDialogPrefDoNotPrompt );

    conn.Start( prefs );
    TUint32 iap( 0 );
    conn.GetIntSetting( _L("IAP\\Id"), iap );
    conn.Close();

    VCXLOGLO2("<<<CIptvTestUtilALR::GetDefaultIap return %D", iap);
    return iap;
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::SetDefaultIapCenRep
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CIptvTestUtilALR::SetDefaultIapCenRep()
    {
    TUint32 iap( 0 );
    iap = GetDefaultIap();
    TInt err( 0 );
    
    if( iap != 0 )
        {
        VCXLOGLO1("CIptvTestUtilALR::SetDefaultIapCenRep Create cenrep.");
        CRepository* cenRep = CRepository::NewLC( VCXTEST_KIptvAlrCenRepUid );
        VCXLOGLO1("CIptvTestUtilALR::SetDefaultIapCenRep Set cenrep.");
        User::LeaveIfError( cenRep->Set( VCXTEST_KIptvCenRepDefaultIapIdKey, (TInt)iap ) );
        CleanupStack::PopAndDestroy( cenRep );
        }
    else
        {
        err = KErrNotFound;
        }
    
    return err;   
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::GetDefaultIap
// ---------------------------------------------------------------------------
//
EXPORT_C TUint32 CIptvTestUtilALR::GetDefaultIapCenRep()
    {
    TInt iap( 0 );
    
    VCXLOGLO1("CIptvTestUtilALR::GetDefaultIapCenRep Create cenrep.");
    CRepository* cenRep = CRepository::NewLC( VCXTEST_KIptvAlrCenRepUid );
    VCXLOGLO1("CIptvTestUtilALR::GetDefaultIapCenRep Set cenrep.");
    User::LeaveIfError( cenRep->Get( VCXTEST_KIptvCenRepDefaultIapIdKey, iap) );
    CleanupStack::PopAndDestroy( cenRep );
    
    TUint32 iapId( iap );
    
    return iapId;
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::PrintDestinationL
// ---------------------------------------------------------------------------
//
void CIptvTestUtilALR::PrintDestinationL( RCmDestinationExt& aDestination )
    {
    HBufC* destName = aDestination.NameLC();

    TPtr destNamePtr = destName->Des();
    VCXLOGLO2( "CIptvIptvTestUtilALR:: Destination: %S", &destNamePtr );

    for(TInt e=0; e < aDestination.ConnectionMethodCount(); e++)
        {
        RCmConnectionMethodExt method = aDestination.ConnectionMethodL( e );
        CleanupClosePushL( method );

        HBufC* methodName = method.GetStringAttributeL( CMManager::ECmName );
        CleanupStack::PushL( methodName );

        TUint priority = aDestination.PriorityL( method );
        TInt iapId = method.GetIntAttributeL( CMManager::ECmIapId );
        VCXLOGLO4( "CIptvIptvTestUtilALR:: Method: %S, priority: %d, id: %d", methodName, priority, iapId );

        CleanupStack::PopAndDestroy( methodName );
        CleanupStack::PopAndDestroy( &method );
        }

    CleanupStack::PopAndDestroy( destName );
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::MethodMatchesL
// ---------------------------------------------------------------------------
//
TBool CIptvTestUtilALR::MethodMatchesL( const TDesC& aSearchedName, const TDesC& aCurrentName, TBool aStrict )
    {
#ifdef __WINSCW__
    if( aSearchedName.Compare( _L("Ethernet with Daemon Dynamic IP") ) == 0 )
        {
        return ETrue;
        }
#endif

    if( aSearchedName.Compare( aCurrentName ) == 0 )
        {
        return ETrue;
        }
        
    if( aStrict) 
        {
        return EFalse;
        }

    if( aSearchedName.Compare( _L("Internet") ) == 0 ||  aSearchedName.Compare( _L("Internet2") ) == 0 )
        {
        TBuf<256> temp( aCurrentName );
        temp.LowerCase();

        _LIT(KIapElisaInternet, "elisa internet");
        _LIT(KIapDnaGPRS, "dna gprs");
        _LIT(KIapSoneraGPRS, "sonera gprs");
        _LIT(KIapRLGPRS, "rl gprs");
        _LIT(KIapInternet, "internet");
        _LIT(KIapProinternet, "prointernet");
        _LIT(KGprsInternet, "gprs internet");

        if( temp.Compare( KIapElisaInternet ) == KErrNone ) return ETrue;
        if( temp.Compare( KIapDnaGPRS ) == KErrNone ) return ETrue;
        if( temp.Compare( KIapRLGPRS ) == KErrNone ) return ETrue;
        if( temp.Compare( KIapInternet ) == KErrNone ) return ETrue;
        if( temp.Compare( KIapProinternet ) == KErrNone ) return ETrue;
        if( temp.Compare( KGprsInternet ) == KErrNone ) return ETrue;
        if( temp.Compare( KIapSoneraGPRS ) == KErrNone ) return ETrue;
        }    

    return EFalse;
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::ConnectionCreated
// ---------------------------------------------------------------------------
//
void CIptvTestUtilALR::ConnectionCreated()
    {
    iActiveWait->Stop();
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::ConnectionClosed
// ---------------------------------------------------------------------------
//
void CIptvTestUtilALR::ConnectionClosed()
    {
    iActiveWait->Stop();
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::ConnectionFailed
// ---------------------------------------------------------------------------
//
void CIptvTestUtilALR::ConnectionFailed()
    {
    iActiveWait->Stop();
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::ConnectionAlreadyExists
// ---------------------------------------------------------------------------
//
void CIptvTestUtilALR::ConnectionAlreadyExists()
    {
    iActiveWait->Stop();
    }

// ---------------------------------------------------------------------------
// CIptvTestUtilALR::ConnectionTimeout
// ---------------------------------------------------------------------------
//
void CIptvTestUtilALR::ConnectionTimeout()
    {
    iActiveWait->Stop();
    }

// End of file
