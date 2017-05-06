/***********************************************************************
***********************************************************************/
#include "main.h"
#include "bsp_can.h"
#include "Task_ADC.h"
#include "Task_Main.h"
#include "Task_sysCheck.h"
#include "shed_ctrl.h"
#include "battery_gt.h"
OS_STK Stk_Task_CAN2_Rev[TASK_CAN2_REV_STK_SIZE];
OS_STK Stk_Task_CAN2_Snd2Buf[TASK_CAN2_SEND_TO_BUF_STK_SIZE];
OS_STK Stk_Task_CAN_Send[TASK_CAN_SEND_STK_SIZE];

OS_EVENT  *sem_CAN2_rec_flag;			//CAN2 receive semphere
OS_EVENT  *sem_CAN2_send_flag;			//CAN2 send semphere
CAN_DATA c2_d;
void sdIntAddrClaim(CAN_TypeDef * CANPort);
void can2Dispatch(CanRxMsg * RxMSG);
void j1939_send(CAN_TypeDef *CANx,J1939_MESSAGE *jmsg);

void		parseIntAddrClaim(CanRxMsg * RxMSG);
void		parseDebugCommand_0x131(CanRxMsg * RxMSG);
static void CAN_Bp2Vcu(void);
static void CAN_Bm2Bp(UNS8 id);
static void CAN_Bp2Relay(UNS8 rl_stat);
static void CAN_Send_Debug(void);
static void J1939MsgAssb(CanRxMsg *nomMsg,J1939_MESSAGE * JMsg);
static void sendExtAddrClaim_FFA0(void);
static void sendExtBPInfo_FFA3(void);
static void sendExtBPInfo_FFA4(void);
static void sendRLCtrl(void);
static void insCellInfo(UNS8 scrAddr,UNS8 * da);
static void insModInfo_1(UNS8 scrAddr,UNS8 * da);
static void insModInfo_2(UNS8 scrAddr,UNS8 * da);
static void CAN_GT_ReadInfo(UNS8 id,GT_OP_CMD op);
static void CAN_GT_BalCtrl(UNS8 id);
static UNS8 isAddrSent=0;
extern 	UNS32  main_task_timer;
extern UNS32 mTsk_rt;

void Task_CAN2MsgBuf_GT(void *pdata){
	static UNS8 vmid=0,tmid=0,bmid=0,bal_mid=0;
	static GT_OP_CMD op_mode=GT_READ_VOL;
	UNS32 static loopCnt=0;
	while(1){
		OSTimeDly(10);//suspend at here for 10ms

		if(loopCnt++>0xFFFFFFFF){loopCnt=0;} 
		if((loopCnt%2)==0){	//10ms
			CAN_GT_ReadInfo(vmid, GT_READ_VOL);
			sendRLCtrl();
			if(vmid++>NUM_BMU_TOTAL){
				vmid=0;
			}
		}
		if((loopCnt%51)==0){
			CAN_GT_ReadInfo(tmid, GT_READ_TEMP);
			if(tmid++>NUM_BMU_TOTAL){
				tmid=0;
			}
		}
		if((loopCnt%52)==0){
			CAN_GT_ReadInfo(tmid, GT_READ_BAL_STAT);
			if(bmid++>NUM_BMU_TOTAL){
				bmid=0;
			}
		}
		if((loopCnt%54)==0){
			CAN_GT_BalCtrl(bal_mid);
			if(bal_mid++>NUM_BMU_TOTAL){
				bal_mid=0;
			}
		}

		if((loopCnt%21)==0){	//210ms
			CAN_Send_Debug();
		}
	}

}

