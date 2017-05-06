/***********************************************************************
***********************************************************************/
#ifndef _APP_SYS_CHECK_H_
#define _APP_SYS_CHECK_H_


//assign stack for task
#define TASK_FAULT_CHECK_STK_SIZE  100


extern OS_STK Stk_TaskFaultCheck[TASK_FAULT_CHECK_STK_SIZE];
extern OS_EVENT *sem_vcu_can_rev;
extern OS_EVENT *sem_bmu_can_rev;
extern OS_EVENT *sem_fault_clr;

extern void Task_FaultCheck(void *pdata);


#endif

