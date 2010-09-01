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


//  INCLUDES
#include "VCXTestStatsKeeper.h"
#include "VCXTestLog.h"
#include <flogger.h>
#include <mmf/common/mmfcontrollerpluginresolver.h> // CleanupResetAndDestroyPushL

// CONSTANTS
const TUint KMaxTraces = 20;

// MACROS

#ifndef __WINSCW__

_LIT( KVCXTestLogFolder, "Fusion" );

#define VCXTESTSTATS_W1(FILE,AAA)                  do { _LIT(tempIPTVLogDes,AAA); RFileLogger::Write(KVCXTestLogFolder,FILE,EFileLoggingModeAppend,tempIPTVLogDes()); } while ( EFalse )
#define VCXTESTSTATS_W2(FILE,AAA,BBB)              do { _LIT(tempIPTVLogDes,AAA); RFileLogger::WriteFormat(KVCXTestLogFolder,FILE,EFileLoggingModeAppend,TRefByValue<const TDesC>(tempIPTVLogDes()),BBB); } while ( EFalse )
#define VCXTESTSTATS_W3(FILE,AAA,BBB,CCC)          do { _LIT(tempIPTVLogDes,AAA); RFileLogger::WriteFormat(KVCXTestLogFolder,FILE,EFileLoggingModeAppend,TRefByValue<const TDesC>(tempIPTVLogDes()),BBB,CCC); } while ( EFalse )
#define VCXTESTSTATS_W4(FILE,AAA,BBB,CCC,DDD)      do { _LIT(tempIPTVLogDes,AAA); RFileLogger::WriteFormat(KVCXTestLogFolder,FILE,EFileLoggingModeAppend,TRefByValue<const TDesC>(tempIPTVLogDes()),BBB,CCC,DDD); } while ( EFalse )
#define VCXTESTSTATS_W5(FILE,AAA,BBB,CCC,DDD,EEE)  do { _LIT(tempIPTVLogDes,AAA); RFileLogger::WriteFormat(KVCXTestLogFolder,FILE,EFileLoggingModeAppend,TRefByValue<const TDesC>(tempIPTVLogDes()),BBB,CCC,DDD,EEE); } while ( EFalse )

#else

#define VCXTESTSTATS_W1(FILE,s)               RDebug::Print(_L(#s))
#define VCXTESTSTATS_W2(FILE,s, a)            RDebug::Print(_L(#s), a)
#define VCXTESTSTATS_W3(FILE,s, a, b)         RDebug::Print(_L(#s), a, b)
#define VCXTESTSTATS_W4(FILE,s, a, b, c)      RDebug::Print(_L(#s), a, b, c)
#define VCXTESTSTATS_W5(FILE,s, a, b, c, d)   RDebug::Print(_L(#s), a, b, c, d)

#endif

