/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/

#ifndef _QUEC_ATCMD_DEF_UNISOC_H_
#define _QUEC_ATCMD_DEF_UNISOC_H_

#ifdef CONFIG_QUEC_PROJECT_FEATURE
//以下AT请勿裁剪,需要用于调试
I,              atCmdHandleI, 0
+GSN,			atCmdHandleGSN, 0
+CGSN,			atCmdHandleCGSN, 0
+CFUN,			atCmdHandleCFUN, 0
+CSQ,			atCmdHandleCSQ, 0
+CREG,			atCmdHandleCREG, 0
+EGMR,			atCmdHandleEGMR, 0
+CGPADDR,		atCmdHandleCGPADDR, 0
+CGDCONT,		atCmdHandleCGDCONT, 0
+CGACT, 		atCmdHandleCGACT, 0
#ifdef CONFIG_QUEC_PROJECT_FEATURE_VOICE_CALL
//volte音频校准AT
+SUPS,			atCmdHandleSUPS, AT_CON_NOT_CLAC
+SADM,			atCmdHandleSADM, AT_CON_NOT_CLAC
+PEINFO,		atCmdHandlePEINFO, AT_CON_NOT_CLAC
+SPADCVS,		atCmdHandleSPADCVS, AT_CON_NOT_CLAC
+SPENHA,		atCmdHandleSPENHA, AT_CON_NOT_CLAC
+SADMDSP,		atCmdHandleSADMDSP, AT_CON_NOT_CLAC
+CINGAIN,		atCmdHandleCINGAIN, AT_CON_NOT_CLAC
+COUTGAIN,		atCmdHandleCOUTGAIN, AT_CON_NOT_CLAC
+CRECSR,		atCmdHandleCRECSR, AT_CON_NOT_CLAC
#endif
#endif

//以下AT可以根据需要来裁剪,打开CONFIG_QUEC_PROJECT_AT_CUT后,
//下方所有AT都会被裁剪,用户也可以根据需要自行保留需要的AT
#ifndef CONFIG_QUEC_PROJECT_AT_CUT
#ifdef CONFIG_AT_CMD_SUPPORT
// V.250
Z,              atCmdHandleZ, 0             // (6.1.1) Reset to default configuration
&F,             atCmdHandleAndF, 0          // (6.1.2) Set to factory defined configuration
+GMI,           atCmdHandleGMI, 0           // (6.1.4) Request manufacturer identification
+GMM,           atCmdHandleGMM, 0           // (6.1.5) Request model identification
+GMR,           atCmdHandleGMR, 0           // (6.1.6) Request revision identification
S3,             atCmdHandleS3, 0            // (6.2.1) Command line termination character
S4,             atCmdHandleS4, 0            // (6.2.2) Response formatting character
S5,             atCmdHandleS5, 0            // (6.2.3) Command line editing character
E,              atCmdHandleE, 0             // (6.2.4) Command echo
Q,              atCmdHandleQ, 0             // (6.2.5) Result code suppression
V,              atCmdHandleV, 0             // (6.2.6) DCE response format
X,              atCmdHandleX, 0             // (6.2.7) Result code selection and call progress monitoring control
&D,             atCmdHandleAndD, 0          // (6.2.9) Circuit 108/2 (DTR) behavior
+IPR,           atCmdHandleIPR, 0           // (6.2.10) Fixed DTE rate
+ICF,           atCmdHandleICF, 0           // (6.2.11) DTE-DCE character framing
+IFC,           atCmdHandleIFC, 0           // (6.2.12) DTE-DCE local flow control

#if defined (CONFIG_AT_CC_COMMAND_DAILUP_SUPPORT) || defined(LTE_NBIOT_SUPPORT)
D,              atCmdHandleD, 0             // (6.3.1) Dial
H,              atCmdHandleH, 0             // (6.3.6) Hook control
#else
H,              atCmdHandleH, AT_CON_NOT_CLAC             // (6.3.6) Hook control
D,              atCmdHandleD, AT_CON_NOT_CLAC             // (6.3.1) Dial
#endif


#if defined(CONFIG_AT_CC_COMMAND_SUPPORT) && defined(CONFIG_QUEC_PROJECT_FEATURE_VOLTE)
A,              atCmdHandleA, 0             // (6.3.5) Answer
+CMOD,          atCmdHandleCMOD, 0          // 6.4 Call mode
+CHUP,          atCmdHandleCHUP, 0          // 6.5 Hangup call
+CHLD,          atCmdHandleCHLD, 0          // 7.13 Call related supplementary services
+CDU,           atCmdHandleCDU, 0           // 13.2.3 Hangup of current calls
+CHCCS,         atCmdHandleCHCCS, 0         // 13.2.3 Hangup of current calls
+CPAS,          atCmdHandleCPAS, 0          // 8.1 Phone activity status
#endif

#ifdef CONFIG_AT_GC_COMMAND_SUPPORT
S0,             atCmdHandleS0, 0            // (6.3.8) Automatic answer
#endif

#ifdef CONFIG_DTMF_KEY_DETECT_SUPPORT
+DVT,           atCmdHandleDVT, 0           // Detect Voice Tone
#endif

#if defined(CFW_VOLTE_SUPPORT) || defined(CONFIG_GSM_SUPPORT)
+VTS,           atCmdHandleVTS, 0           // C.2.11 DTMF and tone generation
+VTD,           atCmdHandleVTD, 0           // C.2.12 Tone duration
+CR,            atCmdHandleCR, 0            // 6.9 Service reporting control
+CRC,           atCmdHandleCRC, 0           // 6.11 Cellular result codes
+CSTA,          atCmdHandleCSTA, 0
#endif

#ifdef CFW_VOLTE_SUPPORT
+SETVOLTE,      atCmdHandleSETVOLTE, 0
+CIREG,         atCmdHandleCIREG, 0
+CCFCU,         atCmdHandleCCFCU, 0         // 7.35 Communication forwarding number and conditions with URI support
+CEUS,          atCmdHandleCEUS, 0          // 10.1.34 UE's usage setting for EPS
+CEVDP,         atCmdHandleCEVDP, 0         // 10.1.35 UE's voice domain preference E-UTRAN
+CFGIMSPDN,     atCmdHandleCFGIMSPDN, 0
+CFGXCAP,       atCmdHandleCFGXCAP, 0
#ifdef CONFIG_GSM_SUPPORT
+CSFBMT, 		atCmdHandleCSFBMT, 0
+CISRVCC,       atCmdHandleCISRVCC, 0       // 8.63 IMS single radio voice call continuity
#endif
#endif

O,              atCmdHandleO, 0             // (6.3.7) Return to online data state

&W,             atCmdHandleAndW, 0          // Store current profile
&Y,             atCmdHandleAndY, 0          // Set default reset profile

#ifndef CONFIG_QUEC_PROJECT_FEATURE
+CLAC,          atCmdHandleCLAC, 0          // List all command
+URC,           atCmdHandleURC, AT_CON_NOT_CLAC	    // Set Urc Enable/disable
#ifdef CONFIG_AT_CAMERA_SUPPORT
+CAM,           atCmdHandleCAM, 0           //Camera Function
#ifdef CONFIG_SOC_8850
+BACKLIGHT,     atCmdHandleBKLED, AT_CON_NOT_CLAC           //Backlight Function
#endif
#endif
#if defined(CONFIG_SOC_8910)||defined(CONFIG_SOC_8850)
+TUEINFO,       atCmdHandleTUEINFO, 0       // Show Tue status info
#endif
#ifdef CONFIG_AT_SSIM_SUPPORT
+SCANRSSI,      atCmdHandleSCANRSSI, 0      // Scan RSSI according to band
#endif
#endif

+DPSD,          atCmdHandleDiscardPSData, 0
//
// 3GPP TS 27.005 (R14)
//
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
#ifdef CONFIG_QUEC_PROJECT_FEATURE_SMS_AT
#ifndef CONFIG_SOC_8811
+CSDH,          atCmdHandleCSDH, 0          // 3.3.3 Show Text Mode Parameters
+CMGC,          atCmdHandleCMGC, 0          // 3.5.5/4.5 Send Command
+CMMS,          atCmdHandleCMMS, 0          // 3.5.6 More Messages to Send
#endif
#ifdef CONFIG_AT_SMS_COMMAND_SUPPORT
+CNMI,          atCmdHandleCNMI, 0          // 3.4.1 New Message Indications to TE
+CSMS,          atCmdHandleCSMS, 0          // 3.2.1 Select Message Service
+CPMS,          atCmdHandleCPMS, 0          // 3.2.2 Preferred Message Storage
+CMGF,          atCmdHandleCMGF, 0          // 3.2.3 Message Format
+CSCA,          atCmdHandleCSCA, 0          // 3.3.1 Service Centre Address
+CSMP,          atCmdHandleCSMP, 0          // 3.3.2 Set Text Mode Parameters
+CMGL,          atCmdHandleCMGL, 0          // 3.4.2/4.1 List Messages
+CMGR,          atCmdHandleCMGR, 0          // 3.4.3/4.2 Read Message
+CNMA,          atCmdHandleCNMA, 0          // 3.4.4 New Message Acknowledgement to ME/TA
+CMGS,          atCmdHandleCMGS, 0          // 3.5.1/4.3 Send Message
+CMSS,          atCmdHandleCMSS, 0          // 3.5.2/4.7 Send Message from Storage
+CMGW,          atCmdHandleCMGW, 0          // 3.5.3/4.4 Write Message to Memory
+CMGD,          atCmdHandleCMGD, 0          // 3.5.4 Delete Message
#endif
#ifdef CONFIG_AT_SMS_CB_SUPPORT
+CSCB,			atCmdHandleCSCB, 0			// 3.3.4 Select Cell Broadcast Message Types
+CSAS,			atCmdHandleCSAS, 0			// 3.3.5 Save Settings
+CRES,			atCmdHandleCRES, 0			// 3.3.6 Restore Settings
#endif
#ifndef CONFIG_ATR_NBONLY_API_SUPPORT
+CGSEND,        atCmdHandleCGSEND, 0
#endif
#endif
#endif

//
// 3GPP TS 27.007 (R14)
//
+CGMI,          atCmdHandleCGMI, 0          // 5.1 Request manufacturer identification
+CGMM,          atCmdHandleCGMM, 0          // 5.2 Request model identification
+CGMR,          atCmdHandleCGMR, 0          // 5.3 Request revision identification
+CSCS,          atCmdHandleCSCS, 0          // 5.5 Select TE character set
+CIMI,          atCmdHandleCIMI, 0          // 5.6 Request international mobile subscriber identity
+CEID,           atCmdHandleEID, AT_CON_NOT_CLAC           // get eid

#ifdef CONFIG_SOC_8910
+CIMIM,         atCmdHandleCIMIM, AT_CON_NOT_CLAC          // read cdma imsi
#endif

