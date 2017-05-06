#include "bmu.h"
#include "main.h"
#include "can.h"
#include "appBMU.h"
#include "Task_main.h"
#include "Task_sysCheck.h"
#include "sysTick.h"
UNS32 abs(UNS32 a, UNS32 b);
UNS32 abso(INT32 a);

BOOLEAN isCriticalFaultExist();
BOOLEAN isAlarmExist();
BOOLEAN isHWShutdownExist();
BOOLEAN strAlarmClr();
BOOLEAN strFaultClr();
void strTCCheck(UNS8 strid);
void TCTableInit(void);
static UNS32 getStrTCVal(UNS8 sid,FAW_ID_T tcid);
UNS8 getStrTCStat(UNS8 sid,FAW_ID_T tcid);
parseBroadMsg(Message *m, UNS8 nodeId);
UINT16 SOC_OCV_Cal(BATT_STR_INFO *bm,UINT16 savedSOC);
UINT16 SOC_OCV_Lookup(INT8 temp,UINT16 Vcell);
void strPlimSet(BATT_STR_INFO *bm);
static void setBalance(UNS8 sid);
void bmuInit(void);
parseBroadMsg(Message *m, UNS8 nodeId){
	UNS16 idx;
	UNS8 subIdx,dcnt;
	UNS32 da;
	UNS8 ds[8];
	static int32_t last_curr,now_curr;
	void *d;
	UNS8 nid;
	float flVolt;
	OSSemPost(sem_bmu_can_rev);
	#if 0
	if (nodeId>0){ nid=1;} else{ nid=nodeId-1;}
	if (nodeId>strInfo.bmu_total) strInfo.bmu_total=nodeId;
	dcnt=4-getSDOn2(m->data[0]);
	memcpy((void *)&da,m->data+4,dcnt);
	idx=getSDOindex(m->data[1],m->data[2]);
	subIdx=m->data[3];
	Task_ADC();
	switch(idx){
		case 0x6D01:	
			bmu[nodeId-1].SOC=da;
			break;
		case 0x6D02:	
			memcpy((void *)&bmu[nodeId-1].SOH,m->data+4,2);
			bmu[nodeId-1].SOH*=10;
			break;
		case 0x6D03:	
		 bmu[nodeId-1].alive=1;
			memcpy((void *)&bmu[nodeId-1].I_bmu,m->data+4,dcnt);
		/*	now_curr=bmu[nodeId-1].I_bmu;
			if((((last_curr-bmu[nodeId-1].I_bmu)>5000)&&(last_curr>bmu[nodeId-1].I_bmu))
				||(((bmu[nodeId-1].I_bmu>last_curr)<5000)&&(last_curr<bmu[nodeId-1].I_bmu))
				){
				bmu[nodeId-1].I_bmu=last_curr;
			}
			last_curr=now_curr;
			*/
			break;

		case 0x6D04:	
			memcpy((void *)&bmu[nodeId-1].V_bmu,m->data+4,dcnt);
			break;
		case 0x6D05:	
			memcpy((void *)&bmu[nodeId-1].ct_max,m->data+4,dcnt);
			break;
		case 0x6D06:	
			memcpy((void *)&bmu[nodeId-1].ct_min,m->data+4,dcnt);
			break;
		case 0x6D07:	
			memcpy((void *)&bmu[nodeId-1].bmuSafeFlag_1,m->data+4,dcnt);
			break;
		case 0x6D08:	
			memcpy((void *)&bmu[nodeId-1].bmuSafeFlag_2,m->data+4,dcnt);
			break;
		case 0x6D09:	
			memcpy((void *)&bmu[nodeId-1].bmuStatus,m->data+4,dcnt);
			break;
		case 0x6D0A:	
			memcpy((void *)&bmu[nodeId-1].fetFlag,m->data+4,dcnt);
			break;
		case 0x6D0B:	
			memcpy((void *)&bmu[nodeId-1].parllFlag,m->data+4,dcnt);
			break;
		case 0x6024:
			memcpy((void *)&bmu[nodeId-1].cc_max,m->data+4,dcnt);
			break;
		case 0x6025:
			memcpy((void *)&bmu[nodeId-1].dc_max,m->data+4,dcnt);
			break;
		case 0x6E80:
			memcpy((void *)&bmu[nodeId-1].dc_max,m->data+4,dcnt);
				break;
		default:
			break;

	}
	#endif
}
void updateBP_BM(void){
	UNS8 mid,mOnNum=0,mOffNum=0,mAliveNum=0,mReadyNum=0,sid,cid,smid;
	UNS8 sOnNum=0,sOffNum=0,sAliveNum=0,sReadyNum=0;
	UNS16 soc=0,soh=0,cc_max=0,dc_max=0,cvmax_str=0,cvmin_str=0xFFFF;
	int16_t ctmin_str=0x7FFF,ctmax_str=-0x7FFF;
	UNS32 vbmu=0,cap,cv_sum=0;
	int32_t Ibmu=0,ct_sum=0;
	uint8_t cvmin_mid_str,cvmax_mid_str,cvmin_cid_str,cvmax_cid_str;
	uint8_t ctmin_mid_str,ctmax_mid_str,ctmin_cid_str,ctmax_cid_str;
	uint8_t cvmin_mid_rack,cvmax_mid_rack,cvmin_cid_rack,cvmax_cid_rack;
	int8_t ctmin_mid_rack,ctmax_mid_rack,ctmin_cid_rack,ctmax_cid_rack;
	uint32_t sv_max=0,sv_min=0xFFFFFFFF;
	uint8_t svmin_sid=0,svmax_sid=0,stmin_sid=0,stmax_sid=0;
	uint8_t cvmin_sid=0,cvmax_sid=0,ctmin_sid=0,ctmax_sid=0;
	BMU_INFO tbmu;
	BATT_STR_INFO *pStrInfo;
	static u32_t loseBMU_tm[NUM_BMU_TOTAL];
	//update string parameters
	for(sid=0;sid<gSysCfg.ucNUM_STR;sid++){
		cc_max=0;dc_max=0;cvmax_str=0;cvmin_str=0xFFFF;ctmin_str=1270;ctmax_str=-1200;
		cv_sum=0;ct_sum=0;
		pStrInfo=(BATT_STR_INFO *)&strInfo[sid];
		for(mid=0;mid<gSysCfg.ucNUM_BUM_PER_STR;mid++){
			smid=sid*gSysCfg.ucNUM_BUM_PER_STR+mid;
			tbmu=bmu[smid];
			soc+=tbmu.SOC;
			soh+=tbmu.SOH;
			cc_max+=tbmu.cc_max/100;
			dc_max+=tbmu.dc_max/100;
			Ibmu+=tbmu.I_bmu;
			vbmu+=tbmu.V_bmu;
			if(tbmu.alive>0){	//received boradcast message
				(pStrInfo->ptBMU+mid)->alive=0;
				mAliveNum++;
				loseBMU_tm[mid]=OSTimeGet();
			}else if(timeDiff(loseBMU_tm[mid],OSTimeGet())<5000){
				mAliveNum++;
			}
			for(cid=0;cid<gSysCfg.ucNUM_CV_IN_BMU;cid++){
				if(tbmu.cv[cid]>cvmax_str){
					cvmax_str=tbmu.cv[cid];
					
					cvmax_cid_str=cid;
					cvmax_mid_str=mid;
				}
				if(tbmu.cv[cid]<cvmin_str){
					cvmin_str=tbmu.cv[cid];
					cvmax_cid_str=cid;
					cvmin_mid_str=mid;
				}
				cv_sum+=tbmu.cv[cid];
			}

			for(cid=0;cid<gSysCfg.ucNUM_CT_IN_BMU;cid++){
				if(tbmu.ct[cid]>ctmax_str){
					ctmax_str=tbmu.ct[cid];
					ctmax_cid_str=cid;
					ctmax_mid_str=mid;
				}
				if(tbmu.ct[cid]<ctmin_str){
					ctmin_str=tbmu.ct[cid];
					ctmax_cid_str=cid;
					ctmin_mid_str=mid;
				}
				ct_sum+=tbmu.ct[cid];
			}
	}

		pStrInfo->mod_volt=vbmu;
		pStrInfo->ptBMU=(BMU_INFO *)&bmu[mid];
		pStrInfo->bmu_alive_num=mAliveNum;
		pStrInfo->bmu_ready_num=mReadyNum;
		pStrInfo->bmu_on_num=mOnNum;
		pStrInfo->bmu_off_num=mOffNum;
	
		pStrInfo->mod_soc=soc/NUM_BMU_TOTAL;
		pStrInfo->mod_soh=soh/pStrInfo->bmu_total;
		pStrInfo->mod_clc=cc_max/100;
		pStrInfo->mod_cld=dc_max/100;
		pStrInfo->mod_curr=Ibmu;
		pStrInfo->s_cv_max=cvmax_str;
		pStrInfo->s_cv_min=cvmin_str;
		pStrInfo->s_ct_max=ctmax_str;
		pStrInfo->s_ct_min=ctmin_str;
		pStrInfo->sv_sum=cv_sum;
		pStrInfo->s_cv_avg=cv_sum/gSysCfg.ucNUM_CV_IN_BMU/gSysCfg.ucNUM_BUM_PER_STR;
		pStrInfo->s_ct_avg=ct_sum/gSysCfg.ucNUM_CT_IN_BMU/gSysCfg.ucNUM_BUM_PER_STR;
		pStrInfo->s_cvmax_cid=cvmax_cid_str;
		pStrInfo->s_cvmin_cid=cvmin_cid_str;
		pStrInfo->s_ctmax_cid=ctmax_cid_str;
		pStrInfo->s_ctmin_cid=ctmin_cid_str;
		pStrInfo->s_cvmax_mid=cvmax_mid_str;
		pStrInfo->s_cvmin_mid=cvmin_mid_str;
		pStrInfo->s_ctmax_mid=ctmax_mid_str;
		pStrInfo->s_ctmin_mid=ctmin_mid_str;
		if(Ibmu>=0){	//charge
			pStrInfo->mod_life_ahr_c+=Ibmu;
			pStrInfo->mod_life_whr_c+=((UNS64)Ibmu*vbmu)/1000;
		}else{
			pStrInfo->mod_life_ahr_d+=(UNS16)(-Ibmu);
			pStrInfo->mod_life_whr_d+=((UNS64)(-Ibmu)*vbmu)/1000;
		
		}

		setBalance(sid);

	}	
	
	
	bpInfo.str=(BATT_STR_INFO*)&strInfo[0];
	bpInfo.num_str=NUM_STR;
	bpInfo.sc_hw_version[0]=1; 
	bpInfo.sc_hw_version[1]=2; 
	bpInfo.sc_hw_version[2]=3; 

	soc=0;soh=0;Ibmu=0;vbmu=0;cc_max=0;dc_max=0;
	sv_min=0xFFFFFFF,sv_max=0;
	cvmin_str=0xFFFF;cvmax_str=0;
	ctmin_str=0xFFF;ctmax_str=0;
	for(sid=0;sid<gSysCfg.ucNUM_STR;sid++){
		pStrInfo=(BATT_STR_INFO *)bpInfo.str+sid;
		soc+=pStrInfo->mod_soc;
		soh+=pStrInfo->mod_soh;
		Ibmu+=pStrInfo->mod_curr;
		cc_max+=pStrInfo->mod_clc;
		dc_max=pStrInfo->mod_cld;
		if(pStrInfo->mod_volt>vbmu) vbmu=pStrInfo->mod_volt;
		cap+=(pStrInfo->mod_life_whr_c+pStrInfo->mod_life_whr_d)/3600000;
		if(pStrInfo->sv_sum>sv_max){
			sv_max=pStrInfo->sv_sum;
			svmax_sid=sid;
		}
		if(pStrInfo->sv_sum<sv_min){
			sv_min=pStrInfo->sv_sum;
			svmin_sid=sid;
		}
		if(pStrInfo->s_ct_min <ctmin_str){
			ctmin_str=pStrInfo->s_ct_min;
			ctmin_sid=sid;
		}
		if(pStrInfo->s_ct_max >ctmax_str){
			ctmax_str=pStrInfo->s_ct_max;
			ctmax_sid=sid;
	
		}
		if(pStrInfo->s_cv_min <cvmin_str){
			cvmin_str=pStrInfo->s_cv_min;
			cvmin_sid=sid;
		}
		if(pStrInfo->s_cv_max >cvmax_str){
			cvmax_str=pStrInfo->s_cv_max;
			cvmax_sid=sid;

		}
		if(pStrInfo->strStat==STR_RUN){
			sOnNum++;
		}
	}
	if(bpInfo.num_str>0){
		bpInfo.bp_soc=soc/bpInfo.num_str;
		bpInfo.bp_soh=soh/bpInfo.num_str;
		bpInfo.bp_curr=Ibmu;
		bpInfo.bp_VBus=vbmu;
		bpInfo.bp_clc=cc_max;
		bpInfo.bp_cld=dc_max;
		bpInfo.bp_lifw_wh=cap;
		bpInfo.sid_vmax=svmax_sid;
		bpInfo.sid_vmin=svmin_sid;
		
		bpInfo.sid_cvmax=cvmax_sid;
		bpInfo.sid_cvmin=cvmin_sid;
		bpInfo.sid_ctmin=ctmin_sid;
		bpInfo.sid_ctmax=ctmax_sid;
		bpInfo.ct_min=ctmin_str;
		bpInfo.str_on_num=sOnNum;
	}
}
	
