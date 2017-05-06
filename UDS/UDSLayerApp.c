/****************************************************************************************************
 *	UDS Diagnostic Layer(APP).
 *    
 *	Auther: 
 *
 *   (c) 2010 ElectroVaya
 *
 *******************************************************************************************************/
/***************************************************/
/*******	INCLUDING FILES DEFINATION SECTION	****/
/***************************************************/

#include "UDSLayerApp.h"
//#include "Timing.h"

#include "UDSLib.h"

//#include "Controller.h"
//#include "Ecu.h"
//#include "Vehicle.h"
//#include "Console.h"
#include "config.h"

/***************************************************/
/*******	VARIABLES DEFINATION SECTION            	****/
/***************************************************/
#ifdef BOOT_MODE
#else

#if 1//Added by Amanda August 5
#define PCODE_SIZE			(87)  
#else
#define PCODE_SIZE			(85)  //76->73->76->80-->81 for backup12 voltage fail of PCA11
#endif
//defination for SID 0x19's DTC status
typedef union
{
	UINT8   uiDataByte; 
	struct{
 		UINT8   TestFailed:1, 
 		     TestFailedThisOC:1, 
 		     PendingDTC:1, 
 		     ConfirmedDTC:1, 
 		     NotCompleted:1, 
 		     FailedSinceLastClear:1, 
 		     NotCompletedThisOC:1, 
 		     WarningIndicatorReq:1; 
	};
}StsMask;


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
}IOControlByte;


UINT8 uiCheckResult=0;
TICK ContactorTick;

UINT8 uiIsoTestByCDA=FALSE;

extern UINT8 uaBuffer[UDS_DATA_TRANSMIT_LEN];
extern UINT16 uiSendSize;
extern UINT8 uiRequestID;
extern UINT16 uiCanSID;
//clear for SID 0x14
FanControl FanControlData;


UINT8 uaIDLongTbl[DTC_NUMBER_LONG];
UINT8 uaStoredIDLongTbl[DTC_NUMBER_LONG];
UINT8 uiIndexLong;
UINT8 uiIndexStoredLong;
//Added by Amanda for keeping ECU awake when there are requests from UDS
TICK lastReqFromUDSTick;
UINT8 uiBalanceByCDA=FALSE;
//For flow control ST time added on Sep 23, 2010
extern UINT8 uiSTtimeFL;

