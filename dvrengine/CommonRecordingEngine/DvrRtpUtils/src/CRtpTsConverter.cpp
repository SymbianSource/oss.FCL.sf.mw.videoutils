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
* Description:    Class to provide TS conversion for an RTP-stream based on*
*/




// INCLUDE FILES
#include "CRtpTsConverter.h"
#include <ipvideo/CRtpUtil.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KValidRtcpType( 200 );
const TInt KTypeBytesPoint( 1 );
const TInt KNtpSecBytesPoint( 8 );
const TInt KNtpFracBytesPoint( 12 );
const TInt KRtcpTsBytesPoint( 16 );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpTsConverter::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpTsConverter* CRtpTsConverter::NewL( const TUint aClockRate )
    {
    CRtpTsConverter* self = new( ELeave ) CRtpTsConverter( aClockRate );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
    
// -----------------------------------------------------------------------------
// CRtpTsConverter::CRtpTsConverter
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CRtpTsConverter::CRtpTsConverter( const TUint aClockRate )
  : iClockRate( aClockRate ),
    iOffset( 0 ),
    iInitiated( EFalse )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpTsConverter::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CRtpTsConverter::ConstructL()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpTsConverter::CRtpTsConverter
// Destructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpTsConverter::~CRtpTsConverter()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpTsConverter::Init
// 
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpTsConverter::Init( const TDesC8& aRtcpPkt )
    {
    // Verify packet type?
    if ( aRtcpPkt.Length() > ( KRtcpTsBytesPoint + KIntegerBytes ) && 
         aRtcpPkt[KTypeBytesPoint] == KValidRtcpType && iClockRate > 0 )
        {
        // Handle RTCP packet
        TUint ntp_sec( 0 );
        TUint ntp_frac( 0 );
        TUint rtcp_ts( 0 );
        TInt err( CRtpUtil::GetValue( 
             aRtcpPkt.Mid( KNtpSecBytesPoint, KIntegerBytes ), ntp_sec ) );
        if ( !err )
            {
            LOG1( "CRtpTsConverter::Init(), NTP Timestamp, MSW: %u", ntp_sec );
            err = CRtpUtil::GetValue( 
                  aRtcpPkt.Mid( KNtpFracBytesPoint, KIntegerBytes ), ntp_frac );
            }
        if ( !err )
            {
            LOG1( "CRtpTsConverter::Init(), NTP Timestamp, LSW: %u", ntp_frac );
            err = CRtpUtil::GetValue( 
                  aRtcpPkt.Mid( KRtcpTsBytesPoint, KIntegerBytes ), rtcp_ts );
            }
        
        if ( !err )
            {
            LOG1( "CRtpTsConverter::Init(), RTP Timestamp: %u", rtcp_ts );
            // Calculate the wallclock time when this RTCP packet is generated
            TUint wallClock( ( ntp_sec & 0x00FF ) * 1000 );
            wallClock += ( ( ( ntp_frac >> 16 ) & 0x0000FFFF ) * 1000 ) >> 16;
            
            // Compute the proper time offset
            iOffset = ComputeOffset( rtcp_ts, wallClock, iClockRate );
            }
        }
    }
// -----------------------------------------------------------------------------
// CRtpTsConverter::Init
// Compute the proper time offset. No wallclok time available.
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpTsConverter::Init( const TUint& aTs )
    {
    iOffset = ComputeOffset( aTs, 0, iClockRate );
    }

// -----------------------------------------------------------------------------
// CRtpTsConverter::Initiated
// Returns: initiated status of converter.
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CRtpTsConverter::Initiated()
    {
    return iInitiated;
    }

// -----------------------------------------------------------------------------
// CRtpTsConverter::UnInitiate
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpTsConverter::UnInitiate()
    {
    iInitiated = EFalse;
    }

// -----------------------------------------------------------------------------
// CRtpTsConverter::ConvertTs
// Returns: converted unsigned integer value of timestamp.
// -----------------------------------------------------------------------------
//
EXPORT_C TUint CRtpTsConverter::ConvertTs( 
    const TUint aTimestamp,
    TBool aUseWallClock )
    {
    TUint ret( KMaxTUint );

    if ( iClockRate == KPipelineClockRate )
        {
        ret = aTimestamp - iOffset;
        }
    else
        {
        if ( iClockRate > 0 )
            {
            ret = aTimestamp - iOffset;
            
            if ( aUseWallClock )
                {
                TUint sec( ret / iClockRate );
                TUint subSec( ret % iClockRate );
                ret = ( ( sec * KPipelineClockRate ) + 
                        ( subSec * KPipelineClockRate ) / iClockRate );
                }
            }
        }

    return ret;
    }

// -----------------------------------------------------------------------------
// CRtpTsConverter::ComputeOffset
// Returns: offset of the RTP timestamp.
// -----------------------------------------------------------------------------
//
TUint CRtpTsConverter::ComputeOffset(
    TUint aRtpTime,
    TUint aSeekTime,
    TInt aClockRate )
    {
    TUint ret( KMaxTUint );

    if ( aClockRate )
        {
        TUint rtpSec( aRtpTime / aClockRate );
        TUint rtpFracSecTU( aRtpTime % aClockRate ); // transport units
        TUint seekSec( aSeekTime / 1000 );
        TUint seekFracSecTU( ( ( aSeekTime % 1000) * aClockRate ) / 1000 );

        TUint offsetSec( rtpSec - seekSec );
        TUint offsetFracSecTU( rtpFracSecTU - seekFracSecTU );

        if ( rtpFracSecTU < seekFracSecTU )
            {
            offsetSec--;
            offsetFracSecTU = aClockRate + rtpFracSecTU - seekFracSecTU;
            }

        ret = ( offsetSec * aClockRate ) + offsetFracSecTU;
        iInitiated = ETrue;
        }
    
    return ret;
    }

// End of File
