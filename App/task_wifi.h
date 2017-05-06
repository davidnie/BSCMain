/***********************************************************************
***********************************************************************/
#ifndef _APP_TASK_WIFI_H_
#define _APP_TASK_WIFI_H_

#define TASK_WIFI_STK_SIZE	400

extern OS_EVENT  *sem_UDP_rec_flag;			//CAN1接收完一桢数据信号量定义

extern OS_STK Stk_Task_WIFI[TASK_WIFI_STK_SIZE];

extern OS_EVENT  *sem_CAN1_rec_flag;			//CAN1接收完一桢数据信号量定义

void Task_wifi(void *pdata);

#endif