#ifdef CONFIG_GSM_SUPPORT
+SGGO,         atCmdHandleSGGO, 0         //if gsmGprsOnly is 1,we will not register for GSM-only network.
#endif
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
#ifdef CONFIG_ATR_CMUX_SUPPORT
+CMUX,          atCmdHandleCMUX, 0          // 5.7 Multiplexing mode
#endif
#endif
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+CGBV,          atCmdHandleCGBV, 0
#endif
#if defined (LTE_NBIOT_SUPPORT) || defined (LTE_SUPPORT)
#if defined (CONFIG_AT_PSM_SUPPORT)
+CPSMS,         atCmdHandleCPSMS, 0         // 7.38 Power saving mode setting
+CEDRXS,        atCmdHandleCEDRXS, 0        // 7.40 eDRX setting
+CEDRXRDP,      atCmdHandleCEDRXRDP, 0      // 7.41 eDRX read dynamic parameters
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+CFGEDRX,       atCmdHandleCFGEDRX, 0
#endif
+PSME,          atCmdHandlePSME, AT_CON_NOT_CLAC
#endif
#endif

// SIM && PBK related commands
//
+CSIM,         atCmdHandleCSIM, 0
+CCHO,         atCmdHandleCCHO, 0
+CCHC,         atCmdHandleCCHC, 0
+CGLA,         atCmdHandleCGLA, 0
+CPIN2,         atCmdHandleCPIN2, 0         // Authentication(for Sim)
^CPINC,         atCmdHandleCPINC, 0         // Total times of Access the Sim Card
+CRSML,         atCmdHandleCRSML, 0         // Read records of EF file on (U)SIM
#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifndef CONFIG_SOC_8811
^SIMOPERTIMES,  atCmdHandleSIMOperationTimes,   0       //Get the total number of SIM card operations.
#endif
+SIMCON,       atCmdHandleSIMCON,AT_CON_NOT_CLAC
+SIMCNT,       atCmdHandleSIMCNT,AT_CON_NOT_CLAC
#endif


// Network service related commands
//
#ifdef CONFIG_AT_CUS_SUPPORT
+COPS,          atCmdHandleCOPSCus, 0          // 7.3 PLMN selection
#else
+COPS,          atCmdHandleCOPS, 0          // 7.3 PLMN selection
#endif
#ifdef CSG_SUPPORT
+COPSCSG,       atCmdHandleCOPSCSG, 0
+CSG,           atCmdHandleCSG, 0
#endif
+CPWD,          atCmdHandleCPWD, 0          // 7.5 Change password
+CPOL,          atCmdHandleCPOL, 0          // 7.19 Preferred PLMN list
+CPLS,          atCmdHandleCPLS, 0          // 7.20 Selection of preferred PLMN list
+COPN,          atCmdHandleCOPN, 0          // 7.21 Read operator names
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
#if defined(CONFIG_AT_SS_COMMAND_SUPPORT) && defined(CONFIG_QUEC_PROJECT_FEATURE_VOLTE)
+CLIP,          atCmdHandleCLIP, 0          // 7.6 Calling line identification presentation
+CLIR,          atCmdHandleCLIR, 0          // 7.7 Calling line identification restriction
+CCWA,          atCmdHandleCCWA, 0          // 7.12 Call waiting
+CLCC,          atCmdHandleCLCC, 0          // 7.18 List current calls
+COLP,          atCmdHandleCOLP, 0          // 7.8 Connected line identification presentation
+CCFC,       	atCmdHandleCCFC, 0          // 7.11 Call forwarding number and conditions
+CSSN,       	atCmdHandleCSSN, 0          // 7.17	Supplementary service notifications +CSSN
+CUSD,          atCmdHandleCUSD, 0          // 7.15	Unstructured supplementary service data +CUSD
#endif
#endif
+SDMBS,        atCmdHandleSDMBS, 0         //Set Pseudo base station identification.
#ifndef CONFIG_QUEC_PROJECT_FEATURE
#if defined (LTE_SUPPORT) && defined (CONFIG_GSM_SUPPORT)
+CTEC,          atCmdHandleCTEC, 0          // Change RAT
#endif
#endif
#ifdef CONFIG_GSM_SUPPORT
^BLACKLISTTMR,  atCmdHandleBLACKLISTTMR, 0
#endif
#endif
#if !defined(CONFIG_ATR_NBONLY_API_SUPPORT)
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+CCED,          atCmdHandleCCED, 0
+MJDC,          atCmdHandleMJDC, 0          //Detect the jamming.
+MJRI,          atCmdHandleMJRI, 0          //jamming rssi info
#endif
^BLACKLIST,     atCmdHandleBLACKLIST, 0
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#if defined(CONFIG_SOC_8910)||defined(CONFIG_SOC_8850)
+UJDC,          atCmdHandleUJDC, AT_CON_NOT_CALIB_MODE //Detect the jamming.
+UJDCP,         atCmdHandleUJDCP,AT_CON_NOT_CALIB_MODE //Detect the jamming parameters.
+UJRI,          atCmdHandleUJRI, AT_CON_NOT_CALIB_MODE //jamming rssi info.
#endif

#if defined(CONFIG_SOC_8910)
+UBWL,          atCmdHandleUBWL, AT_CON_NOT_CALIB_MODE //Black White List
#endif
#endif


#if defined (CONFIG_SOC_8910)
+CDAC,          atCmdHandleCDAC,AT_CON_NOT_CLAC           //Set&Get CDAC value
#endif

#if defined (LTE_NBIOT_SUPPORT) || defined (LTE_SUPPORT)
+CEREG,         atCmdHandleCEREG, 0         // 10.1.22 EPS network registration status
+CSCON,         atCmdHandleCSCON, 0         // 10.1.30 Signalling connection status +CSCON
#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifdef CONFIG_AT_CUS_SUPPORT
+NPSMR,        atCmdHandleNPSMRCus, AT_CON_NOT_CLAC
#else
+NPSMR,        atCmdHandleNPSMR, AT_CON_NOT_CLAC
#endif
#endif

#endif
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
+CSDF,          atCmdHandleCSDF, 0          // 6.22 Settings date format +CSDF
#endif
+CPIN,          atCmdHandleCPIN, 0          // 8.3 Enter PIN
+CLCK,          atCmdHandleCLCK, 0          // 7.4 Facility lock
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
+CDIS,          atCmdHandleCDIS, 0          // 8.8 Display control
+CMER,          atCmdHandleCMER, 0          // 8.10 Mobile termination event reporting
+CEER,          atCmdHandleCEER, 0          // 6.10 Extended error report
#endif
#if !defined(CONFIG_SOC_8811)
+CNUM,          atCmdHandleCNUM, 0          // 7.1 Subscriber number
#endif
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
#ifdef CONFIG_QUEC_PROJECT_FEATURE_PBK_AT
+CPBS,          atCmdHandleCPBS, 0          // 8.11 Select phonebook memory storage +CPBS
+CPBR,          atCmdHandleCPBR, 0          // 8.12 Read phonebook entries +CPBR
+CPBF,          atCmdHandleCPBF, 0          // 8.13 Find phonebook entries +CPBF
+CPBW,          atCmdHandleCPBW, 0          // 8.14 Write phonebook entry +CPBW
#endif
#endif
+CCLK,          atCmdHandleCCLK, 0          // 8.15 Clock +CCLK
+CRSM,          atCmdHandleCRSM, 0          // 8.18 Restricted SIM access
#if !defined(CONFIG_ATR_NBONLY_API_SUPPORT) && !defined(CONFIG_SOC_8850)
+CACM,          atCmdHandleCACM, 0          // 8.25 Accumulated call meter
#endif
+CTZU,          atCmdHandleCTZU, 0          // 8.40 Automatic time zone update +CTZU
+CTZR,          atCmdHandleCTZR, 0          // 8.41 Time zone reporting +CTZR
#ifndef CONFIG_ATR_NBONLY_API_SUPPORT
+CPINR,         atCmdHandleCPINR, 0         // 8.65 Remaining PIN retries +CPINR
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+TPIN,          atCmdHandleCPINR, 0         // same as +CPINR
#endif
#endif
+CCID,          atCmdHandleCCID, 0          // Get Iccid
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+SIM,           atCmdHandleSIM, 0
^SIMIF,         atCmdHandleSIMIF, 0
+SIMCROSS,      atCmdHandleSIMCROSS, 0      //Switch sim id
#endif
+QSPN,          atCmdHandleQSPN, 0
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+QGID,          atCmdHandleQGID, 0
+PSIM,          atCmdHandlePSIM, 0
#ifndef CONFIG_ATR_NBONLY_API_SUPPORT
+POC,           atCmdHandlePOC, AT_CON_NOT_CLAC
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+PAS,           atCmdHandlePAS, 0
+PUSIM,         atCmdHandlePUSIM, 0
#endif
#endif
+STKTR,         atCmdHandleSTKTR, AT_CON_NOT_CLAC
+STKENV,        atCmdHandleSTKENV, AT_CON_NOT_CLAC
+STKEN,         atCmdHandleSTKEN, AT_CON_NOT_CLAC
+STKM,         atCmdHandleSTKM, AT_CON_NOT_CLAC
+STKPD, 	   atCmdHandleSTKPD, AT_CON_NOT_CLAC
#endif
#ifdef CONFIG_ATS_ALARM_SUPPORT
+CALA,          atCmdHandleCALA, 0          // 8.16 Alarm +CALA
+CALD,          atCmdHandleCALD, 0          // 8.38 Delete alarm +CALD
#endif
#if defined RPM_SUPPORT && defined(CONFIG_SOC_8910)
+CFGRPMSWITCH,     atCmdHandleCFGRPMSWITCH, 0
+CFGRPMPARA,         atCmdHandleCFGRPMPARA, 0
+CFGRPMCLR,           atCmdHandleCFGRPMCLR, 0
+CFGRPMCOUNTER,   atCmdHandleCFGRPMCOUNTER, 0
#endif
#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifdef CONFIG_AT_UNIKEY_SUPPORT
+UNIKEYINFO,        AT_CmdFunc_UNIKEYINFO, 0
+UNIDELKEYINFO,        AT_CmdFunc_UNIDELKEYINFO, 0
+UNIKEYINFOM,        AT_CmdFunc_UNIKEYINFOM, 0
+UNIDELKEYINFOM,        AT_CmdFunc_UNIDELKEYINFOM, 0
+UNICERTINFO,        AT_CmdFunc_UNICERTINFO, 0
+UNISHCERTINFO,        AT_CmdFunc_UNISHCERTINFO, 0
+UNIDELCERTINFO,        AT_CmdFunc_UNIDELCERTINFO, 0
#endif
#ifdef CONFIG_AT_TCPIP_SUPPORT
+CIPMUX,        AT_TCPIP_CmdFunc_CIPMUX, 0
+CIPSGTXT,      AT_TCPIP_CmdFunc_CIPSGTXT, 0
+CIPSTART,      AT_TCPIP_CmdFunc_CIPSTART, 0
+CIFSR,         AT_TCPIP_CmdFunc_CIFSR, 0
+CIPSTATUS,     AT_TCPIP_CmdFunc_CIPSTATUS, 0
+CIPSCONT,      AT_TCPIP_CmdFunc_CIPSCONT, 0
+CIPCLOSE,      AT_TCPIP_CmdFunc_CIPCLOSE, 0
+CIPSHUT,       AT_TCPIP_CmdFunc_CIPSHUT, 0
+CIICR,         AT_TCPIP_CmdFunc_CIICR, 0
+CIPSEND,       AT_TCPIP_CmdFunc_CIPSEND, 0
+CIPQSEND,      AT_TCPIP_CmdFunc_CIPQSEND, 0
+CIPRXGET,      AT_TCPIP_CmdFunc_CIPRXGET, 0
+CSTT,          AT_TCPIP_CmdFunc_CSTT, 0
+CIPATS,        AT_TCPIP_CmdFunc_CIPATS, 0
+CDNSGIP,       AT_TCPIP_CmdFunc_CDNSGIP, 0
+CDNSCFG,       AT_TCPIP_CmdFunc_CDNSCFG, 0
+CIPSPRT,       AT_TCPIP_CmdFunc_CIPSPRT, 0
+CIPHEAD,       AT_TCPIP_CmdFunc_CIPHEAD, 0
+CIPCSGP,       AT_TCPIP_CmdFunc_CIPCSGP, 0
+CIPSRIP,       AT_TCPIP_CmdFunc_CIPSRIP, 0
+CIPDPDP,       AT_TCPIP_CmdFunc_CIPDPDP, 0
+CIPMODE,       AT_TCPIP_CmdFunc_CIPMODE, 0
+CIPCCFG,       AT_TCPIP_CmdFunc_CIPCCFG, 0
+CIPSHOWTP,     AT_TCPIP_CmdFunc_CIPSHOWTP, 0
+CIPUDPMODE,    AT_TCPIP_CmdFunc_CIPUDPMODE, 0
+CIPACK,        AT_TCPIP_CmdFunc_CIPACK, 0
+CLPORT,        AT_TCPIP_CmdFunc_CLPORT, 0
+CIPSERVER,     AT_TCPIP_CmdFunc_CIPSERVER, 0
+CIPTKA,        AT_TCPIP_CmdFunc_CIPTKA, 0
+CIPRDTIMER,    AT_TCPIP_CmdFunc_CIPRDTIMER, 0
+CIPRAIMODE,    AT_TCPIP_CmdFunc_CIPRAIMODE, AT_CON_NOT_CLAC
+SETROUTE,    AT_TCPIP_CmdFunc_SETROUTE, AT_CON_NOT_CLAC
+UNSETROUTE,    AT_TCPIP_CmdFunc_UNSETROUTE, AT_CON_NOT_CLAC
#endif
#endif

