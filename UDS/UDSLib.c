/****************************************************************************************************
 *	UDS Diagnostic Layer(common lib).
 *    
 *	Auther: 
 *
 *   (c) 2010 ElectroVaya
 *
 *******************************************************************************************************/
/***************************************************/
/*******	INCLUDING FILES DEFINATION SECTION	****/
/***************************************************/
#if 0
#include "p33FJ256GP710.h"
#include "UDSLayerBoot.h"
#include "Timing.h"
#include "ECANDrv.h"
#include "FlashApp.h"
//#include "Eeprom.h"
#include "SPIDrv.h"
#include "UDScrc32.h"
#include "UDSLib.h"
#include "FlashSPI.h"

#else
#include "p33FJ256GP710.h"
#include "UDSLayerApp.h"
#include "Timing.h"
#include "HAL.h"
#include "ecu.h"
#include "SPIDrv.h"
#include "ECANDrv.h"
#include "UDSLib.h"
#include "Config.h"
#include "Controller.h"
#include "Vehicle.h"
#include "UDS-PID.h"
#include "FlashSPI.h"

#endif

/***************************************************/
/*******	MACRO DEFINATION SECTION             	****/
/***************************************************/

#ifdef BOOT_MODE
#define REAL_ENV 						0
#define MAX_REPROGRAMMING_CYCLES		0xffffffff
#endif



/***************************************************/
/*******	VARIABLES DEFINATION SECTION            	****/
/***************************************************/

//temp to extern 
UINT8 uiProgCounter=0;//store to eeprom
//get from 0x36 msg (Transfer Data)
UINT8 uiBlockSeqCounter;
UINT8 uiErrorCounter;
UINT16 uiFinalBlockNo;

//Add on 03/22/2010 for fix flash bootloader's bug --unexcept resposne
UINT16 uiCanSID;
UINT8 uiDataIDindex[100];

///////////////////////////////////////////////////////////
//for external usage
/* For Read data by identifier */
#ifdef BOOT_MODE
//UINT8 uiReadDataByID_INFO[READ_DATA_BY_ID_MAX][4];
UINT8 uaBuffer[30];
#else
UINT8 uaBuffer[UDS_DATA_TRANSMIT_LEN];
#endif
UINT16 uiSendSize;
//////////////////////////////////////////////////////////
//for inter usage


TICK checkTick;
TICK writeEepromTick;
TICK FlowControlTxTick;
TICK ResetTimeOutTick;


//set by 0x2E msg 
sFingerprint uiFingerprint;


UINT8 uiRequestID=0;

typedef struct{
	UINT8 uiWritingMode;
	UINT8 uiDataRvMode;	
#ifdef BOOT_MODE
	UINT8 uiProgrammingMode;//get from 0x31 msg	
#endif
	UINT8 uiGateWayMode;
	UINT8 uiDiagMode;
	UINT8 uiRSPMode;
}UDS_MODE;
UDS_MODE UDS_ModeFlags;


#ifdef BOOT_MODE
#else
typedef union
{
	UINT8   uiDataByte; 
	struct{
		UINT8 Bit0:1,
			Bit1:1,
 		     Bit2:1, 
 		     Bit3:1, 
 		     Bit4:1, 
 		     Bit5:1, 
 		     Bit6:1, 
 		     Bit7:1; 
	};
}WriteDIDByte;
#endif

sProgPara ProgInfo;

sUDSPara UDSReadInfo;
sUDSPara UDSWriteInfo;


//*******************************Receiving Data*************************//
//recieved data buffer(8bytes) via can bus 
UINT8 uaUDSReqPara[8];
UDS_DATA UDSRvDataPara;
UINT32 uiReceivedSize;
UINT16 uiOffset;
UINT8  uiSubFuncBytes;
////////////////////////////////////////////////////////////////////////

//*******************************Sending Data*************************//
UINT8 uiNegRspTroubleCode=0xff;
//sending data buffer(8bytes)
UINT8 DataBufferBytes[8];
//TX buffer---for sending data
#ifdef BOOT_MODE
UINT8 uiTxData[512];
#else
//UINT8 uiTxData[256];
UINT8 uiTxData[UDS_DATA_TRANSMIT_LEN];
#endif

UINT8 uiTxMode;
UINT8 uiStartBlockSeq;//be used for flow control data sending
UINT16 uiLeftBytes;
UINT8 uiSendBlockNo;//be used for flow control data sending
////////////////////////////////////////////////////////////////////////
UINT8 uiSIDOfFlowControl;
#ifdef BOOT_MODE
UINT32   uiTempAddr;
//*************************For Programming Data*************************//
UINT8 uaProgBufferBytes[MAX_NUM_OF_BLOCK_FOR_PROG];
UINT8 uaProgBufferIVT[MAX_NUM_OF_BLOCK_FOR_PROG];
UINT8 uaBootMemoryBackup[MAX_NUM_OF_BLOCK_FOR_PROG];
////////////////////////////////////////////////////////////////////////

//set by 0x10 msg 
UINT8 uiBootSessionMode=1;
//get from 0x27 msg 
UINT8 uiBootSecurityMode=0;
//get from 0x36 msg (Transfer Data)
UINT32 uiRvDataByteSum;   //
// 0x31 msg 
UINT8 uiSoftValid=0;

//calculated checksum value
UINT32 uiCalcCRC;//for verify the sofware
UINT16 uiAttemptCounter;//setted by erase routine
UINT16 uiMaxProgCounter;


#else
//set by 0x10 msg 
UINT8 uiAppliSessionMode;
//set by 0x85 msg (DTC setting)
UINT8 uiAppDTCType=DTC_TYPE_ON;
UINT8 uiAppFailSafeMode;
//set by 0x28 msg
UINT8 uiCommRxTxType;

#endif


#ifdef GATEWAY_SUPPORT
UINT8 uaUDSGateWayPara[8];
UINT8 uaUDSGateWayRsp[8];
#endif
//For flow control ST time added on Sep 23, 2010
UINT8 uiSTtimeFL=0;
//extern Rv_bufferInfo Spi1RvBufferInfo;


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

#ifdef BOOT_MODE
extern UINT32 uiRxDLMemAddr;
#endif

///////////////////////////////////////////////////////////////
/////////////////TABLE DEFINATION//////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//Sub Function Available 
static const UINT8 uaSID10_SubFuncTbl[]={//size is 6
	0x01,//Default Session [$01/81]
	0x81,//Default Session [$01/81]
	0x02,//Programming Session [$02/82]
	0x82,//Programming Session [$02/82]
	0x03,//Extended Diagnostic Session [$03/83]
	0x83//Extended Diagnostic Session [$03/83]
//,00x49,0xC9  Stand By Session [$49/C9]
};
static const UINT8 uaSID11_SubFuncTbl[]={//size is 2
	0x01,//Hard Reset [$01/81]
	0x81//Hard Reset [$01/81]
//,0x03, //Soft Reset [$03/83]
//0x83	//Soft Reset [$03/83]
};

#ifdef BOOT_MODE
static const UINT8 uaSID27_SubFuncTbl[]={//size is 2
	0x05,0x06
};
static const UINT8 uaSID2E_SubFuncTbl[][2]={//size is 1
	{0xF1,0x5A}//Write Software Fingerprint
};

static const UINT8 uaSID31_SubFuncTbl[][3]={//size is 6
	{0x01,0xFF,0x00},//Erase Memory
	{0x81,0xFF,0x00},//Erase Memory
	{0x01,0xFF,0x01},//Check routine with additional CRC and without additional signature
	{0x81,0xFF,0x01},//Check routine with additional CRC and without additional signature
	{0x01,0xFF,0x03},//Check Programming Preconditions
	{0x81,0xFF,0x03}//Check Programming Preconditions
};
#else

static const UINT8 uaSID19_SubFuncTbl[]={//size is 8
	0x01,//Report Number Of DTC By Status Mask [$01]
	0x02,//Report DTC By Status Mask [$02]
	0x06,//Report DTC Extended Data Record By DTC Number [$06]
	0x0A,//Report Supported DTCs [$0A]
	0x0F,//Report Mirror Memory DTC By Status Mask [$0F]
	0x10,//Report Mirror Memory DTC Extended Data By DTC Number [$10]
	0x11,//Report Number Of Mirror Memory DTC By Status Mask [$11]
	0x14//Report DTC Fault Detection Counter [$14]
};
static const UINT8 uaSID28_SubFuncTbl[][2]={//size is 4
	{0x01,0x01},
	{0x81,0x01},
	{0x00,0x01},
	{0x80,0x01}
};
static const UINT8 uaSID85_SubFuncTbl[]={//size is 4
	0x01,0x81,0x02,0x82
};

#define WRITE_DID_TABLE_SIZE	15

static const UINT8 uaSID2E_SubFuncTbl[][2]={//size is 15
	{0x01,0x03},//VIN Odometer (Counter)  
	{0x01,0x04},// Odometer	
	{0x01,0x07},//Response on Event-Light Activation State
	{0xF1,0x32},//P/N?    READ_ECU_PART_NO
	{0xF1,0x5A},//Write Software Fingerprint
	{0xF1,0xA0},//VIN Current?
	{0xA6,0x01},//HW ID
	{0x24,0x11},//SOC
	{0xA0,0x04},//Diagnostic contactor Dis/Enabler(Engineering -Manufacturing)
	{0xA0,0x06},//MEC Counter 
	{0xA0,0x24},//EOL SCRATCH PAD
	{0xA0,0x29},//HIDDEN SOC
	{0xA0,0x30},//CAN LABLE 
	{0xA0,0x31},//Force to write
	{0xA0,0x40}//Write DTC enabler
	
};

#define ROUTINE_CONTROL_TABLE_SIZE	13
	
static const UINT8 uaSID31_SubFuncTbl[][3]={//size is 13
	{0x01,0x02,0x12},//Reset VIN Values
	{0x81,0x02,0x12},//Reset VIN Values
//NA	{0x01,0x02,0x13},//Set Fault Status
//NA	{0x81,0x02,0x13},//Set Fault Status
//NA	{0x01,0x02,0x14},//Request Echo Routine
//NA	{0x81,0x02,0x14},//Request Echo Routine
//NA	{0x01,0x02,0x15},//Control Network Mirroring Mode Start Routine
//NA	{0x81,0x02,0x15},//Control Network Mirroring Mode Start Routine
//NA	{0x02,0x02,0x15},//Control Network Mirroring Mode Stop Routine
//NA	{0x82,0x02,0x15},//Control Network Mirroring Mode Stop Routine
//NA	{0x03,0x02,0x15},//Control Network Mirroring Mode Return Results
//for boot	{0x01,0xFF,0x00},//Erase Memory
//for boot	{0x01,0xFF,0x01},//Check routine with additional CRC and without additional signature
//NA	{0x01,0xFF,0x02},//Erase Mirror Memory DTCs
//NA	{0x81,0xFF,0x02},//Erase Mirror Memory DTCs
	{0x01,0xFF,0x03},//Check Programming Preconditions
	{0x81,0xFF,0x03},//Check Programming Preconditions
	{0x01,0xFF,0x05},//DIS_FAIL_SAFE_REACTION
	{0x81,0xFF,0x05},//DIS_FAIL_SAFE_REACTION
	{0x02,0xFF,0x05},//ENB_FAIL_SAFE_REACTION
	{0x82,0xFF,0x05},//ENB_FAIL_SAFE_REACTION
	{0x01,0x03,0x06},//Hybrid Battery Fault Reset(active discharge) 
	{0x01,0x03,0x07},//Hybrid Battery Fault Reset (welded contactors)
	{0x01,0x03,0x08},//Hybrid Battery Fault Reset (Isolation run)
	{0x01,0x03,0x09},//Hybrid Battery Fault Reset (open cable)
	{0x01,0x03,0x10}//Hybrid Battery force balance
};

#endif

static const UINT8 uaSID3E_SubFuncTbl[]={//size is 3
	0x00,0x80,0x02
};

//For 0x22 read id
static const UINT8 uaReadIDChkTbl[READ_DATA_BY_ID_MAX][2] = {//size is 2
	{0x01, 0x00},//Repr cnt            READ_PROG_ATTEMPT_COUNTER
	{0x01, 0x03},//VIN Odometer (Counter)  
	{0x01, 0x04},// Odometer 	
	{0x01, 0x07},//Response on Event-Light Activation State
	{0xF1, 0x00},//Active Diagnostic Info  READ_ACTIVE_DIAG_INFO
	{0xF1, 0x0A},//ECU Origin
	{0xF1, 0x0B},//ECU Configuration
	{0xF1, 0x0D},//Spec Info    
	{0xF1, 0x12}, //H/W?   READ_HARD_PART_NO
	{0xF1, 0x22},//S/W?   READ_SOFT_PART_NO
	{0xF1, 0x32},//P/N?    READ_ECU_PART_NO
	{0xF1, 0x50},//H/W ver   READ_HARD_VER_INFO
	{0xF1, 0x51},//S/W ver    READ_SOFT_VER_INFO
	{0xF1, 0x53},//Boot S/W ver  READ_BOOT_VER_INFO
	{0xF1, 0x54},//H/W Supp   READ_HARD_SUPP_ID
	{0xF1, 0x55},//S/W Supp    READ_SOFT_SUPP_ID  
	{0xF1, 0x58},//Vehicle Information
	{0xF1, 0x5B},//Fingerprint    READ_FINGERPRINT
	{0xF1, 0x60},//Software Module Information
	{0xF1, 0x61},//Software Module Information - Channel 1
	{0xF1, 0x70},//Physical Layer Channel Configuration
	{0xF1, 0x71},//Physical Layer Channel Configuration - Channel 1
	{0xF1, 0x8C},//ECU Serial Number?
	{0xA6, 0x01},//READ_HW_DEVICE_NO
	{0xF1, 0x90},//VIN Original?
	{0xF1, 0xA0},//VIN Current?
		{0xF2,0xF2}	//test
};

