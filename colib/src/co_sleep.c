/*
* co_sleep.c - contains sleep services
*
* Copyright (c) 2013-2015 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_sleep.c 10359 2015-08-24 14:41:21Z boe $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*
*/

/********************************************************************/
/**
* \brief Sleep and Wakeup Handling
*
* \file co_sleep.c
* contains routines for sleep/wakeup handling
*
*/


/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stddef.h>
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#include <co_datatype.h>
#include <co_timer.h>
#include <co_drv.h>
#include <co_commtask.h>
#include <co_sleep.h>
#include <co_nmt.h>
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_nmt.h"
#include "ico_sleep.h"
#include "ico_indication.h"

/* constant definitions
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_DYNAMIC_SLEEP
# ifdef CO_EVENT_PROFILE_SLEEP
#  define CO_EVENT_SLEEP_CNT	(CO_EVENT_DYNAMIC_SLEEP + CO_EVENT_PROFILE_SLEEP)
# else /* CO_EVENT_PROFILE_SLEEP */
#  define CO_EVENT_SLEEP_CNT	(CO_EVENT_DYNAMIC_SLEEP)
# endif /* CO_EVENT_PROFILE_SLEEP */
#else /* CO_EVENT_DYNAMIC_SLEEP */
# ifdef CO_EVENT_PROFILE_SLEEP
#  define CO_EVENT_SLEEP_CNT	(CO_EVENT_PROFILE_SLEEP)
# endif /* CO_EVENT_PROFILE_SLEEP */
#endif /* CO_EVENT_DYNAMIC_SLEEP */


#ifdef CO_EVENT_SLEEP

#define CO_SLEEP_MASTER_CAN_ID		0x691u
#define CO_SLEEP_SLAVE_CAN_ID		0x690u
#define CO_SLEEP_REQ_SLEEP_CAN_ID	0x692u

#define CO_SLEEP_CMD_QUERY			0x01u
#define CO_SLEEP_CMD_SLEEP			0x02u
#define CO_SLEEP_STATE_REQ			0x03u
#define CO_SLEEP_STATE_OBJECTION	0x81u

#ifdef CO_SLEEP_447
# ifdef CO_NMT_MASTER
#  define CO_SLEEP_447_CNT	14
# else /* CO_NMT_MASTER */
#  define CO_SLEEP_447_CNT	1
# endif /* CO_NMT_MASTER */

# define CO_SLEEP_447_LAST_NODE	(CO_SLEEP_447_CNT + 2)
#endif /* CO_SLEEP_447 */



/* local defined data types
---------------------------------------------------------------------------*/
typedef struct {
		UNSIGNED8	status;		/* wake up state */
		UNSIGNED8	reason;		/* wake up reason */
		COB_REFERENZ_T	sleepCob;
} WAKEUP_T;

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_STATIC_SLEEP
extern CO_CONST CO_EVENT_SLEEP_T coEventSleepInd;
#endif /* CO_EVENT_STATIC_SLEEP */


/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static UNSIGNED8 callSleepInd(CO_SLEEP_MODE_T sleepMode, UNSIGNED8 node);
static void sleepSilent(void *ptr);
static void sleepDoze(void *ptr);
static void sleepWakeUpMessage(void *ptr);
#ifdef CO_NMT_MASTER
static void sleepNoObjection(void *ptr);
#endif /* CO_NMT_MASTER */

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_SLEEP_CNT
static CO_EVENT_SLEEP_T	sleepTable[CO_EVENT_SLEEP];
static UNSIGNED8		sleepTableCnt = 0u;
#endif /* CO_EVENT_SLEEP_CNT */
static CO_TIMER_T		sleepTimer;
static CO_TIMER_T		awakeTimer;
static COB_REFERENZ_T	sleepMasterCob;
#ifdef CO_SLEEP_454
static COB_REFERENZ_T	sleepSlaveCob;
static COB_REFERENZ_T	sleepRequestCob;
#endif /* CO_SLEEP_454 */
#ifdef CO_SLEEP_447
static COB_REFERENZ_T	sleepSlaveCob[CO_SLEEP_447_CNT];
#endif /* CO_SLEEP_447 */
static BOOL_T 			sleepActive = { CO_FALSE };



