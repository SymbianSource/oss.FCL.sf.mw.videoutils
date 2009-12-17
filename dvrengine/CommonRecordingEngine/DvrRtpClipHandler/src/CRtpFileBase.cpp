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
* Description:    Implementation of the Common Recording Engine RTP file base class.*
*/




// INCLUDE FILES
#include <ipvideo/CRtpFileBase.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KFirstIntegerPoint( 0 );
const TInt KSecondIntegerPoint( KFirstIntegerPoint + KIntegerBytes );
const TInt KThirdIntegerPoint( KSecondIntegerPoint + KIntegerBytes );
const TInt KFourthIntegerPoint( KThirdIntegerPoint + KIntegerBytes );
const TInt KSeekArrayGranularity( 20 );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpFileBase::CRtpFileBase
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CRtpFileBase::CRtpFileBase()
  : CActive( CActive::EPriorityStandard ),
    iMode( EModeNone ),
    iThisGroup( KErrNotFound ),
    iGroupsTotalCount( KErrNotFound ),
    iFirstSeekAddr( KErrNotFound ),
    iLastSeekAddr( KErrNotFound ),
    iGroupTotalLen( KErrNotFound ),
    iNextGroupPoint( KErrNotFound ),
    iPrevGroupPoint( KErrNotFound ),
    iGroupTime( 0 ),
    iSeekHeaderPoint( KErrNotFound ),
    iDataPtr( 0, 0 )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CRtpFileBase::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CRtpFileBase::ConstructL()
    {
    LOG( "CRtpFileBase::ConstructL()" );

    iSeekArray = new( ELeave ) CArrayFixFlat<SSeek>( KSeekArrayGranularity );
    }

// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CRtpFileBase::~CRtpFileBase()
    {
    LOG( "CRtpFileBase::~CRtpFileBase()" );
    // Do not call Cancel here, it will cause crashes (DoCancel of inherited
    // class is never called, instead it jumps to foobar address)
    iFile.Close();
    iFs.Close();
    delete iCurrentPath;
    delete iSeekArray;
    }

// -----------------------------------------------------------------------------
// CRtpFileBase::DeleteTimeShiftFiles
// Deletes files used during time shift mode.
// -----------------------------------------------------------------------------
//
void CRtpFileBase::DeleteTimeShiftFiles( RArray<STimeShiftSeek>& aShiftSeek )
    {
    LOG1( "CRtpFileBase::DeleteTimeShiftFiles(), count: %d", aShiftSeek.Count() );
    
    TInt index( KErrNotFound );
    const TInt count( aShiftSeek.Count() );
    for ( TInt i( 0 ); i < count; i++ )
        {
        if ( aShiftSeek[i].iNameIndex != index )
            {
            TPath clipPath( KDvrTimeShiftFile );
            index = aShiftSeek[i].iNameIndex;
            clipPath.AppendNum( index );
            iFs.Delete( clipPath );
            LOG1( "CRtpFileBase::DeleteTimeShiftFiles(), deleted: %S", &clipPath );
            }
        }

    aShiftSeek.Reset();
    }
    
// -----------------------------------------------------------------------------
// CRtpFileBase::WriteSeekHeaderL
// Writes seek header of all groups.
// -----------------------------------------------------------------------------
//
void CRtpFileBase::WriteSeekHeaderL()
    {
    User::LeaveIfError( iSeekHeaderPoint );

    HBufC8* data = HBufC8::NewLC( KSeekHeaderBytes );
    TPtr8 ptr( data->Des() );
    
    HBufC8* bytes = CRtpUtil::MakeBytesLC( iGroupsTotalCount );
    ptr.Copy( bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );
    
    bytes = CRtpUtil::MakeBytesLC( iFirstSeekAddr );
    ptr.Append( bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );

    bytes = CRtpUtil::MakeBytesLC( iLastSeekAddr );
    ptr.Append( bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );

    User::LeaveIfError( iFile.Write( iSeekHeaderPoint, ptr, KSeekHeaderBytes ) );
    CleanupStack::PopAndDestroy( data );
    }
 
