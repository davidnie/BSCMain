#ifndef _MESA_H_
#define _MESA_H_

#include "config.h"
#include "stm32f4xx.h" 
#include "stm32f4x7_eth.h"
#include "stm32f4x7_eth_bsp.h"
#include "stm32f4xx_spi.h"

#include "lwip/tcp.h"
#include "httpd.h"
#include "STDIO.h"

#include "M_Type_P.h"
#include "M_CMSIS_P.h" 
#include "M_Global.h"
#include "Task_Startup.h"
#include "Bsp_Global.h"
#include "app_inc.h"

#define TASK_MESA_STK_SIZE 400
extern OS_STK Stk_Task_Mesa[TASK_MESA_STK_SIZE];
extern void Task_Mesa(void *pdata);

typedef enum{
	STORAGE=90, // Storage Device	90	
	BATTERY=91, // Battery Device	91	
	LITHIUM_ION=92, // BATTERY Lithium-ion Battery Device	92	
	REDOX_FLOW=93, // BATTERY	Redox Flow Battery Device	93	
	max1=0xFFFF
}DER_TYPE;
typedef enum{
	OFF=1, //	1	
	EMPTY,	//	2	
	DISCHARGING, //	3	
	CHARGING,	//	4	
	FULL,		// 5	
	HOLDING, 	// 6	
	TESTING,	//7	
	max2=0xFFFF
}CHA_STAT;
typedef enum{
	REMOTE=1,	//	Remote Control	1	
	LOCAL=2,	//Local Control	2
	max=0xFFFF
}LOC_REM_CTRL_TYPE;
typedef enum{		
	UNDER_SOC_MIN_WARNING, //	Under State of Charge Minimum Warning	0	
	UNDER_SOC_MIN_ALARM, //    Under State of Charge Minimum Alarm 1	
	OVER_SOC_MAX_WARNING, //	Over State of Charge Maximum Warning	2	
	OVER_SOC_MAX_ALARM,	//	Over State of Charge Maximum Warning	3	
	max3=0xFFFFFFFF
}EVENT_TYPE;

typedef struct{
	/* register 801 */
	
	uint16_t ID_801; //	Energy Storage Base Model	801 uint16
	uint16_t L_801; //		22	uint16
	DER_TYPE DERTyp; //	Distributed Energy Resource Type		enum16
	uint16_t WHRtg; //	Nameplate Energy Capacity		uint16
	uint16_t WMaxChaRte; //	Nameplate Max Charge Rate		uint16
	uint16_t WMaxDisChaRte; //	Namplate Max Discharge Rate 	uint16
	uint16_t DisChaRte; //	Self Discharge Rate 	uint16
	uint16_t SoCNpMaxPct; // Nameplate Max SoC		uint16
	uint16_t SoCNpMinPct; // Nameplate Min SoC		uint16
	uint16_t MaxRsvPct; //	Maximum Reserve Percent 	uint16
	uint16_t MinRsvPct; //	Minimum Reserve Percent 	uint16
	uint16_t SoC; // State of Charge 	uint16
	CHA_STAT ChaSt; //	ChaSt		enum16
	LOC_REM_CTRL_TYPE LocRemCtl; //	Control Mode		enum16
	EVENT_TYPE Evt; //t Event Bitfield		bitfield32
	uint16_t DERHb; //	Distributed Energy Resource Heartbeat		uint16
	uint16_t ControllerHb; //	Controller Heartbeat		uint16
	uint16_t DERAlarmReset; //	Alarm Reset 	uint16
	int16_t WHRtg_SF; //			sunssf
	int16_t WMaxChaDisCha_SF; //			sunssf
	uint16_t DisChaRte_SF; //			sunssf
	int16_t SoC_SF; //			sunssf
	uint16_t Pad; // 		pad
}MESA_DATA_801;
extern MESA_DATA_801 mesa_801;

typedef enum{
	NOT_APPLICABLE_UNKNOWN, //	Not Applicable or Unknown	0	
	LEAD_ACID, //	Lead-Acid	1	
	NICKEL_METAL_HYDRATE, //	Nickel-Metal Hydrate	2	
	NICKEL_CADMIUM, //	Nickel-Cadmium	3	
	LITHIUM_IONa, //	Lithium-Ion	4	
	CARBON_ZINC, //	Carbon-Zinc	5	
	ZINC_CHLORIDE, //	Zinc Chloride	6	
	ALKALINE, //	Alkaline	7	
	RECHARGEABLE_ALKALINE, //	Rechargeable Alkaline	8	
	SODIUM_SULFUR, //	Sodium-Sulfur	9	
	FLOW, //	Flow, //	10	
	OTHER, //	Other	99	
	max4=0xFFFF
}BATT_YPTE;

