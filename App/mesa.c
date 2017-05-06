/***********************************************************************
***********************************************************************/
#include "main.h"
#include "def.h"
#include "mesa.h"
#include "task_main.h"
OS_STK Stk_Task_Mesa[TASK_MESA_STK_SIZE];
MESA_DATA_801 mesa_801;
MESA_DATA_802 mesa_802;
MESA_DATA_803 mesa_803;
extern void fillMbReg(uint16_t dstAddr,uint16_t * ptData,uint8_t len);

static void mesaVarInit(void);
static void mesaVarUpdata(void);
void Task_Mesa(void *pdata);
void Task_Mesa(void *pdata){
	int lp;
	mesaVarInit();
	while(1){
		OSTimeDly(100);//100ms
		
		mesaVarUpdata();
		lp++;
	}
}
static void mesaVarUpdata(void){
	
	mesa_801.ID_801=801; //	Energy Storage Base Model	801 uint16
	mesa_801.L_801=22; //		22	uint16
	mesa_801.DERTyp=91; //	Distributed Energy Resource Type		enum16
	mesa_801.WHRtg=23; //	Nameplate Energy Capacity		uint16
	mesa_801.WMaxChaRte=30; //	Nameplate Max Charge Rate		uint16
	mesa_801.WMaxDisChaRte=44; //	Namplate Max Discharge Rate 	uint16
mesa_801.DisChaRte=-1; //	Self Discharge Rate 	uint16
	mesa_801.SoCNpMaxPct=100; // Nameplate Max SoC		uint16
	mesa_801.SoCNpMinPct=0; // Nameplate Min SoC		uint16
	mesa_801.MaxRsvPct=60; //	Maximum Reserve Percent 	uint16
	mesa_801.MinRsvPct=30; //	Minimum Reserve Percent 	uint16
	mesa_801.SoC=bpInfo.bp_soc/10; // State of Charge 	uint16
	mesa_801.ChaSt=1; //	ChaSt		enum16
	mesa_801.LocRemCtl=1; //	Control Mode		enum16
	mesa_801.Evt=0; //t Event Bitfield		bitfield32
	mesa_801.DERHb=1; //	Distributed Energy Resource Heartbeat		uint16
	mesa_801.ControllerHb^=1; //	Controller Heartbeat		uint16
	mesa_801.DERAlarmReset=0; //	Alarm Reset 	uint16
	mesa_801.WHRtg_SF=bpInfo.bp_soc/10; //			sunssf
	mesa_801.WMaxChaDisCha_SF=1; //			sunssf
	mesa_801.DisChaRte_SF=0; //			sunssf
	mesa_801.SoC_SF=0; //			sunssf
	mesa_801.Pad=0x8000; // 		pad
	
	mesa_802.ID_802=802;	//	Battery Base Model	802	uint16
	mesa_802.L_802=20;		//		20	uint16
				
	mesa_802.BatTyp=4;	//	Battery Type;		//		enum16
	mesa_802.BatSt=1;	//	State of the Battery Bank;		//		enum16 1:disconnected 3:connected
	mesa_802.CycleCt=1;		//	Cycle Count		uint32
	mesa_802.SoH=bpInfo.bp_soh/10;		//	State of Health		uint16
	mesa_802.Evt1=0;	//	Battery Event 1;		// Bitfield		bitfield32
	mesa_802.Evt2=0;	//	Battery Event 2;		// Bitfield		bitfield32
	mesa_802.Vol=bpInfo.bp_VBus/100;		//	External Battery Voltage		uint16
	mesa_802.MaxBatACha=bpInfo.bp_clc;		//	Max Charge Current		uint16
	mesa_802.MaxBatADischa=bpInfo.bp_cld;		//	Max Discharge Current		uint16
	mesa_802.BatReqPCSSt=0;		//	PCS State Request		enum16
	mesa_802.BatReqW=100;		//	Battery Power Request		int16
	mesa_802.BSetOperation=0;		//	Set Operation		enum16
	mesa_802.BSetPCSState=0;		//	PCS State		enum16
	mesa_802.SoH_SF=-1;		//			sunssf
	mesa_802.Vol_SF=-1;		//			sunssf
	mesa_802.MaxBatA_SF=-1;		//			sunssf
	mesa_802.BatReqW_SF=-1;		//			sunssf

	mesa_803.BMinCellVol=strInfo[0].s_cv_min;
	mesa_803.BMaxCellVol=strInfo[0].s_cv_max;
	
	mesa_803.ID_803=803;		//		Lithium-Ion Battery Model	803 uint16			R	M		
	mesa_803.L_803=16; 	//			16	uint16			R	M	Model Length	
										
	mesa_803.BConStrCt=1; 	//		Connected String Count		uint16			R	M	Number of strings with contactor closed.	
	mesa_803.BMaxCellVol=strInfo[0].s_cv_max;		//		Max Cell Voltage		uint16	V	BCellVol_SF R	M	Maximum voltage for all cells in the bank.	Measurement
	mesa_803.BMaxCellVolLoc=((uint16_t)(strInfo[bpInfo.sid_cvmax].s_cvmax_cid+1)<<8)+(bpInfo.sid_cvmax+1);		//		Max Cell Voltage Location		uint16			R	O	Location of the cell with maximum voltage	Bit0:Bit7 = String Number  Bit8:Bit15 = Module Number
	mesa_803.BMinCellVol=strInfo[0].s_cv_min;		//		Min Cell Voltage		uint16	V	BCellVol_SF R	M	Minimum voltage for all cells in the bank.	Measurement
	mesa_803.BMinCellVolLoc=((uint16_t)(strInfo[bpInfo.sid_cvmin].s_cvmin_cid+1)<<8)+(bpInfo.sid_cvmin+1);		//		Min Cell Voltage Location		uint16			R	O	Location of the cell with minimum voltage	Bit0:Bit7 = String Number  Bit8:Bit15 = Module Number
	mesa_803.BMaxModTmp=strInfo[0].s_ct_max/10; 	//		Max Module Temp 	int16		BModTmp_SF	R	M	Maximum temperature for all modules in the bank.	Measurement
	mesa_803.BMaxModTmpLoc=((uint16_t)(bpInfo.sid_ctmax+1)<<8)+(strInfo[bpInfo.sid_ctmax].s_ctmax_cid+1); 	//		Max Module Temp Location		uint16			R	O	Location of the module with max temperature.	Bit0:Bit7 = String Number  Bit8:Bit15 = Module Number
	mesa_803.BMinModTmp=strInfo[0].s_ct_min/10; 	//		Min Module Temp 	int16	C	BModTmp_SF	R	M	Minimum temperature for all modules in the bank.	Measurement
	mesa_803.BMinModTmpLoc=((uint16_t)(bpInfo.sid_ctmin+1)<<8)+(strInfo[bpInfo.sid_ctmin].s_ctmin_mid+1); 	//		Min Module Temp Location		uint16			R	O	Location of the module with min temperature.	Bit0:Bit7 = String Number  Bit8:Bit15 = Module Number
	//mesa_803.BTotDCCur=strInfo[0].mod_curr;		//		Total DC Current		int16	A	BCurrent_SF R	M	Total DC current flowing to/from the battery bank.	Measurement
	mesa_803.BTotDCCur=bpInfo.bp_curr/NUM_STR/10;		//		Total DC Current		int16	A	BCurrent_SF R	M	Total DC current flowing to/from the battery bank.	Measurement
	mesa_803.BMaxStrCur=bpInfo.bp_curr/NUM_STR/10; 	//		Max String Current		int16	A	BCurrent_SF R	O	Maximum current of any string in the bank.	Measurement
	mesa_803.BMinStrCur=bpInfo.bp_curr/NUM_STR/10; 	//		Min String Current		int16	A	BCurrent_SF R	O	Minimm current of any string in the bank.	Measurement
	mesa_803.BCellVol_SF=-3;		//				sunssf			R	M	Scale factor for cell voltage.	
	mesa_803.BModTmp_SF=0;		//				sunssf			R	M	Scale factor for module temperatures.	
	mesa_803.BCurrent_SF=-2;		//				sunssf			R	M	Scale factor for Total DC Current, Max String Current and Min String Current.	
	mesa_803.StrSoH_SF=0; 	//				sunssf			R	O	Scale factor for String State of Health.	
										
	mesa_803.StrModCt=NUM_BMU_TOTAL;		//		Module Count		uint16			R	M	Count of modules in the string. 
	mesa_803.StrSoC=strInfo[0].mod_soc/10;		//		String SoC		uint16	%		R	M	Battery string state of charge, expressed as a percentage.	Measurement
	mesa_803.StrSoH=strInfo[0].mod_soh/10;		//		String SoH		uint16	%	StrSoH_SF	R	O	Battery string state of health, expressed as a percentage.	Measurement
	mesa_803.StrCur=bpInfo.bp_curr/NUM_STR/10; 	//		String Current		int16	A	BCurrent_SF R	M	String current measurement. Measurement
	mesa_803.StrMaxCellVol=strInfo[0].s_cv_max; 	//		Max Cell Voltage		uint16	V	BCellVol_SF R	M	Maximum voltage for all cells in the string.	Measurement
	mesa_803.StrMinCellVol=strInfo[0].s_cv_min; 	//		Min Cell Voltage		uint16	V	BCellVol_SF R	M	Minimum voltage for all cells in the string.	Measurement
	mesa_803.StrCellVolLoc=((uint16_t)(strInfo[bpInfo.sid_vmin].s_cvmin_mid+1)<<8)+(strInfo[bpInfo.sid_vmax].s_cvmax_mid+1); 	//		Max/Min Cell Voltage Loc		uint16			R	O	Location of maximum and minimum cell voltages.	Bit0:Bit7 = Max Module Num	Bit8:Bit15 = Min Module Num
	mesa_803.StrMaxModTmp=strInfo[0].s_ct_max/10;		//		Max Module Temp 	int16	C	BModTmp_SF	R	M	Maximum temperature for all modules in the bank.	Measurement
	mesa_803.StrMinModTmp=strInfo[0].s_ct_min/10;		//		Min Module Temp 	int16	C	BModTmp_SF	R	M	Minimum temperature for all modules in the bank.	Measurement
	mesa_803.StrModTmpLoc=((uint16_t)(strInfo[bpInfo.sid_ctmin].s_ctmin_mid+1)<<8)+(strInfo[bpInfo.sid_ctmax].s_ctmax_mid+1);		//		Max/Min Mod Temp Loc		uint16			R	O	Location of maximum and minimum module temperatures.	Bit0:Bit7 = Max Module Num	Bit8:Bit15 = Min Module Num
	mesa_803.StrEvt1;		//		String Event 1; 	//			bitfield32			R	M	Alarms, warnings and status values.  Bit flags. 
	mesa_803.StrEvt2;		//		String Event 2		bitfield32			R	O	Alarms, warnings and status values.  Bit flags. Reserved for future use.
	mesa_803.StrConFail;		//		Connection Failure Reason		enum16			R	O		
	fillMbReg(70,(uint16_t *)&mesa_801,sizeof(mesa_801));
	fillMbReg(94,(uint16_t *)&mesa_802,sizeof(mesa_802));
	fillMbReg(116,(uint16_t *)&mesa_803,sizeof(mesa_803));
}
static void mesaVarInit(void){
	uint16_t tmp[40];
	uint8_t i;
	for(i=0;i<40;i++){tmp[i]=i;}
	memcpy((uint16_t *)&mesa_801,(uint16_t *)tmp,sizeof(mesa_801));
	memcpy((uint16_t *)&mesa_802,(uint16_t *)tmp,sizeof(mesa_802));
	memcpy((uint16_t *)&mesa_803,(uint16_t *)tmp,sizeof(mesa_803));
}
	
