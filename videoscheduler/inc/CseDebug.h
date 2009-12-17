/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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





#ifndef CSEDEBUG_H
#define CSEDEBUG_H

#include <flogger.h>



// Define logging level here

// Minimum log output
#define CSE_HIGH_LEVEL_TRACES

#define CSE_MIDDLE_LEVEL_TRACES

// Maximum log output
#define CSE_LOW_LEVEL_TRACES




/*
-----------------------------------------------------------------------------

	LOGGING MACROs

	USE THESE MACROS IN YOUR CODE

	Usage:
	
	Example: CSELOGSTRING_HIGH_LEVEL ("Test");
	Example: CSELOGSTRING2_HIGH_LEVEL ("Test %d", aValue);
	Example: CSELOGSTRING3_HIGH_LEVEL ("Test %d %d", aValue1, aValue2);
	Example: CSELOGSTRING4_HIGH_LEVEL ("Test %d %d %d", aValue1, aValue2, aValue3);

	Example: CSE_LOG_STR_DESC_HIGH_LEVEL (own_desc);
	Example: CSE_LOG_STR_DESC2_HIGH_LEVEL (own_desc, aValue));
	Example: CSE_LOG_STR_DESC3_HIGH_LEVEL (own_desc, aValue, aValue2);
	Example: CSE_LOG_STR_DESC4_HIGH_LEVEL (own_desc, aValue, aValue2, aValue3);

-----------------------------------------------------------------------------
*/



#ifdef CSE_HIGH_LEVEL_TRACES

#define CSELOGTEXT_HIGH_LEVEL(AAAA)                     CSE_LOGTEXT(AAAA)
#define CSELOGSTRING_HIGH_LEVEL(AAAA) 	                CSE_LOGSTRING(AAAA) 
#define CSELOGSTRING2_HIGH_LEVEL(AAAA,BBBB) 	        CSE_LOGSTRING2(AAAA,BBBB) 
#define CSELOGSTRING3_HIGH_LEVEL(AAAA,BBBB,CCCC)        CSE_LOGSTRING3(AAAA,BBBB,CCCC) 
#define CSELOGSTRING4_HIGH_LEVEL(AAAA,BBBB,CCCC,DDDD)   CSE_LOGSTRING4(AAAA,BBBB,CCCC,DDDD) 

#define CSE_LOG_STR_DESC_HIGH_LEVEL(AAAA) 	                CSE_LOG_STR_DESC(AAAA) 
#define CSE_LOG_STR_DESC2_HIGH_LEVEL(AAAA,BBBB) 	        CSE_LOG_STR_DESC2(AAAA,BBBB) 
#define CSE_LOG_STR_DESC3_HIGH_LEVEL(AAAA,BBBB,CCCC)        CSE_LOG_STR_DESC3(AAAA,BBBB,CCCC) 
#define CSE_LOG_STR_DESC4_HIGH_LEVEL(AAAA,BBBB,CCCC,DDDD)   CSE_LOG_STR_DESC4(AAAA,BBBB,CCCC,DDDD) 

#else

#define CSELOGTEXT_HIGH_LEVEL(AAAA)
#define CSELOGSTRING_HIGH_LEVEL(AAAA)
#define CSELOGSTRING2_HIGH_LEVEL(AAAA,BBBB)
#define CSELOGSTRING3_HIGH_LEVEL(AAAA,BBBB,CCCC)
#define CSELOGSTRING4_HIGH_LEVEL(AAAA,BBBB,CCCC,DDDD)

#define CSE_LOG_STR_DESC_HIGH_LEVEL(AAAA)
#define CSE_LOG_STR_DESC2_HIGH_LEVEL(AAAA,BBBB)
#define CSE_LOG_STR_DESC3_HIGH_LEVEL(AAAA,BBBB,CCCC)
#define CSE_LOG_STR_DESC4_HIGH_LEVEL(AAAA,BBBB,CCCC,DDDD)

#endif


