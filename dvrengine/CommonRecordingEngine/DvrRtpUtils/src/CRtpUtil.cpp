/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    Implementation of the Common Recording Engine RTP convertions.*
*/




// INCLUDE FILES
#include <ipvideo/CRtpUtil.h>
#include <mmtsy_names.h>
#include <bsp.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpUtil::NewL()
// 
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpUtil* CRtpUtil::NewL()
    {
    CRtpUtil* self = CRtpUtil::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CRtpUtil::NewLC()
// 
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpUtil* CRtpUtil::NewLC()
    {
    CRtpUtil* self = new ( ELeave ) CRtpUtil;
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CRtpUtil::CRtpUtil
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CRtpUtil::CRtpUtil()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpUtil::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CRtpUtil::ConstructL()
    {
    // None
    }

// -----------------------------------------------------------------------------
// Destructor
//
EXPORT_C CRtpUtil::~CRtpUtil()
// -----------------------------------------------------------------------------
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpUtil::MakeBytesLC
// Returns: Buffer of four bytes where integer is stored
// -----------------------------------------------------------------------------
//
EXPORT_C HBufC8* CRtpUtil::MakeBytesLC( const TInt& aValue )
    {
    HBufC8* bytes = HBufC8::NewLC( KIntegerBytes );
    TPtr8 ptr( bytes->Des() );
    MakeBytesL( aValue, ptr );
    return bytes;
    }

// -----------------------------------------------------------------------------
// CRtpUtil::MakeBytes
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpUtil::MakeBytesL( const TInt& aValue, TDes8& aBuf )
    {
    User::LeaveIfError( MakeBytes( aValue, aBuf ) );
    }

// -----------------------------------------------------------------------------
// CRtpUtil::MakeBytes
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CRtpUtil::MakeBytes( const TInt& aValue, TDes8& aBuf )
    {
    if ( aBuf.MaxLength() >= KIntegerBytes )
        {
        aBuf.SetLength( KIntegerBytes );
        aBuf[0] = ( TUint8 )( aValue >> 24 );
        aBuf[1] = ( TUint8 )( aValue >> 16 );
        aBuf[2] = ( TUint8 )( aValue >> 8 );
        aBuf[3] = ( TUint8 )( aValue );
        return KErrNone;
        }
    
    return KErrUnderflow;
    }

// -----------------------------------------------------------------------------
// CRtpUtil::GetValueL
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CRtpUtil::GetValueL( const TDesC8& aBytes )
    {
    TInt value( KErrNotFound );
    GetValueL( aBytes, value );
    return value;
    }
    
// -----------------------------------------------------------------------------
// CRtpUtil::GetValueL
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpUtil::GetValueL( const TDesC8& aBytes, TInt& aValue )
    {
    User::LeaveIfError( GetValue( aBytes, aValue ) );
    }
    
// -----------------------------------------------------------------------------
// CRtpUtil::GetValue
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CRtpUtil::GetValue( const TDesC8& aBytes, TInt& aValue )
    {
    TUint value( 0 );
    TInt err( GetValue( aBytes, value ) );
    aValue = ( TInt )( value );
    return err;
    }
    
// -----------------------------------------------------------------------------
// CRtpUtil::GetValue
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CRtpUtil::GetValue( const TDesC8& aBytes, TUint& aValue )
    {
    if ( aBytes.Length() >= KIntegerBytes )
        {
        aValue = ( TUint )( aBytes[0] );
        aValue <<= 8;
        aValue |= ( TUint )( aBytes[1] );
        aValue <<= 8;
        aValue |= ( TUint )( aBytes[2] );
        aValue <<= 8;
        aValue |= ( TUint )( aBytes[3] );
        return KErrNone;
        }

    return KErrUnderflow;
    }
    
// -----------------------------------------------------------------------------
// CRtpUtil::SpecialPacketL
// Generates new special packet.
// -----------------------------------------------------------------------------
//
EXPORT_C HBufC8* CRtpUtil::SpecialPacketL( const TInt aType ) 
    {
    // Create buffer
    HBufC8* packet = HBufC8::NewLC( KSpecialPacketLength );
    TPtr8 ptr( packet->Des() );

    // Packet length (PTL)
    HBufC8* bytes = MakeBytesLC( KSpecialPacketLength );
    ptr.Copy( bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );
            
    // Packet type
    ptr.Append( KCharSpace );
    ptr[KPacketTypeBytePoint] = ( TUint8 )( aType );
    
    // Dummy payload
    bytes = MakeBytesLC( KMaxTUint );
    ptr.Append( bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );
    
    CleanupStack::Pop( packet );
    return packet;
    }

// -----------------------------------------------------------------------------
// CRtpUtil::GetMimeInfo
// Mime type info of propriatary RTP clip.
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpUtil::GetMimeInfo( TDes& aMime )
    {
    // Meta header has constant room as user info (IMSI) in old clips
    aMime.Copy( KRtpClipMimetype().Right( KUserIdLength ) );
    }

// -----------------------------------------------------------------------------
// CRtpUtil::GetMimeInfo
// Mime type info of propriatary RTP clip.
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpUtil::GetMimeInfo( TDes8& aMime )
    {
    // Meta header has constant room as user info (IMSI) in old clips
    aMime.Copy( KRtpClipMimetype().Right( KUserIdLength ) );
    }

// -----------------------------------------------------------------------------
// CRtpUtil::GetImeiL
// Read IMEI on phone HW, use dummy under WINS
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpUtil::GetImeiL( TDes& aImei )
    {
    aImei.Zero();  

#if defined( __WINS__ ) || defined( __WINSCW__ )
    _LIT( KEmulatorImei, "147407051877780445" );
    aImei.Copy( KEmulatorImei);
#else // __WINS__ || __WINSCW__

    RTelServer server;
    CleanupClosePushL( server );
    RMobilePhone phone;
    CleanupClosePushL( phone );
    GetMobilePhoneInfoL( server, phone );
    
    // Get IMEI code
    TRequestStatus status;
    RMobilePhone::TMobilePhoneIdentityV1 phoneIdentity;
    phone.GetPhoneId( status, phoneIdentity );
    User::WaitForRequest( status );
    if ( !status.Int() )
        {
        TPtrC imei( phoneIdentity.iSerialNumber );
        for ( TInt i( 0 ); i < imei.Length() && i < aImei.MaxLength(); i++ )
            {
            if ( TChar( imei[i] ).IsDigit() )
                {
                aImei.Append( TChar( imei[i] ) );
                }
            }
        }

    CleanupStack::PopAndDestroy( &phone );
    CleanupStack::PopAndDestroy( &server );

#endif // __WINS__ || __WINSCW__
    }
    
//-----------------------------------------------------------------------------
// CRtpUtil::GetMobilePhoneInfo
//-----------------------------------------------------------------------------
//    
void CRtpUtil::GetMobilePhoneInfoL( 
    RTelServer& aServer,
    RMobilePhone& aPhone )
    {
    TInt numPhone( 0 );
    RTelServer::TPhoneInfo phoneInfo;
    User::LeaveIfError( aServer.Connect() );
    User::LeaveIfError( aServer.LoadPhoneModule( KMmTsyModuleName ) );
    User::LeaveIfError( aServer.EnumeratePhones( numPhone ) );

    TInt found( KErrNotFound );
    TName tsyName( KNullDesC );
    for ( TInt i( 0 ); i < numPhone && found == KErrNotFound; i++ )
        {
        User::LeaveIfError( aServer.GetPhoneInfo( i, phoneInfo ) );
        User::LeaveIfError( aServer.GetTsyName( i, tsyName ) );
        if ( tsyName.CompareF( KMmTsyModuleName ) == 0 )
            {
            found = KErrNone;
            }
        }
        
    User::LeaveIfError( found );
    User::LeaveIfError( aPhone.Open( aServer, phoneInfo.iName ) );
    }
    
// End of File
