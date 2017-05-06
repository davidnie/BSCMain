#ifndef _BMU_H
#define _BMU_H
#include "applicfg.h"
#include "data.h"
#include "config.h"
typedef union{
	UNS32 wd;
	struct{
		UNS32 b1:1;
		UNS32 b2:1;
		UNS32 b3:1;
		UNS32 b4:1;
		UNS32 b5:1;
		UNS32 b6:1;
		UNS32 b7:1;
		UNS32 b8:1;
		UNS32 b9:1;
		UNS32 b10:1;
		UNS32 b11:1;
		UNS32 b12:1;
		UNS32 b13:1;
		UNS32 b14:1;
		UNS32 b15:1;
		UNS32 b16:1;
		UNS32 b17:1;
		UNS32 b18:1;
		UNS32 b19:1;
		UNS32 b20:1;
		UNS32 b21:1;
		UNS32 b22:1;
		UNS32 b23:1;
		UNS32 b24:1;
	}bits;
}CELL_BITSTAT;
/* BMU flag 1 (CANOpen 6D07) */
typedef struct{
	UNS8 rev_1:1;
	UNS8 HostFetOff:1;
	UNS8 PF;
	UNS8 FETOT;
	UNS8 Fault;
	UNS8 OV2;
	UNS8 OV;
	UNS8 UV;
	UNS8 COC;
	UNS8 COT;
	UNS8 DOT;
	UNS8 PreTOT;
	UNS8 Charging;
	UNS8 Discharging;
	UNS8 Idle;
	UNS8 WakeupIn;
	UNS8 AlertFail;
	UNS8 I2CError;
	UNS8 PFAleady;
	UNS8 FETOT2;
	UNS8 FETFail2;
	UNS8 IMB;
	UNS8 FC;
	UNS8 FD;
	UNS8 DOC;
	UNS8 CUT;
	UNS8 DUT;
	UNS8 PreDFET;
	UNS8 Chargeable;
	UNS8 Dischargable;
	UNS8 AFE_SC;
	UNS8 button;
}BMU_FLAG;
typedef struct{
	UNS32 button:1;
	UNS32 HostOffline:1;
	UNS32 preDFET:1;
	UNS32 discharable:1;
	UNS32 charable:1;
	UNS32 FD:1;
	UNS32 XReadyFail:1;
	UNS32 AlertFail:1;
	UNS32 I2CError:1;
	UNS32 DryContact:1;
	UNS32 shortRisk:1;
	UNS32 fault:1;
	UNS32 PFAleady:1;
	UNS32 PF:1;
	UNS32 CmdON:1;
	UNS32 balancing:1;
	UNS32 wakeupIn:1;
	UNS32 cmdShutdown:1;
	UNS32 idle:1;
	UNS32 discharging:1;
	UNS32 charging:1;
	UNS32 FC:1;
	UNS32 buttonBlackStart:1;
	UNS32 IDErr:1;
	UNS32 memAllcErr:1;
	UNS32 hostFetOff:1;
	UNS32 rev_1:6;
/*
	UNS32 rev_1:6;
	UNS32 HostFetOff:1;
	UNS32 MemAllcErr:1;
	UNS32 IDErr:1;
	UNS32 ButtonBlackStart:1;
	UNS32 FC:1;
	UNS32 Charging:1;
	UNS32 discharging:1;
	UNS32 Idle:1;
	UNS32 CmdShutdown:1;
	UNS32 WakeupIn:1;   // 16bits
	UNS32 Balancing:1;
	UNS32 CmdON:1;
	UNS32 PF:1;
	UNS32 PFAlready:1;
	UNS32 Fault:1;
	UNS32 ShortRisk:1;
	UNS32 DryContact:1;
	UNS32 I2CError:1;
	UNS32 AlertFail:1;
	UNS32 XReadyFail:1;
	UNS32 FD:1;
	UNS32 Chargeable:1;
	UNS32 Dischargable:1;
	UNS32 PreDFET:1;
	UNS32 HostOffline:1;
	UNS32 Button:1;
	*/
}STU_BMU_FLAG_6D07;
typedef union{
	STU_BMU_FLAG_6D07 b_flag1;
	UNS32 w_flag1;
}BMU_FLAG_6D07_t;
typedef struct{
	UNS32 afe_sc:1;
	UNS32 repeat_sc:1;
	UNS32 pf_sc:1;
	UNS32 a_coc:1;
	UNS32 f_coc:1;
	UNS32 w_ot:1;
	UNS32 a_cot:1;
	UNS32 a_dot:1;
	UNS32 pf_ot:1;
	UNS32 pf_dfet_fail:1;
	UNS32 preR_ot:1;
	UNS32 w_ov:1;
	UNS32 a_ov:1;
	UNS32 pf_ov:1;
	UNS32 w_oc:1;
	UNS32 IMB:1;
	UNS32 w_fetot:1;  // 16bits
	UNS32 a_fetot:1;
	UNS32 pf_fetot:1;
	UNS32 a_doc:1;
	UNS32 f_doc:1;
	UNS32 w_ut:1;
	UNS32 a_cut:1;
	UNS32 a_dut:1;
	UNS32 f_ut:1;
	UNS32 pf_cfet_fail:1;
	UNS32 w_bme:1;
	UNS32 w_uv:1;
	UNS32 a_uv:1;
	UNS32 f_duv:1;
	UNS32 pf_uv:1;	//31
	UNS32 pf_uv2:1;  //32
}STU_BMU_FLAG_6D08;
typedef union{
	STU_BMU_FLAG_6D08 b_flag2;
	UNS32 w_flag2;
}BMU_FLAG_6D08_t;