typedef enum{			
	DISCONNECTED=1, //	Disconnected	1	
	INITIALIZING, //	Initializing	2	
	CONNECTED, //	Connected	3	
	STANDBY, //	Standby	4	
	SOC_PROTECTION, //	SOC Protection	5	
	max5=0xFFFF

}BATT_STAT;	
typedef enum{
	COMMUNICATION_ERROR, //	Communication Error	0	
	OVER_TEMP_ALARM, //	Over Temperature Alarm	1	
	UNDER_TEMP_ALARM, //	Under Temperature Alarm	2	
	OVER_TEMP_WARNING, //	Over Temperature  Warning	3	
	UNDER_TEMP_WARNING, //	Under Temperature Warning	4	
	OVER_CHARGE_CURRENT_ALARM, //	Over Charge Current Alarm	5	
	OVER_DISCHARGE_CURRENT_ALARM, //	Over Discharge Current Alarm	6	
	OVER_CHARGE_CURRENT_WARNING, //	Over Charge Current Warning	7	
	OVER_DISCHARGE_CURRENT_WARNING, //	Over Discharge Current Warning	8	
	VOLTAGE_IMBALANCE_WARNING, //	Voltage Imbalance Warning	9	
	CURRENT_IMBALANCE_WARNING, //	Current Imbalance Warning	10	
	OVER_VOLT_ALARM, //	Over Voltage Alarm	11	
	UNDER_VOLT_ALARM, //	Under Voltage Alarm	12	
	OVER_VOLT_WARNING, //	Over Voltage Warning	13	
	UNDER_VOLT_WARNING, //	Under Voltage Warning	14	
	CONTACTOR_ERROR, //	Contactor Error	15	
	FAN_ERROR, //	Fan Error	16	
	CONTACTOR_STATUS, //	Contactor Status	17	
	GROUND_FAULT, //	Ground Fault Error	18	
	OPEN_DOOR_ERROR, //	Open Door Error	19	
	OTHER_ALARM, //	Other Battery Alarm	20	
	OTHER_WARNING, //	Other Battery Warning	21	
	max6=0xFFFFFFFF
}BATT_EVENT;			
typedef enum{
	NO_REQUEST, //	No Request	0	
	START, //	Start PCS	1	
	STOP, //	Stop PCS	2	
	max7=0xFFFF
}BATT_REQ_PCS_STAT;
typedef enum{
	CONNECT, //	Connect the Battery Bank	1	
	DISCONNECT, //	Disconnect the Battery Bank	2	
	max8=0xFFFF
}BATT_OPERATION;		
typedef enum{
	PCS_STOPPED=1, //	PCS is Stopped	1	
	PCS_STANDBY, //	PCS is in Standby	2	
	PCS_STARTED, //	PCS is Started	3	
	max9=0xFFFF

}BATT_PCS_STAT;

typedef struct{
	uint16_t ID_802;	//	Battery Base Model	802	uint16
	uint16_t L_802;		//		20	uint16
				
	BATT_YPTE BatTyp;	//	Battery Type;		//		enum16
	BATT_STAT BatSt;	//	State of the Battery Bank;		//		enum16
	uint32_t CycleCt;		//	Cycle Count		uint32
	uint16_t SoH;		//	State of Health		uint16
	BATT_EVENT Evt1;	//	Battery Event 1;		// Bitfield		bitfield32
	BATT_EVENT Evt2;	//	Battery Event 2;		// Bitfield		bitfield32
	uint16_t Vol;		//	External Battery Voltage		uint16
	uint16_t MaxBatACha;		//	Max Charge Current		uint16
	uint16_t MaxBatADischa;		//	Max Discharge Current		uint16
	BATT_REQ_PCS_STAT BatReqPCSSt;		//	PCS State Request		enum16
	int16_t BatReqW;		//	Battery Power Request		int16
	BATT_OPERATION BSetOperation;		//	Set Operation		enum16
	BATT_PCS_STAT BSetPCSState;		//	PCS State		enum16
	int16_t SoH_SF;		//			sunssf
	int16_t Vol_SF;		//			sunssf
	int16_t MaxBatA_SF;		//			sunssf
	int16_t BatReqW_SF;		//			sunssf
}MESA_DATA_802;		
extern MESA_DATA_802 mesa_802;

