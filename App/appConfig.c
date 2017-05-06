/***********************************************************************
***********************************************************************/
#include "main.h"  
#include "config.h"
#include "task_main.h"
void SCInit(void );
void BMInit(void);
void SysVarInit(void);
void BPInit(void);

void sysConfigInit(void);


/***********************************************************************
 ***********************************************************************/
void SCInit(void){
	sysConfigInit();
	bmuInit();
	
	BPInit();
	//BMinit must after BPInit
	BMInit();
	SysVarInit();
	vcuInfo.keep_alive_req=VCMD_NA;
	sysInfo.sysStartTime=OSTime;
}
/***********************************************************************
function: fetch battery module infromation from NVM
 ***********************************************************************/
void BMInit(void){
	UNS16 bmsize;
	UNS8 sid;
	bmsize=sizeof(strInfo);
	df_read_open(ADDR_BM_IN_NVM);
	SPI_FLASH_CS_HIGH();
	df_read((UNS8 *)&strInfo[0],sizeof(strInfo)*NUM_STR);
	for(sid=0;sid<NUM_STR;sid++){
		strInfo[sid].Inter_Role=ARTRIBITION;	//always set to slave node when start
		strInfo[sid].intAddr=0xA8;
		strInfo[sid].fullCapacity=15480000;
		strInfo[sid].capacityGain=0;
		strInfo[sid].capacityGainTotal=0;
		strInfo[sid].capacityLose=0;
		strInfo[sid].capacityLoseTotal=0;
		
		if(strInfo[sid].mod_sn>0xFFFFFF) strInfo[sid].mod_sn=DEFAULT_MOD_SN;
		if(strInfo[sid].mod_soc>1000) strInfo[sid].mod_soc=1000;
		if(strInfo[sid].mod_soh>1000) strInfo[sid].mod_soc=1000;
		/*get 32bit random number */		 
	//	strInfo.rand_num=rand();
		if(strInfo[sid].mod_life_ahr_c>0xFFFFFFFFFFFFFFF) strInfo[sid].mod_life_ahr_c=0;
		if(strInfo[sid].mod_life_ahr_d>0xFFFFFFFFFFFFFFF) strInfo[sid].mod_life_ahr_d=0;
		
		if(strInfo[sid].mod_life_whr_c>0xFFFFFFFFFFFFFFF) strInfo[sid].mod_life_whr_c=0;
		if(strInfo[sid].mod_life_whr_d>0xFFFFFFFFFFFFFFF) strInfo[sid].mod_life_whr_d=0;
		strInfo[sid].rand_num=RNG_GetRandomNumber();
		strInfo[sid].ptBMU=(BMU_INFO *)&bmu[0];
		strInfo[sid].bmu_alive_num=0;
		strInfo[sid].bmu_on_num=0;
		strInfo[sid].bmu_ready_num=0;
		bpInfo.str=&strInfo[0];
		//memcpy((void *)&bpInfo.str[0],(void *)&strInfo,sizeof(strInfo));
		SPI_FLASH_CS_HIGH();
		strInfo[sid].SOC_OCV_adjusted=0;
		strInfo[sid].f_bmu_lost=1;
		strInfo[sid].bmuOnCmd=0;
		strInfo[sid].mod_fw_ver_major=BP_FW_VER_MAJOR;
		strInfo[sid].mod_fw_ver_minor=BP_FW_VER_MINOR;
		strInfo[sid].mod_fw_ver_patch=BP_FW_VER_PATCH;
	}
}
void BPInit(void){
	UNS16 bpsize;
	bpsize=sizeof(bpInfo);
		SPI_FLASH_CS_HIGH();

	df_read_open(ADDR_BP_IN_NVM);
	df_read((UNS8 *)&bpInfo,sizeof(bpInfo));
	bpInfo.w_idle=0;
	bpInfo.num_str=1;
	SPI_FLASH_CS_HIGH();
	bpInfo.sc_fw_ver_major=BP_FW_VER_MAJOR;
	bpInfo.sc_fw_ver_minor=BP_FW_VER_MINOR;
	bpInfo.sc_fw_ver_patch=BP_FW_VER_PATCH;

}
void SysVarInit(void){
	UNS16 bsyssize;
	bsyssize=sizeof(sysInfo);
	SPI_FLASH_CS_HIGH();
	df_read_open(ADDR_SYS_IN_NVM);
	df_read((UNS8 *)&sysInfo,sizeof(sysInfo));
	SPI_FLASH_CS_HIGH();
	sysInfo.f_alarm=0;
	sysInfo.f_vcu_lost=0;
	sysInfo.f_fault=0;
	sysInfo.sysResetCnt+=1;

}
void sysConfigInit(void){
	
	uint16_t const_size;
	const_size=sizeof(gSysCfg);
	df_read_open(ADDR_CONFIG_IN_NVM);
	SPI_FLASH_CS_HIGH();
	df_read((UNS8 *)&gSysCfg,sizeof(gSysCfg));
	gSysCfg.ucNUM_BMU_TOTAL=NUM_BMU_TOTAL;
	gSysCfg.ucNUM_STR=NUM_STR;
	gSysCfg.ucNUM_BUM_PER_STR=NUM_BUM_PER_STR;
	gSysCfg.ucNUM_CV_IN_BMU=NUM_CV_IN_BMU;
	gSysCfg.ucNUM_CT_IN_BMU=NUM_CT_IN_BMU;
	gSysCfg.uiCAP_CELL_FULL=CAP_CELL_FULL;	//AHr
	gSysCfg.uiCAL_CS_OFFSET=CAL_CS_OFFSET;
	gSysCfg.uiCAL_CS_AMP=CAL_CS_AMP;
	
}



