#ifndef __UDS_H
#define __UDS_H

#include "config.h"
/***************************************************/
/*******	MACRO DEFINATION SECTION             	****/
/***************************************************/
/*******	ADDRESS DEFINATION            	  ***********/
#define BPCM_BOOT_FUNCTIONAL_ADDRESS				0x441
#define BPCM_BOOT_PHYSICAL_REQUEST_ADDRESS			0x7D6
#define BPCM_BOOT_PHYSICAL_RESPONSE_ADDRESS		0x7D7
/***************************************************/


/*******	Diagnostic seesion Type            	  ***********/
#define DEFAULT_DIAG_SESSION 				1
#define PROGRAMMING_DIAG_SESSION 		2
#define EXTENDED_DIAG_SESSION 			3
/***************************************************/

#ifdef BOOT_MODE
/*******	Security Mode Type            	  	  ***********/
#define SECURITY_MODE_OFF 					0
#define SECURITY_SEED_REQ 					1
#define SECURITY_KEY_VALID		 			2
/***************************************************/

/*******	Write Fingerprint Mode Type        	  ***********/
#define NOT_WRITE_FINGERPRINT	 			0
#define WRITE_FINGERPRINT_ALREADY			1
/***************************************************/

#else
/*******	Communication  Control        	  ***********/
#define COMM_ENBRX_DISTX					1
#define COMM_ENBRX_ENBTX					0
/***************************************************/

/*******	DTC Type			        	  ***********/
#define DTC_TYPE_ON							1
#define DTC_TYPE_OFF						2
/***************************************************/

/*******	FAIL SAFE REACTION	        	  ***********/
#define DIS_FAIL_SAFE_REACTION				1
#define ENB_FAIL_SAFE_REACTION			2
/***************************************************/
#endif

/*******	SERVICE ID			        	  ***********/
#define START_DIAGNOSTIC_SESSION			0x10
#define ECU_RESET							0x11
#define READ_DATA_BY_IDENTIFIER			0x22

#ifdef BOOT_MODE
//	Service ID For BOOT ONLY
#define SECURITY_ACCESS					0x27
#define WRITE_FINGERPRINT					0x2E
#define REQUEST_DOWNLOAD					0x34
#define REQUEST_UPLOAD						0x35//not need to support
#define TRANSFER_DATA						0x36
#define REQUEST_TRANSFER_EXIT				0x37
#else
//	Service ID For Application ONLY
#define CLEAR_DIAG_INFO					0x14
#define READ_DTC_INFO						0x19
#define READ_MEMORY_BY_ADDR				0x23//not need to support
#define COMMUNICATION_CONTROL			0x28
#define DYNAMIC_DEFINE_DATA_ID			0x2C//not need to support
#define WRITE_DID							0x2E
#define CONTROL_DEVICE						0x2F
#define WRITE_MEMORY_BY_ADDR				0x3D//not need to support
#define CONTROL_DTC_SETTING				0x85
#endif

#define ROUTINE_CONTROL					0x31
#define TESTER_PRESENT						0x3E
#define ECU_PASSIVE_MODE					0xA0//not need to support
/***************************************************/


/*******	Response Type		        	  ***********/
#define  POS_RESPONSE 						0x00
#define  NEG_RESPONSE 						0x01
#define  FLOW_CONTROL_RESPONSE 			0x02
#define  FLOW_CONTROL_DATA	 			0x03
/***************************************************/


