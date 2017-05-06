/***********************************************************************
�ļ����ƣ�
��    �ܣ�
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
#ifndef _APP_TASK_TEST_H_
#define _APP_TASK_TEST_H_

#define TASK_TEST_LED_STK_SIZE	400
#define TASK_TEST_HTTP_STK_SIZE	1000

extern OS_STK Stk_Task_LED[TASK_TEST_LED_STK_SIZE];
extern OS_STK Stk_Task_HTTP[TASK_TEST_HTTP_STK_SIZE];

void Task_LED(void *pdata);
void Task_HTTP(void *pdata);

#endif