/***************************************************************************/
/**
* \internal
*
* \brief coNmtMsgHandler - handle received NMT mesages
*
*
* \return none
*
*/
void icoSleepMsgHandler(
		CO_CONST CO_REC_DATA_T	*pRec		/* pointer to receive data */
	)
{
UNSIGNED8	error;
CO_TRANS_DATA_T trData;

	if (pRec->msg.len != 8u)  {
		return;
	}

	/* message from master */
	if (pRec->msg.canCob.canId == CO_SLEEP_MASTER_CAN_ID)  {
		/* query sleep objection ? */
		if (pRec->msg.data[0] == CO_SLEEP_CMD_QUERY)  {
			/* check, if sleep is possible */
			error = callSleepInd(CO_SLEEP_MODE_CHECK, 0u);
			if (error != 0u)  {
				/* answer error */
				memset(&trData.data[0], 0, 8u);
				trData.data[0] = CO_SLEEP_STATE_OBJECTION;
				trData.data[1] = error;
#ifdef CO_SLEEP_454
				(void)icoTransmitMessage(sleepSlaveCob, &trData, 0u);
#endif /* CO_SLEEP_454 */
#ifdef CO_SLEEP_447
				(void)icoTransmitMessage(sleepSlaveCob[pRec->spec], &trData, 0u);
#endif /* CO_SLEEP_447 */
			}
		}
		/* sleep mode ? */
		if (pRec->msg.data[0] == CO_SLEEP_CMD_SLEEP)  {
			/* goto sleep start mode */
			coSleepModeStart(CO_SLEEP_WAITTIME);
		}
	}

	/* objection from slave */
#ifdef CO_SLEEP_454
	if (pRec->msg.canCob.canId == CO_SLEEP_SLAVE_CAN_ID)  {
#endif /* CO_SLEEP_454 */
#ifdef CO_SLEEP_447
	if ((pRec->msg.canCob.canId >= CO_SLEEP_SLAVE_CAN_ID)
	 && (pRec->msg.canCob.canId < (CO_SLEEP_SLAVE_CAN_ID + CO_SLEEP_447_CNT))) {
#endif /* CO_SLEEP_447 */

		if (pRec->msg.data[0] == CO_SLEEP_STATE_OBJECTION)  {
			(void)coTimerStop(&sleepTimer);
			(void)callSleepInd(CO_SLEEP_MODE_OBJECTION, (UNSIGNED8)(pRec->spec + 2u));
		}
	}

#ifdef CO_SLEEP_454
	if (pRec->msg.canCob.canId == CO_SLEEP_REQ_SLEEP_CAN_ID)  {
#endif /* CO_SLEEP_454 */
#ifdef CO_SLEEP_447
	if ((pRec->msg.canCob.canId >= CO_SLEEP_REQ_SLEEP_CAN_ID)
	 && (pRec->msg.canCob.canId < (CO_SLEEP_REQ_SLEEP_CAN_ID + CO_SLEEP_447_CNT))) {
#endif /* CO_SLEEP_447 */

		(void)callSleepInd(CO_SLEEP_MODE_REQUEST_SLEEP, (UNSIGNED8)(pRec->spec + 2u));
	}
}


#ifdef CO_NMT_MASTER
/***************************************************************************/
/**
*
* \brief coSleepModeCheck - check nodes for sleep mode
*
* This function sends the sleep objection command
* If an answer is received, the registered indication function is called.
* If no answer was received, the function starts automatically 
* coSleepModeStart() - to start sleep mode.
*
*
* \return none
*
*/
void coSleepModeCheck(
		UNSIGNED16		waitTime	/**< time timeout value in ms */
	)
{
CO_TRANS_DATA_T trData;

	/* send objection command */
	memset(&trData.data[0], 0, 8u);
	trData.data[0] = CO_SLEEP_CMD_QUERY;
	(void)icoTransmitMessage(sleepMasterCob, &trData, 0);

	/* start timeout timer */
	coTimerStart(&sleepTimer, waitTime * 1000ul,
		sleepNoObjection, NULL, CO_TIMER_ATTR_ROUNDUP);	/*lint !e960 function identifier used without '&' or parenthesized parameter list */
}


/***************************************************************************/
/**
* \internal
*
* \brief sleepNoObjection - timeout and no objection received
*
* \return none
*
*/
static void sleepNoObjection(
		void			*ptr
	)
{
UNSIGNED8	err;
(void)ptr;

	err = callSleepInd(CO_SLEEP_MODE_PREPARE, 0);
	if (err == 0)  {
		coSleepModeStart(1000u);
	}
}
#endif /* CO_NMT_MASTER */


/***************************************************************************/
/**
*
* \brief coSleepModeStart - start sleep mode
*
* This function starts the sleep mode.
* First a timer is started, then the CAN traffic is stopped
* and the CPU is going to sleep.
*
* Each step is indicated by the function
* registered by coEventRegister_SLEEP().
*
*
* \return none
*
*/
void coSleepModeStart(
		UNSIGNED16		waitTime		/**< wait time before stop CAN in ms */
	)
{
#ifdef CO_NMT_MASTER
CO_TRANS_DATA_T trData;

	/* if master, send sleep command */
	memset(&trData.data[0], 0, 8u);
	trData.data[0] = CO_SLEEP_CMD_SLEEP;
	(void)icoTransmitMessage(sleepMasterCob, &trData, 0);
#endif /* CO_NMT_MASTER */

	/* if waitTime == 0, set ist to 1 */
	if (waitTime == 0u)  {
		waitTime++;
	}

	(void)coTimerStart(&sleepTimer, waitTime * 1000ul,
		sleepSilent, NULL, CO_TIMER_ATTR_ROUNDUP);		/*lint !e960 function identifier used without '&' or parenthesized parameter list */
	sleepActive = CO_TRUE;
}


/***************************************************************************/
/**
* \internal
*
* \brief sleepSilent
*
*
* \return none
*
*/
static void sleepSilent(
		void			*ptr
	)
{
UNSIGNED8	error;
(void) ptr;

	error = callSleepInd(CO_SLEEP_MODE_SILENT, 0u);
	if (error == 0u)  {
		/* ok, disable CAN traffic */
		icoQueueDisable(CO_TRUE);



		/* start timer before sleep will be active */
		(void)coTimerStart(&sleepTimer, 1000000ul,
			sleepDoze, NULL, CO_TIMER_ATTR_ROUNDUP);	/*lint !e960 function identifier used without '&' or parenthesized parameter list */
	} else {
		sleepActive = CO_FALSE;
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief sleepDoze
*
*
* \return none
*
*/
static void sleepDoze(
		void			*ptr
	)
{
UNSIGNED8	error;
(void) ptr;

	error = callSleepInd(CO_SLEEP_MODE_DOZE, 0u);
	if (error != 0u)  {
		icoQueueDisable(CO_FALSE);
		sleepActive = CO_FALSE;
	}
}


/***************************************************************************/
/**
*
* \brief coSleepAwake - awake from sleep
*
* This function have to called after the processor is awaked.
* It transmit the wake up message, repeat it after the given repeatTime
* and reinitializes the communication handling.
*
* \return none
*
*/
void coSleepAwake(
		BOOL_T			master,		/**< wake up master */
		UNSIGNED8		status,		/**< wake up state (only for slaves) */
		UNSIGNED8		reason,		/**< wake up reason */
		UNSIGNED16		repeatTime	/**< time interval for repeat wake up message*/
	)
{
	/* if already awaked, return */
	if (sleepActive == CO_FALSE)  {
		return;
	}

	coSleepWakeUp(master, status, reason, repeatTime);
}


/***************************************************************************/
/**
*
* \brief coSleepWakeUp - awake from sleep
*
* This function can be called to send the wake up message
* independ form the actual sleep state.
* It transmit the wake up message, repeat it after the given repeatTime
* and reinitializes the communication handling.
*
* \return none
*
*/
void coSleepWakeUp(
		BOOL_T			master,		/**< wake up master */
		UNSIGNED8		status,		/**< wake up state (only for slaves) */
		UNSIGNED8		reason,		/**< wake up reason */
		UNSIGNED16		repeatTime	/**< time interval for repeat wake up message*/
	)
{
static WAKEUP_T	wakeUpData;
#ifdef CO_LSS_SUPPORTED
UNSIGNED8	nodeId = CO_NODE_ID;
#endif /* CO_LSS_SUPPORTED */

	coCommTask();
	icoQueueDisable(CO_FALSE);
	sleepActive = CO_FALSE;

	/* transmit wakeup */
	wakeUpData.reason = reason;
	if (master == CO_TRUE)  {
		wakeUpData.status = 0x82u;
		wakeUpData.sleepCob = sleepMasterCob;
	} else {
		wakeUpData.status = status;
#ifdef CO_SLEEP_454
		wakeUpData.sleepCob = sleepSlaveCob;
#endif /* CO_SLEEP_454 */
#ifdef CO_SLEEP_447
		wakeUpData.sleepCob = sleepSlaveCob[0];
#endif /* CO_SLEEP_447 */
	}
	sleepWakeUpMessage(&wakeUpData);

	/* start timer for repeat wake up message */
	(void)coTimerStart(&awakeTimer, repeatTime * 1000ul,
		sleepWakeUpMessage, &wakeUpData, CO_TIMER_ATTR_ROUNDUP);	/*lint !e960 function identifier used without '&' or parenthesized parameter list */

#ifdef CO_LSS_SUPPORTED
	/* fixes node-id ? */
	if (nodeId == 255u)  {  /*lint !e774 depends on LSS configuration */
		icoNmtResetNodeId();
	} else
#endif /* CO_LSS_SUPPORTED */
	{
		(void)coNmtLocalStateReq(CO_NMT_STATE_RESET_COMM);
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief sleepWakeUpMessage - transmit sleep wake message
*
* \return none
*
*/
static void sleepWakeUpMessage(
		void			*ptr
	)
{
CO_TRANS_DATA_T trData;
WAKEUP_T	*pWakeUp = (WAKEUP_T *)ptr;

	memset(&trData.data[0], 0, 8u);
	trData.data[0] = pWakeUp->status;
	trData.data[1] = pWakeUp->reason;
	(void)icoTransmitMessage(pWakeUp->sleepCob, &trData, 0u);
}


/***************************************************************************/
/**
*
* \brief coSleepModeActive - check if sleep mode is active
*
*
* \return none
*
*/
BOOL_T coSleepModeActive(
		void	/* no parameter */
	)
{
	return(sleepActive);
}


#ifdef CO_NMT_MASTER
#else /* CO_NMT_MASTER */
/***************************************************************************/
/**
*
* \brief coSleepRequestSleep - request sleep mode to master
*
* Request sleep mode from master by sending sleep request.
*
* \return none
*
*/
void coSleepRequestSleep(
		void
	)
{
CO_TRANS_DATA_T trData;

	/* send objection command */
	memset(&trData.data[0], 0, 8u);
	trData.data[0] = CO_SLEEP_STATE_REQ;
# ifdef CO_SLEEP_454
	(void)icoTransmitMessage(sleepRequestCob, &trData, 0u);
# endif /* CO_SLEEP_454 */
# ifdef CO_SLEEP_447
	(void)icoTransmitMessage(sleepSlaveCob[0], &trData, 0u);
# endif /* CO_SLEEP_447 */
}
#endif /* CO_NMT_MASTER */


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
#ifdef CO_EVENT_SLEEP_CNT
/***************************************************************************/
/**
* \brief coEventRegister_SLEEP - register SLEEP event
*
* register indication function for SLEEP events
*
* \return RET_T
*
*/

RET_T coEventRegister_SLEEP(
		CO_EVENT_SLEEP_T pFunction	/**< pointer to function */
    )
{
	/* set new indication function as first at the list */
	if (sleepTableCnt >= CO_EVENT_SLEEP_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	sleepTable[sleepTableCnt] = pFunction;	/* save function pointer */
	sleepTableCnt++;

	return(RET_OK);
}
#endif /* CO_EVENT_SLEEP_CNT */


/***************************************************************************/
/*
* \internal
*
* \brief callSleepInd - call user specific indications
*
*
* \return RET_T
*
*/
static UNSIGNED8 callSleepInd(
		CO_SLEEP_MODE_T	sleepMode,	
		UNSIGNED8		node
	)
{
UNSIGNED8	retVal = 0u;
#ifdef CO_EVENT_SLEEP_CNT
UNSIGNED8	i;

	for (i = 0u; i < sleepTableCnt; i++)  {
		retVal = sleepTable[i](sleepMode, node);
		if (retVal != 0u)  {
			return(retVal);
		}
	}
#endif /* CO_EVENT_SLEEP_CNT */

#ifdef CO_EVENT_STATIC_SLEEP
	retVal = coEventSleepInd(sleepMode, node);
#endif /* CO_EVENT_STATIC_SLEEP */

	return(retVal);
}


/***************************************************************************/
/*
* \internal
*
* \brief coResetSleep - reset comm for Sleep
*
* If own nodeId is unknown (LSS device),
* 0 is used
*
* \return none
*
*/
void icoSleepReset(
		UNSIGNED8 master,		/* device is master */
		UNSIGNED8 nodeId		/* own node id */
	)
{
#ifdef CO_SLEEP_447
UNSIGNED8	i;
#else /* CO_SLEEP_447 */
(void)master;
(void)nodeId;
#endif /* CO_SLEEP_447 */

	(void)icoCobSet(sleepMasterCob, CO_SLEEP_MASTER_CAN_ID, CO_COB_RTR_NONE, 8u);

#ifdef CO_SLEEP_454
	(void)icoCobSet(sleepSlaveCob, CO_SLEEP_SLAVE_CAN_ID, CO_COB_RTR_NONE, 8u);
	(void)icoCobSet(sleepRequestCob, CO_SLEEP_REQ_SLEEP_CAN_ID, CO_COB_RTR_NONE, 8u);
#endif /* CO_SLEEP_454 */

#ifdef CO_SLEEP_447
	if (master != 0)  {
		for (i = 0; i < CO_SLEEP_447_CNT; i++)  { 
			(void)icoCobSet(sleepSlaveCob[i], CO_SLEEP_SLAVE_CAN_ID + 2 + i,
					CO_COB_RTR_NONE, 8u);
		}
	} else {
		/* slave */
		(void)icoCobSet(sleepSlaveCob[0], CO_SLEEP_SLAVE_CAN_ID + nodeId,
					CO_COB_RTR_NONE, 8u);
	}
#endif /* CO_SLEEP_447 */
}



/***************************************************************************/
/**
* \internal
*
* \brief icoSleepVarInit
*
*
* \return none
*
*/
void icoSleepVarInit(
		void
	)
{

	{
		sleepActive = CO_FALSE;

		sleepMasterCob = 0xffffu;
#ifdef CO_SLEEP_454
		sleepSlaveCob = 0xffffu;
		sleepRequestCob = 0xffffu;
#endif /* CO_SLEEP_454 */
	}

	sleepTableCnt = 0u;
}


/***************************************************************************/
/**
* \internal
*
* \brief icoSleepInit - init Sleep functionality
*
* This function initializes the Sleep functionality
*
* If parameter master is unequal 0 the node will be initialized as NMT master.
*
* \return RET_T
*
*/
RET_T icoSleepInit(
		UNSIGNED8		master		/* master mode */
	)
{
#ifdef CO_SLEEP_447
UNSIGNED16	i;

	if (master != 0u)  {
		sleepMasterCob = icoCobCreate(CO_COB_TYPE_TRANSMIT, CO_SERVICE_SLEEP, 0u);
		for (i = 0u; i < CO_SLEEP_447_CNT; i++)  {
			sleepSlaveCob[i] = icoCobCreate(CO_COB_TYPE_RECEIVE, CO_SERVICE_SLEEP, i);
		}


	} else {
		sleepMasterCob = icoCobCreate(CO_COB_TYPE_RECEIVE, CO_SERVICE_SLEEP, 0u);
		sleepSlaveCob[0] = icoCobCreate(CO_COB_TYPE_RECEIVE, CO_SERVICE_SLEEP, 0u);
	}

	if (sleepMasterCob == 0xffffu)  {
		return(RET_NO_COB_AVAILABLE);
	}

	for (i = 0u; i < CO_SLEEP_447_CNT; i++)  {
		if (sleepSlaveCob[i] == 0xffff)  {
			return(RET_NO_COB_AVAILABLE);
		}
	}
#endif /* CO_SLEEP_447 */

#ifdef CO_SLEEP_454
	if (master != 0u)  {
		sleepMasterCob = icoCobCreate(CO_COB_TYPE_TRANSMIT, CO_SERVICE_SLEEP, 0u);
		sleepSlaveCob = icoCobCreate(CO_COB_TYPE_RECEIVE, CO_SERVICE_SLEEP, 0u);
		sleepRequestCob = icoCobCreate(CO_COB_TYPE_RECEIVE, CO_SERVICE_SLEEP, 0u);
	} else {
		sleepMasterCob = icoCobCreate(CO_COB_TYPE_RECEIVE, CO_SERVICE_SLEEP, 0u);
		sleepSlaveCob = icoCobCreate(CO_COB_TYPE_TRANSMIT, CO_SERVICE_SLEEP, 0u);
		sleepRequestCob = icoCobCreate(CO_COB_TYPE_TRANSMIT, CO_SERVICE_SLEEP, 0u);
	}

	if ((sleepMasterCob == 0xffffu) || (sleepSlaveCob == 0xffffu)
	 || (sleepRequestCob == 0xffffu))  {

		return(RET_NO_COB_AVAILABLE);
	}
#endif /* CO_SLEEP_454 */

	return(RET_OK);
}
#endif /* CO_EVENT_SLEEP */