// -----------------------------------------------------------------------------
// CRtpFileBase::ReadSeekHeaderL
// Readss seek header of all groups.
// -----------------------------------------------------------------------------
//
void CRtpFileBase::ReadSeekHeaderL()
    {
    User::LeaveIfError( iSeekHeaderPoint );

    HBufC8* bytes = HBufC8::NewLC( KSeekHeaderBytes );
    TPtr8 ptr( bytes->Des() );
    User::LeaveIfError( iFile.Read( iSeekHeaderPoint, ptr, KSeekHeaderBytes ) );
    if ( ptr.Length() < KSeekHeaderBytes )
        {
        LOG( "CRtpFileBase::ReadSeekHeaderL(), Seek Header Corrupted" );
        User::Leave( KErrCorrupt );
        }

    iGroupsTotalCount = CRtpUtil::GetValueL( ptr.Mid( KFirstIntegerPoint,
                                                      KIntegerBytes ) );
    User::LeaveIfError( iGroupsTotalCount );
    iFirstSeekAddr = CRtpUtil::GetValueL( ptr.Mid( KSecondIntegerPoint,
                                                   KIntegerBytes ) );
    User::LeaveIfError( iFirstSeekAddr );
    iLastSeekAddr = CRtpUtil::GetValueL( ptr.Mid( KThirdIntegerPoint,
                                                  KIntegerBytes ) );
    User::LeaveIfError( iLastSeekAddr );
    CleanupStack::PopAndDestroy( bytes );
    }

// -----------------------------------------------------------------------------
// CRtpFileBase::ReadGroupHeaderL
// Reads group header.
// -----------------------------------------------------------------------------
//
void CRtpFileBase::ReadGroupHeaderL()
    {
    User::LeaveIfError( iThisGroup );

    HBufC8* bytes = HBufC8::NewLC( KGroupHeaderBytes );
    TPtr8 ptr( bytes->Des() );

    User::LeaveIfError( iFile.Read( iThisGroup, ptr, KGroupHeaderBytes ) );
    UpdateGroupHeaderVariablesL( ptr );

    CleanupStack::PopAndDestroy( bytes );
    }

// -----------------------------------------------------------------------------
// CRtpFileBase::UpdateGroupHeaderVariablesL
// Updates group header variables from readed data.
// -----------------------------------------------------------------------------
//
void CRtpFileBase::UpdateGroupHeaderVariablesL( const TDesC8& aDataPtr )
    {
    if ( aDataPtr.Length() < KGroupHeaderBytes )
        {
        LOG( "CRtpFileBase::UpdateGroupHeaderVariablesL(), Group Header Corrupted" );
        User::Leave( KErrCorrupt );
        }

    iGroupTotalLen  = CRtpUtil::GetValueL( aDataPtr.Mid( KFirstIntegerPoint ,
                                                         KIntegerBytes ) );
    User::LeaveIfError( iGroupTotalLen );
    iNextGroupPoint = CRtpUtil::GetValueL( aDataPtr.Mid( KSecondIntegerPoint,
                                                         KIntegerBytes ) );
    User::LeaveIfError( iNextGroupPoint );
    iPrevGroupPoint = CRtpUtil::GetValueL( aDataPtr.Mid( KThirdIntegerPoint,
                                                         KIntegerBytes ) );
    User::LeaveIfError( iPrevGroupPoint );
    iGroupTime = CRtpUtil::GetValueL( aDataPtr.Mid( KFourthIntegerPoint,
                                                    KIntegerBytes ) );
    User::LeaveIfError( iGroupTime );
    }

// -----------------------------------------------------------------------------
// CRtpFileBase::AppendSeekArrayL
// Appends one item to seek array.
// -----------------------------------------------------------------------------
//
void CRtpFileBase::AppendSeekArrayL( const TUint aTime, const TInt aPoint )
    {
#ifdef CR_ALL_LOGS
    LOG2( "CRtpFileBase::AppendSeekArrayL(), aTime: %u, aPoint: %d", 
                                             aTime, aPoint );
#endif // CR_ALL_LOGS

    SSeek seek;
    seek.iTime = aTime;
    seek.iPoint = aPoint;
    iSeekArray->AppendL( seek );
    }

// -----------------------------------------------------------------------------
// CRtpFileBase::SaveSeekArrayL
//
// -----------------------------------------------------------------------------
//
void CRtpFileBase::SaveSeekArrayL()
    {
    LOG1( "CRtpFileBase::SaveSeekArrayL(), count: %d", iSeekArray->Count() );
    
    const TInt len( KIntegerBytes + iSeekArray->Count() * 2 * KIntegerBytes );
    HBufC8* data = HBufC8::NewLC( len );
    TPtr8 ptr( data->Des() );
    
    // Total count
    HBufC8* bytes = CRtpUtil::MakeBytesLC( iSeekArray->Count() );
    ptr.Copy( bytes->Des() );
    CleanupStack::PopAndDestroy( bytes );

    for ( TInt i( 0 ); i < iSeekArray->Count(); i++ )
        {
        // Time
        bytes = CRtpUtil::MakeBytesLC( iSeekArray->At( i ).iTime );
        ptr.Append( bytes->Des() );
        CleanupStack::PopAndDestroy( bytes );

        // Point
        bytes = CRtpUtil::MakeBytesLC( iSeekArray->At( i ).iPoint );
        ptr.Append( bytes->Des() );
        CleanupStack::PopAndDestroy( bytes );

#ifdef CR_ALL_LOGS
        LOG3( "CRtpFileBase::SaveSeekArrayL(), ind: %d, time: %u, point: %d",
               i, iSeekArray->At( i ).iTime, iSeekArray->At( i ).iPoint );
#endif // CR_ALL_LOGS
        }

    User::LeaveIfError( iFile.Write( iNextGroupPoint, ptr, len ) );
    CleanupStack::PopAndDestroy( data );
    }