static const UINT8 uaDID_ChannelConfig[]={//size is 20
	0x01,//Number of Channels   byte3
	//0 unknown , MAX13041 isn't in the table
	0x00,//Transceiver Type - Last Channel  (MAX13041)  byte4
	//0 unknown 
	0x00,//Common Mode Choke Type - Last Channel   byte5
	//0 unknown 
	0x75,//Terminating Resistor Value - Last Channel  byte6
	0x30,//Terminating Resistor Value - Last Channel byte7
	//C1CFG1
	0x00,//Baud Rate Register 0 - Last Channel  byte8
	0xc0,//Baud Rate Register 0 - Last Channel  byte9
	//C1CFG2
	0x05,//Baud Rate Register 1 - Last Channel  byte10
	0xfc,//Baud Rate Register 1 - Last Channel  byte11
	//500khz
	0x01,//Channel Oscillator Frequency - Last Channel  byte12
	0xf4,//Channel Oscillator Frequency - Last Channel  byte13
	//01  Crystal
	0x01,//Channel Oscillator Type - Last Channel  byte14
	//01 CAN
	0x01,//Channel Type - Last Channel  byte15
	//0 unknown , Microchip PIC33F isn't in the table
	0x00,//Micro Controller Family Type - Last Channel   byte16
	//0 unknown			
	0x00,//Physical Layer Controller Type - Last Channel  byte17
	0x07,//Network ID - Last Channel  byte18
	0xD6,//Reserved - Last Channel  byte19
	0x20,//Reserved - Last Channel  byte20
	0x20,//Reserved - Last Channel  byte21
	0x20//Reserved - Last Channel  byte22
};


//13.42.1 #ifdef VEH_TYPE_DS
//DS:        68070820AA 
#if 1
static const UINT8 uaDID_HardPartNo[]={//size is 10
	'6',//part number
	'8',
	'0',
	'7',
	'0',
	'8',
	'2',
	'0',
	'A',
	'K'
};
#else//RT:        05186015AA 
static const UINT8 uaDID_HardPartNo[]={//size is 10
	'0',//part number
	'5',
	'1',
	'8',
	'6',
	'0',
	'2',
	'0',
	'A',
	'D'
};
#endif
static const UINT8 uaDID_VehicleINFO[]={//size is 7
	12,	//Model Year?
	0x00,//Vehicle Line
	0x00,//Vehicle Line
	0x00,//Body Style DS-PI
	0x00,//Body Style
	0x00,//Country Code
	0x00//Country Code
};
#ifdef BOOT_MODE

static const UINT8 uaDID_BootVersion[]={//size is 3
	//Boot SW - year  HexDump (1Byte)
	BOOT_VERSION_YEAR,	//Dec  year 2010
	//Boot SW - year  HexDump (1Byte)
	BOOT_VERSION_WEEK,	//Dec week  21 weeks
	//Boot SW - patch level HexDump (1Byte)
	BOOT_VERSION_PATCH//hex
};
#else
static const UINT8 uaDID_BootVersion[]={//size is 3
	//Boot SW - year  HexDump (1Byte)
	BOOT_VERSION_YEAR,	//Dec  year 2010
	//Boot SW - year  HexDump (1Byte)
	BOOT_VERSION_WEEK,	//Dec week  21 weeks
	//Boot SW - patch level HexDump (1Byte)
	BOOT_VERSION_PATCH//hex
};

static const UINT8 uaDID_AppliVersion[]={//size is 3
	//SW - year HexDump (1Byte)
	SOFT_VERSION_YEAR,	//Dec  year 2010
	//SW - week Week (1Byte)
	SOFT_VERSION_WEEK,	//Dec week  21 weeks
	//SW - patch level HexDump (1Byte)
	SOFT_VERSION_PATCH//hex
};


#endif
static const UINT8 uaDID_HardVersion[]={//size is 3
	//HW - year HexDump (1Byte)
	HARD_VERSION_YEAR,	//Dec  year 2010
	//HW  - week Week (1Byte)
	HARD_VERSION_WEEK,	//Dec week  21 weeks
	//HW  - patch level HexDump (1Byte)
	HARD_VERSION_PATCH//hex
};

/*
static const UINT8 uaDID_ECUSerialNo[]={//size is 19
	'E',
	'V',	
	'-',
	'C',
	'Y',
	'-',
	'4',
	'0',
	'0',
	'-',
	'D',
	'S',
	'd',
	'e',
	'v',
	'-',
	'0',
	'0',
	'3'
};
*/
static const UINT8 uaDID_ECUSerialNo[]={//size is 14
	'T',//Traceability Dat
	'E',//Energy Storage 	
	'S',//Energy Storage 
	' ',//space
	' ',//space
	0x32,//July 30 day of the year
	0x31,//July 30 day of the year
	0x31,//July 30 day of the year
	0x30,//2010 year
	0x30,
	0x30,
	0x30,
	0x30,
	0x31,
};

/***************************************************/
/*******	FUNCTIONS DEFINATION SECTION            	****/
/***************************************************/
/*******	INNER FUNCTION DEFINATION PART      	****/
void nvSendUDSResponse(UINT8 uiSIType,UINT8 uiRspType,UINT8 * ptDataAddr, UINT16 uiDataSize);

/*******	USING EXTERNAL FUNCTION PART     ****/
#ifdef BOOT_MODE
extern void WriteLatch(UWord16, UWord16,UWord16,UWord16);
extern void ResetDevice(void);
#else
extern void reset(void);
#endif
extern void uvStoreToBuffer(ECAN_MSG_BUF * ptEcanMsg);

/***************************************************/
/*******	FUNCTIONS BODY SECTION                	****/
/***************************************************/
UINT8 uiAppliValidCheck(void)
{
	if(ProgInfo.uiApplicationInfo== 0x01)
		return TRUE;
	else return FALSE;

}

void uvSetMode(UINT8 uiModeType, UINT8 uiModeValue)
{
	switch(uiModeType){
		case DATA_RV_MODE:
			UDS_ModeFlags.uiDataRvMode=uiModeValue;
			break;
#ifdef BOOT_MODE
		case PROGRAMMING_MODE:
			UDS_ModeFlags.uiProgrammingMode=uiModeValue;
			break;
#endif
		case DATA_STORE_MODE:
			UDS_ModeFlags.uiWritingMode=uiModeValue;
			break;
		case GATEWAY_MODE:
			UDS_ModeFlags.uiGateWayMode=uiModeValue;
			break;
#ifndef BOOT_MODE
		case DIAG_MODE:
			UDS_ModeFlags.uiDiagMode=uiModeValue;
			break;
#endif
		case RSP_MODE:
			UDS_ModeFlags.uiRSPMode=uiModeValue;
			break;
		default:
			break;
	}
}

UINT8 uiGetMode(UINT8 uiModeType)
{
	UINT8 uiModeValue;
	switch(uiModeType){
		case DATA_RV_MODE:
			uiModeValue =UDS_ModeFlags.uiDataRvMode;
			break;
#ifdef BOOT_MODE
		case PROGRAMMING_MODE:
			uiModeValue =UDS_ModeFlags.uiProgrammingMode;
			break;
#endif
		case DATA_STORE_MODE:
			uiModeValue =UDS_ModeFlags.uiWritingMode;
			break;
		case GATEWAY_MODE:
			uiModeValue =UDS_ModeFlags.uiGateWayMode;
			break;
#ifndef BOOT_MODE
		case DIAG_MODE:
			if(uiGetAppDiagSessionMode()==EXTENDED_DIAG_SESSION){
				UDS_ModeFlags.uiDiagMode=1;
			}
			else UDS_ModeFlags.uiDiagMode=0;
			uiModeValue =UDS_ModeFlags.uiDiagMode;
			break;
#endif
		case RSP_MODE:
			uiModeValue=UDS_ModeFlags.uiRSPMode;
			break;
		default:
			return FALSE;
			break;
	}
	return uiModeValue;
}

void uvCopyBuffer(UINT8 * pCopyTo,UINT8 * pCopyFrom, UINT16 uiSize){
	UINT16 i;

	for(i=0;i<uiSize;i++){
		*pCopyTo=*pCopyFrom;
		pCopyTo++;
		pCopyFrom++;
	}
}

UINT8 uiGetCheckSum(UINT8 *pBuffer, UINT8 uiSize){
	UINT8 *pTemp;
	UINT8 uiResult;
	UINT8 i;

	pTemp=pBuffer;
	for(i=0;i<uiSize;i++){
		if(i==0){
			uiResult=*pTemp;
		}
		else{
			uiResult^=*pTemp;
		}
		pTemp++;
	}

	uiResult+=1;
	return uiResult;
}

#ifdef STORE_DATA_TO_EXTERNAL_FLASH
void uvWriteUDSParaToExFlash(void)
//void uvWriteUDSParaToExFlash(UINT8 uiAddrOffset, UINT8 uiSize)
{
//write all data 

	flashWritePara.UDSInfoWrReq=EEPROM_WRITE_FLAG;

}
void uvWriteProgParaToExFlash(void)
//void uvWriteProgParaToExFlash(UINT8 uiAddrOffset, UINT8 uiSize)
{

	flashWritePara.ProgInfoWrReq=EEPROM_WRITE_FLAG;
}


void uvClearUDSParaToExFlash(UINT8 uiAddrOffset, UINT8 uiSize)
{
//write all data 

	flashWritePara.UDSInfoWrReq=EEPROM_ERASE_FLAG;

}
void uvClearProgParaToExFlash(UINT8 uiAddrOffset, UINT8 uiSize)
{

	flashWritePara.ProgInfoWrReq=EEPROM_ERASE_FLAG;
}

#endif

UINT8 uiCheckReadIDTbl(void)
{
	UINT8 i,j;
	
	for(i=0;i<READ_DATA_BY_ID_MAX;i++){
		if(UDSRvDataPara.uiSUB[0]==uaReadIDChkTbl[i][0]){
				if(UDSRvDataPara.uiSUB[1]==uaReadIDChkTbl[i][1]){
					return i;
				}
		}
	}
	return 0xff;
}


