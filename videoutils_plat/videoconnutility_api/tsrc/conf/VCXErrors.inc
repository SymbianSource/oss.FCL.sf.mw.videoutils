KIptvTestErrNotFound                -1111
KIptvTestErrVerifyFailed            -1112

KERRNONE                                        0         
KERRNOTFOUND                                    -1        
KERRGENERAL                                     -2        
KERRCANCEL                                      -3
KERRNOMEMORY                                    -4        
KERRNOTSUPPORTED                                -5        
KERRARGUMENT                                    -6        
KERRBADHANDLE                                   -8
KERROVERFLOW                                    -9        
KERRUNDERFLOW                                   -10        
KERRALREADYEXISTS                               -11
KERRINUSE                                       -14
KERRSERVERBUSY                                  -16
KERRNOTREADY                                    -18
KERRUNKNOWN                                     -19
KERRCORRUPT                                     -20
KERRLOCKED                                      -22
KERRDISKFULL                                    -26
KERRBADNAME                                     -28
KERRTIMEDOUT                                    -33
KERRABORT                                       -39
KERRTOOBIG                                      -40
KErrCouldNotConnect                             -34


// EPG manager server side error codes
KIptvErrorGetUpdateInformationLNotFound           123
KIptvErrorEpgUpdateFailed                         133
KIptvErrorEpgUpdateSuccessed                      134
KIptvVodUpdateNotStarted                          137
KIptvVodUpdateStarted                             136
KIptvErrorVodNoIap                                139
KIptvErrorNoService                               140
KIptvThumbnailCount                               141
KIptvContentUpdateCompleted                       146
KIptvServiceThumbnailDownloaded                   143
KIptvContentThumbnailDownloaded                   144
KIptvRssParserError                               145

KIptvErrorRssSearchStarted                        170
KIptvErrorRssSearchFailed                         171
KIptvErrorRssSearchSucceed                        172
KIptvErrorRssSearchNoIap                          173

EIptvDlNoError                           0
EIptvDlOutOfMemory                       1
EIptvDlOutOfMemoryInVodDlPluginCreate    2
EIptvDlVodDlPluginNotFound               3
EIptvDlGeneralErrorInVodDlPluginCreate   4
EIptvDlMessageAlreadyPending             5
EIptvDlTypeNotSupported                  6
EIptvDlMaxDownloadsExceeded              7
EIptvDlAlreadyExists                     8
KIptvDlCouldNotGetServiceData            9
EIptvDlEpgManagerNotReady                10
EIptvDlCouldNotGetContentFullDetails     11
EIptvDlFailedToAddNewVideoToMyVideos     12
EIptvDlCouldNotConnectToS60DlMgr         13
EIptvDlCouldNotSetIapAttrToS60DlMgr      14
EIptvDlCouldNotCreateS60Download         15
EIptvDlCouldNotStartS60Download          16
EIptvDlConnectionFailed                  17
EIptvDlAuthFailed                        18
EIptvDlProxyAuthFailed                   19
EIptvDlContentNotFound                   20
EIptvDlDiskFull                          21
EIptvDlDestFileInUse                     22
EIptvDlBadUrl                            23
EIptvDlMmcRemoved                        24
EIptvDlGeneral                           25
EIptvDlDisconnected                      26
EIptvDlContentUnchanged                  27

KIptvSchemaMismatch                      301
KIptvInvalidServiceType                  302
EXmlNoElements                           -997
KErrNotFound                             -1

KErrGsmMMServiceOptionTemporaryOutOfOrder 	 -4162

// From HttpDownloadMgrCommon.h
EGeneral -30421
EInternal -30422
EContentFileIntegrity -30423
EDiskFull -30424
EConnectionFailed -30425
ETransactionFailed -30426
EMoveFailed -30427
EDestFileWriteFailed -30428
EMMCRemoved -30429
EBadUrl -30430
EWrongDestFilename -30431
EDestFileInUse -30432
EHttpUnhandled -30433
EHttpAuthenticationFailed -30434
EProxyAuthenticationFailed -30435
EObjectNotFound -30436
EPartialContentModified -30437
EContentExpired -30438
EHttpRestartFailed  -30450
