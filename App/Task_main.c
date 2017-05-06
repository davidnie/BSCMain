/***********************************************************************
***********************************************************************/
#define _TASK_MAIN_DEFINE
#include "main.h"
#include "def.h"
#include "Task_main.h"
#include "Task_sysCheck.h"
#include "Task_ADC.h"
#include "wifi.h"

B_TKS_OP_MODE gBOpMd;
BATT_PACK_INFO bpInfo;
BATT_STR_INFO strInfo[NUM_STR];
UNS32  main_task_timer;
UNS32 mTsk_rt;

WK_SOURCE WakeSrcCheck(void);
void wakeupBmu(void);
void setWhoAmI(void);
static void bmu_pwer_ctrl(UNS8 chg);
void Task_Main_Toro(void *pdata);


/***********************************************************************
***********************************************************************/
	void Task_Main(void *pdata)
	{		 
		unsigned  char	os_err,i,isCmdIn;
		Message RxMSG;
		static UNS32 tskmain_lpcnt=0;
		static UNS8 tc_chk_id=0;
		UNS32 nowTime;
		unsigned char sdData[10]={1,2,3,4,6,7,8,9};
		SC_POWER_ON;
		PWR_12V_ON;
		TCTableInit();

		while(1){
			if((tskmain_lpcnt++)>=0xFFFFFFFE) tskmain_lpcnt=0;
			OSTimeDlyHMSM(0, 0, 0, 10);//suspend at here for 10ms
					updateBMSoc(&strInfo,100);

			wdg_feed();		//feed dog
			fork_task();
		
			mTsk_rt=OSTimeGet();	//get real time counter
		
#if 0
		if(isHWShutdownExist()){
			gBOpMd=BP_FINAL;
		}else if((isCriticalFaultExist()||isAlarmExist())&&(gBOpMd!=BP_ERROR)&&(gBOpMd!=BP_FINAL)){
			main_task_timer=mTsk_rt;
			gBOpMd=BP_ERROR;
		}
		switch(gBOpMd){
			case BP_INIT:   //wakeup bmu and make sure 2 BMU alive
				//initilize battery BSCInit();
				bpInfo.bmuWakeupCmd=0;
				strInfo.bmuOnCmd=CMD_BMU_FET_OFF;
				//bmu_pwer_ctrl(1);	//enable 12V output for wake up BMU
				if(timeDiff(main_task_timer,mTsk_rt)>100000){ //
					gBOpMd=BP_FAULT;
				}else if(timeDiff(main_task_timer,mTsk_rt)>15000){
					main_task_timer=mTsk_rt;
					gBOpMd=BP_SYNC_OFF;
				}else if(timeDiff(main_task_timer,mTsk_rt)>5000){
					// nie if(strInfo.bmu_alive_num==2){
					if(strInfo.bmu_alive_num>0){
						main_task_timer=mTsk_rt;
						gBOpMd=BP_DO_NOT_ATTACH;
					}
					BMU_WAKE_LOW;
				}else if(timeDiff(main_task_timer,mTsk_rt)>4000){
					if(strInfo.bmu_alive_num==0){
						BMU_WAKE_HIGH;
					}
				}else if(timeDiff(main_task_timer,mTsk_rt)>3000){
					BMU_WAKE_LOW;
				}
				break;
			case BP_SYNC_OFF:
				strInfo.bmuOnCmd=CMD_BMU_SHUTDOWN;
				if(timeDiff(main_task_timer,mTsk_rt)>2000){
					if(strInfo.bmu_alive_num==0){
						gBOpMd=BP_INIT;			
						main_task_timer=mTsk_rt;
						
						BMU_WAKE_LOW;
					}
				}
				break;
			case BP_DO_NOT_ATTACH:  //no BMU has fault and FET is open
				bpInfo.bmuWakeupCmd=1;
				strInfo.bmuOnCmd=CMD_BMU_FET_OFF;
				BMU_WAKE_LOW;
				//nie if((strInfo.bmu_ready_num>0)&&(strInfo.bmu_alive_num==2)){
				if((strInfo.bmu_ready_num>0)&&(strInfo.bmu_alive_num>0)){
					gBOpMd=BP_READY_TO_ATTACH;
				}else if((timeDiff(main_task_timer,mTsk_rt)>10000)&&(strInfo.bmu_alive_num<2)){
				
					strInfo.bmuOnCmd=CMD_BMU_SHUTDOWN;		
					BMU_WAKE_LOW;
					gBOpMd=BP_INIT;
					main_task_timer=mTsk_rt;
				}
				break;
			case BP_READY_TO_ATTACH: 	//at least one BMU ready to work. wait for controller command
				LS3_OFF;
				if(vcuInfo.vcuCmd==VCMD_CLOSE){
					bpInfo.masterCmd=MCMD_CLOSE;
					gBOpMd=BP_NOMOR_OPERATION;
					if(strInfo.bmu_on_num>0){
						gBOpMd=BP_NOMOR_OPERATION;
					}
				}
				break;
			case BP_NOMOR_OPERATION:
				bpInfo.masterCmd=MCMD_CLOSE;
				strInfo.bmuOnCmd=CMD_BMU_FET_ON;
				if(vcuInfo.vcuCmd==VCMD_OPEN){
					gBOpMd=BP_DO_NOT_ATTACH;
				}else if(isAlarmExist()){
					gBOpMd=BP_REQUEST_DETACH;
				}
				break;
			case BP_REQUEST_DETACH:
				if(vcuInfo.vcuCmd==VCMD_OPEN){
					gBOpMd=BP_DO_NOT_ATTACH;
				}else if(isAlarmExist()==0){
					gBOpMd=BP_NOMOR_OPERATION;
				}
				break;
			case BP_ERROR:
				strInfo.bmuOnCmd=VCMD_OPEN;
				if((isCriticalFaultExist()==0)&&(isAlarmExist()==0)){
					gBOpMd=BP_DO_NOT_ATTACH;
				}
				if(timeDiff(main_task_timer,mTsk_rt)>600000){
					gBOpMd=BP_FINAL;
				}
				break;
			case BP_FINAL:  //turn off MBU power, and SC power
				strInfo.bmuOnCmd=CMD_BMU_SHUTDOWN;
				if(strInfo.bmu_alive_num==0){
					SC_POWER_OFF;
					
				}
				
				break;
			case BP_WARMUP:		//in low temperature. turn on heater by extenal power
				break;
				
			default:
				break;
		}
		#endif
	}	
		
}
WK_SOURCE WakeSrcCheck(void){
	int a;
	WK_SOURCE src;
	if((!DI_WAKE_BY_CHG)&&(sysInfo.f_vcu_lost)){
		src=WK_CHARGE;
	}else{
		src=WK_MACHINE;
	}
	
	
	return src;
}
void wakeupBmu(void){
	if (BMU_WAKE_STAT==1){
		PWR_12V_ON;
		BMU_WAKE_HIGH;
	}
}
void setWhoAmI(void){
	UNS16 mySn,sn;
	BM_POSISTION whoAmI=MASTER;
	UNS8 mid,rank=0;
	bpInfo.str[0].Inter_Role=whoAmI;	
	
}
void masterRun(void){
}
static void bmu_pwer_ctrl(UNS8 chg){
	static UNS8 lastCmd;
	static UNS8 pulse_stat=0;
	static UNS16 lp;
	static UNS32 tskTm=0;
	UNS8 bt[8];

}
	