typedef struct S_BMU_INFO{
UNS16 SOC;  // 0.1%
UNS16 SOH; //0.1%
int32_t I_bmu; //ma
UNS32 V_bmu; //mv
REAL32 ct_max; //C
REAL32 ct_min; //C
BMU_FLAG_6D07_t bmuSafeFlag_1;
BMU_FLAG_6D08_t bmuSafeFlag_2;
UNS32 bmuStatus;
UNS8 fetFlag;      //6D0B
UNS8 parllFlag;
int32_t cc_max;  //0.001A
int32_t dc_max;  //0.001A
//index 6E80 sub_index 1
BMU_FLAG_6D07_t gBatteryStatus; //	UINT32	-	Battery Status
BMU_FLAG_6D08_t gParallelFlags; //	UINT8	-	Parallel Status
uint32_t gFlags1; //	UINT32	-	Warning Flags1
uint8_t gFlags2; //	UINT8	-	Warning Flags2
uint8_t gFetStatus; //	UINT8	-	(CFET/DFET/ON/OFF)
//index 0x6E80 sub_index:2
REAL32 FWVer; //	FLOAT	-	Firmware Version	R
uint8_t SpecVer[8]; //[8]	CHAR[8]	-	Spec Version	R
uint8_t ManufactureDate[8]; //	UINT8[8]	-	Manufacture Date	R/W
uint32_t SN; //	UINT32	-	Serial number	R/W
uint8_t ManufacturerName[8]; //	CHAR[8]	-	Vendor Name	R/W
uint8_t DeviceName[8]; //	CHAR[8]	-	Device Name	R/W
uint8_t ChemistryId[8]; //	CHAR[8]	-	Cell ID	R
uint8_t MaxPower[8]; //[8]	CHAR[8]	-	Max Power	R
uint8_t AfeIC[8]; //[8]	CHAR[8]	-	AFE IC	R
uint32_t DesignVoltage; //	UINT32	mV	Design Voltage	R
uint32_t ChargingCurr; //	UINT32	mA	Charging Curr	R
uint32_t ChargingVolt; //	UINT32	mV	Charging Volt	R
uint8_t ParallelCellNumbers; //	UINT8	-	Cell numbers	R/W
uint32_t PackID; //	UINT8	-	ID	R
uint16_t CANBitRate	; //UINT16	kbps		R/W
//index 0x6E80 sub_index:3
uint16_t Balancing; //	UINT16	-	Balancing status	R
uint16_t cv[14];
/*nie
uint16_t cv_1; // UINT16	mV		R
uint16_t cv_2; // UINT16	mV		R
uint16_t cv_3; // UINT16	mV		R
uint16_t cv_4; // UINT16	mV		R
uint16_t cv_5; // UINT16	mV		R
uint16_t cv_6; // UINT16	mV		R
uint16_t cv_7; // UINT16	mV		R
uint16_t cv_8; // UINT16	mV		R
uint16_t cv_9; // UINT16	mV		R
uint16_t cv_10; // UINT16	mV		R
uint16_t cv_11; // UINT16	mV		R
uint16_t cv_12; // UINT16	mV		R
uint16_t cv_13; // UINT16	mV		R
uint16_t cv_14; // UINT16	mV		R
*/
//index 0x6E80 sub_index:4
uint32_t PackVolt; //	UINT32	mV
uint32_t BatteryVolt; //	UINT32	mV
int32_t Current; //	INT32	mA charge:>0 discharge:<0
int32_t AvgCurr; //	INT32	mA
int32_t RC; //	INT32; //	mAS
int32_t FCC; //	INT32	mAS
uint16_t AverageTimeToEmpty; //	UINT16	min
int32_t DeltaV; //	INT32	mV
int32_t Delta_Bus_Bat_V; //	INT32	mV
int32_t RSOC; //	INT32	0.1%

//Sub-Index=5:Realtime Temperature				
REAL32	ct_1; //[0] FLOAT	?	T-Cell 1	R
REAL32	ct_2; //Temp[1] FLOAT	?	T-Cell 2	R
REAL32	ct_3; //Temp[2] FLOAT	?	T-Cell 3	R
REAL32	t_fet; //Temp[3] FLOAT	?	T-FET	R
REAL32	t_pfet; //[4] FLOAT	?	T-Pre_R R
REAL32	t_ambient; //[5] FLOAT	?	T-Ambient	R
//Sub-Index=6:Criteria of Released and Set up				
int32_t	COCI; //	INT32	mA	OCP (Over Current Protection)Threshold	R/W
uint32_t VPV; //	UINT32	mV	OVP (Over Voltage Protection)Threshold	R/W
uint32_t 	OVPRV; //	UINT32	mV	OVP (Over Voltage Protection) Recovery	R/W
int32_t 	DOCI; //	INT32	mA	OCP threshold	R/W
uint32_t 	UVPV; //	UINT32	mV	UVP  threshold	R/W
uint32_t 	UVPRV; //	UINT32	mV	UVP Recovery Voltage	R/W
uint32_t 	SCVolt; //	UINT32	mV		R/W
REAL32	COTT; //	FLOAT	?	OTP (Over temperature Protection) Chg Threshold R/W
REAL32	CUTT; //	FLOAT	?	UTP ( Under Temperature Protection ) Chg Threshold	R/W
REAL32	DOTT; //	FLOAT	?	OTP Dsg Threshold	R/W
REAL32	DUTT; //	FLOAT	?	UTP Dsg Threshold	R/W
REAL32	FETOT; //	FLOAT	?	Power MOSFET OTP Threshold	R/W
REAL32	RecoverT; //	FLOAT	?	OTP Recovery	R/W
uint32_t	TaperWindowH; //	UINT32	mA	Current windows  if fully charging , current should not be higher than this criterion	R/W
uint32_t	TaperWindowL; //	UINT32	mA	Current windows  if fully charging , current should not be lower than this criterion	R/W
uint32_t	Pusht; //	UINT32	mS	Power key Time	R/W
uint32_t	TaperVpack; //	UINT32	mV	Voltage FC Threshold	R/W
REAL32	FDTH; //	FLOAT	%	SOC=?% ->set up FD	R/W
REAL32	FDRls; //	FLOAT	%	SOC =?% then Recovery FD	R/W
int32_t	Idlel; //	INT32	mA	System In Idle Current	R/W
uint32_t	TaperTime; //	UINT32	mS	 Full Charge Time	R/W
REAL32	ShortRiskThreshold; //	FLOAT	%	If ( (Vpack/VbattMCU)< Threshold ) when"Pre_DSG" flag On, then can into "Running" Mode. 	R/W
uint32_t	ButtonBlackStart; //	UINT32	ms	Time Threshold(T). If button be pressed Time>T,then "black_start".	R/W
uint32_t	PreDsgt; // UINT32	ms	Control PreDFET Time Threshold. R/W
uint32_t	OVPt; //	UINT32	mS	OVP Time	R/W
uint32_t	OVP2t; //	UINT32	mS	PF of OVP Time	R/W
uint32_t	UVPT; //	UINT32	mS	UVP Time	R/W
uint32_t	UVTimerT; //	UINT32	mS	When [UV] is set in Safety Status, how  long get into Sleep Mode.	R/W
uint32_t	TestTimest; //	UINT32	mS	Time. Enter Sleep Mode from Test Mode	R/W
uint32_t	OCPt; //	UINT32	mS	COCP/DOCP Time	R/W
uint32_t	OCP2t; //	UINT32	mS	Time. PF of CFET/DFET	R/W
uint32_t	OCPRlst; // UINT32	mS	COCP/DOCP Recovery	R/W
uint32_t	OTPt; //	UINT32	mS	OTP/UTP R/W
uint32_t	OTP2t; //	UINT32	mS	OTP how long takes PF	R/W
uint32_t	Runt; //	UINT32	mS	Delay Time	R/W
uint32_t	Idlet; //	UINT32	S	Idle Time	R/W
uint32_t	IMBt; //	UINT32	S	Imbalancing Time	R/W
uint32_t	SwBlnct; // UINT32	S	Time, Switch Balance between Two Balancing modules. R/W
uint32_t	OptBlncTime; // UINT32	Min Balance operating time. If OptBlncTime is set to 0 then this function is disabled.	R/W
REAL32	IdealIntR; //	FLOAT	m?	EL40HEP Battery Internal Resistor	R/W
int32_t	BlncTopVTH; //	INT32	mV	Balancing criterion of high volt	R/W
int32_t	BlncBotVTH; //	INT32	mV	Balancing criterion of Low volt R/W
int32_t	MaxImblncDltV; //	INT32	mV	Max DeltaV for imbalancing ( Vcell(n) - MinVCell )	R/W
int32_t	MinImblncDltV; //	INT32	mV	Min DeltaV for imbalancing ( Vcell(n) - MinVCell )	R/W
int32_t	StpBlncDltV; // INT32	mV	Force balancing to stop because of PF ( could not balance forever)	R/W
int32_t	IMBV; //	INT32	mV	Voltage Threshold:could not balance forever R/W
#if 0
//			 Sub-Index=7:Correction Parameter				
		CC_Rsense	FLOAT	-	Cal.Current R/W
		CC_Offset	FLOAT	mA		R/W
		Th1_Offset	FLOAT	?	Cal.Temp	R/W
		Th2_Offset	FLOAT	?		R/W
		Th3_Offset	FLOAT	?		R/W
		BusV_Offset FLOAT	mV	Cal.BusV	R/W
		BusV_Gain	FLOAT	mV/bit		R/W
		BattV_Offset	FLOAT	mV	Cal.BattV	R/W
		BattV_Gain	FLOAT	mV/bit		R/W
		DeadBand	UINT32	mA		R/W
//			 Sub-Index=8:AFE Chip Function				
		AFE_UV	UINT32	mV	AFE Chip function	R/W
		AFE_UVt UINT32	S		R/W
		AFE_OV	UINT32	mV		R/W
		AFE_OVt UINT32	S		R/W
		AFE_SCP UINT32	-	HEX(8)	R/W
		AFE_DOCP	UINT32	-	HEX(8)	R/W
//			Sub-Index=9:GAUGE				
		Dod0	INT32	0.1%	ex:When data=926 is mean 92.6%	R
		Qmax	INT32	mAS 	R
		QLRatio INT32	0.1%		R
		PreviosFlagFC	INT32	bit 	R
		PreviosFlagUV	INT32	bit 	R
		TotalChgCap INT32	mAH 	R
		TotalDsgCap INT32	mAH 	R
		Fcc INT32	mAS 	R
		KeepingCap	INT32	mAS 	R/W
		RatedMaxLoad	INT32	mA		R/W
		LearningPointV	INT32	mV		R/W
		LearningPointT	INT32	s		R/W
		DodFlatHi	INT32	0.1%		R/W
		 DodFlatLo	INT32	0.1%		R/W
		DodMinVar	INT32	0.1%		R/W
		CycleTh INT32	mAS 	R/W
		DesignCap	INT32	mAS 	R/W
		EDV INT32	mV		R/W
		Usoc	INT32	0.1%		R
		 Usoc_Th	INT32	0.1%		R/W
		PreCapRatio INT32	0.1%		R/W
//			Sub-Index=10:GAUGE Real Time Info.				
		Dod0	INT32	0.1%	ex:When data=926 is mean 92.6%	R
		Qmax	INT32	mAS 	R
		QLRatio INT32	0.1%		R
		PreviosFlagFC	INT32	bit 	R
		PreviosFlagUV	INT32	bit 	R
		TotalChgCap INT32	mAH 	R
		TotalDsgCap INT32	mAH 	R
		Fcc INT32	mAS 	R
		KeepingCap	INT32	mAS 	R/W
		RatedMaxLoad	INT32	mA		R/W
		LearningPointV	INT32	mV		R/W
		LearningPointT	INT32	s		R/W
		DodFlatHi	INT32	0.1%		R/W
		 DodFlatLo	INT32	0.1%		R/W
		DodMinVar	INT32	0.1%		R/W
		CycleTh INT32	mAS 	R/W
		DesignCap	INT32	mAS 	R/W
		EDV INT32	mV		R/W
		Usoc	INT32	0.1%		R
		 Usoc_Th	INT32	0.1%		R/W
		PreCapRatio INT32	0.1%		R/W

//			Sub-Index=11:Batter Record				
		QIRatio INT32	0.1%		R
		QTRatio INT32	0.1%		R
		ActualRC	INT32	mAS 	R
		DeltaRC INT32	mAS 	R
		NewFCC	INT32	mAS 	R
		DischargeCap	INT32	mAS 	R
		KeepingCap	INT32	mAS 	R
		PassedCharge	INT32	mAS 	R
		OCVqualifiedTime	UINT16	Sec 	R
		LearnPointTime	UINT8	Sec 	R
		AvgC-Rate	INT32	C	ex:Data=100 that is mean 0.1C	R
		C-Rate	INT32	C		R
		gMaxVcell	UINT16	mV		R
		gMinVcell	UINT16	mV		R
		Add15xCurrent	INT32	mA		R
		Flash_DoD	INT32	0.1%		R
		Flash_Qmax	INT32	mAS 	R
		Flash_QLRatio	INT32	%		R
		Flash_Total_DsgCap	INT32	0.01mAH 	R
		Flash_Fcc	INT32	mAS 	R
//			Sub-Index=12:User Environment				
		Flag_Uvp_Count	UINT32	times	Batpeak Counting	R
		Flag_Ovp_Count	UINT32	times		R
		Flag_Doc_Count	UINT32	times		R
		Flag_Coc_Count	UINT32	times		R
		Flag_Cut_Count	UINT32	times		R
		Flag_Cot_Count	UINT32	times		R
		Flag_Dot_Count	UINT32	times		R
		Flag_Dut_Count	UINT32	times		R
		Flag_Sc_Count	UINT32	times		R
		One_Flag	UINT32	bits	Alert Record	R
		One_Current INT32	mA	One series data ( The newest )	R
		One_Temp	FLOAT	?		R
		One_Vcell[0]	INT32	mV		R
		…	INT32	mV		R
		One_Vcell[13]	INT32	mV		R
		TWO_Flag	UINT32	bits	Two series data ( newer )	R
		TWO_Current INT32	mA		R
		TWO_Temp	FLOAT	?		R
		TWO_Vcell[0]	INT32	mV		R
		…	INT32	mV		R
		TWO_Vcell[13]	INT32	mV		R
		Three_Flag	UINT32	bits	Three series data (new) R
		Three_Current	INT32	mA		R
		Three_Temp	FLOAT	?		R
		Three_Vcell[0]	INT32	mV		R
		…	INT32	mV		R
		Three_Vcell[13] INT32	mV		R
		PFFlag	UINT32	bits	PF Alert	R
		Current INT32	mA		R
		Temp	FLOAT	?		R
		Vcell[0]	INT32	mV		R
		…	INT32	mV		R
		Three_Vcell[13] INT32	mV		R
//			Sub-Index=13:OCV DOD Info.				
		OcvTmpTable[0]	int32	0.01?	ex:When data=2502 is mean 25.02?	R/W
		OcvTmpTable[1]	int32	0.01?		R/W
		OcvTmpTable[2]	int32	0.01?		R/W
		OcvTmpTable[3]	int32	0.01?		R/W
		OcvTable[0][0]	int32	mV		R/W
		OcvTable[0][1]	int32	mV		R/W
		OcvTable[0][2]	int32	mV		R/W
		OcvTable[0][3]	int32	mV		R/W
		OcvTable[1][0]	int32	mV		R/W
		OcvTable[1][1]	int32	mV		R/W
		OcvTable[1][2]	int32	mV		R/W
		OcvTable[1][3]	int32	mV		R/W
		OcvTable[2][0]	int32	mV		R/W
		OcvTable[2][1]	int32	mV		R/W
		OcvTable[2][2]	int32	mV		R/W
		OcvTable[2][3]	int32	mV		R/W
		…	int32	mV		R/W
		OcvTable[14][3] int32	mV		R/W
		DodTable[0] int32	%		R/W
		DodTable[1] int32	%		R/W
		DodTable[2] int32	%		R/W
		DodTable[3] int32	%		R/W
		…	int32	%		R/W
		DodTable[14]	int32	%		R/W
//			Sub-Index=14:QTQIDC Ratio				
		OcvTmpTable[0]	int32	0.01?	ex:When data=2502 is mean 25.02?	R/W
		OcvTmpTable[1]	int32	0.01?		R/W
		OcvTmpTable[2]	int32	0.01?		R/W
		OcvTmpTable[3]	int32	0.01?		R/W
		OcvTable[0][0]	int32	mV		R/W
		OcvTable[0][1]	int32	mV		R/W
		OcvTable[0][2]	int32	mV		R/W
		OcvTable[0][3]	int32	mV		R/W
		OcvTable[1][0]	int32	mV		R/W
		OcvTable[1][1]	int32	mV		R/W
		OcvTable[1][2]	int32	mV		R/W
		OcvTable[1][3]	int32	mV		R/W
		OcvTable[2][0]	int32	mV		R/W
		OcvTable[2][1]	int32	mV		R/W
		OcvTable[2][2]	int32	mV		R/W
		OcvTable[2][3]	int32	mV		R/W
		…	int32	mV		R/W
		OcvTable[14][3] int32	mV		R/W
		DodTable[0] int32	%		R/W
		DodTable[1] int32	%		R/W
		DodTable[2] int32	%		R/W
		DodTable[3] int32	%		R/W
		…	int32	%		R/W
		DodTable[14]	int32	%		R/W
//			Sub-Index=15:AFE Info				
		Pack_Voltage_ADC	UINT32		HEX R
		Batt_Voltage_ADC	UINT32		HEX R
		Curr_I2C	INT32		HEX R
//			Sub-Index=16:AFE Cell Voltage Info				
		CellV_ADC[0]	UINT16	-	Dec.	R
		…	UINT16	-	Dec.	R
		CellV_ADC[13]	UINT16	-	Dec.	R
//			Sub-Index=17:Cell Voltage of Correction 				
		CellV_ADC[0]	UINT16	-	Dec.	R
		…	UINT16	-	Dec.	R
		CellV_ADC[13]	UINT16	-	Dec.	R
//			Sub-Index=18:Cell Voltage of Compensative				
		CellV_Offset[0] FLOAT	mV		R/W
		…	FLOAT	mV		R/W
		CellV_Offset[13]	FLOAT	mV		R/W
		CellV_Gain[0]	FLOAT	mV/bit		R/W
		…	FLOAT	mV/bit		R/W
		CellV_Gain[13]	FLOAT	mV/bit		R/W

#endif
UNS8 alive; // 1: bmu is broading CAN message. 0: slience
UNS8 aliveBuf; //
INT16 ct[14]; // 0.1C
CELL_BITSTAT cbCtrl;
CELL_BITSTAT cbStat;
CELL_BITSTAT cvv;
CELL_BITSTAT ctv;



}BMU_INFO;

extern BMU_FLAG bmuFlag[NUM_BMU_TOTAL];
extern BMU_INFO bmu[NUM_BMU_TOTAL];
#endif
