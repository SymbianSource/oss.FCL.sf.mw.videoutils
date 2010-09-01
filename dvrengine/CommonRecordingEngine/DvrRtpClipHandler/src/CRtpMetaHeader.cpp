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
* Description:    Implementation of the Common Recording Engine RTP read class.*
*/




// INCLUDE FILES
#include <ipvideo/CRtpMetaHeader.h>
#include <bsp.h>
#include <etelmm.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpMetaHeader::NewL
// Static two-phased constructor. Leaves object to cleanup stack.
// -----------------------------------------------------------------------------
//
CRtpMetaHeader* CRtpMetaHeader::NewL( RFile& aFile, const TMetaMode& aMode )
    {
    CRtpMetaHeader* self = CRtpMetaHeader::NewLC( aFile, aMode );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CRtpMetaHeader::NewLC
// Static two-phased constructor. Leaves object to cleanup stack.
// -----------------------------------------------------------------------------
//
CRtpMetaHeader* CRtpMetaHeader::NewLC( RFile& aFile, const TMetaMode& aMode )
    {
    CRtpMetaHeader* self = new( ELeave ) CRtpMetaHeader( aFile, aMode );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CRtpMetaHeader::CRtpMetaHeader
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CRtpMetaHeader::CRtpMetaHeader( RFile& aFile, const TMetaMode& aMode )
  : iFile( aFile ),
    iMode( aMode ),
    iMetaData( NULL ),
    iDataPtr( 0, 0 ),
    iEsgDataPoint( KErrNotFound ),
    iSrtpDataPoint( KErrNotFound ),
    iSdpDataPoint( KErrNotFound ),
    iMetaTotal( KErrNotFound )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpMetaHeader::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::ConstructL()
    {
    LOG( "CRtpMetaHeader::ConstructL()" );
    
    // Mode
    if ( iMode == EMetaRead || iMode == EMetaUpdate )
        {
        // Read whole meta area
        ReadTintFromFileL( KMetaLengthPoint, iMetaTotal );

        // Meta header has reasonable length?
        if ( iMetaTotal > KMaxMetaHeaderLength || iMetaTotal <= 0 )
            {
            LOG( "CRtpMetaHeader::ConstructL(), Meta Total Corrupted" );
            User::Leave( KErrCorrupt );
            }

        // Room for header
        iMetaData = HBufC8::NewL( iMetaTotal );
        iDataPtr.Set( iMetaData->Des() );
        User::LeaveIfError( iFile.Read( KMetaLengthPoint, iDataPtr, iMetaTotal ) );
        
        // All data exist in meta header?
        if ( iDataPtr.Length() < iMetaTotal )
            {
            LOG( "CRtpMetaHeader::ConstructL(), Meta Header Corrupted" );
            User::Leave( KErrCorrupt );
            }

        // ESG data point ( device info point + device info data )
        iEsgDataPoint = KDeviceInfoPoint + KStringLengthBytes +
                        iDataPtr[KDeviceInfoPoint];
        
        // SRTP data point ( ESG data point + Service name + Program name )
        TInt snp( iEsgDataPoint + KStringLengthBytes + iDataPtr[iEsgDataPoint] );
        iSrtpDataPoint = snp + KStringLengthBytes + iDataPtr[snp];

        // SDD file point ( SRTP data point + SRTP data )
        const TInt srtplen( CRtpUtil::GetValueL( 
                            iDataPtr.Mid( iSrtpDataPoint, KIntegerBytes ) ) );
        User::LeaveIfError( srtplen );
        iSdpDataPoint = iSrtpDataPoint + KIntegerBytes + srtplen;
        }
    else // Write
        {
        iMetaData = HBufC8::NewL( 0 );
        // Room for meta length
        iMetaTotal = KMetaLengthPoint;
        AddIntegerL( KMetaLengthPoint, KErrNotFound );
        }
    }

// -----------------------------------------------------------------------------
// Destructor
//
CRtpMetaHeader::~CRtpMetaHeader()
// -----------------------------------------------------------------------------
    {
    LOG( "CRtpMetaHeader::~CRtpMetaHeader()" );
    
    delete iMetaData;
    }

// -----------------------------------------------------------------------------
// CRtpMetaHeader::SeekHeaderPoint
// Getter for seek header point.
// Returns: Point to seek header
// -----------------------------------------------------------------------------
//
TInt CRtpMetaHeader::SeekHeaderPoint()
    {
    return iMetaTotal;
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::CommitL
// Writes meta header to the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::CommitL()
    {
    LOG( "CRtpMetaHeader::CommitL() in" );

    User::LeaveIfError( iMetaTotal );
    User::LeaveIfError( ( iMetaTotal > KMaxMetaHeaderLength ) * KErrCorrupt );
    
    if ( iMode == EMetaWrite )
        {
        User::LeaveIfError( iEsgDataPoint );
        User::LeaveIfError( iSrtpDataPoint );
        User::LeaveIfError( iSdpDataPoint );
        
        // Meta length
        iDataPtr.Delete( KMetaLengthPoint, KIntegerBytes );
        AddIntegerL( KMetaLengthPoint, iMetaTotal );
        
        User::LeaveIfError( iFile.Write( iDataPtr, iMetaTotal ) );
        }

    LOG( "CRtpMetaHeader::CommitL() out" );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::WriteAttributesL
// Writes atributes to meta data header of the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::WriteAttributesL( const SAttributes& aAtt )
    {
    TInt data( 0 );
    data|= aAtt.iOngoing << KOngoingFlagShift;
    data|= aAtt.iCompleted << KCompletedFlagShift;
    data|= aAtt.iProtected << KProtectedFlagShift;
    data|= aAtt.iFailed << KFailedFlagShift;
    data|= aAtt.iVersion << KVersionFieldShift;
    data|= aAtt.iQuality << KQualityFieldShift;
    data|= aAtt.iPostRule << KPostRuleFieldShift;
    data|= aAtt.iParental << KParentalFieldShift;

    AddIntegerL( KAttributesPoint, data );
    AddIntegerL( KPlayCountPoint, aAtt.iPlayCount );
    AddIntegerL( KPlaySpotPoint, aAtt.iPlaySpot );
    
    // Reserved room for 4 integers
    AddIntegerL( KReservedPoint1, 0 );
    AddIntegerL( KReservedPoint2, 0 );
    AddIntegerL( KReservedPoint3, 0 );
    AddIntegerL( KReservedPoint4, 0 );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::WriteStartTimeL
// Writes start date/time info to meta header of the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::WriteStartTimeL( const TTime& aTime )
    {
    AddTimeL( KStartTimePoint, aTime );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::WriteEndTimeL
// Writes end date/time info to meta header of the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::WriteEndTimeL( const TTime& aTime )
    {
    AddTimeL( KEndTimePoint, aTime );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::WriteDurationL
// Writes duration of clip to meta header of the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::WriteDurationL( const TInt aDuration )
    {
    AddIntegerL( KDurationPoint, aDuration );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::WriteSeekArrayPointL
// Writes seek array point of clip to meta header of the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::WriteSeekArrayPointL( const TInt aPoint )
    {
    AddIntegerL( KSeekArrayPoint, aPoint );
    }

// -----------------------------------------------------------------------------
// CRtpMetaHeader::WriteUserIdL
// Writes user id to meta header of the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::WriteUserIdL( const TDesC& aId )
    {
    WriteStringDataL( KUserIdPoint, aId );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::WriteDeviceInfoL
// Writes device info to meta header of the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::WriteDeviceInfoL( const TDesC& aInfo )
    {
    WriteStringDataL( KDeviceInfoPoint, aInfo );
    iEsgDataPoint = KDeviceInfoPoint + KStringLengthBytes + aInfo.Length();
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::WriteEsgDataL
// Writes ESG data to meta header of the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::WriteEsgDataL( const TDesC& aService, const TDesC& aProgram )
    {
    // Device info must exist first
    User::LeaveIfError( iEsgDataPoint );

    // Service name
    WriteStringDataL( iEsgDataPoint, aService );
    
    // Program name
    const TInt prog( iEsgDataPoint + KStringLengthBytes + aService.Length() );
    WriteStringDataL( prog, aProgram );
    iSrtpDataPoint = prog + KStringLengthBytes + aProgram.Length();
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::WriteSrtpDataL
// Writes SRTP data to meta header of the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::WriteSrtpDataL( const TDesC8& aSrtpData )
    {
    // ESG data must exist first
    User::LeaveIfError( iSrtpDataPoint );
    User::LeaveIfError( ( iMode!=EMetaWrite ) * KErrAccessDenied );

    AddIntegerL( iSrtpDataPoint, aSrtpData.Length() );
    AddDataL( iSrtpDataPoint + KIntegerBytes, aSrtpData );
    iSdpDataPoint = iSrtpDataPoint + KIntegerBytes + aSrtpData.Length();
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::WriteSdpDataL
// Writes SDP file data to meta header of the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::WriteSdpDataL( const TDesC8& aSdpData )
    {
    // SRTP data must exist first
    User::LeaveIfError( iSdpDataPoint );
    User::LeaveIfError( ( iMode != EMetaWrite ) * KErrAccessDenied );

    if ( aSdpData.Length() )
        {
        AddIntegerL( iSdpDataPoint, aSdpData.Length() );
        AddDataL( iSdpDataPoint + KIntegerBytes, aSdpData );
        }
    else
        {
        AddIntegerL( iSdpDataPoint, 0 );
        }
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::ReadAttributesL
// Reads attributes of meta data header from clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::ReadAttributesL( SAttributes& aAtt )
    {
    aAtt.iVersion = 0;

    // Attributes
    HBufC8* bytes = iDataPtr.Mid( KAttributesPoint, KIntegerBytes ).AllocLC();
    TUint data( CRtpUtil::GetValueL( bytes->Des() ) );
    CleanupStack::PopAndDestroy( bytes );
    aAtt.iOngoing = ( data >> KOngoingFlagShift ) & ETrue;
    aAtt.iCompleted = ( data >> KCompletedFlagShift ) & ETrue;
    aAtt.iProtected = ( data >> KProtectedFlagShift ) & ETrue;
    aAtt.iFailed = ( data >> KFailedFlagShift ) & ETrue;
    aAtt.iVersion = ( TUint8 )( ( data >> KVersionFieldShift ) & 0xF );
    aAtt.iQuality = ( TUint8 )( ( data >> KQualityFieldShift ) & KMaxTUint8 );
    aAtt.iPostRule = ( TUint8 )( ( data >> KPostRuleFieldShift ) & KMaxTUint8 );
    aAtt.iParental = ( TUint8 )( ( data >> KParentalFieldShift ) & KMaxTUint8 );

    // Play count
    bytes = iDataPtr.Mid( KPlayCountPoint, KIntegerBytes ).AllocLC();
    aAtt.iPlayCount = CRtpUtil::GetValueL( bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );

    // Play start spot
    bytes = iDataPtr.Mid( KPlaySpotPoint, KIntegerBytes ).AllocLC();
    aAtt.iPlaySpot = CRtpUtil::GetValueL( bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );

    // Reserved room for 4 integers
    /*
    bytes = iDataPtr.Mid( KReservedPoint1, KIntegerBytes ).AllocLC();
    aAtt.iReservedX = CRtpUtil::GetValueL( bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );

    bytes = iDataPtr.Mid( KReservedPoint2, KIntegerBytes ).AllocLC();
    aAtt.iReservedX = CRtpUtil::GetValueL( bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );

    bytes = iDataPtr.Mid( KReservedPoint3, KIntegerBytes ).AllocLC();
    aAtt.iReservedX = CRtpUtil::GetValueL( bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );

    bytes = iDataPtr.Mid( KReservedPoint4, KIntegerBytes ).AllocLC();
    aAtt.iReservedX = CRtpUtil::GetValueL( bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );
    */
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::ReadStartTimeL
// Reads date/time of meta header from the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::ReadStartTimeL( TTime& aTime )
    {
    GetTimeL( KStartTimePoint, aTime );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::ReadEndTimeL
// Reads date/time of meta header from the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::ReadEndTimeL( TTime& aTime )
    {
    GetTimeL( KEndTimePoint, aTime );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::ReadDurationL
// Reads clip duration of meta header from the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::ReadDurationL( TInt& aDuration )
    {
    HBufC8* bytes = iDataPtr.Mid( KDurationPoint, KIntegerBytes ).AllocLC();
    aDuration = CRtpUtil::GetValueL( bytes->Des() );
    User::LeaveIfError( aDuration );
    CleanupStack::PopAndDestroy( bytes );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::ReadSeekArrayPointL
// Reads seek array point of meta header from the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::ReadSeekArrayPointL( TInt& aPoint )
    {
    HBufC8* bytes = iDataPtr.Mid( KSeekArrayPoint, KIntegerBytes ).AllocLC();
    aPoint = CRtpUtil::GetValueL( bytes->Des() );
    User::LeaveIfError( aPoint );
    CleanupStack::PopAndDestroy( bytes );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::ReadUserIdL
// Reads user id of meta header from the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::ReadUserIdL( TDes& aId )
    {
    ReadStringDataL( KUserIdPoint, aId );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::ReadDeviceInfoL
// Reads device info of meta header from the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::ReadDeviceInfoL( TDes& aInfo )
    {
    ReadStringDataL( KDeviceInfoPoint, aInfo );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::ReadEsgDataL
// Reads ESG data of meta header from the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::ReadEsgDataL( TDes& aService, TDes& aProgram )
    {
    User::LeaveIfError( iEsgDataPoint );

    // Service name
    ReadStringDataL( iEsgDataPoint, aService );
    
    // Program name
    const TInt prog( iEsgDataPoint + KStringLengthBytes + aService.Length() );
    ReadStringDataL( prog, aProgram );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::ReadSrtpDataL
// Reads SRTP data of meta header from the clip.
// -----------------------------------------------------------------------------
//
HBufC8* CRtpMetaHeader::ReadSrtpDataL()
    {
    User::LeaveIfError( iSrtpDataPoint );

    // Length
    const TInt len( CRtpUtil::GetValueL( 
                    iDataPtr.Mid( iSrtpDataPoint, KIntegerBytes ) ) );
    // Data
    const TInt total( iSrtpDataPoint + KIntegerBytes + len );
    User::LeaveIfError( ( len < 0 || total > iDataPtr.MaxLength() ) * KErrCorrupt );
    HBufC8* buf = iDataPtr.Mid( iSrtpDataPoint + KIntegerBytes, len ).AllocL();
    return buf;
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::ReadSdpDataL
// Reads SDP file data of meta header from the clip.
// -----------------------------------------------------------------------------
//
HBufC8* CRtpMetaHeader::ReadSdpDataL()
    {
    User::LeaveIfError( iSdpDataPoint );

    // Length
    const TInt len( CRtpUtil::GetValueL( 
                    iDataPtr.Mid( iSdpDataPoint, KIntegerBytes ) ) );
    // Data
    const TInt total( iSdpDataPoint + KIntegerBytes + len );
    User::LeaveIfError( ( len <= 0 || total > iDataPtr.MaxLength() ) * KErrCorrupt );
    HBufC8* buf = iDataPtr.Mid( iSdpDataPoint + KIntegerBytes, len ).AllocL();
    return buf;
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::WriteStringDataL
// Writes data with length info to meta header of the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::WriteStringDataL( const TInt aPosition, const TDesC& aData )
    {
    const TInt len( aData.Length() );
    User::LeaveIfError( ( len > TInt( KMaxTUint8 ) ) * KErrArgument );
    User::LeaveIfError( ( iMode != EMetaWrite ) * KErrAccessDenied );
    
    // Length
    TBuf8<KStringLengthBytes> buf( KNullDesC8 );
    buf.Append( KCharSpace );
    buf[0] = ( TUint8 )( len );
    AddDataL( aPosition, buf );

    // Data to 8-bit
    HBufC8* data = HBufC8::NewLC( aData.Length() );
    TPtr8 ptr( data->Des() );
    ptr.Copy( aData );
    AddDataL( aPosition + KStringLengthBytes, ptr );
    CleanupStack::PopAndDestroy( data );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::ReadStringDataL
// Reads data with length info of meta header from the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::ReadStringDataL( const TInt aPosition, TDes& aData )
    {
    User::LeaveIfError( ( aPosition < 0 || aPosition > iDataPtr.Length() )
                        * KErrArgument );
    const TInt len( iDataPtr[aPosition] );
    User::LeaveIfError( ( len < 0 || len > TInt( KMaxTUint8 ) ) * KErrCorrupt );
    User::LeaveIfError( ( len > aData.MaxLength() ) * KErrArgument );

    aData.Copy( iDataPtr.Mid( aPosition + KStringLengthBytes, len ) );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::AddTimeL
// Writes data/time or duration of clip to meta header of the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::AddTimeL( const TInt aPosition, const TTime& aTime )
    {
    HBufC8* bytes = CRtpUtil::MakeBytesLC( I64LOW( aTime.Int64() ) );
    AddDataL( aPosition, bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );

    bytes = CRtpUtil::MakeBytesLC( I64HIGH( aTime.Int64() ) );
    AddDataL( aPosition + KIntegerBytes, bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::GetTimeL
// Reads time value of meta header from the clip.
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::GetTimeL( const TInt aPosition, TTime& aTime )
    {
    TUint low( CRtpUtil::GetValueL(
        iDataPtr.Mid( aPosition, KIntegerBytes ) ) );
    TUint high( CRtpUtil::GetValueL( 
        iDataPtr.Mid( aPosition +  KIntegerBytes, KIntegerBytes ) ) );

    aTime = TInt64( MAKE_TINT64( high, low ) );
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::AddIntegerL
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::AddIntegerL( const TInt aPosition, const TInt aValue )
    {
    HBufC8* bytes = CRtpUtil::MakeBytesLC( aValue );
    AddDataL( aPosition, bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );
    }

// -----------------------------------------------------------------------------
// CRtpMetaHeader::AddDataL
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::AddDataL( const TInt aPosition, const TDesC8& aData )
    {
    // Write must be in certain order
    User::LeaveIfError( ( aPosition > iMetaTotal ) * KErrWrite );

    switch ( iMode )
        {
        case EMetaWrite:
            iMetaTotal = iMetaData->Length() + aData.Length();
            iMetaData = iMetaData->ReAllocL( iMetaTotal );
            iDataPtr.Set( iMetaData->Des() );
            iDataPtr.Insert( aPosition, aData );
            break;

        case EMetaUpdate:
            User::LeaveIfError( iFile.Write( aPosition, aData, aData.Length() ) );
            break;

        default:
            User::Leave( KErrNotSupported );
            break;
        }
    }
 
// -----------------------------------------------------------------------------
// CRtpMetaHeader::ReadTintFromFileL
// -----------------------------------------------------------------------------
//
void CRtpMetaHeader::ReadTintFromFileL( const TInt& aPosition, TInt& aValue )
    {
    HBufC8* bytes = HBufC8::NewLC( KIntegerBytes );
    TPtr8 ptr( bytes->Des() );
    User::LeaveIfError( iFile.Read( aPosition, ptr, KIntegerBytes ) );
    
    aValue = CRtpUtil::GetValueL( ptr );
    CleanupStack::PopAndDestroy( bytes );
    }
    
//  End of File