void bmuInit(){
	uint8_t bid;
	for(bid=0;bid<NUM_BMU_TOTAL;bid++){
		memset(&bmu[bid],0,sizeof(BMU_INFO));
	}
}
void updateBMSoc(BATT_STR_INFO * bs,UNS16 interval_ms){
	
	
	//interval: 100ms
//	UINT16 updateSOC(UINT8 interval_ms){
		UINT16 totalSOC=0;
		UINT8 packNo,mid,cid;
		UINT8 bleedingCellNum;
		UINT16 bleedingEnergyLose;	//0.001VAS
		INT16 I_last_loop_5min,I_last_loop_1min,I_last_loop_30sec,I_last_loop_2sec; //current at 5min, 1min,30sec ago
		//average current at last 5min,1min,30sec. flow out:>0, flow in:<0
	//	static TICK currCbeginTick[STR_NUM_IN_CAB];
		UINT8 interval;
		static UINT8 SOC_OCV_AjuestAllowed=1;
		interval=interval_ms/100;		//interVal_ms unit is ms, so all calculation based on 0.1S (100ms)
	
	//temperatary	updateBatteryFullCapcity();
	

			
			/*battery in discharge status. or idle status(in this situation, maybe have current between two packs*/
			if(1
				&& (abso(bs->mod_curr)>10) ////flow out and I>1A (for test mistake)
				&&(isDelayExp(SECONDS(5),sysInfo.sysStartTime))
				
			){ 
				UINT32 capaDiff=0;	// pack capacity change 
				if(bs->currentDirection==0){		//discharge
					capaDiff=(UINT32)(-bs->mod_curr/100)*interval;
					bs->capacityLose += capaDiff;
					bs->capacityLoseTotal+= capaDiff;
					bs->capacity-=capaDiff;
					if(bs->capacity >bs->fullCapacity) bs->capacity=0; //capacity < 0
				//	eeprom_ram_para.sstrCapacity[packNo]=bm->capacity; //save a copy in eeprom
					
					// for record Energy lost to balance and power
					bleedingEnergyLose=(UINT32)bleedingCellNum*38*38*interval/10;	//0.001VAS
					
				//nie	bs->BalancePower=bleedingEnergyLose/interval;//0.001watt
				//nie	bs->EngLostToBalance+=bleedingEnergyLose/10;//0.01VAS
	
					bs->powerLose += (UINT32)(-bs->mod_curr/100)*bs->mod_volt/100*interval;
					bs->powerLose +=bleedingEnergyLose;	//0.001VAS
	
					bs->powerLoseTotal+= (UINT64)(bs->mod_curr/100)*bs->mod_volt/100*interval;
					bs->powerLoseTotal+= bleedingEnergyLose;	//0.001VAS
					
	
					
				}else{		//charge
					capaDiff=(UINT32)(abso(bs->mod_curr)/100) *interval;
					bs->capacityGain+=capaDiff;
					bs->capacityGainTotal+=capaDiff;
					bs->capacity=bs->capacity + capaDiff;
					if(bs->capacity >bs->fullCapacity) bs->capacity=bs->fullCapacity; 
	
					bs->powerGain+=(UINT64)bs->mod_curr/100*bs->mod_volt/100*interval;
					bs->powerGainTotal+=(UINT64)bs->mod_curr/100*bs->mod_volt/100*interval;
				}
				bs->mod_soc=(UNS16)((UINT64)bs->capacity*1000/bs->fullCapacity);
				#if 0 //nie
				//save in eeprom
				eeprom_ram_para.capacityGainTotal[STR_NUM_IN_CAB]=bm->capacityGainTotal;
				eeprom_ram_para.capacityLoseTotal[STR_NUM_IN_CAB]=bm->capacityLoseTotal;
				//kwallt_dischg
				eeprom_ram_para.KWattHrs_Chg[STR_NUM_IN_CAB]=bm->powerGainTotal/360000000;
				//DisChgAH
				eeprom_ram_para.KWattHrs_DisChg[STR_NUM_IN_CAB]=bm->powerLoseTotal/360000000;
				//DisChgAH
				eeprom_ram_para.DisChgAH[STR_NUM_IN_CAB]=bm->capacityLoseTotal/36000;
				//chgAH 
				eeprom_ram_para.ChgAH[STR_NUM_IN_CAB]=(UINT64)bm->capacityGainTotal/36000;
				#endif
			}
	

				//SOC-OCV calibrate
				if(1
					&&(bs->bmu_alive_num>0)
				//&&(strInfo.bmu_alive_num==strInfo.bmu_total)
		//		&&(systemInfo.CVErrCnt==0)
		//		&&(systemInfo.CTErrCnt==0)
		//		&&(systemStatus!=SYS_WAIT)
		//		&&(systemStatus != SYS_ALL_PACK_CHG)
				 &&(isDelayExp(SECONDS(4),sysInfo.sysStartTime))
				 &&(!isDelayExp(SECONDS(10),sysInfo.sysStartTime))
				 //&&(strInfo.SOC_OCV_adjusted==0)
				 &&(abso(bs->mod_curr)<10)
				 &&(bs->s_cv_min >0) 
				 &&(bs->s_cv_min<5000)
				 &&(bs->s_cv_max<5000)
				){
						UINT16 soc;
					//nie	systemInfo.HVBBattSOCAdjCriMetOCV=1;
						soc=SOC_OCV_Cal(bs,bs->mod_soc);
						bs->mod_soc=soc;
						SOC_OCV_AjuestAllowed=0;
						bs->SOC_OCV_adjusted=1;
						bs->capacity=((UINT64)(bs->mod_soc)*bs->fullCapacity)/1000;
				}
	
	
	#if 0
		//update SYSTEMINFO
		UINT32 cg=0,cl=0,cgl=0,cll=0;
		UINT64 pg=0,pl=0,pgl=0,pll=0;
				cg+=bm->capacityGain;
				cl+=bm->capacityLose;
				cgl+=bm->capacityGainTotal;
				cll+=bm->capacityLoseTotal;
				pg+=bm->powerGain;
				pgl+=bm->powerGainTotal;
				pl+=bm->powerLose;
				pll+=bm->powerLoseTotal;
				bm->SOC=((UINT64)(bm->capacity)*1000/bm->fullCapacity);
			systemInfo.powerGain=pg;
			systemInfo.powerGainTotal=pgl;
			systemInfo.powerLose=pl;
			systemInfo.powerLoseTotal=pll;
			systemInfo.capacityGain=cg;
			systemInfo.capacityLose=cl;
			systemInfo.capacityGainTotal=cgl;
			systemInfo.capacityLoseTotal=cll;
			
			eeprom_ram_para.syscapacityGainTotal=systemInfo.capacityGainTotal;
			eeprom_ram_para.syscapacityLoseTotal=systemInfo.capacityLoseTotal;
			//kwallt_dischg
			eeprom_ram_para.sysKWattHrs_Chg=systemInfo.powerGainTotal/360000000;
			//DisChgAH
			eeprom_ram_para.sysKWattHrs_DisChg=systemInfo.powerLoseTotal/360000000;
	
			//DisChgAH
			eeprom_ram_para.sysDisChgAH=systemInfo.capacityLoseTotal/36000;
			//chgAH 
			eeprom_ram_para.sysChgAH=(UINT64)systemInfo.capacityGainTotal/36000;
	
	
			if(systemInfo.currentSensorValidity==V_VALID){
				int currentDirection;
				if(systemInfo.currentDirection) currentDirection=1;
				else currentDirection=-1;
	
				if(systemInfo.voltBatteryValidity==V_VALID) systemInfo.power=(INT64)currentDirection*systemInfo.voltBattery*systemInfo.current/100;
				else if(HVBMsg.HVBBusVoltageValidity==V_VALID) systemInfo.power=(INT64)currentDirection*systemInfo.voltBus*systemInfo.current/100;
				else systemInfo.power=0;
			}else{
				systemInfo.power=0;
			}
	
	
		 
	
	
	
		totalSOC=0;
		for(packNo=0;packNo<STR_NUM_IN_CAB;packNo++){
			if(isACPresent(0)){
				totalSOC += bm->SOC;
			}else{
				totalSOC += bm->SOC;
			}
			eeprom_ram_para.sstrSOC[packNo]=bm->SOC;
		}
		systemInfo.SOC =totalSOC/STR_NUM_IN_CAB;
		uiBattTaskStage=206;
	#endif
	//	return 1;
	
	}
	
	