// -----------------------------------------------------------------------------
// CVCXTestStatsItem::NewL
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CVCXTestStatsItem* CVCXTestStatsItem::NewL()
    {
    CVCXTestStatsItem* self = new (ELeave)CVCXTestStatsItem();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsItem::~CVCXTestStatsKeeper
// destructor
// -----------------------------------------------------------------------------
//
CVCXTestStatsItem::~CVCXTestStatsItem()
    {
    delete iDesc;
    iDesc = NULL;
    
    delete iDescId;
    iDescId = NULL;
    
    iTraces.ResetAndDestroy();
    iProgresses.ResetAndDestroy();
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsItem::CVCXTestStatsItem()
// -----------------------------------------------------------------------------
//
CVCXTestStatsItem::CVCXTestStatsItem()
    {
    
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsItem::ConstructL()
// -----------------------------------------------------------------------------
//
void CVCXTestStatsItem::ConstructL()
    {
    
    }


// -----------------------------------------------------------------------------
// CVCXTestStatsItemSummary::NewL
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CVCXTestStatsItemSummary* CVCXTestStatsItemSummary::NewL()
    {
    CVCXTestStatsItemSummary* self = new (ELeave)CVCXTestStatsItemSummary();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsItemSummary::~CVCXTestStatsKeeper
// destructor
// -----------------------------------------------------------------------------
//
CVCXTestStatsItemSummary::~CVCXTestStatsItemSummary()
    {
    delete iDesc;
    iDesc = NULL;
    
    delete iDescId;
    iDescId = NULL;
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsItemSummary::CVCXTestStatsItemSummary()
// -----------------------------------------------------------------------------
//
CVCXTestStatsItemSummary::CVCXTestStatsItemSummary()
    {
    
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsItemSummary::ConstructL()
// -----------------------------------------------------------------------------
//
void CVCXTestStatsItemSummary::ConstructL()
    {
    
    }


// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::NewL
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C CVCXTestStatsKeeper* CVCXTestStatsKeeper::NewL()
    {
    CVCXTestStatsKeeper* self = new (ELeave)CVCXTestStatsKeeper();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::~CVCXTestStatsKeeper
// destructor
// -----------------------------------------------------------------------------
//
CVCXTestStatsKeeper::~CVCXTestStatsKeeper ()
    {
    if( iStarted )
        {
        GenerateReportL( _L("FusionStats.txt"), EFalse );
        GenerateReportL( _L("FusionStatsFull.txt"), ETrue );
        }
    
    delete iReportName;
    iReportName = NULL;
    
    delete iReportFileName;
    iReportFileName = NULL;
    
    iActions.ResetAndDestroy();
    iTraces.ResetAndDestroy();
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::CVCXTestStatsKeeper
// -----------------------------------------------------------------------------
//
CVCXTestStatsKeeper::CVCXTestStatsKeeper()
    {
    
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::ConstructL
// -----------------------------------------------------------------------------
//
void CVCXTestStatsKeeper::ConstructL()
    {
    
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::StartStatsKeepingL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestStatsKeeper::StartStatsKeepingL( const TDesC& aName, const TDesC& aFileName )
    {
    iStarted = ETrue;
    iReportName = aName.AllocL();
    iReportFileName = aFileName.AllocL();
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::ActionStartL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestStatsKeeper::ActionStartL( TInt32 aId, const TDesC& aDesc )
    {
    VCXLOGLO3(">>>CVCXTestStatsKeeper::ActionStartL: %d = %S ------>", aId, &aDesc);
    if( !iStarted )
        {
        return;
        }
        
    CVCXTestStatsItem* item = CVCXTestStatsItem::NewL();
    item->iId = aId;
    item->iDesc = aDesc.AllocL();
    item->iStartTime.HomeTime();
    item->iIsActive = ETrue;
    
    iActions.AppendL( item );
    VCXLOGLO1("<<<CVCXTestStatsKeeper::ActionStartL");
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::ActionStartL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestStatsKeeper::ActionStartL( const TDesC& aDescId, const TDesC& aDesc, TBool aDescMerge )
    {
    VCXLOGLO3(">>>CVCXTestStatsKeeper::ActionStartL %S = %S ------>", &aDescId, &aDesc);
    if( !iStarted )
        {
        return;
        }

    CVCXTestStatsItem* item = CVCXTestStatsItem::NewL();
    item->iDescId = aDescId.AllocL();
    if( aDescMerge )
        {
        item->iDesc = HBufC::NewL( aDesc.Length() + aDescId.Length() );
        item->iDesc->Des().Append( aDesc );
        item->iDesc->Des().Append( aDescId );
        }
    else
        {
        item->iDesc = aDesc.AllocL();
        }
    item->iStartTime.HomeTime();
    item->iIsActive = ETrue;
    
    iActions.AppendL( item );
    VCXLOGLO1("<<<CVCXTestStatsKeeper::ActionStartL (desc)");
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::ActionEndL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestStatsKeeper::ActionEndL( TInt32 aId, TInt aError )
    {
    VCXLOGLO3(">>>CVCXTestStatsKeeper::ActionEndL: %d, err: %d ------>", aId, aError);
    if( !iStarted )
        {
        return;
        }
    
    CVCXTestStatsItem* item = GetActiveAction( aId );
    
    if( !item )
        {
        VCXLOGLO1("CVCXTestStatsKeeper:: No active action to end!");
        VCXLOGLO1("<<<CVCXTestStatsKeeper::ActionEndL");
        return;
        }
    
    item->iError = aError;
    item->iEndTime.HomeTime();
    
    TBuf<64> prgBuff( _L("OK") );
    if( aError != KErrNone )
        {
        prgBuff.Format( _L("Failed (%d)"), item->iError);
        }
    ActionProgressL( aId, prgBuff );
    
    item->iIsActive = EFalse;
    
    if( aError != KErrNone )
        {
        for( TInt i = 0; i < iTraces.Count(); i++ )
            {
            item->iTraces.AppendL( iTraces[i] );
            }
        iTraces.Reset();
        }
    
    CheckFailsL();
    
    VCXLOGLO1("<<<CVCXTestStatsKeeper::ActionEndL");
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::ActionEndL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestStatsKeeper::ActionEndL( const TDesC& aDescId, TInt aError )
    {
    VCXLOGLO3(">>>CVCXTestStatsKeeper::ActionEndL :%S, err: %d ------>", &aDescId, aError);
    if( !iStarted )
        {
        return;
        }
    
    CVCXTestStatsItem* item = GetActiveAction( aDescId );
    
    if( !item )
        {
        VCXLOGLO1("CVCXTestStatsKeeper:: No active action to end!");
        VCXLOGLO1("<<<CVCXTestStatsKeeper::ActionEndL (tdesc)");
        return;
        }
    
    item->iError = aError;
    item->iEndTime.HomeTime();
    
    TBuf<64> prgBuff( _L("OK") );
    if( aError != KErrNone )
        {
        prgBuff.Format( _L("Failed (%d)"), item->iError);
        }
    ActionProgressL( aDescId, prgBuff );

    item->iIsActive = EFalse;
    
    if( aError != KErrNone )
        {
        for( TInt i = 0; i < iTraces.Count(); i++ )
            {
            item->iTraces.AppendL( iTraces[i] );
            }
        iTraces.Reset();
        }
    
    CheckFailsL();
    
    VCXLOGLO1("<<<CVCXTestStatsKeeper::ActionEndL (tdesc)");    
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::ActionProgressL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestStatsKeeper::ActionProgressL( TInt32 aId, const TDesC& aProgressDesc )
    {
    VCXLOGLO3(">>>CVCXTestStatsKeeper::ActionProgressL: %d = %S", aId, &aProgressDesc);
    if( !iStarted )
        {
        return;
        }

    CVCXTestStatsItem* item = GetActiveAction( aId );
    
    if( !item )
        {
        User::Leave( KErrNotFound );
        }
    
    CVCXTestStatsItem* progress = CVCXTestStatsItem::NewL();
    
    progress->iStartTime.HomeTime();
    progress->iDesc = aProgressDesc.AllocL();
    
    item->iProgresses.AppendL( progress );        
    VCXLOGLO1("<<<CVCXTestStatsKeeper::ActionProgressL");
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::ActionProgressL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestStatsKeeper::ActionProgressL( const TDesC& aDescId, const TDesC& aProgressDesc )
    {
    VCXLOGLO3(">>>CVCXTestStatsKeeper::ActionProgressL: %S = %S", &aDescId, &aProgressDesc);
    if( !iStarted )
        {
        return;
        }

    CVCXTestStatsItem* item = GetActiveAction( aDescId );
    
    if( !item )
        {
        User::Leave( KErrNotFound );
        }
    
    CVCXTestStatsItem* progress = CVCXTestStatsItem::NewL();
    
    progress->iStartTime.HomeTime();
    progress->iDesc = aProgressDesc.AllocL();
    
    item->iProgresses.AppendL( progress );
    VCXLOGLO1("<<<CVCXTestStatsKeeper::ActionProgressL (desc)");
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::ActionTraceL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVCXTestStatsKeeper::ActionTraceL( const TDesC& aDesc )
    {
    VCXLOGLO1(">>>CVCXTestStatsKeeper::ActionTraceL");
    if( !iStarted )
        {
        return;
        }
    
    while( iTraces.Count() > KMaxTraces )
        {
        iTraces.Remove( 0 );
        }
    
    iTraces.AppendL( aDesc.AllocL() );
    VCXLOGLO1("<<<CVCXTestStatsKeeper::ActionTraceL");
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::GetActiveAction
// -----------------------------------------------------------------------------
//
CVCXTestStatsItem* CVCXTestStatsKeeper::GetActiveAction( TInt32 aId )
    {
    VCXLOGLO1(">>>CVCXTestStatsKeeper::GetActiveAction");
    for( TInt i = 0; i < iActions.Count(); i++ )
        {
        if( iActions[i]->iId == aId && iActions[i]->iIsActive )
            {
            VCXLOGLO2("CVCXTestStatsKeeper:: Found %S.", iActions[i]->iDesc);
            VCXLOGLO1("<<<CVCXTestStatsKeeper::GetActiveAction");
            return iActions[i];
            }
        }
    VCXLOGLO1("<<<CVCXTestStatsKeeper::GetActiveAction");
    return GetLastActiveAction( EFalse );
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::GetActiveAction
// -----------------------------------------------------------------------------
//
CVCXTestStatsItem* CVCXTestStatsKeeper::GetActiveAction( const TDesC& aDescId )
    {
    VCXLOGLO1(">>>CVCXTestStatsKeeper::GetActiveAction (desc)");
    for( TInt i = 0; i < iActions.Count(); i++ )
        {
        if( iActions[i]->iDescId && *iActions[i]->iDescId == aDescId && iActions[i]->iIsActive )
            {
            VCXLOGLO2("CVCXTestStatsKeeper:: Found %S.", iActions[i]->iDesc);
            VCXLOGLO1("<<<CVCXTestStatsKeeper::GetActiveAction (desc)");            
            return iActions[i];
            }
        }
    VCXLOGLO1("<<<CVCXTestStatsKeeper::GetActiveAction (desc)");
    return GetLastActiveAction( ETrue );
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::GetLastActiveAction
// -----------------------------------------------------------------------------
//
CVCXTestStatsItem* CVCXTestStatsKeeper::GetLastActiveAction( TBool aOnlyDescId )
    {
    VCXLOGLO1(">>>CVCXTestStatsKeeper::GetLastActiveAction");
    for( TInt i = iActions.Count()-1; i >= 0; i-- )
        {
        if( iActions[i]->iIsActive )
            {
            if( ( aOnlyDescId && iActions[i]->iDesc ) || ( !aOnlyDescId && !iActions[i]->iDesc ) ) 
                {
                continue;
                }
            VCXLOGLO2("CVCXTestStatsKeeper:: Found: %S.", iActions[i]->iDesc);
            VCXLOGLO1("<<<CVCXTestStatsKeeper::GetLastActiveAction");
            return iActions[i];
            }
        }
    VCXLOGLO1("<<<CVCXTestStatsKeeper::GetLastActiveAction");
    return NULL;
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::FormatDate
// -----------------------------------------------------------------------------
//
TInt CVCXTestStatsKeeper::FormatDate( const TTime& aDate, TDes& aStr )
    {
    _LIT(KDateTimeString,"%D%M%Y%/0%1%/1%2%/2%3%/3 %-B%:0%H%:1%T%:2%S%.%*C2%:3%-B");

    TRAPD( err, aDate.FormatL( aStr, KDateTimeString ) );
    return err;
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::GenerateReport
// -----------------------------------------------------------------------------
//
void CVCXTestStatsKeeper::GenerateReportL( const TDesC& aFileName, TBool aFullReport )
    {
    aFileName.Length();
    
    if( !iStarted )
        {
        return;
        }

    if ( aFullReport )
        {
        VCXTESTSTATS_W2( aFileName, "CVCXTestStatsKeeper:: Data size for the report: %d kBs", DataSizeL() );
        }

    if( iReportName )
        {
        VCXTESTSTATS_W1(aFileName, "");
        VCXTESTSTATS_W1(aFileName, "-------------------------------------------------------------------------");
        VCXTESTSTATS_W2(aFileName, " %S", iReportName);
        VCXTESTSTATS_W1(aFileName, "-------------------------------------------------------------------------");
        }
    
    if( aFullReport )
        {
        const TInt KMaxDescLen = 52;
        
        for( TInt i = 0; i < iActions.Count(); i++ )
            {
            TBuf<256> startTime;
            TBuf<256> endTime;
            
            CVCXTestStatsItem* item = iActions[i];
            
            FormatDate( item->iStartTime, startTime );
            if( !item->iIsActive )
                {
                FormatDate( item->iEndTime, endTime );
                }
            
            TBuf<KMaxDescLen+10> shortened;
            if( item->iDesc->Length() > KMaxDescLen )
                {
                TPtrC left = item->iDesc->Left( KMaxDescLen/2 );
                TPtrC right = item->iDesc->Right( KMaxDescLen/2 );
                shortened.Format( _L("%S ... %S"), &left, &right );
                }
            else
                {
                shortened.Copy( *item->iDesc );
                }
            
            TBuf<1024*2> buff;
            buff.Format( _L("%- *S %S"), 60, &shortened, &startTime);
            VCXTESTSTATS_W2(aFileName, "%S", &buff);
    
            if( item->iProgresses.Count() > 0 )
                {
                for( TInt e = 0; e < item->iProgresses.Count(); e++ )
                    {
                    FormatDate( item->iProgresses[e]->iStartTime, startTime );
                    if( item->iProgresses[e]->iDesc->Length() > KMaxDescLen )
                        {
                        TPtrC left = item->iProgresses[e]->iDesc->Left( KMaxDescLen/2-2 );
                        TPtrC right = item->iProgresses[e]->iDesc->Right( KMaxDescLen/2-2 );
                        shortened.Format( _L("%S .. %S"), &left, &right );
                        }
                    else
                        {
                        shortened.Copy( *item->iProgresses[e]->iDesc );
                        }
                    buff.Format( _L("   %- *S %S"), 57, &shortened, &startTime );
                    VCXTESTSTATS_W2(aFileName, "%S", &buff);
                    }
                }
            
            if( item->iTraces.Count() > 0 )
                {
                VCXTESTSTATS_W1(aFileName, "   Traces:");
                for( TInt e = 0; e < item->iTraces.Count(); e++ )
                    {
                    VCXTESTSTATS_W2(aFileName, "   %S", item->iTraces[e]);
                    }
                }
            }
        }
    
    RPointerArray<CVCXTestStatsItemSummary> summaries;
    CleanupResetAndDestroyPushL( summaries );
    GetActionSummariesL( summaries, ETrue );
    
    TBuf<256> buff;

    _LIT( KVCXTESTDest, "Desc" );
    _LIT( KVCXTESTFails, "Fails" );
    _LIT( KVCXTESTTotal, "Total" ); 
    _LIT( KVCXTESTMinTime, "MinTime" ); 
    _LIT( KVCXTESTMaxTime, "MaxTime" ); 
    _LIT( KVCXTESTAvgTime, "AvgTime");
    _LIT( KVCXTESTTotalTime, "TotTime");
    
    const TInt KDescFieldLen = 32;
    
    // Header.
    buff.Format( _L("%- *S %+ 6S %+ 6S %+ 6S %+ 6S %+ 6S %+ 6S"), KDescFieldLen,
            &KVCXTESTDest, &KVCXTESTFails, &KVCXTESTTotal, &KVCXTESTMinTime, &KVCXTESTMaxTime, &KVCXTESTAvgTime, &KVCXTESTTotalTime );
    VCXTESTSTATS_W2(aFileName, "%S", &buff);
    
    // Data.
    for( TInt e = 0; e < summaries.Count(); e++ )
        {
        TBuf<KDescFieldLen+10> shortened;
        if( summaries[e]->iDesc->Length() > KDescFieldLen )
            {
            TPtrC left = summaries[e]->iDesc->Left( KDescFieldLen/2-2 );
            TPtrC right = summaries[e]->iDesc->Right( KDescFieldLen/2-2 );
            shortened.Format( _L("%S .. %S"), &left, &right );
            }
        else
            {
            shortened.Copy( *summaries[e]->iDesc );
            }        
        
        const TReal KXXXMillion = 1000000;
        TReal minTime = (TReal)summaries[e]->iMinTime / KXXXMillion; 
        TReal maxTime = (TReal)summaries[e]->iMaxTime / KXXXMillion;
        TReal avgTime = (TReal)summaries[e]->iAvgTime / KXXXMillion;
        TInt totalTime = (TReal)summaries[e]->iTotalTime / KXXXMillion;
        
        buff.Format( _L("%- *S %+ 6d %+ 6d %+ 6.2f %+ 6.2f %+ 6.2f %+ 6d"), KDescFieldLen, 
                &shortened, summaries[e]->iFailCount, summaries[e]->iCount,  
                minTime, maxTime, avgTime, totalTime );
        VCXTESTSTATS_W2(aFileName, "%S", &buff);
        }
    
    if( iAbortedBecauseTooManyFails )
        {
        VCXTESTSTATS_W1(aFileName, "Too many fails, test was aborted!");
        }
    
    CleanupStack::PopAndDestroy( &summaries );
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::GetActionSummariesL
// -----------------------------------------------------------------------------
//
void CVCXTestStatsKeeper::GetActionSummariesL( RPointerArray<CVCXTestStatsItemSummary>& aSummaries, TBool aFailActiveActions )
    {
    if( aFailActiveActions )
        {
        for( TInt i = 0; i < iActions.Count(); i++ )
            {
            CVCXTestStatsItem* item = iActions[i];
            if( item->iIsActive )
                {
                item->iError = KErrUnknown;
                item->iEndTime.HomeTime();
                item->iIsActive = EFalse;
                }
            }
        }
    
    for( TInt i = 0; i < iActions.Count(); i++ )
        {
        CVCXTestStatsItem* item = iActions[i];
        if( item->iIsActive )
            {
            continue;
            }
        
        CVCXTestStatsItemSummary* itemSummary( NULL );
        
        // Find summary item.
        for( TInt e = 0; e < aSummaries.Count(); e++ )
            {
            if( item->iDesc && aSummaries[e]->iDesc && *item->iDesc == *aSummaries[e]->iDesc )
                {
                itemSummary = aSummaries[e];
                break;
                }
            }

        // Not found, create new one.
        if( !itemSummary )
            {
            itemSummary = CVCXTestStatsItemSummary::NewL();
            itemSummary->iId = item->iId;
            itemSummary->iTotalTime = 0;
            itemSummary->iMaxTime = -1;
            itemSummary->iMinTime = -1;
            if( item->iDescId ) itemSummary->iDescId = item->iDescId->AllocL();
            itemSummary->iDesc = item->iDesc->AllocL();
            aSummaries.Append( itemSummary );
            }
        
        // Increase counts.
        itemSummary->iCount++;
        if( item->iError != KErrNone )
            {
            itemSummary->iFailCount++;
            }
        
        // Add item to stats.
        TTimeIntervalMicroSeconds runTime( 0 );
        runTime = item->iEndTime.MicroSecondsFrom( item->iStartTime );
        if( runTime.Int64() > 0 )
            {
            if( item->iError != KErrNone )
                {
                itemSummary->iFailTime += runTime.Int64();
                }
            itemSummary->iTotalTime += runTime.Int64();
            TReal run = (TReal)runTime.Int64() / 1000000;

            itemSummary->iMaxTime = 
                ( runTime.Int64() > itemSummary->iMaxTime || itemSummary->iMaxTime < 0 ) ? runTime.Int64() : itemSummary->iMaxTime;  
            itemSummary->iMinTime = 
                ( runTime.Int64() < itemSummary->iMinTime || itemSummary->iMinTime < 0) ? runTime.Int64() : itemSummary->iMinTime;
            }
        }
    
    // Calc avg times and check other times.
    for( TInt e = 0; e < aSummaries.Count(); e++ )
        {
        aSummaries[e]->iAvgTime = (TReal)aSummaries[e]->iTotalTime / (TReal)aSummaries[e]->iCount;
        aSummaries[e]->iMinTime = aSummaries[e]->iMinTime < 0 ? 0 : aSummaries[e]->iMinTime;
        aSummaries[e]->iMaxTime = aSummaries[e]->iMaxTime < 0 ? 0 : aSummaries[e]->iMaxTime;
        }
    
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::DataSizeL
// -----------------------------------------------------------------------------
//
TUint CVCXTestStatsKeeper::DataSizeL()
    {
    TUint size( 0 );
    
    size += iActions.Count() * sizeof( CVCXTestStatsItem );
    
    for( TInt i = 0; i < iActions.Count(); i++ )
        {
        CVCXTestStatsItem* item = iActions[i];
        if( item->iDescId ) size += item->iDescId->Size();
        size += item->iDesc->Size();
        
        size += item->iProgresses.Count() * sizeof( CVCXTestStatsItem );
        
        for( TInt e = 0; e < item->iProgresses.Count(); e++ )
            {
            size += item->iProgresses[e]->iDesc->Size();
            }
        
        if( item->iTraces.Count() > 0 )
            {
            for( TInt e = 0; e < item->iTraces.Count(); e++ )
                {
                size += item->iTraces[e]->Size();
                }
            }
        }
    return size/1024;
    }

// -----------------------------------------------------------------------------
// CVCXTestStatsKeeper::CheckFailsL
// -----------------------------------------------------------------------------
//
void CVCXTestStatsKeeper::CheckFailsL()
    {
    RPointerArray<CVCXTestStatsItemSummary> summaries;
    CleanupResetAndDestroyPushL( summaries );
    GetActionSummariesL( summaries, EFalse );
    
    const TReal KVCXTestStatsKeeperMaxFailPercent = 0.33;
    
    for( TInt i = 0; i < summaries.Count(); i++ )
        {
        // Check fails only when there's some actions done. 
        if( summaries[i]->iCount > 10 )
            {
            // Check if there's too many fails.
            TReal failPercent = (TReal)summaries[i]->iFailCount / (TReal)summaries[i]->iCount;
            
            if( failPercent > KVCXTestStatsKeeperMaxFailPercent )
                {
                iAbortedBecauseTooManyFails = ETrue;
                VCXLOGLO1("CVCXTestStatsKeeper:: Too many fails, leaving with KErrAbort");
                User::Leave( KErrAbort );                
                }
            
            const TInt64 KTest10Minutes = 1000000 * 60 * 10;
            
            // Check if failing cases take too long to run.
            if( summaries[i]->iTotalTime > 0 && summaries[i]->iFailTime > KTest10Minutes ) 
                {
                TReal failTimePercent = (TReal)summaries[i]->iFailTime / (TReal)summaries[i]->iTotalTime;
                if( failTimePercent > KVCXTestStatsKeeperMaxFailPercent )
                    {
                    VCXLOGLO1("CVCXTestStatsKeeper:: Too many fails, leaving with KErrAbort");
                    iAbortedBecauseTooManyFails = ETrue;
                    User::Leave( KErrAbort );
                    }
                }            
            }
        }
    CleanupStack::PopAndDestroy( &summaries );
    }

// End of File
