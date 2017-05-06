/***********************************************************************
name: main.c
function: entry point
***********************************************************************/
#include "main.h"
#include "Task_EFlash.h"
#include "Task_AVChg.h"
#include "Task_Main.h"
#include "Task_sysCheck.h"
#include "Task_wifi.h"
#include "mesa.h"
SYS_CONFIG_CONST_ST gSysCfg;

extern int mbtcp_task( void *pdata );
BMU_INFO bmu[NUM_BMU_TOTAL];

uint32_t main_loop_count=0;
int main(void)
{
	//board initlize
	BSP_Init();
	//battery System parameter initlize
	SCInit();

	//OS initlize
	OSInit();
	//create first task
	OSTaskCreate(	Task_StartUp,               		    				//pointer to routine
                  	(void *) 0,												//pointer of argument pass to the task
					(OS_STK *)&Stk_TaskStartUp[TASK_STARTUP_STK_SIZE - 1],	//top address of stack to the task
					(INT8U) OS_USER_PRIO_LOWEST);							//lowest priority 59 

	//clear timer
	OSTimeSet(0);

	//start OS 
	OSStart();
 LED1_ON;
	return 0;
}
//allocte start task stack
OS_STK Stk_TaskStartUp[TASK_STARTUP_STK_SIZE];


void Task_StartUp(void *pdata)
{

	//init os timer
	//OS_TICKS_PER_SEC  is UCOS-II tick per second
	SysTick_Config(SystemCoreClock/OS_TICKS_PER_SEC - 1);	 
 
	OSTaskCreate(Task_CAN2_Rev, (void *)0, &Stk_Task_CAN2_Rev[TASK_CAN2_REV_STK_SIZE-1], OS_USER_PRIO_GET(6));
	OSTaskCreate(Task_CO, (void *)0, &Stk_Task_CO[TASK_TEST_CO_STK_SIZE-1], OS_USER_PRIO_GET(9));
	OSTaskCreate(Task_CAN1, (void *)0, &Stk_Task_CAN1[TASK_TEST_CAN1_STK_SIZE-1], OS_USER_PRIO_GET(7));
 	OSTaskCreate(Task_CANSend, (void *)0, &Stk_Task_CAN_Send[TASK_CAN2_REV_STK_SIZE-1], OS_USER_PRIO_GET(23));
	//OSTaskCreate(Task_CAN2MsgBuf, (void *)0, &Stk_Task_CAN2_Snd2Buf[TASK_CAN2_SEND_TO_BUF_STK_SIZE-1], OS_USER_PRIO_GET(22));
	OSTaskCreate(Task_CAN2MsgBuf_GT, (void *)0, &Stk_Task_CAN2_Snd2Buf[TASK_CAN2_SEND_TO_BUF_STK_SIZE-1], OS_USER_PRIO_GET(22));
	OSTaskCreate(Task_AVChg, (void *)0, &Stk_Task_AVChg[TASK_CAN2_REV_STK_SIZE-1], OS_USER_PRIO_GET(20));
//	OSTaskCreate(Task_EFlash, (void *)0, &Stk_Task_EFlash[400-1], OS_USER_PRIO_GET(19));
//OSTaskCreate(Task_HTTP, (void *)0, &Stk_Task_HTTP[2*TASK_TEST_HTTP_STK_SIZE-1], OS_USER_PRIO_GET(30));
	
	OSTaskCreate(Task_Main, (void *)0, &Stk_Task_Main[TASK_MAIN_STK_SIZE-1], OS_USER_PRIO_GET(5));
	OSTaskCreate(Task_FaultCheck, (void *)0, &Stk_TaskFaultCheck[TASK_FAULT_CHECK_STK_SIZE-1], OS_USER_PRIO_GET(4));
	OSTaskCreate(Task_wifi, (void *)0, &Stk_Task_WIFI[TASK_WIFI_STK_SIZE-1], OS_USER_PRIO_GET(30));
	//	OSTaskCreate(Task_LED, (void *)0, &Stk_Task_LED[TASK_TEST_LED_STK_SIZE-1], OS_USER_PRIO_GET(11));
	OSTaskCreate(Task_Mesa, (void *)0, &Stk_Task_Mesa[TASK_MESA_STK_SIZE-1], OS_USER_PRIO_GET(31));
	while (1)
	{
		//guard routine
		//normally flash a LED to show system is runing
		updateBP_BM();
		LED1_Toggle;
		main_loop_count++;
		if(main_loop_count==3){
			OSTaskCreate(mbtcp_task, (void *)0, &Stk_Task_HTTP[2*TASK_TEST_HTTP_STK_SIZE-1], OS_USER_PRIO_GET(29));
			//OSTaskCreate(Task_HTTP, (void *)0, &Stk_Task_HTTP[2*TASK_TEST_HTTP_STK_SIZE-1], OS_USER_PRIO_GET(29));
		}
		
		strPlimSet(&strInfo);
		OSTimeDly(1000);//1000ms
	}
}