UINT16 SOC_OCV_Cal(BATT_STR_INFO *bm,UINT16 savedSOC){
	UINT16 minSOC, maxSOC, avgSOC,deltaMinSOC, deltaMaxSOC, blendedSOC,retSOC,ampSOC;
	UINT8 socadj;

	
	minSOC=SOC_OCV_Lookup(bm->s_ct_min/10,bm->s_cv_min);
	maxSOC=SOC_OCV_Lookup(bm->s_ct_min/10,bm->s_cv_max);
	avgSOC=SOC_OCV_Lookup(bm->s_ct_min/10,bm->s_cv_max);
	
	//cal SOCamp
		deltaMinSOC=avgSOC-minSOC;
		deltaMaxSOC=maxSOC-avgSOC;
		ampSOC=avgSOC;
	
	//cal SOCblended
	if(ampSOC<SOC_CAL_LOW){
		if(ampSOC>deltaMinSOC) blendedSOC=ampSOC-deltaMinSOC;
		else blendedSOC=0;
	
	}else if(ampSOC>SOC_CAL_HIGH){
		blendedSOC=ampSOC+deltaMaxSOC;
		if(blendedSOC>1000) blendedSOC=1000;
	}else{
		blendedSOC=(UINT32)(ampSOC+deltaMaxSOC)*(ampSOC-SOC_CAL_LOW)/(SOC_CAL_HIGH-SOC_CAL_LOW);
		blendedSOC+=(UINT32)(ampSOC-deltaMinSOC)*(SOC_CAL_HIGH - ampSOC)/(SOC_CAL_HIGH-SOC_CAL_LOW);
	}

	retSOC=savedSOC;
	socadj=1;
	if(blendedSOC>=(savedSOC+250)){
		retSOC=blendedSOC;
//nie		bm->AHrAdjTgt=((INT32)(systemInfo.fullCapacity/1000)*(blendedSOC - savedSOC));	//AHrAdj is 0.01A.S, fullCapacity is A.S
//nie		bm->AHrAdjLeft=0;
//		sstrinfo[0].HVBBattSOCAdj=1;
	}else if(blendedSOC>=(savedSOC+100)){
		retSOC=blendedSOC;
//nie		bm->AHrAdjTgt=((INT32)(systemInfo.fullCapacity/1000)*(blendedSOC - savedSOC));	//AHrAdj is 0.01A.S, fullCapacity is A.S
//nie		bm->AHrAdjLeft=0;
/* from 11.8.2    not adjust SOC if target SOC big than exist SOC and difference lower than 25%
		systemInfo.AHrAdjTgt=((INT32)(systemInfo.fullCapacity/1000)*(blendedSOC - savedSOC));	//AHrAdj is 0.01A.S, fullCapacity is A.S
		systemInfo.AHrAdjLeft=systemInfo.AHrAdjTgt;
		systemInfo.HVBBattSOCAdj=1;
*/
//10.51.5	}else if(blendedSOC>=(savedSOC+50)){
	}else if(blendedSOC>=(savedSOC+80)){
/* from 11.8.2    not adjust SOC if target SOC big than exist SOC and difference lower than 25%
		systemInfo.AHrAdjTgt=((INT32)(systemInfo.fullCapacity/1000)*(blendedSOC - savedSOC));	//AHrAdj is 0.01A.S, fullCapacity is A.S
		systemInfo.AHrAdjLeft=systemInfo.AHrAdjTgt;
		systemInfo.HVBBattSOCAdj=0;
*/
	}else if(savedSOC >= (blendedSOC+250)){
//nie		bm->AHrAdjTgt=((INT32)(systemInfo.fullCapacity/1000)*(savedSOC- blendedSOC));	//AHrAdj is 0.01A.S, fullCapacity is A.S
//nie		bm->AHrAdjLeft=0;
		retSOC=blendedSOC;
//nie		systemInfo.HVBBattSOCAdj=1;
//10.51.5	}else if(savedSOC>=(blendedSOC+50)){
	}else if(savedSOC>=(blendedSOC+80)){
		retSOC=blendedSOC;
	//nei	bm->AHrAdjTgt=((INT32)(systemInfo.fullCapacity/1000)*(savedSOC - blendedSOC));	//AHrAdj is 0.01A.S, fullCapacity is A.S
	//nie	bm->AHrAdjLeft=0;

/*		systemInfo.AHrAdjTgt=-((INT32)(systemInfo.fullCapacity/1000)*(savedSOC - blendedSOC));	//AHrAdj is 0.01A.S, fullCapacity is A.S
		systemInfo.AHrAdjLeft=systemInfo.AHrAdjTgt;
		systemInfo.HVBBattSOCAdj=0;
*/
	}else{
		socadj=0;
		bm->AHrAdjTgt=0;
		bm->AHrAdjLeft=0;
//		bm->HVBBattSOCAdj=0;
	}



	retSOC=minSOC;	// to be sample
	
	return retSOC;
	
}

