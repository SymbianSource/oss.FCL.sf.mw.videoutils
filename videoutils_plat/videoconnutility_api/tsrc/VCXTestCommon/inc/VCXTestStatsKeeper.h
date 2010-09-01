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


#ifndef VCXTESTSTATSKEEPER_H
#define VCXTESTSTATSKEEPER_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
* Class to hold a single statistics item.
*/
class CVCXTestStatsItem : public CBase
    {
    public:  // Constructors and destructor
    
        /**
        * Two-phased constructor.
        */
        static CVCXTestStatsItem* NewL();

        /**
        * Destructor.
        */
        virtual ~CVCXTestStatsItem();
	
    private:

        /**
        * C++ default constructor.
        */
        CVCXTestStatsItem();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
    
    public: // Data

        TBool iIsActive;
        
        TInt32 iId;
        HBufC* iDescId;
        HBufC* iDesc;
        TInt iError;
        TTime iStartTime;
        TTime iEndTime;
        
        RPointerArray<HBufC> iTraces;
        RPointerArray<CVCXTestStatsItem> iProgresses;
    };

/**
* Class to hold summary for statistics.
*/
class CVCXTestStatsItemSummary : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CVCXTestStatsItemSummary* NewL();

        /**
        * Destructor.
        */
        virtual ~CVCXTestStatsItemSummary();
    
    private:

        /**
        * C++ default constructor.
        */
        CVCXTestStatsItemSummary();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
    
    public: // Data
        TInt32 iId;
        HBufC* iDescId;
        HBufC* iDesc;
        
        TUint iCount;
        TUint iFailCount;

        // These are all microseconds.
        TInt64 iMinTime;
        TInt64 iMaxTime;
        TReal iAvgTime;
        
        TInt64 iTotalTime;
        TInt64 iFailTime;
    };

/**
* Class to keep track of statistics.
*/
class CVCXTestStatsKeeper : public CBase
    {
    public:  // Constructors and destructor
    
        /**
        * Two-phased constructor.
        */
        IMPORT_C static CVCXTestStatsKeeper* NewL();

        /**
        * Destructor.
        */
        virtual ~CVCXTestStatsKeeper ();

    public: // New functions

    	/**
    	* Sets the name and filename for statistics. Report is written when class is desctructed.
    	*/
		IMPORT_C void StartStatsKeepingL( const TDesC& aName, const TDesC& aFileName );

		/**
		 * Adds an action to the statistics lists.
		 */
		IMPORT_C void ActionStartL( TInt32 aId, const TDesC& aDesc );

        /**
         * Adds an action to the statistics lists. 
         * if param aDescMerge is true then action descsription will contain the descid. 
         */
        IMPORT_C void ActionStartL( const TDesC& aDescId, const TDesC& aDesc, TBool aDescMerge = EFalse );
		
		/**
		 * Ends an action.
		 * Leaves with KErrAbort if more than 33% of actions have failed. Test should abort itself in that case.
		 */
		IMPORT_C void ActionEndL( TInt32 aId, TInt aError );

        /**
         * Ends an action.
         * Leaves with KErrAbort if more than 33% of actions have failed. Test should abort itself in that case.
         */
        IMPORT_C void ActionEndL( const TDesC& aDescId, TInt aError );
		
		/**
		 * Sets progress information for an action. These are saved in the report.
		 */
		IMPORT_C void ActionProgressL( TInt32 aId, const TDesC& aProgressDesc );

        /**
         * Sets progress information for an action. These are saved in the report.
         */
        IMPORT_C void ActionProgressL( const TDesC& aDescId, const TDesC& aProgressDesc );
		
		/**
		 * Generic trace for different actions. Last 20 of these are saved in the report only if action fails.
		 */
		IMPORT_C void ActionTraceL( const TDesC& aDesc );
		
    private:

        /**
        * C++ default constructor.
        */
        CVCXTestStatsKeeper();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private: // New functions.
        
        CVCXTestStatsItem* GetActiveAction( TInt32 aId );
        
        CVCXTestStatsItem* GetActiveAction( const TDesC& aDesc );
        
        CVCXTestStatsItem* GetLastActiveAction( TBool aOnlyDescId );
        
        TInt FormatDate( const TTime& aDate, TDes& aStr );
        
        void GenerateReportL( const TDesC& aFileName, TBool aFullReport );
        
        TUint DataSizeL();
        
        void CheckFailsL();

        void GetActionSummariesL( RPointerArray<CVCXTestStatsItemSummary>& aSummaries, TBool aFailActiveActions );
        
    private:    // Data

        TBool iStarted;
        
        HBufC* iReportName;
        
        HBufC* iReportFileName;       
        
        RPointerArray<CVCXTestStatsItem> iActions;
        
        RPointerArray<HBufC> iTraces;
        
        TBool iAbortedBecauseTooManyFails;
    };

#endif      // VCXTESTSTATSKEEPER_H

// End of File