UINT8 uiGetReadID(void)
{
	UINT8 uiReadID=0xff;
	UINT8 i,j;
	UINT8 uiSoftMode;
	UINT8 uiGatewayMode;
	UINT8 uiActiveDiagVariant;
	UINT8 uiActiveDiagVersion;
	UINT8 uiSoftMismatch;
	UINT8 uiHardMismatch;
	

	for(j=0;j<30;j++){
		uaBuffer[j]=0;
	}
	uiSendSize=0;
	uiReadID=uiCheckReadIDTbl();
	switch(uiReadID){
		case READ_PROG_ATTEMPT_COUNTER:
			//read programming Attempt Counter
			//read from eeprom
			uiSendSize=4;
			UDSWriteInfo.uiProgMaxNo_L=0xff;
			UDSWriteInfo.uiProgMaxNo_H=0xff;
			uvCopyBuffer(&uaBuffer[0], &UDSWriteInfo.uiProgAtteNo_H,uiSendSize);
			break;
		case READ_VIN_ODOMETER:
			uiSendSize=1;
			//eeprom_ram_para.ODO_BPCM
			uaBuffer[0]=UDSWriteInfo.uiVinOdometer;
			break;
		case READ_ODOMETER:
			uiSendSize=3;
			//vehicleInfo.distance_lifetime
		//	UDSWriteInfo.uaOdometer[0]=((UINT32)eeprom_ram_para.vehicalDriveDistance>>16 )& 0xFF;
		//	UDSWriteInfo.uaOdometer[1]=((UINT32)eeprom_ram_para.vehicalDriveDistance >>8)& 0xFF;
		//	UDSWriteInfo.uaOdometer[2]=(UINT32)eeprom_ram_para.vehicalDriveDistance& 0xFF;
			uvCopyBuffer(&uaBuffer[0], &UDSWriteInfo.uaOdometer[0],uiSendSize);
			break;
		case READ_RESPONSE_ON_EVENT_LIGHT_ACTIVATION_STATE:
			uiSendSize=1;
			//deactivated ---$00
			//activated ---$01
			uaBuffer[0]=UDSWriteInfo.uiRspOnELActState;
			break;
		case READ_ACTIVE_DIAG_INFO:
			//read active diagnostic information
			//read from flash
#ifdef BOOT_MODE
			//this as release version
			//Bit 0-- 3 [0] 每 Running Mode (1Bit)
			//Running in Application ----$0,Running in Boot  3 [0]  ----$1
			uiSoftMode=1;
			//Bit 1 -- 3 [1] 每  Gateway Functionality (1Bit)
			//False ----$0, True ----$1
			uiGatewayMode=0;
			//test for Euthie
			//Active Diagnostic Variant每   Unsigned (1Byte)
			uiActiveDiagVariant=0x02;
			//Active Diagnostic Version每   Unsigned (1Byte)
			uiActiveDiagVersion=0x00;
			uaBuffer[0]=(uiGatewayMode<<1)|uiSoftMode;
			uaBuffer[1]=uiActiveDiagVariant;
			uaBuffer[2]=uiActiveDiagVersion;
			//Bit 0-6 --6 [0]-[6] Active Diagnostic Session- Session Type (7Bit)
			//Default----$1, Programming----$2, Extended----$3
			uaBuffer[3]=UDSWriteInfo.uiSessionMode;
#else
			//Bit 0 每 ECU Software Mode 
			//Running in Application 3 [0] ----$0
			//Running in Boot  3 [0]  ----$1
			uiSoftMode=0;
			//Bit 1 -- 3 [1] 每  Gateway Functionality (1Bit)
			//False ----$0
			//True ----$1
			uiGatewayMode=0;
			//Active Diagnostic Variant每   Unsigned (1Byte)
			uiActiveDiagVariant=0x02;
			//Active Diagnostic Version每   Unsigned (1Byte)
	//		uiActiveDiagVersion=0x03;//Modified by Amanda on Oct 14 2010
			uiActiveDiagVersion=0x04;//Modified by Amanda on Feb 15,2011
			
			uaBuffer[0]=(uiGatewayMode<<1)|uiSoftMode;
			uaBuffer[1]=uiActiveDiagVariant;
			uaBuffer[2]=uiActiveDiagVersion;
			//Bit 0-6 --6 [0]-[6] Active Diagnostic Session- Session Type (7Bit)
			//Default----$1, Programming----$2, Extended----$3
			uaBuffer[3]=UDSWriteInfo.uiSessionMode;
#endif
			uiSendSize=4;
			break;
		case READ_ECU_ORIGIN:
			//ECU Origin (1Byte)
			uaBuffer[0]=0;//table 3701
			uiSendSize=1;
			break;
		case READ_ECU_CONFIGURATION:
			uaBuffer[0]=0;//no useful
			uiSendSize=1;
			break;
		case READ_SPEC_INFO:
			//DCX Unified Diagnostic Services - Diagnostic Protocol Version 
			//New CS-11729 Rev. _ --> 07
			uaBuffer[0]=0x07;
			//DCX ECU Flash Reprogramming Requirements Definition Version
			//New CS-11825 Rev. _ -->67
			uaBuffer[1]=0x67;
			//DCX Diagnostic Performance Requirements Standard
			//New CS-11736 Rev. _ -->B8
			uaBuffer[2]=0xB8;
			uiSendSize=3;
			break;
		case READ_HARD_PART_NO:
			//read from eeprom/boot
			//Chrysler Group Hardware Part Number 10bytes ASCII
			//If part number is less than 10 characters return space [20 hex] for the unused bytes.
			uiSendSize=10;
			uvCopyBuffer(&uaBuffer[0], (UINT8 *)&uaDID_HardPartNo[0],uiSendSize);
			break;
		case READ_SOFT_PART_NO:
			uiSendSize=10;
#ifdef BOOT_MODE
//			for(i=0;i<uiSendSize;i++){
//				uaBuffer[i]=0xff;
//			}
			//set same as hardware partnumber
			uvCopyBuffer(&uaBuffer[0], (UINT8 *)&uaDID_HardPartNo[0],uiSendSize);
#else
			//read from  flash
			//Chrysler Group Software Part Number 10bytes ASCII
			//set same as hardware partnumber
			uvCopyBuffer(&uaBuffer[0], (UINT8 *)&uaDID_HardPartNo[0],uiSendSize);
#endif
			break;
		case READ_ECU_PART_NO:
			//read from  flash
			//ECU Part Number 10bytes ASCII
			uiSendSize=10;
			uvCopyBuffer(&uaBuffer[0], &UDSWriteInfo.uaEcuPartNumber[0],uiSendSize);
			break;
		case READ_HARD_VER_INFO:
			//read hardware version information
			//read from eeprom/boot
			uiSendSize=3;
			uvCopyBuffer(&uaBuffer[0], &UDSWriteInfo.uiHardVer_YEAR,uiSendSize);
			break;
		case READ_SOFT_VER_INFO:
			//read software version information
			//read from  flash
			uiSendSize=3;
			uvCopyBuffer(&uaBuffer[0], &UDSWriteInfo.uiAppSoftVer_YEAR,uiSendSize);
			break;
		case READ_BOOT_VER_INFO:
			//read boot software version information
			//read from eeprom/boot
			uiSendSize=3;
			uvCopyBuffer(&uaBuffer[0], &UDSWriteInfo.uiBootSoftVer_YEAR,uiSendSize);
			break;
		case READ_HARD_SUPP_ID:
			//read hardware supplier identification
			//read from eeprom
			//Hardware Supplier Identification(2Byte)			
			uiSendSize=2;
			uvCopyBuffer(&uaBuffer[0], &UDSWriteInfo.uiHDSupplierID_H,uiSendSize);
			break;
		case READ_SOFT_SUPP_ID:
			//read software supplier identification
			//read from  flash
			//Software Logical Block Supplier Identification(2Byte)			
			uaBuffer[0]=0x00;
			uaBuffer[1]=0xD0;
			uiSendSize=2;
			break;
		case READ_VEHICLE_INFORMATION:
			uiSendSize=7;
#ifdef BOOT_MODE
			for(i=0;i<uiSendSize;i++){
				uaBuffer[i]=0xff;
			}
#else
#endif
			uvCopyBuffer(&uaBuffer[0], (UINT8 *)&uaDID_VehicleINFO[0],uiSendSize);
			break;
		case READ_FINGERPRINT:
			//read fingerprint 
			//read from eeprom
			uiSendSize=10;
#ifdef BOOT_MODE
			for(i=0;i<uiSendSize;i++){
				uaBuffer[i]=0xff;
			}
#else
			//Software Programmed and Valid ->3 [0] 1bit   :ProgInfo.uiApplicationInfo
			//Software - Software Mismatch->3 [1] 1bit
			uiSoftMismatch=0;
			//Software - Hardware Mismatch->3 [2] 1bit
			uiHardMismatch=0;
			uaBuffer[0]=((uiHardMismatch<<2)|(uiSoftMismatch<<1)|ProgInfo.uiApplicationInfo);
			//Supplier Identification  2bytes
			//SW Programming - year  1byte			
			//SW Programming - month  1byte			
			//SW Programming - day 1byte			
			//Diagnostic Tool Serial Number 4bytes
			uvCopyBuffer(&uaBuffer[1], &UDSWriteInfo.uiFingerprint.uiSupplierID_H,uiSendSize-1);
#endif
			break;
		case READ_SOFTWARE_MODULE_INFORMATION:
			uiSendSize=20;
			for(i=0;i<uiSendSize;i++){
				uaBuffer[i]=0x00;
			}
		     uaBuffer[5]=10;
     			uaBuffer[6]=11;
		break;
		case READ_SOFTWARE_MODULE_INFORMATION_CHANNEL_1:
			uiSendSize=20;
			for(i=0;i<uiSendSize;i++){
				uaBuffer[i]=0x00;
			}
			uaBuffer[5]=10;
			uaBuffer[6]=11;
			break;
		case READ_PHYSICAL_LAYER_CHANNEL_CONFIGURATION:
			uiSendSize=20;
			uvCopyBuffer(&uaBuffer[0], (UINT8 *)&uaDID_ChannelConfig[0],uiSendSize);
			break;
		case READ_PHYSICAL_LAYER_CHANNEL_CONFIGURATION_CHANNEL_1:
			uiSendSize=20;
			uvCopyBuffer(&uaBuffer[0], (UINT8 *)&uaDID_ChannelConfig[0],uiSendSize);
			break;
		case READ_ECU_SERIAL_NO:
			//ECU Serial Number 12bytes ASCII
		//	uiSendSize=12;
		//	uiSendSize=19;
			uiSendSize=12;
	//		uvCopyBuffer(&uaBuffer[0], (UINT8 *)&uaDID_ECUSerialNo[0],uiSendSize);
			uvCopyBuffer(&uaBuffer[0], &UDSWriteInfo.uaECUSerialNo[0],uiSendSize);
			break;
		case READ_HW_DEVICE_NO:
			uiSendSize=1;
			uaBuffer[0]=0x00;
			if(UDSWriteInfo.uiMBHWVersion==2){
				uaBuffer[0]=0x01;			
			}
			break;
		case READ_VIN_ORIGINAL:
			uiSendSize=17;
			uvCopyBuffer(&uaBuffer[0], &UDSWriteInfo.uaVinORG[0],uiSendSize);
			break;
		case READ_VIN_CURRENT:
			uiSendSize=17;
			uvCopyBuffer(&uaBuffer[0], &UDSWriteInfo.uaVinCurrent[0],uiSendSize);
			break;
		default:
			break;
	}
	return uiReadID;
}


void uvInitUDSWriteInfo(void){
	UINT8 i;
	UINT8 * pData;

	pData=&UDSWriteInfo.uiFirstByte;
	for(i=0;i<UDSPara_SIZE;i++){
		*pData=0;
		pData++;
	}
	UDSWriteInfo.uiSessionMode=DEFAULT_DIAG_SESSION;
#ifdef BOOT_MODE
	uiBootSessionMode=DEFAULT_DIAG_SESSION;
#else
	uiAppliSessionMode=DEFAULT_DIAG_SESSION;
#endif
	//ECU Part Number 10bytes ASCII
	uvCopyBuffer(&UDSWriteInfo.uaEcuPartNumber[0],(UINT8 *)&uaDID_HardPartNo[0] ,10);
	for(i=0;i<17;i++){
		UDSWriteInfo.uaVinCurrent[i]=0x20;
		UDSWriteInfo.uaVinORG[i]=0x20;
	}
//added start by Amanda on Jun 03 2010
	UDSWriteInfo.uiProgAtteNo_H=0;
	UDSWriteInfo.uiProgAtteNo_L=0;

	UDSWriteInfo.uiProgMaxNo_H=0xff;
	UDSWriteInfo.uiProgMaxNo_L=0xff;
	UDSWriteInfo.uiHDSupplierID_H=0x00;
	UDSWriteInfo.uiHDSupplierID_L=0xD0;
#ifdef BOOT_MODE
	uvCopyBuffer(&UDSWriteInfo.uiBootSoftVer_YEAR,(UINT8 *)&uaDID_BootVersion[0] ,3);
#else
	uvCopyBuffer(&UDSWriteInfo.uiAppSoftVer_YEAR,(UINT8 *)&uaDID_AppliVersion[0] ,3);
//	UDSWriteInfo.uiAppSoftVer_PATCH=eeprom_ram_para.BPCM_Version[2]-'0';
#endif
	uvCopyBuffer(&UDSWriteInfo.uiHardVer_YEAR,(UINT8 *)&uaDID_HardVersion[0] ,3);
	uvCopyBuffer(&UDSWriteInfo.uaECUSerialNo[0],(UINT8 *)&uaDID_ECUSerialNo[0] ,12);
	UDSWriteInfo.uaECUSerialNo[9]=UDSWriteInfo.uiCANLable_PackNo_H/100+'0';
	UDSWriteInfo.uaECUSerialNo[10]=(UDSWriteInfo.uiCANLable_PackNo_H/10)%10+'0';
	UDSWriteInfo.uaECUSerialNo[11]=UDSWriteInfo.uiCANLable_PackNo_H%10+'0';
	

	UDSWriteInfo.uiContactorEnabler=0x03;//all enable
//Added by Amanda for DTC enabler
	for(i=0;i<22;i++){
		UDSWriteInfo.uiDTCGroupEnabler[i]=0xFF;//all enable
	}
#if 1//Added by Amanda August 5
	UDSWriteInfo.uiPassConterDTC=0;
#endif
//added end by Amanda on Jun 03 2010


}


UINT8 readUDSDataFromEEPROM(UINT16 uiAddr ){
	sUDSPara  * ptUDSReadInfo;
	UINT16 temp;
	UINT8 * pt;
	UINT8 val,val1;
	UINT16 len;
	UINT8 uiCheckSumCalc;

	pt=(UINT8 *)&UDSReadInfo;
	ptUDSReadInfo=&UDSReadInfo;
	ptUDSReadInfo++;
	len=(UINT16)ptUDSReadInfo - (UINT16)pt-1;

	uiCheckSumCalc=0;
	

	for(temp=0;temp<len;temp++){
		val=EepRead((UINT32)uiAddr+temp);
		*pt=val;
		uiCheckSumCalc^=val;
		pt++;
	}

	uiCheckSumCalc+=1;

	val1=EepRead((UINT32)uiAddr+len);
	
	//recovery process
	if(uiCheckSumCalc!=val1){
		return FALSE;
	}
	else{
		return TRUE;
	}
}

UINT8 readProgDataFromEEPROM(UINT16 uiAddr ){
	sProgPara  * ptProgReadInfo;
	UINT16 temp;
	UINT8 * pt;
	UINT8 val,val1;
	UINT16 len;
	UINT8 uiCheckSumCalc;

	pt=(UINT8 *)&ProgInfo;
	ptProgReadInfo=&ProgInfo;
	ptProgReadInfo++;
	len=(UINT16)ptProgReadInfo - (UINT16)pt-1;

	uiCheckSumCalc=0;
	

	for(temp=0;temp<len;temp++){
		val=EepRead((UINT32)uiAddr+temp);
		*pt=val;
		uiCheckSumCalc^=val;
		pt++;
	}

	uiCheckSumCalc+=1;

	val1=EepRead((UINT32)uiAddr+len);
	
	//recovery process
	if(uiCheckSumCalc!=val1){
		return FALSE;
	}
	else{
		return TRUE;
	}
}

void iniEEPROMParaUDS(void)
{
#if 0
	UINT8 i;
	UINT8 * pData;

	pData=&UDSWriteInfo.uiFirstByte;
	for(i=0;i<UDSPara_SIZE;i++){
		*pData=0xff;
		pData++;
	}
	uvClearUDSParaToExFlash(0,UDSPara_SIZE);
#else

	UINT8 i;
	UINT8 * pData;

	pData=&UDSWriteInfo.uiFirstByte;
	for(i=0;i<UDSPara_SIZE;i++){
		*pData=0;
		pData++;
	}
	UDSWriteInfo.uiSessionMode=DEFAULT_DIAG_SESSION;
#ifdef BOOT_MODE
	uiBootSessionMode=DEFAULT_DIAG_SESSION;
#else
	uiAppliSessionMode=DEFAULT_DIAG_SESSION;
#endif

	//ECU Part Number 10bytes ASCII
	uvCopyBuffer(&UDSWriteInfo.uaEcuPartNumber[0],(UINT8 *)&uaDID_HardPartNo[0] ,10);
	for(i=0;i<17;i++){
		UDSWriteInfo.uaVinCurrent[i]=0x20;
		UDSWriteInfo.uaVinORG[i]=0x20;
	}
//added start by Amanda on Jun 03 2010
	UDSWriteInfo.uiProgAtteNo_H=0;
	UDSWriteInfo.uiProgAtteNo_L=0;

	UDSWriteInfo.uiProgMaxNo_H=0xff;
	UDSWriteInfo.uiProgMaxNo_L=0xff;
	UDSWriteInfo.uiHDSupplierID_H=0x00;
	UDSWriteInfo.uiHDSupplierID_L=0xD0;
#ifdef BOOT_MODE
	uvCopyBuffer(&UDSWriteInfo.uiBootSoftVer_YEAR,(UINT8 *)&uaDID_BootVersion[0] ,3);
#else
	uvCopyBuffer(&UDSWriteInfo.uiAppSoftVer_YEAR,(UINT8 *)&uaDID_AppliVersion[0] ,3);
	UDSWriteInfo.uiAppSoftVer_PATCH=eeprom_ram_para.BPCM_Version[2]-'0';
#endif
	uvCopyBuffer(&UDSWriteInfo.uiHardVer_YEAR,(UINT8 *)&uaDID_HardVersion[0] ,3);
	uvCopyBuffer(&UDSWriteInfo.uaECUSerialNo[0],(UINT8 *)&uaDID_ECUSerialNo[0] ,12);
	UDSWriteInfo.uaECUSerialNo[9]=UDSWriteInfo.uiCANLable_PackNo_H/100+'0';
	UDSWriteInfo.uaECUSerialNo[10]=(UDSWriteInfo.uiCANLable_PackNo_H/10)%10+'0';
	UDSWriteInfo.uaECUSerialNo[11]=UDSWriteInfo.uiCANLable_PackNo_H%10+'0';

	UDSWriteInfo.uiContactorEnabler=0x03;//all enable
//Added by Amanda for DTC enabler
	for(i=0;i<22;i++){
		UDSWriteInfo.uiDTCGroupEnabler[i]=0xFF;//all enable
	}
#if 1//Added by Amanda August 5
	UDSWriteInfo.uiPassConterDTC=0;
#endif

#endif
}
void iniEEPROMParaProg(void)
{
	UINT8 i;
	UINT8 * pData;

	pData=&ProgInfo.uiRequestID;
	for(i=0;i<3;i++){
		*pData=0xff;
		pData++;
	}
	uvClearProgParaToExFlash(0,3);
}