/*
	UINT16 current : 0.1A
	UINT8 currentDirection: 0 - in, 1- out
	INT8 temp: -40 to 85
	unsigned int Vmin: mv
*/
UINT16 SOC_OCV_Lookup(INT8 temp,UINT16 Vcell)
{
	int intSOC;
	#define TEMP_N40_LOW	0
	#define TEMP_N40_N30	1
	#define TEMP_N30_N20	2
	

	UINT16 SOCTable[11]={3500,3670,3740,3810,3880,3930,4000,4050,4100,4140,4200};
 	int i,tem,j,k,index_table_i,index_table_j,index_table_k;
 	int count=0;
	float fSOC;
	UINT16 I;
	UINT8   Idir;
	UINT16 * socTbl_1, *socTbl_2, *socTbl;

	if(temp<-40) {
		SOCTable[0]=3320;SOCTable[1]=3470;SOCTable[2]=3570;SOCTable[3]=3620;SOCTable[4]=3650;
		SOCTable[5]=3690;SOCTable[6]=3730;SOCTable[7]=3790;SOCTable[8]=3860;SOCTable[9]=3940;SOCTable[10]=4050;
	}else if(temp<-30){
		SOCTable[0]=3320;SOCTable[1]=3470;SOCTable[2]=3570;SOCTable[3]=3620;SOCTable[4]=3650;
		SOCTable[5]=3690;SOCTable[6]=3730;SOCTable[7]=3790;SOCTable[8]=3860;SOCTable[9]=3940;SOCTable[10]=4050;
	}else if(temp<5){
		SOCTable[0]=3320;SOCTable[1]=3470;SOCTable[2]=3570;SOCTable[3]=3620;SOCTable[4]=3650;
		SOCTable[5]=3690;SOCTable[6]=3730;SOCTable[7]=3790;SOCTable[8]=3860;SOCTable[9]=3940;SOCTable[10]=4050;
	}else if(temp<40){
		SOCTable[0]=3320;SOCTable[1]=3470;SOCTable[2]=3570;SOCTable[3]=3620;SOCTable[4]=3650;
		SOCTable[5]=3690;SOCTable[6]=3730;SOCTable[7]=3790;SOCTable[8]=3860;SOCTable[9]=3940;SOCTable[10]=4050;
	}else if(temp<50){
		SOCTable[0]=3320;SOCTable[1]=3470;SOCTable[2]=3570;SOCTable[3]=3620;SOCTable[4]=3650;
		SOCTable[5]=3690;SOCTable[6]=3730;SOCTable[7]=3790;SOCTable[8]=3860;SOCTable[9]=3940;SOCTable[10]=4050;
	}else if(temp<60){
		SOCTable[0]=3320;SOCTable[1]=3470;SOCTable[2]=3570;SOCTable[3]=3620;SOCTable[4]=3650;
		SOCTable[5]=3690;SOCTable[6]=3730;SOCTable[7]=3790;SOCTable[8]=3860;SOCTable[9]=3940;SOCTable[10]=4050;
	}
		

  i=0;  j=10;
  if (Vcell>SOCTable[10]) {
     	intSOC=100;
  }else  if (Vcell<SOCTable[0]) {
  	intSOC=0;
  }else{
	  for (;;){
		    count++;
		    index_table_i=i;
		    index_table_j=j;
		    index_table_k=(index_table_i+index_table_j)/2;
		    k=index_table_k;
		    if (j-i<=1) {
		      	tem=i*10+(float)(Vcell-SOCTable[index_table_i])/(float)(SOCTable[index_table_j]-SOCTable[index_table_i])*10;
				intSOC=tem;
				break;
	          }else{
			    if  (Vcell>SOCTable[index_table_k])      i=k;
			    else if (Vcell<SOCTable[index_table_k])      j=k;
			    else{
					intSOC=index_table_k*10;
					break;
				}

		    }  
	    }
   }
	return intSOC*10;
		
}
//new version power limit