/*******	Negative Response Trouble Code Type  *******/
#define NRTC_SERVICE_NOTSUPPORTED		0x11
#define NRTC_SUB_FUN_NOTSUPPORTED		0x12
#define NRTC_INCORR_MSG_LENGTH			0x13
#define NRTC_CON_NOT_CORRECT				0x22
#define NRTC_REQ_SEQ_ERROR				0x24
#define NRTC_REQ_OUT_OF_RANGE			0x31
#define NRTC_SECURITY_ACCESS_DENIED		0x33
#define NRTC_INVALID_KEY					0x35	
#define NRTC_EXCEEDNUM_OF_ATTEMPT		0x36
#define NRTC_REQTIME_DELAY_NOEXPIRED		0x37
#define NRTC_UPDOWNLOAD_NOT_ACCEPT		0x70	
#define NRTC_TRANSFER_DATA_SUSPEND		0x71	
#define NRTC_GEN_PRO_FAIL					0x72	
#define NRTC_WRONG_BLOCKSEQ_COUNTER		0x73
#define NRTC_SERCORRECT_RSPPENDING		0x78
#define NRTC_VOL_TOO_HIGH					0x92
#define NRTC_VOL_TOO_LOW					0x93
/***************************************************/


#ifdef BOOT_MODE
/*******	BPCM APPLICATION SEGMENT DEFINITION  *******/
#define APPLICATION_PROG_START_ADDR		(0x0)
#define APPLICATION_CODE_START_ADDR		(0xc00)
#define APPLICATION_CODE_END_ADDR		(0x02AC00)
#define APPLICATION_PROG_END_ADDR		(0xFFFFFF)
/***************************************************/

/*******	BATT APPLICATION SEGMENT DEFINITION  *******/
#define BATTAPP_CODE_START_ADDR			(0x400)
#define BATTAPP_CODE_END_ADDR			(0x010000)
/***************************************************/

/***************************************************/
#define MAX_NUM_OF_BLOCKLEN_PARA			(0x4)
#define MAX_NUM_OF_BLOCK_FOR_PROG		(512*4+1)
#define MAX_SECURITY_ATTEMPT_NUM			(5)
/***************************************************/
#endif
/*******	Request Information Type DEFINITION  *******/
#define DIAG_DEFAULT_SESSION_REQ_FROM_BOOT 				1
#define DIAG_PROG_SESSION_REQ_FROM_APP		 			2
#define RESET_REQ_FROM_BOOT 								3
#define DIAG_DEFAULT_SESSION_REQ_FROM_BOOT_NORSP		4
#define DIAG_PROG_SESSION_REQ_FROM_APP_NORSP		 	5
#define RESET_REQ_FROM_BOOT_NORSP 						6
#define COMMUNICATION_TIME_OUT_BOOT 						7
#define RESET_REQ_FROM_APP 								8
/***************************************************/


/*******	UDS Layer MODE Flags	  			*******/
#define DATA_RV_MODE  					1
#define PROGRAMMING_MODE  				2
#define DATA_STORE_MODE  				3
#define GATEWAY_MODE	  				4
#define DIAG_MODE	  					5
#define RSP_MODE	  					6
/***************************************************/


#ifdef BOOT_MODE
/*******	PROGRAMMING_MOD	  			*******/
#define PROGRMMING_MODE_DISABLE 		0
#define PROGRMMING_MODE_ENABLE		1
#define PROGRMMING_ON_GOING	 		2
/***************************************************/
#endif
/*******	DATA_RV_MODE	  				*******/
#define NORMAL_PROCESSING				0
#define FLOW_CONTROL_BEGIN			1
#define FLOW_CONTROL_CONTINUE		2
/***************************************************/

/*******	DATA_STORE_MODE				*******/
#define DATA_STORING					1
#define DATA_IVT_STORED				2
#define DATA_READING					3
#define DATA_STORED					4
/***************************************************/


/*******	Response mode					*******/
#define RSP_PENDING						1
#define RSP_NO_PENDING					0
/***************************************************/




/*******	Timer defination					*******/
#define TIMER_PERIOD_5S  				0
#define TIMER_PERIOD_10MS  				1
#define TIMER_PERIOD_5MS  				2
#define TIMER_PERIOD_1S  				4
/***************************************************/

/*******	Sub Function Check Type			*******/
#define ONE_BYTE_CHECK					1
#define TWO_BYTES_CHECK				2
#define THREE_BYTES_CHECK				3
/***************************************************/




