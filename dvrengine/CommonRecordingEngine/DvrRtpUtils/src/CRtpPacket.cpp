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
* Description:    Class to RTP packet parsing.*
*/




// INCLUDE FILES
#include "CRtpPacket.h"

// CONSTANTS
const TInt KRtpPacketVersion( 2 );
const TInt KRtpMinHeaderLength( 12 );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpPacket::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpPacket* CRtpPacket::NewL()
    {
    CRtpPacket* self = CRtpPacket::NewLC();
    CleanupStack::Pop();
    return self;
    }
    
// -----------------------------------------------------------------------------
// CRtpPacket::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpPacket* CRtpPacket::NewLC()
    {
    CRtpPacket* self = new( ELeave ) CRtpPacket();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CRtpPacket::CRtpPacket
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CRtpPacket::CRtpPacket() : iPayload( NULL, 0 )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpPacket::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CRtpPacket::ConstructL()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpPacket::CRtpPacket
// Destructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpPacket::~CRtpPacket()
    {
    if ( iRtpRecvHeader.iHeaderExtension )
        {
        // Remove iData
        delete[] ( iRtpRecvHeader.iHeaderExtension )->iData;
        delete iRtpRecvHeader.iHeaderExtension;
        }
    
    if ( iRtpRecvHeader.iCsrcList )
        {
        delete[] iRtpRecvHeader.iCsrcList;
        }
    }

// -----------------------------------------------------------------------------
// CRtpPacket::ParseRtcp
// 
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CRtpPacket::ParseRtp( const TDesC8& aPktBuf )
    {
    if ( aPktBuf.Length() < KRtpMinHeaderLength )
        {
        SetTimeStamp( 0 );
        iPayload.Set( NULL, 0 );
        return KErrUnderflow;
        }
    
    // 1st byte
    TInt byte( 0 );
    TUint8 version_flag( ( aPktBuf[byte] & 0xc0 ) >> 6 );
    // v=2 is mandatory 
    if ( version_flag != KRtpPacketVersion ) 
        {
        SetTimeStamp( 0 );
        iPayload.Set( NULL, 0 );
        return KErrNotSupported;
        } 
        
    iRtpRecvHeader.iPadding = ( aPktBuf[byte] >> 5 ) & 1;
    iRtpRecvHeader.iExtension = ( aPktBuf[byte] >> 4 ) & 1;
    iRtpRecvHeader.iCsrcCount = aPktBuf[byte++] & 0xf;
    
    // 2nd byte
    iRtpRecvHeader.iMarker = ( aPktBuf[byte] >> 7 ) & 1;
    iRtpRecvHeader.iPayloadType = aPktBuf[byte++] & 0x7f;
    
    // 3rd - 4th bytes
    TUint16 seq_no( aPktBuf[byte++] << 8 );
    seq_no |= aPktBuf[byte++];
    iRtpRecvHeader.iSeqNum = seq_no;
    
    // 5th - 8th bytes
    TUint32 timestamp( Read32Bits( aPktBuf, byte ) );
    iRtpRecvHeader.iTimestamp = timestamp;

    // 9th - 12th bytes
    TInt32 ssrc( Read32Bits( aPktBuf, byte ) );
    
    // 13th - bytes (optional)
    // total len = 4 * csrc_len (bytes)
    if ( iRtpRecvHeader.iCsrcCount > 0 )
        {
        iRtpRecvHeader.iCsrcList = new TUint32[iRtpRecvHeader.iCsrcCount];
        //TInt32* csrc = new TInt32[csrc_len];
        for ( TInt i( 0 ); i < iRtpRecvHeader.iCsrcCount; i++ )
            {
            iRtpRecvHeader.iCsrcList[i] = Read32Bits( aPktBuf, byte );
            }
        }

    // optional extension field
    if ( iRtpRecvHeader.iExtension == 1 )
        {
        /*
        iRtpRecvHeader.iHeaderExtension = new TRtpHeaderExtension();
        // 16 bits
        TInt16 op_code( aPktBuf[byte++] << 8 );
        op_code |= aPktBuf[byte++];
        iRtpRecvHeader.iHeaderExtension->iType = op_code;
        
        // 16 bits
        TInt16 op_code_data_length = aPktBuf[byte++] << 8;
        op_code_data_length |= aPktBuf[byte++];
        iRtpRecvHeader.iHeaderExtension->iLength = op_code_data_length;
        
        op_code_data_length * 4 bytes of extension data
        op_code_data = new TInt32[op_code_data_length];
        iRtpRecvHeader.iHeaderExtension->iData = new TInt32[op_code_data_length];
        for ( TInt i( 0 ); i < op_code_data_length; i++ )
            {
            iRtpRecvHeader.iHeaderExtension)->iData[i] = Read32Bits( aPktBuf, byte );
            }
        */
        }
    
    // The rest is payload data
    iPayload.Set( aPktBuf.Mid( byte ) );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CRtpPacket::SetTimeStamp
// 
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpPacket::SetTimeStamp( const TUint32 aTs )
    {
    iRtpRecvHeader.iTimestamp = aTs;
    }

// -----------------------------------------------------------------------------
// CRtpPacket::Read32Bits
// 
// -----------------------------------------------------------------------------
//
TUint CRtpPacket::Read32Bits( const TPtrC8& aPktBuf, TInt& aByte )
    {
    TUint ret( ( ( TUint )( aPktBuf[aByte++] ) ) << 24 );
    ret |= ( ( TUint )( aPktBuf[aByte++] ) ) << 16;
    ret |= ( ( TUint )( aPktBuf[aByte++] ) ) << 8;
    ret |= ( ( TUint )( aPktBuf[aByte++] ) );
    
    return ret;
    }

// End of File