void strPlimSet(BATT_STR_INFO *bm){

	#define PL_DERATE_MODE 1
	#define PL_NORMAL_MODE 0
// Minimum Charge Power at low SOC
#define MPchg 15000 /* minimum charge power at low SOC (W) */
static UINT8 plimcMode=PL_NORMAL_MODE;
static UINT8 plimdMode=PL_NORMAL_MODE;
	UINT16  PowLimC, PowLimD, contPowD, contPowC ;
	UINT16 plimcT = (UINT16)FPchg ;
	UINT16 plimcV =(UINT16) FPchg ;
	UINT16 plimcSOC = (UINT16)FPchg ;
	UINT16 plimdT =(UINT16) FPdis ;
	UINT16 plimdV = (UINT16)FPdis ;
	UINT16 plimdSOC = (UINT16)FPdis ;
	UINT16 CA,CB,CV,CP,CQ;	//some constant
	//static TICK cDerateBeginTick,cNomrolBeginTick;
	//static TICK dDerateBeginTick,dNomrolBeginTick;
	UINT16 NPL4,NPL3,NPL2,NPL1,NPL;
	UINT32 avgPow30sec,avgPow1_8sec;

//UINT16 plim[101];
INT8 i,j,tempSensorFailCount,voltSensorFailCount;
//nie	if((Iavg_d_5min[0]>600) ||(Iavg_d_1min[0]>750) ||(Iavg_d_30sec[0]>900)){	// Iavg_5min>60A or Iavg_1min>75A or Iavg_30sec>90A
if(1){
		CA=48; CB=25;
	}else{
		CA=60; CB=28;
	}
/*
for(i=0;i<=100;i++){plim[i]=0;}
systemInfo.cellVltMin=3000;
systemInfo.cellVltMax=3000;

for(i=0;i<=100;i++){
	plimcT = (UINT16)FPchg ;
	plimcV =(UINT16) FPchg ;
	plimcSOC = (UINT16)FPchg ;
	plimdT =(UINT16) FPdis ;
	plimdV = (UINT16)FPdis ;
	plimdSOC = (UINT16)FPdis ;

	systemInfo.cellVltMin+=i*10;
	systemInfo.cellVltMax+=i*10;
	systemInfo.SOC=(UINT16)i*10;
	systemInfo.current=100;
	systemInfo.cellTempMax=40;
	systemInfo.voltBattery=3700;

*/

/*power limit calibrated by cell voltage */
	//discharge power limit by cell voltage
	NPL4=FPdis;
		if((bm->s_cv_min) >3360){ 
			NPL4=FPdis;
		}else if(bm->s_cv_min>3200){
//			NPL4=FPdis -(UINT32)FPdis*(100-CA)*(3360-bm->s_cv_min)/16000;
			NPL4=FPdis -(UINT32)FPdis*(3360-bm->s_cv_min)/250;	//when Vcell_min is 3.2V, power limit change to 36% of full pl
		}else if(bm->s_cv_min>3150){
			NPL4=(UINT32)FPdis*(bm->s_cv_min - 3150)/150;
		}else NPL4=0;
	NPL1=FPdis;
	NPL2=FPdis;
#if 1

/* SOC calication */
	
	//discharge power limit
		if (bm->mod_soc  <=30){
			NPL3=0;
		}else if (bm->mod_soc  <=300){
			NPL3  =(UINT32) 35000-(UINT32)(30 - bm->mod_soc/10)*(30 - bm->mod_soc/10)*464/10; 
		}else if (bm->mod_soc  <=550){
			NPL3  =(UINT32) 35000+(UINT32)(bm->mod_soc/10-30)*(bm->mod_soc/10-30)*464/10; 
		}else if (bm->mod_soc  <= 1000) {
			NPL3=FPdis;
		}
#endif	
/* temperature calibration */		
NPL=NPL1;
if(NPL>NPL2) NPL=NPL2;
if(NPL>NPL3) NPL=NPL3;
if(NPL>NPL4) NPL=NPL4;

	if(bm->s_ct_max>ThighH){
		NPL=0;
	}else if(bm->s_ct_max>ThighS){
		NPL=(UINT32)NPL*(ThighH-bm->s_ct_max)/(ThighH-ThighS);
	}else if(bm->s_ct_max<25){
		if(bm->s_ct_min<-40){
		 	NPL=0;
		}else if(bm->s_ct_min<15){
	 		NPL=(UINT32)NPL*(bm->s_ct_min+40)*(bm->s_ct_min+40)/3025;
		}
	}


	PowLimD = NPL;


//charge power limit

/*power limit calibrated by cell voltage */
	plimcV=FPchg;

			if (bm->s_cv_max>4100) plimcV = 0 ;
			else if (bm->s_cv_max > 3950) {
//				plimcV=FPchg - (UINT32)(bm->s_cv_max-V40)/4; //*250/1000
#ifdef PRO_HATCH
				plimcV=FPchg - (UINT32)(bm->s_cv_max-3950)*120; //*250/1000
#else
				plimcV=FPchg - (UINT32)(bm->s_cv_max-3950)*100; //*250/1000
#endif
			}else if(bm->s_cv_max>3250){
//				plimcV=(UINT32)sstrinfo[strid].strVolt*13;
				//plimcV=(UINT32)bm->mod_volt*4;
				if(plimcV>FPchg) plimcV=FPchg;
					
			}else if(bm->s_cv_max<=3250){
			
				plimcV =FPchg - (UINT32)FPchg*(3250-bm->s_cv_max)/250;
				//nie if(plimcV>(UINT32)(bm->mod_volt)*13) plimcV=(UINT32)bm->mod_volt*13;
			}
#if 1
/* SOC calication */
	if (bm->mod_soc >900) plimcSOC = (UINT32)FPchg*(1000-bm->mod_soc)*3/550;
	
	else if (bm->mod_soc > 600) plimcSOC = (UINT32)FPchg*(1260-bm->mod_soc)/660;
	else if (bm->mod_soc  > 200) {
		plimcSOC = FPchg ;
	}else{
		plimcSOC=(UINT32)FPchg*(900 + bm->mod_soc)/1100;
	}
#endif

    plimcT=plimcV;
   if(plimcT>plimcSOC) plimcT=plimcSOC;
    
/* temperature calication */
	if(bm->s_ct_max>ThighH) PowLimC=0;
	else if(bm->s_ct_max>ThighS){
		PowLimC=(UINT32)plimcT*(ThighH-bm->s_ct_max)/(ThighH-ThighS); 
	}else if(bm->s_ct_max>15){
		PowLimC=plimcT;
	}else if(bm->s_ct_min>(-20)){
		PowLimC=(UINT32)plimcT*(bm->s_ct_max+20)*(bm->s_ct_max+20)/35/35;
	}else{
		PowLimC=0;
	}
		
	

//get temperature sensor fail count
tempSensorFailCount=0;
for(i=0;i<4;i++){
	for(j=0;j<24;j++){
//		if(!((sstrinfo[0].cellTmpValidity[i]>>j) & 0x01)) tempSensorFailCount++;
	}
}
voltSensorFailCount++;
for(i=0;i<4;i++){
	for(j=0;j<24;j++){
//		if(!((sstrinfo[0].cellVolValidity[i]>>j) & 0x01)) voltSensorFailCount++;
	}
}

#if 0
	if(systemInfo.bmsDebugMode==1){
		if((PowLimD>PowLimD_Last[strid]) && (sstrinfo[strid].current>20) && (sstrinfo[strid].currentDirection ==1)){
			PowLimD=PowLimD_Last[strid];
		}else{
			PowLimD_Last[strid]=PowLimD;
		}
		if((PowLimC>PowLimC_Last[strid]) && (sstrinfo[strid].current<(-20))  && (sstrinfo[strid].currentDirection ==0)){
			PowLimC=PowLimC_Last[strid];
		}else{
			PowLimC_Last[strid]=PowLimC;
		}

	}
#endif
	
	bm->mod_pld=PowLimD;
	bm->mod_plc=PowLimC;



	
}
UNS32 abs(UNS32 a, UNS32 b){
	if(a>b) return (a-b);
	else return (b-a);
}
UNS32 abso(INT32 a){
	if(a>=0) return (a);
	else return (-a);
}
BOOLEAN isSysFaultExist(){
	BOOLEAN ret=0;
	if(0
		//||(bpInfo.str_on_num<NUM_BMU_TOTAL)
		||(bpInfo.str_on_num<0)
	){
		ret=1;
	}
	return ret;
}

