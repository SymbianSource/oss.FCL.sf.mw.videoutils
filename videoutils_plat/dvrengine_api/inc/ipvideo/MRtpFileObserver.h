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
* Description:    RTP file observer for Common Recording Engine.*
*/




#ifndef MRTPFILEOBSERVER_H
#define MRTPFILEOBSERVER_H

// INCLUDES
// None

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
*  A RTP read observer.
*
*  @lib RtpClipHandler.lib
*  @since Series 60 3.0
*/
class MRtpFileObserver
    {

public: // New functions
        
    /**
    * Called when RTP group save is ready.
    * @since Series 60 3.0
    * @param aAction a save action for last saved group.
    * @return None.
    */
    virtual void RtpGroupSaved( const TInt aAction ) = 0;

    /**
    * Getter for current file reader point.
    * @since Series 60 3.0
    * @param aIndex a index of shift seek array.
    * @return a read point in file reader.
    */
    virtual TInt CurrentFileReadPoint( const TInt aIndex ) = 0;
  
    };

#endif // MRTPFILEOBSERVER_H

// End of File
