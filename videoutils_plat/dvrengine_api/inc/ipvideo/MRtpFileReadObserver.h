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
* Description:    Interface for RTP file actions.*
*/




#ifndef MRTPFILEREADOBSERVER_H
#define MRTPFILEREADOBSERVER_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS
// None

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION

/**
*  RTP file data read observer.
*
*  @lib RtpClipHandler.lib
*  @since Series 60 3.0
*/
class MRtpFileReadObserver
    {
    
public: // Data types

    /**
    * Defines status indication to client.
    */
    enum TRtpReadStatus
        {
        ERtpTimeShifTEnd = 500
        };

public: // New methods

    /**
    * Called when RTP packets group readed from a file.
    * @since Series 60 3.0
    * @param aConversion a converter for packets bytes.
    * @param aGroup a RTP data readed from a file.
    * @param aGroupTime a group time in milli seconds.
    * @param aLastGroup a indication of last group in clip.
    * @return none.
    */
    virtual void GroupReadedL( const TDesC8& aGroup,
                               const TUint aGroupTime,
                               const TBool aLastGroup ) = 0;

    /**
    * Clip handler status of playback.
    * @since Series 60 3.0
    * @param aStatus a status of file reading.
    * @return none.
    */
    virtual void ReadStatus( TInt aStatus ) = 0;

    };
    
#endif // MRTPFILEREADOBSERVER_H

// End of file