TICK IsoTestTick;
//static const UINT8 uaPcodeTbl[PCODE_SIZE][2] = {//size is 2
UINT8 uaPcodeTbl[PCODE_SIZE][2] = {//size is 2
//OBD related Pcode
	{0xD8, 0x85},//BPCM Lost Communication With HCP
	{0x0A, 0xBC},//BPCM Hybrid Battery Pack Voltage Sense 1 Circuit Low  P0ABC
	{0x0A, 0xBD},//BPCM Hybrid Battery Pack Voltage Sense 1 Circuit High P0ABD
	{0x0A, 0xBB},//BPCM Hybrid Battery Pack Voltage Sense 1 Circuit Performance 
	{0x0A, 0xC1},//BPCM Hybrid Battery Pack Current Sensor 1 Circuit Low P0AC1
	{0x0A, 0xC2},//BPCM Hybrid Battery Pack Current Sensor 1 Circuit High P0AC2
	{0x0A, 0xC0},//BPCM Hybrid Battery Pack Current Sensor 1 Circuit Performance P0AC0
	{0x0B, 0xC1},//BPCM Hybrid Battery Pack Cooling Fan Supply Voltage Circuit High   
	{0x0A, 0x81},//BPCM Hybrid Battery Pack Cooling Fan Performance Fail
	{0xD1, 0xC4},//BPCM Lost Communication With BTM  //new
//10
	{0xD1, 0xC5},//BPCM Lost Communication With OBCM //new
	{0x1A, 0x25},//BPCM Hybrid Battery Contactors State Performance
	{0x0B, 0xBD},//BPCM Hybrid Battery Pack Voltage Variation Exceeded Limit
	{0x0A, 0x80},//BPCM Replace Hybrid Battery Pack
	//TBD temp setting
//	{0x1C, 0xEF},//BPCM Hybrid Battery Pack Cooling Fan Supply Voltage Circuit Low
	{0x0B, 0xC0},//BPCM Hybrid Battery Pack Cooling Fan Supply Voltage Circuit Low
	{0x1A, 0x05},//BPCM Random Access Memory (RAM)
	{0x1A, 0x06},//BPCM Read Only Memory (ROM)
	{0x1A, 0x01},//BPCM EEPROM Failure
	{0x0A, 0x1F},//BPCM Internal Performance
	{0x1C, 0xCF},//BPCM Module1 Failure
//20
	{0x1C, 0xD0},//BPCM Module2 Failure
	{0x1C, 0xD1},//BPCM Module3 Failure
	{0x1C, 0xD2},//BPCM Module4 Failure
	{0x1C, 0xD3},//BPCM Module5 Failure
	{0x1C, 0xD4},//BPCM Module6 Failure
	{0x1C, 0xD5},//BPCM Module7 Failure
	{0x1C, 0xD6},//BPCM Module8 Failure
	//TBD all temp setting
	{0x1D, 0x0F},//Module 1  Sensor Failure
	{0x1D, 0x10},//Module 2  Sensor Failure
	{0x1D, 0x11},//Module 3  Sensor Failure
//30
	{0x1D, 0x12},//Module 4  Sensor Failure
	{0x1D, 0x13},//Module 5  Sensor Failure
	{0x1D, 0x14},//Module 6  Sensor Failure
	{0x1D, 0x15},//Module 7  Sensor Failure
	{0x1D, 0x16},//Module 8  Sensor Failure
	{0xD1, 0xFC},//CAN Communication Loss - Module
	{0xD1, 0xFD},//CAN Communication Loss - HVB
//	{0x1E, 0xD4},//Charger Voltage Fail
	{0x1E, 0x04},//Charger Voltage Fail   changed by Amanda for requirement from Deidra chrysler
	{0x1C, 0xF7},//Voltage Fail-Heater Fuse Fail
	{0x1E, 0x07},//Voltage Fail-Charger Fuse Fail
//40	
	{0x0D, 0x14},//Negative Charging Contactor Output –  shorted load
	{0x0D, 0x13},//Negative Charging Contactor Output – short to ground/open load
	{0x0D, 0x0D},//Postive Charging Contactor Output –  shorted load
	{0x0D, 0x0C},//Postive Charging Contactor Output – short to ground/open load
	{0x0C, 0x44},//Coolant inlet sensor Measurement – Out of Range - Low
	{0x0C, 0x45},//Coolant inlet sensor Measurement – Out of Range - High
	{0x0C, 0x43},//Coolant inlet sensor Measurement – Rationality
	{0x0C, 0xD7},//Coolant outlet sensor Measurement – Out of Range - Low
	{0x0C, 0xD8},//Coolant outlet sensor Measurement – Out of Range - High
	{0x0C, 0xD6},//Coolant outlet sensor Measurement – Rationality

//50
	{0x0C, 0x32},//HV Battery Cooling System Performance
//51
//Non OBD Pcode
	{0x1A, 0x0D},//BPCM System Voltage High  P1A0D
	{0x1A, 0x0C},//BPCM System Voltage Low  P1A0C
	{0xD8, 0x66},//BPCM Lost Communication With Hybrid Gateway Module?
	{0xD8, 0x86},//BPCM Lost Communication With ECM/PCM
	//TBD
#if 1//Added by Amanda August 5
	{0x0A, 0xE6},//Pre-charge Contactor Output – shorted load//Hybrid Battery Precharge Contactor Control Circuit Low
#else
	{0x1C, 0xF0},//Pre-charge Contactor Output – shorted load
#endif
	{0x1A, 0x0B},//BPCM Hybrid High Speed Communication Enable Circuit High 
	{0x1A, 0x0F},//BPCM Hybrid Accessory Wake-up Circuit High
	{0x1A, 0x0E},//BPCM Hybrid Accessory Wake-up Circuit Low
	{0x1A, 0xE5},//BPCM Hybrid Battery Power Off Circuit
//60
	//TBD
	{0x1E, 0x05},//Hybrid Battery Charger Contactor Circuit Stuck Closed
	{0x0A, 0xE7},//BPCM Hybrid Battery Precharge Contactor Control Circuit High
	{0x0A, 0xDB},//BPCM Hybrid Battery Positive Contactor Control Circuit Low 
	{0x0A, 0xDC},//BPCM Hybrid Battery Positive Contactor Control Circuit High
	{0x0A, 0xA1},//BPCM Hybrid Battery Positive Contactor Circuit Stuck Closed -->WELD
	{0x0A, 0xDF},//BPCM Hybrid Battery Negative Contactor Control Circuit Low
	{0x0A, 0xE0},//BPCM Hybrid Battery Negative Contactor Control Circuit High
	{0x0C, 0x76},//BPCM HV System Discharge Time Too Long
	{0x1A, 0xE3},//BPCM High Voltage System Interlock Circuit Low
	{0x1A, 0xE4},//BPCM High Voltage System Interlock Circuit High
//70
	{0x1A, 0xE2},//BPCM High Voltage System Interlock Circuit
	{0x1A, 0xE7},//BPCM Hybrid Battery Voltage System Isolation Circuit
	{0x1A, 0x20},//BPCM Hybrid Battery System Precharge Time Too Long
	//TBD
	{0x1C, 0x48},//Open Cable Detection Failure
	{0x1A, 0x4E},//Battery Module – Over Voltage P1A4E
	{0x1A, 0x1F},//Battery Module – Under Voltage P1A1F
//	{0x1A, 0xBF},//BPCM Hybrid Battery Pack Undertemperature?
	{0x1A, 0xBE},//BPCM Hybrid Battery Pack Overtemperature
	{0x0C, 0x77},//BPCM HV System Precharge Time Too Short
#if 1//Added by Amanda August 5
	{0x17, 0x58},//Voltage Fail-Battery Postive to Ground
	{0x17, 0x58},//Voltage Fail-Battery Negative to Ground
#else
	{0x1C, 0xF3},//Voltage Fail-Battery Postive to Ground
	{0x1C, 0xF4},//Voltage Fail-Battery Negative to Ground
#endif
//80
	{0xCA, 0x11},//backup 12v wrong
//81	
	{0xCA, 0x12},//HVB SW Batt SW
	{0xCA, 0x13},//BPCM Boot
	{0x0A, 0x95},//Battery fuse out of range
#if 1//Added by Amanda August 5
	{0x17, 0x58},//Battery voltage sensor
	{0x17, 0x58},//Charger positive to bus negative
	{0x17, 0x58},//Charger negative to bus positive
#else
	{0x1C, 0xF5},//Battery voltage sensor
#endif
	
//85
};
//static const UINT8 uaPcodeIndexTbl[PCODE_SIZE] = {
UINT8 uaPcodeIndexTbl[PCODE_SIZE] = {
	DTC_CAN_COMM_LOSS_HCP,				//Battery Energy Control Module Lost Communication With HCP
	DTC_VOL_SENSOR_L,						//Hybrid Battery Pack Voltage Sense 1 Circuit Low
	DTC_VOL_SENSOR_H,						//Hybrid Battery Pack Voltage Sense 1 Circuit High
	DTC_VOL_RATIONALITY,					//Hybrid Battery Pack Voltage Sense 1 Circuit 
	DTC_CURR_SENSOR_L,						//Hybrid Battery Pack Current Sensor 1 Circuit Low
	DTC_CURR_SENSOR_H,						//Hybrid Battery Pack Current Sensor 1 Circuit High
	DTC_CURR_RATIONALITY,					//Hybrid Battery Pack Current Sensor 1 Circuit 
	DTC_FAN_VOL_H,							//Hybrid Battery Pack Cooling Fan Supply Voltage Circuit High
	DTC_FAN_PERFORMANCE_FAIL,				//Hybrid Battery Pack Cooling Fan 1 Control Circuit/Open
	DTC_CAN_COMM_LOSS_BTM,				//BPCM Lost Communication with Battery Thermal Module
//10
	DTC_CAN_COMM_LOSS_OBCM,					//BPCM Lost Communication with On Board Charging Module
	DTC_CONTRACTOR_OPEN_WITHOUT_CMD,			// BPCM Hybrid Battery Contactors State Performance
	DTC_BATTERY_MODULE_VOL_DEVIATION_EOL,			// Hybrid Battery Pack Voltage Variation Exceeded Limit
	DTC_REPLACE_BATTERY_PACK,						// Replace Hybrid Battery Pack
	DTC_FAN_VOL_L,									//BPCM Hybrid Battery Pack Cooling Fan Supply Voltage Circuit Low
	DTC_CONTROLLER_RAM_ERROR,						// Battery Energy Control Module Random Access Memory (RAM)
	DTC_CONTROLLER_ROM_ERROR,						//Battery Energy Control Module Read Only Memory (ROM)
	DTC_CONTROLLER_EEPROM_ERROR,					//Battery Energy Control Module EEPROM Failure
	DTC_MICROCONTROLLER_ERROR,					//Battery Energy Control Module Internal Performance
	DTC_MODULE_1_FAILURE,							//HV Battery Module 1 Failure
//20
	DTC_MODULE_2_FAILURE,							//HV Battery Module 2 Failure
	DTC_MODULE_3_FAILURE,							//HV Battery Module 3 Failure
	DTC_MODULE_4_FAILURE,							//HV Battery Module 4 Failure
	DTC_MODULE_5_FAILURE,							//HV Battery Module 5 Failure
	DTC_MODULE_6_FAILURE,							//HV Battery Module 6 Failure
	DTC_MODULE_7_FAILURE,							//HV Battery Module 7 Failure
	DTC_MODULE_8_FAILURE,							//HV Battery Module 8 Failure
	DTC_MODULE_1_SENSOR_FAILURE,					//Module 1  Sensor Failure
	DTC_MODULE_2_SENSOR_FAILURE,					//Module 2  Sensor Failure
	DTC_MODULE_3_SENSOR_FAILURE,					//Module 3  Sensor Failure
//30
	DTC_MODULE_4_SENSOR_FAILURE,					//Module 4  Sensor Failure
	DTC_MODULE_5_SENSOR_FAILURE,					//Module 5  Sensor Failure
	DTC_MODULE_6_SENSOR_FAILURE,					//Module 6  Sensor Failure
	DTC_MODULE_7_SENSOR_FAILURE,					//Module 7  Sensor Failure
	DTC_MODULE_8_SENSOR_FAILURE,					//Module 8  Sensor Failure
	DTC_CAN_COMM_LOSS_MODULE,					//CAN Communication Loss - Module
	DTC_CAN_COMM_LOSS_HVB,						//CAN Communication Loss - HVB
	DTC_CHARGER_VOLTAGE_FAIL,						//Charger Voltage Fail
	DTC_HEATER_FUSE_FAIL,							//Voltage Fail-Heater Fuse Fail
	DTC_CHARGER_FUSE_FAIL,							//Voltage Fail-Charger Fuse Fail
//40
	DTC_NEGATIVE_CHARGING_CONTACTOR_SHORTED_LOAD,		//Negative Charging Contactor Output –  shorted load
	DTC_NEGATIVE_CHARGING_CONTACTOR_SHORTG_OPENLOAD,	//Negative Charging Contactor Output – short to ground/open load
	DTC_POSITIVE_CHARGING_CONTACTOR_SHORTED_LOAD,		//Postive Charging Contactor Output –  shorted load
	DTC_POSITIVE_CHARGING_CONTACTOR_SHORTG_OPENLOAD,	//Postive Charging Contactor Output – short to ground/open load
	DTC_COOLANT_INLET_OUT_OF_RANGE_LOW,					//Coolant Temperature Sensor Inlet Measurement – Out of Range - Low
 	DTC_COOLANT_INLET_OUT_OF_RANGE_HIGH,					//Coolant Temperature Sensor Inlet Measurement – Out of Range - High
	DTC_COOLANT_INLET_RATIONALITY,							//Coolant Temperature Sensor Inlet Measurement – Rationality
	DTC_COOLANT_OUTLET_OUT_OF_RANGE_LOW,					//Coolant Temperature Sensor Outlet Measurement – Out of Range - Low
	DTC_COOLANT_OUTLET_OUT_OF_RANGE_HIGH,				//Coolant Temperature Sensor Outlet Measurement – Out of Range - High
	DTC_COOLANT_OUTLET_RATIONALITY,						//Coolant Temperature Sensor Outlet Measurement – Rationality
//50
	DTC_COOLING_SYS_PERFORMANCE,							//HV Battery Cooling System Performance
//51
//Non OBD Pcode
 	DTC_12V_VOL_H,							//Battery Energy Control Module System Voltage High
	DTC_12V_VOL_L,							//Battery Energy Control Module System Voltage Low
	DTC_CAN_COMM_LOSS_FCM,				//Battery Energy Control Module Lost Communication With Hybrid Gateway Module
	DTC_CAN_COMM_LOSS_ECM,				//Battery Energy Control Module Lost Communication With ECM/PCM
#if 1//Added by Amanda August 5
	DTC_PCR_CONTACTOR_CIRCUIT_LOW,		//Hybrid Battery Precharge Contactor Control Circuit low
#else
	DTC_PCR_CONTACTOR_SHORTED_LOAD,		//Pre-charge Contactor Output - shorted load
#endif
 	DTC_HS_COMM_ENABLE_SIGNAL_H,			//BPCM Hybrid High Speed Communication Enable Circuit High 
	DTC_HYBRID_ACC_SIGNAL_H,				//BPCM Hybrid Accessory Wake-up Circuit High
	DTC_HYBRID_ACC_SIGNAL_L,					//BPCM Hybrid Accessory Wake-up Circuit Low
	DTC_CONTACTOR_CMD_OUT_OF_RANGE,			//Battery Energy Control Module Hybrid Battery Power Off Circuit
//60
	DTC_CHG_CONTRACTOR_WELD,					//Hybrid Battery Charger Contactor Circuit Stuck Closed
#if 1//Added by Amanda August 5
	DTC_PCR_CONTACTOR_CIRCUIT_HIGH,			// Hybrid Battery Precharge Contactor Control Circuit High
#else
	DTC_PCR_CONTACTOR_SHORTG_OPENLOAD,			// Hybrid Battery Precharge Contactor Control Circuit High
#endif
#if 1//Added by Amanda August 5
	DTC_DCHG_CONTRACTOR_POS_CIRCUIT_LOW,		// Hybrid Battery Positive Contactor Control Circuit Low
	DTC_DCHG_CONTRACTOR_POS_CIRCUIT_HIGH,	// Hybrid Battery Positive Contactor Control Circuit High
#else
	DTC_DCHG_CONTRACTOR_POS_SHORTED_LOAD,		// Hybrid Battery Positive Contactor Control Circuit Low
	DTC_DCHG_CONTRACTOR_POS_SHORTG_OPENLOAD,	// Hybrid Battery Positive Contactor Control Circuit High
#endif
	DTC_CONTRACTOR_WELD,							// Hybrid Battery Positive Contactor Circuit Stuck Closed
#if 1//Added by Amanda August 5
	DTC_DCHG_CONTRACTOR_NEG_CIRCUIT_LOW,		// Hybrid Battery Negative Contactor Control Circuit Low
	DTC_DCHG_CONTRACTOR_NEG_CIRCUIT_HIGH,	// Hybrid Battery Negative Contactor Control Circuit High
#else
	DTC_DCHG_CONTRACTOR_NEG_SHORTED_LOAD,		// Hybrid Battery Negative Contactor Control Circuit Low
	DTC_DCHG_CONTRACTOR_NEG_SHORTG_OPENLOAD,	// Hybrid Battery Negative Contactor Control Circuit High
#endif
	DTC_ACTIVE_DISCHARGE_FAULT,					// HV Battery System Discharge Time Too Long - Active Discharge
	DTC_HVIL_OUT_OF_RANGE_L,						//Battery Energy Control Module High Voltage System Interlock Circuit Low
	DTC_HVIL_OUT_OF_RANGE_H,						//Battery Energy Control Module High Voltage System Interlock Circuit High
//70
	DTC_HVIL_FAULT,									// Battery Energy Control Module High Voltage System Interlock Circuit
	DTC_HV_ISOLATION_FAULT,							// Battery Energy Control Module Hybrid Battery Voltage System Isolation Circuit
	DTC_PRE_CHARGE_TOO_SLOW,						// Battery Energy Control Module Hybrid Battery System Precharge Time Too Long
	DTC_OPEN_CABLE_FAILURE,							//Open Cable Detection Failure
	DTC_BATTERY_MODULE_OV,							//Battery Energy Control Module HV Battery System Voltage High
	DTC_BATTERY_MODULE_UV,							//Battery Energy Control Module HV Battery System Voltage Low
//	DTC_BATTERY_UNDER_TEMPERATURE,				// Battery – Under temperature
	DTC_BATTERY_OVER_TEMPERATURE,					//Battery Energy Control Module Hybrid Battery Pack Overtemperature
	DTC_PRE_CHARGE_FAST, 							//HV Battery System Precharge Time Too Short
	DTC_POSBAT_TO_GROUND_VOL_FAIL,				//Voltage Fail-Battery Postive to Ground
	DTC_NEGBAT_TO_GROUND_VOL_FAIL,				//Voltage Fail-Battery Negative to Ground
//80
	DTC_BACKUP12V_FAIL,				//Voltage Fail-Battery Negative to Ground
//81
	DTC_HVB_BATT_SW_FAIL,				//hvb or batt sw
	DTC_BPCM_BOOT_SW_FAIL,				//BPCM boot SW
	DTC_FUSE_OUT_OF_RANGE,//84
	DTC_BATT_SENSOR_FAIL,//85 //Voltage Fail-battery sensor
#if 1//Added by Amanda August 5
	DTC_CHGPOS_TO_BUSNEG_VOL_FAIL,//Voltage Fail-Charger positive to bus negative
	DTC_CHGNEG_TO_BUSPOS_VOL_FAIL,//Voltage Fail-Charger negative to bus positive
#endif
	
};	
extern void initDTCTable(void);
UINT8 sub_uiClearFault(void);
/***************************************************/
/*******	FUNCTIONS BODY SECTION                	****/
/***************************************************/
//for app parts
/*******************************************************************************************************/
//[Negative Response Trouble Code] Supported
// NRTC_SUB_FUN_NOTSUPPORTED		0x12	implemented	
// NRTC_INCORR_MSG_LENGTH			0x13	implemented
// NRTC_CON_NOT_CORRECT			0x22	implemented
// NRTC_REQ_OUT_OF_RANGE			0x31	implemented
/*******************************************************************************************************/