#ifdef CONFIG_AT_IPERF_SUPPORT
+IPERF, AT_CmdFunc_IPERF, AT_CON_NOT_CLAC
+IPERFSTOP, AT_CmdFunc_IPERFSTOP, AT_CON_NOT_CLAC
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
+PING,          AT_TCPIP_CmdFunc_PING, 0
+PINGSTOP,      AT_TCPIP_CmdFunc_PINGSTOP, 0

#ifdef CONFIG_AT_UDP_SUPPORT
+TSOCR,         AT_TCPIP_CmdFunc_TSOCR, 0
+TSOST,         AT_TCPIP_CmdFunc_TSOST, 0
+TSOSTF,       AT_TCPIP_CmdFunc_TSOSTF, 0
+TSORF,         AT_TCPIP_CmdFunc_TSORF, 0
+TSOCL,         AT_TCPIP_CmdFunc_TSOCL, 0
+TPING,         AT_TCPIP_CmdFunc_TPING, 0
#endif
#endif

^NETIF,         AT_NET_CmdFunc_NetInfo, 0

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifdef CONFIG_AT_MBEDTLS_TCPIP_SUPPORT
+SSLSTART,        AT_TCPIP_CmdFunc_SSLSTART, 0
+SSLSEND,         AT_TCPIP_CmdFunc_SSLSEND, 0
+SSLCLOSE,        AT_TCPIP_CmdFunc_SSLCLOSE, 0
#endif

#ifdef CONFIG_AT_USSL_SUPPORT
+TLSSETCRT,     AT_TLS_CmdFunc_SSLSETCRT, 0
+USSLCFG,              AT_TLS_CmdFunc_CFG, 0
+USSLOPEN,             AT_TLS_CmdFunc_SSLOPEN, 0
+USSLSEND,             AT_TLS_CmdFunc_SSLSEND, 0
+USSLRECV,             AT_TLS_CmdFunc_SSLREAD, 0
+USSLCLOSE,            AT_TLS_CmdFunc_SSLCLOSE, 0
#endif
#endif
//
// Mobile Termination control and status commands
//
//#ifdef CONFIG_SUPPORT_BATTERY_CHARGER  //quectel update
+CBC,           atCmdHandleCBC, 0           // 8.4 Battery charge
//#endif
+CIND,          atCmdHandleCIND, 0          // 8.9 Indicator control
+CESQ,          atCmdHandleCESQ, 0          // 8.69 Extended signal quality
+CALIBINFO,     atCmdHandleCALIBINFO, 0     // Check the calibration mark bit
+CMEE,          atCmdHandleCMEE, 0          // 9.1 Report mobile termination error

#ifndef CONFIG_QUEC_PROJECT_FEATURE
+CPOF,          atCmdHandleCPOF, 0
+TRB,           atCmdHandleTRB, 0
#endif
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+CSCLK,         atCmdHandleCSCLK, 0
#endif
+CGATT,         atCmdHandleCGATT, 0         // 10.1.9 PS attach or detach
+SPREBOOTCMD,	atCmdHandleSPREBOOT, 0
#ifndef CONFIG_ATR_NBONLY_API_SUPPORT
+CGQMIN,        atCmdHandleCGQMIN, 0        // 10.1.5 Quality of service profile (minimum acceptable)
+CGREG,         atCmdHandleCGREG, 0         // 10.1.20 GPRS network registration status
+CGDEL,         atCmdHandleCGDEL, 0         // 10.1.29 Delete non-active PDP contexts
#endif

#ifndef CONFIG_QL_OPEN_EXPORT_PKG
^PDNACTINFO,    atCmdHandlePDNACTINFO, 0    // RW PDN act or deact retry timer and retry maxcount
#endif
+CGAUTH,        atCmdHandleCGAUTH, 0        // 10.1.31 Define PDP context authentication parameters
#ifdef LTE_NBIOT_SUPPORT
#ifdef DEDICATED_BEARER_SUPPORT
+CGDSCONT,      atCmdHandleCGDSCONT, 0      // 10.1.2 Define secondary PDP context
#endif
#endif
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
#if defined(LTE_NBIOT_SUPPORT) || defined(LTE_SUPPORT)
+CGTFT,         atCmdHandleCGTFT, 0         // 10.1.3 Traffic flow
#ifndef CONFIG_ATR_NBONLY_API_SUPPORT
+CGTFTRDP,      atCmdHandleCGTFTRDP, 0      // 10.1.25 Traffic flow template read dynamic parameters
+CGEQREQ,       atCmdHandleCGEQREQ, 0       // 10.1.6 3G quality of service profile (requested)
+CGEQMIN,       atCmdHandleCGEQMIN, 0       // 10.1.7 3G quality of service profile (minimum acceptable)
#endif
#endif
#endif
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
#if defined(LTE_NBIOT_SUPPORT) || defined(LTE_SUPPORT)
+CGCMOD,        atCmdHandleCGCMOD, 0        // 10.1.11 PDP context modify
#endif
+CGEREP,        atCmdHandleCGEREP, 0        // 10.1.19 Packet domain event reporting
+CGCONTRDP,      atCmdHandleCGCONTRDP, 0    // 10.1.23 PDP context read dynamic parameters +CGCONTRDP
#ifndef CONFIG_ATR_NBONLY_API_SUPPORT
#ifndef CONFIG_SOC_8850
#ifdef CONFIG_GSM_SUPPORT
+CGSMS,         atCmdHandleCGSMS, 0         // 10.1.21 Select service for MO SMS messages
#endif
#endif
+CGSCONTRDP,    atCmdHandleCGSCONTRDP, 0    // 10.1.24 Secondary PDP context read dynamic parameters
#endif

#if defined(LTE_NBIOT_SUPPORT) || defined(LTE_SUPPORT)
+CGEQOS,        atCmdHandleCGEQOS, 0        // 10.1.26 Define EPS quality of service
#endif
#endif
#ifndef CONFIG_ATR_NBONLY_API_SUPPORT
+CEMODE,        atCmdHandleCEMODE, 0        // 10.1.28 UE modes of operation for EPS
#if defined(LTE_NBIOT_SUPPORT) || defined(LTE_SUPPORT)
+CGEQOSRDP,     atCmdHandleCGEQOSRDP, 0     // 10.1.27 EPS quality of service read dynamic parameters
#endif
#endif
#if defined(LTE_NBIOT_SUPPORT) || defined(LTE_SUPPORT)
+CSODCP,        atCmdHandleCSODCP, 0        // 10.1.43 Sending of originating data via the control plane +CSODCP
#endif
+CGDATA,        atCmdHandleCGDATA, 0        // 10.1.12 Enter data state
+CGAUTO,        atCmdHandleCGAUTO, 0        // 10.1.15 Automatic response to a network request for PDP context activation
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
#ifndef CONFIG_ATR_NBONLY_API_SUPPORT
+CGQREQ,        atCmdHandleCGQREQ, 0        // 10.1.4 Quality of service profile (requested)
+CGANS,         atCmdHandleCGANS, 0         // 10.1.16 Manual response to a network request for PDP context activation
#if defined(CONFIG_GSM_SUPPORT)
+CGCLASS,       atCmdHandleCGCLASS, 0       // 10.1.17 GPRS mobile station class
#endif
#endif
#endif
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+CGPDNSADDR,    atCmdHandleCGPDNSADDR, 0
#endif
#if defined(LTE_NBIOT_SUPPORT)
+CRTDCP,        atCmdHandleCRTDCP, 0        // 10.1.44 Reporting of terminating data via the control plane +CRTDCP
#ifndef CONFIG_QUEC_PROJECT_FEATURE
#if defined (CONFIG_SOC_8811) || defined (CONFIG_SOC_6760)
+CFGDFTPDN,     atCmdHandleCFGDFTPDN, 0
+NVCFGARFCN,    atCmdHandleCfgArfcn, 0
+NVSETLOCKFREQ, atCmdHandleLockFreq, 0
+CALIBFLAG,     atCmdHandleCALIBFLAG, 0     // Check the calibration mark bit for 8811
+CALIBMODE,     atCmdHandleCALIBMODE, 0
+CALIBRXOFF,    atCmdHandleCALIBRXOFF, 0
+CALIBRXON,     atCmdHandleCALIBRXON, 0
+CALIBTXOFF,    atCmdHandleCALIBTXOFF, 0
+CALIBTXON,     atCmdHandleCALIBTXON, 0
+CALIBRSSI,     atCmdHandleCALIBRSSI, 0
#endif
#ifdef CONFIG_SOC_6760
+SETCARRIERNUM, atCmdHandleCarrierNum, 0
+LOGRANK,       atCmdHandleLOGRANK, 0
#endif
#endif
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifdef CONFIG_SUPPORT_INNER_DELTA_NV
+DELTANV,       atCmdHandleMergeDeltaNV, 0     // read or write deltanv param
#endif
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#if defined(CONFIG_AT_QSCANF_SUPPORT)
+QSCANF,        atCmdHandleQSCANF, 0        // use to freq scan
#ifdef CONFIG_SOC_8910
+QSCANFA,	   atCmdHandleQSCANFA, 0		// use to freq scan all(LTE and GSM)
#endif
#endif