BOOLEAN isStrFaultExist(UNS8 sid){
	BOOLEAN ret=0;
	if(isDelayExp(5000,sysInfo.sysStartTime)==0){
		return 0;
	}
	if(0
		//||(LID_SW_STAT==0)
		//||(MANUAL_SW_STAT==0)
		||(str_tc[sid][F_STR_OV].tc_stat)
		||(str_tc[sid][F_STR_UV].tc_stat)
		||(str_tc[sid][F_STR_OT].tc_stat)
		||(str_tc[sid][F_STR_UT].tc_stat)
		||(str_tc[sid][F_STR_DOC].tc_stat)
		||(str_tc[sid][F_STR_COC].tc_stat)
		){
		ret=1;
	}
		
	return ret;
}
BOOLEAN strFaultClr(UNS8 sid){
	BOOLEAN ret=0;
	str_tc[sid][F_STR_OV].tc_stat=0;
	str_tc[sid][F_STR_UV].tc_stat=0;
	str_tc[sid][F_STR_OT].tc_stat=0;
	str_tc[sid][F_STR_UT].tc_stat=0;
	str_tc[sid][F_STR_DOC].tc_stat=0;
	str_tc[sid][F_STR_COC].tc_stat=0;
	strAlarmClr();
	ret=1;
	return ret;
}
BOOLEAN isStrAlarmExist(UNS8 sid){
	BOOLEAN ret=0;
	if(isDelayExp(5000,sysInfo.sysStartTime)==0){
		return 0;
	}
	if(0
		//||(LID_SW_STAT==0)
		//||(MANUAL_SW_STAT==0)
		||(str_tc[sid][F_STR_OV].tc_stat)
		||(str_tc[sid][A_STR_OV].tc_stat)
		||(str_tc[sid][F_STR_UV].tc_stat)
		||(str_tc[sid][A_STR_UV].tc_stat)
		||(str_tc[sid][F_STR_OT].tc_stat)
		||(str_tc[sid][A_STR_OT].tc_stat)
		||(str_tc[sid][F_STR_UT].tc_stat)
		||(str_tc[sid][A_STR_UT].tc_stat)
		||(str_tc[sid][F_STR_DOC].tc_stat)
		||(str_tc[sid][A_STR_DOC].tc_stat)
		||(str_tc[sid][F_STR_COC].tc_stat)
		||(str_tc[sid][A_STR_COC].tc_stat)
		){
			ret=1;
		}else{
			ret=0;
		}

	return ret;
}
BOOLEAN strAlarmClr(UNS8 sid){
	BOOLEAN ret=0;
		str_tc[sid][F_STR_OV].tc_stat=0;
		 str_tc[sid][A_STR_OV].tc_stat=0;
		 str_tc[sid][F_STR_UV].tc_stat=0;
		 str_tc[sid][A_STR_UV].tc_stat=0;
		 str_tc[sid][F_STR_OT].tc_stat=0;
		 str_tc[sid][A_STR_OT].tc_stat=0;
		 str_tc[sid][F_STR_UT].tc_stat=0;
		 str_tc[sid][A_STR_UT].tc_stat=0;
		 str_tc[sid][F_STR_DOC].tc_stat=0;
		 str_tc[sid][A_STR_DOC].tc_stat=0;
		 str_tc[sid][F_STR_COC].tc_stat=0;
		 str_tc[sid][A_STR_COC].tc_stat=0;
		ret=1;
	return ret;
}