UINT8 uvReadProgFlags(UINT8 uiReadType)
{
	UINT8 val1;
	UINT8 result;
	UINT16 uiAddr;

	if(uiReadType==0){
	
		val1=EepRead((UINT32)EEPROM_WRITE_BUFFER_FLAGS_START+EEPROM_WRITE_PROG_INFO-1);
		flashWritePara.ProgInfoWrBuff=val1;

		result=uvReadDataBuffer(EEPROM_WRITE_PROG_INFO,val1);
		return result;
	}
	else{

		val1=EepRead((UINT32)EEPROM_WRITE_BUFFER_FLAGS_START+EEPROM_WRITE_PROG_INFO-1);
		flashWritePara.ProgInfoWrBuff=val1;
		if(val1==0){
			uiAddr=EEPROM_PROG_INFO_LOCATION;
		}
		else if(val1==1){
			uiAddr=EEPROM_PROG_INFO_LOCATION_1;
		}
		else if(val1==2){
			uiAddr=EEPROM_PROG_INFO_LOCATION_2;
		}
		else{
			//write again
			uvWriteProgParaToExFlash();
			return FALSE;
		}
		result=readProgDataFromEEPROM(uiAddr);

		//recovery process
		if(result==FALSE){
			return FALSE;
		}
		else{
			return TRUE;
		}

	}


}

#ifdef BOOT_MODE
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
////////////////FOR BOOTLOADER UDS LAYER/////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void uvSetBootDiagSessionMode(UINT8 uiSessionMode)
{
	//	uiBootSessionMode=uiSessionMode+0x10;
	uiBootSessionMode=uiSessionMode;
	UDSWriteInfo.uiSessionMode=uiBootSessionMode;

}

//for external Calling
UINT8 uiGetBootDiagSessionMode(void)
{
	return uiBootSessionMode;
}

void uvSetSecurityMode(UINT8 uiSecurity)
{
	uiBootSecurityMode=uiSecurity;
}
UINT8 uiGetSecurityMode(void)
{
	return uiBootSecurityMode;
}
//Check Programming Preconditions
UINT8 uiChkProgPrecFromBoot(void)
{
	return 0;
}


void nvGenRandomSeed(UINT8 *pSeed)
{
	TICK RandomTick;
	uiProgCounter=0;

#ifdef CAN_FLASH
	*pSeed=0x01;
	*(pSeed+1)=0x02;
	*(pSeed+2)=0x03;
	*(pSeed+3)=0x04;
#else
	RandomTick=TickGet();
	
	*pSeed=RandomTick.byte.b0;
	*(pSeed+1)=RandomTick.byte.b1;
	*(pSeed+2)=RandomTick.byte.b2;
	*(pSeed+3)=RandomTick.byte.b3;

#endif
}

UINT32 niCalcKey(UINT32 uiSeed)
{
	UINT32 uiKey;
	UINT32 uiConstant1,uiConstant2;

	uiConstant1=0x4302FCDE;
	uiConstant2=0x80DACC3B;

	uiKey= ((uiSeed << 8) & 0xFF000000) + ((uiSeed >> 8) & 0x00FF0000) + ((uiSeed << 8) & 0x0000FF00) + ((uiSeed >> 8) & 0x000000FF);
	uiKey = ((uiKey << 11) + (uiKey >> 22)) & 0xFFFFFFFF;
	uiConstant2 = (uiConstant2 & uiSeed) & 0xFFFFFFFF;
	uiKey=(uiKey ^ uiConstant1 ^ uiConstant2) & 0xFFFFFFFF;
	return uiKey;
}


void nvGenKey(UINT8 *pKey,UINT8 *pSeed)
{

	UINT32 uiCalcKey,uiGenSeed;
#ifdef CAN_FLASH
	*pKey=0x8A;
	*(pKey+1)=0xFE;
	*(pKey+2)=0x00;
	*(pKey+3)=0x00;
#else
	uiGenSeed=((UINT32)*pSeed<<24)+((UINT32)*(pSeed+1)<<16)+((UINT32)*(pSeed+2)<<8)+*(pSeed+3);

	uiCalcKey=niCalcKey(uiGenSeed);
	
	*pKey=(uiCalcKey>>24)&0xff;
	*(pKey+1)=(uiCalcKey>>16)&0xff;
	*(pKey+2)=(uiCalcKey>>8)&0xff;
	*(pKey+3)=uiCalcKey&0xff;
#endif
}
UINT8 uiCompareKey(UINT8 *pRvKey,UINT8 *pInnerKey, UINT8 uiSize )
{
	UINT8 uiCounter;
	UINT8 uiSameCounter=0;
	
	for (uiCounter=0;uiCounter<uiSize;uiCounter++){
		if (*pRvKey==*pInnerKey){
			uiSameCounter++;
		}
		pRvKey++;
		pInnerKey++;
	}
	if(uiSameCounter==uiSize) 
		return TRUE;
	else 
		return FALSE;
}

//Check software integrity after programming
UINT8 uiCompareCRC(void)
{
	UINT32 uiCheckSum;
	UINT16 uiCRCLen, uiCounter;
	uiCRCLen=0;
	uiCheckSum=0;

	uiCRCLen=UDSRvDataPara.uiData[0]<<8;
	uiCRCLen|=UDSRvDataPara.uiData[1];
	uiCheckSum|=UDSRvDataPara.uiData[2];
		
	for(uiCounter=1;uiCounter<uiCRCLen;uiCounter++){
		uiCheckSum<<=8;
		uiCheckSum|=UDSRvDataPara.uiData[uiCounter+2];
	}
	uvCalcCRC(&uiCalcCRC);
	if(uiCheckSum==uiCalcCRC){
		uvCrc32Init(&uiCalcCRC);	
		return TRUE;
	}
	else{
		uvCrc32Init(&uiCalcCRC);	
		return FALSE;

	}		
}

void uvWriteIVT(void)
{
	UINT32 SourceAddr;
	register unsigned int i;
 	int current_cpu_iplSave;

	//get  the IVT of application
	for(i=8;i<512;i++){
		uaBootMemoryBackup[i]=uaProgBufferIVT[i];
	}
	//prepare buffers
	for (i=0;i<2048;i++){
		uaProgBufferBytes[i]=  uaBootMemoryBackup[i];
	}
	SourceAddr=0;
	SET_AND_SAVE_CPU_IPL(current_cpu_iplSave, 7);		//disable all interruptions
//	SRbits.IPL=7;/* Set CPU IPL to 7, disable level 1-7 interrupts */
	StartProgramming(&uaProgBufferBytes[0], SourceAddr);
	RESTORE_CPU_IPL(current_cpu_iplSave);		//enable interruptions	
//	SRbits.IPL=0;/* Set CPU IPL to 0, enable level 1-7 interrupts */
}

//judge the application's CRC (CRC32)
UINT8 uiCheckRoutine(void)
{
	uiSoftValid=uiCompareCRC();
	if(uiSoftValid){
		uvWriteIVT();
	}
	ProgInfo.uiApplicationInfo=uiSoftValid;
#ifdef STORE_DATA_TO_EXTERNAL_FLASH
	uvWriteProgParaToExFlash();
#endif
	return uiSoftValid;
}


UINT8 uiEraseRoutine(UINT8 uiBlockNo)
{
	UINT16 uiProgMax;
	//need to write 4 bytes
#ifdef STORE_DATA_TO_EXTERNAL_FLASH
	uiAttemptCounter=(UDSWriteInfo.uiProgAtteNo_H<<8)|UDSWriteInfo.uiProgAtteNo_L;
#else
#endif
	uiProgMax=(UDSWriteInfo.uiProgMaxNo_H<<8)|UDSWriteInfo.uiProgMaxNo_L;
	if(uiAttemptCounter>=uiProgMax){
		return FALSE;
	}

	uiAttemptCounter++;
	UDSWriteInfo.uiProgAtteNo_H=(uiAttemptCounter>>8)&0xff;
	UDSWriteInfo.uiProgAtteNo_L=uiAttemptCounter&0xff;

	uiSoftValid=0;
	ProgInfo.uiApplicationInfo=uiSoftValid;
#ifdef STORE_DATA_TO_EXTERNAL_FLASH
	uvWriteProgParaToExFlash();
	uvWriteUDSParaToExFlash();
#endif
	//erase memory
	EraseProgByBlockNo(uiBlockNo);
	return TRUE;
}

UINT8 uiEraseBlock(void)
{
	if(uiEraseRoutine(uiFingerprint.uiLogBlock)){
		return TRUE;
	}
	else{
		return FALSE;
	}	
}

UINT8 uiCheckAppliMemory(void)
{
 	register unsigned int uiCounter;
	uReg32 temp1;

	for(uiCounter=0;uiCounter<MAX_NUM_OF_BLOCK_FOR_PROG;uiCounter++){
		uaBootMemoryBackup[uiCounter]=0;
	}
#ifdef BATT_MODE
	temp1.Val32=BATTAPP_CODE_START_ADDR;
#else
	temp1.Val32=APPLICATION_CODE_START_ADDR;
#endif
	ReadPM(&uaBootMemoryBackup[0],temp1);
	if(((uaBootMemoryBackup[0]==0)&&(uaBootMemoryBackup[1]==0)&&(uaBootMemoryBackup[2]==0))||
		((uaBootMemoryBackup[0]==0xff)&&(uaBootMemoryBackup[1]==0xff)&&(uaBootMemoryBackup[2]==0xff)))
		{
		return FALSE;
	}
	else {
		return TRUE;
	}
}

void uvInitBeforeDownload(void)
{
	register unsigned int uiCounter;
	
	//init buffer for programming
	for(uiCounter=0;uiCounter<MAX_NUM_OF_BLOCK_FOR_PROG;uiCounter++){
		uaProgBufferBytes[uiCounter]=0x00;
	}
	uiBlockSeqCounter=0x00;
	uiErrorCounter=0x00;
	uiRvDataByteSum=0x00;
}

//timer function
#if (REAL_ENV)
UINT8 niTimerOutOfSecurity(void)
{
	if(uiGetSecurityMode()==SECURITY_SEED_REQ){
		uvSetSecurityMode(SECURITY_MODE_OFF);
		nvSendUDSNegRes(SECURITY_ACCESS,NRTC_REQTIME_DELAY_NOEXPIRED);
	}
}
#endif
UINT8 uvBootReadAllUDSFlags(void)
{
#ifdef STORE_DATA_TO_EXTERNAL_FLASH

	UINT8 val1;
	UINT8 result;
	
	val1=EepRead((UINT32)EEPROM_WRITE_BUFFER_FLAGS_START+EEPROM_WRITE_UDS_INFO-1);
	flashWritePara.UDSInfoWrBuff=val1;

	result=uvReadDataBuffer(EEPROM_WRITE_UDS_INFO,val1);
	return result;
#endif
}


UINT8 nuBootGetReadID(void)
{
	return uiGetReadID();
}


void uvProcessData(void)
{

	register unsigned int uiCopyCounter;
	UINT16	i;
	UINT32 uiStartAddr, uiEndAddr;

#ifdef BATT_MODE
	uiStartAddr=BATTAPP_CODE_START_ADDR;
	uiEndAddr=BATTAPP_CODE_END_ADDR;
#else
	uiStartAddr=APPLICATION_CODE_START_ADDR;
	uiEndAddr=APPLICATION_CODE_END_ADDR;
#endif

	uiErrorCounter=0;
	UDS_ModeFlags.uiProgrammingMode=PROGRMMING_ON_GOING;
	//byte3~n transfer request parameter record
	for(uiCopyCounter=0;uiCopyCounter<MAX_NUM_OF_BLOCK_FOR_PROG;uiCopyCounter++){ 
		uaProgBufferBytes[uiCopyCounter]=0;
	}
	for(uiCopyCounter=0;uiCopyCounter<(UDSRvDataPara.uiDataLen-2);uiCopyCounter++){ 
		uaProgBufferBytes[uiCopyCounter]=UDSRvDataPara.uiData[uiCopyCounter];
		uvUpdateCRC(&uiCalcCRC, UDSRvDataPara.uiData[uiCopyCounter]);
	}
//	if(uiRvDataByteSum==MAX_NUM_OF_BLOCK_FOR_PROG-1){
//	if(uiRvDataByteSum==UDSRvDataPara.uiDataLen-2){
//		if(uiFingerprint.uiLogBlock==0){
		if((uiRxDLMemAddr/2)<uiStartAddr){
			for (i=0;i<(UDSRvDataPara.uiDataLen-2);i++){
				uaProgBufferIVT[i]=uaProgBufferBytes[i];
			}
				nvSendUDSResponse(TRANSFER_DATA, POS_RESPONSE, 0x00,0x00);
			}
		else {
			if((uiRxDLMemAddr/2)>uiEndAddr)	{
				nvSendUDSResponse(TRANSFER_DATA, POS_RESPONSE, 0x00,0x00);
			}
			else{
				//write to 
				if(UDSRvDataPara.uiSUB[0]<1){
					uiTempAddr=0;
				}
				else{
					uiTempAddr=(UDSRvDataPara.uiSUB[0]-1);
				}
				uiTempAddr=uiTempAddr*0x400;
//				uiTempAddr=uiTempAddr+APPLICATION_CODE_START_ADDR;
				uiTempAddr=uiTempAddr+(uiRxDLMemAddr/2);
				if(StartProgramming(&uaProgBufferBytes[0],uiTempAddr)){
					nvSendUDSResponse(TRANSFER_DATA, POS_RESPONSE, 0x00,0x00);
				}
				else{
					nvSendUDSNegResp(TRANSFER_DATA,NRTC_GEN_PRO_FAIL);
				}
			}

		}
//	}
}

UINT8 uiChkIVT(void)
{

	UINT32 uiDataValue,uiDataValue1;
	uReg32 uiAddr,uiAddr1;
	
	uiAddr.Val32=0x000FE;
	uiAddr1.Val32=0x001FE;
	uiDataValue=readWordFlash(uiAddr);
	uiDataValue1=readWordFlash(uiAddr1);

	if((uiDataValue !=0)&&(uiDataValue !=0x00FFFFFF)&&
		(uiDataValue1 !=0)&&(uiDataValue1 !=0x00FFFFFF)){
		return TRUE;
	}
	else return FALSE;
}