#ifndef CONFIG_SOC_8850
#if defined(LTE_NBIOT_SUPPORT)
+NIPDATA, atCmdHandleNIPDATA, AT_CON_NOT_CLAC   // 17.7 send None-IP data
#endif
#endif

#ifdef LTE_SUPPORT
+SETLOCK,       atCmdHandleSETLOCK, 0
+SETSTSEN,      atCmdHandleSETSTSEN,   0
+SETRATEPRIOR,  atCmdHandleSETRATEPRIOR, 0
#endif

#if defined(CONFIG_SOC_8811) || defined(CONFIG_CFW_DEBUG_IPFILTER)
+IPFLT,         atCmdHandleIPFLT, 0         // Filter IP packets inside CFW. DEBUG ONLY!!
#endif
#endif

#ifdef CONFIG_GSM_SUPPORT
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+QGPCLASS,      atCmdHandleQGPCLASS, 0

#ifndef CONFIG_SOC_8850
+DLST,          atCmdHandleDLST, 0
#endif
+ECSQ,          atCmdHandleECSQ, 0          // Enhanced signal quality report
#endif
#endif


#ifndef CONFIG_QUEC_PROJECT_FEATURE
// filesystem
#ifdef CONFIG_ATS_FS_SUPPORT
+FSDWNFILE,     atCmdHandleFSDWNFILE, 0     // Download file
+FSRDFILE,      atCmdHandleFSRDFILE, 0      // Read file
+FSRDBLOCK,     atCmdHandleFSRDBLOCK, 0     // Read file block
+FSDELFILE,     atCmdHandleFSDELFILE, 0     // Delete file
+FSMKDIR,       atCmdHandleFSMKDIR, 0       // Create directory
+FSRMDIR,       atCmdHandleFSRMDIR, 0       // Remove directory
+FSLSTFILE,     atCmdHandleFSLSTFILE, 0     // List file or directory
+FSLSTPART,     atCmdHandleFSLSTPART, 0     // List partition free space
+FSREMOUNT,     atCmdHandleFSREMOUNT, 0     // Remount as read-only or read-write
#endif

#ifdef CONFIG_AT_HTTP_SUPPORT
+HTTPINIT,      AT_CmdFunc_HTTPINIT, 0
+HTTPTERM,      AT_CmdFunc_HTTPTERM, 0
+HTTPPARA,      AT_CmdFunc_HTTPPARA, 0
+HTTPREAD,      AT_CmdFunc_HTTPREAD, 0
+HTTPDATA,      AT_CmdFunc_HTTPDATA, 0
+HTTPACTION,    AT_CmdFunc_HTTPACTION, 0
+HTTPSTATUS,    AT_CmdFunc_HTTPSTATUS, 0
+HTTPSSETCRT,   AT_CmdFunc_HTTPSSETCRT, 0
#endif

+SNTP,          AT_CmdFunc_SNTP, 0

#if defined(CONFIG_AT_FTP_SUPPORT) || defined(CONFIG_AT_NEWFTP_SUPPORT)
#ifdef CONFIG_AT_NEWFTP_SSL_SUPPORT
^FTPSETCRT,     AT_FTP_CmdFunc_SETCRT, 0
^FTPSETMODE,    AT_FTP_CmdFunc_SETMODE, 0
#endif
^FTPOPEN,       AT_FTP_CmdFunc_OPEN, 0
^FTPCLOSE,      AT_FTP_CmdFunc_CLOSE, 0
^FTPSIZE,       AT_FTP_CmdFunc_SIZE, 0
^FTPGET,        AT_FTP_CmdFunc_GET, 0
^FTPGETSET,     AT_FTP_CmdFunc_GETSET, 0
^FTPPUT,        AT_FTP_CmdFunc_PUT, 0
^FTPPUTSET,     AT_FTP_CmdFunc_PUTSET, 0
#endif

#ifdef CONFIG_AT_IDS_SUPPORT
+IDSSET, AT_IDS_CmdFunc_SET, AT_CON_NOT_CLAC
+IDSGET, AT_IDS_CmdFunc_GET, AT_CON_NOT_CLAC
+IDSONOFF, AT_IDS_CmdFunc_OnOff, AT_CON_NOT_CLAC
#endif

#ifdef CONFIG_AT_LIBCOAP_SUPPORT
^COAPGET,       AT_COAP_CmdFunc_GET, 0
^COAPPUT,       AT_COAP_CmdFunc_PUT, 0
^COAPPOST,      AT_COAP_CmdFunc_POST, 0
^COAPDELETE,    AT_COAP_CmdFunc_DELETE, 0
^COAPDATA,      AT_COAP_CmdFunc_DATA, 0
#endif

#ifdef CONFIG_AT_LWIP_MQTT_SUPPORT
+MQTTCONN,      AT_GPRS_CmdFunc_MQTTCONN, 0
+MQTTDISCONN,   AT_GPRS_CmdFunc_MQTTDISCONN, 0
+MQTTSUBUNSUB,  AT_GPRS_CmdFunc_MQTT_SUB_UNSUB, 0
+MQTTPUB,       AT_GPRS_CmdFunc_MQTTPUB, 0
#endif

#ifdef CONFIG_AT_PAHO_MQTT_SUPPORT
+MQTTCONN,      AT_GPRS_CmdFunc_MQTTCONN, 0
+MQTTDISCONN,   AT_GPRS_CmdFunc_MQTTDISCONN, 0
+MQTTSUBUNSUB,  AT_GPRS_CmdFunc_MQTT_SUB_UNSUB, 0
+MQTTPUB,       AT_GPRS_CmdFunc_MQTTPUB, 0
#endif

#ifdef CONFIG_AT_UNI_MQTT_SUPPORT
+UNIMQTTCON,         AT_CmdFunc_UNIMQTTCON, 0
+UNIMQTTDISCON,      AT_CmdFunc_UNIMQTTDISCON, 0
+UNIMQTTSTATE,       AT_CmdFunc_UNIMQTTSTATE, 0
+UNIMQTTSUB,         AT_CmdFunc_UNIMQTTSUB, 0
+UNIMQTTPUB,         AT_CmdFunc_UNIMQTTPUB, 0
+UNIPSMSET,      AT_CmdFunc_UNIPSMSET, 0
#endif

#ifdef CONFIG_AT_ALIC_SUPPORT
+ALICAUTH,            AT_ALIC_CmdFunc_AUTH, 0
+ALICCONN,            AT_ALIC_CmdFunc_CONN, 0
+ALICPUB,              AT_ALIC_CmdFunc_PUB, 0
+ALICSUB,              AT_ALIC_CmdFunc_SUB, 0
+ALICUNSUB,          AT_ALIC_CmdFunc_UNSUB, 0
+ALICDISCONN,       AT_ALIC_CmdFunc_DISCONN, 0
#endif

#ifdef CONFIG_SOC_6760
+SIGMOD,        atCmdHandleSIGMOD, 0        // signal mode
+DLPARAM,       atCmdHandleDLPARAM, 0       // downlink param
+DLRCV,         atCmdHandleDLRCV, 0         // downlink recieve enable
+ULPARAM,       atCmdHandleULPARAM, 0       // uplink param
+ULRCV,         atCmdHandleULRCV, 0         // uplink recieve enable
+APC,           atCmdHandleAPC, 0           // APC
+AGC,           atCmdHandleAGC, 0           // AGC
+SETLIMIT,      atCmdHandleSETLIMIT, 0      // set temperature upper limit
+NWSTATUS,      atCmdHandleNWSTATUS, 0      // set network status ind
#endif

// debug
#ifndef CONFIG_SOC_8811
^SWJTAG,        atCmdHandleSWJTAG, 0        // Switch JTAG pin mux
^PMSTART,       atCmdHandlePMSTART, 0       // Start PM core
^PMSTOP,        atCmdHandlePMSTOP, 0        // Stop PM core
#endif
^FORCEDNLD,     atCmdHandleFORCEDNLD, 0     // Reset to force download mode
+SPREF,         atCmdHandleSPREF, AT_CON_NOT_CLAC         // Reset to force download mode by Download tools
#endif

#ifndef CONFIG_SOC_6760
#ifndef CONFIG_QUEC_PROJECT_FEATURE
^TRACECTRL,     atCmdHandleTrace, 0         // Enable/disable trace
^CALIBTRACECTRL,     atCmdHandleCalibTrace, 0         // Enable/disable trace when calib mode
#endif
+SYSNV,         atCmdHandleSYSNV, 0         // read/write sysnv
#endif

#ifndef CONFIG_SOC_8811
^SSIT,          atCmdHandleSSIT, 0
#endif
#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifndef CONFIG_SOC_6760
^HEAPINFO,      atCmdHandleHEAPINFO, 0      // Show heap info
^BLKDEVINFO,    atCmdHandleBLKDEVINFO, AT_CON_NOT_CLAC    // Show block device info
#endif
^TIMEOUTABORT,  atCmdHandleTIMEOUTABORT, 0  // Trivial command to test timeout and abort
^UPTIME,        atCmdHandleUpTime, 0        // Get up time
+UPTIME,        atCmdHandleUpTimePlus, 0    // Get up time
+SPENGMD,       atCmdHandleSPENGMD, 0       // Only support assert system
#if defined(CONFIG_SOC_8910)||defined(CONFIG_SOC_8850)
+SET7SRESET,    atCmdHandleSet7sReset, 0    // set powerkey 7s reset(1:true 0:false)
#ifdef CONFIG_USB_DEVICE_SUPPORT
+USBSWITCH,     atCmdHandleUSBSWITCH, 0     // enable/disable usb function
+USBRMTWK,      atCmdHandleUSBRMTWK, 0      // usb remote wakeup
#endif
#endif
#ifdef CONFIG_TWOLINE_WAKEUP_ENABLE
+SLEEPDELAY,    atCmdHandleMcuNotifySleep, 0,    //mcu notify module should into deepsleep
#endif