//0x28
void nvCommControl(void)
{
	if(uiGetMode(DATA_RV_MODE)==NORMAL_PROCESSING){
		if((UDSRvDataPara.uiSUB[0] ==0x01)||(UDSRvDataPara.uiSUB[0]==0x81)){
				if(UDSRvDataPara.uiSUB[1] ==0x01){
					if(uiGetAppDiagSessionMode()!=DEFAULT_DIAG_SESSION){
						uvSetCommType(COMM_ENBRX_DISTX);
					}
					else{
#if 0
						nvSendUDSNegResp(COMMUNICATION_CONTROL,NRTC_CON_NOT_CORRECT);
						return;
#else
						uvSetCommType(COMM_ENBRX_DISTX);
#endif
					}
				}
				else{
					nvSendUDSNegResp(COMMUNICATION_CONTROL,NRTC_REQ_OUT_OF_RANGE);
					return;
				}
		}
		else if((UDSRvDataPara.uiSUB[0] ==0x00)||(UDSRvDataPara.uiSUB[0]==0x80)){
				if(UDSRvDataPara.uiSUB[1] ==0x01){
					if(uiGetAppDiagSessionMode()!=DEFAULT_DIAG_SESSION){
						uvSetCommType(COMM_ENBRX_ENBTX);
					}
					else{
#if 0
						nvSendUDSNegResp(COMMUNICATION_CONTROL,NRTC_CON_NOT_CORRECT);
						return;
#else
						uvSetCommType(COMM_ENBRX_ENBTX);
#endif
					}
				}
				else{
					nvSendUDSNegResp(COMMUNICATION_CONTROL,NRTC_REQ_OUT_OF_RANGE);
					return;
				}
		}
		if((UDSRvDataPara.uiSUB[0]==0x01)||(UDSRvDataPara.uiSUB[0]==0x00)){
			nvSendUDSPosResp(COMMUNICATION_CONTROL, 0x00,0x00);
		}
	}
}
/*******************************************************************************************************/
//[Negative Response Trouble Code] Supported
// NRTC_SUB_FUN_NOTSUPPORTED		0x12	implemented
// NRTC_INCORR_MSG_LENGTH			0x13	implemented
// NRTC_CON_NOT_CORRECT			0x22	implemented
// NRTC_REQ_OUT_OF_RANGE			0x31	N/A
/*******************************************************************************************************/
//0x85
void nvControlDtcSetting(void)
{
	if(uiGetMode(DATA_RV_MODE)==NORMAL_PROCESSING){
		if((UDSRvDataPara.uiSUB[0] ==0x01)||(UDSRvDataPara.uiSUB[0] ==0x81)){
			if(uiGetAppDiagSessionMode()!=DEFAULT_DIAG_SESSION){
				nvSetDTCType(DTC_TYPE_ON);
			}
			else{
#if 0
				nvSendUDSNegResp(COMMUNICATION_CONTROL,NRTC_CON_NOT_CORRECT);
				return;
#else
				nvSetDTCType(DTC_TYPE_ON);
#endif
			}
		}
		else if((UDSRvDataPara.uiSUB[0] ==0x02)||(UDSRvDataPara.uiSUB[0]==0x82)){
			if(uiGetAppDiagSessionMode()!=DEFAULT_DIAG_SESSION){
				nvSetDTCType(DTC_TYPE_OFF);
			}
			else{
#if 0
				nvSendUDSNegResp(COMMUNICATION_CONTROL,NRTC_CON_NOT_CORRECT);
				return;
#else
				nvSetDTCType(DTC_TYPE_OFF);
#endif
			}
		}
		if((UDSRvDataPara.uiSUB[0]==0x01)||(UDSRvDataPara.uiSUB[0]==0x02)){
			nvSendUDSPosResp(CONTROL_DTC_SETTING,  0x00,0x00);
		}
	}
}

/*******************************************************************************************************/
//[Negative Response Trouble Code] Supported
// NRTC_SUB_FUN_NOTSUPPORTED		0x12	implemented		check ok	
// NRTC_INCORR_MSG_LENGTH			0x13	implemented		check ok
// NRTC_CON_NOT_CORRECT			0x22	implemented		check ok		
/*******************************************************************************************************/
//0x10
void nvDiagSessionChange(void)
{
	if(uiGetMode(DATA_RV_MODE)==NORMAL_PROCESSING){
		if((UDSRvDataPara.uiSUB[0] ==0x01)||(UDSRvDataPara.uiSUB[0] ==0x81)){
			uvSetAppDiagSessionMode(DEFAULT_DIAG_SESSION);
		}
		else if((UDSRvDataPara.uiSUB[0]==0x02)||(UDSRvDataPara.uiSUB[0]==0x82)){
				if(uiChkProgPrecFromApp()==0){
					uvSetAppDiagSessionMode(PROGRAMMING_DIAG_SESSION);
					if(uiRequestID!=DIAG_PROG_SESSION_REQ_FROM_APP){
						uvSetReqInfo(DIAG_PROG_SESSION_REQ_FROM_APP);
//why BPCM always send a negtive response before it reset?
						nvSendUDSNegResp(START_DIAGNOSTIC_SESSION,NRTC_SERCORRECT_RSPPENDING);

					}
					return;
				}
				else{
					nvSendUDSNegResp(START_DIAGNOSTIC_SESSION,NRTC_CON_NOT_CORRECT);
					return;
				}
		}
		else if((UDSRvDataPara.uiSUB[0] ==0x03)||(UDSRvDataPara.uiSUB[0] ==0x83)){
			uvSetAppDiagSessionMode(EXTENDED_DIAG_SESSION);
			uvStartTimer(TIMER_PERIOD_5S);
		}
		if((UDSRvDataPara.uiSUB[0]==0x01)||(UDSRvDataPara.uiSUB[0]==0x02)||(UDSRvDataPara.uiSUB[0]==0x03)){
		// Scaling Application Layer Timing parameter 4 bytes		
			nvSendUDSPosResp(START_DIAGNOSTIC_SESSION,  0x00,0x00);
		}
	}
}

/*******************************************************************************************************/
//[Negative Response Trouble Code] Supported
// NRTC_SUB_FUN_NOTSUPPORTED		0x12	implemented		check ok
// NRTC_INCORR_MSG_LENGTH			0x13	implemented		check ok
// NRTC_CON_NOT_CORRECT			0x22
/*******************************************************************************************************/
//0x11
void nvEcuReset(void)
{
	if(uiGetMode(DATA_RV_MODE)==NORMAL_PROCESSING){
		if(UDSRvDataPara.uiSUB[0]==0x01){
			//Hard Reset
//after version 112503 start
			HCPCmd.BMCPCANWorkEnableSWReceivedInKeyCycle=TRUE;
//after version 112503 end
			uvSetReqInfo(RESET_REQ_FROM_APP);
			nvSendUDSNegResp(ECU_RESET,NRTC_SERCORRECT_RSPPENDING);
			
		}
		else if(UDSRvDataPara.uiSUB[0]==0x81){
			//Hard Reset no resp
//after version 112503 start
			HCPCmd.BMCPCANWorkEnableSWReceivedInKeyCycle=TRUE;
//after version 112503 end
			uvSetReqInfo(RESET_REQ_FROM_APP);
			nvSendUDSNegResp(ECU_RESET,NRTC_SERCORRECT_RSPPENDING);
		}
	}
}

/*******************************************************************************************************/
//[Negative Response Trouble Code] Supported
// NRTC_SUB_FUN_NOTSUPPORTED		0x12	implemented		check ok
// NRTC_INCORR_MSG_LENGTH			0x13	implemented		check ok
// NRTC_CON_NOT_CORRECT			0x22	implemented		check ok
// NRTC_REQ_OUT_OF_RANGE			0x31	implemented		check ok
// NRTC_SECURITY_ACCESS_DENIED	0x33	implemented		check ok
// NRTC_GEN_PRO_FAIL				0x72	implemented		check ok
/*******************************************************************************************************/
//0x31
void nvRoutineControl(void)
{

	uiCheckResult=0;
	
//	if(uiGetAppDiagSessionMode()!=DEFAULT_DIAG_SESSION){
		if(uiGetMode(DATA_RV_MODE)==NORMAL_PROCESSING){

#if 0			
			if( ((UDSRvDataPara.uiSUB[0]==0x01)||(UDSRvDataPara.uiSUB[0]==0x81))&&
				(UDSRvDataPara.uiSUB[1] ==0xFF)&&(UDSRvDataPara.uiSUB[2] ==0x03) ){
				//Check Programming Preconditions -pos Resp
				uiCheckResult=uiChkProgPrecFromApp();
			}
			else{
				if((UDSRvDataPara.uiSUB[0] ==0x01)||(UDSRvDataPara.uiSUB[0]==0x81)){
					 if((UDSRvDataPara.uiSUB[1] ==0xFF)&&(UDSRvDataPara.uiSUB[2]==0x05)){
						uvSetFailSafeMode(DIS_FAIL_SAFE_REACTION);
						uiCheckResult=0;
					}
					else if((UDSRvDataPara.uiSUB[1] ==0x02)&&(UDSRvDataPara.uiSUB[2]==0x12)){
						uiCheckResult=uiResetVIN();
					}
					else{
						nvSendUDSNegResp(ROUTINE_CONTROL,NRTC_REQ_OUT_OF_RANGE);
						return;
					}
				}
				else if((UDSRvDataPara.uiSUB[0] ==0x02)||(UDSRvDataPara.uiSUB[0] ==0x82)){
					 if((UDSRvDataPara.uiSUB[1]==0xFF)&&(UDSRvDataPara.uiSUB[2]==0x05)){
					 	uvSetFailSafeMode(ENB_FAIL_SAFE_REACTION);
						uiCheckResult=0;
					 }
					else{
						nvSendUDSNegResp(ROUTINE_CONTROL,NRTC_REQ_OUT_OF_RANGE);
						return;
					}
				}
				
			}
#endif
			if((UDSRvDataPara.uiSUB[0] ==0x01)||(UDSRvDataPara.uiSUB[0]==0x81)){
				 if((UDSRvDataPara.uiSUB[1] ==0xFF)&&(UDSRvDataPara.uiSUB[2]==0x03)){
					//Check Programming Preconditions -pos Resp
					uiCheckResult=uiChkProgPrecFromApp();
				 }
				 else if((UDSRvDataPara.uiSUB[1] ==0xFF)&&(UDSRvDataPara.uiSUB[2]==0x05)){
					uvSetFailSafeMode(DIS_FAIL_SAFE_REACTION);
					uiCheckResult=0;
				 }
				 else if((UDSRvDataPara.uiSUB[1] ==0x03)&&
				 	((UDSRvDataPara.uiSUB[2]==0x06)||(UDSRvDataPara.uiSUB[2]==0x07)
				 	||(UDSRvDataPara.uiSUB[2]==0x08)||(UDSRvDataPara.uiSUB[2]==0x09)
					||(UDSRvDataPara.uiSUB[2]==0x10)
				 )){
					//clear faults
					uiCheckResult=sub_uiClearFault();
					if((uiCheckResult==TRUE)&&(UDSRvDataPara.uiSUB[0] ==0x01)){
						nvSendUDSPosResp(ROUTINE_CONTROL, &uaBuffer[0],uiSendSize);
						return;
					}
					else{
						uiCheckResult=0;
						return;
					}
				 }
				else if((UDSRvDataPara.uiSUB[1] ==0x02)&&(UDSRvDataPara.uiSUB[2]==0x12)){
					uiCheckResult=uiResetVIN();
				}
				else{
					nvSendUDSNegResp(ROUTINE_CONTROL,NRTC_REQ_OUT_OF_RANGE);
					return;
				}
			}
			else if((UDSRvDataPara.uiSUB[0] ==0x02)||(UDSRvDataPara.uiSUB[0] ==0x82)){
				 if((UDSRvDataPara.uiSUB[1]==0xFF)&&(UDSRvDataPara.uiSUB[2]==0x05)){
				 	uvSetFailSafeMode(ENB_FAIL_SAFE_REACTION);
					uiCheckResult=0;
				 }
				else{
					nvSendUDSNegResp(ROUTINE_CONTROL,NRTC_REQ_OUT_OF_RANGE);
					return;
				}
			}
			if(uiCheckResult==0){
				//routine status record
				 if((UDSRvDataPara.uiSUB[0] ==0x01)||(UDSRvDataPara.uiSUB[0] ==0x02)){
					nvSendUDSPosResp(ROUTINE_CONTROL, &uiCheckResult,1);
				 }
			}
			else{
				nvSendUDSNegResp(ROUTINE_CONTROL,NRTC_GEN_PRO_FAIL);
			}
		}
//	}
//	else{
//		nvSendUDSNegResp(ROUTINE_CONTROL,NRTC_CON_NOT_CORRECT);
//	}
}