/***********************************************************************
note: STM32F407 only has 3 send mail box
***********************************************************************/
void Task_CAN2_Rev(void *pdata)
{		 
    unsigned  char  os_err;
	static UNS8 sd_qid=0;
	UNS8 bt[8];
	UNS16 wd[4];
	CanRxMsg * RxMSG;
	CAN2_Configuration();
	sem_CAN2_rec_flag = OSSemCreate(1); //create a semphare
	while(1)
	{  	
		OSSemPend(sem_CAN2_rec_flag, 0,&os_err);//suspend at here wait for CAN2 message
		if(can2_rec_flag == 1)							//get a message
		{
			if(timeDiff(sysInfo.sysStartTime,OSTime)>40){
				can2Dispatch(&CAN2RxMsg);
			}
			can2_rec_flag=0;
		}
		
	}
}
void can2Dispatch(CanRxMsg * RxMSG){
	UNS8 bt[8];
	J1939_MESSAGE jmsg;
	static VREQ_ENUM lastMtnReg=0;
	if(RxMSG->IDE==CAN_Id_Extended){
		J1939MsgAssb(RxMSG,&jmsg);
		if((jmsg.pgn>=0x1100)&&(jmsg.pgn<=0x6400)){	//GT tempereture
			 GTInfo_parse(&jmsg);
			
		}else if(jmsg.pgn==0x11700){
			schnider_parse(&jmsg);
		}else if(jmsg.pgn==0xFF80){ //report request
			if((jmsg.data[0]==0xFF) ||(jmsg.data[0]==bpInfo.extAddr)){
				if(jmsg.data[1]==0x84){
				//	sendExtBPVersion();
				}
			}
		}else if(jmsg.pgn==0xFF81){ //ECU keep alive
			vcuInfo.keep_alive_req=jmsg.data[0] & 0x03;
			vcuInfo.vcuCmd=jmsg.data[1] & 0x0F;
			vcuInfo.vcuMtnReq=jmsg.data[2] & 0x0F;
			if(lastMtnReg==VREQ_NA){
				if(vcuInfo.vcuMtnReq==VREQ_CLEAR_ALARM){
					strAlarmClr();
				}else if(vcuInfo.vcuMtnReq==VREQ_CLEAR_FAULT){
					strFaultClr();
				}
			}
			lastMtnReg=vcuInfo.vcuMtnReq;
			
			OSSemPost(sem_vcu_can_rev);
		}else if(jmsg.pgn==0xFFF0){ //address claim
		
			NVM_sysCfgWrite();
		}else if(jmsg.pgn==0xFFF1){ //address contention
		}else if(jmsg.pgn==0xFFF2){ //address directed
		}else if(jmsg.pgn==0xFFF3){ //address request
			//sendExtAddrClaim_FFA0();
			//CAN2_WriteData(0x8FFF4,bt,8,CAN_Id_Extended);
		}else if(jmsg.pgn==0xFF88){ //cell voltage and temperture
			insCellInfo(jmsg.jid.j1939_id.SourceAddress,&jmsg.data[0]);
		}else if(jmsg.pgn==0xFF89){ //Battery internal address claim
			parseIntAddrClaim(RxMSG);
		}else if(jmsg.pgn==0xFF8A){ //BMU master command
		}else if(jmsg.pgn==0xFF8B){ //BMU flag
		}else if(jmsg.pgn==0xFF8C){ //BMU infor 1
			insModInfo_1(jmsg.jid.j1939_id.SourceAddress,&jmsg.data[0]);
		}else if(jmsg.pgn==0xFF8D){ //BMU info 2
			insModInfo_2(jmsg.jid.j1939_id.SourceAddress,&jmsg.data[0]);
		}else if(jmsg.pgn==0xFF8E){ //reserver
		}
		
	}
	if((RxMSG->IDE==CAN_Id_Extended)&&((RxMSG->ExtId & 0x1020000)==0x1020000)){
	}else if((RxMSG->IDE==CAN_Id_Extended)&&(RxMSG->ExtId ==0x1901FFE8)){
		bpInfo.masterCmd=RxMSG->Data[0];
	}else if((RxMSG->IDE==CAN_Id_Extended)&&(RxMSG->ExtId ==0x18B1A8)){
	}else if((RxMSG->IDE==CAN_Id_Extended)&&((RxMSG->ExtId & 0xFFF3)==0xFFF3)){
	}else if((RxMSG->IDE==CAN_Id_Standard)&&(RxMSG->StdId==0x131)){
		parseDebugCommand_0x131(RxMSG);
	}else if((RxMSG->IDE==CAN_Id_Standard)&&(RxMSG->StdId==0x132)){
		if((RxMSG->Data[2] & 0x01)>0) {PWR_12V_ON;} else {PWR_12V_OFF;}
		if((RxMSG->Data[2] & 0x02)>0) {BMU_WAKE_HIGH;} else {BMU_WAKE_LOW;}
		if((RxMSG->Data[2] & 0x04)>0) {HS1_ON;} else {HS1_OFF;}
		if((RxMSG->Data[2] & 0x08)>0) {HS2_ON;} else {HS2_OFF;}
		if((RxMSG->Data[2] & 0x10)>0) {HS3_ON;} else {HS3_OFF;}
		if((RxMSG->Data[2] & 0x20)>0) {HS4_ON;} else {HS4_OFF;}
		if((RxMSG->Data[2] & 0x40)>0) {HS5_ON;} else {HS5_OFF;}
		if((RxMSG->Data[2] & 0x80)>0) {ISODO_ON_1;} else {ISODO_OFF_1;}
		if((RxMSG->Data[3] & 0x01)>0) {ISODO_ON_2;} else {ISODO_OFF_2;}
		if((RxMSG->Data[3] & 0x02)>0) {ISODO_ON_3;} else {ISODO_OFF_3;}
		if((RxMSG->Data[3] & 0x4)>1) {
				LED1_ON;
		} else {
				LED1_OFF;
		}
		if((RxMSG->Data[3] & 0x8)>0) {
			SC_POWER_ON;
		} else {
			SC_POWER_OFF;
		}
		if((RxMSG->Data[3] & 0x20)>0) {LS1_ON;} else {LS1_OFF;}
		if((RxMSG->Data[3] & 0x40)>0) {LS2_ON;} else {LS2_OFF;}
		if((RxMSG->Data[3] & 0x80)>0) {LS3_ON;} else {LS3_OFF;}
	}
}