#ifdef CONFIG_SRV_SIMLOCK_ENABLE
^SPHUK,          atCmdHandleSimlockSPHUK, AT_CON_NOT_CLAC    //save the random number "HUK"
^SPFACAUTHPUBKEY, atCmdHandleSimlockSPFACAUTHPUBKEY, AT_CON_NOT_CLAC    //check and save the RSA public key
^SPIDENTIFYSTART, atCmdHandleSimlockSPIDENTIFYSTART, AT_CON_NOT_CLAC    //start authentication
^SPIDENTIFYEND, atCmdHandleSimlockSPIDENTIFYEND, AT_CON_NOT_CLAC    //end authentication
^SPDATAENCRYPT, atCmdHandleSimlockSPDATAENCRYPT, AT_CON_NOT_CLAC    //encrypt the original simlock keys
^SPSIMLOCKDATAWRITE, atCmdHandleSPSIMLOCKDATAWRITE, AT_CON_NOT_CLAC    //update the simlock data
^SPSIMLOCKIMEI, atCmdHandleSimlockSPSIMLOCKIMEI, AT_CON_NOT_CLAC    // no handle this cmd just return ok.
#ifdef CONFIG_SOC_8850
+SPGETUID, atCmdHandleSimlockSPGETUID, AT_CON_NOT_CLAC
+SPUNLOCKCHECK, atCmdHandleSimlockSPUNLOCKCHECK, AT_CON_NOT_CLAC
^SPSIMLOCKCHECK, atCmdHandleSimlockSPSIMLOCKCHECK, AT_CON_NOT_CLAC
+SPIMEICHECK, atCmdHandleSimlockSPIMEICHECK, AT_CON_NOT_CLAC
+SPSIMLOCKSETVER, atCmdHandleSimlockSPSIMLOCKSETVER, AT_CON_NOT_CLAC
#endif
#endif

#ifdef CONFIG_AT_EMMCDDRSIZE_SUPPORT
+EMMCDDRSIZE,     atCmdHandleEmmcDDRSize, 0    // read flash size and exram size
#endif
#ifdef CONFIG_ATR_CMUX_SUPPORT
+CMUXEND,       atCmdHandleCMUXEND, AT_CON_NOT_CLAC     // exit cmux mode forcedly
#endif
#endif

#ifdef CONFIG_AT_AUDIO_SUPPORT
#ifndef CONFIG_QUEC_PROJECT_FEATURE    //adjust by kevin 20201128, as quec at command have at+crsl, at+clvl, but param is not same to it 
+CRSL,          atCmdHandleCRSL, 0          // 8.21 Ringer sound level
+CLVL,          atCmdHandleCLVL, 0          // 8.23 Loudspeaker volume level
+CMUT,          atCmdHandleCMUT, 0          // 8.24 Mute control
+AUDCH,         atCmdHandleAUDCH, 0         // Set receiver, headset and loudspeaker channel
+CACCP,         atCmdHandleCACCP, 0         // Audio Codec Calibration Param
+CAVQE,         atCmdHandleCAVQE, 0         // Audio ZSP VQE Calibration Param
+CAPRE,         atCmdHandleCAPRE, 0         // Audio ZSP Pre -Processing Calibration Param
+CAPOST,        atCmdHandleCAPOST, 0        // Audio ZSP Post -Processing Calibration Param
+CAWTF,         atCmdHandleCAWTF, 0         // Write calibration param to flash
+CAIET,         atCmdHandleCAIET, 0         // Export calibration param from flash or import calibration param to flash
+CADTF,         atCmdHandleCADTF, 0         // dump PCM data to Tflash card
#ifdef CONFIG_SOC_8850
+CADTL,         atCmdHandleCADTL, 0         // dump PCM data to log
#endif
+CAVCT,         atCmdHandleCAVCT, 0         // version
+CANXP,         atCmdHandleCANXP, 0         //
+CAUDPLAY,      atCmdHandleCAUDPLAY, 0      // Play/stop/pause/resume audio file
#ifdef CONFIG_AT_AUDIO_COMMAND_SUPPORT
+CAUDREC,       atCmdHandleCAUDREC, 0       // Voice call recording
+CDTMF,         atCmdHandleCDTMF, 0         // Local DTMF tone
#endif
+CAUDPOC,       atCmdHandleCAUDPOC, AT_CON_NOT_CLAC       // Poc test
+LBTEST,        atCmdHandleLBTEST, 0        // Loopback test
#ifdef CONFIG_SOC_8910
+MAI2SY,        atCmdHandleMAI2SY, 0        // Config external codec mode
+PCMMODE,       atCmdHandlePCMMODE, AT_CON_NOT_CLAC       // Set PCM frame synchronization singal mode
+CALM,          atCmdHandleCALM, AT_CON_NOT_CLAC          // Control playtone of call and sms
#endif
#endif
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifdef CONFIG_TTS_SUPPORT
+CTTS,          atCmdHandleCTTS, AT_CON_NOT_CLAC          // TTS
#endif

#ifdef CONFIG_SOC_8910
+TSTSETCS,      atCmdHandleTSTSETCS, AT_CON_NOT_CLAC      // Just use to test close CS domain
+SIMLOCKTEST,      atCmdHandleSimlockTest, AT_CON_NOT_CLAC      // Just use to test close CS domain
#endif
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
//command for lwm2m
#if defined(CONFIG_AT_CISSDK_MIPL_SUPPORT)
+MIPLCREATE,        AT_CmdFunc_MIPLCREATE, 0
+MIPLDELETE,        AT_CmdFunc_MIPLDELETE, 0
+MIPLOPEN,          AT_CmdFunc_MIPLOPEN, 0
+MIPLCLOSE,         AT_CmdFunc_MIPLCLOSE, 0
+MIPLADDOBJ,        AT_CmdFunc_MIPLADDOBJ, 0
+MIPLDELOBJ,        AT_CmdFunc_MIPLDELOBJ, 0
+MIPLNOTIFY,        AT_CmdFunc_MIPLNOTIFY, 0
+MIPLREADRSP,       AT_CmdFunc_MIPLREADRSP, 0
+MIPLWRITERSP,      AT_CmdFunc_MIPLWRITERSP, 0
+MIPLEXECUTERSP,    AT_CmdFunc_MIPLEXECUTERSP, 0
+MIPLOBSERVERSP,    AT_CmdFunc_MIPLOBSERVERSP, 0
+MIPLDISCOVERRSP,   AT_CmdFunc_MIPLDISCOVERRSP, 0
+MIPLPARAMETERRSP,  AT_CmdFunc_MIPLPARAMETERRSP, 0
+MIPLUPDATE,        AT_CmdFunc_MIPLUPDATE, 0
+MIPLVER,           AT_CmdFunc_MIPLVER, 0
+MIPLCLEARSTATE,    AT_CmdFunc_MIPLCLEARSTATE, 0
#elif defined(CONFIG_AT_LWM2M_MIPL_SUPPORT)
+MIPLCREATE,        AT_CmdFunc_Lwm2m_MIPLCREATE, 0
+MIPLDELETE,        AT_CmdFunc_Lwm2m_MIPLDELETE, 0
+MIPLOPEN,          AT_CmdFunc_Lwm2m_MIPLOPEN, 0
+MIPLCLOSE,         AT_CmdFunc_Lwm2m_MIPLCLOSE, 0
+MIPLADDOBJ,        AT_CmdFunc_Lwm2m_MIPLADDOBJ, 0
+MIPLDELOBJ,		AT_CmdFunc_Lwm2m_MIPLDELOBJ, 0
+MIPLNOTIFY,		AT_CmdFunc_Lwm2m_MIPLNOTIFY, 0
+MIPLREADRSP,		AT_CmdFunc_Lwm2m_MIPLREADRSP, 0
+MIPLWRITERSP,		AT_CmdFunc_Lwm2m_MIPLWRITERSP, 0
+MIPLEXECUTERSP,	AT_CmdFunc_Lwm2m_MIPLEXECUTERSP, 0
+MIPLOBSERVERSP,	AT_CmdFunc_Lwm2m_MIPLOBSERVERSP, 0
+MIPLDISCOVERRSP,	AT_CmdFunc_Lwm2m_MIPLDISCOVERRSP, 0
+MIPLPARAMETERRSP,	AT_CmdFunc_Lwm2m_MIPLPARAMETERRSP, 0
+MIPLUPDATE,		AT_CmdFunc_Lwm2m_MIPLUPDATE, 0
+MIPLVER,			AT_CmdFunc_Lwm2m_MIPLVER, 0
+MIPLCLEARSTATE,	AT_CmdFunc_Lwm2m_MIPLCLEARSTATE, 0
#ifdef CONFIG_AT_LWM2M_MIPL_SOTA_SUPPORT
+MIPLSOTAINFO,   AT_CmdFunc_Lwm2m_MIPLSOTAINFO, 0
+MIPLSOTARESULT,   AT_CmdFunc_Lwm2m_MIPLSOTARESULT, 0
#endif
#elif defined(CONFIG_AT_CUS_MIPL_SUPPORT)
+MIPLCONFIG,		AT_CmdFunc_CUS_MIPLCONFIG, 0
+MIPLCREATE,		AT_CmdFunc_CUS_MIPLCREATE, 0
+MIPLDELETE,		AT_CmdFunc_CUS_MIPLDELETE, 0
+MIPLOPEN,			AT_CmdFunc_CUS_MIPLOPEN, 0
+MIPLCLOSE,			AT_CmdFunc_CUS_MIPLCLOSE, 0
+MIPLADDOBJ,		AT_CmdFunc_CUS_MIPLADDOBJ, 0
+MIPLDELOBJ,		AT_CmdFunc_CUS_MIPLDELOBJ, 0
+MIPLNOTIFY,		AT_CmdFunc_CUS_MIPLNOTIFY, 0
+MIPLREADRSP,		AT_CmdFunc_CUS_MIPLREADRSP, 0
+MIPLWRITERSP,		AT_CmdFunc_CUS_MIPLWRITERSP, 0
+MIPLEXECUTERSP,	AT_CmdFunc_CUS_MIPLEXECUTERSP, 0
+MIPLOBSERVERSP,	AT_CmdFunc_CUS_MIPLOBSERVERSP, 0
+MIPLDISCOVERRSP,	AT_CmdFunc_CUS_MIPLDISCOVERRSP, 0
+MIPLPARAMETERRSP,	AT_CmdFunc_CUS_MIPLPARAMETERRSP, 0
+MIPLUPDATE,		AT_CmdFunc_CUS_MIPLUPDATE, 0
+MIPLVER,			AT_CmdFunc_CUS_MIPLVER, 0
+MIPLRD,			AT_CmdFunc_CUS_MIPLRD, 0
+MIPLCLEARSTATE,	AT_CmdFunc_CUS_MIPLCLEARSTATE, 0
#endif