/*******	 Read data by identifier(DID)			*******/
enum{
	READ_PROG_ATTEMPT_COUNTER=0,
	READ_VIN_ODOMETER,
	READ_ODOMETER,
	READ_RESPONSE_ON_EVENT_LIGHT_ACTIVATION_STATE,
	READ_ACTIVE_DIAG_INFO,
	READ_ECU_ORIGIN,
	READ_ECU_CONFIGURATION,
	READ_SPEC_INFO,
	READ_HARD_PART_NO,
	READ_SOFT_PART_NO,
	READ_ECU_PART_NO,
	READ_HARD_VER_INFO,
	READ_SOFT_VER_INFO,
	READ_BOOT_VER_INFO,
	READ_HARD_SUPP_ID,
	READ_SOFT_SUPP_ID,
	READ_VEHICLE_INFORMATION,	
	READ_FINGERPRINT,
	READ_SOFTWARE_MODULE_INFORMATION,
	READ_SOFTWARE_MODULE_INFORMATION_CHANNEL_1,
	READ_PHYSICAL_LAYER_CHANNEL_CONFIGURATION,
	READ_PHYSICAL_LAYER_CHANNEL_CONFIGURATION_CHANNEL_1,
	READ_ECU_SERIAL_NO,
	READ_HW_DEVICE_NO,
	
	READ_VIN_ORIGINAL,
	READ_VIN_CURRENT
};
#define READ_DATA_BY_ID_MAX			(READ_VIN_CURRENT+1+1)
/***************************************************/

#define UDS_DATA_TRANSMIT_LEN		400
/*******	 UDS Parameter SIZE		*******/
#define UDSPara_SIZE			(157)
/***************************************************/

/*******	 Write data by identifier(DID)			*******/
enum{
	WRITE_DID_FINGERPRINT=0,
	WRITE_DID_VIN_ODO_COUNTER,
	WRITE_DID_ODOMETER,
	WRITE_DID_RSP_ON_EVENT,
	WRITE_DID_ECU_PART_NUMBER,
	WRITE_DID_VIN_CURRENT,
	WRITE_DID_SOC,
	WRITE_DID_HW_ID,
	WRITE_DID_CONTACTOR_ENABLER,
	WRITE_DID_MEC_COUNTER,
	WRITE_DID_EOL_SCRATCH_PAD,
	WRITE_DID_HIDDEN_SOC,
	WRITE_DID_CANLABLE_PACK_NO,
	WRITE_DID_FORCE_TO_WRITE,
	//Added by Amanda for DTC Disable/Enable Dec20,2010
	WRITE_DID_DTC_ENABLER
};
//#define WRITE_DATA_BY_ID_MAX			(WRITE_DID_FORCE_TO_WRITE+1)
#define WRITE_DATA_BY_ID_MAX			(WRITE_DID_DTC_ENABLER+1)


/***************************************************/
/*******	STRUCTER DEFINATION SECTION             	****/
/***************************************************/
typedef struct{
   UINT16 uiDataLen;
   UINT8 uiSID;
   UINT8 uiSUB[3];
#ifdef BOOT_MODE
   UINT8 uiData[MAX_NUM_OF_BLOCK_FOR_PROG];
#else
   UINT8 uiData[512];
#endif
}UDS_DATA;


typedef struct{
	UINT8 uiRequestID;		    	//Addr 1 important
	UINT8 uiApplicationInfo;		//Addr15 important
	UINT8 uiCheckSum;
} sProgPara;

typedef union
{
	UINT8   uiFanControlByte; 
	struct{
		UINT8 FanControl:1,
			FanType:2,
 		     Reserved:5; 
	};
}FanControlByte;

typedef struct{
	UINT8 uiCoolingDataByCDA;
	FanControlByte FanControlByCDA;
}FanControl;

