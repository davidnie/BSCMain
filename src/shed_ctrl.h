#ifndef __SCHNIDER_H
#define __SCHNIDER_H

extern void schnider_ctrl(void);
extern void schnider_parse(J1939_MESSAGE *msg);
typedef uint8_t bool;
typedef uint32_t time_t;
typedef struct{
	uint8_t nodeAddr;  //byte;
	uint32_t uniqueNumber;  //DWORD;
	uint16_t manufactureCode;  //WORD;
	uint8_t devInstLower;  //byte;
	uint8_t devInstUpper;  //byte;
	uint8_t devFunc;  //byte;
	uint8_t rev;  //byte;
	uint8_t devClass;  //byte;
	uint8_t sysInst;  //byte;
	uint8_t induGroup;  //byte;
	uint8_t sefConf;  //byte;

}X_NODE;
typedef struct{
	uint8_t sndId;
	uint8_t cfgType;
	uint8_t msgCnt;
	uint8_t conCurAccCtr;
	uint8_t dcSrcId;
	uint8_t battType;

	uint32_t desiredBulkV;
	uint32_t desiredBulkVMin;
	uint32_t desiredBulkVMax;

	int32_t desiredBulkI;
	int32_t desiredBulkIMin;
	int32_t desiredBulkIMax;
	uint32_t bulkTermV;
	uint32_t bulkTermVmin;
	uint32_t bulkTermVmax;
	uint16_t bulkTermTime;
	uint16_t bulkTermTimeMin;
	uint16_t bulkTermTimeMax;
	uint16_t bulkPeriodTo;
	uint16_t bulkPeriodToMin;
	uint16_t bulkPeriodToMax;

}CHG_CFG_BULK_STRUCT;
extern CHG_CFG_BULK_STRUCT chgCfgBulk[3];
typedef struct{
uint8_t inv_num;  //byte;
uint8_t bcm_id_min;  //byte; (*keep id value of BCM*)
uint8_t bcm_id_max;  //byte; (*keep max BCM id*)
uint8_t chg_cmd;  //byte; (* 1-start charge, 0- stop charge *)

uint8_t bcm_num_total;  //byte;
uint8_t bcm_num_online;  //byte;
uint8_t bcm_num_offline;  //byte;
uint8_t bcm_num_fault;  //byte;
uint8_t bcm_num_alive;  //byte;
uint16_t s_soc;  //UINT;  (*0.1%*)
uint16_t s_soh;  //UINT;(*0.1%*)
float s_ct_min;  //REAL;(*0.1C*)
float s_ct_max;  //REAL;(*0.1C*)
uint16_t s_cv_min;  //UINT;(*mv*)
uint16_t s_cv_avg;  //UINT; (*mv*)
uint16_t s_cv_max;  //UINT; (*mv*)
uint16_t s_bv_max;  //UINT; (*0.1V*)
uint16_t s_bv_min;  //UINT; (*0.1V*)
uint16_t s_bv_avg;  //UINT; (*0.1V*)
uint16_t s_soc_min;  //UINT;  (*0.1%*)
uint16_t s_soc_max;  //UINT;  (*0.1%*)

float s_clc;  //REAL; (*A *)
float s_cld;  //REAL; (*A *)
float s_plc;  //REAL; (*kw *)
float s_pld;  //REAL; (*kw *)
float s_current;  //REAL; (*A*)
float s_busVolt;  //REAL; (*V*)
float instPow;  //REAL;		(*KW*)

float invBulkV_exp;  //REAL;
float invAbsV_exp;  //REAL;
float invChgLim_exp;  //byte; (*%*)
float sccBukV_exp;  //REAL;
float sccAbsV_exp;  //REAL;
uint8_t sccChgLim_exp;  //byte;


uint8_t cntr_stat;  //byte; 	(*readiness OF control. init:determining contactor status  ready: ready TO open/close contactors*)
uint16_t volt_block;  //UINT;	 (*measured BY CSM OF each block*)
uint8_t cntr_pos;  //byte; (*The contactor status shall be reported as open, closed, fault.  Contactor position shall be determined BY the contactor auxiliary contact.*)
uint8_t  cntr_aux_stat;  //byte;

uint8_t f_EPO;  //BOOL;
uint8_t f_EPO_Latch;  //BOOL;
uint8_t f_prechg;  //BOOL;
uint8_t InvMsgReceived;  //BOOL;
time_t InvMsgRevTime;  //TIME;

time_t sysStartTime;  //TIME;
time_t toRmtCtrlMdTime;  //TIME;
time_t toLocCANCtrlMdTime;  //TIME;

uint32_t contSwClsDur;  //DWORD; (* ms *)
uint32_t activeSwClsDur;  //DWORD; (* ms *)
uint32_t contClsDur;  //DWORD; (* ms *)
uint32_t contOpnDur;  //DWORD; (* ms *)

uint16_t charge_capacity;  //WORD;	(*0.1Ah*)
uint16_t discharge_capacity;  //WORD; (*0.1Ah*)

float discharge_capacity_kwh;  //REAL;(*0.1Kwh*)
float charge_capacity_kwh;  //REAL;	(*0.1Kwh*)
float nor_capacity_kwh;  //REAL:=12.5;	(*Kwh*)
float nor_capacity_AHr;  //REAL:=30;	(*AHr*)

uint8_t RL1400_MB_ADDR;  //byte;

uint8_t IFM_MB_ADDR;  //byte;

uint8_t DC_OV_F;  //BOOL;
uint8_t DC_OC_F;  //BOOL;
uint8_t DC_Lose_F;  //BOOL;
uint8_t AC_lose_F;  //BOOL;
uint8_t lose_str_fault;  //BOOL;
uint8_t CAN_LocCtrl_Reved;  //BOOL;
}BCM_SYS_INFO;
typedef enum{
	SC_INIT,	//(*/system controller will enter this after power up*)
	SC_ACTIVE_SWITCH,
	SC_BCM_CHECK,
	SC_READY_ALL_OPEN,
	SC_BATT_CLOSED,
	SC_BCM_ALL_CLOSEED,
	SC_READY_CLOSE,
	SC_CLS_CNTR,
	SC_CLS_CNTR_NEG,

	SC_MAIN_CLOSED,
	SC_OPEN_TRANSIT,
	SC_FAULT,
	SC_TRANSIT_TO_OPEN,
	SC_BCM_TRANSIT_TO_CLS,

	INIT,
	READY,
	PRE_CHARGE,
	RUNNING,
	GRACE_SD,
	WARNING,
	FAIL_OUT,
	MANUAL,
	ZERO_LOAD,
	ABB_START,
	HARD_SD
}BATT_STATE;