#ifdef CONFIG_AT_OCEANCONNECT_SUPPORT
#ifndef  CONFIG_AT_CUS_SUPPORT
+NCDPOPEN,    AT_CmdFunc_NCDPOPEN, 0
+NCDPCLOSE,   AT_CmdFunc_NCDPCLOSE, 0
+NMGS,        AT_CmdFunc_NMGS, 0
+NMGR,        AT_CmdFunc_NMGR, 0
+NNMI,        AT_CmdFunc_NNMI, 0
#endif
#endif
#ifdef CONFIG_AT_CTWING_SUPPORT
+CTM2MINIT,    AT_CmdFunc_CTM2MINIT, 0
+CTM2MREG,     AT_CmdFunc_CTM2MREG, 0
+CTM2MDEREG,   AT_CmdFunc_CTM2MDEREG, 0
+CTM2MSEND,    AT_CmdFunc_CTM2MSEND, 0
+CTM2MUPDATE,  AT_CmdFunc_CTM2MUPDATE, 0
+CTM2MVER,  AT_CmdFunc_CTM2MVER, 0
#ifdef CONFIG_AT_CTWING_SELFREG_SUPPORT
+CTM2MREGSWT,  AT_CmdFunc_CTM2MREGSWT, 0
#endif
#ifdef CONFIG_AT_CTWING_NEW_STANDARD
+CTM2MLIFETIME,  AT_CmdFunc_CTM2MLIFETIME, 0
#endif
#ifdef CONFIG_AT_CTIOTSM_SUPPORT
+CTLWSETPCRYPT,        AT_CmdFunc_CTLWSETPCRYPT, 0
+CTLWSETAUTH,        AT_CmdFunc_CTLWSETAUTH, 0
+CTSIBCREQKEY,        AT_CmdFunc_CTSIBCREQKEY, 0
#endif
#endif

#ifdef CONFIG_AT_CTIOTSM_SUPPORT
+CTSIBCIMPORTKEY,        AT_CmdFunc_CTSIBCIMPORTKEY, 0
+CTSSELECTKEY,        AT_CmdFunc_CTSSELECTKEY, 0
+CTSIBCVERIFY,      AT_CmdFunc_CTSIBCVERIFY, 0
+CTSIBCSIGN,      AT_CmdFunc_CTSIBCSIGN, 0
+CTSIBCENC,        AT_CmdFunc_CTSIBCENC, 0
+CTSIBCDEC,        AT_CmdFunc_CTSIBCDEC, 0

#endif

#ifdef CONFIG_ATS_UPDATE_SUPPORT
+UPDATE,            atCmdHandleUPDATE, 0            // Firmware update, data from AT
#endif

#if defined(CONFIG_SOC_8910) || defined(CONFIG_SOC_8811)||defined(CONFIG_SOC_8850)
+CFGCIOT,           atCmdHandleCFGCIOT, 0           // NB static NVM
#endif
#endif

#if defined(CONFIG_SOC_8910)||defined(CONFIG_SOC_8850)
#if defined(CONFIG_SOC_8850)
+SPCLEANINFO,       atCmdHandleSPCLEANINFO, AT_CON_NOT_CALIB_MODE       // This command is used to clean history ba into for all rat.
+SPLTEDUMMYPARA,    atCmdHandleSPLTEDUMMYPARA, AT_CON_NOT_CALIB_MODE    // This command is used to dual-mode LTE parameters for LTE.
#ifdef CONFIG_GSM_SUPPORT
+RRTMPARAM,         atCmdHandleRRTMPARAM, AT_CON_NOT_CALIB_MODE         // This command is used to dual-mode LTE parameters for GRR.
#endif
#else
+SPCLEANINFO,		atCmdHandleSPCLEANINFO, 0		// This command is used to clean history ba into for all rat.
#ifdef CONFIG_GSM_SUPPORT
+SPLTEDUMMYPARA,    atCmdHandleSPLTEDUMMYPARA, AT_CON_NOT_CALIB_MODE    // This command is used to dual-mode LTE parameters for LTE.
+RRTMPARAM, 		atCmdHandleRRTMPARAM, 0 		// This command is used to dual-mode LTE parameters for GRR.
#endif
#endif
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+CFGDFTPDN,         atCmdHandleCFGDFTPDN, 0
#endif
#endif

#if defined(CONFIG_SOC_8910)
#ifdef CONFIG_GSM_SUPPORT
+L1PARAM,           atCmdHandleL1PARAM, AT_CON_NOT_CALIB_MODE           // This command is used to set L1 dual-mode parameters of GSM.
#endif
+GRRLTEFREQ,        atCmdHandleGRRLTEFREQ, AT_CON_NOT_CALIB_MODE        // Set dummy FDD/TDD LTE freq configuration for GSM.
+CFGDFTPDNMODE,         atCmdHandleCFGDFTPDNMODE, 0
#endif

#if defined(CONFIG_SOC_8910)|| defined(CONFIG_SOC_8811)||defined(CONFIG_SOC_8850)
+VERCTRL,           atCmdHandleVERCTRL, 0
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifdef CONFIG_AT_SSIM_SUPPORT
+SSIMAUTH,          atCmdHandleSSIMAUTH, 0          // begin soft sim authentication
+SSIMAUTHEXIT,      atCmdHandleSSIMAUTHEXIT, 0      // exit soft sim authentication
+SSIMCHANGEPW,      atCmdHandleSSIMCHANGEPW, 0      // change authentication password
+SSIMINFO,          atCmdHandleSSIMINFO, 0          // set and query soft sim info
+SSIMPLMN,          atCmdHandleSSIMPLMN, 0          // set and query sim plmn
+SSIMFILEHASH,      atCmdHandleSSIMFILEHASH, 0      // get soft sim file hash value
+SSIMMSIN,          atCmdHandleSSIMMSIN, 0          // set and get the high 3 bit of the MSIN
+SSIMDELFILE,       atCmdHandleSSIMDELFILE, 0       // delete file
#endif

#ifdef CONFIG_AT_DM_SUPPORT
+SELFREGISTER,      atCmdHandleSELFREGISTER, 0      // self-register flag
#ifndef CONFIG_SOC_8811
+SETDMADPPLATFORM,  atCmdHandleSETDMADPPLATFORM, 0  // set platform address
#endif
#elif defined(CONFIG_AT_DM_CUS_SUPPORT)
+QDMPCFG, AT_CmdFunc_QDMPCFG, 0
+QDMPCFGEX, AT_CmdFunc_QDMPCFGEX, 0
#endif

#if defined(CONFIG_SOC_6760) || (defined(CONFIG_ATS_SGCC_CATM_SUPPORT) && defined(CONFIG_SOC_8910))
$MYGETKEY,      AT_TCPIP_CmdFunc_MYGETKEY, 0
$MYSETINFO,     AT_TCPIP_CmdFunc_MYSETINFO, 0
$MYTYPE,        AT_TCPIP_CmdFunc_MYTYPE, 0
$MYCALIB,       AT_TCPIP_CmdFunc_MYCALIB, 0
$MYSYSINFO,     atCmdHandleMYSYSINFO, 0
$MYSYSMODE,     atCmdHandleMYSYSMODE, 0
#endif

#if defined(CONFIG_AT_MYFTP_SUPPORT)
$MYFTPOPEN,     AT_GPRS_CmdFunc_MYFTPOPEN, 0
$MYFTPCLOSE,    AT_GPRS_CmdFunc_MYFTPCLOSE, 0
$MYFTPSIZE,     AT_GPRS_CmdFunc_MYFTPSIZE, 0
$MYFTPGET,      AT_GPRS_CmdFunc_MYFTPGET, 0
$MYFTPPUT,      AT_GPRS_CmdFunc_MYFTPPUT, 0
#elif defined(CONFIG_AT_NEWMYFTP_SUPPORT)
$MYFTPOPEN,     AT_MYFTP_CmdFunc_OPEN, 0
$MYFTPCLOSE,    AT_MYFTP_CmdFunc_CLOSE, 0
$MYFTPSIZE,     AT_MYFTP_CmdFunc_SIZE, 0
$MYFTPGET,      AT_MYFTP_CmdFunc_GET, 0
$MYFTPPUT,      AT_MYFTP_CmdFunc_PUT, 0
#endif

#if defined(CONFIG_AT_MYNET_TCPIP_SUPPORT)
$MYNETSRV,      AT_TCPIP_CmdFunc_MYNETSRV, 0
$MYNETOPEN,     AT_TCPIP_CmdFunc_MYNETOPEN, 0
$MYNETREAD,     AT_TCPIP_CmdFunc_MYNETREAD, 0
$MYNETWRITE,    AT_TCPIP_CmdFunc_MYNETSEND, 0
$MYNETCLOSE,    AT_TCPIP_CmdFunc_MYNETCLOSE, 0
$MYNETACK,      AT_TCPIP_CmdFunc_MYNETACK, 0
$MYNETACCEPT,   AT_TCPIP_CmdFunc_MYNETACCEPT, 0
$MYNETCREATE,   AT_TCPIP_CmdFunc_MYNETCREATE, 0
$MYNETCON,      AT_TCPIP_CmdFunc_MYNETCON, 0
$MYNETACT,      AT_GPRS_CmdFunc_MYNETACT, 0
$MYIPFILTER,    AT_TCPIP_CmdFunc_MYIPFILTER, 0
$MYNETURC,      AT_TCPIP_CmdFunc_MYNETURC, 0
$MYSOCKETLED,   AT_TCPIP_CmdFunc_MYSOCKETLED, AT_CON_NOT_CLAC
#endif
#endif

// NV
#ifndef CONFIG_SOC_8811
+NVPC,          atCmdHandleNVPC, 0
+NVGV,          atCmdHandleNVGV, 0
+RFCALIB,       atCmdHandleRFCALIB, 0
#endif

#ifdef CONFIG_SOC_8910
#ifndef CONFIG_QUEC_PROJECT_FEATURE
$MYCCID,        atCmdHandleCCID, 0
$MYPOWEROFF,    AT_TCPIP_CmdFunc_MYPOWEROFF, AT_CON_NOT_CLAC
+MYGMR,         atCmdHandleGMR, AT_CON_NOT_CLAC// (6.1.6) Request revision identification
#endif
#ifdef CONFIG_GSM_SUPPORT
$MYBCCH,        atCmdHandleMYBCCH, 0
+LOCKBCCH,      atCmdHandleMYBCCH, 0
+SETBAND,       atCmdHandleMYBAND, 0
$MYBAND,		atCmdHandleMYBAND, 0
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
+SETDTPORT,     atCmdHandleSETDTPORT, 0
+SPLMN,         atCmdHandleSPLMN, AT_CON_NOT_CALIB_MODE
#endif
+BANDINFO,      atCmdHandleBANDINFO, 0
+LCT,           atCmdHandleLCT, 0
+NASTIMER,		  atCmdHandleNASTIMER, 0  //set nas timerlength
+SNWPRIOR,		atCmdHandleSNWPRIOR, 0	//set or read search network priority
+ERRCSCFG,		atCmdHandleERRCSCFG, 0	//errc statistical information config
+ERRCMEAS,		atCmdHandleERRCMEAS,  0  //errc threshold measurement
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifdef CONFIG_AT_AP_CALL_CP_SUPPORT
+RFTEMPERATURE, atCmdHandleRFTEMPERATURE, 0
+SETLTEFRP, 	  atCmdHandleSETLTEFRP, 0
+SETRFFRP,		  atCmdHandleSETRFFRP, 0
#endif
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifdef CONFIG_AT_BT_APP_SUPPORT
+BTAPP,         atCmdHandle_BTAPP, AT_CON_NOT_CLAC //BT APP handle, used to running BT Demo
#endif
#endif

