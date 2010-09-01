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
* Description:    Base class for all packet sinks*
*/




// INCLUDES
#include "CCRPacketSinkBase.h"

// CONSTANTS
// None

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRPacketSinkBase::CCRPacketSinkBase
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//  
CCRPacketSinkBase::CCRPacketSinkBase(
    CCRStreamingSession& aSession,
    CCRStreamingSession::TCRSinkId aSinkId ) 
  : iOwningSession( aSession ),
    iSinkId( aSinkId )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRPacketSinkBase::CCRPacketSinkBase
// Destructor.
// -----------------------------------------------------------------------------
//
CCRPacketSinkBase::~CCRPacketSinkBase()
    {    
    iBuffer = NULL; // Does not delete and it is right thing.   
    }

// -----------------------------------------------------------------------------
// CCRPacketSinkBase::SetBuffer
//
// -----------------------------------------------------------------------------
//      
void CCRPacketSinkBase::SetBuffer( CCRPacketBuffer* aBuffer )
    {
    iBuffer = aBuffer;    
    }

// -----------------------------------------------------------------------------
// CCRPacketSinkBase::BufferResetDone
//
// -----------------------------------------------------------------------------
//      
void CCRPacketSinkBase::BufferResetDone()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRPacketSinkBase::StatusChanged
//
// -----------------------------------------------------------------------------
//      
void CCRPacketSinkBase::StatusChanged(
    MCRPacketSource::TCRPacketSourceState /*aNewState*/ )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRPacketSinkBase::Id
// 
// -----------------------------------------------------------------------------
//  
CCRStreamingSession::TCRSinkId CCRPacketSinkBase::Id( void ) const
    {
    return iSinkId; 
    }

// -----------------------------------------------------------------------------
// CCRPacketSinkBase::Pause
// -----------------------------------------------------------------------------
//
TInt CCRPacketSinkBase::Pause()
    {
    return KErrCompletion;
    }

// -----------------------------------------------------------------------------
// CCRPacketSinkBase::Restore
// -----------------------------------------------------------------------------
//
TInt CCRPacketSinkBase::Restore()
    {
    return KErrCompletion;
    }

// -----------------------------------------------------------------------------
// CCRPacketSinkBase::Stop
// -----------------------------------------------------------------------------
//
void CCRPacketSinkBase::Stop()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRPacketSinkBase::SetSeqAndTS
// 
// -----------------------------------------------------------------------------
//      
void CCRPacketSinkBase::SetSeqAndTS(
    TUint& /*aAudioSeq*/,
    TUint& /*aAudioTS*/,
    TUint& /*aVideoSeq*/,
    TUint& /*aVideoTS*/ )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRPacketSinkBase::SetRange
// 
// -----------------------------------------------------------------------------
//      
void CCRPacketSinkBase::SetRange( TReal /*aLower*/, TReal /*aUpper*/ )
    {
    // None
    }

//  End of File