UINT8 uiIsIVTSaved(void)
{
	return uiChkIVT();
}

UINT8 uvReadUDSFlags(void)
{
	//uvBootReadAllUDSFlags();
	UINT8 result;
	UINT8 val1;
	UINT16 uiAddr;
	
	val1=EepRead((UINT32)EEPROM_WRITE_BUFFER_FLAGS_START+EEPROM_WRITE_UDS_INFO-1);

	flashWritePara.UDSInfoWrBuff=val1;
	if(val1==0){
		uiAddr=EEPROM_UDS_REQ_INFO;
	}
	else if(val1==1){
		uiAddr=EEPROM_UDS_REQ_INFO_BACKUP_1;
	}
	else if(val1==2){
		uiAddr=EEPROM_UDS_REQ_INFO_BACKUP_2;
	}
	else{
		//write again
		uvWriteUDSParaToExFlash();
		return FALSE;
	}
	result=readUDSDataFromEEPROM(uiAddr);

	//recovery process
	if(result==FALSE){
		return FALSE;
	}
	else{
		return TRUE;
	}
}

void uvReqReset(void)
{
	//reset
	IEC0=0;IEC1=0;IEC2=0;IEC3=0;IEC4=0;
	ResetDevice();
}

UINT8 uiGetUsedDataSize(UINT8 uiSIType)
{
	UINT8 uiUsedDataBytes=0;
	switch(uiSIType){
		case REQUEST_DOWNLOAD://	DataBufferBytes[2]=Length format identifier;
								//	DataBufferBytes[3~n]=max number of block length;
		case REQUEST_TRANSFER_EXIT://	DataBufferBytes[2~n]=transfer response parameter record;
			uiUsedDataBytes=2;				
			break;
		case START_DIAGNOSTIC_SESSION://	DataBufferBytes[3~n]=Data;//session parameter record
		case ECU_RESET:
		case TESTER_PRESENT:
		case SECURITY_ACCESS://	DataBufferBytes[3~n]=Seed Data;
		case TRANSFER_DATA://DataBufferBytes[2]=uaUDSReqPara[2];//Block sequence counter
							//DataBufferBytes[3~n]=transfer response parameter record;  for upload
			uiUsedDataBytes=3;				
			break;
		case WRITE_FINGERPRINT:
		case READ_DATA_BY_IDENTIFIER://not finish
			uiUsedDataBytes=4;				
			break;
		case ROUTINE_CONTROL://finished
			uiUsedDataBytes=5;				
			break;
		default:
			break;
	}
	return uiUsedDataBytes;
}


UINT8 uiGetSubFuncBytes(UINT8 uiSID)
{
	UINT8 uiFuncBytes=0;
	switch(uiSID){
		case START_DIAGNOSTIC_SESSION:
		case ECU_RESET:
		case TESTER_PRESENT:
		case SECURITY_ACCESS:
		case TRANSFER_DATA:
		case 0x85:
			uiFuncBytes=1;
			break;			
		case READ_DATA_BY_IDENTIFIER:
		case WRITE_FINGERPRINT:
		case REQUEST_DOWNLOAD:
		case 0x28:
			uiFuncBytes=2;
			break;
		case ROUTINE_CONTROL:
			uiFuncBytes=3;
			break;
		case REQUEST_TRANSFER_EXIT:
			uiFuncBytes=0;
			break;
		case 0x23:
			break;
		case 0x2C:
			break;
		case 0x3D:
			break;
		default:
			uiFuncBytes=0xff;
			break;
	}

	return uiFuncBytes;
}

void uv5Stimerout(void){
	//reset
	if(uiGetBootDiagSessionMode()!=DEFAULT_DIAG_SESSION){
		uvSetBootDiagSessionMode(DEFAULT_DIAG_SESSION);
		uvSetReqInfo(COMMUNICATION_TIME_OUT_BOOT);
//		ResetDevice();
	}

 }


void uvSetReqInfo(UINT8 uiReqInfo)
{
	uiRequestID=uiReqInfo;
	//store request


#ifdef STORE_DATA_TO_EXTERNAL_FLASH

	UDSWriteInfo.uiSessionMode=uiBootSessionMode;
	ProgInfo.uiRequestID=uiRequestID;
	uvWriteProgParaToExFlash();
	if(uiSoftValid){
		uvSetMode(DATA_STORE_MODE,DATA_STORING);
	}
	else{
		uvSetMode(DATA_STORE_MODE,DATA_IVT_STORED);
	}
	writeEepromTick=TickGet();
	ResetTimeOutTick=writeEepromTick;
#else

#endif

}
void uvClearProgReqFlag(void)
{
	ProgInfo.uiRequestID=0xff;
#ifdef STORE_DATA_TO_EXTERNAL_FLASH
	uvWriteProgParaToExFlash();
#endif
}

