/***********************************************************************
***********************************************************************/
#ifndef _BATTERY_GT_H_
#define _BATTERY_GT_H_
void Task_CAN2MsgBuf_GT(void *pdata);

typedef enum{
	GT_READ_VOL,
	GT_READ_TEMP,
	GT_READ_BAL_STAT,
	GT_READ_SOC,
	GT_READ_CURR
}GT_OP_CMD;
typedef enum{
	FK_INIT,
	FK_READY,
	FK_CHG_PRE,
	FK_DCHG_PRE,
	FK_CHG,
	FK_RUN,
	FK_WARMUP,
	FK_FAULT,
}FK_STAGE;
extern FK_STAGE gFkStat;
#endif

