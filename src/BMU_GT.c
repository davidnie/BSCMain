/***********************************************************************
***********************************************************************/
#include "main.h"
#include "bsp_can.h"
#include "Task_ADC.h"
#include "Task_Main.h"
#include "Task_sysCheck.h"
#include "shed_ctrl.h"
#include "battery_gt.h"
#include "HAL.H"
FK_STAGE gFkStat;
void sdIntAddrClaim(CAN_TypeDef * CANPort);
void GTInfo_parse(J1939_MESSAGE* jmsg);
static INT8 selChgStr(void);
static INT8 selDChgStr(void);
void fork_task(void);
static void str_task(void);
static void sstr_task(UINT8 sid);
static void rlAllOpen(void);
void fork_task(void){
	INT8 sid;
	switch(gFkStat){
		case FK_INIT:
			if(timeDiff(sysInfo.sysStartTime,OSTime)>500){
				gFkStat=FK_READY;
			}
			break;
		case FK_READY:
			rlAllOpen();
			if(isPlugin()){
				gFkStat=FK_CHG_PRE;
				relayControl(RL_DCDC,CONT_ON);	//precharge 
			}else{
				gFkStat=FK_DCHG_PRE;
				relayControl(RL_DCDC,CONT_ON);
			}
			break;
		case FK_CHG_PRE:
			sid=selChgStr();
			sid=0; //nie
			if(sid>=0){
				sstrMainRLCtrl(sid,CONT_ON);
				if((abso(bpInfo.VBus-VLT_STR_MIN)>2000) && (bpInfo.VBus>28000)){
					gFkStat=FK_CHG;
				}
			}
			break;
		case FK_DCHG_PRE:
			sid=selDChgStr();
		sid=0;	//nie
			relayControl(RL_PCHG,CONT_ON);	//precharge 
			if(sid>=0){
				sstrMainRLCtrl(sid,CONT_ON);
				if((abso(bpInfo.VBus-VLT_STR_MIN)>2000) &&(bpInfo.VBus>28000)){
					gFkStat=FK_RUN;
				}
			}
			break;
		case FK_CHG:
			relayControl(RL_CHG_POS,CONT_ON);
			str_task();
			if(!isPlugin()){
				gFkStat=FK_READY;
			}
			
			break;
		case FK_RUN:
			relayControl(RL_MAIN_POS,CONT_ON);
			str_task();
			if(isPlugin()){
				gFkStat=FK_READY;
			}else if(isSysFaultExist()){
				gFkStat=FK_FAULT;
			}else{
				
			}
			break;
		case FK_FAULT:
			if(!isSysFaultExist()){
				gFkStat=FK_INIT;
			}
			break;
		default:
			break;
	}
}
void GTInfo_parse(J1939_MESSAGE* jmsg){
	uint8_t mid,cid,pid;
	uint8_t srcAdd;
	uint16_t respCode;
	srcAdd=jmsg->jid.j1939_id.SourceAddress;
	respCode=jmsg->pgn;
	switch(respCode){
	case 0x1100:
		if((srcAdd>=0x80) &&(srcAdd<(0x80+NUM_BMU_TOTAL))){
			mid=jmsg->jid.j1939_id.SourceAddress-0x80;
			cid=jmsg->data[0];
			if((cid>=1) && (cid<12)){
				bmu[mid].cv[cid-1]=((uint16_t)(jmsg->data[2])<<8)+jmsg->data[3];
				bmu[mid].cv[cid]=((uint16_t)(jmsg->data[4])<<8)+jmsg->data[5];
				bmu[mid].cv[cid+1]=((uint16_t)(jmsg->data[6])<<8)+jmsg->data[7];
					if((bmu[mid].cv[cid-1]>6000)
						||(bmu[mid].cv[cid]>6000)
				||(bmu[mid].cv[cid+1]>6000))
						{
						pid=12;
					}
			}
		}
		break;
	case 0x1200:
		if((srcAdd>=0x80) &&(srcAdd<(0x80+NUM_BMU_TOTAL))){
			mid=jmsg->jid.j1939_id.SourceAddress-0x80;
			cid=jmsg->data[0];
			if((cid>=1) && (cid<7)){
				bmu[mid].ct[cid-1]=(int16_t)(jmsg->data[2]-40)*10;
				bmu[mid].ct[cid]=(int16_t)(jmsg->data[3]-40)*10;
				bmu[mid].ct[cid+1]=(int16_t)(jmsg->data[4]-40)*10;
				bmu[mid].ct[cid+2]=(int16_t)(jmsg->data[5]-40)*10;
				bmu[mid].ct[cid+3]=(int16_t)(jmsg->data[6]-40)*10;
				bmu[mid].ct[cid+4]=(int16_t)(jmsg->data[7]-40)*10;
			}
		}
		break;
	case 0x1500:
			if((srcAdd>=0x80) &&(srcAdd<(0x80+NUM_BMU_TOTAL))){
				mid=jmsg->jid.j1939_id.SourceAddress-0x80;
				cid=jmsg->data[0];
					bmu[mid].cbStat.wd=((int16_t)(jmsg->data[2])<<8)+jmsg->data[3];
			}
			break;
		

	default:
			break;
		}
}

