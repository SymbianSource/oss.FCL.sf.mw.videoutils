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




#ifndef MRTPFILEWRITEOBSERVER_H
#define MRTPFILEWRITEOBSERVER_H

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
*  RTP file data write observer.
*
*  @lib RtpClipHandler.lib
*  @since Series 60 3.0
*/
class MRtpFileWriteObserver 
    {

public: // Data types

    /**
    * Defines RTP recording params
    */
    class SRtpRecParams
        {
    
    public: // Data

        // Clip full path.
        TPath iClipPath;
        
        // SDP data.
        TPtrC8 iSdpData;
        
        // Service name
        TPtrC iService;
        
        // Program name.
        TPtrC iProgram;
        
        // Post accuisition rule.
        TInt iPostRule;

        // Parental rate.
        TInt iParental;

        // Recording start time in terminal time.
        TTime iStartTime;
        
        // Estimated recording end time in terminal time.
        TTime iEndTime;

        };

    /**
    * Defines save action.
    */
    enum TRtpSaveAction
        {
        ESaveIdle = KErrNotFound,
        ESaveNormal = KErrNone,
        ESaveTimeShift,
        ESavePause,
        ESaveEnd
        };

    /**
    * Defines RTP packet type.
    */
    enum TRtpType
        {
        ERtpNone = KErrNotFound,
        ERtpAudio = 10,
        ERtcpAudio,
        ERtpVideo,
        ERtcpVideo,
        ERtpClipPause,
        ERtpClipEnd,
        ERtpSubTitle,
        ERtcpSubTitle
        };

public: // New methods

    /**
    * Group saved indication.
    * @since Series 60 3.0
    * @return none.
    */
    virtual void GroupSaved() = 0;

    /**
    * Clip handler status of recording.
    * @since Series 60 3.0
    * @param aStatus a status of file writing.
    * @return none.
    */
    virtual void WriteStatus( const TInt aStatus ) = 0;

    };
    
#endif // MRTPFILEWRITEOBSERVER_H

// End of file