/***************************************************************
name: Task_CANSend
function: send CAN Message in the  buffer. excute every 1ms
*************************************************************/
void Task_CAN2MsgBuf(void *pdata){
	static UNS8 C2CVSendId=0;
	UNS32 static loopCnt=0;
	while(1){
		OSTimeDly(10);//suspend at here for 10ms

		if(loopCnt++>0xFFFFFFFF){loopCnt=0;} 
		if((loopCnt%20)==0){	//200ms
			//sdIntAddrClaim(CAN2);
			if(C2CVSendId++>13) C2CVSendId=0;
			CAN_Bm2Bp(C2CVSendId);
		}
		if((loopCnt%101)==0){
			sendExtAddrClaim_FFA0();
		}
		if((loopCnt%1001)==0){
			sendExtBPInfo_FFA3();
			sendExtBPInfo_FFA4();
		}
		if((loopCnt%21)==0){	//210ms
			CAN_Send_Debug();
		}
	}

}

void Task_CANSend(void *pdata){
		static UNS8 C1RdPoint=0;
	static UNS8 C2RdPoint=0;
	UNS8 transmit_mailbox;
    unsigned  char  os_err;
	sem_CAN2_send_flag = OSSemCreate(10); //create a semphere,
	while(1){
	//OSSemPend(sem_CAN2_send_flag, 0,&os_err);//suspend at here wait for CAN send command
	OSTimeDly(1);//suspend at here for 5ms
	if(C2_wait_num>0){
			transmit_mailbox = CAN_Transmit(CAN2,&CAN2TxBuf[C2RdPoint]);  /*  */	
			if(transmit_mailbox<CAN_TxStatus_NoMailBox){
				if(++C2RdPoint>=CAN_MSG_SEND_BUFFER_SIZE) C2RdPoint=0;
				C2_wait_num--;
			}
		}
		if(C2_wait_num>0){
			transmit_mailbox = CAN_Transmit(CAN2,&CAN2TxBuf[C2RdPoint]);  /*  */	
			if(transmit_mailbox<CAN_TxStatus_NoMailBox){
				if(++C2RdPoint>=CAN_MSG_SEND_BUFFER_SIZE) C2RdPoint=0;
				C2_wait_num--;
			}
		}
		if(C2_wait_num>0){
			transmit_mailbox = CAN_Transmit(CAN2,&CAN2TxBuf[C2RdPoint]);  /*  */	
			if(transmit_mailbox<CAN_TxStatus_NoMailBox){
				if(++C2RdPoint>=CAN_MSG_SEND_BUFFER_SIZE) C2RdPoint=0;
				C2_wait_num--;
			}
		}
			
		if(C1_wait_num>0){
			transmit_mailbox = CAN_Transmit(CAN1,&CAN1TxBuf[C1RdPoint]);  /*  */	
			if(transmit_mailbox<CAN_TxStatus_NoMailBox){
				if(++C1RdPoint>=CAN_MSG_SEND_BUFFER_SIZE) C1RdPoint=0;
				C1_wait_num--;
			}
		}
		if(C1_wait_num>0){
			transmit_mailbox = CAN_Transmit(CAN1,&CAN1TxBuf[C1RdPoint]);  /*  */	
			if(transmit_mailbox<CAN_TxStatus_NoMailBox){
				if(++C1RdPoint>=CAN_MSG_SEND_BUFFER_SIZE) C1RdPoint=0;
				C1_wait_num--;
			}
		}
		if(C1_wait_num>0){
			transmit_mailbox = CAN_Transmit(CAN1,&CAN1TxBuf[C1RdPoint]);  /*  */	
			if(transmit_mailbox<CAN_TxStatus_NoMailBox){
				if(++C1RdPoint>=CAN_MSG_SEND_BUFFER_SIZE) C1RdPoint=0;
				C1_wait_num--;
			}
		}
	}
}
void sdIntAddrClaim(CAN_TypeDef * CANPort){
	UNS8 bt[8];

	
//	CAN2_WriteData(0x18FF8900+strInfo.intAddr,bt,8,CAN_Id_Extended);
}
void parseIntAddrClaim(CanRxMsg * RxMSG){
	UNS16 ser_no;
	UNS8 i,j,k, bm_num,isNewMod=0,rand_num,addr;
	addr=(RxMSG->ExtId)&0xFF;

}
void parseDebugCommand_0x131(CanRxMsg * RxMSG){
	//nie if(RxMSG->Data[0]==strInfo.modId){
		switch(RxMSG->Data[1]){
			case 1:
				strInfo[0].modId=RxMSG->Data[2];
				break;
			case 2:
				strInfo[0].mod_soc=RxMSG->Data[2]*10;
				break;
			case 3:
				strInfo[0].mod_soh=RxMSG->Data[2]*10;
				break;
			case 4:
				strInfo[0].mod_sn=((UNS16)(RxMSG->Data[2])<<8)+RxMSG->Data[3];
				break;
			case 5:
				sysInfo.f_fault=0;
				OSSemPost(sem_fault_clr);
				
				break;
			case 100:
				sysInfo.boot_option=0;
				NVM_SysinfoWrite();
				if(sysInfo.boot_option==0){
					NVIC_SystemReset();
					}
				break;
			default:
				break;
		}
	//nie}		
	
}
/*below message will send to VCH */
static void CAN_Bm2Bp(UNS8 cid){
	UNS8 bt[8];
	INT32 whr;

}
/*below message will send to VCH */
static void CAN_Bp2Vcu(void){
	CanTxMsg txMsg;
	UNS8 bt[8];

	
}
static void CAN_Bp2Relay(UNS8 chg){
	static UNS8 lastCmd;
	static UNS8 pulse_stat=0;
	static UNS8 lp;
	UNS8 bt[8];

	switch(pulse_stat){
		case 0:
			if(lp++>10){
				pulse_stat=	1;
				lp=0;
			}
			break;
		case 1:
			if(((bpInfo.bmuWakeupCmd && (strInfo[0].f_bmu_lost==1))
				||((bpInfo.bmuWakeupCmd==0) && (strInfo[0].f_bmu_lost==0)))
				&&(gBOpMd!=B_RUN)&&(gBOpMd!=B_CHARGE))
			{
					pulse_stat=2;
					lp=0;
				
			}
			bt[0]=0;
			BMU_WAKE_LOW;
			break;
		case 2:
			if(lp++>30){
				pulse_stat=3;
				lp=0;
			}
			bt[0]=1;
			BMU_WAKE_HIGH;
			break;
		case 3:
			if(lp++>30){
				pulse_stat=0;
				lp=0;
			}
			bt[0]=0;
			BMU_WAKE_LOW;
			break;
		case 4:
			break;
	}

	CAN2_WriteData(0xAA5701,bt,8,CAN_Id_Extended);
}
	