typedef struct{
	UINT8 uiLogBlock;
	UINT8 uiSupplierID_H;
	UINT8 uiSupplierID_L;
	UINT8 uiProgDate_Y;
	UINT8 uiProgDate_M;
	UINT8 uiProgDate_D;
	UINT8 uiToolSerialNo_H;
	UINT8 uiToolSerialNo_M;
	UINT8 uiToolSerialNo_L;
	UINT8 uiToolSerialNo_LL;
} sFingerprint;

typedef struct{
	UINT8 uiFirstByte;			//no use
	UINT8 uiSessionMode;		//no use
	UINT8 uiProgAtteNo_H;
	UINT8 uiProgAtteNo_L;
	UINT8 uiProgMaxNo_H;
	UINT8 uiProgMaxNo_L;
// above total is 6
	UINT8 uiHardVer_YEAR;		//Master board HW version
	UINT8 uiHardVer_WEEK;		//Master board HW version
	UINT8 uiHardVer_PATCH;		//Master board HW version
	UINT8 uiBootSoftVer_YEAR;	//Master board Boot SW version	
	UINT8 uiBootSoftVer_WEEK;	//Master board Boot SW version	
	UINT8 uiBootSoftVer_PATCH;	//Master board Boot SW version	
	UINT8 uiAppSoftVer_YEAR;	//Master board SW version	
	UINT8 uiAppSoftVer_WEEK;	//Master board SW version	
	UINT8 uiAppSoftVer_PATCH;	//Master board SW version	
// above total is 6+9=15
	UINT8 uiHDSupplierID_H;
	UINT8 uiHDSupplierID_L;//current count is 14
// above total is 6+9+2=17
	sFingerprint uiFingerprint;//10  
// above total is 17+10=27
	UINT8 uaVinORG[17];
// above total is 27+17=44
	UINT8 uaEcuPartNumber[10];
// above total is 44+10=54
	UINT8 uaVinCurrent[17];
// above total is 54+17=71
	UINT8 uaOdometer[3];
// above total is 27+47=74
	UINT8 uiVinOdometer;
	UINT8 uiRspOnELActState;
	UINT8 uaECUSerialNo[20];//use 19
// above total is 76+20=96
	UINT8 uiMBHWVersion;		//Master board inner ID
	UINT8 uiHVBHWVersion;		//High voltage board inner ID
	UINT8 uiBattHWVersion;		//Battery board inner ID
	UINT8 uiBleedHWVersion;		//Bleed board inner ID
	UINT8 uiHVBSWVer_YEAR;		//High voltage board's SW version
	UINT8 uiHVBSWVer_WEEK;		//High voltage board's SW version
	UINT8 uiHVBSWVer_PATCH;	//High voltage board's SW version
	UINT8 uiBattSWVer_YEAR;		//Battery board's SW version
	UINT8 uiBattSWVer_WEEK;	//Battery board's SW version
	UINT8 uiBattSWVer_PATCH;	//Battery board's SW version
// above total is 96+10=106
	UINT8 uiContactorEnabler;
	UINT8 uiMECCounter;
	UINT8 uiEOLScratchPad[5];
	UINT8 uiHiddenSOC;
	UINT8 uiCANLable_PackNo_H;
	UINT8 uiCANLable_PackNo_L;
	UINT8 uiForceToWriteFlag;
	//Added by Amanda for DTC Disable/Enable Dec20,2010
//	UINT8 uiReserverByte[39];
	UINT8 uiDTCGroupEnabler[22];//add one byte by Amanda 
	UINT8 uiPassConterDTC;
	UINT8 uiReserverByte[16];
//	UINT8 uiDTCGroupEnabler[21];//add one byte by Amanda 
//	UINT8 uiReserverByte[18];
	UINT8 uiCheckSum;//finished 157
} sUDSPara;

/***************************************************/
/******* EXTERN VARIABLES DEFINATION SECTION      	****/
/***************************************************/
extern sUDSPara UDSWriteInfo;
extern sUDSPara UDSReadInfo;


extern sProgPara ProgInfo;


extern FanControl FanControlData;
extern UINT8 uiIsoTestByCDA;

