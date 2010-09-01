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
* Description:    Help methods for propriatary RTP format.*
*/




// INCLUDE FILES
#include <pathinfo.h>
#include <ipvideo/CRtpClipManager.h>
#include "CRtpClipRepairer.h"
#include <BaUtils.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt EContentRightsRecordingAllowed( 0 );
const TInt EContentRightsLockToDevice( 2 );
const TInt KMaxProgramChars( 8 );
const TInt KFirstFileIndex( 1 );
const TInt KMaxFileIndex( 99 );
_LIT( KDvrClipExtension, ".rtp" );
_LIT( KIndexFormat, "(%02d)" );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpClipManager::NewL
// Static two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpClipManager* CRtpClipManager::NewL()
    {
    CRtpClipManager* self = CRtpClipManager::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CRtpClipManager::NewLC
// Static two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpClipManager* CRtpClipManager::NewLC()
    {
    CRtpClipManager* self = new( ELeave ) CRtpClipManager();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CRtpClipManager::CRtpClipManager
//
// -----------------------------------------------------------------------------
//
CRtpClipManager::CRtpClipManager()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CRtpClipManager::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CRtpClipManager::ConstructL()
    {
    LOG( "CRtpClipManager::ConstructL() in" );
    
    // IMEI
    TName buf( KNullDesC );
    CRtpUtil::GetImeiL( buf );
    iImei = buf.AllocL();
    LOG1( "CRtpClipManager::ConstructL(), IMEI: %S", &*iImei );

    // File server
    User::LeaveIfError( iFs.Connect() );

    LOG( "CRtpClipManager::ConstructL() out" );
    }

// -----------------------------------------------------------------------------
// CRtpClipManager::~CRtpClipManager
//
// -----------------------------------------------------------------------------
//
EXPORT_C CRtpClipManager::~CRtpClipManager()
    {
    LOG( "CRtpClipManager::~CRtpClipManager()" );

    delete iImei;
    iRepairQueue.ResetAndDestroy();
    delete iClipRepairer;
    iFile.Close();
    iFs.Close();
    }

// -----------------------------------------------------------------------------
// CRtpClipManager::GetClipDetailsL
// Getter for clip details.
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipManager::GetClipDetailsL(
    const TDesC& aClipPath,
    SRtpClipDetails& aDetails )
    {
    iFile.Close();
    TBool clipOpen( EFalse );
    iFs.IsFileOpen( aClipPath, clipOpen );
    User::LeaveIfError( iFile.Open( iFs, aClipPath,
                        EFileShareAny | EFileStream | EFileRead ) );
    CRtpMetaHeader* metaheader = CRtpMetaHeader::NewLC( 
                                 iFile, CRtpMetaHeader::EMetaRead );
    // Attributes
    CRtpMetaHeader::SAttributes att;
    metaheader->ReadAttributesL( att );

    // Under version 2 clips not work any longer
    // Recording ongoing without file open indicates interrupted recording
    if ( att.iVersion < 2 || ( att.iOngoing && !clipOpen ) )
        {
        LOG3( "CRtpClipManager::GetClipDetailsL(), iVersion: %d, iOngoing: %d, clipOpen: %d",
                                                   att.iVersion, att.iOngoing, clipOpen );
        iFile.Close();
        User::Leave( KErrGeneral ); 
        }
    
    // Details
    GetDetailsL( att, aDetails, metaheader );
    CleanupStack::PopAndDestroy( metaheader );
    iFile.Close();
    }

// -----------------------------------------------------------------------------
// CRtpClipManager::GetClipDetailsL
// Getter for clip details.
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipManager::GetClipDetailsL(
    RFile& aFile,
    SRtpClipDetails& aDetails )
    {
    CRtpMetaHeader* metaheader = CRtpMetaHeader::NewLC( 
                                 aFile, CRtpMetaHeader::EMetaRead );
    // Attributes
    CRtpMetaHeader::SAttributes att;
    metaheader->ReadAttributesL( att );
    
    // Details
    GetDetailsL( att, aDetails, metaheader );
    CleanupStack::PopAndDestroy( metaheader );
    }

// -----------------------------------------------------------------------------
// CRtpClipManager::ProtectClipL
// Protects clip from deleting automatically during recording.
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipManager::ProtectClipL(
    const TDesC& aClipPath,
    const TBool aProtected )
    {
    LOG1( "CRtpClipManager::ProtectClipL(), aClipPath: %S", &aClipPath );
    LOG1( "CRtpClipManager::ProtectClipL(), aProtected: %d", aProtected );
    
    iFile.Close();
    User::LeaveIfError( iFile.Open( iFs, aClipPath,
                        EFileShareExclusive | EFileStream | EFileWrite ) );
    CRtpMetaHeader* metaheader = CRtpMetaHeader::NewLC( 
                                 iFile, CRtpMetaHeader::EMetaUpdate );
    // Read Attributes
    CRtpMetaHeader::SAttributes att;
    metaheader->ReadAttributesL( att );
    // Update protected attribute
    if ( aProtected != att.iProtected )
        {
        att.iProtected = aProtected;
        metaheader->WriteAttributesL( att );
        }
    
    CleanupStack::PopAndDestroy( metaheader );
    iFile.Close();
    }
    
// -----------------------------------------------------------------------------
// CRtpClipManager::FixMetaHeaderL
// Fixes corrupted clip's meta header.
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipManager::FixMetaHeaderL(
    MRtpClipRepairObserver* aObs,
    const TDesC& aClipPath )
    {
    LOG( "RM-CRtpClipManager::FixMetaHeaderL()" );

    iFile.Close();
    if ( !iClipRepairer )
        {
        LOG1( "CRtpClipManager::FixMetaHeaderL(), Fix started: %S", &aClipPath );
        AddClipToRepairQueueL( aClipPath );
        iClipRepairer = CRtpClipRepairer::NewL( aObs );
        iClipRepairer->CheckMetaHeaderL( aClipPath );
        }
    else
        {
        // Verify that not exist in queue
        TInt loop( iRepairQueue.Count() - 1 );
        for ( ; loop >= 0; loop-- )
            {
            if ( !aClipPath.CompareC( iRepairQueue[loop]->Des() ) )
                {
                break;
                }
            }

        // Add to queue
        if ( loop < 0 )
            {
            LOG1( "CRtpClipManager::FixMetaHeaderL(), Fix queued: %S", &aClipPath );
            AddClipToRepairQueueL( aClipPath );
            }
        }
    }
    
// -----------------------------------------------------------------------------
// CRtpClipManager::FixMetaHeaderL
// Fixes corrupted clip's meta header syncronously.
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipManager::FixMetaHeaderL( const TDesC& aClipPath )
    {
    LOG1( "CRtpClipManager::FixMetaHeaderL(), aClipPath: %S", &aClipPath );

    iFile.Close();
    delete iClipRepairer; iClipRepairer = NULL;
    iClipRepairer = CRtpClipRepairer::NewL( NULL );
    iClipRepairer->CheckMetaHeaderL( aClipPath );
    }
    
// -----------------------------------------------------------------------------
// CRtpClipManager::DeleteRtpRepairer
// Kills clip repairer after work done.
// -----------------------------------------------------------------------------
//
EXPORT_C void CRtpClipManager::DeleteRtpRepairer( MRtpClipRepairObserver* aObs )
    {
    // Remove handled name from the queue first
    iFile.Close();
    TInt last( iRepairQueue.Count() - 1 );
    if ( last > KErrNotFound && iClipRepairer &&
         !iRepairQueue[last]->Des().Compare( iClipRepairer->CurrentClipName() ) )
        {
        delete iRepairQueue[last];
        iRepairQueue[last] = NULL;
        iRepairQueue.Remove( last );
        }

    // Repairer must be deleted in any case
    delete iClipRepairer; iClipRepairer = NULL;
    TInt err( KErrNotFound );
    last = iRepairQueue.Count() - 1;
    LOG1( "CRtpClipManager::DeleteRtpRepairer(), queue count: %d", iRepairQueue.Count() );
    
    while ( last > KErrNotFound && err )
        {
        // Create new repairer and start it
        TPath path( iRepairQueue[last]->Des() );
        TRAP( err, iClipRepairer = CRtpClipRepairer::NewL( aObs ) );
        if ( !err )
            {
            TRAP( err, iClipRepairer->CheckMetaHeaderL( path ) );
            if ( err )
                {
                LOG1( "CRtpClipManager::DeleteRtpRepairerL(), CheckMetaHeaderL Leaved: %d", err );

                // Remove clip which can't be repaired from the queue
                delete iRepairQueue[last];
                iRepairQueue[last] = NULL;
                iRepairQueue.Remove( last );
                
                // Ready for the next clip
                last = iRepairQueue.Count() - 1;
                delete iClipRepairer; iClipRepairer = NULL;
                }
            }
        else
            {
            LOG1( "CRtpClipManager::DeleteRtpRepairerL(), No memory for new repairer: %d", err );
            break;
            }
        }
    }
    
// -----------------------------------------------------------------------------
// CRtpClipManager::VerifyPostRuleL
// Verifies post acqusition rule of clip.
// -----------------------------------------------------------------------------
//
TInt CRtpClipManager::VerifyPostRuleL(
    const TUint8 aPostRule,
    CRtpMetaHeader* aMetaHeader )
    {
    LOG( "CRtpClipManager::VerifyPostRule()" );

    switch ( aPostRule )
        {
        case EContentRightsRecordingAllowed:
            LOG( "CRtpClipManager::VerifyPostRule(), EContentRightsRecordingAllowed !" );
            break;
        
        case EContentRightsLockToDevice:
            {
            TName imei( KNullDesC );
            aMetaHeader->ReadDeviceInfoL( imei );
            if ( !iImei || imei.Compare( iImei->Des() ) )
                {
                LOG( "CRtpClipManager::VerifyPostRule(), EContentRightsLockToDevice" );
                LOG1( "CRtpClipManager::VerifyPostRule(), ERmPlayDeviceLockError: %S", &imei );
                LOG1( "CRtpClipManager::VerifyPostRule(), Phone's IMEI: %S", &*iImei );
                return KErrAccessDenied;
                }
            }
            break;
        
        default:
            LOG1( "RM-CRtpClipManager::VerifyPostRule(), Default case: %d", aPostRule );
            break;
        }
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CRtpClipManager::GetDetailsL
// Updates details from meta header attributes.
// -----------------------------------------------------------------------------
//
void CRtpClipManager::GetDetailsL(
    const CRtpMetaHeader::SAttributes& aAttributes,
    SRtpClipDetails& aDetails,
    CRtpMetaHeader* aMetaHeader )
    {
    aDetails.iRecOngoing = aAttributes.iOngoing;
    aDetails.iCompleted = aAttributes.iCompleted;
    aDetails.iProtected = aAttributes.iProtected;
    aDetails.iFailed = aAttributes.iFailed;
    aDetails.iQuality = aAttributes.iQuality;
    aDetails.iPlayCount = aAttributes.iPlayCount;
    aDetails.iPlaySpot = aAttributes.iPlaySpot;
    aDetails.iParental = aAttributes.iParental;
    
    LOG1( "CRtpClipManager::GetDetailsL(), iRecOngoing: %d", aDetails.iRecOngoing );
    LOG1( "CRtpClipManager::GetDetailsL(), iCompleted: %d", aDetails.iCompleted );
    LOG1( "CRtpClipManager::GetDetailsL(), iProtected: %d", aDetails.iProtected );
    LOG1( "CRtpClipManager::GetDetailsL(), iFailed: %d", aDetails.iFailed );
    LOG1( "CRtpClipManager::GetDetailsL(), iQuality: %d", aDetails.iQuality );
    LOG1( "CRtpClipManager::GetDetailsL(), iPlayCount: %d", aDetails.iPlayCount );
    LOG1( "CRtpClipManager::GetDetailsL(), iPlaySpot: %d", aDetails.iPlaySpot );
    LOG1( "CRtpClipManager::GetDetailsL(), iParental: %d", aDetails.iParental );

    // ESG
    aMetaHeader->ReadEsgDataL( aDetails.iService, aDetails.iProgram );
    LOG1( "CRtpClipManager::GetDetailsL(), iService: %S", &aDetails.iService );
    LOG1( "CRtpClipManager::GetDetailsL(), iProgram: %S", &aDetails.iProgram );
    
    // Start time
    aMetaHeader->ReadStartTimeL( aDetails.iStartTime );
    
    // End time
    aMetaHeader->ReadEndTimeL( aDetails.iEndTime );

#if defined( LIVE_TV_RDEBUG_TRACE ) || defined( LIVE_TV_FILE_TRACE )
    TName time( KNullDesC ); aDetails.iEndTime.FormatL( time, KTimeDateFormat );
    LOG1( "CRtpClipManager::GetDetailsL(), End time: %S", &time );
#endif // LIVE_TV_RDEBUG_TRACE || LIVE_TV_FILE_TRACE

    // Duration
    aMetaHeader->ReadDurationL( aDetails.iDuration );
    LOG1( "CRtpClipManager::GetDetailsL(), iDuration: %d", aDetails.iDuration );
    aDetails.iDuration/= 1000; // convert to seconds

    // Post acquisition
    aDetails.iPostRuleOk = !VerifyPostRuleL( aAttributes.iPostRule, aMetaHeader );
    LOG1( "CRtpClipManager::GetDetailsL(), iPostRuleOk: %d", aDetails.iPostRuleOk );
    }

// -----------------------------------------------------------------------------
// CRtpClipManager::NewClipRootL
// Root path of the new clip, depends on user media setting.
// If memory card is selected, but not available, default saving to phone memory.
// -----------------------------------------------------------------------------
//
void CRtpClipManager::NewClipRootL( TDes& aClipPath, const TDriveNumber aDrive )
    {
    // Begin of the save path
    if ( aDrive == EDriveC )
        {
        aClipPath = PathInfo::PhoneMemoryRootPath();
        aClipPath.Append( PathInfo::VideosPath() );
        }
    else
        {
        aClipPath = PathInfo::MemoryCardRootPath();
        aClipPath.Append( PathInfo::VideosPath() );
        }
    
    // Verify and create path if not exist
    if ( !BaflUtils::PathExists( iFs, aClipPath ) )
        {
        TInt err( iFs.MkDirAll( aClipPath ) );
        if ( err && aDrive != EDriveC )
            {
            LOG1( "CRtpClipManager::NewClipRootL(), Forced to Use Phone Memory !", err );

            // Memorycard not acceptable -> Use phone memory
            err = KErrNone;
            aClipPath = PathInfo::PhoneMemoryRootPath();
            aClipPath.Append( PathInfo::VideosPath() );
            BaflUtils::EnsurePathExistsL( iFs, aClipPath );
            }
        
        User::LeaveIfError( err );
        }
    }

// -----------------------------------------------------------------------------
// CRtpClipManager::NewIndexNameL
// Creates new clip name from program name (eigth first letters + index).
// If program name allready in use, adds indexing to the end of name.
// -----------------------------------------------------------------------------
//
void CRtpClipManager::NewIndexNameL( TDes& aClipPath, const TDesC& aProgram )
    {
    LOG1( "CRtpClipManager::NewIndexNameL(), aClipPath  : %S", &aClipPath );
    LOG1( "CRtpClipManager::NewIndexNameL(), aProgram   : %S", &aProgram );

    // Remove special characters
    TBuf<KMaxProgramChars> program( aProgram.Left( KMaxProgramChars ) );
    for ( TInt i( program.Length() - 1 ); i >= 0; i-- )
        {
        TChar letter( program[i] );
        // Remove if not alpha nor space
        if ( !letter.IsAlphaDigit() && !letter.IsSpace() )
            {
            program.Delete( i, 1 );
            }
        }
    program.TrimRight();
    
    TInt index( KFirstFileIndex );
    
    // Test name for existing clip check
    TPath testName( aClipPath );
    if ( program.Length() )
        {
        testName.Append( program );
        }
    else
        {
        // Zero length program name, start from "(01).rtp"
        testName.AppendFormat( KIndexFormat, index++ );        
        }

    // Name already used ?
    testName.Append( KDvrClipExtension );
    if ( BaflUtils::FileExists( iFs, testName ) )
        {
        do
            {
            LOG1( "CRtpClipManager::NewIndexNameL(), Clip exist: %S", &testName );
            
            // Abort if file index exceeds "(99)"
            User::LeaveIfError( ( index > KMaxFileIndex ) * KErrOverflow );
            
            // New test name
            testName.Copy( aClipPath );
            testName.Append( program );
            testName.AppendFormat( KIndexFormat, index++ );
            testName.Append( KDvrClipExtension );
            }
            while ( BaflUtils::FileExists( iFs, testName ) );
        }
    
    // Return suitable filename
    aClipPath.Copy( testName );
    }
    
// -----------------------------------------------------------------------------
// CRtpClipManager::AddClipToRepairQueueL
// Inserts new clip name to the first in queue.
// -----------------------------------------------------------------------------
//
void CRtpClipManager::AddClipToRepairQueueL( const TDesC& aClipPath )
    {
    HBufC* clip = aClipPath.AllocLC();
    User::LeaveIfError( iRepairQueue.Insert( clip, 0 ) );
    CleanupStack::Pop( clip );
    }

// End of File

