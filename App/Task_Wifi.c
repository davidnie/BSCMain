/***********************************************************************
***********************************************************************/
#include "main.h"  
#include "task_wifi.h"

OS_STK Stk_Task_WIFI[TASK_WIFI_STK_SIZE];


void Task_wifi(void *pdata );

/***********************************************************************
***********************************************************************/
void Task_wifi(void *pdata){
	static unsigned int lp_cnt;
	OSTimeDly(5000);//wait 5 sec before wifi module enable to avoid infulence system start speed
	wifiInit();
	while(1){
		OSTimeDly(100);
		lp_cnt++;
	}
}