/*******************************************************************************************************/
//[Negative Response Trouble Code] Supported
// NRTC_SUB_FUN_NOTSUPPORTED		0x12	implemented	check OK
// NRTC_INCORR_MSG_LENGTH			0x13	implemented	check OK
/*******************************************************************************************************/
//0x3E
void nvTesterPresent(void)
{
	if(uiGetMode(DATA_RV_MODE)==NORMAL_PROCESSING){
		if(UDSRvDataPara.uiSUB[0]==0x00){
			uvStartTimer(TIMER_PERIOD_5S);
			//tester present pos response
//Modified by Amanda for DTC inhibited problem
			HCPCmd.BMCToolsPresentTick=TickGet(); 
			nvSendUDSPosResp(TESTER_PRESENT,  0x00,0x00);
		}
		else if(UDSRvDataPara.uiSUB[0]==0x80){
			uvStartTimer(TIMER_PERIOD_5S);
//Modified by Amanda for DTC inhibited problem
			HCPCmd.BMCToolsPresentTick=TickGet(); 
			//tester present  no pos response
		}
		else  if(UDSRvDataPara.uiSUB[0]==0x02){
		}
		else{
			nvSendUDSNegResp(TESTER_PRESENT,NRTC_SUB_FUN_NOTSUPPORTED);
		}
	}
}
/*******************************************************************************************************/
//[Negative Response Trouble Code] Supported
// NRTC_INCORR_MSG_LENGTH			0x13             implemented	check ok
// NRTC_CON_NOT_CORRECT			0x22
// NRTC_REQ_OUT_OF_RANGE			0x31             implemented	check ok
// NRTC_SECURITY_ACCESS_DENIED	0x33		?
/*******************************************************************************************************/
//0x22
void nvReadData(void)
{
	UINT8 uiReadID=0xff;

	uiReadID=nuAppGetReadID();
	if(uiReadID!=0xff){
		nvSendUDSPosResp(READ_DATA_BY_IDENTIFIER, &uaBuffer[0],uiSendSize);
	}
	else{
		nvSendUDSNegResp(READ_DATA_BY_IDENTIFIER,NRTC_REQ_OUT_OF_RANGE);
	}
}


/*******************************************************************************************************/
//[Negative Response Trouble Code] Supported
// NRTC_INCORR_MSG_LENGTH			0x13	implemented		check ok	
// NRTC_CON_NOT_CORRECT			0x22	implemented		check ok
// NRTC_REQ_OUT_OF_RANGE			0x31	implemented		check ok
// NRTC_SECURITY_ACCESS_DENIED	0x33	implemented		check ok
/*******************************************************************************************************/
//0x2E
void nvWriteData(void)
{
	UINT8 uiStoreFlag=0xff;
	IOControlByte DataTemp;

	
//	if(uiGetAppDiagSessionMode()!=DEFAULT_DIAG_SESSION){
		if(uiGetMode(DATA_RV_MODE)==NORMAL_PROCESSING){
			if((UDSRvDataPara.uiSUB[0]==0xF1)&&(UDSRvDataPara.uiSUB[1]==0x5A)){
				//Finger Print
#if 0
				if(uiGetSecurityMode()==SECURITY_KEY_VALID){
#endif
#if (REAL_ENV)
					if(nuReadFromEeprom(PROGRAMMING_ATTEMPT_COUNTER_ADDR)>=MAX_REPROGRAMMING_CYCLES){
						nvSendUDSNegResp(WRITE_DID,NRTC_CON_NOT_CORRECT);
						return;
					}
#endif
					uiStoreFlag=WRITE_DID_FINGERPRINT;
#if 0
				}
				else{
					nvSendUDSNegResp(WRITE_FINGERPRINT,NRTC_SECURITY_ACCESS_DENIED);
				}
#endif
			}
			else if((UDSRvDataPara.uiSUB[0]==0x01)&&(UDSRvDataPara.uiSUB[1]==0x03)){
				//VIN ODOMETER counter
				uiStoreFlag=WRITE_DID_VIN_ODO_COUNTER;
			}
			else if((UDSRvDataPara.uiSUB[0]==0x01)&&(UDSRvDataPara.uiSUB[1]==0x04)){
				// ODOMETER
				uiStoreFlag=WRITE_DID_ODOMETER;
			}
			else if((UDSRvDataPara.uiSUB[0]==0x01)&&(UDSRvDataPara.uiSUB[1]==0x07)){
				// RESPONSE ON EVENT-LIGHT ACTIVATION STATE
				uiStoreFlag=WRITE_DID_RSP_ON_EVENT;
			}
			else if((UDSRvDataPara.uiSUB[0]==0xF1)&&(UDSRvDataPara.uiSUB[1]==0x32)){
				// ECU Part Number
				uiStoreFlag=WRITE_DID_ECU_PART_NUMBER;
			}
			else if((UDSRvDataPara.uiSUB[0]==0xF1)&&(UDSRvDataPara.uiSUB[1]==0xA0)){
				//VIN Current
				uiStoreFlag=WRITE_DID_VIN_CURRENT;
			}
			else if((UDSRvDataPara.uiSUB[0]==0x24)&&(UDSRvDataPara.uiSUB[1]==0x11)){
				//SOC
				uiStoreFlag=WRITE_DID_SOC;
			}
			else if((UDSRvDataPara.uiSUB[0]==0xA6)&&(UDSRvDataPara.uiSUB[1]==0x01)){
				//write hw ID
				uiStoreFlag=WRITE_DID_HW_ID;
			}
			else if((UDSRvDataPara.uiSUB[0]==0xA0)&&(UDSRvDataPara.uiSUB[1]==0x04)){
				//Diagnostic contactor Dis/Enabler(Engineering -Manufacturing)
				uiStoreFlag=WRITE_DID_CONTACTOR_ENABLER;
			}
			else if((UDSRvDataPara.uiSUB[0]==0xA0)&&(UDSRvDataPara.uiSUB[1]==0x06)){
				//MEC Counter 
				uiStoreFlag=WRITE_DID_MEC_COUNTER;
			}
			else if((UDSRvDataPara.uiSUB[0]==0xA0)&&(UDSRvDataPara.uiSUB[1]==0x24)){
				//EOL SCRATCH PAD
				uiStoreFlag=WRITE_DID_EOL_SCRATCH_PAD;
			}
			else if((UDSRvDataPara.uiSUB[0]==0xA0)&&(UDSRvDataPara.uiSUB[1]==0x29)){
				//Write hidden SOC
				uiStoreFlag=WRITE_DID_HIDDEN_SOC;
			}
			else if((UDSRvDataPara.uiSUB[0]==0xA0)&&(UDSRvDataPara.uiSUB[1]==0x30)){
				//Write hidden SOC
				uiStoreFlag=WRITE_DID_CANLABLE_PACK_NO;
			}
			else if((UDSRvDataPara.uiSUB[0]==0xA0)&&(UDSRvDataPara.uiSUB[1]==0x31)){
				//Write hidden SOC
				uiStoreFlag=WRITE_DID_FORCE_TO_WRITE;
			}
			//Added by Amanda for DTC Disable/Enable Dec20,2010
			else if((UDSRvDataPara.uiSUB[0]==0xA0)&&(UDSRvDataPara.uiSUB[1]==0x40)){
				//Write DTC enabler
				uiStoreFlag=WRITE_DID_DTC_ENABLER;
			}
			
			if(uiStoreFlag<WRITE_DATA_BY_ID_MAX){
				if(uiStoreFlag==WRITE_DID_FINGERPRINT){
					//Write Fingerprint to EEPROM
					nvStoreFingerPrintData();
				}
				else{
					nvStoreDID(uiStoreFlag);
				}
				nvSendUDSPosResp(WRITE_DID,  0x00,0x00);
			}
			else{
				nvSendUDSNegResp(WRITE_DID,NRTC_REQ_OUT_OF_RANGE);
			}
		}
//	}
//	else{
//		nvSendUDSNegResp(WRITE_DID,NRTC_CON_NOT_CORRECT);
//	}
}	


/*******************************************************************************************************/
//[Negative Response Trouble Code] Supported
// NRTC_INCORR_MSG_LENGTH			0x13	implemented		check ok	
// NRTC_CON_NOT_CORRECT			0x22	?
// NRTC_REQ_OUT_OF_RANGE			0x31	implemented		check ok
/*******************************************************************************************************/
//0x14
void nvClearDiagInfo(void)
{
	UINT16	i;

	
	if(uiGetMode(DATA_RV_MODE)==NORMAL_PROCESSING){
		if((UDSRvDataPara.uiSUB[0]==0xFF)&&(UDSRvDataPara.uiSUB[1]==0xFF)&&(UDSRvDataPara.uiSUB[2]==0xFF)){
			//All Groups /All DTCs
			//initDTCTable();
			for (i=0;i<DTC_NUMBER_LONG;i++){


#if 1//Added by Amanda August 5
				if(uaStoredDTC[i]!=DTC_NO_STATUS){
					if((DTCLongTable[i].DTCStatus!=DTC_FAILED)
						||(i==DTC_BACKUP12V_FAIL)
						||(i==DTC_BPCM_BOOT_SW_FAIL)
						||(i==DTC_HVB_BATT_SW_FAIL))
						{
							uaStoredDTC[i]=DTC_NO_STATUS;
						}
				}
#else
				uaStoredDTC[i]=DTC_NO_STATUS;
#endif
			}
			writeDTCInfoToEEPROMAll();
			nvSendUDSPosResp(CLEAR_DIAG_INFO,  0x00,0x00);
		}
		else{
			nvSendUDSNegResp(CLEAR_DIAG_INFO,NRTC_REQ_OUT_OF_RANGE);
		}
	}
}
////////////////////////////////////////////////////////
//subfunctions for 0x19
///////////////////////////////////////////////////////
UINT16 sub_uiCheckDTCTbl(UINT8 uiStatusMask){
	StsMask  uiStsMask;
	UINT16 dtcId;
	UINT16 uiMatchedNo=0;
	UINT8 uiStatus;

	uiStsMask.uiDataByte=uiStatusMask;
	//Modified by Amanda for stored DTC

		for(dtcId=0;dtcId<DTC_NUMBER_LONG;dtcId++){	//check id number's max is 128
			if( ( (uiStsMask.ConfirmedDTC==1)&&(uaStoredDTC[dtcId]==DTC_FAILED) )
//				||( (uiStsMask.NotCompleted==1)&&(DTCLongTable[dtcId].DTCStatus==DTC_NO_STATUS) )
				||( (uiStsMask.TestFailed==1)&&(DTCLongTable[dtcId].DTCStatus==DTC_FAILED) ) ){
					uiMatchedNo++;
			}
		}
	return uiMatchedNo;

}


UINT16  sub_uiFilteDTCNo(UINT8 uiStatusMask)
{
	StsMask  uiStsMask;
	UINT16 uiResultNo=0;
	
	uiStsMask.uiDataByte=uiStatusMask;
//	if((uiStsMask.ConfirmedDTC==1)||(uiStsMask.NotCompleted==1)||(uiStsMask.TestFailed==1)){
	if((uiStsMask.ConfirmedDTC==1)||(uiStsMask.TestFailed==1)){
		uiResultNo=sub_uiCheckDTCTbl(uiStatusMask);
	}
	//other bits don't be supported.
	return uiResultNo;
	
}

