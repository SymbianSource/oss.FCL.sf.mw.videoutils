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
* Description:    Implementation of RC client's Service*
*/




// INCLUDE FILES
#include "RCRService.h"
#include <ipvideo/CRTypeDefs.h>
#include <e32math.h>

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// RCRService::RCRService
//
// -----------------------------------------------------------------------------
//
RCRService::RCRService() : RCRServiceBase()
    {
    // None
    }

// -----------------------------------------------------------------------------
// RCRService::SetIap
// LiveTV specific.
// -----------------------------------------------------------------------------
//
TInt RCRService::SetIap( const TSubConnectionUniqueId& aIapId )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        SCRRtspIapParams msg;
        RProcess process;
        TInt handleId( process.Handle() );
        msg.iQueueName.Format( KCRMsgQueueName, handleId );
        msg.iQueueName.Trim();
        msg.iConnectionId = aIapId;

        TPckgBuf<SCRRtspIapParams> pckg( msg );
        TIpcArgs args( &pckg );
        return SendReceive( ECRSetIap, args );
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::CancelSetIap
// LiveTV specific.
// -----------------------------------------------------------------------------
//
TInt RCRService::CancelSetIap()
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TIpcArgs args;
        return SendReceive( ECRCancelSetIap, args );
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::PlayRtspUrl
// LiveTV specific.
// -----------------------------------------------------------------------------
//
TInt RCRService::PlayRtspUrl(
    TUint& aSessionChk,
    const SCRRtspParams& aRtspParams )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( 0 );
        TPckgBuf<SCRRtspParams> pckg1( aRtspParams );
        TIpcArgs args( &pckg0, &pckg1 );
        TInt err( SendReceive( ECRPlayRtspUrl, args ) );
        aSessionChk = pckg0();
        return err;
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::PlayDvbhStream
// DVB-H specific.
// -----------------------------------------------------------------------------
//
TInt RCRService::PlayDvbhStream(
    TUint& aSessionChk,
    const SCRLiveParams& aLiveParams  )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( 0 );
        TPckgBuf<SCRLiveParams> pckg1( aLiveParams );
        TIpcArgs args( &pckg0, &pckg1 );
        TInt err( SendReceive( ECRPlayDvbhLive, args ) );
        aSessionChk = pckg0();
        return err;
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::ChangeDvbhService
// DVB-H specific.
// -----------------------------------------------------------------------------
//
TInt RCRService::ChangeDvbhService(
    TUint& aSessionChk,
    const SCRLiveParams& aLiveParams  )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( aSessionChk );
        TPckgBuf<SCRLiveParams> pckg1( aLiveParams );
        TIpcArgs args( &pckg0, &pckg1 );
        TInt err( SendReceive( ECRChangeService, args ) );
        aSessionChk = pckg0();
        return err;
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::PlayRtpFile
// DVB-H specific.
// -----------------------------------------------------------------------------
//
TInt RCRService::PlayRtpFile(
    TUint& aSessionChk,
    const SCRRtpPlayParams& aRtpFile  )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( 0 );
        TPckgBuf<SCRRtpPlayParams> pckg1( aRtpFile );
        TIpcArgs args( &pckg0, &pckg1 );
        TInt err( SendReceive( ECRPlayRtpFile, args ) );
        aSessionChk = pckg0();
        return err;
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::PlayRtpFile
// DVB-H specific.
// -----------------------------------------------------------------------------
//
TInt RCRService::PlayRtpFile(
    TUint& aSessionChk,
    const RFile& aRtpHandle )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg( 0 );
        TIpcArgs args( &pckg );
        aRtpHandle.TransferToServer( args, 1, 2 );
        TInt err( SendReceive( ECRPlayRtpHandle, args ) );
        aSessionChk = pckg();
        return err;
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::RecordCurrentStream
// UI command.
// -----------------------------------------------------------------------------
//
TInt RCRService::RecordCurrentStream(
    const TUint aSessionChk,
    const SCRRecordParams& aRecordParams )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( aSessionChk );
        TPckgBuf<SCRRecordParams> pckg1( aRecordParams );
        TIpcArgs args( &pckg0, &pckg1 );
        return SendReceive( ECRRecordCurrentStream, args );
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::RecordRtspStream
// UI command.
// -----------------------------------------------------------------------------
//
TInt RCRService::RecordRtspStream(
    TUint& aSessionChk,
    const SCRRtspParams& aRtspUrl,
    const SCRRecordParams& aRecordParams )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( 0 );
        TPckgBuf<SCRRtspParams> pckg1( aRtspUrl );
        TPckgBuf<SCRRecordParams> pckg2( aRecordParams );
        TIpcArgs args( &pckg0, &pckg1, &pckg2 );
        TInt err( SendReceive( ECRRecordRtspStream, args ) );
        aSessionChk = pckg0();
        return err;
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::RecordDvbhStream
// UI command.
// -----------------------------------------------------------------------------
//
TInt RCRService::RecordDvbhStream(
    TUint& aSessionChk,
    const SCRLiveParams& aLiveParams,
    const SCRRecordParams& aRecordParams )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( 0 );
        TPckgBuf<SCRLiveParams> pckg1( aLiveParams );
        TPckgBuf<SCRRecordParams> pckg2( aRecordParams );
        TIpcArgs args( &pckg0, &pckg1, &pckg2 );
        TInt err( SendReceive( ECRRecordDvbhStream, args ) );
        aSessionChk = pckg0();
        return err;
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::PauseRecordStream
// UI command.
// -----------------------------------------------------------------------------
//
TInt RCRService::PauseRecordStream( const TUint aSessionChk, const TBool& aStart )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( aSessionChk );
        TPckgBuf<TBool> pckg1( aStart );
        TIpcArgs args( &pckg0, &pckg1 );
        return SendReceive( ECRPauseRecordStream, args );
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::StopRecordStream
// UI command.
// -----------------------------------------------------------------------------
//
TInt RCRService::StopRecordStream( const TUint aSessionChk )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( aSessionChk );
        TIpcArgs args( &pckg0 );
        return SendReceive( ECRStopRecordStream, args );
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::StartTimeShift
// UI command.
// -----------------------------------------------------------------------------
//
TInt RCRService::StartTimeShift(
    TUint& aTimeShiftChk,
    const TUint aCurrentChk )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( 0 );
        TPckgBuf<TUint> pckg1( aCurrentChk );
        TIpcArgs args( &pckg0, &pckg1 );
        TInt err( SendReceive( ECRStartTimeShift, args ) );
        aTimeShiftChk = pckg0();
        return err;
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::StopTimeShift
// UI command.
// -----------------------------------------------------------------------------
//
TInt RCRService::StopTimeShift( 
    const TUint aTimeShiftChk,
    const TUint aCurrentChk )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( aTimeShiftChk );
        TPckgBuf<TUint> pckg1( aCurrentChk );
        TIpcArgs args( &pckg0, &pckg1 );
        return SendReceive( ECRStopTimeShift, args );
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::PlayCommand
// Player command.
// -----------------------------------------------------------------------------
//
TInt RCRService::PlayCommand(
    const TUint aSessionChk,
    const TReal& aStartPos,
    const TReal& aEndPos )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( aSessionChk );
        TPckgBuf<TInt64> pckg1( aStartPos );
        TPckgBuf<TInt64> pckg2( aEndPos );
        TIpcArgs args( &pckg0, &pckg1, &pckg2 );
        return SendReceive( ECRPlayCommand, args );
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::PauseCommand
// Player command.
// -----------------------------------------------------------------------------
//
TInt RCRService::PauseCommand( const TUint aSessionChk )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( aSessionChk );
        TIpcArgs args( &pckg0 );
        return SendReceive( ECRPauseCommand, args );
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::StopCommand
// Player command.
// -----------------------------------------------------------------------------
//
TInt RCRService::StopCommand( const TUint aSessionChk )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( aSessionChk );
        TIpcArgs args( &pckg0 );
        return SendReceive( ECRStopCommand, args );
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::SetPosition
// Sets position of packet source.
// -----------------------------------------------------------------------------
//
TInt RCRService::SetPosition(
    const TUint aSessionChk,
    const TInt64 aPosition )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( aSessionChk );
        TPckgBuf<TInt64> pckg1( aPosition );

        TIpcArgs args( &pckg0, &pckg1 );
        return SendReceive( ECRSetPosition, args );
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::GetPosition
// Gets position of packet source.
// -----------------------------------------------------------------------------
//
TInt RCRService::GetPosition(
    const TUint aSessionChk,
    TInt64& aPosition,
    TInt64& aDuration )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( aSessionChk );
        TPckgBuf<TInt64> pckg1( aPosition );
        TPckgBuf<TInt64> pckg2( 0 );

        TIpcArgs args( &pckg0, &pckg1, &pckg2 );
        TInt err( SendReceive( ECRGetPosition, args ) );
        aPosition = pckg1();
        aDuration = pckg2();
        return err;
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::CloseSession
// Player command.
// -----------------------------------------------------------------------------
//
TInt RCRService::CloseSession( const TUint aSessionChk )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( aSessionChk );
        TIpcArgs args( &pckg0 );
        return SendReceive( ECRCloseSession, args );
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::PlayNullSource
// Debug purposes.
// -----------------------------------------------------------------------------
//
TInt RCRService::PlayNullSource( TUint& aSessionChk )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( 0 );
        TIpcArgs args( &pckg0 );
        TInt err( SendReceive( ECRPlayNullSource, args ) );
        aSessionChk = pckg0();
        return err;
        }
    
    return KErrServerTerminated;
    }

// -----------------------------------------------------------------------------
// RCRService::PlayRtspUrlToNullSink
// Debug purposes.
// -----------------------------------------------------------------------------
//
TInt RCRService::PlayRtspUrlToNullSink(
    TUint& aSessionChk,
    const SCRRtspParams& aRtspParams  )
    {
    __ASSERT_ALWAYS( &( SessionHandle() ), PanicClient( KErrBadHandle ) );
    if ( SubSessionHandle() )
        {
        TPckgBuf<TUint> pckg0( 0 );
        TPckgBuf<SCRRtspParams> pckg1( aRtspParams );
        TIpcArgs args( &pckg0, &pckg1 );
        TInt err( SendReceive( ECRPlayRtspUrlToNullSink, args ) );
        aSessionChk = pckg0();
        return err;
        }
    
    return KErrServerTerminated;
    }

// End of File
