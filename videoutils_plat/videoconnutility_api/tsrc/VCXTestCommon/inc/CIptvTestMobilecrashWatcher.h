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




#ifndef CIPTVTESTMOBILECRASHWATCHER_H
#define CIPTVTESTMOBILECRASHWATCHER_H

//  INCLUDES
#include <e32base.h>
#include <e32cmn.h> // RArray
#include <f32file.h> // RFs

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS


// CLASS DECLARATION


/**
*
*
*
*
*  @since
*/
class CIptvTestMobilecrashWatcher : public CBase
    {
    public:  // Constructors and destructor


        /**
        * Two-phased constructor.
        * This has iPanicEnabled member set ETrue which will cause panic in destructor if there's new mobilecrashes
        */
        IMPORT_C static CIptvTestMobilecrashWatcher* NewL();

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CIptvTestMobilecrashWatcher* NewL(TBool aPanicEnabled);

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CIptvTestMobilecrashWatcher();


    public: // New functions

		/**
        * Reads all mobilecrashes in c:\data and e:\ directories.
        * @since
        * @param aCrashes list of mobilecrash files are stored into here
        * @return
        */
		void ReadMobilecrashesL( RPointerArray<HBufC>& aCrashes, const TDesC& aDrive );

    	/**
        * Compares the two arrays of filenames and reports new files in aCrashesNow and causes a panic.
        * @since
        * @param aCrashesBefore old filelist
        * @param aCrashesNow new filelist
        * @return ETrue if new mobilecrashes are found, otherwise EFalse
        */
		IMPORT_C TBool CompareMobilecrashesL(RPointerArray<HBufC>& aCrashesBefore, RPointerArray<HBufC>& aCrashesNow);

    	/**
        * Checks for new mobilecrashes. Panics if found.
        * @since
        * @param
        * @return
        */
		IMPORT_C void CheckNewCrashesL();

		/**
        * Checks for new mobilecrashes.
        * @since
        * @param aArray is populated of the new mobilecrashes.
        * @return EFalse if new mobilecrashes are not found. ETrue if found.
        */
		IMPORT_C TBool ReturnNewCrashes(RPointerArray<HBufC>& aArray);

		/**
        * Returns the count of mobilecrashes in c:\\data and e:\\
        * @since
        * @param
        * @return Count of mobilecrashes in c:\\data and e:\\
        */
		IMPORT_C TInt ReturnMobileCrashCount();

    protected: //from base classes


    private:

        /**
        * C++ default constructor.
        */
        CIptvTestMobilecrashWatcher();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL(TBool aPanicEnabled);

    private:    // Data
		// If ETrue and new mobilecrashes are found then a panic will be thrown in destructor
		TBool iPanicEnabled;

		// File system session
		RFs iFsSession;

		RPointerArray<HBufC> iCrashesBefore;

		RPointerArray<HBufC> iCrashesNow;
    };


#endif      // CIPTVTESTMOBILECRASHWATCHER_H

// End of File