// -----------------------------------------------------------------------------
// CRtpFileBase::ReadSeekArrayL
//
// -----------------------------------------------------------------------------
//
TBool CRtpFileBase::ReadSeekArrayL( const TInt aPoint )
    {
    User::LeaveIfError( aPoint );
    HBufC8* bytes = HBufC8::NewLC( KIntegerBytes );
    TPtr8 ptr( bytes->Des() );
    
    // Verify read point
    TInt size( KErrNotFound );
    iFile.Size( size );
    User::LeaveIfError( ( aPoint > ( size - KIntegerBytes ) ) * KErrCorrupt );
    
    // Total count
    User::LeaveIfError( iFile.Read( aPoint, ptr, KIntegerBytes ) );
    const TInt count( CRtpUtil::GetValueL( ptr ) );
    CleanupStack::PopAndDestroy( bytes );
    LOG1( "CRtpFileBase::ReadSeekArrayL(), count: %d", count );
    
    // Any point stored?
    if ( count > 0 )
        {
        // Read array from clip
        User::LeaveIfError( ( count > ( KMaxTInt / 2 / KIntegerBytes ) ) * KErrCorrupt );
        const TInt len( count * 2 * KIntegerBytes );
        HBufC8* data = HBufC8::NewLC( len );
        ptr.Set( data->Des() );
        User::LeaveIfError( iFile.Read( aPoint + KIntegerBytes, ptr, len ) );
    
        // Set seek array
        for ( TInt i( 0 ); i < count; i++ )
            {
            const TInt next( i * 2 * KIntegerBytes );
            if ( ptr.Length() < ( next + ( 2 * KIntegerBytes ) ) )
                {
                LOG( "CRtpFileBase::ReadSeekArrayL(), Seek Array Corrupted" );
                User::Leave( KErrCorrupt );
                }

            // Time
            TUint time( CRtpUtil::GetValueL( ptr.Mid( next, KIntegerBytes ) ) );
            // Point
            TInt point( CRtpUtil::GetValueL( ptr.Mid( next + KIntegerBytes,
                                                      KIntegerBytes ) ) );
            User::LeaveIfError( point );
            AppendSeekArrayL( time, point );
#ifdef CR_ALL_LOGS
            LOG3( "CRtpFileBase::ReadSeekArrayL(), ind: %d, time: %u, point: %d",
                                                   i, time, point );
#endif // CR_ALL_LOGS
            }
    
        CleanupStack::PopAndDestroy( data );
        }

    return ( count > 0 );
    }

// -----------------------------------------------------------------------------
// CRtpFileBase::LogVariables
// -----------------------------------------------------------------------------
//
void CRtpFileBase::LogVariables( const TDesC& aMethod )
    {
#ifdef CR_ALL_LOGS
    LOG1( "CRtpFileBase::LogVariables(), Method: %S", &aMethod );
    LOG1( "CRtpFileBase::LogVariables(), iMode : %d", iMode );
    LOG1( "CRtpFileBase::LogVariables(), GTC   : %d", iGroupsTotalCount );
    LOG1( "CRtpFileBase::LogVariables(), FSA   : %d", iFirstSeekAddr );
    LOG1( "CRtpFileBase::LogVariables(), LSA   : %d", iLastSeekAddr );
    LOG1( "CRtpFileBase::LogVariables(), This  : %d", iThisGroup );
    LOG1( "CRtpFileBase::LogVariables(), GTL   : %d", iGroupTotalLen );
    LOG1( "CRtpFileBase::LogVariables(), NGP   : %d", iNextGroupPoint );
    LOG1( "CRtpFileBase::LogVariables(), PGP   : %d", iPrevGroupPoint );
    LOG1( "CRtpFileBase::LogVariables(), GTime : %u", iGroupTime );
#else // CR_ALL_LOGS
    ( void )aMethod;
#endif // CR_ALL_LOGS
    }

// End of File