BOOLEAN isWarningExist(UNS8 sid){
	BOOLEAN ret=0;
	if(0
		||(str_tc[sid][W_STR_OV].tc_stat)
		||(str_tc[sid][W_STR_UV].tc_stat)
		||(str_tc[sid][W_STR_OT].tc_stat)
		||(str_tc[sid][W_STR_UT].tc_stat)
		||(str_tc[sid][W_STR_DOC].tc_stat)
		||(str_tc[sid][W_STR_COC].tc_stat))
		{
		ret=1;
	}else{
		ret=0;
	}
	return ret;
}
BOOLEAN isHWShutdownExist(){
	BOOLEAN ret=0;
	if(isDelayExp(5000,sysInfo.sysStartTime)==0){
		return 0;
	}
	if((0
		||(vcuInfo.vcuCmd==VCMD_SHUTDOWN)
		//||(LID_SW_STAT==0)
		//||(MANUAL_SW_STAT==0)
		)){
			ret=1;
		}else{
			ret=0;
	}
	return ret;
}
void strTCCheck(UNS8 sid){
	FAW_ID_T tcid;
	TCODE_T *ptc;
	TCODE_T *pStart;
	TCODE_T tc;
	UINT32 rtVal;
	static uint16_t tm_rem_s[NUM_STR][TC_NUM]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static uint16_t tm_rem_r[NUM_STR][TC_NUM]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	pStart=(TCODE_T *)&str_tc[sid][0];
	for(tcid=0;tcid<20;tcid++){
		rtVal=getStrTCVal(sid,tcid);
		ptc=pStart+tcid;
		if (ptc->tc_stat==0){  //no fault
			if(ptc->dir>0){	/* upside threshold */
				if(rtVal > ptc->thd_s){
					tm_rem_s[sid][tcid]++;
					if (tm_rem_s[sid][tcid] > ptc->tm_s){
						ptc->tc_stat=1;
						tm_rem_r[sid][tcid]=0;
					}
				}else{
					tm_rem_s[sid][tcid]=0;
				}
			}else{
				if(rtVal < ptc->thd_s){
					tm_rem_s[sid][tcid] ++;
					if (tm_rem_s[sid][tcid] > ptc->tm_s){
						ptc->tc_stat=1;
						tm_rem_r[sid][tcid]=0;
					}
				}else{
					tm_rem_s[sid][tcid]=0;
				}
			}
		}else{	//fault exist
			if(ptc->tm_r !=0xFFFF){	//fault recoverable
			if(ptc->dir>0){	/* upside threshold reset when val<threshold */
				if(rtVal < ptc->thd_r){
					tm_rem_r[sid][tcid] ++;
					if (tm_rem_r[sid][tcid] > ptc->tm_r){
						ptc->tc_stat=0;
						tm_rem_s[sid][tcid]=0;
					}
				}else{
					tm_rem_r[sid][tcid]=0;
				}
			}else{
				if(rtVal > ptc->thd_r){	//lowside. reset when val>threshold
					tm_rem_r[sid][tcid] ++;
					if (tm_rem_r[sid][tcid] > ptc->tm_r){
						ptc->tc_stat=0;
						tm_rem_s[sid][tcid]=0;
					}
				}else{
					tm_rem_r[sid][tcid]=0;
				}
			}
			}
	}
	}
}