////////////////////////External Variables//////////////////////////////
extern UDS_DATA UDSRvDataPara;
#ifdef BOOT_MODE
//extern UINT8	uiReadDataByID_INFO[READ_DATA_BY_ID_MAX][4];
#endif
extern UINT8 uaUDSReqPara[8];

//temp for extern 
extern UINT8 uiProgCounter;//store to eeprom
//get from 0x36 msg (Transfer Data)
extern UINT8 uiBlockSeqCounter;
extern UINT8 uiErrorCounter;
extern UINT16 uiFinalBlockNo;
extern UINT32 uiReceivedSize;
extern UINT8 uiTxMode;
extern sFingerprint uiFingerprint;

#ifdef GATEWAY_SUPPORT
extern UINT8 uaUDSGateWayPara[8];
extern UINT8 uaUDSGateWayRsp[8];
#endif

/***************************************************/
/*******	FUNCTIONS DEFINATION SECTION            	****/
/***************************************************/
UINT8 uiAppliValidCheck(void);
void uvSetMode(UINT8 uiModeType, UINT8 uiModeValue);
UINT8 uiGetMode(UINT8 uiModeType);

#ifdef BOOT_MODE
void uvSetBootDiagSessionMode(UINT8 uiSessionMode);
UINT8 uiGetBootDiagSessionMode(void);
void uvSetSecurityMode(UINT8 uiSecurity);
UINT8 uiGetSecurityMode(void);
UINT8 uiChkProgPrecFromBoot(void);
void nvGenRandomSeed(UINT8 *pSeed);
void nvGenKey(UINT8 *pKey,UINT8 *pSeed);
UINT8 uiCompareKey(UINT8 *pRvKey,UINT8 *pInnerKey, UINT8 uiSize );
UINT8 uiBootReadReqInfo(void);
UINT8 uiCheckRoutine(void);
void nvStoreFingerPrintData(void);
UINT8 uiEraseBlock(void);
UINT8 uiCheckAppliMemory(void);
void uvInitBeforeDownload(void);
UINT8 uvBootReadAllUDSFlags(void);
UINT8 nuBootGetReadID(void);
void uvProcessData(void);
#else
UINT8 uiChkProgPrecFromApp(void);
UINT8 uiGetAppDiagSessionMode(void);
void uvSetAppDiagSessionMode(UINT8 uiSessionMode);
void uvSetFailSafeMode(UINT8 uiFailSafeMode);
UINT8 uiGetFailSafeMode(void);
UINT8 uiGetCommType(void);
void uvSetCommType(UINT8 uiCommType);
UINT8 uiGetDTCType(void);
void nvSetDTCType(UINT8 uiDTCtype);
void	 uvAppClearProgReqFlag(void);
UINT8 uiAppReadReqInfo(void);
UINT8 uvAppReadAllUDSFlags(void);
UINT8 nuAppGetReadID(void);
void nvStoreDID(UINT8 uiDIDtype);
UINT8 uiResetVIN(void);

#endif
void	uvStartTimer(UINT8 uiType);
void nvFCData(void);
void nvSendUDSNegResp(UINT8 uiSIType,UINT8 uiRspNegType);
void nvSendUDSPosResp(UINT8 uiSIType,UINT8 * ptDataAddr, UINT16 uiDataSize);
UINT8 uvChkFlowControl(void);
void uv5mStimerout(void);
UINT8 uvSendLastOptResp(void);
UINT8 uiIsTimeout(UINT8 uiType);
void initUDSData(void);
void uv5Stimerout(void);
void uvSetReqInfo(UINT8 uiReqInfo);
UINT8 uvProcessMsgData(void);
void uv30mstimerout(void);
UINT8 uvReadProgFlags(UINT8 uiReadType);

#ifdef GATEWAY_SUPPORT
void nvSendReqGateway(void);
void nvSendRspGateway(void);
#endif
void uvWriteDeviceID(UINT8 uiDeviceID);
UINT8 uiGetDeviceID(void);


#endif