#ifdef CSE_MIDDLE_LEVEL_TRACES
#define CSELOGSTRING_MIDDLE_LEVEL(AAAA)                 CSE_LOGSTRING(AAAA)
#define CSELOGSTRING2_MIDDLE_LEVEL(AAAA,BBBB)           CSE_LOGSTRING2(AAAA,BBBB)
#define CSELOGSTRING3_MIDDLE_LEVEL(AAAA,BBBB,CCCC)      CSE_LOGSTRING3(AAAA,BBBB,CCCC)
#define CSELOGSTRING4_MIDDLE_LEVEL(AAAA,BBBB,CCCC,DDDD) CSE_LOGSTRING4(AAAA,BBBB,CCCC,DDDD)

#define CSE_LOG_STR_DESC_MIDDLE_LEVEL(AAAA) 	            CSE_LOG_STR_DESC(AAAA) 
#define CSE_LOG_STR_DESC2_MIDDLE_LEVEL(AAAA,BBBB) 	        CSE_LOG_STR_DESC2(AAAA,BBBB) 
#define CSE_LOG_STR_DESC3_MIDDLE_LEVEL(AAAA,BBBB,CCCC)      CSE_LOG_STR_DESC3(AAAA,BBBB,CCCC) 
#define CSE_LOG_STR_DESC4_MIDDLE_LEVEL(AAAA,BBBB,CCCC,DDDD) CSE_LOG_STR_DESC4(AAAA,BBBB,CCCC,DDDD) 

#else

#define CSELOGSTRING_MIDDLE_LEVEL(AAAA)
#define CSELOGSTRING2_MIDDLE_LEVEL(AAAA,BBBB)
#define CSELOGSTRING3_MIDDLE_LEVEL(AAAA,BBBB,CCCC)
#define CSELOGSTRING4_MIDDLE_LEVEL(AAAA,BBBB,CCCC,DDDD)

#define CSE_LOG_STR_DESC_MIDDLE_LEVEL(AAAA)
#define CSE_LOG_STR_DESC2_MIDDLE_LEVEL(AAAA,BBBB)
#define CSE_LOG_STR_DESC3_MIDDLE_LEVEL(AAAA,BBBB,CCCC)
#define CSE_LOG_STR_DESC4_MIDDLE_LEVEL(AAAA,BBBB,CCCC,DDDD)

#endif
	    

#ifdef CSE_LOW_LEVEL_TRACES

#define CSELOGTEXT_LOW_LEVEL(AAAA)                      CSE_LOGTEXT(AAAA)
#define CSELOGSTRING_LOW_LEVEL(AAAA)                    CSE_LOGSTRING(AAAA)
#define CSELOGSTRING2_LOW_LEVEL(AAAA,BBBB)              CSE_LOGSTRING2(AAAA,BBBB)
#define CSELOGSTRING3_LOW_LEVEL(AAAA,BBBB,CCCC)         CSE_LOGSTRING3(AAAA,BBBB,CCCC)
#define CSELOGSTRING4_LOW_LEVEL(AAAA,BBBB,CCCC,DDDD)    CSE_LOGSTRING4(AAAA,BBBB,CCCC,DDDD)

#define CSE_LOG_STR_DESC_LOW_LEVEL(AAAA) 	                CSE_LOG_STR_DESC(AAAA) 
#define CSE_LOG_STR_DESC2_LOW_LEVEL(AAAA,BBBB) 	            CSE_LOG_STR_DESC2(AAAA,BBBB) 
#define CSE_LOG_STR_DESC3_LOW_LEVEL(AAAA,BBBB,CCCC)         CSE_LOG_STR_DESC3(AAAA,BBBB,CCCC) 
#define CSE_LOG_STR_DESC4_LOW_LEVEL(AAAA,BBBB,CCCC,DDDD)    CSE_LOG_STR_DESC4(AAAA,BBBB,CCCC,DDDD) 

#else

#define CSELOGTEXT_LOW_LEVEL(AAAA)
#define CSELOGSTRING_LOW_LEVEL(AAAA)
#define CSELOGSTRING2_LOW_LEVEL(AAAA,BBBB)
#define CSELOGSTRING3_LOW_LEVEL(AAAA,BBBB,CCCC)
#define CSELOGSTRING4_LOW_LEVEL(AAAA,BBBB,CCCC,DDDD)

#define CSE_LOG_STR_DESC_LOW_LEVEL(AAAA)
#define CSE_LOG_STR_DESC2_LOW_LEVEL(AAAA,BBBB)
#define CSE_LOG_STR_DESC3_LOW_LEVEL(AAAA,BBBB,CCCC)
#define CSE_LOG_STR_DESC4_LOW_LEVEL(AAAA,BBBB,CCCC,DDDD)

