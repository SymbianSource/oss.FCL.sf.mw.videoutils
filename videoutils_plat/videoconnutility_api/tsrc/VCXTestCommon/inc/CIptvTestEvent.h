/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*
*/


#ifndef CIPTVTESTEVENT_H
#define CIPTVTESTEVENT_H

//  INCLUDES
#include <e32base.h>
#include <e32std.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CIptvTestEvent;

// CLASS DECLARATION

/**
*
*
*
*
*  @since
*/
class CIptvTestEvent : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        //IMPORT_C static CIptvTestEvent* NewL(TInt aTimeoutSeconds);

        /**
        * C++ default constructor.
        */
        IMPORT_C CIptvTestEvent( const TDesC& aName, TInt aTimeoutSeconds);

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CIptvTestEvent();


    public: // New functions

		/**
		* Compares if this and aEvent are same. Returns ETrue if so, else EFalse.
		*/
		IMPORT_C virtual TBool Compare( CIptvTestEvent& aEvent ) = 0;

		/**
		* Returns error code if there's error in the event, otherwise KErrNone.
		*/
		IMPORT_C virtual TInt GetError( ) = 0;

		/**
		* Getter for even name.
		*/
		IMPORT_C void GetName( TDes& aName );

		/**
		* Returns if event has timed out.
		*/
		IMPORT_C TBool HasTimeouted( TTime& aTimeNow );

		/*
		* Starts timing for this event
		*/
		IMPORT_C void Activate( );

		/*
		* Stops timing for this event
		*/
		IMPORT_C void Deactivate( );

    protected: //from base classes

    private:


        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data

          TInt iTimeoutSeconds;
          TTime iActivationTime;
          TBuf<32> iName;
          TBool iActive;
    };


#endif      // CIPTVTESTEVENT_H

// End of File