#ifdef CONFIG_SOC_8910
+SPBTCTRL,      atCmdHandleSPBTCTRL, AT_CON_NOT_CLAC      // BT control commands
#endif


#ifdef CONFIG_AT_BLUEU_VERIFY_SUPPORT
+SPBTTEST,      atCmdHandle_SPBTTEST, AT_CON_NOT_CLAC
+SPBLETEST,     atCmdHandle_SPBLETEST, AT_CON_NOT_CLAC
+SPBQBTEST,     atCmdHandle_SPBQBTEST, AT_CON_NOT_CLAC

#ifndef CONFIG_QUEC_PROJECT_FEATURE
+BTCOMM,        atCmdHandle_BTCOMM, AT_CON_NOT_CLAC
+BTHF,          atCmdHandle_BTHF,  AT_CON_NOT_CLAC
+BTAG,          atCmdHandle_BTAG,   AT_CON_NOT_CLAC
+BTSINK,        atCmdHandle_BTSINK, AT_CON_NOT_CLAC
+BTSRC,         atCmdHandle_BTSRC,  AT_CON_NOT_CLAC
+BTSPP,         atCmdHandle_BTSPP, AT_CON_NOT_CLAC


+BLECOMM,       atCmdHandle_BLECOMM, AT_CON_NOT_CLAC
+BLEADV,        atCmdHandle_BLEADV, AT_CON_NOT_CLAC
+BLESCAN,       atCmdHandle_BLESCAN, AT_CON_NOT_CLAC
+BLESMP,       atCmdHandle_BLESMP, AT_CON_NOT_CLAC
#endif
#endif
#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifdef CONFIG_AT_BT_CLASSIC_SUPPORT
+BTONOFF,       AT_BTApp_CmdFunc_ONOFF, 0
+BTVISIBLE,     AT_BTApp_CmdFunc_VISIBLE, 0
+BTNAME,        AT_BTApp_CmdFunc_NAME, 0
+BTADDR,        AT_BTApp_CmdFunc_ADDR, 0
+BTINQ,         AT_BTApp_CmdFunc_INQ, 0
+BTPAIR,        AT_BTApp_CmdFunc_PAIR, 0
+BTPIN,         AT_BTApp_CmdFunc_PIN, 0
+BTLISTPD,      AT_BTApp_CmdFunc_LISTPD, 0
+BTREMOVEPD,    AT_BTApp_CmdFunc_REMOVEPD, 0
+BTCONNECT,     AT_BTApp_CmdFunc_CONNECT, 0
+BTSEND,        AT_BTApp_CmdFunc_SEND, 0
+BTREAD,        AT_BTApp_CmdFunc_READ, 0
#endif
#endif


#ifdef CONFIG_AT_WIFISCAN_SUPPORT
#ifdef CONFIG_AT_8850_WIFI_SENTEST_SUPPORT
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+WIFION,      atCmdHandleWifiOpen,   0
+WIFIOFF,     atCmdHandleWifiClose,  0
+UWIFISENTEST,   atCmdHandleUWIFISENTEST, 0
#endif
+WIFISENTESTON, atCmdHandleWIFISENTESTON,  0
+WIFISENTESTOFF, atCmdHandleWIFISENTESTOFF,  0
+WIFISENTEST,   atCmdHandleWifiSensitivityTest, 0
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+WIFISCAN,    atCmdHandleWIFISCAN,   0
#endif
#else
+WIFION,      atCmdHandleWifiOpen,   AT_CON_NOT_CLAC
+WIFIOFF,     atCmdHandleWifiClose,  AT_CON_NOT_CLAC
+WIFISCAN,      atCmdHandleWifiScan,   AT_CON_NOT_CLAC
#ifdef CONFIG_AT_WIFI_SENSITIVITY_TEST_SUPPORT
+WIFISENTEST,   atCmdHandleWifiSensitivityTest, 0
#endif
#endif
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
+SIMHOTSWAP,    atCmdHandleSIMHOTSWAP, 0      //set sim hot-plug function
+GTSET,         atCmdHandleGTSET,      0      //set sim hot-plut trigger mode
#ifndef CONFIG_ATR_NBONLY_API_SUPPORT
+NETMSG,        atCmdHandleNETMSG,     0      //Get netinfo
+GTCCINFO,      atCmdHandleGTCCINFO,   0      //inqury GSM/LTE service cell and neighbor cell
#endif
#endif
#ifdef CONFIG_SOC_8910
#ifdef CONFIG_GSM_SUPPORT
+SETCSPAGFLAG,  atCmdHandleSETCSPAGFLAG, 0    // Set flag of ingnoring CS paging
#endif
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifndef CONFIG_SOC_8811
+UNINETLOG,   atCmdHandleUNINETLOG, 0
#endif
#endif

#ifdef CONFIG_SOC_8910
+CSVM,          atCmdHandleCSVM, 0

#ifndef CONFIG_QUEC_PROJECT_FEATURE_NW
+MMICG,         atCmdHandleMMICG, 0
#endif
+DRXTM,         atCmdHandleDRXTM, 0
+EFCI,			atCmdHandleEFCI, 0			  //Set electric fence cell info.
+CFGNVUPFLAG,   atCmdHandleCFGNVUPFLAG, AT_CON_NOT_CALIB_MODE|AT_CON_NOT_CLAC
#ifndef CONFIG_QUEC_PROJECT_FEATURE_NW
+QENG,          atCmdHandleQENG, AT_CON_NOT_CALIB_MODE // Get cell info
#endif
#endif

#if defined(CONFIG_SOC_8910)||defined(CONFIG_SOC_8850)
+NCESIM,        atCmdHandleNCESIM, 0          // close or open network card authentication
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+T3302,         atCmdHandleT3302, 0
+NSTCFG,        atCmdHandleNSTCFG, 0
+NST,           atCmdHandleNST, 0
+NSTGETRSSI,    atCmdHandleNSTGETRSSI, AT_CON_NOT_CLAC
+LOCREL,		atCmdHandleLOCREL, 0
#endif
#endif

#if defined(CONFIG_SOC_8850) || defined(CONFIG_SOC_8811)
+GETUID,              atCmdHandleGetUid, 0    //Get soc id
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifdef CONFIG_SOC_8811
+CAUTOATT,            atCmdHandleCAUTOATT,  0
+EDT,                 atCmdHandleEDT,  0
+USMTRF,              atCmdHandleUSMTRF, 0
+CGAPNRC,             atCmdHandleCGAPNRC, 0
+CRCES,               atCmdHandleCRCES, 0
+CCIOTOPT,            atCmdHandleCCIOTOPT, 0
+RESTORE,             atCmdHandleRESTORE, 0
+UNBCFG,              atCmdHandleUNBCFG, 0
+TUESTATS,            atCmdHandleTUESTATS, 0
+NVSETPOWERCLASS,     atCmdHandleNVSETPOWCLASS, 0
#ifdef NBIOT_POSITION_SUPPORT
+SETLPPCAPA,          atCmdHandleSETLPPCAPA, 0
#if defined(CONFIG_SOC_8850)
+CMOLR,               atCmdHandleCMOLR, 0
+CMOLRE,              atCmdHandleCMOLRE, 0
+CMTLR,               atCmdHandleCMTLR, 0
+CMTLRA,              atCmdHandleCMTLRA, 0
#else
+CMOLR,               atCmdHandleCMOLR, AT_CON_NOT_CLAC
+CMOLRE,              atCmdHandleCMOLRE, AT_CON_NOT_CLAC
+CMTLR,               atCmdHandleCMTLR, AT_CON_NOT_CLAC
+CMTLRA,              atCmdHandleCMTLRA, AT_CON_NOT_CLAC
#endif
#endif

#ifdef CONFIG_AT_NBIOTRAI_SUPPORT
*NBIOTRAI,      AT_TCPIP_CmdFunc_NBIOTRAI, 0
*RAIREQ,      AT_TCPIP_CmdFunc_RAIREQ, 0
#endif
#ifdef CONFIG_AT_CUS_SUPPORT
+EGACT,               atCmdHandleEGACT, 0
+NATSPEED,            atCmdHandleNATSPEED, 0
+NEARFCN,             atCmdHandleNEARFCN, 0
+NITZ,                atCmdHandleNITZ, 0
+NPTWEDRXS,           atCmdHandleNPTWEDRXS, 0
+NRB,                 atCmdHandleNRB, 0
+NUESTATS,            atCmdHandleNUESTATS, 0
+NIPINFO,             atCmdHandleNIPINFO, 0
+NCCID,               atCmdHandleNCCID, 0
+NCONFIG,             atCmdHandleNCONFIG, 0
+NBAND,               atCmdHandleNBAND, 0
+NCSEARFCN,           atCmdHandleNCSEARFCN, 0
+QSREGENABLE,       atCmdHandleQSREGENABLE, 0
+QDNS,                AT_TCPIP_CmdFunc_QDNS, 0
+NPING,               AT_TCPIP_CmdFunc_NPING, 0
+QIDNSCFG,            AT_TCPIP_CmdFunc_QIDNSCFG, 0
+NSOCR,             AT_TCPIP_CmdFunc_NSOCR, 0
+NSOST,             AT_TCPIP_CmdFunc_NSOST, 0
+NSOCO,             AT_TCPIP_CmdFunc_NSOCO, 0
+NSOSD,             AT_TCPIP_CmdFunc_NSOSD, 0
+NSOCL,              AT_TCPIP_CmdFunc_NSOCL, 0
+NSONMI,           AT_TCPIP_CmdFunc_NSONMI, 0
+NSORF,             AT_TCPIP_CmdFunc_NSORF, 0
+NFWUPD,            atCmdHandleNFWUPD, 0
+NSOSTF,            AT_TCPIP_CmdFunc_NSOSTF, 0
+NQSOS,            AT_TCPIP_CmdFunc_NQSOS, 0
+NSOSTATUS,            AT_TCPIP_CmdFunc_NSOSTATUS, 0
+NSNPD,            AT_TCPIP_CmdFunc_NSNPD, 0
+NRNPDM,            AT_TCPIP_CmdFunc_NRNPDM, 0
+NQPNPD,            AT_TCPIP_CmdFunc_NQPNPD, 0
#ifdef CONFIG_AT_OCEANCONNECT_SUPPORT
+QCFG,    AT_CmdFunc_QCFG, 0
+NCDP,   AT_CmdFunc_NCDP, 0
+QLWSREGIND,    AT_CmdFunc_QLWSREGIND, 0
+QLWULDATA,    AT_CmdFunc_QLWULDATA, 0
+QLWULDATAEX,    AT_CmdFunc_QLWULDATAEX, 0
+QLWULDATASTATUS,    AT_CmdFunc_QLWULDATASTATUS, 0
+QREGSWT,   AT_CmdFunc_QREGSWT,    0
+NMGS,    AT_CmdFunc_NMGS, 0
+NMGR,    AT_CmdFunc_NMGR, 0
+NNMI,    AT_CmdFunc_NNMI, 0
+NSMI,    AT_CmdFunc_NSMI, 0
+NQMGR,    AT_CmdFunc_NQMGR, 0
+NQMGS,    AT_CmdFunc_NQMGS, 0
+NMSTATUS,        AT_CmdFunc_NMSTATUS, 0
+QLWSERVERIP,        AT_CmdFunc_QLWSERVERIP, 0
+QSECSWT,       AT_CmdFunc_QSECSWT,     0
+QSETPSK,       AT_CmdFunc_QSETPSK,     0
+QLWFOTAIND,        AT_CmdFunc_QLWFOTAIND,      0
+QRESETDTLS,        AT_CmdFunc_QRESETDTLS,      0
+QDTLSSTAT,     AT_CmdFunc_QDTLSSTAT,       0
+QBOOTSTRAPHOLDOFF,     AT_CmdFunc_QBOOTSTRAPHOLDOFF,       0
+QCRITICALDATA,     AT_CmdFunc_QCRITICALDATA,       0
+QSETBSPSK,     AT_CmdFunc_QSETBSPSK,       0
+QBSSECSWT,     AT_CmdFunc_QBSSECSWT,       0
+QLWEVTIND,     AT_CmdFunc_QLWEVTIND,       0
#endif
#endif
#endif
#endif
#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifndef CONFIG_ATR_NBONLY_API_SUPPORT
+COCSIM,              atCmdHandleCOCSIM, 0
+CSSUP,               atCmdHandleCSSUP, AT_CON_NOT_CLAC
+CSRES,               atCmdHandleCSRES, AT_CON_NOT_CLAC
#endif
#ifdef CONFIG_AT_UTEST_SUPPORT
+UTEST,               atCmdHandleUTEST, AT_CON_NOT_CLAC
+SETSIGQUA,           atCmdHandleSETSIGQUA,   AT_CON_NOT_CLAC
#endif