#endif



/* 
-----------------------------------------------------------------------------

	LOG SELECTION

-----------------------------------------------------------------------------
*/

// 0 = No logging, 
// 1 = Flogger, 
// 2 = RDebug


#ifndef _DEBUG

// UREL BUILD:
#define CSE_LOGGING_METHOD  0   // No logging in UREL builds

#else // urel



#ifdef __WINSCW__

// WINSCW BUILD:
#define CSE_LOGGING_METHOD  2  // RDebug is default with emulator

#else

// ARMV5 BUILD:
#define CSE_LOGGING_METHOD  1  // Flogger is default with target device

#endif
                                  
                                 

#endif // _DEBUG




/* 
-----------------------------------------------------------------------------

	LOG SETTINGS

-----------------------------------------------------------------------------
*/

#if CSE_LOGGING_METHOD == 1      // Flogger

#include <flogger.h>
_LIT(KCSELogFolder,"CSE");
_LIT(KCSELogFile,"CSELOG.TXT");

#elif CSE_LOGGING_METHOD == 2    // RDebug

#include <e32svr.h>

#endif





#if CSE_LOGGING_METHOD == 1      // Flogger


#define CSE_LOGTEXT(AAA)                        RFileLogger::Write(KCSELogFolder(),KCSELogFile(),EFileLoggingModeAppend, AAA)

#define CSE_LOG_STR_DESC(AAA)                   RFileLogger::Write(KCSELogFolder(),KCSELogFile(),EFileLoggingModeAppend, AAA)
#define CSE_LOG_STR_DESC2(AAA,BBB)         do { RFileLogger::WriteFormat(KCSELogFolder(),KCSELogFile(),EFileLoggingModeAppend,TRefByValue<const TDesC>(AAA()),BBB); } while (0)
#define CSE_LOG_STR_DESC3(AAA,BBB,CCC)     do { RFileLogger::WriteFormat(KCSELogFolder(),KCSELogFile(),EFileLoggingModeAppend,TRefByValue<const TDesC>(AAA()),BBB,CCC); } while (0)
#define CSE_LOG_STR_DESC4(AAA,BBB,CCC,DDD) do { RFileLogger::WriteFormat(KCSELogFolder(),KCSELogFile(),EFileLoggingModeAppend,TRefByValue<const TDesC>(AAA()),BBB,CCC,DDD); } while (0)

#define CSE_LOGSTRING(AAA)                 do { _LIT(tempCSELogDes,AAA); RFileLogger::Write(KCSELogFolder(),KCSELogFile(),EFileLoggingModeAppend,tempCSELogDes()); } while (0)
#define CSE_LOGSTRING2(AAA,BBB)            do { _LIT(tempCSELogDes,AAA); RFileLogger::WriteFormat(KCSELogFolder(),KCSELogFile(),EFileLoggingModeAppend,TRefByValue<const TDesC>(tempCSELogDes()),BBB); } while (0)
#define CSE_LOGSTRING3(AAA,BBB,CCC)        do { _LIT(tempCSELogDes,AAA); RFileLogger::WriteFormat(KCSELogFolder(),KCSELogFile(),EFileLoggingModeAppend,TRefByValue<const TDesC>(tempCSELogDes()),BBB,CCC); } while (0)
#define CSE_LOGSTRING4(AAA,BBB,CCC,DDD)    do { _LIT(tempCSELogDes,AAA); RFileLogger::WriteFormat(KCSELogFolder(),KCSELogFile(),EFileLoggingModeAppend,TRefByValue<const TDesC>(tempCSELogDes()),BBB,CCC,DDD); } while (0)

// Time stamp
_LIT(KCseLogTimeFormatString, "CSE TIMESTAMP: %H:%T:%S:%*C3");
#define CSE_LOGSTRING_TIMESTAMP { \
                                TTime logTime; \
                                logTime.HomeTime(); \
                                TBuf<256> logBuffer; \
                                logTime.FormatL(logBuffer, KCseLogTimeFormatString); \
                                RFileLogger::Write(KCSELogFolder(), KCSELogFile(), EFileLoggingModeAppend, logBuffer); \
                                }

