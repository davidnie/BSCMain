/***********************************************************************
***********************************************************************/
#include "main.h"
#include "def.h"
#include "bsp_can.h"
#include "Task_ADC.h"
#include "task_main.h"
extern	AD_RESULT ad_res;
	
OS_STK Stk_Task_CAN1[TASK_TEST_CAN1_STK_SIZE];
	OS_STK Stk_Task_CO[TASK_TEST_CO_STK_SIZE];
	
	OS_EVENT  *sem_CAN1_rec_flag;			//CAN1接收完一桢数据信号量定义
/***********************************************************************
***********************************************************************/
	void Task_CAN1(void *pdata)
	{		 
		unsigned  char	os_err,i;
		Message RxMSG;
		sem_CAN1_rec_flag = OSSemCreate(1); //create a semphere. 

		while(1)
		{  
			OSSemPend(sem_CAN1_rec_flag,0,&os_err); 		//wait sem_CAN1_rec_flag being set in ISR

		}
	}
	void Task_CO(void *pdata){
		unsigned  char	os_err;
		Message RxMSG;
		static uint32_t t_co;
		static uint8_t CO_tid=0;
		static uint32_t t_stat;
		uint32_t tnow;
		static uint32_t loop_cnt=0;
		uint8_t sndData[8];
		uint8_t *pData,*pt;
		uint8_t i,j,k;
		static uint8_t pollBmuNum=1;
		static uint8_t svrNodeId=0;
		UNS16 sdo_id;
		UNS8 bt[8];
		UNS16 wd[4];
		sem_CAN1_rec_flag = OSSemCreate(1); //create a semphere. 
		/* init framework and canopen */
		ebFrameworkInit(250, 8200, NULL); /* bitrate, timer *///TMB orig
		/* init application */	
		batApplInit();
		while(1)
		{  
		OSTimeDlyHMSM(0, 0, 0, 10);// 10ms
		//OSSemPend(sem_CAN1_rec_flag,0,&os_err); 		//wait sem_CAN1_rec_flag being set in ISR
			coCommTask();
		  codrvTimerISR();
		}
}