static void CAN_Send_Debug(void){
	static UNS8 lastCmd;
	static UNS8 pulse_stat=0;
	static UNS8 lp;
	UNS8 wd[8];
	UNS16 w_d[8];
#if 1
	switch(pulse_stat){
		case 0:
			wd[0]=gBOpMd;
			wd[1]=sysInfo.f_fault;
			wd[1]|=sysInfo.f_vcu_lost<<1;
			//wd[1]|=strInfo.f_bmu_lost<<2;
			wd[1]|=bpInfo.w_idle<<3;
			wd[2]=sysInfo.wkSrc;
			wd[3]=bpInfo.bmuWakeupCmd<<1;
			wd[3]|=SC_POWER<<2;
			wd[3]|=BMU_WAKE_STAT<<3;
			pulse_stat++;
			CAN2_WriteData(0x410,wd,8,CAN_Id_Standard);
			break;
		case 1:
			w_d[0]=ADC_REV_Instant[0];w_d[1]=ADC_REV_Instant[1];w_d[2]=ADC_REV_Instant[2];w_d[3]=ADC_REV_Instant[3];
			CAN2_WriteData(0X411,(UNS8 *)w_d,8,CAN_Id_Standard); 	
			w_d[0]=ADC_REV_Instant[4];w_d[1]=ADC_REV_Instant[5];w_d[2]=ADC_REV_Instant[6];w_d[3]=ADC_REV_Instant[7];
			CAN2_WriteData(0X412,(UNS8 *)w_d,8,CAN_Id_Standard); 	
			w_d[0]=ADC_REV_Instant[8];w_d[1]=ADC_REV_Instant[9];w_d[2]=ADC_REV_Instant[10];w_d[3]=ADC_REV_Instant[11];
			CAN2_WriteData(0X413,(UNS8 *)w_d,8,CAN_Id_Standard); 	
			w_d[0]=ad_res.V_DC_Batt;w_d[1]=ad_res.V_DC_Bus;w_d[2]=ad_res.V_12VOut;w_d[3]=ad_res.V_5VOut;
			CAN2_WriteData(0X414,(UNS8 *)w_d,8,CAN_Id_Standard); 	
			w_d[0]=ad_res.V_HV_in;w_d[1]=ad_res.T_1;w_d[2]=ad_res.T_2;w_d[3]=ad_res.T_3;
			CAN2_WriteData(0X415,(UNS8 *)w_d,8,CAN_Id_Standard); 	
			wd[0]=(sysInfo.sysResetCnt>>8)&0xff;
			wd[1]=(sysInfo.sysResetCnt>>0)&0xff;
			CAN2_WriteData(0X416,(UNS8 *)w_d,8,CAN_Id_Standard); 
			wd[0]=(mTsk_rt>>24)&0xff;
			wd[1]=(mTsk_rt>>16)&0xff;
			wd[2]=(mTsk_rt>>8)&0xff;
			wd[3]=(mTsk_rt>>0)&0xff;
			wd[4]=gBOpMd;
			wd[5]=(sysInfo.sysResetCnt>>8)&0xff;
			wd[6]=(sysInfo.sysResetCnt>>0)&0xff;
			//wd[5]=(main_task_timer>>16)&0xff;
			//wd[6]=(main_task_timer>>8)&0xff;
			wd[7]=(main_task_timer>>0)&0xff;
			CAN2_WriteData(0X417,(UNS8 *)wd,8,CAN_Id_Standard); 	
			pulse_stat++;
			break;
		default:
			pulse_stat=0;
			break;
	}
	#endif
}
static void J1939MsgAssb(CanRxMsg *nomMsg,J1939_MESSAGE *JMsg){
	unsigned char i;
	JMsg->jid.eid=nomMsg->ExtId;

	JMsg->pgn=(unsigned int)(JMsg->jid.j1939_id.DataPage)<<16;
	JMsg->pgn+=(unsigned int)(JMsg->jid.j1939_id.PDUFormat)<<8;
	if (JMsg->jid.j1939_id.PDUFormat>240){
		JMsg->pgn+=(unsigned int)JMsg->jid.j1939_id.PDUSpecific;
	}
	JMsg->dlc=nomMsg->DLC;
	for(i=0;i<8;i++){
		JMsg->data[i]=nomMsg->Data[i];
	}
}
 void J1939IdAssb_pgn(unsigned int pgn,J1939_MESSAGE *JMsg){
	unsigned char i;
	unsigned char pd,ps;
	JMsg->jid.j1939_id.DataPage=(pgn>>16) && 0x1;
	pd=(pgn>>8)&& 0xFF;
	ps=pgn&&0xFF;
	if(pd>=0xF0){	//broad cast
		JMsg->jid.eid&=0x1C0000FF;
		JMsg->jid.j1939_id.PDUFormat=pd;
		JMsg->jid.j1939_id.PDUSpecific=ps;
	}else{   //to specisic node
		JMsg->jid.j1939_id.PDUFormat=pd;
		JMsg->jid.eid &= 0x1C00FFFF;
	}
	JMsg->jid.eid|=pgn<<8;

}