UINT16 sub_uvGetDTCData(UINT8 uiStatusMask)
{
	StsMask  uiStsMask;
	UINT16 dtcId;
	UINT8 uiStatus;
#if 1//Added by Amanda August 5
	UINT8 uiFirstTimeIn;
	uiFirstTimeIn=0;

#endif
	
	uiStsMask.uiDataByte=uiStatusMask;
		for(dtcId=0,uiIndexLong=0;dtcId<DTC_NUMBER_LONG;dtcId++){	//check id number's max is 160
			if( ( (uiStsMask.ConfirmedDTC==1)&&(uaStoredDTC[dtcId]==DTC_FAILED) )
//				||( (uiStsMask.NotCompleted==1)&&(DTCLongTable[dtcId].DTCStatus==DTC_NO_STATUS) )
				||( (uiStsMask.TestFailed==1)&&(DTCLongTable[dtcId].DTCStatus==DTC_FAILED) ) ){
#if 1//Added by Amanda August 5
					if((dtcId==DTC_BATT_SENSOR_FAIL)
						||(dtcId==DTC_POSBAT_TO_GROUND_VOL_FAIL)
						||(dtcId==DTC_NEGBAT_TO_GROUND_VOL_FAIL)
						||(dtcId==DTC_CHGPOS_TO_BUSNEG_VOL_FAIL)
						||(dtcId==DTC_CHGNEG_TO_BUSPOS_VOL_FAIL))
					{
						if(uiFirstTimeIn==0){
							uiFirstTimeIn=1;
							uaIDLongTbl[uiIndexLong]=dtcId;
							uiIndexLong++;
						}
						else{
						}
					}
					else{
						uaIDLongTbl[uiIndexLong]=dtcId;
						uiIndexLong++;
					}
#else
					uaIDLongTbl[uiIndexLong]=dtcId;
					uiIndexLong++;
#endif
			}
		}
	return (uiIndexLong);

}


UINT8  sub_uiGetPcodeIndex(UINT8 uiDTCID)
{
	UINT8  i;

	for(i=0;i<PCODE_SIZE;i++){
		if(uaPcodeIndexTbl[i]==uiDTCID){
			return i;
		}
	}
	return 0xff;

}
void sub_uvCopyDTCData(UINT8 uiType){

	UINT16  	i,j,k;
	UINT16  	uiDataID;
	UINT16  	uiPcode;
	UINT8 	uiIndex;
	StsMask  uiStsSend;
	for(i=0;i<uiIndexLong;i++){
		if((4+i*4)>=UDS_DATA_TRANSMIT_LEN){
			return;
		}
		uiDataID=uaIDLongTbl[i];
		uiIndex=sub_uiGetPcodeIndex((UINT8)uiDataID);
		uaBuffer[1+i*4]=uaPcodeTbl[uiIndex][0] ;
		uaBuffer[2+i*4]=uaPcodeTbl[uiIndex][1] ;
		uaBuffer[3+i*4]=0 ;//not supported
		uiStsSend.uiDataByte=0;
/*		if((uaBuffer[1+i*4]==0xCA)&&(uaBuffer[2+i*4]==0x11)){
			if(FailureU0A11DByte.bit0==1){
				uaBuffer[3+i*4]=0 ;//backup12v
			}
			else if(FailureU0A11DByte.bit1==1){
				uaBuffer[3+i*4]=1 ;//HVB SW or Batt SW
			}
			else if(FailureU0A11DByte.bit2==1){
				uaBuffer[3+i*4]=2 ;//BPCM boot
			}
		}*/
#if 1//Added by Amanda August 5
	if((uiDataID==DTC_BATT_SENSOR_FAIL)
		||(uiDataID==DTC_POSBAT_TO_GROUND_VOL_FAIL)
		||(uiDataID==DTC_NEGBAT_TO_GROUND_VOL_FAIL)
		||(uiDataID==DTC_CHGPOS_TO_BUSNEG_VOL_FAIL)
		||(uiDataID==DTC_CHGNEG_TO_BUSPOS_VOL_FAIL)
	){
		if((DTCLongTable[DTC_BATT_SENSOR_FAIL].DTCStatus==DTC_FAILED)
			||(DTCLongTable[DTC_POSBAT_TO_GROUND_VOL_FAIL].DTCStatus==DTC_FAILED)
			||(DTCLongTable[DTC_NEGBAT_TO_GROUND_VOL_FAIL].DTCStatus==DTC_FAILED)
			||(DTCLongTable[DTC_CHGPOS_TO_BUSNEG_VOL_FAIL].DTCStatus==DTC_FAILED)
			||(DTCLongTable[DTC_CHGNEG_TO_BUSPOS_VOL_FAIL].DTCStatus==DTC_FAILED)
			){
				uiStsSend.TestFailed=1;
		}
		if(uiType==1){
			if((uaStoredDTC[DTC_BATT_SENSOR_FAIL]==DTC_FAILED)
				||(uaStoredDTC[DTC_POSBAT_TO_GROUND_VOL_FAIL]==DTC_FAILED)
				||(uaStoredDTC[DTC_NEGBAT_TO_GROUND_VOL_FAIL]==DTC_FAILED)
				||(uaStoredDTC[DTC_CHGPOS_TO_BUSNEG_VOL_FAIL]==DTC_FAILED)
				||(uaStoredDTC[DTC_CHGNEG_TO_BUSPOS_VOL_FAIL]==DTC_FAILED)
			){
				uiStsSend.ConfirmedDTC=1;
			}
		}
		uaBuffer[4+i*4]=uiStsSend.uiDataByte;
	}
	else{
		if(DTCLongTable[uiDataID].DTCStatus==DTC_FAILED){
			uiStsSend.TestFailed=1;
		}
		if(uiType==1){
			if(uaStoredDTC[uiDataID]==DTC_FAILED){
				uiStsSend.ConfirmedDTC=1;
			}
		}
		uaBuffer[4+i*4]=uiStsSend.uiDataByte;
	}
#else
		if(DTCLongTable[uiDataID].DTCStatus==DTC_FAILED){
			uiStsSend.TestFailed=1;
		}
		if(uiType==1){
			if(uaStoredDTC[uiDataID]==DTC_FAILED){
				uiStsSend.ConfirmedDTC=1;
			}
		}
		uaBuffer[4+i*4]=uiStsSend.uiDataByte;
#endif
	}


}
void sub_uvFilterDTCData(UINT8 uiStatusMask){
	StsMask  uiStsMask;
	UINT16  uiDataSum;

	uiStsMask.uiDataByte=uiStatusMask;
//	if((uiStsMask.ConfirmedDTC==1)||(uiStsMask.NotCompleted==1)||(uiStsMask.TestFailed==1)){
	if((uiStsMask.ConfirmedDTC==1)||(uiStsMask.TestFailed==1)){
		uiDataSum=sub_uvGetDTCData(uiStatusMask);
		sub_uvCopyDTCData(1);
	}
	//other bits don't be supported.
}

//return DTC information by specific status mask.
UINT16 sub_uiReadDTCByStatus(UINT8 uiReadType ,UINT8 uiStatusMask)
{
	UINT8 uiDtcStsAvailMask;
	UINT8 uiDtcFormatID;
	UINT16 uiDtcCount;
	UINT16 uiAvailableNumber;//<=255

	uiDtcStsAvailMask=0x19;//only support confirmed, testFailed and testNotCompleted bits.
	switch (uiReadType){
		case 0x01:
		case 0x11://mirror?
			uaBuffer[0]=uiDtcStsAvailMask;
			uiDtcFormatID=0;//ISO15031-6DTCFormat
			uaBuffer[1]=uiDtcFormatID;
			uiDtcCount=sub_uiFilteDTCNo(uiStatusMask);
			uaBuffer[2]=(uiDtcCount>>8)&0xFF;
			uaBuffer[3]=uiDtcCount&0xFF;
			uiSendSize=4;
			break;
		case 0x02:
		case 0x0F://mirror
			uaBuffer[0]=uiDtcStsAvailMask;
			//read DTC number and status
			sub_uvFilterDTCData(uiStatusMask);
			uiAvailableNumber=uiIndexLong;
			uiAvailableNumber=uiAvailableNumber*4+1;
			
			if(uiAvailableNumber>=UDS_DATA_TRANSMIT_LEN){
				uiSendSize=UDS_DATA_TRANSMIT_LEN;
			}
			else{
				uiSendSize=uiAvailableNumber;
			}
			break;
		default:
			break;
	}

}
UINT8 sub_uiScanPcodeTbl(UINT8 * ptPcode)
{
	UINT8 * ptTemp;
	UINT8 i;
	UINT8 uiTempValue;

	ptTemp=ptPcode;
	for(i=0;i<PCODE_SIZE;i++){//max 53
		if(*(ptTemp)==uaPcodeTbl[i][0]){//middle bytes
			if(*(ptTemp+1)==uaPcodeTbl[i][1]){//lowbytes
				return i;
			}
		}
	}
	return 0xff;
}

