/***********************************************************************
***********************************************************************/
#ifndef _APP_TASK_CAN1_H_
#define _APP_TASK_CAN1_H_

#define TASK_TEST_CAN1_STK_SIZE	2000
#define TASK_TEST_CO_STK_SIZE	2000

extern OS_STK Stk_Task_CAN1[TASK_TEST_CAN1_STK_SIZE];
extern OS_STK Stk_Task_CO[TASK_TEST_CO_STK_SIZE];

extern OS_EVENT  *sem_CAN1_rec_flag;			//CAN1接收完一桢数据信号量定义

void Task_CAN1(void *pdata);
void Task_CO(void *pdata);
#endif
