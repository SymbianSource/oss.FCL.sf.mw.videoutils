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
* Description:    Implementation of RTP file recognizer class.*
*/




// INCLUDE FILES
#include "CRtpClipRecognizer.h"
#include <ecom/ImplementationProxy.h>
#include <mmtsy_names.h>

// CONSTANTS
const TInt KIntegerBytes( 4 );
const TInt KStringLengthBytes( 1 );
const TInt KMaxMetaHeaderLength( 2048 );
const TInt KMetaLengthPoint( 0 );
const TInt KVersionFieldShift( 4 );
const TInt KPostRuleFieldShift( 16 );
const TInt KAttributesPoint( KMetaLengthPoint + KIntegerBytes ); // 4
const TInt KPlayCountPoint( KAttributesPoint + KIntegerBytes );	 // 8
const TInt KPlaySpotPoint( KPlayCountPoint + KIntegerBytes );	 // 12
const TInt KReservedPoint1( KPlaySpotPoint + KIntegerBytes );	 // 16
const TInt KReservedPoint2( KReservedPoint1 + KIntegerBytes );	 // 20
const TInt KReservedPoint3( KReservedPoint2 + KIntegerBytes );	 // 24
const TInt KReservedPoint4( KReservedPoint3 + KIntegerBytes );	 // 28
const TInt KStartTimePoint( KReservedPoint4 + KIntegerBytes );	 // 32
const TInt KEndTimePoint( KStartTimePoint + 2 * KIntegerBytes ); // 40
const TInt KDurationPoint( KEndTimePoint + 2 * KIntegerBytes );	 // 48
const TInt KSeekArrayPoint( KDurationPoint + KIntegerBytes );	 // 52
const TInt KUserIdPoint( KSeekArrayPoint + KIntegerBytes );		 // 56
const TInt KDeviceInfoPoint( KUserIdPoint + KStringLengthBytes + // 72
                             KUserIdLength );

const TInt KMaxRtpPostRule( 3 );
const TInt KMinRtpVersion( 2 );
const TInt KMaxRtpVersion( 8 );
const TInt KMaxDuration( 24 * 60 * 60 * 1000 );
const TInt KMaxDurationError( 60 * 1000 );
const TInt EContentRightsLockToDevice( 2 );
const TInt KSupportedMimeTypes( 1 );
const TInt KRtpClipRecogImplUIDValue( 0x10208446 );
const TUid KUidMimeRtpClipRecognizer = { 0x10208445 };
const TUint KNeededMetaBytes( KDeviceInfoPoint + KDeviceIdLength );
// Result of: TInt64( 1000 * 60 * 60 * 24 ) * TInt64( 365 * 2000 )
const TInt64 KSecondInMillenium( 63072000000000 );
_LIT8( KRtpClipMimetype, "application/x-nokia-teh-rtp" );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpClipRecognizer::CreateRecognizerL
// Static method to create instance of CRtpClipRecognizer
// -----------------------------------------------------------------------------
//
CApaDataRecognizerType* CRtpClipRecognizer::CreateRecognizerL()
    {
    CApaDataRecognizerType* rtpRecogType = NULL;
    rtpRecogType = new( ELeave ) CRtpClipRecognizer();
    return rtpRecogType; // NULL if new failed
    }
    
// -----------------------------------------------------------------------------
// CRtpClipRecognizer::CRtpClipRecognizer
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CRtpClipRecognizer::CRtpClipRecognizer()
  : CApaDataRecognizerType( KUidMimeRtpClipRecognizer,
                            CApaDataRecognizerType::EHigh )
    {
    iImei.Zero();
    iCountDataTypes = KSupportedMimeTypes;
    }
    
// -----------------------------------------------------------------------------
// CRtpClipRecognizer::PreferredBufSize
// Overwritten method from CApaDataRecognizerType
// -----------------------------------------------------------------------------
//
TUint CRtpClipRecognizer::PreferredBufSize()
    {
    return KNeededMetaBytes;
    }

// -----------------------------------------------------------------------------
// CRtpClipRecognizer::SupportedDataTypeL
// Overwritten method from CApaDataRecognizerType
// -----------------------------------------------------------------------------
//
TDataType CRtpClipRecognizer::SupportedDataTypeL( TInt aIndex ) const
    {
    __ASSERT_DEBUG( aIndex >= 0 &&
                    aIndex < KSupportedMimeTypes, User::Invariant() );
    
    switch ( aIndex )
        {
        case 0:
            {
            TDataType type( KRtpClipMimetype );
            return type;
            }
        
        default:
            break;
        }
    
    return TDataType();
    }

