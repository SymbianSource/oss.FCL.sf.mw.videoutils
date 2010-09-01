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
* Description:    Implementation of the Common Recording Engine RTP read format*
*/





// INCLUDE FILES

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpFromFile::Duration
// Getter for current duration of the clip.
// -----------------------------------------------------------------------------
//
inline TUint CRtpFromFile::Duration()
    {
    return iDuration;
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::ThisGroup
// Getter for currently active group point.
// -----------------------------------------------------------------------------
//
inline TInt CRtpFromFile::ThisGroup()
    {
    return iThisGroup;
    }

// -----------------------------------------------------------------------------
// CRtpFromFile::IsTimeShift
// Getter for time shift mode.
// -----------------------------------------------------------------------------
//
inline TBool CRtpFromFile::IsTimeShift()
    {
    return ( iMode == EModeTimeShift );
    }
    
// End of File
