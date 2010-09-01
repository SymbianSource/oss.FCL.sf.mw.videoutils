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
* Description:    Implementation of the Common Recording Engine RTP save format class.*
*/




// INCLUDE FILES
#include "CRtpToFile.h"
#include <ipvideo/CRtpMetaHeader.h>
#include <e32math.h>
#include <bsp.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TUint KMaxValidDelta( 500 );     // 0.5 s
const TUint8 KDummyFullQuality( 100 ); // 100%

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpToFile::NewL
// Static two-phased constructor. Leaves object to cleanup stack.
// -----------------------------------------------------------------------------
//
CRtpToFile* CRtpToFile::NewL(
    MRtpFileObserver& aFileObs,
    MRtpFileWriteObserver& aWriteObs )
    {
    CRtpToFile* self = new( ELeave ) CRtpToFile( aFileObs, aWriteObs );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CRtpToFile::CRtpToFile
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CRtpToFile::CRtpToFile(
    MRtpFileObserver& aFileObs,
    MRtpFileWriteObserver& aWriteObs )
  : CRtpFileBase(),
    iFileObs( aFileObs ),
    iWriteObs( aWriteObs ),
    iCurrentTime( 0 ),
    iPreviousTime( 0 ),
    iPreviousDelta( 0 ),
    iReferenceTime( 0 ),
    iRecordEndTime( 0 ),
    iSeekArrayReference( 0 ),
    iGroupReUse( KErrNotFound ),
    iAction( MRtpFileWriteObserver::ESaveEnd )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpToFile::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CRtpToFile::ConstructL()
    {
    LOG( "CRtpToFile::ConstructL()" );
    
    CRtpFileBase::ConstructL();
    iCurrentPath = HBufC::NewL( 0 );
    }

// -----------------------------------------------------------------------------
// Destructor
//
CRtpToFile::~CRtpToFile()
// -----------------------------------------------------------------------------
    {
    LOG( "CRtpToFile::~CRtpToFile()" );

    Cancel();
    }

// -----------------------------------------------------------------------------
// CRtpToFile::InitRtpSaveL
// Sets path of RTP file and initiates variables.
// -----------------------------------------------------------------------------
//
void CRtpToFile::InitRtpSaveL(
    const MRtpFileWriteObserver::SRtpRecParams& aParams,
    const MRtpFileWriteObserver::TRtpSaveAction& aAction )
    {
    LOG1( "CRtpToFile::InitRtpSaveL() in, ClipPath: %S", &aParams.iClipPath );
    User::LeaveIfError( ( iMode != EModeNone ) * KErrInUse );
    
    // Mode
    switch ( aAction )
        {
        case MRtpFileWriteObserver::ESaveTimeShift:
            iMode = EModeTimeShift;
            break;

        default:
            iMode = EModeNormal;
            break;
        }

    // File server
    if ( !iFs.Handle() )
        {
        User::LeaveIfError( iFs.Connect() );
        }

    // Create clip
    CreateNewClipL( aParams );

    // Real clip's end time
    iPreviousTime = 0;
    iReferenceTime = iGroupTime * KSiKilo;
    UpdateCurrentTimeL();
    TInt64 duration( aParams.iEndTime.Int64() - 
                     aParams.iStartTime.Int64() ); 
    iRecordEndTime = iCurrentTime.Int64() + duration;
    
    // Prepare variables
    iSeekArrayReference = iGroupTime;
    iStartGroupTime = iGroupTime;

    LOG( "CRtpToFile::InitRtpSaveL() out" );
    }

// -----------------------------------------------------------------------------
// CRtpToFile::ActivateGroupsReuseL
// Starts reuse packet groups for live record when they are played.
// -----------------------------------------------------------------------------
//
void CRtpToFile::ActivateGroupsReuseL()
    {
    LOG2( "CRtpToFile::ActivateGroupsReuseL(), iMode: %d, iGroupReUse: %d",
                                              iMode, iGroupReUse );
    if ( iGroupReUse != KErrNotFound || iMode != EModeTimeShift )
        {
        User::Leave( KErrInUse );
        }

    iGroupReUse = KErrInUse;
    }
    
// -----------------------------------------------------------------------------
// CRtpToFile::SwapClipL
// Sets new path of RTP file and initiates variables.
// -----------------------------------------------------------------------------
//
void CRtpToFile::SwapClipL( const MRtpFileWriteObserver::SRtpRecParams& aParams )
    {
    LOG1( "CRtpToFile::SwapClipL(), aClipPath: %S", &aParams.iClipPath );

    User::LeaveIfError( ( iMode != EModeTimeShift ) * KErrGeneral );

    // Update old clip
    WriteSeekHeaderL();
    iGroupReUse = KErrNotFound;
        
    // Open new clip
    CreateNewClipL( aParams );
    }
    
// -----------------------------------------------------------------------------
// CRtpToFile::SaveNextRtpGroupL
// Saves one RTP packet group to a specified file.
// -----------------------------------------------------------------------------
//
TInt CRtpToFile::SaveNextGroupL(
    TPtr8& aGroup,
    TUint& aGroupLength,
    const MRtpFileWriteObserver::TRtpSaveAction& aAction )
    {
    // Verify data and mode
    User::LeaveIfError( iMode );
    
    // Group
    iDataPtr.Set( aGroup );
    
    // Set group variables
    AddGroupL();
    GroupTimeL( aGroupLength );
    AddGroupHeaderL(); 

    // Write to file
    iAction = aAction;
    iFile.Write( iThisGroup, iDataPtr, iGroupTotalLen, iStatus );
    SetActive();

    LOG2( "CRtpToFile::SaveNextGroupL(), iThisGroup: %d, iGroupTime: %u",
                                         iThisGroup, iGroupTime );
#ifdef CR_ALL_LOGS
    LogVariables( _L( "SaveNextGroupL()" ) );
#endif // CR_ALL_LOGS
    
    return iThisGroup;
    }

// -----------------------------------------------------------------------------
// CRtpToFile::UpdatePreviousTimeL
// Updates previous time after pause.
// -----------------------------------------------------------------------------
//
void CRtpToFile::UpdatePreviousTimeL()
    {
    UpdateCurrentTimeL();
    iPreviousTime = iCurrentTime.Int64();
    }
    
// -----------------------------------------------------------------------------
// CRtpToFile::StopRtpSave
// Stops file saving and finalizes header.
// -----------------------------------------------------------------------------
//
void CRtpToFile::StopRtpSave( const TInt aError )
    {
    Cancel();
    const TRtpFileMode mode( iMode ); 
    
    // If active
    if ( mode != EModeNone )
        {
#ifdef CR_ALL_LOGS
        LogVariables( _L( "StopRtpSave()" ) );
#endif // CR_ALL_LOGS
        iMode = EModeNone;

        // Update clip headers
        if ( mode != EModeTimeShift )
            {
            TRAP_IGNORE( WriteFinalMetaHeaderL( aError ) );
            }
        else
            {
            iLastSeekAddr = KMaxTInt;
            TRAP_IGNORE( WriteSeekHeaderL() );
            }

        // Close file
        iFile.Flush();
        iFile.Close();

        if ( aError == KErrNoMemory && !iGroupsTotalCount )
            {
            // Failed due to insufficient disk space, and couldn't save any
            // packets to clip. Happens when recording is started with disk 
            // space already below threshold, and failed to free any space.
            // Delete the clip completely, otherwise we are just consuming 
            // space below threshold(s).
            LOG( "CRtpToFile::StopRtpSave(), deleting file without packets !" );
            iFs.Delete( *iCurrentPath );
            }
        }
    }

// -----------------------------------------------------------------------------
// CRtpToFile::GetClipPath
// Getter for full path of currently recorded clip.
// -----------------------------------------------------------------------------
//
HBufC* CRtpToFile::ClipPath()
    {
    return iCurrentPath;
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::GetCurrentLength
// Gets the current length of the clip during recording.
// -----------------------------------------------------------------------------
//
TUint CRtpToFile::GetCurrentLength()
    {
    return iGroupTime;
    }
    
// -----------------------------------------------------------------------------
// CRtpFromFile::UpdateRecordEndTime
// Uppdates the current recording end time.
// -----------------------------------------------------------------------------
//
void CRtpToFile::UpdateRecordEndTime( const TTime& aEndTime )
    {
    if ( aEndTime > iCurrentTime )
        {
        iRecordEndTime = aEndTime.Int64();
        }
    }
    
// -----------------------------------------------------------------------------
// CRtpToFile::UpdatePlayAttL
// Updates clip's playback count and spot attributes after watching.
// -----------------------------------------------------------------------------
//
void CRtpToFile::UpdatePlayAttL( const TInt aNewSpot )
    {
    CRtpMetaHeader* metaheader = CRtpMetaHeader::NewLC(
                                 iFile, CRtpMetaHeader::EMetaUpdate );
    CRtpMetaHeader::SAttributes att;
    metaheader->ReadAttributesL( att );
    
    // Step playback counter by one
    att.iPlayCount++;
    // Update playback spot
    att.iPlaySpot = ( aNewSpot > 0 && aNewSpot < iLastSeekAddr )? aNewSpot:
                                                                  KErrNone;
    metaheader->WriteAttributesL( att );
    CleanupStack::PopAndDestroy( metaheader );
    LOG2( "CRtpToFile::UpdatePlayAttL(), New playback count: %d, spot: %d", 
                                         att.iPlayCount, att.iPlaySpot );
    }
    
// -----------------------------------------------------------------------------
// CRtpToFile::RunL
// -----------------------------------------------------------------------------
//
void CRtpToFile::RunL()
    {
    User::LeaveIfError( iStatus.Int() );
    User::LeaveIfError( iFile.Flush() );
    
    // Start packets re-use?
    if ( iGroupReUse == KErrInUse )
        {
        const TInt point( iFileObs.CurrentFileReadPoint( 0 ) );
        if ( point > ( iSeekHeaderPoint + KSeekHeaderBytes ) )
            {
            iGroupReUse = KErrNone;
            }
        }

    // Stop recording if time shift too close to live
    if ( iGroupReUse > KErrNone && 
         iFileObs.CurrentFileReadPoint( 1 ) < KErrNone )
        {
        iAction = MRtpFileWriteObserver::ESaveEnd;
        LOG( "CRtpToFile::RunL(), Time shift play too close to record !" );
        }
    
    // Stop recording if end time reached
    if ( iCurrentTime.Int64() > iRecordEndTime )
        {
        iAction = MRtpFileWriteObserver::ESaveEnd;
        LOG( "CRtpToFile::RunL(), Record end time reached !" );
        }
    
    iFileObs.RtpGroupSaved( iAction );
    }
    
// -----------------------------------------------------------------------------
// CRtpToFile::RunError
// -----------------------------------------------------------------------------
//
TInt CRtpToFile::RunError( TInt aError )
    {
    LOG1( "CRtpToFile::RunError(), RunL Leaved: %d", aError );

    if ( &iWriteObs )
        {
        iWriteObs.WriteStatus( aError );
        }
    
    StopRtpSave( aError );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CRtpToFile::DoCancel
// -----------------------------------------------------------------------------
//
void CRtpToFile::DoCancel()
    {
    LOG( "CRtpToFile::DoCancel()" );
    }

// -----------------------------------------------------------------------------
// CRtpToFile::CreateNewClipL
// Opens new clip and creates initial headers.
// -----------------------------------------------------------------------------
//
void CRtpToFile::CreateNewClipL(
    const MRtpFileWriteObserver::SRtpRecParams& aParams )
    {
    // Open file
    iFile.Close();
    User::LeaveIfError( iFile.Replace( iFs, aParams.iClipPath,
                        EFileShareAny | EFileStream | EFileWrite ) );
    // Headers
    WriteInitialMetaHeaderL( aParams );
    WriteSeekHeaderL();
    const TInt firstGroup( iSeekHeaderPoint + KSeekHeaderBytes );
    
    // Variables
    iGroupTime = 0;
    iGroupsTotalCount = 0;
    iFirstSeekAddr = firstGroup;
    iLastSeekAddr = firstGroup;
    iNextGroupPoint = firstGroup;
    delete iCurrentPath; iCurrentPath = NULL;
    iCurrentPath = aParams.iClipPath.AllocL();
    }
    
// -----------------------------------------------------------------------------
// CRtpToFile::AddGroupL
// Updates file and packet group header variables for a new group.
// -----------------------------------------------------------------------------
//
void CRtpToFile::AddGroupL()
    {
    // New group
    iThisGroup = iNextGroupPoint;
    
    // Group header
    // Note ! KGroupHeaderBytes size is allocated to incoming group in 
    //        CCRRtpRecordSink::ResetGroupVariables(), but data does not exits
    //        before CRtpToFile::AddGroupHeaderL() method is called.
    iGroupTotalLen = KGroupHeaderBytes + iDataPtr.Length();
    iNextGroupPoint = iThisGroup + iGroupTotalLen;
    const TInt prevGroup( iLastSeekAddr );

    // Time shift handling
    if ( iGroupReUse > KErrNone )
        {
        iGroupReUse--;
        }
    else
        {
        iGroupsTotalCount++;
        iLastSeekAddr = ( iMode != EModeTimeShift )? iThisGroup: 0;
        }
    
    // Start write to the beginning of the clip?
    if ( iGroupReUse == KErrNone )
        {
        iGroupReUse = iGroupsTotalCount;
        iNextGroupPoint = iSeekHeaderPoint + KSeekHeaderBytes;
        LOG2( "CRtpToFile::AddGroupL(), iGroupReUse: %d, iNextGroupPoint: %d",
                                        iGroupReUse, iNextGroupPoint );
        }

    // First group in clip?
    if ( iGroupsTotalCount == 1 )
        {
        iPrevGroupPoint = 0;
        WriteSeekHeaderL();
        iSeekArrayReference = iGroupTime;
        }
    else
        {
        iPrevGroupPoint = prevGroup;
        }
    }
    
// -----------------------------------------------------------------------------
// CRtpToFile::GroupTimeL
// Generates group time from group length reported by ring buffer and actual
// network time difference to previous group. Reference time is used to avoid
// running time error caused by network burst.
// -----------------------------------------------------------------------------
//
void CRtpToFile::GroupTimeL( TUint& aGroupLength )
    {
    UpdateCurrentTimeL();
    TUint syncLength( 0 );
    
    // previous time initiated?
    if ( iPreviousTime > 0 )
        {
        const TInt64 delta( iCurrentTime.Int64() - iPreviousTime );
        iReferenceTime+= delta;
        const TInt timeDelta( delta / KSiKilo );
        const TInt burstDelta( Abs( timeDelta - iPreviousDelta ) );
#ifdef CR_ALL_LOGS    
        LOG3( "CRtpToFile::GroupTimeL(), aGroupLength: %u, burstDelta: %d, timeDelta: %d",
                                         aGroupLength, burstDelta, timeDelta );
#endif // CR_ALL_LOGS    
        
        // Use reference time?
        if ( timeDelta > KNormalRecGroupLength && 
             Abs( burstDelta - aGroupLength ) < KMaxValidDelta )
            {
            iPreviousDelta = 0;
            syncLength = iReferenceTime / KSiKilo;
            }
        else
            {
            iPreviousDelta = timeDelta;
            syncLength = aGroupLength;
            }
        }
    else
        {
        // In record start and after pause uses only the reported group length
        iPreviousDelta = 0;
        syncLength = aGroupLength;
        iReferenceTime+= aGroupLength * KSiKilo;
        }

    // Update group time
    iGroupTime += syncLength;
    iPreviousTime = iCurrentTime.Int64();
    
    // Time shift ongoing?
    if ( iMode == EModeTimeShift )
        {
        aGroupLength = syncLength;
        }
    else
        {
        // Update seek array
        aGroupLength = 0;
        if ( ( iGroupTime - iSeekArrayReference ) >= KSeekArrayInterval )
            {
            AppendSeekArrayL( iGroupTime, iThisGroup );
            iSeekArrayReference = iGroupTime;
            }
        }
    }
    
// -----------------------------------------------------------------------------
// CRtpToFile::WriteInitialMetaHeaderL
// Writes initial meta data header of clip.
// -----------------------------------------------------------------------------
//
void CRtpToFile::WriteInitialMetaHeaderL(
    const MRtpFileWriteObserver::SRtpRecParams& aParams )
    {
    LOG( "CRtpToFile::WriteInitialMetaHeaderL() in" );

    CRtpMetaHeader* metaheader = CRtpMetaHeader::NewLC(
                                 iFile, CRtpMetaHeader::EMetaWrite );
    // Attributes
    CRtpMetaHeader::SAttributes att;
    att.iOngoing = ETrue;
    att.iCompleted = EFalse;
    att.iProtected = EFalse;
    att.iFailed = EFalse;
    att.iVersion = KCurrentClipVersion;
    att.iQuality = KDummyFullQuality;
    att.iPostRule = aParams.iPostRule;
    att.iParental = aParams.iParental;
    att.iPlayCount = 0;
    att.iPlaySpot = KErrNone;
    metaheader->WriteAttributesL( att );
    LOG1( "CRtpToFile::WriteInitialMetaHeaderL(), iPostRule: %d", att.iPostRule );
    LOG1( "CRtpToFile::WriteInitialMetaHeaderL(), iParental: %d", att.iParental );
    
    // Start date/time
    metaheader->WriteStartTimeL( aParams.iStartTime );
    TName buf( KNullDesC );
#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    aParams.iStartTime.FormatL( buf, KTimeDateFormat );
    LOG1( "CRtpToFile::WriteInitialMetaHeaderL(), iStartTime: %S", &buf );
#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE

    // End time
    metaheader->WriteEndTimeL( aParams.iEndTime );

    // Duration
    metaheader->WriteDurationL( 0 );
    
    // Seek array point
    metaheader->WriteSeekArrayPointL( 0 );
    
    // Mime info
    CRtpUtil::GetMimeInfo( buf );
    metaheader->WriteUserIdL( buf );
    LOG1( "CRtpToFile::WriteInitialMetaHeaderL(), Mime: %S", &buf );

    // Device info
    CRtpUtil::GetImeiL( buf );
    metaheader->WriteDeviceInfoL( buf );
    LOG1( "CRtpToFile::WriteInitialMetaHeaderL(), IMEI: %S", &buf );

    // ESG info
    metaheader->WriteEsgDataL( aParams.iService, aParams.iProgram );
    LOG1( "CRtpToFile::WriteInitialMetaHeaderL(), Service: %S", 
                                                 &aParams.iService );
    LOG1( "CRtpToFile::WriteInitialMetaHeaderL(), Program: %S", 
                                                 &aParams.iProgram );
    // SRTP data ( Reserved for future use )
    TBuf8<3> srtp;
    srtp.Num( KErrNotFound ); 
    metaheader->WriteSrtpDataL( srtp );
    
    // SDP file
    metaheader->WriteSdpDataL( aParams.iSdpData );
    LOG1( "CRtpToFile::WriteInitialMetaHeaderL(), SDP length: %d", 
                                                  aParams.iSdpData.Length() );
    metaheader->CommitL();
    iSeekHeaderPoint = metaheader->SeekHeaderPoint();
    CleanupStack::PopAndDestroy( metaheader );

    LOG( "CRtpToFile::WriteInitialMetaHeaderL() out" );
    }
    
// -----------------------------------------------------------------------------
// CRtpToFile::WriteFinalMetaHeaderL
// Writes final meta data header of clip.
// -----------------------------------------------------------------------------
//
void CRtpToFile::WriteFinalMetaHeaderL( const TInt aStatus )
    {
    LOG( "CRtpToFile::WriteFinalMetaHeaderL() in" );
    CRtpMetaHeader* metaheader = CRtpMetaHeader::NewLC(
                                 iFile, CRtpMetaHeader::EMetaUpdate );
    // Update duration
    UpdateDurationL( metaheader );

    // Attributes
    CRtpMetaHeader::SAttributes att;
    metaheader->ReadAttributesL( att );
    att.iOngoing = EFalse;
    att.iCompleted = !aStatus;
    att.iFailed = !iGroupsTotalCount;
    metaheader->WriteAttributesL( att );
    LOG1( "CRtpToFile::WriteFinalMetaHeaderL(), Completed: %d", att.iCompleted );
    LOG1( "CRtpToFile::WriteFinalMetaHeaderL(), iFailed  : %d", att.iFailed );

    // End date/time
    metaheader->ReadStartTimeL( iCurrentTime );
    iRecordEndTime = iCurrentTime.Int64() + iGroupTime;
    metaheader->WriteEndTimeL( iRecordEndTime );
#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    TName buf( KNullDesC ); TTime( iRecordEndTime ).FormatL( buf, KTimeDateFormat );
    LOG1( "CRtpToFile::WriteFinalMetaHeaderL(), endTime: %S", &buf );
#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE

    // Seek array point
    metaheader->WriteSeekArrayPointL( iNextGroupPoint );
    LOG1( "CRtpToFile::WriteFinalMetaHeaderL(), Seek array: %d", iNextGroupPoint );
    CleanupStack::PopAndDestroy( metaheader );

    // Final seek header
    SaveSeekArrayL();
    WriteSeekHeaderL();
    
    // Set orginal start time as file date
    iFile.SetModified( iCurrentTime );

    LOG( "CRtpToFile::WriteFinalMetaHeaderL() out" );
    }
 
// -----------------------------------------------------------------------------
// CRtpToFile::AddGroupHeaderL
// Adds header of one RTP group.
// Room for group header bytes and packets count comes from CCRRtpRecordSink.
// -----------------------------------------------------------------------------
//
void CRtpToFile::AddGroupHeaderL()
    {
    // Packets count (PTC) is added in CCRRtpRecordSink::SaveGroup()

    // Group time
    HBufC8* bytes = CRtpUtil::MakeBytesLC( iGroupTime );
    iDataPtr.Insert( 0, bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );

    // Previous group point
    bytes = CRtpUtil::MakeBytesLC( iPrevGroupPoint );
    iDataPtr.Insert( 0, bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );

    // Next Group point
    bytes = CRtpUtil::MakeBytesLC( iNextGroupPoint );
    iDataPtr.Insert( 0, bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );

    // Group total size
    bytes = CRtpUtil::MakeBytesLC( iGroupTotalLen );
    iDataPtr.Insert( 0, bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );
    }
 
// -----------------------------------------------------------------------------
// CRtpToFile::UpdateDurationL
// Updates clip's duration.
// -----------------------------------------------------------------------------
//
void CRtpToFile::UpdateDurationL( CRtpMetaHeader* aMetaHeader )
    {
    aMetaHeader->WriteDurationL( TInt( iGroupTime ) );
    LOG1( "CRtpToFile::UpdateDurationL(), new duration: %u", iGroupTime );
    }
    
// -----------------------------------------------------------------------------
// CRtpToFile::UpdateCurrentTimeL
// Gets current time as network time.
// -----------------------------------------------------------------------------
//
void CRtpToFile::UpdateCurrentTimeL()
    {
    iCurrentTime.UniversalTime();
    }

// End of File

