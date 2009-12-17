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
* Description:    Implementation of the common recording engine file repairer class.*
*/




// INCLUDE FILES
#include "CRtpClipRepairer.h"
#include <ipvideo/CRtpMetaHeader.h>
#include <ipvideo/CRtpUtil.h>
#include <e32math.h>
#include <bsp.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KMaxGroupTime( 4000 ); // 4s

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpClipRepairer::NewL
// Static two-phased constructor. Leaves object to cleanup stack.
// -----------------------------------------------------------------------------
//
CRtpClipRepairer* CRtpClipRepairer::NewL( MRtpClipRepairObserver* aObs )
    {
    CRtpClipRepairer* self = new( ELeave ) CRtpClipRepairer( aObs );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CRtpClipRepairer::CRtpClipRepairer
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CRtpClipRepairer::CRtpClipRepairer( MRtpClipRepairObserver* aObs )
  : CRtpFileBase(),
    iObs( aObs ),
    iSeekArrayPoint( KErrNotFound )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpClipRepairer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CRtpClipRepairer::ConstructL()
    {
    LOG( "CRtpClipRepairer::ConstructL()" );

    CRtpFileBase::ConstructL();
    }

// -----------------------------------------------------------------------------
// Destructor
//
CRtpClipRepairer::~CRtpClipRepairer()
// -----------------------------------------------------------------------------
    {
    LOG( "CRtpClipRepairer::~CRtpClipRepairer()" );
    
    Cancel();
    delete iFileData;
    delete iMetaHeader;
    }

// -----------------------------------------------------------------------------
// CRtpClipRepairer::CurrentClipName
// Getter for the clip name under repairing.
// -----------------------------------------------------------------------------
//
TPtrC CRtpClipRepairer::CurrentClipName()
    {
    if ( iCurrentPath )
        {
        return iCurrentPath->Des();
        }
    
    return KNullDesC();
    }
    
// -----------------------------------------------------------------------------
// CRtpClipRepairer::CheckMetaHeaderL
// Checks if corrupted meta header of clip is possible to fix.
// -----------------------------------------------------------------------------
//
void CRtpClipRepairer::CheckMetaHeaderL( const TDesC& aClipName )
    {
    LOG1( "CRtpClipRepairer::CheckMetaHeaderL(), aClipName: %S", &aClipName );
    
    // Only one repair at the time
    if ( iMetaHeader || iCurrentPath )
        {
        User::Leave( KErrAlreadyExists );
        }
    
    // Open clip and read the meta header
    delete iCurrentPath; iCurrentPath = NULL;
    iCurrentPath = aClipName.AllocL();
    if ( !iFs.Handle() )
        {
        User::LeaveIfError( iFs.Connect() );
        }
    iFile.Close();
    User::LeaveIfError( iFile.Open( iFs, aClipName,
                        EFileShareExclusive | EFileStream | EFileWrite ) );
    delete iMetaHeader; iMetaHeader = NULL;
    iMetaHeader = CRtpMetaHeader::NewL( iFile, CRtpMetaHeader::EMetaUpdate );
    
    // Attributes
    CRtpMetaHeader::SAttributes att;
    TRAPD( err, iMetaHeader->ReadAttributesL( att ) );
    
    // Verify that clip version not too old?
    if ( att.iVersion < KMinValidClipVersion )
        {
        LOG( "CRtpClipRepairer::CheckMetaHeaderL(), Not Valid Clip Version" );
        User::Leave( KErrGeneral ); 
        }
    
    // If error or rec ongoing -> recording interrupted ie. battery removed
    if ( err || att.iOngoing )
        {
        att.iOngoing = EFalse;
        att.iPlayCount = 0;
        att.iPlaySpot = KErrNone;
        iMetaHeader->WriteAttributesL( att );
        }
    
    // Duration
    TInt duration( 0 );
    iMetaHeader->ReadDurationL( duration );
    LOG1( "CRtpClipRepairer::CheckMetaHeaderL(), duration: %d", duration );

    // Update seek array
    if ( ( !duration || duration > KSeekArrayInterval ) && !ValidSeekHeaderL() )
        {
        UpdateSeekArrayL();
        }
    else
        {
        // Set start time to file date
        TTime startTime( 0 );
        iMetaHeader->ReadStartTimeL( startTime );
        iFile.SetModified( startTime );
        iFile.Close();
        }
    }

// -----------------------------------------------------------------------------
// CRtpClipRepairer::RunL
// -----------------------------------------------------------------------------
//
void CRtpClipRepairer::RunL()
    {
    User::LeaveIfError( iStatus.Int() );
    const TUint prevTime( iGroupTime );
    UpdateGroupHeaderVariablesL( iDataPtr );
    delete iFileData; iFileData = NULL;

    // Current header valid?
    if ( iThisGroup > iNextGroupPoint || iThisGroup < iPrevGroupPoint ||
         iGroupTime < prevTime || iGroupTime > ( prevTime + KMaxGroupTime ) )
        {
        iLastSeekAddr = iPrevGroupPoint;
        iThisGroup = iSeekArrayPoint;
        }
    else
        {
        // Handle readed group
        GroupToSeekArrayL();
        }
    
    // Continue with next group if repair asyncronous?
    if ( iObs )
        {
        if ( iThisGroup < iSeekArrayPoint )
            {
            // Asyncronous ( normal clip repairing )
            ReadNextGroupHeaderFromFileL();
            }
        else
            {
            // All done, finalize the clip
            AddSpecialPacketL( MRtpFileWriteObserver::ERtpClipEnd );
            FinalizeSeekArrayL( KErrNone );
            iObs->RtpClipRepaired( KErrNone );
            }        
        }
    }
    
// -----------------------------------------------------------------------------
// CRtpClipRepairer::RunError
// Returns: System wide error code of indication send leave reason
// -----------------------------------------------------------------------------
//
TInt CRtpClipRepairer::RunError( TInt aError )
    {
    LOG1( "CRtpClipRepairer::RunError(), RunL Leaved: %d", aError );

    TRAP_IGNORE( FinalizeSeekArrayL( aError ) );
    if ( iObs )
        {
        iObs->RtpClipRepaired( ( iGroupsTotalCount )? KErrNone: aError );
        }
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CRtpClipRepairer::DoCancel
// -----------------------------------------------------------------------------
//
void CRtpClipRepairer::DoCancel()
    {
    LOG( "CRtpClipRepairer::DoCancel()" );
    }

// -----------------------------------------------------------------------------
// CRtpClipRepairer::ValidSeekHeaderL
// Verifies if seek header and seek array are valid.
// -----------------------------------------------------------------------------
//
TBool CRtpClipRepairer::ValidSeekHeaderL()
    {
    // Seek header
    iSeekHeaderPoint = iMetaHeader->SeekHeaderPoint();
    ReadSeekHeaderL();
    
    // Seek array point
    iMetaHeader->ReadSeekArrayPointL( iSeekArrayPoint );
    
    // Verify seek array
    if ( iLastSeekAddr > iFirstSeekAddr && iSeekArrayPoint > iLastSeekAddr )
        {
        TInt count( KErrNotFound );
        TRAPD( err, count = ReadSeekArrayL( iSeekArrayPoint ) );
        if ( !err && count > 0 )
            {
            // Seek array is ok
            return ETrue; 
            }
        }
    
    return EFalse;
    }
    
// -----------------------------------------------------------------------------
// CRtpClipRepairer::UpdateSeekArrayL
// Scans all packet groups in clip and updates seek array.
// -----------------------------------------------------------------------------
//
void CRtpClipRepairer::UpdateSeekArrayL()
    {
    LOG( "CRtpClipRepairer::UpdateSeekArrayL()" );

    // Scan complete clip for seek array
    ResetSeekArray();
    User::LeaveIfError( iFile.Size( iSeekArrayPoint ) );
    iThisGroup = iFirstSeekAddr;
    iGroupTime = 0;
    iNextGroupPoint = 0;
    iGroupsTotalCount = 0;
    
    // Start reading
    if ( iObs )
        {
        // Asyncronous
        ReadNextGroupHeaderFromFileL();
        }
    else
        {
        // Syncronous
        TInt err( KErrNone );
        do
            {
            ReadNextGroupHeaderFromFileL();
            TRAP( err, RunL() );
            }
            while ( !err && iThisGroup < iSeekArrayPoint );

        // All done, finalize the clip
        AddSpecialPacketL( MRtpFileWriteObserver::ERtpClipPause );
        FinalizeSeekArrayL( KErrNone );
        }
    }
    
// -----------------------------------------------------------------------------
// CRtpClipRepairer::ReadNextGroupHeaderFromFileL
// Reads RTP payload from a file.
// Payload is allways after data header, so read position set is not needed.
// -----------------------------------------------------------------------------
//
void CRtpClipRepairer::ReadNextGroupHeaderFromFileL()
    {
    iLastSeekAddr = iThisGroup;

    iFileData = HBufC8::NewL( KGroupHeaderBytes );
    iDataPtr.Set( iFileData->Des() );
    if ( iObs )
        {
        iFile.Read( iThisGroup, iDataPtr, KGroupHeaderBytes, iStatus );
        SetActive();
        }
    else
        {
        iStatus = iFile.Read( iThisGroup, iDataPtr, KGroupHeaderBytes );
        }
    }
    
// -----------------------------------------------------------------------------
// CRtpClipRepairer::GroupToSeekArrayL
// Appends next rec group's group time to seek array if interval time exeeded.
// -----------------------------------------------------------------------------
//
void CRtpClipRepairer::GroupToSeekArrayL()
    {
    iGroupsTotalCount++;
    const TInt delta( iGroupTime - iLastGroupTime );
    if ( delta > KSeekArrayInterval )
        {
        LOG2( "CRtpClipRepairer::GroupToSeekArrayL(), iGroupsTotalCount: %d, iThisGroup: %d", 
                                                      iGroupsTotalCount, iThisGroup );
        iLastGroupTime = iGroupTime;
        AppendSeekArrayL( iGroupTime, iThisGroup );
        }
    
    iPrevGroupPoint = iThisGroup;
    iThisGroup = iNextGroupPoint;
    }
    
// -----------------------------------------------------------------------------
// CRtpClipRepairer::AddSpecialPacketL
// Adds special packet to a new group to the end of clip.
// -----------------------------------------------------------------------------
//
void CRtpClipRepairer::AddSpecialPacketL( 
    const MRtpFileWriteObserver::TRtpType aType )
    {
    LOG1( "CRtpClipRepairer::AddSpecialPacketL(), aType: %d", aType );
    
    // Read last group header
    iThisGroup = iPrevGroupPoint;
    ReadGroupHeaderL();

    // End packet
    HBufC8* bytes = CRtpUtil::SpecialPacketL( aType );
    CleanupStack::PushL( bytes );
    iFile.Write( iThisGroup + iGroupTotalLen, bytes->Des(), KSpecialPacketLength );
    CleanupStack::PopAndDestroy( bytes );

    // Update group total size (GTS)
    iGroupTotalLen+= KSpecialPacketLength;
    bytes = CRtpUtil::MakeBytesLC( iGroupTotalLen );
    iFile.Write( iThisGroup, bytes->Des(), KIntegerBytes );
    CleanupStack::PopAndDestroy( bytes );
    
    // Update next group point (NGP)
    iNextGroupPoint+= KSpecialPacketLength;
    bytes = CRtpUtil::MakeBytesLC( iNextGroupPoint );
    iFile.Write( iThisGroup + KIntegerBytes, bytes->Des(), KIntegerBytes );
    CleanupStack::PopAndDestroy( bytes );
    
    // Read packets total count (PTC)
    bytes = HBufC8::NewLC( KPacketsCountBytes );
    TPtr8 ptr( bytes->Des() );
    iFile.Read( iThisGroup + KGroupHeaderBytes, ptr, KPacketsCountBytes );
    const TInt packetsCount( CRtpUtil::GetValueL( ptr ) );
    User::LeaveIfError( packetsCount );
    CleanupStack::PopAndDestroy( bytes );
    
    // Increment packets tolal count (PTC) by one
    bytes = CRtpUtil::MakeBytesLC( packetsCount + 1 );
    iFile.Write( iThisGroup + KGroupHeaderBytes, bytes->Des(), KPacketsCountBytes );
    CleanupStack::PopAndDestroy( bytes );
    }
    
// -----------------------------------------------------------------------------
// CRtpClipRepairer::FinalizeSeekArrayL
// Appends next rec group to seek array.
// -----------------------------------------------------------------------------
//
void CRtpClipRepairer::FinalizeSeekArrayL( const TInt aError )
    {
    LOG1( "CRtpClipRepairer::FinalizeSeekArrayL(), aError: %d", aError );

    // New seek array point
    iMetaHeader->WriteSeekArrayPointL( iNextGroupPoint );

    // Update duration
    iMetaHeader->WriteDurationL( iGroupTime );

    // Last group time
    CRtpMetaHeader::SAttributes att;
    iMetaHeader->ReadAttributesL( att );
    att.iFailed = !( !aError );
    att.iOngoing = EFalse;
    att.iPlayCount = 0;
    att.iPlaySpot = KErrNone;
    iMetaHeader->WriteAttributesL( att );
    
    // Finalise
    SaveSeekArrayL();
    WriteSeekHeaderL();
    iFile.Flush();

    // Set start time to file date
    TTime startTime( 0 );
    iMetaHeader->ReadStartTimeL( startTime );
    iFile.SetModified( startTime );
    iFile.Close();
    }
    
// End of File
