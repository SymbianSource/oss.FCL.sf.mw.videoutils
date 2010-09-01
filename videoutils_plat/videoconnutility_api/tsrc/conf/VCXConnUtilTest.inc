KVcxConnUtilTestModeSubscriber   0 // No connectivity functionality in this mode
KVcxConnUtilTestModeConnectivity 1 // No P&S property subscriptions, properties can be read 
KVcxConnUtilTestModeBoth         2 

// From vcxconnectionutility.hrh
EVCxNotConnected       10
EVCxConnecting         11
EVCxConnected          12
EVCxDisconnecting      13
EVCxRoamingRequest     14
EVCxRoamingAccepted    15
EVCxError              16

// From vcxconnectionutility.hrh
EVCxPSConnectionStatus      1
EVCxPSIapId                 2
EVCxPSSnapId                3 
EVCxPSMasterExists          4
EVCxPSNbrConnInstances      5
EVCxPSRoamingRequestStatus  6
EVCxPSNbrRoamResp           7
EVCxPSNbRoamAccepted        8

// From vcxconnectionutility.hrh
EVCxRoamingNotInit 50
EVCxRoamingAllowed 51

// From cmconnectionmethoddef.h
// Only part of attributes are listed here.
ECmName                501
ECmStartPage           503
ECmHidden              504 // Not supported yet
ECmProtected           505 // Needs NW ctrl cap
ECmNamingMethod        506
ECmSeamlessnessLevel   507

ECmMetaHighlight       600
ECmMetaHiddenAgent     601

ECmNextLayerIapId      602
ECmNextLayerSNAPId     603

ECmProxyUsageEnabled   1000 // Proxy record is deleted on UpdateL() if ECmProxyServerName is an empty string.
ECmProxyServerName     1001
ECmProxyProtocolName   1002
ECmProxyPortNumber     1003
ECmProxyExceptions     1004

ECmIsLinked            6001

DEST_TEST              "dest_test_connutil"
DEST_TEST2             "dest_test_connutil2"


AP1		       "test_ap_1"
AP2		       "test_ap_2"
AP3		       "test_ap_3"
AP4		       "test_ap_4"
AP5		       "test_ap_5"
AP6		       "test_ap_6"
AP7		       "test_ap_7"
AP8		       "test_ap_8"
AP9		       "test_ap_9"
AP10		       "test_ap_10"