void j1939_send(CAN_TypeDef *CANx,J1939_MESSAGE *jmsg){
	J1939_MESSAGE msg;
	msg=*jmsg;
	CAN2_WriteData(msg.jid.eid,msg.data,msg.dlc,CAN_Id_Extended);
}
static void sendExtAddrClaim_FFA0(void){
	UNS8 bt[8];
	bpInfo.hb ^=bpInfo.hb;
	bt[0]=bpInfo.hb;
	bt[1]=gBOpMd;
	//bt[2]|=(UINT8)HEATER_2_STAT<<2;
	//bt[2]|=(UINT8)HEATER_2_STAT<<3;
	CAN2_WriteData(0x08FFA0F0,bt,8,CAN_Id_Extended);
}
static void sendRLCtrl(void){
	UNS8 bt[8];
	#if 0
		bt[0]=sysInfo.mainRelayPosInfo.actionCmd;
		bt[1]=sysInfo.mainRelayNegInfo.actionCmd;
		bt[2]=sysInfo.pchgRlInfo.actionCmd;
		bt[3]=sysInfo.fanPwrRelayInfo.actionCmd;
		bt[4]=sysInfo.strRL[0].actionCmd+(sysInfo.strRL[1].actionCmd<<4);
		bt[5]=sysInfo.strRL[2].actionCmd+(sysInfo.strRL[3].actionCmd<<4);
		bt[6]=sysInfo.strRL[4].actionCmd+(sysInfo.strRL[5].actionCmd<<4);
		bt[7]=sysInfo.strRL[6].actionCmd+(sysInfo.strRL[7].actionCmd<<4);
	#else
		bt[0]=sysInfo.strRL[0].actionCmd;
		bt[1]=sysInfo.strRL[1].actionCmd;
		bt[2]=sysInfo.strRL[2].actionCmd;
		bt[3]=sysInfo.strRL[3].actionCmd;
		bt[4]=sysInfo.strRL[4].actionCmd;
		bt[5]=sysInfo.strRL[5].actionCmd;
		bt[6]=sysInfo.strRL[6].actionCmd;
		bt[7]=sysInfo.strRL[7].actionCmd;
	#endif
	CAN2_WriteData(0xAA5701,bt,8,CAN_Id_Extended);
}
static void sendExtBPInfo_FFA3(void){
	UNS8 bt[8];
	
	bt[0]=(ad_res.T_2) & 0xFF;
	bt[1]=(ad_res.T_3) & 0xFF;
	CAN2_WriteData(0x08FFA3F0,bt,8,CAN_Id_Extended);
}
static void sendExtBPInfo_FFA4(void){
	UNS8 bt[8];
	bt[0]=bpInfo.bp_sn & 0xFF;
	bt[1]=(bpInfo.bp_sn>>8) & 0xFF;
	bt[2]=(UNS8)bpInfo.sc_hw_version & 0xFF;

	bt[3]=bpInfo.sc_fw_ver_major;
	bt[4]=bpInfo.sc_fw_ver_minor;
	bt[5]=bpInfo.sc_fw_ver_patch;
	CAN2_WriteData(0x08FFA4F0,bt,8,CAN_Id_Extended);
}
static void sendExt_FFF0(void){
	UNS8 bt[8];
	CAN2_WriteData(0x18FFF000+bpInfo.extAddr,bt,8,CAN_Id_Extended);
}
static void sendExt_FFF1(void){
	UNS8 bt[8];
	CAN2_WriteData(0x18FFF100+bpInfo.extAddr,bt,8,CAN_Id_Extended);
}
static void sendExt_FFF2(void){
	UNS8 bt[8];
	CAN2_WriteData(0x18FFF200+bpInfo.extAddr,bt,8,CAN_Id_Extended);
}
static void sendExt_FFF3(void){
	UNS8 bt[8];
	CAN2_WriteData(0x18FFF300+bpInfo.extAddr,bt,8,CAN_Id_Extended);
}
static void CAN_GT_BalCtrl(UNS8 id){
	UNS8 bt[8];
	bt[0]=0x64;
	
	bt[2]=(bmu[id].cbCtrl.wd>>8) & 0xFF;
	bt[3]=bmu[id].cbCtrl.wd & 0xFF;
	CAN2_WriteData(0x184280F5+((UNS16)id<<8),bt,8,CAN_Id_Extended);
}
static void CAN_GT_ReadInfo(UNS8 id,GT_OP_CMD op){
	UNS8 bt[8];
	if(op==GT_READ_VOL){
		bt[0]=1;
	}else if(op==GT_READ_TEMP){
		bt[0]=2;
	}else if(op==GT_READ_BAL_STAT){
		bt[0]=4;
	}
	bt[1]=0xFF;bt[2]=0xFF;bt[3]=0xFF;bt[4]=0xFF;bt[5]=0xFF;bt[6]=0xFF;bt[7]=0xFF;
	CAN2_WriteData(0x181080F5+((UNS16)id<<8),bt,8,CAN_Id_Extended);
}