//return DTC information by specific DTC ID-pcode
UINT8 sub_uiReadDTCByNumber(UINT8 uiReadType ,UINT8 * ptMaskRecord)
{
	UINT8 * ptTemp;
	UINT8 i;
	UINT8 uiDtcStatus;
	UINT8 uiPcodeID;
	UINT8 uiDTCID;
	UINT8 uiCellNo=0xff;
	UINT8 uiSensorNo=0;
	UINT8 uiGetDTCStatus;
	
	ptTemp=ptMaskRecord;
	// UDSRvDataPara.uiData[0]
	// DTC number  3 bytes (DTC – high byte, middle byte,low byte)
	// DTC Extended Data Record Number  1 byte.
	if(uiReadType==0x06){
		//read extended data record by DTC number
		uiPcodeID=sub_uiScanPcodeTbl(ptTemp);
			if(uiPcodeID<PCODE_SIZE){
				uiDTCID=uaPcodeIndexTbl[uiPcodeID];
#if 1//Added by Amanda August 5
				if((uiDTCID==DTC_BATT_SENSOR_FAIL)
					||(uiDTCID==DTC_POSBAT_TO_GROUND_VOL_FAIL)
					||(uiDTCID==DTC_NEGBAT_TO_GROUND_VOL_FAIL)
					||(uiDTCID==DTC_CHGPOS_TO_BUSNEG_VOL_FAIL)
					||(uiDTCID==DTC_CHGNEG_TO_BUSPOS_VOL_FAIL)
					){
						if((DTCLongTable[DTC_BATT_SENSOR_FAIL].DTCStatus==DTC_FAILED)
							||(DTCLongTable[DTC_POSBAT_TO_GROUND_VOL_FAIL].DTCStatus==DTC_FAILED)
							||(DTCLongTable[DTC_NEGBAT_TO_GROUND_VOL_FAIL].DTCStatus==DTC_FAILED)
							||(DTCLongTable[DTC_CHGPOS_TO_BUSNEG_VOL_FAIL].DTCStatus==DTC_FAILED)
							||(DTCLongTable[DTC_CHGNEG_TO_BUSPOS_VOL_FAIL].DTCStatus==DTC_FAILED)
							){
								uiGetDTCStatus=DTC_FAILED;
						}
						else if((DTCLongTable[DTC_BATT_SENSOR_FAIL].DTCStatus==DTC_PASSED)
							&&(DTCLongTable[DTC_POSBAT_TO_GROUND_VOL_FAIL].DTCStatus==DTC_PASSED)
							&&(DTCLongTable[DTC_NEGBAT_TO_GROUND_VOL_FAIL].DTCStatus==DTC_PASSED)
							&&(DTCLongTable[DTC_CHGPOS_TO_BUSNEG_VOL_FAIL].DTCStatus==DTC_PASSED)
							&&(DTCLongTable[DTC_CHGNEG_TO_BUSPOS_VOL_FAIL].DTCStatus==DTC_PASSED)
							){
								uiGetDTCStatus=DTC_PASSED;
						}
						else{
								uiGetDTCStatus=DTC_NO_STATUS;
						}
				}
				else{
					uiGetDTCStatus=DTCLongTable[uiDTCID].DTCStatus;
				}
#else
				uiGetDTCStatus=DTCLongTable[uiDTCID].DTCStatus;
#endif
				switch(uiGetDTCStatus){
					case DTC_FAILED:
//						uiDtcStatus=0x09;//bit0 Test Failed is 1,bit3 Confirmed DTC is 1
						uiDtcStatus=0x01;//bit3 Confirmed DTC is 1
						break;
					case DTC_NO_STATUS:
						uiDtcStatus=0x10;//bit4 Test Not Completed Since Last Clear is 1
						break;
					case DTC_PASSED:
//						uiDtcStatus=0x08;//bit3 Confirmed DTC is 1
						uiDtcStatus=0;
						break;
					default:
						uiDtcStatus=0;
						break;
				}			
			}
			else{
				return FALSE;
			}		
	}
	else if(uiReadType==0x10){
		//read Mirror memory DTC extended data by DTC number
	
		uiPcodeID=sub_uiScanPcodeTbl(ptTemp);
			if(uiPcodeID<PCODE_SIZE){
				uiDTCID=uaPcodeIndexTbl[uiPcodeID];
#if 1//Added by Amanda August 5
				if((uiDTCID==DTC_BATT_SENSOR_FAIL)
					||(uiDTCID==DTC_POSBAT_TO_GROUND_VOL_FAIL)
					||(uiDTCID==DTC_NEGBAT_TO_GROUND_VOL_FAIL)
					||(uiDTCID==DTC_CHGPOS_TO_BUSNEG_VOL_FAIL)
					||(uiDTCID==DTC_CHGNEG_TO_BUSPOS_VOL_FAIL)
					){
						if((uaStoredDTC[DTC_BATT_SENSOR_FAIL]==DTC_FAILED)
							||(uaStoredDTC[DTC_POSBAT_TO_GROUND_VOL_FAIL]==DTC_FAILED)
							||(uaStoredDTC[DTC_NEGBAT_TO_GROUND_VOL_FAIL]==DTC_FAILED)
							||(uaStoredDTC[DTC_CHGPOS_TO_BUSNEG_VOL_FAIL]==DTC_FAILED)
							||(uaStoredDTC[DTC_CHGNEG_TO_BUSPOS_VOL_FAIL]==DTC_FAILED)
							){
								uiGetDTCStatus=DTC_FAILED;
						}
						else if((uaStoredDTC[DTC_BATT_SENSOR_FAIL]==DTC_PASSED)
							&&(uaStoredDTC[DTC_POSBAT_TO_GROUND_VOL_FAIL]==DTC_PASSED)
							&&(uaStoredDTC[DTC_NEGBAT_TO_GROUND_VOL_FAIL]==DTC_PASSED)
							&&(uaStoredDTC[DTC_CHGPOS_TO_BUSNEG_VOL_FAIL]==DTC_PASSED)
							&&(uaStoredDTC[DTC_CHGNEG_TO_BUSPOS_VOL_FAIL]==DTC_PASSED)
							){
								uiGetDTCStatus=DTC_PASSED;
						}
						else{
								uiGetDTCStatus=DTC_NO_STATUS;
						}
				}
				else{
					uiGetDTCStatus=uaStoredDTC[uiDTCID];
				}
#else
				uiGetDTCStatus=uaStoredDTC[uiDTCID];
#endif
				switch(uiGetDTCStatus){
					case DTC_FAILED:
//						uiDtcStatus=0x09;//bit0 Test Failed is 1,bit3 Confirmed DTC is 1
						uiDtcStatus=0x08;//bit3 Confirmed DTC is 1
						break;
					case DTC_NO_STATUS:
						uiDtcStatus=0x10;//bit4 Test Not Completed Since Last Clear is 1
						break;
					case DTC_PASSED:
						uiDtcStatus=0;
//						uiDtcStatus=0x08;//bit3 Confirmed DTC is 1
						break;
					default:
						uiDtcStatus=0;
						break;
				}			
			}
			else{
				return FALSE;
			}		
	}
	else{
		return FALSE;
	}
	//get status
	for (i=0;i<3;i++){
		uaBuffer[i]=*ptTemp;
		ptTemp++;
	}
	uaBuffer[3]=uiDtcStatus;
	//need extended data record number?
	uaBuffer[4]=0xff;//DTC Extended Data Record Number - All
	uaBuffer[5]=0;//Occurrence Flag  0 [1], Reserved 0 [7]
	uaBuffer[6]=0;//Original Odometer Value
	uaBuffer[7]=0;//Original Odometer Value
	uaBuffer[8]=0;//Most Recent Odometer Value
	uaBuffer[9]=0;//Most Recent Odometer Value
	uaBuffer[10]=0;//Frequency Counter
	uaBuffer[11]=0;//Ignition Cycle Counter
	uiSendSize=12;
	
	return TRUE;
}

//scan and save DTC IDs of supported
UINT16 sub_uvGetSupportedDTC(void)
{
	UINT16 dtcId;
	UINT8 uiStatus;
#if 1//Added by Amanda August 5
	UINT8 uiFirstTimeIn;
	uiFirstTimeIn=0;
#endif
	
	for(dtcId=0,uiIndexLong=0;dtcId<DTC_NUMBER_LONG;dtcId++){	//check id number's max is 256
		if(DTCLongTable[dtcId].DTCStatus!=DTC_NOT_SUPPORT){
#if 1//Added by Amanda August 5
			if((dtcId==DTC_BATT_SENSOR_FAIL)
				||(dtcId==DTC_POSBAT_TO_GROUND_VOL_FAIL)
				||(dtcId==DTC_NEGBAT_TO_GROUND_VOL_FAIL)
				||(dtcId==DTC_CHGPOS_TO_BUSNEG_VOL_FAIL)
				||(dtcId==DTC_CHGNEG_TO_BUSPOS_VOL_FAIL))
			{
				if(uiFirstTimeIn==0){
					uiFirstTimeIn=1;
					uaIDLongTbl[uiIndexLong]=dtcId;
					uiIndexLong++;
				}
				else{
				}
			}
			else{
				uaIDLongTbl[uiIndexLong]=dtcId;
				uiIndexLong++;
			}
#else
			uaIDLongTbl[uiIndexLong]=dtcId;
			uiIndexLong++;
#endif
		}
	}
	return uiIndexLong;
}
//return supported DTC informations (DTCID-3 bytes and DTC status-1 byte)
UINT16 sub_uiReadDTCInfomation(UINT8 uiReadType)
{
	UINT8 uiDtcStsAvailMask;
	UINT16 uiAvailableNumber;//<=255

	uiDtcStsAvailMask=0x19;//only support confirmed, testFailed and testNotCompleted bits.
	if(uiReadType==0x0A){
		uaBuffer[0]=uiDtcStsAvailMask;
		sub_uvGetSupportedDTC();
		sub_uvCopyDTCData(0);
		uiAvailableNumber=uiIndexLong;

		if( (uiAvailableNumber*4+1) >=UDS_DATA_TRANSMIT_LEN){
//		if( (uiAvailableNumber*3+1) >255){
			uiSendSize=UDS_DATA_TRANSMIT_LEN;
		}
		else{
			uiSendSize=uiAvailableNumber*4+1;	
//			uiSendSize=uiAvailableNumber*3+1;	
		}
		
	}
}
/*******************************************************************************************************/
//[Negative Response Trouble Code] Supported
// NRTC_INCORR_MSG_LENGTH			0x13	implemented		check ok	
// NRTC_CON_NOT_CORRECT			0x22	?
// NRTC_REQ_OUT_OF_RANGE			0x31	implemented		check ok
/*******************************************************************************************************/
//0x19
void nvReadDTCInfo(void)
{
	UINT8 j;
	if(uiGetMode(DATA_RV_MODE)==NORMAL_PROCESSING){
		for(j=0;j<30;j++){
			uaBuffer[j]=0;
		}
		uiSendSize=0;

		switch(UDSRvDataPara.uiSUB[0]){
			//Report Number Of DTC By Status Mask [$01]
			case 0x01:
			//Report DTC By Status Mask [$02]
			case 0x02:
			//Report Mirror Memory DTC By Status Mask [$0F]
			case 0x0F:
			//Report Number Of Mirror Memory DTC By Status Mask [$11]
			case 0x11:
				sub_uiReadDTCByStatus(UDSRvDataPara.uiSUB[0],UDSRvDataPara.uiData[0]);
				nvSendUDSPosResp(READ_DTC_INFO, &uaBuffer[0],uiSendSize);
				break;
			//Report DTC Extended Data Record By DTC Number [$06]
			case 0x06:
			//Report Mirror Memory DTC Extended Data By DTC Number [$10]
			case 0x10:
				if(sub_uiReadDTCByNumber(UDSRvDataPara.uiSUB[0],&UDSRvDataPara.uiData[0])){
					nvSendUDSPosResp(READ_DTC_INFO, &uaBuffer[0],uiSendSize);
				}
				else{
					nvSendUDSNegResp(READ_DTC_INFO,NRTC_REQ_OUT_OF_RANGE);
				}
				break;
			//Report Supported DTCs [$0A]
			case 0x0A:
			//Report DTC Fault Detection Counter [$14]
			case 0x14:
				sub_uiReadDTCInfomation(UDSRvDataPara.uiSUB[0]);
				nvSendUDSPosResp(READ_DTC_INFO, &uaBuffer[0],uiSendSize);
				break;
			default:
				nvSendUDSNegResp(READ_DTC_INFO,NRTC_REQ_OUT_OF_RANGE);
				break;
		}
	}

}

UINT8 sub_uiContactorControl(UINT8 uiContactorType,UINT8 uiControlType)
{
	IOControlByte IOControlData1;
	static UINT8 uiEnterAgain=0;
	
	IOControlData1.uiDataByte=uiControlType;


	if((uiContactorType>=1)&&(uiContactorType<=3)){
		if(systemInfo.HVILLoop!=0){//not open ,SD not out
			uiSendSize=2;
			uaBuffer[0]=0x05;
			uaBuffer[1]=0x0B;
			return FALSE;
		}
	}
	if(((uiContactorType==1)&&(IOControlData1.Bit3==1)&& (MCU_RELAY_MAIN_POS))||
		((uiContactorType==2)&&(IOControlData1.Bit4==1)&& (MCU_RELAY_PRE))||
		((uiContactorType==3)&&(IOControlData1.Bit2==1)&& (MCU_RELAY_MAIN_NEG)))
	{
		uiSendSize=2;
		uaBuffer[0]=0x05;
		uaBuffer[1]=0x0C;
		return FALSE;
	}
		
	if(uiContactorType==1){
		//Positive Contactor device control 
		if(IOControlData1.Bit3==1){
//			if((isDelayExpired(SECONDS(15),ContactorTick))||(!uiEnterAgain)){
			MCU_RELAY_MAIN_POS_SET(CLOSE);
//			uiEnterAgain=1;
//			ContactorTick=TickGet();
#if 0
			else{
				uiSendSize=2;
				uaBuffer[0]=0x00;
				uaBuffer[1]=0x08;
			}
#endif
		}
		else{
			MCU_RELAY_MAIN_POS_SET(OPEN);
		}
		uiSendSize=1;
		uaBuffer[0]=0;
		return TRUE;
	}
	else if(uiContactorType==2){
		if(IOControlData1.Bit4==1){
			MCU_RELAY_PRE_SET(CLOSE);
		}
		else{
			MCU_RELAY_PRE_SET(OPEN);
		}
		uiSendSize=1;
		uaBuffer[0]=0;
		return TRUE;
	}
	else if(uiContactorType==3){
		if(IOControlData1.Bit2==1){
			MCU_RELAY_MAIN_NEG_SET(CLOSE);
		}
		else{
			MCU_RELAY_MAIN_NEG_SET(OPEN);
		}
		uiSendSize=1;
		uaBuffer[0]=0;
		return TRUE;
	}
	return FALSE;
}

