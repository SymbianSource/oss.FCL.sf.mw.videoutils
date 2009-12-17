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
* Description:    Implementation of the Common Recording Engine RTP save format*
*/




// INCLUDE FILES
#include <ipvideo/CRtpFileBase.h>

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpToFile::GroupsTotalCount
// Returns: Total count of packet groups.
// -----------------------------------------------------------------------------
//
inline TInt CRtpToFile::GroupsTotalCount()
    {
    return iGroupsTotalCount;
    }

// -----------------------------------------------------------------------------
// CRtpToFile::FirstSeekAddr
// Returns: First group's address in RTP file.
// -----------------------------------------------------------------------------
//
inline TInt CRtpToFile::FirstSeekAddr()
    {
    return iFirstSeekAddr;
    }

// -----------------------------------------------------------------------------
// CRtpToFile::LastSeekAddr
// Returns: Last group's address in RTP file.
// -----------------------------------------------------------------------------
//
inline TInt CRtpToFile::LastSeekAddr()
    {
    return ( iMode == EModeTimeShift )? KMaxTInt:
        ( IsActive() )? iPrevGroupPoint: iLastSeekAddr;
    }

// -----------------------------------------------------------------------------
// CRtpToFile::SeekHeaderPoint
// Returns: Seek header point in RTP file.
// -----------------------------------------------------------------------------
//
inline TInt CRtpToFile::SeekHeaderPoint()
    {
    return iSeekHeaderPoint;
    }

// -----------------------------------------------------------------------------
// CRtpToFile::SeekArray
// Returns: Seek arry pointer.
// -----------------------------------------------------------------------------
//
inline CArrayFix<CRtpFileBase::SSeek>* CRtpToFile::SeekArray()
    {
    return iSeekArray;
    }

// -----------------------------------------------------------------------------
// CRtpToFile::Action
// Returns: Current save action.
// -----------------------------------------------------------------------------
//
inline MRtpFileWriteObserver::TRtpSaveAction CRtpToFile::Action() const
    {
    return iAction;
    }

// End of File