// -----------------------------------------------------------------------------
// CRtpClipRecognizer::DoRecognizeL
// Overwritten method from CApaDataRecognizerType
// -----------------------------------------------------------------------------
//
void CRtpClipRecognizer::DoRecognizeL(
    const TDesC& /*aName*/,
    const TDesC8& aBuffer )
    {
    iConfidence = ENotRecognized;
    iDataType = TDataType();

    // Verify invest buffer length
    if ( aBuffer.Length() < KNeededMetaBytes )
        {
        return; // Nothing to recognize 
        }

    // Meta length
    const TInt metaLength( GetValueL( 
        aBuffer.Mid( KMetaLengthPoint, KIntegerBytes ) ) );
    if ( metaLength > KErrNotFound && metaLength < KMaxMetaHeaderLength )
        {
        // Attributes
        TUint attr( GetValueL( 
            aBuffer.Mid( KAttributesPoint, KIntegerBytes ) ) );
        TUint8 ver( ( TUint8 )( ( attr >> KVersionFieldShift ) & 0xF ) );
        TUint8 post( ( TUint8 )( ( attr >> KPostRuleFieldShift ) & KMaxTUint8 ) );
        
        // Atributes valid?
        if ( post <= KMaxRtpPostRule && 
             ver >= KMinRtpVersion && ver <= KMaxRtpVersion )
            {
            // Verify mime
            TBool certain( EFalse );
            TBuf8<KUserIdLength> info( KNullDesC8 );
            GetMimeInfo( info );
            const TInt len( aBuffer[KUserIdPoint] );
            if ( len == KUserIdLength )
                {
                TPtrC8 mime( aBuffer.Mid( KUserIdPoint + KStringLengthBytes, len ) );
                if ( !mime.Compare( info ) )
                    {
                    certain = ETrue;
                    }
                else // old clip without mime in meta header
                    {
                    // Star time, end time, duration
                    TInt64 start( GetTInt64L( 
                        aBuffer.Mid( KStartTimePoint, KIntegerBytes * 2 ) ) );
                    TInt64 end( GetTInt64L(
                        aBuffer.Mid( KEndTimePoint, KIntegerBytes * 2 ) ) );
                    TUint dur( GetValueL( 
                        aBuffer.Mid( KDurationPoint, KIntegerBytes ) ) );
                    const TInt delta( TInt( ( end - start ) / 1000 ) );
                    
                    if ( start > KSecondInMillenium &&
                         end > KSecondInMillenium && 
                         delta > 0 && dur < KMaxDuration && 
                         delta > ( ( dur > KMaxDurationError )? 
                                     dur - KMaxDurationError: 0 ) )
                        {
                        certain = ETrue;
                        }
                    }
                }
            
            // Verify content rights
            if ( certain && post == EContentRightsLockToDevice )
                {
                // IMSI from the phone
                if ( !iImei.Length() )
                    {
                    TBuf<KDeviceIdLength> buf;
                    GetImeiL( buf );
                    iImei.Copy( buf );
                    }
                
                // Verify IMEI
                const TInt len( aBuffer[KDeviceInfoPoint] );
                TPtrC8 imei( NULL, 0 );
                if ( len == KDeviceIdLength )
                    {
                    imei.Set( aBuffer.Mid( KDeviceInfoPoint + 
                                           KStringLengthBytes, len ) );
                    }
                if ( len != KDeviceIdLength || imei.Compare( iImei ) )
                    {
                    certain = EFalse;
                    }
                }

            // Set confidence and mime
            if ( certain )
                {
                iConfidence = ECertain;
                iDataType = TDataType( KRtpClipMimetype );
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CRtpClipRecognizer::GetValueL
// -----------------------------------------------------------------------------
//
TInt CRtpClipRecognizer::GetValueL( const TDesC8& aBytes )
    {
    TInt value( KErrNotFound );
    GetValueL( aBytes, value );
    return value;
    }
    
// -----------------------------------------------------------------------------
// CRtpClipRecognizer::GetValueL
// -----------------------------------------------------------------------------
//
void CRtpClipRecognizer::GetValueL( const TDesC8& aBytes, TInt& aValue )
    {
    User::LeaveIfError( GetValue( aBytes, aValue ) );
    }
    
// -----------------------------------------------------------------------------
// CRtpClipRecognizer::GetValue
// -----------------------------------------------------------------------------
//
TInt CRtpClipRecognizer::GetValue( const TDesC8& aBytes, TInt& aValue )
    {
    TUint value( 0 );
    TInt err( GetValue( aBytes, value ) );
    aValue = ( TInt )( value );
    return err;
    }
    
// -----------------------------------------------------------------------------
// CRtpClipRecognizer::GetValue
// -----------------------------------------------------------------------------
//
TInt CRtpClipRecognizer::GetValue( const TDesC8& aBytes, TUint& aValue )
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
// CRtpClipRecognizer::GetTInt64L
// Reads 64 bits integer from descriptor.
// -----------------------------------------------------------------------------
//
TInt64 CRtpClipRecognizer::GetTInt64L( const TDesC8& aBytes )
    {
    TUint low( GetValueL( aBytes.Mid( 0, KIntegerBytes ) ) );
    TUint high( GetValueL( aBytes.Mid( KIntegerBytes, 
                                                 KIntegerBytes ) ) );
    return TInt64( MAKE_TINT64( high, low ) );
    }
 
// -----------------------------------------------------------------------------
// CRtpClipRecognizer::GetMimeInfo
// Mime type info of propriatary RTP clip.
// -----------------------------------------------------------------------------
//
void CRtpClipRecognizer::GetMimeInfo( TDes8& aMime )
    {
    // Meta header has constant room as user info (IMSI) in old clips
    aMime.Copy( KRtpClipMimetype().Right( KUserIdLength ) );
    }

// -----------------------------------------------------------------------------
// CRtpClipRecognizer::GetImeiL
// Read IMEI on phone HW, use dummy under WINS
// -----------------------------------------------------------------------------
//
void CRtpClipRecognizer::GetImeiL( TDes& aImei )
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
// CRtpClipRecognizer::GetMobilePhoneInfo
//-----------------------------------------------------------------------------
//    
void CRtpClipRecognizer::GetMobilePhoneInfoL( 
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
    
// -----------------------------------------------------------------------------
// CRtpClipRecognizer::ImplementationTable
// Table containing the data concerning CRtpClipRecognizer 
// -----------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY( KRtpClipRecogImplUIDValue,
                                CRtpClipRecognizer::CreateRecognizerL )
    };

// -----------------------------------------------------------------------------
// CRtpClipRecognizer::ImplementationGroupProxy
// Function called by framework to return data about this recognizer
// -----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof( ImplementationTable ) / 
                  sizeof( TImplementationProxy );
    
    return ImplementationTable;
    }

//  End of File