static void str_task(void){
	sstr_task(0);
	sstr_task(1);
	sstr_task(2);
	sstr_task(3);
	sstr_task(4);
	sstr_task(5);
	sstr_task(6);
	sstr_task(7);
}
void sstr_task(uint8_t sid){

	STR_STAGE_T sstat;
	sstat=strInfo[sid].strStat;
	switch(sstat){
		case STR_INIT:
			sstat=STR_READY;
			break;
		case STR_READY:
			if(1
				&&(!isStrFaultExist(sid))
			//	&&(abso(strInfo[sid].sv_sum-bpInfo.VBat)<500)
			){
				sstrMainRLCtrl(sid,CONT_ON);
				sstat=STR_RUN;
			}
			break;
		case STR_RUN:
			sstrMainRLCtrl(sid,CONT_ON);
			if(0
				||(isStrFaultExist(sid))
			){
				sstat=STR_FAULT;
			}
			break;
		case STR_FAULT:
			sstrMainRLCtrl(sid,CONT_OFF);
			if(1
				&&(!isStrFaultExist(sid))
			){
				sstat=STR_INIT;
			}
			break;
		default:
			sstat=STR_INIT;
			break;
	}
	strInfo[sid].strStat=sstat;
			
}

INT8 selChgStr(void){
	UINT8 cid,sid;
	UINT16 gapToAvg=0xFFFF;
	UINT16 gapToAvgTemp;
	UINT8 chgCid=0x11,chgSid=0x5;
	UINT8 ret=0;
	UINT16 cv_min=0xFFFF;
	sid=bpInfo.sid_vmin;
	if(1
		&&(sid<gSysCfg.ucNUM_STR)
		&&(strInfo[sid].sv_sum>((UINT32)2600*gSysCfg.ucNUM_CV_IN_BMU*gSysCfg.ucNUM_BUM_PER_STR))
		&& (isStrFaultExist(sid)==0)
		){
			ret=sid;
	}else{
		ret=-1;
	}
	return ret;
}



INT8 selDChgStr(void){
	UINT8 cid,sid;
	UINT16 gapToAvg=0xFFFF;
	UINT16 gapToAvgTemp;
	UINT8 forceClsCid=0x11,forceClsSid=0x5;
	UINT8 ret=0,temp;
	sid=bpInfo.sid_vmax;

	if(1
		&&(sid<gSysCfg.ucNUM_STR)
		&&(strInfo[sid].sv_sum>((UINT32)3100*gSysCfg.ucNUM_CV_IN_BMU*gSysCfg.ucNUM_BUM_PER_STR))
		){
			ret=sid;
	}else{
		ret=-1;
	}
	return ret;
}
static void rlAllOpen(void){
	uint8_t sid;
	for(sid=0;sid<NUM_STR;sid++){
		sstrMainRLCtrl(sid,CONT_OFF);
		strInfo[sid].strStat=0;
	}
	relayControl(RL_MAIN_POS,CONT_OFF);
	relayControl(RL_CHG_POS,CONT_OFF);
	relayControl(RL_HEATER_1,CONT_OFF);
	relayControl(RL_HEATER_2,CONT_OFF);
}

