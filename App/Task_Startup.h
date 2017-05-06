/***********************************************************************
***********************************************************************/
#ifndef _APP_TASK_STARTUP_H_
#define _APP_TASK_STARTUP_H_


//assign stack for task
#define TASK_STARTUP_STK_SIZE  1000


extern OS_STK Stk_TaskStartUp[TASK_STARTUP_STK_SIZE];

void Task_StartUp(void *pdata);


#endif
