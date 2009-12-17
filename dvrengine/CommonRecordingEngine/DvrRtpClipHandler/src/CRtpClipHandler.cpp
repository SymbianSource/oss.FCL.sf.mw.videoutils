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
* Description:    Implementation of RTP clip handler class.*
*/




// INCLUDE FILES
#include <ipvideo/CRtpClipHandler.h>
#include "CRtpToFile.h"
#include "CRtpFromFile.h"
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KDvrMaxTimeshiftDelta( 1 * 60 * 60 ); //  1 hour

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpClipHandler::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpClipHandler* CRtpClipHandler::NewL()
    {
    CRtpClipHandler* self = new( ELeave ) CRtpClipHandler();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::CRtpClipHandler
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CRtpClipHandler::CRtpClipHandler()
  : iClipVersion( 0 ),
    iSaveNameIndex( KErrNotFound ),
    iReadNameIndex( KErrNotFound ),
    iTsPauseState( EFalse )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CRtpClipHandler::ConstructL()
    {
    LOG( "CRtpClipHandler::ConstructL()" );
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::~CRtpClipHandler
// Destructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpClipHandler::~CRtpClipHandler()
    {
    LOG( "CRtpClipHandler::~CRtpClipHandler() in" );
    
    StopRecording( KErrCancel );
    StopPlayBack( KErrCancel, 0 );

    LOG( "CRtpClipHandler::~CRtpClipHandler() out" );
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::RegisterWriteObserver
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::RegisterWriteObserver(
    MRtpFileWriteObserver* aObs )
    {
    LOG1( "CRtpClipHandler::RegisterWriteObserver(), aObs: %d", aObs );
    iWriteObs = aObs;
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::RegisterReadObserver
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::RegisterReadObserver( MRtpFileReadObserver* aObs )
    {
    LOG1( "CRtpClipHandler::RegisterReadObserver(), aObs: %d", aObs );

    iReadObs = aObs;
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::StartRecordingL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::StartRecordingL(
    const MRtpFileWriteObserver::SRtpRecParams& aRecParams,
    const MRtpFileWriteObserver::TRtpSaveAction& aAction )
    {
    LOG1( "CRtpClipHandler::StartRecordingL() in, aAction: %d", aAction );
    
    // Stop possible existing recording
    StopRecording( KErrNone );
    iSaveNameIndex = 0;
    
    // If InitRtpSaveL leaves, iRtpSave is not NULL, but is in undefined state.
    // This causes problems - may crash when StopRecording is called.
    // Better to trap and cleanup here.
    TRAPD( err,
         // Init recording ( iRtpSave deleted in StopRecording() )
         iRtpSave = CRtpToFile::NewL( *this, *iWriteObs );
         iRtpSave->InitRtpSaveL( aRecParams, aAction );
         );
    if ( err )
        {
        delete iRtpSave; iRtpSave = NULL;
        User::Leave( err );
        }
    
    LOG( "CRtpClipHandler::StartRecordingL() out" );
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::SaveNextGroupL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::SaveNextGroupL(
    TPtr8& aGroup,
    TUint aGroupLength,
    const MRtpFileWriteObserver::TRtpSaveAction& aAction )
    {
    User::LeaveIfNull( iWriteObs );
    User::LeaveIfNull( iRtpSave );
    
    // Passes save action to ring buffer
    const TUint seekPoint( iRtpSave->SaveNextGroupL( 
                           aGroup, aGroupLength, aAction ) ); 
    
    // aGroupLength is set to non zero if time shift ongoing?
    if ( aGroupLength > 0 )
        {
        CRtpFileBase::STimeShiftSeek shiftSeek;
        shiftSeek.iGroupTime = aGroupLength;
        shiftSeek.iSeekpoint = seekPoint;
        shiftSeek.iNameIndex = iSaveNameIndex;
        iShiftSeek.Append( shiftSeek );
        
        // Time shift max length
        if ( ( ( iShiftSeek.Count() - 1 ) *  KNormalRecGroupLength ) > 
                                             KDvrMaxTimeshiftDelta )
            {
            LOG( "CRtpClipHandler::SaveNextGroupL(), Timeshift max time !" );
            }
        }
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::WritingActive
//
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CRtpClipHandler::WritingActive( void ) const
    {
    if ( iRtpSave )
        {
        return iRtpSave->IsActive();
        }
    
    return EFalse;
    }
    
// -----------------------------------------------------------------------------
// CRtpClipHandler::GetCurrentLength
//
// -----------------------------------------------------------------------------
//
EXPORT_C TUint CRtpClipHandler::GetCurrentLength( void ) const
    {
    TUint length( 0 );
    if ( iRtpRead )
        {
        length = iRtpRead->Duration();
        }
    else
        {
        if ( iRtpSave )
            {
            const TInt count( iShiftSeek.Count() );
            if ( count > 0 )
                {
                // Time shift
                for ( TInt i( 0 ); i < count; i++ )
                    {
                    length += iShiftSeek[i].iGroupTime;
                    }
                }
            else
                {
                // Rec ongoing
                length = iRtpSave->GetCurrentLength();
                }
            }
        }

    return length;
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::TimeShiftPauseL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::TimeShiftPauseL()
    {
    LOG2( "CRtpClipHandler::TimeShiftPauseL(), iRtpSave: %d, iTsPauseState: %d", 
                                               iRtpSave, iTsPauseState ); 
    User::LeaveIfNull( iRtpSave );
    if ( !iTsPauseState )
        {
        // Use clip as a ring buffer
        iRtpSave->ActivateGroupsReuseL();
        iTsPauseState = ETrue;
        }
    else
        {
        // Switch to next file
        iSaveNameIndex++;
        MRtpFileWriteObserver::SRtpRecParams recParams;
        DefaultRecParams( recParams, iSaveNameIndex );
        iRtpSave->SwapClipL( recParams );
        iTsPauseState = EFalse;
        }
    }
    
// -----------------------------------------------------------------------------
// CRtpClipHandler::DefaultRecParams
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::DefaultRecParams(
    MRtpFileWriteObserver::SRtpRecParams& aParams,
    const TInt aIndex )
    {
    aParams.iClipPath = KDvrTimeShiftFile;
    aParams.iClipPath.AppendNum( aIndex );
    aParams.iSdpData.Set( NULL, 0 );
    aParams.iService.Set( KNullDesC );
    aParams.iProgram.Set( KNullDesC );
    aParams.iPostRule = 0;
    aParams.iParental = 0;
    aParams.iStartTime = 0;
    aParams.iEndTime = TInt64( KDvrMaximumTimeShift ) * 1e6;
    }
    
// -----------------------------------------------------------------------------
// CRtpClipHandler::UpdateRecordEndTime
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::UpdateRecordEndTime( const TTime& aEndTime ) 
    {
    if ( iRtpSave )
        {
        iRtpSave->UpdateRecordEndTime( aEndTime );
        }
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::ResumeRecordingL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::ResumeRecordingL( void )
    {
    LOG1( "CRtpClipHandler::ResumeRecordingL(), iRtpSave: %d", iRtpSave ); 
    
    if ( iRtpSave )
        {
        iRtpSave->UpdatePreviousTimeL();
        }
    }
    
// -----------------------------------------------------------------------------
// CRtpClipHandler::StopRecording
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::StopRecording( const TInt aError )
    {
    LOG2( "CRtpClipHandler::StopRecording(), aError: %d, iRtpSave: %d",
                                             aError, iRtpSave );
    
    if ( iRtpSave )
        {
        // Update clip end point if watching sametime
        if ( iRtpRead )
            {
            iRtpRead->UpdateLastSeekAddr();
            }
        
        // Stop recording
        iRtpSave->StopRtpSave( aError );

        // Delete possible time shift files
        if ( !iRtpRead )
            {
            iRtpSave->DeleteTimeShiftFiles( iShiftSeek );
            }
        }
    
    delete iRtpSave; iRtpSave = NULL;
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::StartPlayBackL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::StartPlayBackL(
    const SCRRtpPlayParams& aParams,
    const TBool aTimeShift )
    {
    LOG1( "CRtpClipHandler::StartPlayBackL(), path: %S", &aParams.iFileName );
    LOG1( "CRtpClipHandler::StartPlayBackL(), aTimeShift: %d", aTimeShift );

    // Stop possible existing
    StopPlayBack( KErrNone, 0 );
    
    // Time shift mode?
    if ( aTimeShift )
        {
        // Open file for time shift mode
        User::LeaveIfNull( iRtpSave );
        iRtpRead = CRtpFromFile::NewL( *iReadObs, iRtpSave );
        iReadNameIndex = ( iShiftSeek.Count() )? iShiftSeek[0].iNameIndex: 0;
        TPath clipPath( KDvrTimeShiftFile );
        clipPath.AppendNum( iReadNameIndex );
        iRtpRead->InitRtpReadL( clipPath, iClipVersion, aTimeShift );
        }
    else
        {
        // Open file, during recording?
        if ( iRtpSave && iRtpSave->ClipPath() )
            {
            if ( !aParams.iFileName.Compare( iRtpSave->ClipPath()->Des() ) )
                {
                iRtpRead = CRtpFromFile::NewL( *iReadObs, iRtpSave );
                }
            }
        
        // Open file, normal playback?
        if ( !iRtpRead )
            {
            iRtpRead = CRtpFromFile::NewL( *iReadObs, NULL );
            }
        
        iRtpRead->InitRtpReadL( aParams.iFileName, iClipVersion, EFalse );
        }
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::StartPlayBackL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::StartPlayBackL( const RFile& aFileHandle )
    {
    LOG( "CRtpClipHandler::StartPlayBackL(), with handle" );

    // Stop possible existing
    StopPlayBack( KErrNone, 0 );
    
    // Open file, during recording?
    if ( iRtpSave && iRtpSave->ClipPath() )
        {
        TPath name( KNullDesC );
        aFileHandle.FullName( name );
        if ( !name.Compare( iRtpSave->ClipPath()->Des() ) )
            {
            iRtpRead = CRtpFromFile::NewL( *iReadObs, iRtpSave );
            }
        }
    
    // Open file, normal playback?
    if ( !iRtpRead )
        {
        iRtpRead = CRtpFromFile::NewL( *iReadObs, NULL );
        }
    
    iRtpRead->InitRtpReadL( aFileHandle, iClipVersion );
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::GetClipSdpL
//
// -----------------------------------------------------------------------------
//
EXPORT_C HBufC8* CRtpClipHandler::GetClipSdpL( void ) const
    {
    HBufC8* sdp = NULL;
    if ( iRtpRead )
        {
        sdp = iRtpRead->GetClipSdpL();
        }
    
    return sdp;
    }
    
// -----------------------------------------------------------------------------
// CRtpClipHandler::NextClipGroupL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::NextClipGroupL( void ) 
    {
    User::LeaveIfNull( iRtpRead );
    if ( !iRtpRead->IsTimeShift() )
        {
        iRtpRead->ReadNextGroupL( KErrNotFound );
        }
    else 
        {
        // Handle time shift
        TInt err( KErrNotFound );
        if ( iShiftSeek.Count() )
            {
            // Need swap file?
            SwapClipIfNeededL();

            // Read next group
            TRAP( err, iRtpRead->ReadNextGroupL( iShiftSeek[0].iSeekpoint ) );
            if ( err != KErrInUse  )
                {
                iShiftSeek.Remove( 0 );
                }
            }
        
        // End time shift?
        if ( err == KErrEof )
            {
            LOG( "CRtpClipHandler::NextClipGroupL(), Time shift play reached live !" );
            User::LeaveIfNull( iReadObs );
            iReadObs->ReadStatus( MRtpFileReadObserver::ERtpTimeShifTEnd );
            }
        }
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::SetSeekPointL
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::SetSeekPointL( const TUint aTime ) 
    {
    LOG1( "CRtpClipHandler::SetSeekPointL(), aTime: %u", aTime );

    User::LeaveIfNull( iRtpRead );
    if ( !iShiftSeek.Count() )
        {
        iRtpRead->SetSeekPointL( aTime );
        }
    else
        {
        const TInt count( iShiftSeek.Count() );
        if ( count > 0 )
            {
            TUint total( 0 );
            for ( TInt index( 0 ); index < count && aTime > total; index++ )
                {
                total += iShiftSeek[0].iGroupTime;
                SwapClipIfNeededL();
                iShiftSeek.Remove( 0 );
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::StopPlayBack
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipHandler::StopPlayBack(
    const TInt aError,
    const TUint aPlayerBuf )
    {
    LOG2( "CRtpClipHandler::StopPlayBack(), aError: %d, iRtpRead: %d",
                                            aError, iRtpRead );
    if ( iRtpRead )
        {
        iRtpRead->StopRtpRead( aError, aPlayerBuf );

        // Delete possible time shift files
        if ( !iRtpSave )
            {
            iRtpRead->DeleteTimeShiftFiles( iShiftSeek );
            }
        }

    delete iRtpRead; iRtpRead = NULL;
    }

// -----------------------------------------------------------------------------
// CRtpClipHandler::RtpGroupSaved
// Indicates that RPT packet save is ready.
// -----------------------------------------------------------------------------
//
void CRtpClipHandler::RtpGroupSaved( const TInt aAction ) 
    {
    // Inform file reader that new group saved
    if ( iRtpRead && aAction >= KErrNone )
        {
        iRtpRead->ReadSkippedGroup();
        }

    // Group saved
    if ( iWriteObs )
        {
        if ( aAction != MRtpFileWriteObserver::ESaveEnd )
            {
            // Ready for next group
            iWriteObs->GroupSaved();
            }
        else
            {
            // Recording full time
            iWriteObs->WriteStatus( KErrNone );
            }
        }
    }
    
// -----------------------------------------------------------------------------
// CRtpClipHandler::CurrentFileReadPoint
// Getter for file reader current point.
// -----------------------------------------------------------------------------
//
TInt CRtpClipHandler::CurrentFileReadPoint( const TInt aIndex )
    {
    if ( iShiftSeek.Count() > aIndex )
        {
        if ( iShiftSeek[aIndex].iNameIndex == iSaveNameIndex )
            {
            return iShiftSeek[aIndex].iSeekpoint;
            }
        
        return KErrNone;
        }

    return KErrNotFound;
    }
    
// -----------------------------------------------------------------------------
// CRtpClipHandler::SwapClipIfNeededL
// Swap to next available clip in time shift array if needed.
// -----------------------------------------------------------------------------
//
void CRtpClipHandler::SwapClipIfNeededL( void )
    {
    if ( iShiftSeek[0].iNameIndex != iReadNameIndex )
        {
        iReadNameIndex = iShiftSeek[0].iNameIndex;
        TPath clipPath( KDvrTimeShiftFile );
        clipPath.AppendNum( iReadNameIndex );
        iRtpRead->SwapClipL( clipPath );
        }
    }
    
//  End of File