#ifdef CONFIG_LPA_SUPPORT
+LPA,       atCmdHandleLPA, 0
#endif
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#if defined(CONFIG_AT_U_CREATE_AT_CHANNEL) && defined(CONFIG_USB_DEVICE_SUPPORT)
#ifdef CONFIG_SOC_8850
+UCREATEATCH,         atCmdUCreateAtChannel, AT_CON_NOT_CLAC
#else
+UCREATEATCH,         atCmdUCreateAtChannel, 0
#endif
#endif

#ifdef CONFIG_AT_CTIOTAEP_SUPPORT
+CTLWVER,                 AT_NBIOT_CmdFunc_CTLWVER, 0
+CTLWGETSRVFRMDNS,        AT_NBIOT_CmdFunc_CTLWGETSRVFRMDNS, 0
+CTLWSETSERVER,           AT_NBIOT_CmdFunc_CTLWSETSERVER, 0
+CTLWSETLT,               AT_NBIOT_CmdFunc_CTLWSETLT, 0
+CTLWSETPSK,              AT_NBIOT_CmdFunc_CTLWSETPSK, 0
+CTLWSETAUTH,             AT_NBIOT_CmdFunc_CTLWSETAUTH, 0
+CTLWSETPCRYPT,           AT_NBIOT_CmdFunc_CTLWSETPCRYPT, 0
+CTLWSETMOD,              AT_NBIOT_CmdFunc_CTLWSETMOD, 0
+CTLWREG,                 AT_NBIOT_CmdFunc_CTLWREG, 0
+CTLWUPDATE,              AT_NBIOT_CmdFunc_CTLWUPDATE, 0
+CTLWDTLSHS,              AT_NBIOT_CmdFunc_CTLWDTLSHS, 0
+CTLWDEREG,               AT_NBIOT_CmdFunc_CTLWDEREG, 0
+CTLWGETSTATUS,           AT_NBIOT_CmdFunc_CTLWGETSTATUS, 0
+CTLWCFGRST,              AT_NBIOT_CmdFunc_CTLWCFGRST, 0
+CTLWSESDATA,             AT_NBIOT_CmdFunc_CTLWSESDATA, 0
+CTLWSEND,                AT_NBIOT_CmdFunc_CTLWSEND, 0
+CTLWRECV,                AT_NBIOT_CmdFunc_CTLWRECV, 0
+CTLWGETRECVDATA,         AT_NBIOT_CmdFunc_CTLWGETRECVDATA, 0
#endif

#ifdef CONFIG_WDT_ENABLE
+UWDTCTL,             atCmdUWdtCtlHandle, 0
#endif
+SYSWDTCTL,           atCmdSysWdtCtlHandle, AT_CON_NOT_CALIB_MODE
#endif

#ifdef CONFIG_AT_ARMLOG_SUPPORT
+ARMLOG, atCmdHandleARMLOG, AT_CON_NOT_CLAC
+SPLOGLEVEL, atCmdHandleAT, AT_CON_NOT_CLAC
+SPDSPOP, atCmdHandleAT, AT_CON_NOT_CLAC
+SPPHYLOG, atCmdHandleAT, AT_CON_NOT_CLAC
+SPCAPLOG, atCmdHandleAT, AT_CON_NOT_CLAC
+SPCAPLEN, atCmdHandleAT, AT_CON_NOT_CLAC
#endif

#ifdef CONFIG_SOC_8811
+INFRARED, atCmdHandleINFRARED, 0          // infrared communication function
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifdef CONFIG_DUAL_SIM_SUPPORT
+SETSIM,        atCmdHandleSetSim,0
#endif

#ifdef AT_EXT_CONF_SUPPORT
+CGU,         atCmdHandleCGU, AT_CON_NOT_CALIB_MODE
#endif

#ifdef TEST_INTEGRATE_AP_AT
+OFF,     AT_GC_CmdFunc_OFF_test, 0
#endif
#endif

#ifdef CONFIG_PAM_LTE_GNSS_WIFISCAN_SUPPORT
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
+CGNSSON, atCmdHandleCGNSSON, 0
+CGNSSOFF, atCmdHandleCGNSSOFF, 0
+CGNSSST, atCmdHandleCGNSSST, 0
+SPGPSTEST, atCmdHandleSPGPSTEST, 0
#endif
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+CGNSSRESTART, atCmdHandleCGNSSRESTART, 0
+CGNSSR, atCmdHandleCGNSSR, 0
+CGNSSDMH, atCmdHandleCGNSSDMH, 0
+CGNSSOLIF, atCmdHandleCGNSSOLIF, 0
+CGNSSD, atCmdHandleCGNSSD, 0
+CGNSSEPHSAVE, atCmdHandleCGNSSEPHSAVE, 0
+LOCPRIOR, atCmdHandleLOCPRIOR,  0
+LOCSLEEP, atCmdHandleLOCSLEEP,  0
+LOCUSEAGNSS, atCmdHandleLOCUSEAGNSS, 0
+LOCGNSSRESUME, atCmdHandleLOCGNSSRESUME, 0
#endif
#endif //CONFIG_PAM_LTE_GNSS_WIFISCAN_SUPPORT

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#if (defined(CONFIG_PAM_LTE_GNSS_WIFISCAN_SUPPORT) || defined(CONFIG_SOC_8910))
+CMOLR,               atCmdHandleCMOLR, 0
+CMOLRE,              atCmdHandleCMOLRE, 0
+CMTLR,               atCmdHandleCMTLR, 0
+CMTLRA,              atCmdHandleCMTLRA, 0
#endif

#if defined(CONFIG_SOC_8910)||defined(CONFIG_SOC_8850)
+QENG,          atCmdHandleQENG, AT_CON_NOT_CALIB_MODE // Get cell info
#endif

#endif //CONFIG_AT_CMD_SUPPORT

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#if defined(CONFIG_AUDIO_SET_LOOPBACK_GAIN_SUPPORT) && defined(CONFIG_SOC_8910)
+AUDLBGAIN,        atCmdHandleAUDLBGAIN,AT_CON_NOT_CLAC
#endif

#ifdef CONFIG_CONCAT_SMS_SUPPORT
+CONCATMSG,    atCmdHandleCONCATMSG, AT_CON_NOT_CALIB_MODE
#endif

+ADCVOLT,      atCmdHandleADCVOLT, AT_CON_NOT_CLAC

#ifdef CONFIG_USB_HOST_SUPPORT
+UsbHost,      atCmdHandleUsbHost, 0
+MTP,          atCmdHandleMTP, 0
+MassStorage,   atCmdHandleMassStorage, 0
#endif

#ifndef CONFIG_SOC_8811
+MOSRC,   atCmdHandleMOSRC, 0 // MO SMS RETRY COUNT
#endif

#if defined(CONFIG_SOC_8910) || defined(CONFIG_SOC_8850)
+CUSPPSM,       atCmdHandleCUSPPSM,     // customer private PSM
+CUSDLPSM,      atCmdHandleCUSDLPSM,    // customer downlink PSM parameters

+ULB,           atCmdHandleULB, AT_CON_NOT_CALIB_MODE|AT_CON_NOT_CALIB_MODE   //unisoc lte band

+EPSNCE,        atCmdHandleEPSNCE, 0
#endif
#if defined(CONFIG_AT_I2S_TEST_SUPPORT) && defined(CONFIG_SOC_8811)
+I2STEST,   atCmdHandleI2STest, 0
#endif
#ifndef CONFIG_QUEC_PROJECT_FEATURE
+CUSCMER,          atCmdHandleCUSCMER, 0          // 8.10 Custom Mobile termination event reporting
#endif

#ifdef CONFIG_TFM_TEST_SUPPORT
+TFMTEST,   atCmdHandleTfmTest, 0
#endif

+ISIM,          atCmdhHandleISIM, AT_CON_NOT_CALIB_MODE|AT_CON_NOT_CLAC
#endif

#ifdef CONFIG_SOC_8910
+TAU,           atCmdHandleTAU, 0
+COPST,         atCmdHandleCOPST, 0
+HISFREQ,       atCmdHandleHISFREQ, 0   // history frequency and plmn info

+HNBN,          atCmdHandleHNBN, AT_CON_NOT_CLAC

+UMAF,          atCmdHandleUMAF, 0
+USIMP,         atCmdHandleUSIMP, 0

+CUSCMER,       atCmdHandleCUSCMER, 0          // Custom Mobile termination event reporting
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
#ifdef CONFIG_SOC_8850
^UCMEM,         atCmdHandleUCMEM    //unisoc CP memory info
#endif
#endif

#endif
#endif
