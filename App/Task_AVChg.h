#ifndef _APP_TASK_AVC_H_
#define _APP_TASK_AVC_H_

#define TASK_TEST_AVC_STK_SIZE	400

extern OS_STK Stk_Task_AVChg[TASK_TEST_AVC_STK_SIZE];

extern OS_EVENT  *sem_CAN1_rec_flag;			//CAN1������һ�������ź�������

void Task_AVChg(void *pdata);
#endif

