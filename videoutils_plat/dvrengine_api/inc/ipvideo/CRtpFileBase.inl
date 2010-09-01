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
* Description:    Implementation of the Common Recording Engine RTP file base*
*/




// INCLUDE FILES

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CRtpFileBase::ResetSeekArray
// Resets whole array.
// -----------------------------------------------------------------------------
//
inline void CRtpFileBase::ResetSeekArray()
    {
    iSeekArray->Reset();
    }

// End of File