UINT8 sub_uiChgContactorControl(UINT8 uiContactorType,UINT8 uiControlType)
{
	IOControlByte IOControlData1;
	static UINT8 uiEnterAgain=0;
	
	IOControlData1.uiDataByte=uiControlType;


	if((uiContactorType==1)||(uiContactorType==3)){
		if(systemInfo.HVILLoop!=0){//not open ,SD not out
//		if(systemInfo.HVILStatus==0){
			uiSendSize=2;
			uaBuffer[0]=0x05;
			uaBuffer[1]=0x0B;
			return FALSE;
		}
	}
	if( ((uiContactorType==3)&&(IOControlData1.Bit3==1)&& (MCU_RELAY_CHG_NEG))||
		((uiContactorType==1)&&(IOControlData1.Bit4==1)&& (MCU_RELAY_CHG_POS)))
	{
		uiSendSize=2;
		uaBuffer[0]=0x05;
		uaBuffer[1]=0x0C;
		return FALSE;
	}
		
	if(uiContactorType==3){
		rs232Cmd.ChgContactorByRs232=TRUE;

		//Positive Contactor device control 
		if(IOControlData1.Bit3==1){
//			if((isDelayExpired(SECONDS(15),ContactorTick))||(!uiEnterAgain)){
			MCU_RELAY_CHG_NEG_SET(CLOSE);
//			uiEnterAgain=1;
//			ContactorTick=TickGet();
#if 0
			else{
				uiSendSize=2;
				uaBuffer[0]=0x00;
				uaBuffer[1]=0x08;
			}
#endif
		}
		else{
			MCU_RELAY_CHG_NEG_SET(OPEN);
		}
		uiSendSize=1;
		uaBuffer[0]=0;
		return TRUE;
	}
	else if(uiContactorType==1){
		rs232Cmd.ChgContactorByRs232=TRUE;
		if(IOControlData1.Bit4==1){
			MCU_RELAY_CHG_POS_SET(CLOSE);
		}
		else{
			MCU_RELAY_CHG_POS_SET(OPEN);
		}
		uiSendSize=1;
		uaBuffer[0]=0;
		return TRUE;
	}
	//for panel test
	else if(uiContactorType==2){
//		rs232Cmd.ChgContactorByRs232=TRUE;
		if(IOControlData1.Bit5==1){
			MCU_RELAY_OCD_SET(CLOSE);
		}
		else{
			MCU_RELAY_OCD_SET(OPEN);
		}
		uiSendSize=1;
		uaBuffer[0]=0;
		return TRUE;
	}

	

	
	return FALSE;
}

void sub_uvResetDCFault(void)
{
	//if vehicle speed not zero return $0401
	if(1){
/*		if(vehicleInfo.speed!=0){
		uiSendSize=2;
		uaBuffer[0]=0x04;
		uaBuffer[1]=0x01;
	}
	//if HV manual disconnect not removed return $050B
	else if(systemInfo.HVILLoop!=0){//not open ,SD not out
	else if(systemInfo.HVILStatus==0){
		uiSendSize=2;
		uaBuffer[0]=0x05;
		uaBuffer[1]=0x0B;
	}
	else{
	*/
		systemInfo.HVBusCapacityDischargeAlarm=FALSE;
		systemInfo.HVBusActiveDischargeConsecutive=FALSE;
		systemInfo.ADFCount=0;
		uaStoredDTC[DTC_ACTIVE_DISCHARGE_FAULT]=DTC_NO_STATUS;
		DTCLongTable[DTC_ACTIVE_DISCHARGE_FAULT].timeToFail=1;
		DTCLongTable[DTC_ACTIVE_DISCHARGE_FAULT].timeToSample=1;
		DTCLongTable[DTC_ACTIVE_DISCHARGE_FAULT].sampleCount=0;
		DTCLongTable[DTC_ACTIVE_DISCHARGE_FAULT].failCount=0;
		DTCLongTable[DTC_ACTIVE_DISCHARGE_FAULT].DTCStatus=DTC_NO_STATUS;	//
		DTCLongTable[DTC_ACTIVE_DISCHARGE_FAULT].isFailReported=YES;
		DTCLongTable[DTC_ACTIVE_DISCHARGE_FAULT].isPassReported=YES;
		uiSendSize=1;
		uaBuffer[0]=0;
	}
}

void sub_uvResetWeldFault(void)
{
	//if vehicle speed not zero return $0401
	if(vehicleInfo.speed!=0){
		uiSendSize=2;
		uaBuffer[0]=0x04;
		uaBuffer[1]=0x01;
	}
	//if HV manual disconnect not removed return $050B
/*
else if(systemInfo.HVILLoop!=0){//not open ,SD not out
else if(systemInfo.HVILStatus==0){
		uiSendSize=2;
		uaBuffer[0]=0x05;
		uaBuffer[1]=0x0B;
	}*/
	else{
		systemInfo.prechargeFailLatching=0;	
		systemInfo.contractorNegWeldLatching=0;
		systemInfo.contractorPosWeldLatching=0;
		systemInfo.contractorPcrWeldLatching=0;
		systemInfo.mainRelayPosInfo.weld=0;
		systemInfo.mainRelayNegInfo.weld=0;
		systemInfo.preChargeRelayInfo.weld=0;
		
		systemInfo.chgRelayPosInfo.weld=0;
		systemInfo.chgRelayNegInfo.weld=0;
		
		
		uaStoredDTC[DTC_CHG_CONTRACTOR_WELD]=DTC_NO_STATUS;
		DTCLongTable[DTC_CHG_CONTRACTOR_WELD].timeToFail=1;
		DTCLongTable[DTC_CHG_CONTRACTOR_WELD].timeToSample=1;
		DTCLongTable[DTC_CHG_CONTRACTOR_WELD].sampleCount=0;
		DTCLongTable[DTC_CHG_CONTRACTOR_WELD].failCount=0;
		DTCLongTable[DTC_CHG_CONTRACTOR_WELD].DTCStatus=DTC_NO_STATUS;	//
		DTCLongTable[DTC_CHG_CONTRACTOR_WELD].isFailReported=YES;
		DTCLongTable[DTC_CHG_CONTRACTOR_WELD].isPassReported=YES;

		uaStoredDTC[DTC_CONTRACTOR_WELD]=DTC_NO_STATUS;
		DTCLongTable[DTC_CONTRACTOR_WELD].timeToFail=1;
		DTCLongTable[DTC_CONTRACTOR_WELD].timeToSample=1;
		DTCLongTable[DTC_CONTRACTOR_WELD].sampleCount=0;
		DTCLongTable[DTC_CONTRACTOR_WELD].failCount=0;
		DTCLongTable[DTC_CONTRACTOR_WELD].DTCStatus=DTC_NO_STATUS;	//
		DTCLongTable[DTC_CONTRACTOR_WELD].isFailReported=YES;
		DTCLongTable[DTC_CONTRACTOR_WELD].isPassReported=YES;

		
		systemInfo.HBCNC_WC=0;
		uiSendSize=1;
		uaBuffer[0]=0;
	}
}

UINT8  sub_uvCoolingControl(UINT8 uiControlType,UINT8 uiControlData)
{
	if((systemInfo.V12>KE_SystemVoltageUpperThresholdVolts)||(systemInfo.V12<KE_SystemVoltageLowerThresholdVolts)){
		uiSendSize=2;
		uaBuffer[0]=0x04;
		uaBuffer[1]=0x05;
		return FALSE;
	}
	else {
		if((DTCLongTable[DTC_BATTERY_MODULE_OV].DTCStatus==DTC_FAILED)||
			(DTCLongTable[DTC_BATTERY_MODULE_UV].DTCStatus==DTC_FAILED)||
			(DTCLongTable[DTC_BATTERY_MODULE_VOL_DEVIATION_EOL].DTCStatus==DTC_FAILED)
			//||	(DTCLongTable[DTC_FAN_UNIT_FAIL].DTCStatus==DTC_FAILED)
			){
				uiSendSize=2;
				uaBuffer[0]=0x03;
				uaBuffer[1]=0x08;
				return FALSE;
		}
		else{			
			//later
			rs232Cmd.fanControlByRs232BeginTick=TickGet();
			rs232Cmd.fanControlByRs232=TRUE;
			
		/*	if(uiControlData>90){
				uiControlData=90;//max is 90%
			}*/
			FanControlData.FanControlByCDA.FanControl=ON;
			FanControlData.uiCoolingDataByCDA=uiControlData;
			FanControlData.FanControlByCDA.FanType=uiControlType;
			if(uiControlType==1){//passenger side
				FANSpeedCrl(1,uiControlData);
			}
			else if(uiControlType==2){//driver side
				FANSpeedCrl(2,uiControlData);
			}
			else{
				FANSpeedCrl(1,uiControlData);
				FANSpeedCrl(2,uiControlData);
			}
			uiSendSize=1;
			uaBuffer[0]=0;
			return TRUE;

		}
	}
}

void sub_uvIsolationDiag(void)
{
////Disable this by Amanda for isolation test precondition are not clear. March11,2011
//	if((!MCU_RELAY_MAIN_POS)||(!MCU_RELAY_MAIN_NEG)){
	if(((!MCU_RELAY_MAIN_POS)&&(!MCU_RELAY_PRE))||(!MCU_RELAY_MAIN_NEG)){
		uiSendSize=2;
		uaBuffer[0]=0x0A;
		uaBuffer[1]=0x10;
	}
	else {
	
		if((DTCLongTable[DTC_BATTERY_MODULE_OV].DTCStatus==DTC_FAILED)||
			(DTCLongTable[DTC_BATTERY_MODULE_UV].DTCStatus==DTC_FAILED)||
			(DTCLongTable[DTC_BATTERY_MODULE_VOL_DEVIATION_EOL].DTCStatus==DTC_FAILED)
			//||(DTCLongTable[DTC_FAN_UNIT_FAIL].DTCStatus==DTC_FAILED)
			){
				uiSendSize=2;
				uaBuffer[0]=0x03;
				uaBuffer[1]=0x08;
		}
		else{
			//run isolation diag
			uiIsoTestByCDA=TRUE;
			IsoTestTick=TickGet();
			uiSendSize=1;
			
			uaBuffer[0]=0;
		}
	}
}

void sub_uvRunBalanceDiag(UINT8 uiForceBalance)
{
	if((systemInfo.V12>KE_SystemVoltageUpperThresholdVolts)||(systemInfo.V12<KE_SystemVoltageLowerThresholdVolts)){
		uiSendSize=2;
		uaBuffer[0]=0x04;
		uaBuffer[1]=0x05;
	}
	else {
		uiBalanceByCDA=uiForceBalance;
		uiSendSize=1;
		uaBuffer[0]=0;
	}
}

UINT8 sub_uiClearFault(void)
{
	IOControlByte ClearData;

	ClearData.uiDataByte=UDSRvDataPara.uiData[0];

	switch(UDSRvDataPara.uiSUB[2]){
		case 0x06:
			if(ClearData.Bit0==1){
				//clear Active Discharge Fault
				//discharge Fault status eeprom reset device control
				sub_uvResetDCFault();
				
			}
			else{
				uiSendSize=1;
				uaBuffer[0]=0;
			}
			
			break;
		case 0x07: 
			if(ClearData.Bit0==1){
				//clear Weld Check Fault
				//weld check status eeprom reset device control 
				sub_uvResetWeldFault();
			}		
			else{
				uiSendSize=1;
				uaBuffer[0]=0;
			}
			break;
		case 0x08:
			if(ClearData.Bit0==1){
				//HV Isolation Diagnostic
				//Isolation Fault Diagnostic device control 
				//Run
				sub_uvIsolationDiag();
			}		
			else{
				uiSendSize=1;
				uaBuffer[0]=0;
			}
			break;
		case 0x09:
			if(ClearData.Bit0==1){
				uiSendSize=1;
				uaBuffer[0]=0;
				
			}		
			else{
				uiSendSize=1;
				uaBuffer[0]=0;
			}
			break;
		case 0x10:
			if(ClearData.Bit0==1){
				sub_uvRunBalanceDiag(TRUE);
			}		
			else{
				sub_uvRunBalanceDiag(FALSE);
			}
			break;
		
		default:
			break;
	}
	return TRUE;
}