typedef enum{
	STR_COMMUNICATION_ERROR,		//		Communication Error	0						Unable to communcate with BMS.	
	STR_OVER_TEMP_ALARM,		//	Over Temperature Alarm	1						Battery string has exceeded maximum operating temperature.	
	STR_UNDER_TEMP_ALARM,		//	Under Temperature Alarm	2						Battery string has exceeded minimum operating temperature.	
	STR_OVER_TEMP_WARNING,		//	Over Temperature  Warning	3						Battery string is approaching maximum operating temperature.	
	STR_UNDER_TEMP_WARNING,		//	Under Temperature Warning	4						Battery string is approaching minimum operating temperature.	
	STR_OVER_CHARGE_CURRENT_ALARM,		//	Over Charge Current Alarm	5						Battery string maximum charge current has been exceeded.	See AMaxCha
	STR_OVER_DISCHARGE_CURRENT_ALARM,		//	Over Discharge Current Alarm	6						Battery string maximum discharge current has been exceeded.	See AMaxDischa
	STR_OVER_CHARGE_CURRENT_WARNING,		//	Over Charge Current Warning	7						Approaching battery string maximum charge current.	See AMaxCha
	STR_OVER_DISCHARGE_CURRENT_WARNING,		//	Over Discharge Current Warning	8						Approaching battery string maximum discharge current.	See AMaxDischa
	STR_OVER_VOLT_ALARM,		//	Over Voltage Alarm	9						Battery string voltage has exceeded maximum limit.	
	STR_UNDER_VOLT_ALARM,		//	Under Voltage Alarm	10						Battery string voltage has exceeded minimum limit.	
	STR_OVER_VOLT_WARNING,		//	Over Voltage Warning	11						Battery string voltage is approaching maximum limit.	
	STR_UNDER_VOLT_WARNING,		//	Under Voltage Warning	12						Battery string voltage is approaching minimum limit.	
	STR_CONTACTOR_ERROR,		//	Contactor Error	13						String contactor failed to open or close as requested.	
	STR_FAN_ERROR,		//	Fan Error	14						Battery string fan has failed.	
	STR_CONTACTOR_STATUS,		//	Contactor Status	15						String contactor is closed.	
	STR_GROUND_FAULT,		//	Ground Fault Error	16						Ground fault detected.	
	STR_OPEN_DOOR_ERROR,		//	Open Door Error	17						One or more door are open.	
	STR_OTHER_ALARM,		//	Other Alarm	18						An alarm has been triggered which does not map to one of the alarms above.	Operator should view battery HMI and/or logs for more information.
	STR_OTHER_WARNING,		//	Other Warning	19						A warning has been triggered which does not map to one of the alarms above.	Operator should view battery HMI and/or logs for more information.
	STR_STRING_ENABLED,		//	String Enabled	20	The string is currently enabled.	
	STR_max11=0xFFFFFFFF
}STRING_EVENT;
typedef enum{
	NO_FAILURE,		//		0						Connect did not fail.	
	BUTTON_PUSHED,		//		1						A button was pushed which prevented connection.	
	STR_GND_FAULT,		//		2						Ground fault during auto-connect.	
	OUTSIDE_VOLTAGE_RANGE,		//		3						Outside voltage target window during auto-connect.	
	max12=0xFFFF
}STRING_FAIL_REASON;
typedef enum{
	STR_ENABLE,		//		1						Enable the string.	
	STR_DISABLE,		//		2						Disable the string.	
	max13=0xFFFF
}STRING_SET;

