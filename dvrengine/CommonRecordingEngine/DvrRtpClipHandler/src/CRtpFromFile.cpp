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
* Description:    Implementation of the DVB-H Recording Manager RTP read class.*
*/




// INCLUDE FILES
#include "CRtpToFile.h"
#include "CRtpFromFile.h"
#include <ipvideo/CRtpMetaHeader.h>
#include <ipvideo/CRtpClipManager.h>
#include "CRtpTimer.h"
#include <bsp.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KReadTimerInterval( 1000 );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpFromFile::NewL
// Static two-phased constructor. Leaves object to cleanup stack.
// -----------------------------------------------------------------------------
//
CRtpFromFile* CRtpFromFile::NewL( 
    MRtpFileReadObserver& aReadObs,
    CRtpToFile* aToFile )
    {
    CRtpFromFile* self = new( ELeave ) CRtpFromFile( aReadObs, aToFile );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::CRtpFromFile
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CRtpFromFile::CRtpFromFile( MRtpFileReadObserver& aReadObs, CRtpToFile* aToFile )
  : CRtpFileBase(),
    iReadObs( aReadObs ),
    iToFile( aToFile ),
    iSkippedRead( EFalse ),
    iDuration( 0 )
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CRtpFromFile::ConstructL()
    {
    LOG( "CRtpFromFile::ConstructL()" );

    CRtpFileBase::ConstructL();
    }

// -----------------------------------------------------------------------------
// Destructor
//
CRtpFromFile::~CRtpFromFile()
// -----------------------------------------------------------------------------
    {
    LOG( "CRtpFromFile::~CRtpFromFile()" );

    Cancel();
    delete iTimer; iTimer = NULL;
    delete iFileData; iFileData = NULL;
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::InitRtpReadL
// Sets path of RTP file.
// -----------------------------------------------------------------------------
//
void CRtpFromFile::InitRtpReadL(
    const TDesC& aClipPath,
    TInt8& aVersion,
    const TBool aTimeShift )
    {
    LOG1( "CRtpFromFile::InitRtpReadL(), aClipPath: %S", &aClipPath );
    
    // File server
    if ( !iFs.Handle() )
        {
        User::LeaveIfError( iFs.Connect() );
        }

    // Open clip
    aVersion = SwapClipL( aClipPath );
    
    // Mode
    iMode = ( aTimeShift )? EModeTimeShift: EModeNormal;

#ifdef CR_ALL_LOGS
    LogVariables( _L( "InitRtpReadL()" ) );
#endif // CR_ALL_LOGS
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::InitRtpReadL
// Sets path of RTP file.
// -----------------------------------------------------------------------------
//
void CRtpFromFile::InitRtpReadL(
    const RFile& aFileHandle,
    TInt8& aVersion )
    {
    LOG( "CRtpFromFile::InitRtpReadL(), with handle" );
    
    // File handle
    if ( !iFs.Handle() )
        {
        User::LeaveIfError( iFs.Connect() );
        }
    
    // Duplicate handle
    iFile.Close();
    iFile.Duplicate( aFileHandle );
    
    // File header
    ReadClipHeaderL( aVersion );
    delete iCurrentPath; iCurrentPath = NULL;
    TFileName name( KNullDesC );
    iFile.FullName( name );
    iCurrentPath = name.AllocL();

    // Mode
    iMode = EModeNormal;

#ifdef CR_ALL_LOGS
    LogVariables( _L( "InitRtpReadL()" ) );
#endif // CR_ALL_LOGS
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::SwapClipL
// Sets new path of RTP file.
// -----------------------------------------------------------------------------
//
TInt8 CRtpFromFile::SwapClipL( const TDesC& aClipPath )
    {
    LOG1( "CRtpFromFile::SwapClipL(), aClipPath: %S", &aClipPath );

    iFile.Close();
    if ( !iFs.Handle() )
        {
        User::Leave( KErrBadHandle );
        }
        
    // Delete used clip
    if ( iMode == EModeTimeShift )
        {
        iFs.Delete( *iCurrentPath );
        }
    
    // Open new
    User::LeaveIfError( iFile.Open( iFs, aClipPath,
                        EFileShareAny | EFileStream | EFileRead ) );
    // File header
    TInt8 version( 0 );
    ReadClipHeaderL( version );
    delete iCurrentPath; iCurrentPath = NULL;
    iCurrentPath = aClipPath.AllocL();

    return version;
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::ReadNextGroupL
// Reads next RTP packets group from a specified file.
// -----------------------------------------------------------------------------
//
TInt CRtpFromFile::ReadNextGroupL( const TInt aGroupPoint )
    {
    User::LeaveIfError( iMode );
    TBool delayedRead( EFalse );

    // Allready active??
    if ( iFileData )
        {
        if ( iTimer || IsActive() )
            {
            return KErrInUse; // Read already started, indication, not error
            }
        else
            {
            // Packet read may happen during iReadObs.RtpGroupReaded() call
            LOG( "CRtpFromFile::ReadNextGroupL(), Delayed read !" );
            delayedRead = ETrue;
            }
        }
    
    // Is watch during recording too close to live?
    if ( iToFile && iNextGroupPoint >= LastSeekAddr() )
        {
        iSkippedRead = ETrue;
        LOG( "CRtpFromFile::ReadNextGroupL(), Too close to live !" );
        return KErrEof; // No read actions now, indication, not error
        }

    // Group
    iThisGroup = ( aGroupPoint > KErrNotFound )? aGroupPoint: iNextGroupPoint;

    // Ok to read more?
    if ( iThisGroup > iLastSeekAddr || iGroupTime >= iDuration )
        {
        LOG( "CRtpFromFile::ReadNextGroupL(), All packets readed !" );
        User::Leave( KErrEof );
        }

    // Read group
    if ( delayedRead )
        {
        delete iTimer; iTimer = NULL;
        iTimer = CRtpTimer::NewL( *this );
        iTimer->After( KReadTimerInterval );
        }
    else
        {
        ReadGroupHeaderL();
        ReadNextGroupFromFileL();
        }
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::GetClipSdpL
// Reads SDP from a current clip. SDP is stored to meta header during recording.
// -----------------------------------------------------------------------------
//
HBufC8* CRtpFromFile::GetClipSdpL()
    {
    User::LeaveIfError( iMode );
    CRtpMetaHeader* metaheader = CRtpMetaHeader::NewLC(
                                 iFile, CRtpMetaHeader::EMetaRead );
    HBufC8* sdp = metaheader->ReadSdpDataL();
    CleanupStack::PopAndDestroy( metaheader );
    return sdp;
    }
    
// -----------------------------------------------------------------------------
// CRtpFromFile::ReadSkippedGroup
// Reads one RTP packet from a specified file if previous read was skipped.
// -----------------------------------------------------------------------------
//
void CRtpFromFile::ReadSkippedGroup()
    {
    if ( iSkippedRead && iNextGroupPoint < LastSeekAddr() &&
         iMode != EModeNone && iFileData != NULL )
        {
        iSkippedRead = EFalse;
        iThisGroup = iNextGroupPoint;
        TRAP_IGNORE( ReadGroupHeaderL();
                     ReadNextGroupFromFileL() );
        }
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::UpdateLastSeekAddr
// Updates final last seek addres from clip write when recording stopped.
// -----------------------------------------------------------------------------
//
void CRtpFromFile::UpdateLastSeekAddr()
    {
    if ( iToFile )
        {
        iDuration = iToFile->GetCurrentLength();
        iLastSeekAddr = iToFile->LastSeekAddr();
    
        LOG2( "CRtpFromFile::UpdateLastSeekAddr(), iLastSeekAddr: %d, iDuration: %d",
                                                   iLastSeekAddr, iDuration ); 
        // Recording is stopped
        iToFile = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::SetSeekPointL
// Sets the seek point of the clip.
// -----------------------------------------------------------------------------
//
void CRtpFromFile::SetSeekPointL( const TUint aTime )
    {
    Cancel();
    delete iTimer; iTimer = NULL;
    delete iFileData; iFileData = NULL;
    User::LeaveIfError( iMode );
    
    // Group from the seek array, accuracy 30s
    iThisGroup = FindSeekGroup( aTime, ( iToFile )? iToFile->SeekArray(): iSeekArray );
    LOG2( "CRtpFromFile::SetSeekPointL(), aTime: %d, group from seek array: %d", 
                                          aTime, iThisGroup );
    if ( iThisGroup == KErrNotFound )
        {
        iThisGroup = iFirstSeekAddr;
        }
    ReadGroupHeaderL();
    
    // Find group basing on the seek time, accuracy 0 - 3 s
    if ( aTime > 0 )
        {
        while ( aTime > iGroupTime && iNextGroupPoint < iLastSeekAddr )
            {
            // Next group
            iThisGroup = iNextGroupPoint;
            ReadGroupHeaderL();
#ifdef CR_ALL_LOGS
            LOG2( "CRtpFromFile::SetSeekPointL(), iThisGroup: %u, iGroupTime: %u", 
                                                  iThisGroup, iGroupTime );
#endif // CR_ALL_LOGS
            }
        }
    
    // Prepare for next read, one extra group back looks better
    iNextGroupPoint = ( iPrevGroupPoint > iFirstSeekAddr ) ?
        iPrevGroupPoint : iThisGroup;
    delete iFileData; iFileData = NULL;
        
    LOG1( "CRtpFromFile::SetSeekPointL(), iNextGroupPoint: %d",
                                          iNextGroupPoint );
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::StopRtpRead
// Stops file reading.
// -----------------------------------------------------------------------------
//
void CRtpFromFile::StopRtpRead( const TInt aStatus, const TUint aPlayerBuf )
    {
    LOG2( "CRtpFromFile::StopRtpRead(), aStatus: %d, aPlayerBuf: %u",
                                        aStatus, aPlayerBuf );
    LOG2( "CRtpFromFile::StopRtpRead(), iMode: %d, iGroupTime: %d",
                                        iMode, iGroupTime );
    Cancel();
    if ( iMode != EModeNone )
        {
        iFile.ReadCancel();

#ifdef CR_ALL_LOGS
        LogVariables( _L( "StopRtpRead()" ) );
#endif // CR_ALL_LOGS
        }
    
    delete iTimer; iTimer = NULL;
    delete iFileData; iFileData = NULL;
    if ( iMode == EModeNormal || iMode == EModeHandle )
        {
        // Try to seek back to what user sees for continue play spot
        if ( !aStatus & iThisGroup > 0 && iThisGroup < iLastSeekAddr )
            {
            const TInt thisGroup( iThisGroup );
            TRAPD( err, SetSeekPointL( iGroupTime - aPlayerBuf ) );
            if ( err )
                {
                LOG1( "CRtpFromFile::StopRtpRead(), SetSeekPointL Leaved: %d", err ); 
                iThisGroup = thisGroup;
                }
            }
        
        // Update meta header if no error
        if ( !aStatus )
            {
            TInt err ( KErrNone );
            if ( iToFile )
                {
                TRAP( err, iToFile->UpdatePlayAttL( iThisGroup ) );
                }
            else
                {
                TRAP( err, UpdatePlayAttL() );
                }

            // Possible error ignored
            if ( err )
                {
                LOG1( "CRtpFromFile::StopRtpRead(), UpdatePlayAttL Leaved: %d", err ); 
                }
            }
        }

    iMode = EModeNone;
    iFile.Close();
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::RunL
// -----------------------------------------------------------------------------
//
void CRtpFromFile::RunL()
    {
    User::LeaveIfError( iStatus.Int() );

    // All groups readed?
    if ( iThisGroup >= iLastSeekAddr ||
       ( iNextGroupPoint >= iLastSeekAddr &&
         iToFile && iToFile->Action() == MRtpFileWriteObserver::ESavePause ) )
        {
        LOG2( "CRtpFromFile::RunL(), All groups readed ! total: %d, iDuration: %d",
                                                         iThisGroup, iDuration ); 
        iGroupTime = iDuration;
        }
    
    iReadObs.GroupReadedL( iDataPtr, iGroupTime, ( iGroupTime >= iDuration ) );
    delete iFileData; iFileData = NULL;
    }
    
// -----------------------------------------------------------------------------
// CRtpFromFile::RunError
// -----------------------------------------------------------------------------
//
TInt CRtpFromFile::RunError( TInt aError )
    {
    LOG1( "CRtpFromFile::RunError(), RunL Leaved: %d", aError );

    iReadObs.ReadStatus( aError );
    StopRtpRead( aError, 0 );
    return KErrNone;
    }
 
// -----------------------------------------------------------------------------
// CRtpFromFile::DoCancel
// -----------------------------------------------------------------------------
//
void CRtpFromFile::DoCancel()
    {
    LOG( "CRtpFromFile::DoCancel()" );
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::TimerEventL
// Internal timer call this when triggered.
// -----------------------------------------------------------------------------
//
void CRtpFromFile::TimerEventL()
    {
    LOG( "CRtpFromFile::TimerEventL() in" );

    ReadGroupHeaderL();
    ReadNextGroupFromFileL();
    delete iTimer; iTimer = NULL;

    LOG( "CRtpFromFile::TimerEventL() out" );
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::TimerError
// Internal timer call this when TimerEventL() leaves.
// -----------------------------------------------------------------------------
//
void CRtpFromFile::TimerError( const TInt aError )
    {
    LOG1( "CRtpFromFile::TimerError(), TimerEventL Leaved: %d", aError );

    StopRtpRead( aError, 0 );
    delete iTimer; iTimer = NULL;
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::ReadClipHeaderL
// Reads meta data and seek header from the beginning of the file.
// -----------------------------------------------------------------------------
//
void CRtpFromFile::ReadClipHeaderL( TInt8& aVersion )
    {
    LOG1( "CRtpFromFile::ReadClipHeaderL(), iToFile: %d", iToFile );

    if ( !iToFile )
        {
        TInt seekArrayPoint( KErrNotFound );
        aVersion = ReadMetaHeaderL( iSeekHeaderPoint, seekArrayPoint );
        ReadSeekHeaderL();
        
        // Read seek array if exist
        if ( seekArrayPoint > iLastSeekAddr )
            {
            ReadSeekArrayL( seekArrayPoint );
            }
        }
    else // Recording ongoing with the same clip
        {
        aVersion = KCurrentClipVersion;
        iSeekHeaderPoint = iToFile->SeekHeaderPoint();
        iGroupsTotalCount = iToFile->GroupsTotalCount();
        iFirstSeekAddr = iToFile->FirstSeekAddr();
        iLastSeekAddr = iToFile->LastSeekAddr();
        }

    iNextGroupPoint = iFirstSeekAddr;
    }
    
// -----------------------------------------------------------------------------
// CRtpFromFile::ReadMetaHeaderL
// Reads meta data header from the beginning of the file.
// -----------------------------------------------------------------------------
//
TInt8 CRtpFromFile::ReadMetaHeaderL(
    TInt& aSeekHeaderPoint,
    TInt& aSeekArrayPoint )
    {
    CRtpMetaHeader* metaheader = CRtpMetaHeader::NewLC(
                                 iFile, CRtpMetaHeader::EMetaRead );
    aSeekHeaderPoint = metaheader->SeekHeaderPoint();
    metaheader->ReadSeekArrayPointL( aSeekArrayPoint );
    LOG2( "CRtpFromFile::ReadMetaHeaderL(), aSeekHeaderPoint: %d, aSeekArrayPoint: %d",
                                            aSeekHeaderPoint, aSeekArrayPoint );
    // Clip version
    CRtpMetaHeader::SAttributes att;
    metaheader->ReadAttributesL( att );
    metaheader->ReadDurationL( iDuration );
    
    // Verify post rule
    CRtpClipManager* clipManager = CRtpClipManager::NewLC();
    clipManager->VerifyPostRuleL( att.iPostRule, metaheader );
    CleanupStack::PopAndDestroy( clipManager );
    CleanupStack::PopAndDestroy( metaheader );
    
    LOG2( "CRtpFromFile::ReadMetaHeaderL(), Version: %d, Duration: %d",
                                            att.iVersion, iDuration );
    return att.iVersion;
    }
    
// -----------------------------------------------------------------------------
// CRtpFromFile::ReadNextGroupFromFileL
// Reads RTP payload from a file.
// Payload is allways after data header, so read position set is not needed.
// -----------------------------------------------------------------------------
//
void CRtpFromFile::ReadNextGroupFromFileL()
    {
    LOG2( "CRtpFromFile::ReadNextGroupFromFileL(), iThisGroup: %d, iGroupTime: %u", 
                                                   iThisGroup, iGroupTime  );
#ifdef CR_ALL_LOGS
    LogVariables( _L( "ReadNextGroupFromFileL()" ) );
#endif // CR_ALL_LOGS

    const TInt len( iGroupTotalLen - KGroupHeaderBytes );
    if ( len <= 0 || iThisGroup < iFirstSeekAddr || iThisGroup > iLastSeekAddr )
        {
#ifdef CR_ALL_LOGS
        LogVariables( _L( "ReadNextGroupFromFileL()" ) );
#endif // CR_ALL_LOGS

        LOG( "CRtpFromFile::ReadNextGroupFromFileL(), No More Groups" );
        User::Leave( KErrEof );
        }

    // Reading should never be active at this point
    if ( iFileData != NULL )
        {
        LOG( "CRtpFromFile::ReadNextGroupFromFileL(), Invalid usage of class !" );
        User::Leave( KErrGeneral );
        }

    // Start reading group
    iFileData = HBufC8::NewL( len );
    iDataPtr.Set( iFileData->Des() );
    iFile.Read( iThisGroup + KGroupHeaderBytes, iDataPtr, len, iStatus );
    SetActive();
    }
    
// -----------------------------------------------------------------------------
// CRtpFromFile::FindSeekGroup
// Finds closes point with seek array, accuracy about 0 - 30 s.
// -----------------------------------------------------------------------------
//
TInt CRtpFromFile::FindSeekGroup( const TUint aTime, CArrayFix<SSeek>* aArray )
    {
    if ( aArray->Count() && aTime >= aArray->At( 0 ).iTime )
        {
        for ( TInt i( aArray->Count() - 1 ); i > 0 ; i-- )
            {
#ifdef CR_ALL_LOGS
            LOG3( "CRtpFromFile::FindSeekGroup(), ind: %d, aTime: %u, array time: %u", 
                                                  i, aTime, aArray->At( i ).iTime );
#endif //CR_ALL_LOGS

            if ( aTime > aArray->At( i ).iTime )
                {
                return aArray->At( i ).iPoint;
                }
            }
        
        return aArray->At( 0 ).iPoint;
        }
    
    return KErrNotFound;
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::UpdatePlayAttL
// Updates clip's playback count and spot attributes after watching.
// -----------------------------------------------------------------------------
//
void CRtpFromFile::UpdatePlayAttL()
    {
    // Update attributes
    if ( iMode == EModeNormal )
        {
        iFile.Close();
        User::LeaveIfError( iFile.Open( iFs, *iCurrentPath,
                            EFileShareAny | EFileStream | EFileWrite ) );
        }

    CRtpMetaHeader* metaheader = CRtpMetaHeader::NewLC(
                                 iFile, CRtpMetaHeader::EMetaUpdate );
    TTime startTime( 0 );
    metaheader->ReadStartTimeL( startTime );

    CRtpMetaHeader::SAttributes att;
    metaheader->ReadAttributesL( att );
    
    // Step playback counter by one
    att.iPlayCount++;
    // Update play spot
    att.iPlaySpot = ( iThisGroup > 0 && iThisGroup < iLastSeekAddr )? iThisGroup:
                                                                      KErrNone;
    metaheader->WriteAttributesL( att );
    CleanupStack::PopAndDestroy( metaheader );
    LOG2( "CRtpFromFile::UpdatePlayAttL(), New play count: %d, spot: %d", 
                                           att.iPlayCount, att.iPlaySpot );
    // Set start time to file date
    iFile.SetModified( startTime );
    }
    
// -----------------------------------------------------------------------------
// CRtpFromFile::LastSeekAddr
// Gets last seek addres.
// It is either from opened clip or from file writer when recording is ongoing.
// -----------------------------------------------------------------------------
//
TInt CRtpFromFile::LastSeekAddr()
    {
    if ( iToFile )
        {
        iLastSeekAddr = iToFile->LastSeekAddr();
        }
    
    return iLastSeekAddr;
    }

// End of File

