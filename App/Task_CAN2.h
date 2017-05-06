/***********************************************************************
***********************************************************************/
#ifndef _APP_TASK_CAN2_H_
#define _APP_TASK_CAN2_H_
#include "BSP_CAN.h"

#define TASK_CAN2_REV_STK_SIZE	400
#define TASK_CAN2_SEND_TO_BUF_STK_SIZE	400
#define TASK_CAN_SEND_STK_SIZE	400
extern OS_STK Stk_Task_CAN2_Rev[TASK_CAN2_REV_STK_SIZE];
extern OS_STK Stk_Task_CAN2_Snd2Buf[TASK_CAN2_SEND_TO_BUF_STK_SIZE];
extern OS_STK Stk_Task_CAN_Send[TASK_CAN_SEND_STK_SIZE];


extern OS_EVENT  *sem_CAN2_rec_flag;			//CAN2接收完一桢数据信号量定义
extern OS_EVENT  *sem_CAN2_send_flag;			//CAN2 send semphere

void Task_CAN2_Rev(void *pdata);
void Task_CANSend(void *pdata);
void Task_CAN2MsgBuf(void *pdata);
void Task_CAN2MsgBuf_GT(void *pdata);
void j1939_send(CAN_TypeDef *CANx,J1939_MESSAGE *jmsg);


#endif

