/***********************************************************************
***********************************************************************/
#include "main.h"  
#include "Task_sysCheck.h"
#include "Task_main.h"

OS_EVENT  *fault_flag = 0;			//any fault detected
OS_STK Stk_TaskFaultCheck[TASK_FAULT_CHECK_STK_SIZE];
OS_EVENT *sem_vcu_can_rev;
OS_EVENT *sem_bmu_can_rev;
OS_EVENT *sem_fault_clr;
TCODE_T str_tc[NUM_STR][TC_NUM];
TCODE_T bp_tc[TC_NUM];
static UNS32 getSysTCVal(FAW_ID_T tcid);

void Task_faultCheck(void);
static void sysTCCheck(void);

/***********************************************************************
***********************************************************************/
void Task_FaultCheck(void *pdata)
{
	int16_t vcu_rev_flag,bmu_rev_flag;
	static uint16_t vcu_lose_cnt=0,bmu_lose_cnt=0;
	static uint32_t idleStartTime;
	uint8_t tc_chk_id;
	sem_vcu_can_rev=OSSemCreate(0);
	sem_bmu_can_rev=OSSemCreate(0);

	while(1)
	{  
		OSTimeDlyHMSM(0, 0, 0, 10);//suspend at here for 10ms
	Task_ADC();
		/*check battery string fault */
		if(timeDiff(OSTime,sysInfo.sysStartTime)>10000){ //
			//if((tc_chk_id++)>=gSysCfg.ucNUM_STR){tc_chk_id=0;}
			for(tc_chk_id=0;tc_chk_id<NUM_STR;tc_chk_id++){
				strTCCheck(tc_chk_id);
			}
		}
		
		sysTCCheck();
		/* check system fault */
		// VCU comm fault
		vcu_rev_flag=OSSemAccept(sem_vcu_can_rev);
		if(vcu_rev_flag){
			vcu_lose_cnt=0;
			sysInfo.f_vcu_lost=0;
		}else{
			if(vcu_lose_cnt++>600){
			//v1.02	if(vcu_lose_cnt++>60){
				sysInfo.f_vcu_lost=1;
			}
		}
		//BMU comm fault
		bmu_rev_flag=OSSemAccept(sem_bmu_can_rev);
		if(bmu_rev_flag){
			bmu_lose_cnt=0;
			strInfo[0].f_bmu_lost=0;
		}else{
			if(bmu_lose_cnt++>600){
				strInfo[0].f_bmu_lost=1;
			}
		}		
		//VCU comm fault
		if(0
			||sysInfo.f_vcu_lost
			||strInfo[0].f_bmu_lost
		){
			sysInfo.f_fault=1;
			//vcuInfo.keep_alive_req=VCMD_OPEN;
		}else{
			sysInfo.f_fault=0;
		}
		//IDLE when chargef
		if (bpInfo.bp_curr>300){
			bpInfo.w_idle=0;
			idleStartTime=OSTime;
		}else{
			if(timeDiff(idleStartTime,OSTime)>60000){
				bpInfo.w_idle=1;
			}
		}
			
	}
}

void sysTCCheck(){
	FAW_ID_T tcid;
	TCODE_T *ptc;
	TCODE_T *pStart;
	TCODE_T tc;
	UINT32 rtVal;
	static uint16_t tm_rem_s[SYS_TC_NUM]={0,0,0,0,0};
	static uint16_t tm_rem_r[SYS_TC_NUM]={0,0,0,0,0};
	pStart=(TCODE_T *)&bp_tc[0];
	for(tcid=F_PK_DUT;tcid<(F_PK_DUT+SYS_TC_NUM);tcid++){
		rtVal=getSysTCVal(tcid);
		ptc=pStart+tcid;
		if (ptc->tc_stat==0){  //no fault
			if(ptc->dir>0){	/* upside threshold */
				if(rtVal > ptc->thd_s){
					tm_rem_s[tcid]++;
					if (tm_rem_s[tcid] > ptc->tm_s){
						ptc->tc_stat=1;
						tm_rem_r[tcid]=0;
					}
				}else{
					tm_rem_s[tcid]=0;
				}
			}else{
				if(rtVal < ptc->thd_s){
					tm_rem_s[tcid] ++;
					if (tm_rem_s[tcid] > ptc->tm_s){
						ptc->tc_stat=1;
						tm_rem_r[tcid]=0;
					}
				}else{
					tm_rem_s[tcid]=0;
				}
			}
		}else{	//fault exist
			if(ptc->tm_r !=0xFFFF){	//fault recoverable
			if(ptc->dir>0){	/* upside threshold reset when val<threshold */
				if(rtVal < ptc->thd_r){
					tm_rem_r[tcid] ++;
					if (tm_rem_r[tcid] > ptc->tm_r){
						ptc->tc_stat=0;
						tm_rem_s[tcid]=0;
					}
				}else{
					tm_rem_r[tcid]=0;
				}
			}else{
				if(rtVal > ptc->thd_r){	//lowside. reset when val>threshold
					tm_rem_r[tcid] ++;
					if (tm_rem_r[tcid] > ptc->tm_r){
						ptc->tc_stat=0;
						tm_rem_s[tcid]=0;
					}
				}else{
					tm_rem_r[tcid]=0;
				}
			}
			}
	}
	}
}
UNS8 getSysTCStat(FAW_ID_T tcid){
	return bp_tc[tcid].tc_stat;
}
		
static UNS32 getSysTCVal(FAW_ID_T tcid){
		UNS32 val;
		switch(tcid){
			case F_PK_DUT:
			case F_PK_CUT:
					if(bpInfo.ct_min>=0) val=0xFFFFFFFF;
					else val=(UINT32)bpInfo.ct_min;
				break;
		case F_PK_DOC:
			if(bpInfo.bp_curr>=0) val=0; //charging
			else val=(UINT32)(-bpInfo.bp_curr);
			break;
		case F_PK_COC:
			if(bpInfo.bp_curr>=0) val=bpInfo.bp_curr; //charging
			else val=(UINT32)(0);
			break;
		default:
			break;
	}
	return val;
}

