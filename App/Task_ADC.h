/***********************************************************************
***********************************************************************/
#ifndef _APP_TASK_ADC_H_
#define _APP_TASK_ADC_H_


extern OS_EVENT  *sem_CAN1_rec_flag;			//CAN1接收完一桢数据信号量定义
typedef struct{
	UNS16 V_DC_Bus;	// mv
	UNS16 V_DC_Batt;	// mv
	UNS16 V_HV_in; //mv
	UNS16 V_5VOut; //mv
	UNS16 V_12VOut; //mv
	UNS16 I_curr; //mA
	UNS16 T_1; //0.1C
	UNS16 T_2; //0.1C
	UNS16 T_3; //0.1C
}AD_RESULT;
extern AD_RESULT ad_res;

#endif