typedef struct{
 	int32_t strCurr;  //DINT;	(* 0.001A*)
	uint16_t strVolt;  //WORD; (*0.01V measured BY CSM OF each block*)
	uint16_t strSOC;  //WORD; (*0.1%*)
	uint16_t strSOH;  //WORD; (*0.1%*)
	uint16_t cv_min;  //WORD;
	uint16_t cv_max;  //WORD;
	uint16_t cv_avg;  //WORD;
	int16_t ct_min;  //INT;(*0.1C*)
	int16_t ct_max;  //INT; (*0.1C*)
	uint32_t strClc;  //DWORD;  (* 0.01*)
	uint32_t strCld;  //DWORD;  (* 0.01*)
	uint16_t strVoltMeas;  //WORD; (*V*)
	uint8_t strRlStat;  //byte;	(* 0:init, 1: ReadyOpen 2:ReadyClose 3:Fault 4:FaultOpen *)
	uint8_t epo;  //BOOL;
	uint8_t hvil;  //BOOL;
	uint8_t actv;  //BOOL;(* The contactor status shall be reported as open, closed, fault.  Contactor position shall be determined BY the contactor auxiliary contact.*)

	uint8_t bcm_dtc_statusMask;  //byte;
	uint8_t bcm_dtc_fault_detected;  //byte;
	uint8_t bcm_dtc_pages;  //byte;
	uint8_t bcm_dtc_testFailedThisOp;  //byte;
	uint8_t bcm_dtc_type1;  //byte;
	uint8_t bcm_dtc_type2;  //byte;
	uint8_t bcm_dtc_type3;  //byte;

	bool CANReceived;  //BOOL;
	time_t CANRevTime;  //TIME;
	bool f_lose_comm;  //BOOL;
	bool f_cell_ov;  //BOOL;
	bool f_cell_uv;  //BOOL;
	bool f_cell_ov_serious;  //BOOL;
	bool f_cell_ot;  //BOOL;
	bool f_cell_ut;  //BOOL;
	bool f_oc_dchg;  //BOOL;
	bool f_oc_chg;  //BOOL;
	bool f_lose_mbb;  //BOOL;
	bool f_lose_csm;  //BOOL;
	bool f_cntr_cls_fail;  //BOOL;
	bool f_cntr_weld_fail;  //BOOL;
	bool f_cnt_open_fail;  //BOOL;
	bool f_mbb_ov_ot;  //BOOL;
	bool f_pwr_ov;  //BOOL;
	bool f_hvil;  //BOOL;
	bool w_cell_ov;  //BOOL;
	bool w_cell_uv;  //BOOL;
	bool w_cell_ot;  //BOOL;
	bool w_cell_ut;  //BOOL;
}BCM_INFO ;
typedef struct{
	uint8_t cfgTyep;  //byte;
	uint8_t msgCnt;  //byte;
	uint8_t conCurAccCtr;  //byte;
	uint8_t dcSrcId;  //byte;
	uint8_t battType;  //byte;
	uint16_t maxDischargeI;  //WORD; (*precentage*)
	float maxDischargeIMax;  //REAL;
	float maxDischargeIMin;  //REAL;
	uint16_t maxDchgTimeInt;  //WORD;
	uint16_t maxDchgTimeIntMax;  //WORD;
	uint16_t maxDchgTimeIntMin;  //WORD;
}BMS_CFG;
	typedef struct{ //	(*0x1F029 BMSSts*)
		uint8_t msgCnt;  //byte;
		uint8_t dcSrcId;  //byte;
		uint8_t rsvBits;  //byte;
		float readbackI;  //REAL; (*A*)
		float readbackV;  //REAL;(*V*)
		float referenceI;  //REAL; (*A*)
		float referenceV;  //REAL; (*V*)
}BMS_STS;

