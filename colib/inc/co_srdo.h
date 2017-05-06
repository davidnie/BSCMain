/*
* co_srdo.h - contains defines for srdo services
*
* Copyright (c) 2015-2016 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_srdo.h 14545 2016-07-15 08:50:09Z boe $
*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for srdo services
*
* \file co_srdo.h - contains defines for srdo services
*/

#ifndef CO_SRDO_H
#define CO_SRDO_H 1

#include <co_datatype.h>


#define CO_SRDO_TYPE_NONE		0u
#define CO_SRDO_TYPE_PRODUCER	1u
#define CO_SRDO_TYPE_CONSUMER	2u


#define CO_SRDO_VALID_SIGNATURE		0xa5u

/* datatypes */

typedef enum {
	CO_SRDO_STATE_AWAIT_FIRST_MSG1,
	CO_SRDO_STATE_AWAIT_MSG1,
	CO_SRDO_STATE_AWAIT_MSG2,
	CO_SRDO_STATE_ERROR
} CO_SRDO_STATE_T;


typedef enum {
	CO_SRDO_TIMER_TYPE_SRVT,
	CO_SRDO_TIMER_TYPE_SCT
} CO_SRDO_TIMER_TYPE_T;

typedef struct srdo_timer {
	struct srdo_timer *pNext;	/**< pointer to next timer */
	UNSIGNED32		actTicks;	/**< actual timer ticks */
	CO_SRDO_TIMER_TYPE_T	type;
	UNSIGNED16	srdoNr;
} sTimer;
typedef struct srdo_timer CO_SRDO_TIMER_T;



typedef struct {
	UNSIGNED8	direction;		/* information direction */
	UNSIGNED16	refreshTime;	/* refresh time */
	UNSIGNED8	srvt;			/* SRVT */
	UNSIGNED8	transType;		/* transmission type */
	UNSIGNED32	cobId1;			/* cob id 1 */
	UNSIGNED32	cobId2;			/* cob id 2 */

	UNSIGNED8	mapCnt;			/* number of actual mappings */
	UNSIGNED8	maxMapCnt;		/* number of maximum mappings */
	UNSIGNED16  signature;		/* signature for the srdo */
	const UNSIGNED32	*mapTable;		/* pointer to mapping table */

	
	CO_SRDO_STATE_T	state;			/* internal srdo state */
	CO_SRDO_TIMER_T	timerSct;		/* SCT timer */
	CO_SRDO_TIMER_T	timerSrvt;		/* SRVT timer */
} CO_SRDO_DATA_T;


/*
 * defines for error reasons 
 */
typedef enum {
	CO_SRDO_ERROR_WRONG_MSG = 0,			/* wrong message received */
	CO_SRDO_ERROR_DATA_MISMATCH,		/* data doesnt match */
	CO_SRDO_ERROR_TIMEOUT_SCT,			/* timeout SCT */
	CO_SRDO_ERROR_TIMEOUT_SRVT,			/* timeout SRVT */
	CO_SRDO_ERROR_WRONG_LEN
} CO_SRDO_ERROR_T;




EXTERN_DECL RET_T genSrdoDataGet_u8(UNSIGNED16 index, UNSIGNED8	subIndex,
				UNSIGNED8	*pData);
EXTERN_DECL RET_T genSrdoDataGet_u16(UNSIGNED16 index, UNSIGNED8 subIndex,
				UNSIGNED16 *pData);
EXTERN_DECL RET_T genSrdoDataGet_u32(UNSIGNED16 index, UNSIGNED8 subIndex,
				UNSIGNED32 *pData);
EXTERN_DECL RET_T genSrdoDataPut_u8(UNSIGNED16 index, UNSIGNED8 subIndex,
				UNSIGNED8 data);
EXTERN_DECL RET_T genSrdoDataPut_u16(UNSIGNED16 index, UNSIGNED8 subIndex,
				UNSIGNED16 data);
EXTERN_DECL RET_T genSrdoDataPut_u32(UNSIGNED16 index, UNSIGNED8 subIndex,
				UNSIGNED32 data);
EXTERN_DECL void genSrdoSetDefaultValue(UNSIGNED8 ownNodeId);

EXTERN_DECL RET_T genSrdoCheckOperational(void);
EXTERN_DECL void genSrdoGoOperational(void);
EXTERN_DECL void genSrdoGoPreOperational(void);

EXTERN_DECL RET_T genSrdoTransmitMapping(UNSIGNED16 srdoNr,
		UNSIGNED8 *pBufferNormal, UNSIGNED8 *pBufferInverted);

EXTERN_DECL RET_T coSrdoInit(UNSIGNED16 srdoNr);
EXTERN_DECL void genSrdoReceiveHandler(UNSIGNED16 srdoNr,
		CO_CONST CO_CAN_MSG_T *pRecMsg);

EXTERN_DECL void genSrdoTimerInit(UNSIGNED32 timerVal);
EXTERN_DECL void genSrdoTimerIsr(void);
EXTERN_DECL void genSrdoTimerCheck(void);

void	srdoErrorIndication(UNSIGNED16 srdoNr, CO_SRDO_ERROR_T reason);
void	srdoConsumerIndication(UNSIGNED16 srdoNr);
void	srdoProducerIndication(UNSIGNED16 srdoNr);

#endif /* CO_SRDO_H */
