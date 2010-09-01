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


#ifndef IptvTestUtilALR_H
#define IptvTestUtilALR_H

#include <e32base.h>
#include "MTestUtilConnectionObserver.h"

#include <cmmanagerext.h>
#include <cmdestinationext.h>
#include <cmconnectionmethodext.h>
#include <cmconnectionmethoddef.h>
#include <commdbconnpref.h>

// FORWARD DECLARATIONS;
class CIptvTestActiveWait;

// CONSTANTS

// Declared in CIptvUtil.h and vcxconnutilimpl.cpp
const TUid VCXTEST_KIptvAlrCenRepUid = { 0x2000B438 };
const TInt VCXTEST_KIptvCenRepUsedSnapIdKey   = 0x1;
const TInt VCXTEST_KIptvCenRepDefaultIapIdKey   = 0x2;
const TInt VCXTEST_KIptvCenRepAPModeAlwaysAsk = 0;

// CLASS DECLARATION

/**
* CIptvTestUtilALR  Controls the internet connection initiation and shutdown
*/
class CIptvTestUtilALR : public CBase, public MTestUtilConnectionObserver
    {
public:

    /*
     * NewL()
     */
    IMPORT_C static CIptvTestUtilALR* NewL();

    /*
     * NewL()
     */
    IMPORT_C static CIptvTestUtilALR* NewLC();

    /*
     * ~CIptvTestUtilALR()
     */
    IMPORT_C virtual ~CIptvTestUtilALR();

public: // Functions from base classes

    void ConnectionCreated();
    void ConnectionClosed();
    void ConnectionFailed();
    void ConnectionAlreadyExists();
    void ConnectionTimeout();

public: // New functions

    /**
    * Delete the used destination cenrep value.
    */
    IMPORT_C void RemoveUsedDestinationCenRepL();

    /**
    * Set the used destination cenrep value for Video Center & IPTV_Engine.
    */
    IMPORT_C void SetUsedDestinationL( TDesC& aDestinationName );

    /**
    * Create new destination (SNAP)
    */
    IMPORT_C void CreateDestinationL( TDesC& aDestinationName );

    /**
    * Delete a destination (SNAP)
    */
    IMPORT_C void DeleteDestinationL( TDesC& aDestinationName );

    /**
    * Copy existing connection method to defined SNAP.
    */
    IMPORT_C void CopyMethodL( TDesC& aMethodName, TDesC& aNewMethodName, TDesC& aNewDestinationName );

    /**
    * Delete a connection method.
    */
    IMPORT_C void DeleteMethodL( TDesC& aDestinationName, TDesC& aMethodName );

    /**
    * Set priority for a connection method.
    */
    IMPORT_C void SetMethodPriorityL( TDesC& aDestinationName, TDesC& aMethodName, TUint32 aPriority );

    /**
     * Sets specified string attribute for a method.
     */
    IMPORT_C void SetMethodStringAttributeL( TDesC& aDestinationName, TDesC& aMethodName, TInt aAttribute, TDesC& aAttributeValue );

    /**
     * Sets specified integer attribute for a method.
     */
    IMPORT_C void SetMethodIntAttributeL( TDesC& aDestinationName, TDesC& aMethodName, TInt aAttribute, TInt aAttributeValue );

    /**
     * Sets specified boolean attribute for a method.
     */
    IMPORT_C void SetMethodBoolAttributeL( TDesC& aDestinationName, TDesC& aMethodName, TInt aAttribute, TBool aAttributeValue );

    /**
    * Returns id of given destination.
    */
    IMPORT_C TInt GetDestinationIdL( TDesC& aDestinationName );

    /**
    * Terminates active connections for the destination's IAPs.
    */
    IMPORT_C void TerminateDestinationConnectionsL( TDesC& aDestinationName );
    
    /**
     * Gets the default access point from connection manager
     */
    IMPORT_C TUint32 GetDefaultIap( void );
    
    /**
     * Gets the default iap, saves it to cenrep
     */
    IMPORT_C TInt SetDefaultIapCenRep( void );
    
    /**
     * Gets the default iap id from cenrep
     */
    IMPORT_C TUint32 GetDefaultIapCenRep( void );

    
private: // New functions

    /**
     * Prints destination's IAPs and their priorities to debug output.
     */
    void PrintDestinationL( RCmDestinationExt& aDestination );

    /**
     * Compares two connection method names. 
     * @param aSearchedName name for the method to search for. 
     * @param aCurrentName name to compare against.
     * @param aStrict If false then and aSearchedName is "Internet" there's few alternatives for 
     *        aCurrentName which will match too.
     * @return ETrue if match, otherwise EFalse.
     */
    TBool MethodMatchesL( const TDesC& aSearchedName, const TDesC& aCurrentName, TBool aStrict );

private:

    /*
     * CIptvTestUtilALR()
     */
    CIptvTestUtilALR();

    /*
     * ConstructL()
     */
    void ConstructL();

private: // Data

    RCmManagerExt iCmManager;

    // CIptvTestActiveWait
    CIptvTestActiveWait* iActiveWait;

    };


#endif // IAPCONNECT_V20ENGINE_H