UINT8 sub_uiIOControl(void)
{
	IOControlByte IOControlData1,IOControlData2,IOControlData3;
	UINT16 uiDataTemp;
	UINT8 uiDataTemp1;

	
	IOControlData1.uiDataByte=UDSRvDataPara.uiData[0];//control enable bits-contactors
	if(UDSRvDataPara.uiSUB[1]==0x01){
		//DDT 0202 Added by Amanda for short term adjustment on August 3
		if(UDSRvDataPara.uiSUB[2]==0x03){//short term adjustment
			if((IOControlData1.Bit0==1)||(IOControlData1.Bit1==1)){
				uiDataTemp1=(IOControlData1.Bit1<<1)|IOControlData1.Bit0;
				if(sub_uiContactorControl(uiDataTemp1,IOControlData1.uiDataByte)==TRUE){
					return TRUE;
				}
				else{
					return FALSE;
				}
			}
			return FALSE;
		}
		else if(UDSRvDataPara.uiSUB[2]==0x00){//return control to ECU
			uiSendSize=1;
			uaBuffer[0]=0;
			//after 112503 version
			MCU_RELAY_MAIN_POS_SET(OPEN);
			MCU_RELAY_MAIN_NEG_SET(OPEN);
			MCU_RELAY_PRE_SET(OPEN);
			return TRUE;
		}
	}
	else if((UDSRvDataPara.uiSUB[1]==0x02)||(UDSRvDataPara.uiSUB[1]==0x03)||(UDSRvDataPara.uiSUB[1]==0x04)){//?
		//Cooling system device control enable
		//DDT 0202 Added by Amanda for short term adjustment on August 3
		if(UDSRvDataPara.uiSUB[2]==0x03){//short term adjustment
			uiDataTemp=(IOControlData1.uiDataByte);
			uiDataTemp=uiDataTemp*100/255;
			IOControlData2.uiDataByte=uiDataTemp;
			if(UDSRvDataPara.uiSUB[1]==0x02){//all control
				return sub_uvCoolingControl(3, IOControlData2.uiDataByte);
			}
			else if(UDSRvDataPara.uiSUB[1]==0x03){//$D003 - Hybrid Battery Passenger Side Cooling Control
				return sub_uvCoolingControl(1, IOControlData2.uiDataByte);
			}
			else if(UDSRvDataPara.uiSUB[1]==0x04){//$D004 - Hybrid Battery Driver Side Cooling Control
				return sub_uvCoolingControl(2, IOControlData2.uiDataByte);
			}
			return TRUE;
		}
		else if(UDSRvDataPara.uiSUB[2]==0x00){//return control to ECU
			uiSendSize=1;
			uaBuffer[0]=0;
			FanControlData.FanControlByCDA.FanControl=OFF;
			FanControlData.FanControlByCDA.FanType=0;
			FanControlData.uiCoolingDataByCDA=0;
			rs232Cmd.fanControlByRs232=FALSE;
			return TRUE;
		}
	}
	else if(UDSRvDataPara.uiSUB[1]==0x05){
		//DDT 0202 Added by Amanda for short term adjustment on August 3
		if(UDSRvDataPara.uiSUB[2]==0x03){//short term adjustment
			if((IOControlData1.Bit0==1)||(IOControlData1.Bit1==1)){
				uiDataTemp1=(IOControlData1.Bit1<<1)|IOControlData1.Bit0;
				return sub_uiChgContactorControl(uiDataTemp1,IOControlData1.uiDataByte);
		//		return TRUE;
			}
			return FALSE;
		}
		else if(UDSRvDataPara.uiSUB[2]==0x00){//return control to ECU
			uiSendSize=1;
			uaBuffer[0]=0;
			rs232Cmd.ChgContactorByRs232=FALSE;

			return TRUE;
		}
	}
	return FALSE;
}

/*******************************************************************************************************/
//[Negative Response Trouble Code] Supported
// NRTC_INCORR_MSG_LENGTH			0x13	implemented		check ok	
// NRTC_CON_NOT_CORRECT			0x22	?
// NRTC_REQ_OUT_OF_RANGE			0x31	implemented		check ok
/*******************************************************************************************************/
//0x2F-->0x30

void nvDeviceControl(void)
{
	UINT8 uiControlResult=0;

//	if(uiGetAppDiagSessionMode()!=DEFAULT_DIAG_SESSION){
		if(uiGetMode(DATA_RV_MODE)==NORMAL_PROCESSING){
			//Hybrid Battery Device Control
//			if((UDSRvDataPara.uiSUB[0]==0x01)&&(UDSRvDataPara.uiSUB[0]==0x07)){
			if((UDSRvDataPara.uiSUB[0]==0xD0)&&((UDSRvDataPara.uiSUB[1]>=0x01)&&(UDSRvDataPara.uiSUB[1]<=0x05))&&(UDSRvDataPara.uiSUB[2]==0x03)||(UDSRvDataPara.uiSUB[2]==0x00)){
				uiControlResult=sub_uiIOControl();
				if(uiControlResult==TRUE){
					//IO status record
					nvSendUDSPosResp(CONTROL_DEVICE, &uaBuffer[0],uiSendSize);

				}
				else{
					nvSendUDSNegResp(CONTROL_DEVICE,NRTC_CON_NOT_CORRECT);
				}
			
			}
			else{
				nvSendUDSNegResp(CONTROL_DEVICE,NRTC_REQ_OUT_OF_RANGE);
			}
		}
		
//	}
//	else{
//		nvSendUDSNegResp(CONTROL_DEVICE,NRTC_CON_NOT_CORRECT);
//	}
}



void nvClearStoredDTCbyKC(void)
{
	UINT16	i;
	
	//All Groups /All DTCs
	//initDTCTable();
	for (i=0;i<DTC_NUMBER_LONG;i++){
		if(uaStoredDTC[i]!=DTC_NO_STATUS){
			uaStoredDTC[i]=DTC_NO_STATUS;
		}
	}
	writeDTCInfoToEEPROMAll();

}
/****************************************************************************************************
 *	Function: nvUDSProcessInBoot
 *    
 *	Description: 
 *		0x101 FE XX UDSMode
 *		0x7E7 XX XX UDSMode
 *		UDS Process
 *
 *  Parameters: none
 *
 *	Return :none		
 *
 *	Auther: David
 *
 *   (c) 2009 ElectroVaya
 *
 *******************************************************************************************************/
void  nvUDSTask(void){//parameters is temp
	// CANRxMsg.DATA.bytes[2]
	UINT8 i;
	UINT8 uiSID;

	
	if((uiGetMode(DATA_STORE_MODE)==DATA_STORING)||
		(uiGetMode(DATA_STORE_MODE)==DATA_READING)||
		(uiGetMode(DATA_STORE_MODE)==DATA_IVT_STORED)){

		if(uiIsTimeout(TIMER_PERIOD_1S)){
			uv30mstimerout();//saving data to flash needs more time(1s)
		}
	}

	else{
		if(uvSendLastOptResp()){
			for(i=0;i<8;i++){
				uaUDSReqPara[i]=0;
			
			}
			return;
		}
		
	}
	if(uiIsTimeout(TIMER_PERIOD_5S)){
		uv5Stimerout();
	}
	if((uiTxMode==FLOW_CONTROL_CONTINUE)&&(uiIsTimeout(TIMER_PERIOD_5MS))){
		uv5mStimerout();
	}

	if(uiGetMode(RSP_MODE)==RSP_PENDING){
		return;
	}
	//Data check
//	if(uaUDSReqPara[0]!=0){
	if((uaUDSReqPara[0]!=0)&&(uaUDSReqPara[0]<=0x30)){//valid length, need process based on service id. filter correct data modified by Amanda on Oct 26 2010
		if((uaUDSReqPara[0]==0x30)&&(uaUDSReqPara[1]==0x00)){ // multi package process
			//send flow control data mulit package
			if(uiTxMode==FLOW_CONTROL_BEGIN){
				//For flow control ST time added on Sep 23, 2010
				uiSTtimeFL=uaUDSReqPara[2];
				uiTxMode=FLOW_CONTROL_CONTINUE;
				nvFCData();
			}
		}
		else{	//
			//receive data
			if((uiGetMode(DATA_RV_MODE)!=NORMAL_PROCESSING)&&
				//not support 0x101 and 0x7df according to cs11736 modified by Amanda on Oct 27 2010
//				((uiCanSID==0x101)||(uiCanSID==0x441)||(uiCanSID==0x7DF))&&
				((uiCanSID==BPCM_BOOT_FUNCTIONAL_ADDRESS))&&
//				((uaUDSReqPara[1]==0x3E)&&((uaUDSReqPara[2]==0x80)||(uaUDSReqPara[2]==0x02))) ){
				((uaUDSReqPara[1]==0x3E)) ){
				uvStartTimer(TIMER_PERIOD_5S);
				for(i=0;i<8;i++){
					uaUDSReqPara[i]=0;
				}
				return;
			}
			else{
				uiSID=uvChkFlowControl();   //check if need flow control
				if(uvProcessMsgData()==FALSE){ 	//need not procell
					for(i=0;i<8;i++){
						uaUDSReqPara[i]=0;
					}
					return;
				}
			}
			for(i=0;i<8;i++){
				uaUDSReqPara[i]=0;
			
			}
			// uiSiD set in uvChkFlowControl()
			switch(uiSID){
				case START_DIAGNOSTIC_SESSION://Start Diagnostic Session
					uvStartTimer(TIMER_PERIOD_5S);
					lastReqFromUDSTick=TickGet();
					nvDiagSessionChange();
					break;
				case ECU_RESET://ECU Reset
					uvStartTimer(TIMER_PERIOD_5S);
					lastReqFromUDSTick=TickGet();
					nvEcuReset();
					break;
				case CLEAR_DIAG_INFO://clear diagnostic info
					uvStartTimer(TIMER_PERIOD_5S);
					lastReqFromUDSTick=TickGet();
					nvClearDiagInfo();
					break;
				case READ_DTC_INFO://read dtc info
					uvStartTimer(TIMER_PERIOD_5S);
					lastReqFromUDSTick=TickGet();
					nvReadDTCInfo();
					break;
				case ROUTINE_CONTROL://Routine Control
					uvStartTimer(TIMER_PERIOD_5S);
					lastReqFromUDSTick=TickGet();
					nvRoutineControl();
					break;
				case TESTER_PRESENT://Tester Present
					uvStartTimer(TIMER_PERIOD_5S);
					lastReqFromUDSTick=TickGet();
					nvTesterPresent();
					break;
				case READ_DATA_BY_IDENTIFIER://Read Data by Identifier...
					uvStartTimer(TIMER_PERIOD_5S);
					lastReqFromUDSTick=TickGet();
					nvReadData();
					break;
				case COMMUNICATION_CONTROL://Comm control...
					uvStartTimer(TIMER_PERIOD_5S);
					lastReqFromUDSTick=TickGet();
					nvCommControl();
					break;
				case CONTROL_DTC_SETTING://Control dtc setting...
					uvStartTimer(TIMER_PERIOD_5S);
					lastReqFromUDSTick=TickGet();
					nvControlDtcSetting();
					break;
				case WRITE_DID:
					uvStartTimer(TIMER_PERIOD_5S);
					lastReqFromUDSTick=TickGet();
					nvWriteData();
					break;
				case CONTROL_DEVICE:
					uvStartTimer(TIMER_PERIOD_5S);
					lastReqFromUDSTick=TickGet();
					nvDeviceControl();
					break;					
				default:
					break;
				}
			}
	}

}
#endif