typedef struct{
	uint8_t sndId;  //byte;
	uint8_t battTempNoSensor;  //byte;
	uint8_t cfgType;  //byte;
	uint8_t chgAlgorithm;  //byte;
	uint8_t chgILimit;  //byte;
	uint8_t chgILimitMax;  //byte;
	uint8_t chgILimitMin;  //byte;
	uint8_t chgMode;  //byte;
	uint8_t chgOverriideEnDis;  //byte;
	uint8_t concurAccCtr;  //byte;
	uint8_t DcSrcId;  //byte;
	uint8_t eqtzOneTimeEnDis;  //byte;
	uint8_t msgCount;  //byte;
	uint8_t overChgEnDis;  //byte;
	uint8_t rsvBits;  //byte;
	uint8_t chgILimit_now;  //byte;
}CHG_CFG;
typedef struct{
	int32_t CurrentMax;  //DINT;  (*mA*)
	uint8_t DcSrcld;  //byte;
	uint32_t Energy;  //DWORD; (*WH*)
	uint8_t MsgCount;  //byte;
	uint16_t PeakPower;  //WORD; (*W*)
	uint8_t PreviousPeriod;  //byte;
	uint8_t PreviousPeriodCount;  //byte;
	uint8_t RsvBits;  //byte;
	int32_t VoltageMax;  //DINT; (*mV*)
	int32_t VoltageMin;  //DINT; (*mV*)
}CHG_STAT;
typedef struct{
	uint8_t sndId;  //byte;
	uint8_t cfgType;  //byte;
	uint8_t msgCnt;  //byte;
	uint8_t conCurAccCtr;  //byte;
	uint8_t dcSrcId;  //byte;
	uint8_t battType;  //byte;
	uint32_t desiredAbsV;  //DWORD; (*mv*)
	uint32_t desiredAbsVMin;  //DWORD;(**)
	int32_t desiredAbsVMax;  //DWORD;(**)
	int32_t desiredAbsI;  //DINT; (**)
	int32_t desiredAbsIMin;  //DINT;(**)
	int32_t desiredAbsIMax;  //DINT;(**)
	uint32_t absTermI;  //DWORD; (**)
	uint32_t absTermIMin;  //DWORD;(**)
	uint32_t absTermIMax;  //DWORD;(**)
	uint16_t absTermTimeI;  //WORD; (**)
	uint16_t absTermTimeIMin;  //WORD;(**)
	uint16_t absTermTimeIMax;  //WORD;(**)
	uint16_t absTermConstV;  //WORD; (**)
	uint16_t absTermConstVMin;  //WORD;(**)
	uint16_t absTermConstVMax;  //WORD;(**)
	uint16_t absPeriodTo;  //WORD;
	uint16_t absPeriodToMin;  //WORD;
	uint16_t absPeriodToMax;  //WORD;
}chgCfgAbsorp;
typedef struct{
	uint8_t sw_ver_year;  //byte;
	uint8_t sw_ver_month;  //byte;
	uint8_t sw_ver_date;  //byte;
	uint8_t sw_ver_patch;  //byte;


	X_NODE MY_NAME; //:X_NODE;
	X_NODE MY_NAME_UI; //:X_NODE; (* to broadcast discharge power limit. BMS need claim itself as user interface*)

	uint8_t BCM_TOTAL_NUM;  //byte;
	uint16_t BCM_CAPACITY;  //WORD; (* AHr*)
	uint16_t BCM_ENERYG;  //WORD; (*0.1KWHr*)
}CONST_VAR;
typedef struct{
	uint32_t bcmContCtrl;  //DWORD;	(*bit status is bcm command *)
	bool ccmDebugEn;  //BOOL;

	uint8_t cabReqId;  //byte;
	uint8_t strReqId;  //byte;
	uint8_t cntrCmd;  //byte; (* 1-close 2-open 3-force close 4-force check neg contactor 5-leak check *)
	uint8_t sysCANSndEnbl;  //byte;
	uint8_t cabNegCntrCmd;  //byte;
	uint8_t cellBalReq;  //byte;
	uint16_t sysBusVlt;  //WORD;
	uint8_t forceClsCabId;  //byte;
	uint8_t forceClsStrId;  //byte;
	uint8_t forceBalCabId;  //byte;
	uint8_t forceBalStrId;  //byte;
	uint8_t faultClr;  //byte; 	(* 1: clear warning 2:clear fault 3: clear serious error *)

	uint16_t abbVlt;  //WORD; (*0.1V*)
	bool sysRLNegStat;  //BOOL;
	bool sysRLPosStat;  //BOOL;
	bool abbEnlRLStat;  //BOOL;
	bool sysErrorFlag;  //BOOL;
	uint8_t sysNumBal;  //byte;
	uint8_t sysNumOn;  //byte;
	uint16_t sysSOC;  //WORD;
	uint16_t sysCurrent;  //WORD;
}ESU_CMD;
typedef struct{
	bool F_BCM_ALIVE_TIMEOUT;  //BOOL;
	bool F_BCM_FAIL_CLOSE;  //BOOL;
	bool F_BCM_FAIL_OPEN;  //BOOL;
	bool F_SYS_FAULT;  //BOOL;
	bool F_CONT_FAIL_CLOSE;  //BOOL;
	bool F_CONT_FAIL_OPEN;  //BOOL;

	bool F_FLOOD;  //BOOL;
	bool F_SMOKE;  //BOOL;
	bool F_ESU;  //BOOL;
	bool F_DOOR_OPEN;  //BOOL;
	bool F_DC_ISL;  //BOOL;
	bool F_AC_ISL;  //BOOL;
	bool F_LOSE_ABB;  //BOOL;
	bool F_LOSE_PLC;  //BOOL;
	bool F_LOSE_G59;  //BOOL;
	bool F_LOSE_SSE;  //BOOL;
	bool F_G59_TRIP;  //BOOL;
	bool F_OC_STR;  //BOOL;
	bool F_OC_BUS;  //BOOL;
	bool F_OV_BUS;  //BOOL;
	bool F_PEU_1_OT;  //BOOL;
	bool F_Precharg;  //BOOL;
}FAIL_LATCH;
typedef struct{
	uint8_t nodeAddr;  //byte;
	uint32_t uniqueNumber;  //DWORD;
	uint16_t manufactureCode;  //WORD;
	uint8_t devInstLower;  //byte;
	uint8_t devInstUpper;  //byte;
	uint8_t devFunc;  //byte;
	uint8_t rev;  //byte;
	uint8_t devClass;  //byte;
	uint8_t sysInst;  //byte;
	uint8_t induGroup;  //byte;
	uint8_t sefConf;  //byte;

	uint8_t cfgTyep;  //byte;
	uint8_t msgCnt;  //byte;
	uint8_t conCurAccCtr;  //byte;
	uint8_t dcSrcId;  //byte;
	uint8_t battType;  //byte;
	uint16_t maxDischargeI;  //WORD; (*precentage*)
	float maxDischargeIMax;  //REAL;
	float maxDischargeIMin;  //REAL;
	uint16_t maxDchgTimeInt;  //WORD;
	uint16_t maxDchgTimeIntMax;  //WORD;
	uint16_t maxDchgTimeIntMin;  //WORD;

	float D_Pow_Max;  //REAL; (*KW*)
	float C_Pow_Max;  //REAL; (*KW*)
	uint16_t maxDischargeI_now;  //WORD; (*precentage. read from inverter response *)
}INV_INFO;
typedef struct{
	uint16_t nodeId;  //byte;
	uint8_t func;  //byte;
	uint16_t addr;  //WORD;
	uint16_t len;  //WORD;
	uint8_t sndD[30]; //:ARRAY[0..30] OF BYTE;
	uint8_t revD[30]; //:ARRAY[0..30] OF BYTE;
	uint16_t wPos;  //WORD;
	uint16_t rPos;  //WORD;
	bool ifReved;  //BOOL;
	bool ifSnd;  //BOOL;
}MB_MSG;
/*
typedef struct{
	ISOAck:=16#E800,
	ISOAddrClaim:=16#EE00,
	ISOCmdAddr:=16#EED8,
	ISOReq:=16#EA00,
	ISOTpConnManAbort:=16#EC00,
	ISOTpXfer:=16#EB00
}PGN_NUMBER;
*/
#endif

