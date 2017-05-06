/***********************************************************************
***********************************************************************/
#ifndef _APP_TASK_EFLASH_H_
#define _APP_TASK_EFLASH_H_

#define TASK_EFLASH_STK_SIZE	2000
#define SECTOR_MOD_INFO_START   2
#define SECTOR_PACK_INFO_START   4
#define SECTOR_LOG_START   10

extern OS_STK Stk_Task_EFlash[TASK_EFLASH_STK_SIZE];


extern void Task_EFlash(void *pdata);
extern void NVM_BMWrite(void);
extern void NVM_SysinfoWrite(void);
extern void NVM_BPInfoWrite(void);
extern void NVM_AllInfoWrite(void);
extern void NVM_sysCfgWrite(void);

#endif