typedef struct{
uint16_t ID_803;		//		Lithium-Ion Battery Model	803	uint16			R	M		
uint16_t L_803;		//			16	uint16			R	M	Model Length	
									
uint16_t BConStrCt;		//		Connected String Count		uint16			R	M	Number of strings with contactor closed.	
uint16_t BMaxCellVol;		//		Max Cell Voltage		uint16	V	BCellVol_SF	R	M	Maximum voltage for all cells in the bank.	Measurement
uint16_t BMaxCellVolLoc;		//		Max Cell Voltage Location		uint16			R	O	Location of the cell with maximum voltage	Bit0:Bit7 = String Number  Bit8:Bit15 = Module Number
uint16_t BMinCellVol;		//		Min Cell Voltage		uint16	V	BCellVol_SF	R	M	Minimum voltage for all cells in the bank.	Measurement
uint16_t BMinCellVolLoc;		//		Min Cell Voltage Location		uint16			R	O	Location of the cell with minimum voltage	Bit0:Bit7 = String Number  Bit8:Bit15 = Module Number
int16_t BMaxModTmp;		//		Max Module Temp		int16		BModTmp_SF	R	M	Maximum temperature for all modules in the bank.	Measurement
uint16_t BMaxModTmpLoc;		//		Max Module Temp Location		uint16			R	O	Location of the module with max temperature.	Bit0:Bit7 = String Number  Bit8:Bit15 = Module Number
int16_t BMinModTmp;		//		Min Module Temp		int16	C	BModTmp_SF	R	M	Minimum temperature for all modules in the bank.	Measurement
uint16_t BMinModTmpLoc;		//		Min Module Temp Location		uint16			R	O	Location of the module with min temperature.	Bit0:Bit7 = String Number  Bit8:Bit15 = Module Number
int16_t BTotDCCur;		//		Total DC Current		int16	A	BCurrent_SF	R	M	Total DC current flowing to/from the battery bank.	Measurement
int16_t BMaxStrCur;		//		Max String Current		int16	A	BCurrent_SF	R	O	Maximum current of any string in the bank.	Measurement
int16_t BMinStrCur;		//		Min String Current		int16	A	BCurrent_SF	R	O	Minimm current of any string in the bank.	Measurement
int16_t BCellVol_SF;		//				sunssf			R	M	Scale factor for cell voltage.	
int16_t BModTmp_SF;		//				sunssf			R	M	Scale factor for module temperatures.	
int16_t BCurrent_SF;		//				sunssf			R	M	Scale factor for Total DC Current, Max String Current and Min String Current.	
int16_t StrSoH_SF;		//				sunssf			R	O	Scale factor for String State of Health.	
									
uint16_t StrModCt;		//		Module Count		uint16			R	M	Count of modules in the string.	
uint16_t StrSoC;		//		String SoC		uint16	%		R	M	Battery string state of charge, expressed as a percentage.	Measurement
uint16_t StrSoH;		//		String SoH		uint16	%	StrSoH_SF	R	O	Battery string state of health, expressed as a percentage.	Measurement
int16_t StrCur;		//		String Current		int16	A	BCurrent_SF	R	M	String current measurement.	Measurement
uint16_t StrMaxCellVol;		//		Max Cell Voltage		uint16	V	BCellVol_SF	R	M	Maximum voltage for all cells in the string.	Measurement
uint16_t StrMinCellVol;		//		Min Cell Voltage		uint16	V	BCellVol_SF	R	M	Minimum voltage for all cells in the string.	Measurement
uint16_t StrCellVolLoc;		//		Max/Min Cell Voltage Loc		uint16			R	O	Location of maximum and minimum cell voltages.	Bit0:Bit7 = Max Module Num  Bit8:Bit15 = Min Module Num
int16_t StrMaxModTmp;		//		Max Module Temp		int16	C	BModTmp_SF	R	M	Maximum temperature for all modules in the bank.	Measurement
int16_t StrMinModTmp;		//		Min Module Temp		int16	C	BModTmp_SF	R	M	Minimum temperature for all modules in the bank.	Measurement
uint16_t StrModTmpLoc;		//		Max/Min Mod Temp Loc		uint16			R	O	Location of maximum and minimum module temperatures.	Bit0:Bit7 = Max Module Num  Bit8:Bit15 = Min Module Num
STRING_EVENT StrEvt1;		//		String Event 1;		//			bitfield32			R	M	Alarms, warnings and status values.  Bit flags.	
STRING_EVENT StrEvt2;		//		String Event 2		bitfield32			R	O	Alarms, warnings and status values.  Bit flags.	Reserved for future use.
STRING_FAIL_REASON StrConFail;		//		Connection Failure Reason		enum16			R	O		
STRING_SET StrSetEna;		//		Enable/Disable String		enum16			RW	O	Enables and disables the string.  Should reset to 0 upon completion.	
}MESA_DATA_803;	
extern MESA_DATA_803 mesa_803;


#endif
