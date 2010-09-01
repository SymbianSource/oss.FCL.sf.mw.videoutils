/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __VIDEOSERVICEUTILSLOGGER_H__
#define __VIDEOSERVICEUTILSLOGGER_H__

#include <e32debug.h>
#include <flogger.h>

_LIT( KDirectory, "livetv" );
_LIT( KFileName, "livetv2.log" );
_LIT( KTimeDateFormat,"%D%M%Y%/0%1%/1%2%/2%3%/3 %-B%:0%J%:1%T%:2%S%:3%+B");

//#define LIVE_TV_RDEBUG_TRACE
#ifdef _DEBUG
#define LIVE_TV_FILE_TRACE
#endif // _DEBUG
#ifdef LIVE_TV_RDEBUG_TRACE
  #define LIVE_TV_TRACE1(a) RDebug::Print(a)
  #define LIVE_TV_TRACE2(a,b) RDebug::Print(a,b)
  #define LIVE_TV_TRACE3(a,b,c) RDebug::Print(a,b,c)
  #define LIVE_TV_TRACE4(a,b,c,d) RDebug::Print(a,b,c,d)
  #define LIVE_TV_TRACE5(a,b,c,d,e) RDebug::Print(a,b,c,d,e)
  #define LOG(AAA) { LIVE_TV_TRACE1(_L(AAA)); }
  #define LOG1(AAA,BBB) { LIVE_TV_TRACE2(_L(AAA),BBB); }
  #define LOG2(AAA,BBB,CCC) { LIVE_TV_TRACE3(_L(AAA),BBB,CCC); }
  #define LOG3(AAA,BBB,CCC,DDD) { LIVE_TV_TRACE4(_L(AAA),BBB,CCC,DDD); }
  #define LOG4(AAA,BBB,CCC,DDD,EEE) { LIVE_TV_TRACE4(_L(AAA),BBB,CCC,DDD,EEE); }
#else
  #ifdef LIVE_TV_FILE_TRACE
    #define LIVE_TV_TRACE1(a) { RFileLogger::Write( KDirectory, KFileName, EFileLoggingModeAppend, a ); }
    #define LIVE_TV_TRACE2(a,b) { RFileLogger::WriteFormat( KDirectory, KFileName, EFileLoggingModeAppend, a, b );}
    #define LIVE_TV_TRACE3(a,b,c) { RFileLogger::WriteFormat( KDirectory, KFileName, EFileLoggingModeAppend, a, b, c ); }
    #define LIVE_TV_TRACE4(a,b,c,d) { RFileLogger::WriteFormat( KDirectory, KFileName, EFileLoggingModeAppend, a, b, c, d ); }
    #define LIVE_TV_TRACE5(a,b,c,d,e) { RFileLogger::WriteFormat( KDirectory, KFileName, EFileLoggingModeAppend, a, b, c, d, e ); }
    #define LOG(AAA) { LIVE_TV_TRACE1(_L(AAA)); }
    #define LOG1(AAA,BBB) { LIVE_TV_TRACE2(_L(AAA),BBB); }
    #define LOG2(AAA,BBB,CCC) { LIVE_TV_TRACE3(_L(AAA),BBB,CCC); }
    #define LOG3(AAA,BBB,CCC,DDD) { LIVE_TV_TRACE4(_L(AAA),BBB,CCC,DDD); }
    #define LOG4(AAA,BBB,CCC,DDD,EEE) { LIVE_TV_TRACE4(_L(AAA),BBB,CCC,DDD,EEE); }
  #else
    #define LIVE_TV_TRACE1(a)
    #define LIVE_TV_TRACE2(a,b)
    #define LIVE_TV_TRACE3(a,b,c)
    #define LIVE_TV_TRACE4(a,b,c,d)
    #define LIVE_TV_TRACE5(a,b,c,d,e)
    #define LOG(AAA)
    #define LOG1(AAA,BBB)
    #define LOG2(AAA,BBB,CCC)
    #define LOG3(AAA,BBB,CCC,DDD)
    #define LOG4(AAA,BBB,CCC,DDD,EEE)
  #endif
#endif
	
#endif// __VIDEOSERVICEUTILSLOGGER_H__

// End of file