static void insCellInfo(UNS8 scrAddr,UNS8 * da){

	
}
static void insModInfo_1(UNS8 scrAddr,UNS8 * da){
	UNS8 iid,cid;
	int32_t I;
	for(iid=0;iid<bpInfo.num_str;iid++){
		if(bpInfo.str[iid].intAddr==scrAddr){
			bpInfo.str[iid].mod_soh=((UNS16)(*(da+1))<<8)+(*(da+0));
			bpInfo.str[iid].mod_soc=((UNS16)(*(da+3))<<8)+(*(da+2));
			I=((int32_t)(*(da+5))<<8)+(*(da+4));
			I=I*50-1600000;
			bpInfo.str[iid].mod_curr=(int16_t)I;
			bpInfo.str[iid].mod_volt=(((UNS16)(*(da+7))<<8)+(*(da+6)))*10;
			break;
		}
	}
}
static void insModInfo_2(UNS8 scrAddr,UNS8 * da){
	UNS8 iid,cid;
	int32_t I;
/*	for(iid=0;iid<bpInfo.num_str;iid++){
		if(bpInfo.mod[iid].intAddr==scrAddr){
			I=((int32_t)(*(da+5))<<8)+(*(da+4));
			I=I*50-1600000;
			bpInfo.mod[iid].mod_cld=(uint16_t)I;
			I=((int32_t)(*(da+1))<<7)+(*(da+6));
			I=I*50-1600000;
			bpInfo.mod[iid].mod_clc=(uint16_t)I;
			break;
		}
	}
	*/
}