void initUDSData(void)
{
	register unsigned int uiCounter;
	uReg32 temp1;
	UINT8 uiCheckSumCalc;

	for(uiCounter=0;uiCounter<8;uiCounter++){
		uaUDSReqPara[uiCounter]=0;
		DataBufferBytes[uiCounter]=0;
	}
	for(uiCounter=0;uiCounter<MAX_NUM_OF_BLOCK_FOR_PROG;uiCounter++){
		uaBootMemoryBackup[uiCounter]=0;
		uaProgBufferIVT[uiCounter]=0;
		uaProgBufferBytes[uiCounter]=0;
	}
	uvCrc32Init(&uiCalcCRC);	
	temp1.Val32=0x0;
	ReadPM(&uaBootMemoryBackup[0],temp1);
	UDS_ModeFlags.uiDataRvMode=NORMAL_PROCESSING;
	UDS_ModeFlags.uiWritingMode=DATA_STORED;

	UDS_ModeFlags.uiProgrammingMode=PROGRMMING_MODE_DISABLE;

	if(uvBootReadAllUDSFlags()==FALSE){
		uvInitUDSWriteInfo();
		uvWriteUDSParaToExFlash();
	}
	else{
		uvCopyBuffer(&UDSWriteInfo.uiFirstByte, &UDSReadInfo.uiFirstByte, UDSPara_SIZE);
		if((UDSWriteInfo.uiBootSoftVer_PATCH!=uaDID_BootVersion[2])||
			(UDSWriteInfo.uiBootSoftVer_WEEK!=uaDID_BootVersion[1])||
			(UDSWriteInfo.uiBootSoftVer_YEAR!=uaDID_BootVersion[0])){
			//update bootloader version
			uvCopyBuffer(&UDSWriteInfo.uiBootSoftVer_YEAR,(UINT8 *)&uaDID_BootVersion[0] ,3);
			uvWriteUDSParaToExFlash();
		}	
	}

	
}
void uvInitUDSRvData(void)
{
	register unsigned int k;
	//init
	UDSRvDataPara.uiSID=0;
	UDSRvDataPara.uiSUB[0]=0;
	UDSRvDataPara.uiSUB[1]=0;
	UDSRvDataPara.uiSUB[2]=0;
	UDSRvDataPara.uiDataLen=0;
	for(k=0;k<MAX_NUM_OF_BLOCK_FOR_PROG;k++){
		UDSRvDataPara.uiData[k]=0;
	}

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#else
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
////////////////FOR APPLICATION UDS LAYER/////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void uiCopyBootVersion(void)
{
	uvCopyBuffer(&UDSWriteInfo.uiBootSoftVer_YEAR,(UINT8 *)&uaDID_BootVersion[0] ,3);
	uvWriteUDSParaToExFlash();

}
UINT8 uiChkProgPrecFromApp(void)
{
//   vehicleInfo.speed ?
#ifdef BMS_MODE

		UINT8 uiChkProgResult;
		 if(systemInfo.V12>KE_SystemVoltageUpperThresholdVolts){//?
			uiChkProgResult=2;
		 }
		else if(systemInfo.V12<KE_SystemVoltageLowerThresholdVolts){//?
			uiChkProgResult=3;
		}
		else if(systemInfo.cellTempMax> KE_TemperatureSensorUpperThreshold_HW){
			uiChkProgResult=0x0B;
		}
		else if( systemInfo.cellTempMin <KE_TemperatureSensorLowerThreshold_HW){
			uiChkProgResult=0x0C;
		}
		else uiChkProgResult=0;
		if(uiChkProgResult==0){
			writePackInfoToEEPROMAll();
			writeHCPCanParaToEEPROMAll();
			writeSystemInfoToEEPROMAll();
		}
		return uiChkProgResult;
		
//		return 0;
#else
		return 0;
#endif
}
UINT8 uiGetAppDiagSessionMode(void)
{
	return uiAppliSessionMode;
}

void uvSetAppDiagSessionMode(UINT8 uiSessionMode)
{
	uiAppliSessionMode=uiSessionMode;
	UDSWriteInfo.uiSessionMode=uiAppliSessionMode;

}

void uvSetFailSafeMode(UINT8 uiFailSafeMode)
{
	if(uiFailSafeMode==DIS_FAIL_SAFE_REACTION){
		uiAppFailSafeMode=DIS_FAIL_SAFE_REACTION;
	}
	else if(uiFailSafeMode==ENB_FAIL_SAFE_REACTION){
		uiAppFailSafeMode=ENB_FAIL_SAFE_REACTION;
	}
}

UINT8 uiGetFailSafeMode(void)
{
	return uiAppFailSafeMode;
}

#ifdef BMS_MODE
#else
UINT8 uiGetCommType(void)
{
	return uiCommRxTxType;
}
#endif	

void uvSetCommType(UINT8 uiCommType)
{
	//do communication control
	if(uiCommType==COMM_ENBRX_DISTX){
#ifdef BMS_MODE
			HCPCmd.BMCPCANWorkEnableSW=FALSE;
			HCPCmd.BMCToolsPresent=TRUE;		
#else
			uiCommRxTxType=COMM_ENBRX_DISTX;
#endif	
	}
	else if(uiCommType==COMM_ENBRX_ENBTX){
#ifdef BMS_MODE
			HCPCmd.BMCPCANWorkEnableSW=TRUE;
//Modified by Amanda for DTC inhibited problem
			HCPCmd.BMCPCANWorkEnableSWTick=TickGet();
			HCPCmd.BMCToolsPresent=FALSE;		
#else
			uiCommRxTxType=COMM_ENBRX_ENBTX;
#endif	
	}
}
UINT8 uiGetDTCType(void)
{
	return uiAppDTCType;
}
void nvSetDTCType(UINT8 uiDTCtype)
{
	if(uiDTCtype==DTC_TYPE_ON){
#ifdef BMS_MODE
		HCPCmd.CanDTCsEnabled=TRUE;		
#endif	
		uiAppDTCType=DTC_TYPE_ON;
	}
	else if(uiDTCtype==DTC_TYPE_OFF){
#ifdef BMS_MODE
		HCPCmd.CanDTCsEnabled=FALSE;		
#endif	
		uiAppDTCType=DTC_TYPE_OFF;
	}
}
UINT8 uvAppReadAllUDSFlags(void)
{
#ifdef STORE_DATA_TO_EXTERNAL_FLASH

	UINT8 val1;
	UINT8 result;
	
	val1=EepRead((UINT32)EEPROM_WRITE_BUFFER_FLAGS_START+EEPROM_WRITE_UDS_INFO-1);
	flashWritePara.UDSInfoWrBuff=val1;

	result=uvReadDataBuffer(EEPROM_WRITE_UDS_INFO,val1);
	return result;
#endif

}
UINT8 uvAppReadAllProgFlags(void)
{
#ifdef STORE_DATA_TO_EXTERNAL_FLASH

	UINT8 val1;
	UINT8 result;
	
	val1=EepRead((UINT32)EEPROM_WRITE_BUFFER_FLAGS_START+EEPROM_WRITE_PROG_INFO-1);
	flashWritePara.ProgInfoWrBuff=val1;

	result=uvReadDataBuffer(EEPROM_WRITE_PROG_INFO,val1);
	return result;
#endif

}


UINT8 nuAppGetReadID(void)
{
	UINT8 uiCheckedID;

	if((UDSRvDataPara.uiSUB[0]==0xA0)||(UDSRvDataPara.uiSUB[0]==0xA1)||(UDSRvDataPara.uiSUB[0]==0xA2)){
		uiCheckedID=0xff;
	}
	else{
		uiCheckedID=uiGetReadID();
	}
	if(uiCheckedID==0xff){
		//do PID parts
		uiCheckedID=uvGetPIDValue();
		if(uiCheckedID==0xff){
			return 0xff;
		}
		else return uiCheckedID;
	}
	else return uiCheckedID;
}


void nvStoreDID(UINT8 uiDIDtype)
{
	UINT8 uiSize;
	WriteDIDByte DataWriteDID;

	UINT16 uiTemp;
	
	DataWriteDID.uiDataByte=0;
	
	switch(uiDIDtype){
		case WRITE_DID_VIN_ODO_COUNTER:
			UDSWriteInfo.uiVinOdometer=UDSRvDataPara.uiData[0];
			uvWriteUDSParaToExFlash();
			break;
		case WRITE_DID_ODOMETER:
			uiSize=3;
			uvCopyBuffer(&UDSWriteInfo.uaOdometer[0], &UDSRvDataPara.uiData[0],uiSize);
	//		vehicleInfo.distance_lifetime=(((UINT32)UDSWriteInfo.uaOdometer[0]<<16)+((UINT32)UDSWriteInfo.uaOdometer[1]<<8)+UDSWriteInfo.uaOdometer[2] );
			uvWriteUDSParaToExFlash();
			break;
		case WRITE_DID_RSP_ON_EVENT:
			UDSWriteInfo.uiRspOnELActState=UDSRvDataPara.uiData[0];
			uvWriteUDSParaToExFlash();
			break;
		case WRITE_DID_ECU_PART_NUMBER:
			uiSize=UDSRvDataPara.uiDataLen-1-uiSubFuncBytes;
			if(uiSize>10){
				uiSize=10;
			}
			uvCopyBuffer(&UDSWriteInfo.uaEcuPartNumber[0], &UDSRvDataPara.uiData[0],uiSize);
			uvWriteUDSParaToExFlash();
			break;
		case WRITE_DID_VIN_CURRENT:
			uiSize=UDSRvDataPara.uiDataLen-1-uiSubFuncBytes;
			if(uiSize>17){
				uiSize=17;
			}
			UINT8 i,j,m,n;
			for(i=0,j=0,m=0,n=0;i<17;i++){
				if(UDSWriteInfo.uaVinORG[i]==0x20){
					j++;
				}
				else if(UDSWriteInfo.uaVinORG[i]==0xFF){
					m++;
				}
				else if(UDSWriteInfo.uaVinORG[i]==0x00){
					n++;
				}
			}
			if((j==17)||(m==17)||(n==17)){
				uvCopyBuffer(&UDSWriteInfo.uaVinORG[0], &UDSRvDataPara.uiData[0],uiSize);
			}
			uvCopyBuffer(&UDSWriteInfo.uaVinCurrent[0], &UDSRvDataPara.uiData[0],uiSize);
			uvWriteUDSParaToExFlash();
			break;
		case WRITE_DID_SOC:
			uiTemp=(UINT32)UDSRvDataPara.uiData[0]*1000/255;
			if (uiTemp>=1000){
				batteryPackInfo[0].SOC=1000;
				batteryPackInfo[0].capacity=(UINT32)batteryPackInfo[0].fullCapacity;
				batteryPackInfo[0].SOCmin=1000;
				batteryPackInfo[0].capacityMin=(UINT32)batteryPackInfo[0].fullCapacity;
				batteryPackInfo[0].SOCmax=1000;
				batteryPackInfo[0].capacityMax=(UINT32)batteryPackInfo[0].fullCapacity;
			}
			else{
				batteryPackInfo[0].SOC=uiTemp;
				batteryPackInfo[0].capacity=(UINT32)(batteryPackInfo[0].fullCapacity/1000)*batteryPackInfo[0].SOC;
				batteryPackInfo[0].SOCmin=uiTemp;
				batteryPackInfo[0].capacityMin=(UINT32)(batteryPackInfo[0].fullCapacity/1000)*batteryPackInfo[0].SOCmin;
				batteryPackInfo[0].SOCmax=uiTemp;
				batteryPackInfo[0].capacityMax=(UINT32)(batteryPackInfo[0].fullCapacity/1000)*batteryPackInfo[0].SOCmax;
			}
			break;
		case WRITE_DID_HW_ID:
			DataWriteDID.uiDataByte=UDSRvDataPara.uiData[0];
			if((DataWriteDID.Bit2==0)&&(DataWriteDID.Bit1==0)){
				if(DataWriteDID.Bit0==1){
					UDSWriteInfo.uiMBHWVersion=2;
				}
				else{
					UDSWriteInfo.uiMBHWVersion=1;
				}
				uvWriteUDSParaToExFlash();
			}
			else if(DataWriteDID.Bit2==1){
				iniEEPROMParaUDS();
			}
			else if(DataWriteDID.Bit1==1){
				vehicleInfo.distance_lifetime=0;//need clear
				vehicleInfo.distance_current=0;
				systemInfo.capacityGain=0;
				systemInfo.capacityLose=0;
				systemInfo.capacityGainTotal=0;//need clear
				systemInfo.capacityLoseTotal=0;//need clear
				systemInfo.prechargePenaltyTime=0;//need clear
				systemInfo.powerGainTotal=0;//need clear
				systemInfo.powerLoseTotal=0;//need clear
				systemInfo.powerGain=0;
				systemInfo.powerLose=0;
				batteryPackInfo[0].capacityGain=0;
				batteryPackInfo[0].capacityLose=0;
				batteryPackInfo[0].capacityGainTotal=0;
				batteryPackInfo[0].capacityLoseTotal=0;
				batteryPackInfo[0].powerGain=0;
				batteryPackInfo[0].powerGainTotal=0;
				batteryPackInfo[0].powerLose=0;
				batteryPackInfo[0].powerLoseTotal=0;
				initEEPROMParameters();	
				writeHCPCanParaToEEPROMAll();//force to write
			}
			break;
		case WRITE_DID_CONTACTOR_ENABLER:
			UDSWriteInfo.uiContactorEnabler=UDSRvDataPara.uiData[0];
			uvWriteUDSParaToExFlash();
			break;
		case WRITE_DID_MEC_COUNTER:
			UDSWriteInfo.uiMECCounter=UDSRvDataPara.uiData[0];
			uvWriteUDSParaToExFlash();
			break;
		case WRITE_DID_EOL_SCRATCH_PAD:
			uiSize=5;
			uvCopyBuffer(&UDSWriteInfo.uiEOLScratchPad[0], &UDSRvDataPara.uiData[0],uiSize);
			uvWriteUDSParaToExFlash();
			break;
		case WRITE_DID_HIDDEN_SOC:
			UDSWriteInfo.uiHiddenSOC=UDSRvDataPara.uiData[0];
	//		systemInfo.SOC=(UINT32)UDSRvDataPara.uiData[0]*1000/255;
			uiTemp=(UINT32)UDSRvDataPara.uiData[0]*1000/255;

			if (uiTemp>=1000){
				batteryPackInfo[0].SOC=1000;
				batteryPackInfo[0].capacity=(UINT32)batteryPackInfo[0].fullCapacity;
				batteryPackInfo[0].SOCmin=1000;
				batteryPackInfo[0].capacity=(UINT32)batteryPackInfo[0].fullCapacity;
				batteryPackInfo[0].SOCmax=1000;
				batteryPackInfo[0].capacity=(UINT32)batteryPackInfo[0].fullCapacity;
			}
			else{
				batteryPackInfo[0].SOC=uiTemp;
				batteryPackInfo[0].capacity=(UINT32)(batteryPackInfo[0].fullCapacity/1000)*batteryPackInfo[0].SOC;
				batteryPackInfo[0].SOCmin=uiTemp;
				batteryPackInfo[0].capacityMin=(UINT32)(batteryPackInfo[0].fullCapacity/1000)*batteryPackInfo[0].SOCmin;
				batteryPackInfo[0].SOCmax=uiTemp;
				batteryPackInfo[0].capacityMax=(UINT32)(batteryPackInfo[0].fullCapacity/1000)*batteryPackInfo[0].SOCmax;
			}
			
			uvWriteUDSParaToExFlash();
			break;			
		case WRITE_DID_CANLABLE_PACK_NO:
			UDSWriteInfo.uiCANLable_PackNo_H=UDSRvDataPara.uiData[0];
//			UDSWriteInfo.uiCANLable_PackNo_L=UDSRvDataPara.uiData[1];

			UDSWriteInfo.uaECUSerialNo[9]=UDSWriteInfo.uiCANLable_PackNo_H/100+'0';
			UDSWriteInfo.uaECUSerialNo[10]=(UDSWriteInfo.uiCANLable_PackNo_H/10)%10+'0';
			UDSWriteInfo.uaECUSerialNo[11]=UDSWriteInfo.uiCANLable_PackNo_H%10+'0';
			uvWriteUDSParaToExFlash();
			break;			
		case WRITE_DID_FORCE_TO_WRITE:
			if(UDSRvDataPara.uiData[0]==1){
				UDSWriteInfo.uiForceToWriteFlag=TRUE;
				writeHCPCanParaToEEPROMAll();
				writePackInfoToEEPROMAll();
				writeSystemInfoToEEPROMAll();
				writeDTCInfoToEEPROMAll();
				uvWriteUDSParaToExFlash();
			}
			else{
				UDSWriteInfo.uiForceToWriteFlag=FALSE;
			}
			break;
			//Added by Amanda for DTC Disable/Enable Dec20,2010
		case WRITE_DID_DTC_ENABLER:
			uiSize=20;
			uvCopyBuffer(&UDSWriteInfo.uiDTCGroupEnabler[0], &UDSRvDataPara.uiData[0],uiSize);
			uvWriteUDSParaToExFlash();
			break;
		default:
			break;
	}
}

UINT8 uiResetVIN(void)
{
	UINT8 i;
	for (i=0;i<17;i++){
		UDSWriteInfo.uaVinCurrent[i]=0x20;
	}
	uvWriteUDSParaToExFlash();
	return 0;
}

UINT8 uiIsIVTSaved(void)
{
	return TRUE;
}

UINT8 uvReadUDSFlags(void)
{
	UINT8 result;
	UINT8 val1;
	UINT16 uiAddr;
	
	val1=EepRead((UINT32)EEPROM_WRITE_BUFFER_FLAGS_START+EEPROM_WRITE_UDS_INFO-1);
	flashWritePara.UDSInfoWrBuff=val1;

	if(val1==0){
		uiAddr=EEPROM_UDS_REQ_INFO;
	}
	else if(val1==1){
		uiAddr=EEPROM_UDS_REQ_INFO_BACKUP_1;
	}
	else if(val1==2){
		uiAddr=EEPROM_UDS_REQ_INFO_BACKUP_2;
	}
	else{
		//write again
		uvWriteUDSParaToExFlash();
		return FALSE;
	}
	result=readUDSDataFromEEPROM(uiAddr);

	//recovery process
	if(result==FALSE){
		return FALSE;
	}
	else{
		return TRUE;
	}
}



void uvReqReset(void)
{
	reset();
}

UINT8 uiGetUsedDataSize(UINT8 uiSIType)
{
	UINT8 uiUsedDataBytes=0;
	switch(uiSIType){
		case START_DIAGNOSTIC_SESSION://	DataBufferBytes[3~n]=Data;//session parameter record
		case ECU_RESET:
		case TESTER_PRESENT:
		case COMMUNICATION_CONTROL:
		case CONTROL_DTC_SETTING:
		case READ_DTC_INFO:
//		case CONTROL_DEVICE:
			uiUsedDataBytes=3;				
			break;
		case WRITE_DID:
		case READ_DATA_BY_IDENTIFIER://not finish
			uiUsedDataBytes=4;				
			break;
		case CLEAR_DIAG_INFO:
		case ROUTINE_CONTROL://finished
		case CONTROL_DEVICE:
			uiUsedDataBytes=5;				
			break;
		default:
			break;
	}
	return uiUsedDataBytes;
}

UINT8 uiGetSubFuncBytes(UINT8 uiSID)
{
	UINT8 uiFuncBytes=0;

	switch(uiSID){
		case START_DIAGNOSTIC_SESSION:
		case ECU_RESET:
		case TESTER_PRESENT:
		case CONTROL_DTC_SETTING:
		case READ_DTC_INFO:
//		case CONTROL_DEVICE:
			uiFuncBytes=1;
			break;
		case READ_DATA_BY_IDENTIFIER:
		case WRITE_DID:
		case COMMUNICATION_CONTROL:
			uiFuncBytes=2;
			break;
		case ROUTINE_CONTROL:
		case CLEAR_DIAG_INFO:
		case CONTROL_DEVICE:
			uiFuncBytes=3;
			break;
		case 0x23:
			break;
		case 0x2C:
			break;
		case 0x3D:
			break;
		default:
			uiFuncBytes=0xff;
			break;
	}
	return uiFuncBytes;
}

void uv5Stimerout(void){
	//reset

	if(uiGetAppDiagSessionMode()!=DEFAULT_DIAG_SESSION){
		uvSetAppDiagSessionMode(DEFAULT_DIAG_SESSION);
		uvSetCommType(COMM_ENBRX_ENBTX);
		nvSetDTCType(DTC_TYPE_ON);
	}	
 }




void uvSetReqInfo(UINT8 uiReqInfo)
{
	uiRequestID=uiReqInfo;
	//store request
#ifdef STORE_DATA_TO_EXTERNAL_FLASH

	UDSWriteInfo.uiSessionMode=uiAppliSessionMode;
	ProgInfo.uiRequestID=uiRequestID;
	uvWriteProgParaToExFlash();
	uvSetMode(DATA_STORE_MODE,DATA_IVT_STORED);
	writeEepromTick=TickGet();

#else


#endif

}

void uvClearProgReqFlag(void)
{

	ProgInfo.uiRequestID=0xff;
#ifdef STORE_DATA_TO_EXTERNAL_FLASH
	uvWriteProgParaToExFlash();
#endif
}

void initUDSData(void)
{
	register unsigned int uiCounter;
		
	for(uiCounter=0;uiCounter<8;uiCounter++){
		uaUDSReqPara[uiCounter]=0;
		DataBufferBytes[uiCounter]=0;
	}

	UDS_ModeFlags.uiDataRvMode=NORMAL_PROCESSING;
	UDS_ModeFlags.uiWritingMode=DATA_STORED;

	UDSWriteInfo.uiRspOnELActState=0x01;
	if(uvAppReadAllProgFlags()==FALSE){//Amanda changed back from 1.51.6,since 1.50.1 has problem, no reset reponse after flashing

#if 0
		ProgInfo.uiRequestID=0xff;
		ProgInfo.uiApplicationInfo=0xff;
		ProgInfo.uiCheckSum=0xff;
		uvWriteProgParaToExFlash();
#endif
	}
	
	if(uvAppReadAllUDSFlags()==FALSE){
		uvInitUDSWriteInfo();
		uvWriteUDSParaToExFlash();
		
	}
	else{
		uvCopyBuffer(&UDSWriteInfo.uiFirstByte, &UDSReadInfo.uiFirstByte, UDSPara_SIZE);
		if((UDSWriteInfo.uiAppSoftVer_PATCH!=uaDID_AppliVersion[2])||
			(UDSWriteInfo.uiAppSoftVer_WEEK!=uaDID_AppliVersion[1])||
			(UDSWriteInfo.uiAppSoftVer_YEAR!=uaDID_AppliVersion[0])){
			//update bootloader version
			uvCopyBuffer(&UDSWriteInfo.uiAppSoftVer_YEAR,(UINT8 *)&uaDID_AppliVersion[0] ,3);
		//	UDSWriteInfo.uiAppSoftVer_PATCH=eeprom_ram_para.BPCM_Version[2]-'0';
			uvCopyBuffer(&UDSWriteInfo.uaEcuPartNumber[0],(UINT8 *)&uaDID_HardPartNo[0] ,10);
			UDSWriteInfo.uiContactorEnabler=0x03;//all enable
			//Disabled by Amanda for not init to enabler after flash new version 
			for(uiCounter=0;uiCounter<22;uiCounter++){
				UDSWriteInfo.uiDTCGroupEnabler[uiCounter]=0xFF;//all enable
			}
#if 1//Added by Amanda August 5
				UDSWriteInfo.uiPassConterDTC=0;
			#endif

			//For 12 packs already in Chrysler, disable all Fan DTCs
			
			uvWriteUDSParaToExFlash();
		}


	}
}

void uvInitUDSRvData(void)
{
	register unsigned int k;
	//init
	UDSRvDataPara.uiSID=0;
	UDSRvDataPara.uiSUB[0]=0;
	UDSRvDataPara.uiSUB[1]=0;
	UDSRvDataPara.uiSUB[2]=0;
	UDSRvDataPara.uiDataLen=0;
	for(k=0;k<512;k++){
		UDSRvDataPara.uiData[k]=0;
	}
}


























#endif


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
////////////////FOR APPLICATION/BOOTLOADER UDS LAYER/////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void nvStoreFingerPrintData(void)
{
	uvCopyBuffer(&uiFingerprint.uiLogBlock,&UDSRvDataPara.uiData[0],10 );
	uvCopyBuffer(&UDSWriteInfo.uiFingerprint.uiLogBlock, &uiFingerprint.uiLogBlock,10);
	uvWriteUDSParaToExFlash();
}


void	uvStartTimer(UINT8 uiType)
{
	if(uiType==TIMER_PERIOD_5S){
		checkTick=TickGet();
	}
	else if(uiType==TIMER_PERIOD_5MS){
		FlowControlTxTick=TickGet();
	}
	else{
		writeEepromTick=TickGet();
	}
	
}

void uvSetPosData(UINT8 uiSIType, UINT8 uiStartByte, UINT8 uiUsedSize)
{

	UINT8 i;

	DataBufferBytes[uiStartByte]=uiSIType+0x40;
	if(uiUsedSize>2){
		for(i=0;i<(uiUsedSize-2);i++){
			DataBufferBytes[uiStartByte+1+i]=UDSRvDataPara.uiSUB[i];
		}
	}
}


UINT8 uiCheckBytes(UINT8 uiByteSize, UINT8 uiDataSize, UINT8 * pData)
{
	UINT8 i;
	UINT8 * pt;

	pt=pData;

	
	switch (uiByteSize){
		case 1:
			for(i=0;i<uiDataSize;i++,pt++){
				if(UDSRvDataPara.uiSUB[0]==*pt){
					return TRUE;
				}
			}
			break;
		case 2:
			for(i=0;i<uiDataSize;i++){
				if(UDSRvDataPara.uiSUB[0]==*pt){
					if(UDSRvDataPara.uiSUB[1]==*(pt+1)){
						return TRUE;
					}
				}
				pt+=2;
			}	
			break;
		case 3:
			for(i=0;i<uiDataSize;i++){
				if(UDSRvDataPara.uiSUB[0]==*pt){
					if(UDSRvDataPara.uiSUB[1]==*(pt+1)){
						if(UDSRvDataPara.uiSUB[2]==*(pt+2)){
							return TRUE;
						}
					}
				}
				pt+=3;
			}
			break;
		default:
			break;
	}
	return FALSE;
}

UINT8 uiSubFuncAvailCheck(UINT8 uiSID)
{
	UINT8 uiResult=TRUE;
	
	switch(uiSID){
		case START_DIAGNOSTIC_SESSION:
			uiResult=uiCheckBytes(ONE_BYTE_CHECK,6,(UINT8 *)&uaSID10_SubFuncTbl[0]);
			break;
		case ECU_RESET:
			uiResult=uiCheckBytes(ONE_BYTE_CHECK,2,(UINT8 *)&uaSID11_SubFuncTbl[0]);
			break;
		case TESTER_PRESENT:
			uiResult=uiCheckBytes(ONE_BYTE_CHECK,3,(UINT8 *)&uaSID3E_SubFuncTbl[0]);
			break;
#ifdef BOOT_MODE
		case SECURITY_ACCESS:
			uiResult=uiCheckBytes(ONE_BYTE_CHECK,2,(UINT8 *)&uaSID27_SubFuncTbl[0]);
			break;
//		case 0x85:
//			uiResult=uiCheckBytes(ONE_BYTE_CHECK,4,(UINT8 *)&uaSID85_SubFuncTbl[0]);
//			break;
		case WRITE_FINGERPRINT:
			uiResult=uiCheckBytes(TWO_BYTES_CHECK,1,(UINT8 *)&uaSID2E_SubFuncTbl[0][0]);
			break;
//		case 0x28:
//			uiResult=uiCheckBytes(TWO_BYTES_CHECK,4,(UINT8 *)&uaSID28_SubFuncTbl[0][0]);
//			break;
		case ROUTINE_CONTROL:
			uiResult=uiCheckBytes(THREE_BYTES_CHECK,6,(UINT8 *)&uaSID31_SubFuncTbl[0][0]);
			break;
		case REQUEST_DOWNLOAD:
		case TRANSFER_DATA:
		case REQUEST_TRANSFER_EXIT:
			uiResult=TRUE;
			break;
#else
		case CLEAR_DIAG_INFO:
		case CONTROL_DEVICE:
			uiResult=TRUE;
			break;
		case READ_DTC_INFO:
			uiResult=uiCheckBytes(ONE_BYTE_CHECK,8,(UINT8 *)&uaSID19_SubFuncTbl[0]);
			break;
		case CONTROL_DTC_SETTING:
			uiResult=uiCheckBytes(ONE_BYTE_CHECK,4,(UINT8 *)&uaSID85_SubFuncTbl[0]);
			break;
		case COMMUNICATION_CONTROL:
			uiResult=uiCheckBytes(TWO_BYTES_CHECK,4,(UINT8 *)&uaSID28_SubFuncTbl[0][0]);
			break;
		case WRITE_DID:
			uiResult=uiCheckBytes(TWO_BYTES_CHECK,WRITE_DID_TABLE_SIZE,(UINT8 *)&uaSID2E_SubFuncTbl[0][0]);
			break;
		case ROUTINE_CONTROL:
			uiResult=uiCheckBytes(THREE_BYTES_CHECK,ROUTINE_CONTROL_TABLE_SIZE,(UINT8 *)&uaSID31_SubFuncTbl[0][0]);
			break;
#endif
		case READ_DATA_BY_IDENTIFIER:
//			uiResult=uiCheckBytes(TWO_BYTES_CHECK,READ_DATA_BY_ID_MAX,(UINT8 *)&uaReadIDChkTbl[0][0]);
			uiResult=TRUE;
			break;
		default:
			uiResult=TRUE;
			break;
	}
	return uiResult;	
}



void nvSendData(void)
{

	ECAN_MSG_BUF canMsg;

	UINT8 j;

	canMsg.HEADER.words[0]=0;canMsg.HEADER.words[1]=0;canMsg.HEADER.words[2]=0;
#ifdef BATT_MODE
	canMsg.HEADER.sid=0x3D7;
#else
	canMsg.HEADER.sid=BPCM_BOOT_PHYSICAL_RESPONSE_ADDRESS;
#endif
	canMsg.HEADER.dlc=8;
	//send message				
	for(j=0;j<8;j++){
		canMsg.DATA.bytes[j]=DataBufferBytes[j];
	}
//	ECAN1SendMsg(&canMsg);
	uvStoreToBuffer(&canMsg);

}


void nvMakeFCData(UINT8 uiUsedBytes,UINT8 * ptDataAddr, UINT16 uiDataSize)
{

	int k;
	UINT8 * ptAddr;
	ptAddr=	ptDataAddr;

	for(k=0;k<(8-uiUsedBytes);k++){
		DataBufferBytes[uiUsedBytes+k]=*ptAddr;
		ptAddr++;
	}
#ifdef BOOT_MODE
	for(k=0;k<512;k++){
		uiTxData[k]=0;	
	}
#else
	for(k=0;k<UDS_DATA_TRANSMIT_LEN;k++){
		uiTxData[k]=0;	
	}
#endif
	for(k=0;k<(uiDataSize-(8-uiUsedBytes));k++){
		uiTxData[k]=*ptAddr;	
		ptAddr++;
	}
	uiLeftBytes=k;
	uiStartBlockSeq=0x21;
	uiSendBlockNo=0;	
	uiTxMode=FLOW_CONTROL_BEGIN;
}

void nvFCData(void)
{
	int k;
	
	if(uiTxMode==FLOW_CONTROL_CONTINUE){
		for(k=0;k<8;k++){
		//	DataBufferBytes[k]=0;
			DataBufferBytes[k]=0xAA;
		}

		DataBufferBytes[0]=uiStartBlockSeq;
		if(uiLeftBytes>7){
			for(k=0;k<7;k++){
				DataBufferBytes[k+1]=uiTxData[uiSendBlockNo*7+k];	
			}
			nvSendData();
			uvStartTimer(TIMER_PERIOD_5MS);
			if(uiStartBlockSeq!=0x2F){
				uiStartBlockSeq++;
			}
			else{
				uiStartBlockSeq=0x20;				
			}
			uiSendBlockNo++;
			uiLeftBytes-=7;
		}
		else{
			for(k=0;k<uiLeftBytes;k++){
				DataBufferBytes[k+1]=uiTxData[uiSendBlockNo*7+k];	
			}
			nvSendData();
			uiTxMode=NORMAL_PROCESSING;
		}
	}

}

void nvMakeData(UINT8 uiSIType,UINT8 uiRspType,UINT8 * ptDataAddr, UINT16 uiDataSize)
{
	UINT8 uiCount;
	int j,k;
	UINT8 * ptAddr;
	ptAddr=	ptDataAddr;
	UINT8 uiUsedDataBytes=0;
	
	uvSetMode(RSP_MODE,RSP_NO_PENDING);
	for (uiCount=0;uiCount<8;uiCount++){
	//	DataBufferBytes[uiCount]=0;
		DataBufferBytes[uiCount]=0xAA;
	}
	if (uiRspType==NEG_RESPONSE){
		DataBufferBytes[1]=0x7F;
		DataBufferBytes[2]=uiSIType;
		DataBufferBytes[3]=*ptAddr;
		DataBufferBytes[0]=3;
		if(DataBufferBytes[3]==NRTC_SERCORRECT_RSPPENDING){
			uvSetMode(RSP_MODE,RSP_PENDING);
		}
	}
	else if (uiRspType==POS_RESPONSE){
		uiUsedDataBytes=uiGetUsedDataSize(uiSIType);
		if(uiDataSize!=0){
			if(uiDataSize>(8-uiUsedDataBytes)){
				uvSetPosData(uiSIType,2,uiUsedDataBytes);
				nvMakeFCData(uiUsedDataBytes+1,ptDataAddr,uiDataSize);
				j=uiUsedDataBytes+uiDataSize-1;
				
				DataBufferBytes[0]=((j>>8)&0x0F)+0x10;
				DataBufferBytes[1]=j&0xFF;
			}
			else{
				uvSetPosData(uiSIType,1,uiUsedDataBytes);
				for(k=0;k<uiDataSize;k++){
					DataBufferBytes[uiUsedDataBytes+k]=*ptAddr;
					ptAddr++;
				}
				DataBufferBytes[0]=uiUsedDataBytes+uiDataSize-1;
			}
		}
		else{
			uvSetPosData(uiSIType,1,uiUsedDataBytes);

			DataBufferBytes[0]=uiUsedDataBytes-1;
		}
	}
	else if(uiRspType==FLOW_CONTROL_RESPONSE){
		DataBufferBytes[0]=0x30;
		DataBufferBytes[1]=0x00;
		DataBufferBytes[2]=0x02;//ST time is 2ms 
	}

	
}

void nvSendUDSResponse(UINT8 uiSIType,UINT8 uiRspType,UINT8 * ptDataAddr, UINT16 uiDataSize)
{
	nvMakeData(uiSIType,uiRspType,ptDataAddr,uiDataSize);
	nvSendData();
}
void nvSendUDSFLResp(UINT8 uiSIType,UINT8 * ptDataAddr, UINT16 uiDataSize)
{
	nvSendUDSResponse(uiSIType,FLOW_CONTROL_RESPONSE,ptDataAddr,uiDataSize);
}
//modified by Amanda for flash fail on OCT 27
void nvSendUDSPosResp(UINT8 uiSIType,UINT8 * ptDataAddr, UINT16 uiDataSize)
{
	//October 27 2010
	//Modified by Amanda for supporting 0x441(global function address) and 0x7D6(physical address request) according to cs11736
	if ( (uiCanSID==BPCM_BOOT_FUNCTIONAL_ADDRESS)
		&& ((UDSRvDataPara.uiSUB[0]>>7)==1) ){
		return;
	}
	else{
		//for physical address
		//no response request
		if(  (uiCanSID==BPCM_BOOT_PHYSICAL_REQUEST_ADDRESS)
			&& ((UDSRvDataPara.uiSUB[0]>>7)==1)
			&& ((UDSRvDataPara.uiSID==START_DIAGNOSTIC_SESSION)||(UDSRvDataPara.uiSID==ECU_RESET)
				||(UDSRvDataPara.uiSID==ROUTINE_CONTROL)||(UDSRvDataPara.uiSID==TESTER_PRESENT)) ){
				return;
		}
		else{		
			nvSendUDSResponse(uiSIType,POS_RESPONSE,ptDataAddr,uiDataSize);
		}
	}
}
//modified by Amanda for flash fail on OCT 27
void nvSendUDSNegResp(UINT8 uiSIType,UINT8 uiRspNegType)
{
	//October 27 2010
	//Modified by Amanda for supporting 0x441(global function address) and 0x7D6(physical address request) according to cs11736
	if( (uiCanSID==BPCM_BOOT_FUNCTIONAL_ADDRESS)
		&&((uiRspNegType==NRTC_SERVICE_NOTSUPPORTED)||(uiRspNegType==NRTC_SUB_FUN_NOTSUPPORTED)||(uiRspNegType==NRTC_REQ_OUT_OF_RANGE))){
		return;
	}
	else{
		//BOOT  -->READ_DATA_BY_IDENTIFIER, SECURITY_ACCESS,WRITE_DID,REQUEST_DOWNLOAD no that bit
		//TRANSFER_DATA,REQUEST_TRANSFER_EXIT no that bit
		//BOOT  -->START_DIAGNOSTIC_SESSION,ECU_RESET,ROUTINE_CONTROL,TESTER_PRESENT have that bit
		//APP  -->COMMUNICATION_CONTROL,CONTROL_DTC_SETTING have that bit
		//APP -->CLEAR_DIAG_INFO,READ_DTC_INFO,CONTROL_DEVICE no that bit
		//Not support--->READ_MEMORY_BY_ADDR,REQUEST_UPLOAD,WRITE_MEMORY_BY_ADDR no that bit
		//Not support--->DYNAMIC_DEFINE_DATA_ID have that bit
		//for physical address
		if(  (uiCanSID==BPCM_BOOT_PHYSICAL_REQUEST_ADDRESS)
			&& ((UDSRvDataPara.uiSUB[0]>>7)==1)
			&& ((UDSRvDataPara.uiSID==START_DIAGNOSTIC_SESSION)||(UDSRvDataPara.uiSID==ECU_RESET)
				||(UDSRvDataPara.uiSID==ROUTINE_CONTROL)||(UDSRvDataPara.uiSID==TESTER_PRESENT)) ){
				return;
		}
		else{		
			uiNegRspTroubleCode=uiRspNegType;
			if(UDSRvDataPara.uiSID<10){
				return;
			}
			nvSendUDSResponse(uiSIType,NEG_RESPONSE,&uiNegRspTroubleCode,1 );
		}
	}
}

UINT8 uvChkFlowControl(void)
{
	UINT8 uiFC_begin;
	UINT8 uiFC_Continue;

	
	if(UDS_ModeFlags.uiDataRvMode==FLOW_CONTROL_BEGIN){
		uiFC_Continue=(uaUDSReqPara[0]&0x20)>>5;
		if(uiFC_Continue==1){
			UDS_ModeFlags.uiDataRvMode=FLOW_CONTROL_CONTINUE;
		}
	}
	else if(UDS_ModeFlags.uiDataRvMode==NORMAL_PROCESSING){
		uiFC_begin=(uaUDSReqPara[0]&0x10)>>4;
		if(uiFC_begin==1){
			uiSIDOfFlowControl=uaUDSReqPara[2];
			UDS_ModeFlags.uiDataRvMode=FLOW_CONTROL_BEGIN;
		}
		else{
			uiSIDOfFlowControl=uaUDSReqPara[1];
		}
	}
	else if(UDS_ModeFlags.uiDataRvMode==FLOW_CONTROL_CONTINUE){
		uiFC_Continue=(uaUDSReqPara[0]&0x20)>>5;
		if((uiFC_Continue!=1)){
			uiFC_begin=(uaUDSReqPara[0]&0x10)>>4;
			if(uiFC_begin==1){
				uiSIDOfFlowControl=uaUDSReqPara[2];
				UDS_ModeFlags.uiDataRvMode=FLOW_CONTROL_BEGIN;
			}
			else{
				uiSIDOfFlowControl=uaUDSReqPara[1];
				UDS_ModeFlags.uiDataRvMode=NORMAL_PROCESSING;
			}
		}
		
	}
	return uiSIDOfFlowControl;
}




void uv5mStimerout(void){
	nvFCData();
}

UINT8 uvRvUDSData(UINT16 uiLen,UINT8 uiStartPoint )
{
	UINT8  i;
	
	UDSRvDataPara.uiDataLen=uiLen;
	UDSRvDataPara.uiSID=uaUDSReqPara[uiStartPoint];
	uiSubFuncBytes=uiGetSubFuncBytes(UDSRvDataPara.uiSID);
	if(uiSubFuncBytes==0xff){
		//send negative response (Service not supported)
		nvSendUDSNegResp(UDSRvDataPara.uiSID,NRTC_SERVICE_NOTSUPPORTED);
		if(UDS_ModeFlags.uiDataRvMode!=NORMAL_PROCESSING){
			UDS_ModeFlags.uiDataRvMode=NORMAL_PROCESSING;
		}
		return FALSE;
	}
	else{
		if(UDSRvDataPara.uiDataLen<(uiSubFuncBytes+1)){
			//send negative response (Incorrect Msg Length)
			nvSendUDSNegResp(UDSRvDataPara.uiSID,NRTC_INCORR_MSG_LENGTH);
			if(UDS_ModeFlags.uiDataRvMode!=NORMAL_PROCESSING){
				UDS_ModeFlags.uiDataRvMode=NORMAL_PROCESSING;
			}
			return FALSE;
		}
		else{
			for(i=0;i<uiSubFuncBytes;i++){
				UDSRvDataPara.uiSUB[i]=uaUDSReqPara[uiStartPoint+1+i];
			}
			if(uiSubFuncAvailCheck(UDSRvDataPara.uiSID)){
				for (i=0;i<(8-uiStartPoint-1-uiSubFuncBytes);i++){
					UDSRvDataPara.uiData[i]=uaUDSReqPara[uiStartPoint+1+uiSubFuncBytes+i];
				}
				uiOffset=(8-uiStartPoint-1-uiSubFuncBytes);
				if(UDS_ModeFlags.uiDataRvMode==FLOW_CONTROL_BEGIN){
					nvSendUDSFLResp(0x00, 0x00,0x00);
					return FALSE;
				}
			}
			else{
				//send negative response (Sub function not supported)
				nvSendUDSNegResp(UDSRvDataPara.uiSID,NRTC_SUB_FUN_NOTSUPPORTED);
				if(UDS_ModeFlags.uiDataRvMode!=NORMAL_PROCESSING){
					UDS_ModeFlags.uiDataRvMode=NORMAL_PROCESSING;
				}
				return FALSE;
			}
		}
	}
	return TRUE;

}

UINT8 uvSendLastOptResp(void)
{
	UINT8 uiFlag;

	uiFlag=ProgInfo.uiRequestID;

	if((uiFlag==DIAG_DEFAULT_SESSION_REQ_FROM_BOOT)||(uiFlag==DIAG_PROG_SESSION_REQ_FROM_APP)){
		uvClearProgReqFlag();
		UDSRvDataPara.uiSUB[0]=uiFlag;
		nvSendUDSPosResp(START_DIAGNOSTIC_SESSION, 0x00,0x00);
		
		UDSRvDataPara.uiSUB[0]=0;

		if(uiFlag==DIAG_PROG_SESSION_REQ_FROM_APP){
	//		uiBootSessionMode=UDSWriteInfo.uiSessionMode;
#ifdef BOOT_MODE
			uiBootSessionMode=PROGRAMMING_DIAG_SESSION;
#endif
		}
		return TRUE;
	}
//	else if(uiFlag==RESET_REQ_FROM_BOOT){
	else if((uiFlag==RESET_REQ_FROM_BOOT)||(uiFlag==RESET_REQ_FROM_APP)){	
		uvClearProgReqFlag();
		UDSRvDataPara.uiSUB[0]=0x01;
		nvSendUDSPosResp(ECU_RESET, 0x00,0x00);
		
		UDSRvDataPara.uiSUB[0]=0;
		return TRUE;
	}
	return FALSE;
}


UINT8 uiIsTimeout(UINT8 uiType)
{
	if(uiType==TIMER_PERIOD_5S){
		if(isDelayExpired(SECONDS(5),checkTick)){
			return TRUE;
		}
		else return FALSE;
	}
	else if(uiType==TIMER_PERIOD_5MS){
		//For flow control ST time added on Sep 23, 2010
		if((uiSTtimeFL<=2)||(uiSTtimeFL>=0x7F)){
			if(isDelayExpired(MILISECOND(2),FlowControlTxTick)){
				return TRUE;
			}
			else return FALSE;
		}
		else{
			if(isDelayExpired(MILISECOND(uiSTtimeFL),FlowControlTxTick)){
				return TRUE;
			}
			else return FALSE;
		}
	}
	else if(uiType==TIMER_PERIOD_1S){
		if(isDelayExpired(MILISECOND(100),writeEepromTick)){
			return TRUE;
		}
		else return FALSE;
	}
	else if(uiType==TIMER_PERIOD_10MS){
		if(isDelayExpired(MILISECOND(10),writeEepromTick)){
			return TRUE;
		}
		else return FALSE;
	}
	return FALSE;
	
}

UINT8 uvProcessMsgData(void)
{
	UINT16 uiDataLen;
	UINT8  uiStartByte;
	UINT8  i;

	if(UDS_ModeFlags.uiDataRvMode==FLOW_CONTROL_BEGIN){
		uvInitUDSRvData();	
		uiDataLen=uaUDSReqPara[0]&0x0F;
		uiDataLen=(uiDataLen<<8)+uaUDSReqPara[1];
		uiStartByte=2;
		if(uvRvUDSData(uiDataLen, uiStartByte)==TRUE){
#ifdef BOOT_MODE
			if(UDSRvDataPara.uiSID==TRANSFER_DATA){
				uiFinalBlockNo=UDSRvDataPara.uiSUB[0];
			}
#endif	
		}
		else{
			return FALSE;
		}
	}
	else if(UDS_ModeFlags.uiDataRvMode==FLOW_CONTROL_CONTINUE){
//Add on 03/22/2010 for fix flash bootloader's bug --unexcept resposne
		for (i=0;i<7;i++,uiOffset++){
			if(uiOffset<(UDSRvDataPara.uiDataLen-1-uiSubFuncBytes)){
				UDSRvDataPara.uiData[uiOffset]=uaUDSReqPara[1+i];
			}
			if(uiOffset==(UDSRvDataPara.uiDataLen-1-uiSubFuncBytes)-1){
				uiReceivedSize+=(uiOffset+1);
				UDS_ModeFlags.uiDataRvMode=NORMAL_PROCESSING;
				return TRUE;
			}
		}
	}
	else{
		uvInitUDSRvData();
		uiDataLen=uaUDSReqPara[0];
		uiStartByte=1;
		if(uvRvUDSData(uiDataLen, uiStartByte)==TRUE){
#ifdef BOOT_MODE
			if(UDSRvDataPara.uiSID==TRANSFER_DATA){
				uiReceivedSize=uiDataLen-uiStartByte-1;
			}
#endif		
		}
		else{
			return FALSE;
		}
	}
	return TRUE;
}


UINT8 uiCompareDataBuffer(UINT8 * pCompareTo, UINT8 * pCompareFrom, UINT8 uiSize){

	UINT8 i,j;

    for(i=0,j=0;i<uiSize;i++){
		if(*pCompareTo==*pCompareFrom){
           j++;
  		}      
		else{
			return FALSE;
		}
		pCompareTo++;
		pCompareFrom++;
	}
    if(j==uiSize){
		return TRUE;
	}
    else return FALSE;
}

void uv30mstimerout(void)
{

	UINT8 uiDataStoreFlag;
	uiDataStoreFlag=uiGetMode(DATA_STORE_MODE);


#ifdef STORE_DATA_TO_EXTERNAL_FLASH

	if(isDelayExpired(SECONDS(20),ResetTimeOutTick)){
		uvSetMode(DATA_STORE_MODE,DATA_STORED);
		uvReqReset();
	}
	else{
		switch(uiDataStoreFlag){
			case DATA_STORING:
				if(uiIsIVTSaved()){
					uvSetMode(DATA_STORE_MODE,DATA_IVT_STORED);
				}
				writeEepromTick=TickGet();
				break;
			case DATA_IVT_STORED:
				if(flashWritePara.ProgInfoWrReq==0){
					if(uvReadProgFlags(1)){
						uvSetMode(DATA_STORE_MODE,DATA_READING);
					}
					else{
						writeEepromTick=TickGet();
					}
				}	
				else{
					writeEepromTick=TickGet();
				}
				
				break;
			case DATA_READING:
				if(flashWritePara.UDSInfoWrReq==0){
					if(uvReadUDSFlags()){
						uvSetMode(DATA_STORE_MODE,DATA_STORED);
						uvReqReset();
					}
					else{
						writeEepromTick=TickGet();
					}
				}
				else{
					writeEepromTick=TickGet();
				}
				
				break;
			default:
				break;

		}
	}
#else

#endif

}

void uvWriteDeviceID(UINT8 uiDeviceID)
{
	UDSWriteInfo.uiMBHWVersion=uiDeviceID;
	uvWriteUDSParaToExFlash();
}

UINT8 uiGetDeviceID(void)
{

	return UDSWriteInfo.uiMBHWVersion;
}

UINT8 uiGetDtcSts(void)
{
	UINT8 uiTemp,uiImpleCnt,uiIsRunCnt,uiIsFailOCCnt;
	UINT8 uiType;
	UINT8 i;
	
	uiImpleCnt=uiIsRunCnt=uiIsFailOCCnt=0;
	uiType=0;
	i=0;
	for(uiTemp=0;uiTemp<DTC_NUMBER_LONG;uiTemp++){
		if(DTCLongTable[uiTemp].isImplemented==1){
			if(DTCLongTable[uiTemp].isRunOC==1){
				uiIsRunCnt++;
			}
			else{
				uiDataIDindex[i]=uiTemp;
				i++;
			}
			if(DTCLongTable[uiTemp].isFailOC==1){
				uiIsFailOCCnt++;
			}
			uiImpleCnt++;
		}
	}
	if(uiIsFailOCCnt!=0){
		uiType=2;//any one fail
	}
	else{
		if(uiImpleCnt==uiIsRunCnt){
			uiType=1;//all run and pass
		}
		else{
			uiType=0;//not all run and no fail
		}
	}
	return uiType;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
////////////////END//////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifdef GATEWAY_SUPPORT
void nvReturnNormalMode(void)
{
	uvSetCommType(COMM_ENBRX_ENBTX);
	uvSetMode(GATEWAY_MODE, FALSE);
}
void nvSendReqGateway(void)
{

	ECAN_MSG_BUF canMsg;

	UINT8 j;

	uvSetCommType(COMM_ENBRX_DISTX);
	uvSetMode(GATEWAY_MODE, TRUE);
	canMsg.HEADER.words[0]=0;canMsg.HEADER.words[1]=0;canMsg.HEADER.words[2]=0;
//	canMsg.HEADER.sid=canMsg.HEADER.sid-0x400;
	canMsg.HEADER.sid=0x3D6;
	canMsg.HEADER.dlc=8;
	//send message				
	for(j=0;j<8;j++){
		canMsg.DATA.bytes[j]=uaUDSGateWayPara[j];
	}
//delete by Amanda	ECAN2SendMsg(&canMsg);
	uvStoreToCan2Buffer(&canMsg);
}
void nvSendRspGateway(void)
{

	ECAN_MSG_BUF canMsg;

	UINT8 j;

	canMsg.HEADER.words[0]=0;canMsg.HEADER.words[1]=0;canMsg.HEADER.words[2]=0;
	canMsg.HEADER.sid=0x7EF;
	canMsg.HEADER.dlc=8;
	//send message				
	for(j=0;j<8;j++){
		canMsg.DATA.bytes[j]=uaUDSGateWayRsp[j];
	}
	if((uaUDSGateWayRsp[1]==0x7F)&&(uaUDSGateWayRsp[2]==0x11)&&(uaUDSGateWayRsp[3]==0x78)){
		nvReturnNormalMode();
	}
	ECAN1SendMsg(&canMsg);
}
#endif