void TCTableInit(void){
	
	FAW_ID_T fawId;
	UNS8 sid;
	int i;
	for(sid=0;sid<NUM_STR;sid++){
	for(i=0;i<TC_NUM;i++){
		str_tc[sid][i].id=i;
	}
	fawId=F_STR_OV;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=4150;
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=3900;
	str_tc[sid][fawId].tm_r=300;
	fawId=A_STR_OV;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=4050;
	str_tc[sid][fawId].tm_s=1000;
	str_tc[sid][fawId].thd_r=3900;
	str_tc[sid][fawId].tm_r=300;
	fawId=W_STR_OV;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=4000;
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=3900;
	str_tc[sid][fawId].tm_r=100;
	fawId=F_STR_UV;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=0;
	str_tc[sid][fawId].thd_s=2950;
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=3200;
	str_tc[sid][fawId].tm_r=300;
	fawId=A_STR_UV;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=0;
	str_tc[sid][fawId].thd_s=3000;
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=3150;
	str_tc[sid][fawId].tm_r=300;
	fawId=W_STR_UV;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=0;
	str_tc[sid][fawId].thd_s=3050;
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=3100;
	str_tc[sid][fawId].tm_r=100;
	fawId=F_STR_OT;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=550; // 0.1C
	str_tc[sid][fawId].tm_s=100;
	str_tc[sid][fawId].thd_r=400; // 0.1C
	str_tc[sid][fawId].tm_r=500;
	fawId=A_STR_OT;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=500; // 0.1C
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=450; // 0.1C
	str_tc[sid][fawId].tm_r=100;
	fawId=W_STR_OT;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=480; // 0.1C
	str_tc[sid][fawId].tm_s=46;
	str_tc[sid][fawId].thd_r=460; // 0.1C
	str_tc[sid][fawId].tm_r=100;
	fawId=F_STR_UT;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=0;
	str_tc[sid][fawId].thd_s=(UINT32)-350; // 0.1C
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=(UINT32)-100; // 0.1C
	str_tc[sid][fawId].tm_r=100;
	fawId=A_STR_UT;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=0;
	str_tc[sid][fawId].thd_s=(UINT32)-300; // 0.1C
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=(UINT32)-100; // 0.1C
	str_tc[sid][fawId].tm_s=100;
	fawId=W_STR_UT;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=0;
	str_tc[sid][fawId].thd_s=(UINT32)-100; // 0.1C
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=(UINT32)-80; // 0.1C
	str_tc[sid][fawId].tm_r=100;
	
	fawId=F_STR_DOC;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=50000; //mA
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=2000; //mA
	str_tc[sid][fawId].tm_r=100;
	fawId=A_STR_DOC;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=40000; //mA
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=35000; //mA
	str_tc[sid][fawId].tm_r=0;
	fawId=W_STR_DOC;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=30000; //mA
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=30000; //mA
	str_tc[sid][fawId].tm_r=100;
	fawId=F_STR_COC;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=35000; //mA
	str_tc[sid][fawId].tm_s=300;	//10ms 
	str_tc[sid][fawId].thd_r=30000; //mA
	str_tc[sid][fawId].tm_r=100;
	fawId=A_STR_COC;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=30000; //mA
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=25000; //mA
	str_tc[sid][fawId].tm_r=100;
	fawId=W_STR_COC;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=25000; //mA
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=25000; //mA
	str_tc[sid][fawId].tm_r=0;


	fawId=F_HEATER_OT;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=100; //C
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=100; //C
	str_tc[sid][fawId].tm_r=0;
		fawId=A_HEATER_OT;
		str_tc[sid][fawId].tc_stat=0;
		str_tc[sid][fawId].dir=1;
		str_tc[sid][fawId].thd_s=100; //C
		str_tc[sid][fawId].tm_s=300;
		str_tc[sid][fawId].thd_r=100; //C
		str_tc[sid][fawId].tm_r=0;
		fawId=F_PK_DUT;
		str_tc[sid][fawId].tc_stat=0;
		str_tc[sid][fawId].dir=0;
		str_tc[sid][fawId].thd_s=(UINT32)-350; //0.1C
		str_tc[sid][fawId].tm_s=300;
		str_tc[sid][fawId].thd_r=(UINT32)-100; //0.1C
		str_tc[sid][fawId].tm_r=0;
		fawId=F_PK_CUT;
		str_tc[sid][fawId].tc_stat=0;
		str_tc[sid][fawId].dir=0;
		str_tc[sid][fawId].thd_s=(UINT32)-300; //0.1C
		str_tc[sid][fawId].tm_s=300;
		str_tc[sid][fawId].thd_r=(UINT32)-100; //0.1C
		str_tc[sid][fawId].tm_r=0;
	
	fawId=F_PK_DOC;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=500000; //mA
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=200000; //mA
	str_tc[sid][fawId].tm_r=0;
	fawId=F_PK_COC;
	str_tc[sid][fawId].tc_stat=0;
	str_tc[sid][fawId].dir=1;
	str_tc[sid][fawId].thd_s=500000; //mA
	str_tc[sid][fawId].tm_s=300;
	str_tc[sid][fawId].thd_r=200000; //mA
	str_tc[sid][fawId].tm_r=0;

	
}
}
UNS8 getStrTCStat(UNS8 sid,FAW_ID_T tcid){
	return str_tc[sid][tcid].tc_stat;
}
		
static UNS32 getStrTCVal(UNS8 sid,FAW_ID_T tcid){
		UNS32 val;
		switch(tcid){
		case F_STR_OV:
		case A_STR_OV:
		case W_STR_OV:
			val=strInfo[sid].s_cv_max;
			break;
		case F_STR_UV:
		case A_STR_UV:
		case W_STR_UV:
			val=strInfo[sid].s_cv_min;
			break;
		case F_STR_OT:
		case A_STR_OT:
		case W_STR_OT:
			if(strInfo[sid].s_ct_max<=0) val=0;
			val=(UINT32)strInfo[sid].s_ct_max;
			break;
		case F_STR_UT:
		case A_STR_UT:
		case W_STR_UT:
			if(strInfo[sid].s_ct_min>=0) val=0xFFFFFFFF;
			else val=(UINT32)strInfo[sid].s_ct_min;
			break;
		case F_STR_DOC:
		case A_STR_DOC:
		case W_STR_DOC:
			if(strInfo[sid].mod_curr>=0) val=0;	//charging
			else val=(UINT32)(-strInfo[sid].mod_curr);
			break;
		case F_STR_COC:
		case A_STR_COC:
		case W_STR_COC:
			if(strInfo[sid].mod_curr>=0) val=strInfo[sid].mod_curr;	//charging
			else val=(UINT32)(0);
			break;
		case F_CV_DIFF:
		case A_CV_DIFF:
		case W_CV_DIFF:
			val=(strInfo[sid].s_cv_max>strInfo[sid].s_cv_min)?(strInfo[sid].s_cv_max-strInfo[sid].s_cv_min):0;
			break;
		case F_HEATER_OT:
		case A_HEATER_OT:
		case W_HEATER_OT:
			//bpInfo.
			break;
		case F_PK_DUT:
		case F_PK_CUT:
				if(strInfo[sid].s_ct_min>=0) val=0xFFFFFFFF;
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

static void setBalance(UNS8 sid){
	uint8_t mid,cid;
	for(mid=sid*gSysCfg.ucNUM_BUM_PER_STR;mid<(sid+1)*gSysCfg.ucNUM_BUM_PER_STR;mid++){
		for(cid=0;cid<gSysCfg.ucNUM_CV_IN_BMU;cid++){
			if((bmu[mid].cv[cid]-strInfo[sid].s_cv_min)>10){
				bmu[mid].cbCtrl.wd |= (UINT32)1<<cid;
			}else{
				bmu[mid].cbCtrl.wd &= ~((UINT32)1<<cid);
			}
		}
	}
		
}