// Memory stamp
_LIT(KCseLogMemoryStampString, "CSE MEMORYSTAMP: %d KB");
#define CSE_LOGSTRING_MEMORYSTAMP { \
                                  User::CompressAllHeaps(); \
                                  TMemoryInfoV1Buf logMemory; \
                                  UserHal::MemoryInfo(logMemory); \
                                  TInt logMemoryInt = (TInt)(logMemory().iFreeRamInBytes); \
                                  TBuf<256> logMemoryStr; \
                                  logMemoryStr.Format(KCseLogMemoryStampString, (logMemoryInt / 1024) ); \
                                  RFileLogger::Write(KCSELogFolder(), KCSELogFile(), EFileLoggingModeAppend, logMemoryStr); \
                                  }


#elif CSE_LOGGING_METHOD == 2    // RDebug


#define CSE_LOGTEXT(AAA)                    RDebug::Print(AAA)

#define CSE_LOG_STR_DESC(AAA)               RDebug::Print(AAA)
#define CSE_LOG_STR_DESC2(AAA,BBB)          do {  RDebug::Print(AAA, BBB); }            while (0)
#define CSE_LOG_STR_DESC3(AAA,BBB,CCC)      do {  RDebug::Print(AAA, BBB, CCC); }       while (0)
#define CSE_LOG_STR_DESC4(AAA,BBB,CCC,DDD)  do {  RDebug::Print(AAA, BBB, CCC, DDD); }  while (0)

#define CSE_LOGSTRING(AAA)                  do { _LIT(tempCSELogDes,AAA); RDebug::Print(tempCSELogDes); }                   while (0)
#define CSE_LOGSTRING2(AAA,BBB)             do { _LIT(tempCSELogDes,AAA); RDebug::Print(tempCSELogDes, BBB); }              while (0)
#define CSE_LOGSTRING3(AAA,BBB,CCC)         do { _LIT(tempCSELogDes,AAA); RDebug::Print(tempCSELogDes, BBB, CCC); }         while (0)
#define CSE_LOGSTRING4(AAA,BBB,CCC,DDD)     do { _LIT(tempCSELogDes,AAA); RDebug::Print(tempCSELogDes, BBB, CCC, DDD); }    while (0)

// Time stamp
_LIT(KCseLogTimeFormatString, "CSE TIMESTAMP: %H:%T:%S:%*C3");
#define CSE_LOGSTRING_TIMESTAMP { \
                                TTime logTime; \
                                logTime.HomeTime(); \
                                TBuf<256> logBuffer; \
                                logTime.FormatL(logBuffer, KCseLogTimeFormatString); \
                                RDebug::Print(logBuffer); \
                                }

// Memory stamp
_LIT(KCseLogMemoryStampString, "CSE MEMORYSTAMP: %d KB");
#define CSE_LOGSTRING_MEMORYSTAMP { \
                                  User::CompressAllHeaps(); \
                                  TMemoryInfoV1Buf logMemory; \
                                  UserHal::MemoryInfo(logMemory); \
                                  TInt logMemoryInt = (TInt)(logMemory().iFreeRamInBytes); \
                                  TBuf<256> logMemoryStr; \
                                  logMemoryStr.Format(KCseLogMemoryStampString, (logMemoryInt / 1024) ); \
                                  RDebug::Print(logMemoryStr); \
                                  }


#else	// TF_LOGGING_METHOD == 0 or invalid


#define CSE_LOGSTRING(AAA)              
#define CSE_LOGSTRING2(AAA,BBB)         
#define CSE_LOGSTRING3(AAA,BBB,CCC)     
#define CSE_LOGSTRING4(AAA,BBB,CCC,DDD) 

#define CSE_LOGTEXT(AAA)                

#define CSE_LOG_STR_DESC(AAA)              
#define CSE_LOG_STR_DESC2(AAA,BBB)         
#define CSE_LOG_STR_DESC3(AAA,BBB,CCC)     
#define CSE_LOG_STR_DESC4(AAA,BBB,CCC,DDD) 

#define CSE_LOGSTRING_TIMESTAMP
#define CSE_LOGSTRING_MEMORYSTAMP


#endif  // TF_LOGGING_METHOD







#endif // CSEDEBUG_H

// End of File
