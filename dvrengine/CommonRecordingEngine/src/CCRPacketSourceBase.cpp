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
* Description:    Base class for all packet sources*
*/




// INCLUDES
#include "CCRPacketSourceBase.h"
#include "CCRPacketBuffer.h"
#include "videoserviceutilsLogger.h"

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::CCRPacketSourceBase
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//  
CCRPacketSourceBase::CCRPacketSourceBase(
    CCRStreamingSession& aSession,
    CCRStreamingSession::TCRSourceId aSourceId )
  : iOwningSession( aSession ),
    iSourceId( aSourceId )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::CCRPacketSourceBase
// Destructor.
// -----------------------------------------------------------------------------
//
CCRPacketSourceBase::~CCRPacketSourceBase()
    {    
    iBuffer = NULL; // Does not delete and it is right thing.   
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::SetBuffer
//
// -----------------------------------------------------------------------------
//      
void CCRPacketSourceBase::SetBuffer( CCRPacketBuffer* aBuffer )
    {
    iBuffer = aBuffer;
    // By default variables are set for continous streaming
    iBuffer->ContinousStream( ETrue );
    iBuffer->MoreComing( ETrue );
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::SeqAndTS
// -----------------------------------------------------------------------------
//
TInt CCRPacketSourceBase::SeqAndTS(
    TUint& /*aAudioSeq*/,
    TUint& /*aAudioTS*/,
    TUint& /*aVideoSeq*/,
    TUint& /*aVideoTS*/ )
    {
    return KErrNotReady;
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::Id
// 
// -----------------------------------------------------------------------------
//  
CCRStreamingSession::TCRSourceId CCRPacketSourceBase::Id( void ) const
    {
    return iSourceId; 
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::RegisterConnectionObs
// -----------------------------------------------------------------------------
//
void CCRPacketSourceBase::RegisterConnectionObs(
    MCRConnectionObserver* /*aObserver*/ )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::Play
// -----------------------------------------------------------------------------
//
TInt CCRPacketSourceBase::Play(
    const TReal& /*aStartPos*/,
    const TReal& /*aEndPos*/ )
    {
    LOG( "CCRPacketSourceBase::Play(), KErrCompletion !" );

    return KErrCompletion;
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::Pause
// -----------------------------------------------------------------------------
//
TInt CCRPacketSourceBase::Pause()
    {
    LOG( "CCRPacketSourceBase::Pause(), KErrCompletion !" );

    return KErrCompletion;
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::Stop
// -----------------------------------------------------------------------------
//
TInt CCRPacketSourceBase::Stop()
    {
    LOG( "CCRPacketSourceBase::Stop(), KErrCompletion" );
    
    return KErrCompletion;
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::SetPosition
//
// -----------------------------------------------------------------------------
//      
TInt CCRPacketSourceBase::SetPosition( const TInt64 /*aPosition*/ )
    {
    LOG( "CCRPacketSourceBase::SetPosition(), KErrCompletion !" );

    return KErrCompletion;
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::GetPosition
//
// -----------------------------------------------------------------------------
//      
TInt CCRPacketSourceBase::GetPosition( TInt64& aPosition, TInt64& aDuration )
    {
    LOG( "CCRPacketSourceBase::GetPosition(), KErrCompletion !" );

    aPosition = 0;
    aDuration = 0;
    return KErrCompletion;
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::GetRange
// -----------------------------------------------------------------------------
//
void CCRPacketSourceBase::GetRange( TReal& aLower, TReal& aUpper )
    {
    aLower = KRealZero;
    aUpper = KRealMinusOne;
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::PostActionL
// -----------------------------------------------------------------------------
//
void CCRPacketSourceBase::PostActionL()
    {
    LOG( "CCRPacketSourceBase::PostActionL(), Leaves KErrCompletion !" );

    User::Leave( KErrCompletion );
    }

// -----------------------------------------------------------------------------
// CCRPacketSourceBase::Restore
// -----------------------------------------------------------------------------
//
void CCRPacketSourceBase::Restore()
    {
    // None
    }

//  End of File
