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
* Description:    Implementation of IptvRTP client's API*
*/




// INCLUDE FILES
#include <ipvideo/CCRDvrApi.h>
#include "RCRService.h"
#include "RCRClient.h"
#include <ipvideo/MCREngineObserver.h>
#include "CCRMsgQueueObserver.h"
#include "CCRServerHandleSingleton.h"

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRDvrApi::NewL
// Static two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CCRDvrApi* CCRDvrApi::NewL( MCREngineObserver *aObserver )
    {
    CCRDvrApi* self = new( ELeave ) CCRDvrApi();
    CleanupStack::PushL( self );
    self->ConstructL( aObserver );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::NewL
// Static two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CCRDvrApi* CCRDvrApi::NewL()
    {
    CCRDvrApi* self = new( ELeave ) CCRDvrApi();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::CCRDvrApi
// -----------------------------------------------------------------------------
//
CCRDvrApi::CCRDvrApi()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRDvrApi::ConstructL( MCREngineObserver *aObserver )
    {
    CCRApiBase::BaseConstructL();
    iQueueObserver = CCRMsgQueueObserver::NewL();
    iQueueObserver->SetSessionId( 0 );
    iQueueObserver->AddMsgQueueObserverL( aObserver );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRDvrApi::ConstructL()
    {
    CCRApiBase::BaseConstructL();
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::~CCRDvrApi
// -----------------------------------------------------------------------------
//
EXPORT_C CCRDvrApi::~CCRDvrApi()
    {
    // note what happens in CCRAPIBase, the iClient et al.
    // might get deleted there. 
    delete iQueueObserver; 
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::SetIap
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::SetIap( const TSubConnectionUniqueId& aIapId ) 
    {
    return iSingleton->Service().SetIap( aIapId );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::CancelSetIap
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::CancelSetIap() 
    {
    return iSingleton->Service().CancelSetIap();
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::PlayRtspUrl
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::PlayRtspUrl(
    TUint& aSessionChk,
    const SCRRtspParams& aRtspUrl )
    {
    iQueueObserver->SetSessionId( 0 );
    TInt err( iSingleton->Service().PlayRtspUrl( aSessionChk, aRtspUrl ) );
    if ( !err )
        {
        iQueueObserver->SetSessionId( aSessionChk );
        }
    
    return err;
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::PlayDvbhStream
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::PlayDvbhStream(
    TUint& aSessionChk,
    const SCRLiveParams& aDvbhLive )
    {
    iQueueObserver->SetSessionId( 0 );
    TInt err( iSingleton->Service().PlayDvbhStream( aSessionChk, aDvbhLive ) );
    if ( !err )
        {
        iQueueObserver->SetSessionId( aSessionChk );
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::ChangeDvbhService
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::ChangeDvbhService(
    TUint& aSessionChk,
    const SCRLiveParams& aDvbhLive )
    {
    TInt err( iSingleton->Service().ChangeDvbhService( aSessionChk, aDvbhLive ) );
    if ( !err )
        {
        iQueueObserver->SetSessionId( aSessionChk );
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::PlayRtpFile
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::PlayRtpFile(
    TUint& aSessionChk,
    const SCRRtpPlayParams& aRtpFile )
    {
    iQueueObserver->SetSessionId( 0 );
    TInt err( iSingleton->Service().PlayRtpFile( aSessionChk, aRtpFile ) );
    if ( !err )
        {
        iQueueObserver->SetSessionId( aSessionChk );
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::PlayRtpFile
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::PlayRtpFile(
    TUint& aSessionChk,
    const RFile& aRtpHandle )
    {
    iQueueObserver->SetSessionId( 0 );
    TInt err( iSingleton->Service().PlayRtpFile( aSessionChk, aRtpHandle ) );
    if ( !err )
        {
        iQueueObserver->SetSessionId( aSessionChk );
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::RecordCurrentStream
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::RecordCurrentStream( 
    const TUint aSessionChk,
    const SCRRecordParams& aRecordParams )
    {
    return iSingleton->Service().RecordCurrentStream( aSessionChk, aRecordParams );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::RecordRtspStream
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::RecordRtspStream( 
    TUint& aSessionChk,
    const SCRRtspParams& aRtspUrl,
    const SCRRecordParams& aRecordParams )
    {
    return iSingleton->Service().RecordRtspStream( 
        aSessionChk, aRtspUrl, aRecordParams );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::RecordDvbhStream
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::RecordDvbhStream( 
    TUint& aSessionChk,
    const SCRLiveParams& aLiveParams,
    const SCRRecordParams& aRecordParams )
    {
    return iSingleton->Service().RecordDvbhStream( 
        aSessionChk, aLiveParams, aRecordParams );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::PauseRecordStream
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::PauseRecordStream(
    const TUint aSessionChk,
    const TBool& aStart )
    {
    return iSingleton->Service().PauseRecordStream( aSessionChk, aStart );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::StopRecordStream
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::StopRecordStream( const TUint aSessionChk )
    {
    return iSingleton->Service().StopRecordStream( aSessionChk );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::StartTimeShift
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::StartTimeShift( 
    TUint& aTimeShiftChk,
    const TUint aSessionChk )
    {
    return iSingleton->Service().StartTimeShift( aTimeShiftChk, aSessionChk );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::StopTimeShift
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::StopTimeShift(
    const TUint aTimeShiftChk,
    const TUint aCurrentChk )
    {
    return iSingleton->Service().StopTimeShift( aTimeShiftChk, aCurrentChk );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::PlayCommand
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::PlayCommand(
    const TUint aSessionChk,
    const TReal aStartPos,
    const TReal aEndPos ) 
    {
    return iSingleton->Service().PlayCommand( aSessionChk, aStartPos, aEndPos );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::PauseCommand
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::PauseCommand( const TUint aSessionChk ) 
    {
    return iSingleton->Service().PauseCommand( aSessionChk );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::StopCommand
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::StopCommand( const TUint aSessionChk ) 
    {
    return iSingleton->Service().StopCommand( aSessionChk );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::SetPosition
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::SetPosition(
    const TUint aSessionChk,
    const TInt64 aPosition )
    {
    return iSingleton->Service().SetPosition( aSessionChk, aPosition );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::GetPosition
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::GetPosition(
    const TUint aSessionChk,
    TInt64& aPosition,
    TInt64& aDuration )
    {
    return iSingleton->Service().GetPosition( aSessionChk, aPosition, aDuration );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::CloseSession
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::CloseSession( const TUint aSessionChk ) 
    {
    return iSingleton->Service().CloseSession( aSessionChk );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::PlayNullSource
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::PlayNullSource( TUint& aSessionChk )
    {
    return iSingleton->Service().PlayNullSource( aSessionChk );
    }

// -----------------------------------------------------------------------------
// CCRDvrApi::PlayRtspUrlToNullSink
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCRDvrApi::PlayRtspUrlToNullSink(
    TUint& aSessionChk,
    const SCRRtspParams& aRtspUrl )
    {
    iQueueObserver->SetSessionId( 0 );
    TInt err( iSingleton->Service().PlayRtspUrlToNullSink( aSessionChk, aRtspUrl ) );
    if ( !err )
        {
        iQueueObserver->SetSessionId( aSessionChk );
        }
    return err;
    }

//  End of File


