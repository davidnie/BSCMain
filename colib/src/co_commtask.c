/*
* co_commtask.c - contains communication task of canopen library
*
* Copyright (c) 2012-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_commtask.c 15302 2016-09-22 15:26:53Z boe $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief communication task routines
*
* \file co_commtask.c
* contains communication task functions of canopen library
*
*/


/********************************************************************/
/**
* \mainpage CANopen Stack Reference Manual
*
* \section i Introduction
* The CANopen Slave Stack of emtas is a software library
* that provides all communication services
* of the "CANopen Application Layer and Communication Profile"
* CiA 301 V4.2 and other profiles of CiA e.V. and EN50325-4.
*
* The main features are:
*	- well-defined interface between driver and CANopen stack
*	- ANSI-C conform
*	- MISRA checked
*	- easy-to-handle Application Programming Interface
*	- static and dynamic object dictionary are possible
*	- LED CiA-303
*	- Layer Setting Services (CiA 305),
*	- configurable and scalable
*	- extensions for additional communication profiles such as
*		- redundant communication (CiA 302),
*		- safety relevant communication (CiA 304)
*	as well as device profile implementations like
*		- Generic I/O Modules (CiA 401)
*		- EnergyBus Protokoll (CiA 454)
* are available.
*
* This reference manual describes the functions 
* for the API to evaluate the received data
* and to use the CANopen services in the network.
*
* Configuration and features settings are supported
* by the graphical configuration tool CANopen DeviceDesigner.
*
* \section g General
* The CANopen stack use strict data hiding,
* so access to internal data are only possible by functions.
* The same is valid for access to the communication segment of the 
* object dictionary.
*
* \section u Using CANopen stack in an application
* At startup, some initialization functions are necessary:
* 	- codrvHardwareInit()	- generic, CAN related hardware initialization
* 	- codrvCanInit()	- initialize CAN driver
* 	- coCanOpenStackInit()	- initialize CANopen functionality
* 	- codrvTimerSetup()	- initialize hardware timer
* 	- codrvCanEnable()	- start CAN communication
*
* For the CANopen functionality,
* the central function
* coCommTask()
* has to be called in case of
*	- new CAN message was received
*	- timer period has been ellapsed.
*
* Therefore signal handlers should be used
* or a cyclic call of the function coCommTask() is necessary.
* For operating systems (like LINUX) the function
* codrvWaitForEvent()
* can be used to wait for events.
* <br>All CANopen functionality is handled inside this function.
*
* The start of CANopen services are also possible.
*
* \section c Indication functions
* Indication functions inform application about CAN and CANopen service events.
* <br>To receive an indication,
* the application has to register a function
* by the apropriate service register function like coEventRegister_PDO().
* <br>Every time the event occures, 
* the registered indication function is called.
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>
#include <co_datatype.h>
#include <co_timer.h>
#include <co_drv.h>
#ifdef J1939_SUPPORTED
#else /* J1939_SUPPORTED */
# include <co_odaccess.h>
# include <co_led.h>
# include <co_nmt.h>
# include "ico_indication.h"
#endif /* J1939_SUPPORTED */
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_commtask.h"
#include "ico_timer.h"
#include "ico_event.h"
#ifdef J1939_SUPPORTED
#else /* J1939_SUPPORTED */
# include "ico_nmt.h"
# include "ico_pdo.h"
# ifdef CO_SDO_NETWORKING
#  include "ico_sdoserver.h"
# endif /* CO_SDO_NETWORKING */
# include "ico_sdo.h"
# include "ico_sync.h"
# include "ico_emcy.h"
# include "ico_time.h"
#endif /* J1939_SUPPORTED */
#ifdef CO_LSS_SUPPORTED
# include "ico_lss.h"
#endif /* CO_LSS_SUPPORTED */
#ifdef CO_LSS_MASTER_SUPPORTED
# include "ico_lss.h"
#endif /* CO_LSS_MASTER_SUPPORTED */
#ifdef CO_EVENT_SLEEP
# include "ico_sleep.h"
#endif /* CO_EVENT_SLEEP */
#ifdef CO_USER_EXTENSION_SUPPORTED
# include "ico_user.h"
#endif /* CO_USER_EXTENSION_SUPPORTED */
#ifdef CO_GFC_SUPPORTED
# include "ico_gfc.h"
#endif /* CO_GFC_SUPPORTED */
#ifdef CO_SRDO_SUPPORTED
# include "ico_srdo.h"
#endif /* CO_GFC_SUPPORTED */
#ifdef J1939_SUPPORTED
# include "ico_j1939.h"
#endif /* J1939_SUPPORTED */
#ifdef CO_USDO_SUPPORTED
# include "ico_usdoserver.h"
#endif /* CO_USDO_SUPPORTED */

/* constant definitions
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_DYNAMIC_CAN
# ifdef CO_EVENT_PROFILE_CAN
#  define CO_EVENT_CAN_CNT	(CO_EVENT_DYNAMIC_CAN + CO_EVENT_PROFILE_CAN)
# else /* CO_EVENT_PROFILE_CAN */
#  define CO_EVENT_CAN_CNT	(CO_EVENT_DYNAMIC_CAN)
# endif /* CO_EVENT_PROFILE_CAN */
#else /* CO_EVENT_DYNAMIC_CAN */
# ifdef CO_EVENT_PROFILE_CAN
#  define CO_EVENT_CAN_CNT	(CO_EVENT_PROFILE_CAN)
# endif /* CO_EVENT_PROFILE_CAN */
#endif /* CO_EVENT_DYNAMIC_CAN */

#if defined(CO_EVENT_STATIC_CAN) || defined(CO_EVENT_CAN_CNT)
# define CO_EVENT_CAN	1u
#endif /* defined(CO_EVENT_STATIC_CAN) || defined(CO_EVENT_CAN_CNT) */


#define COMM_STATE_TABLE_CNT	12u

/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_STATIC_CAN
extern CO_CONST CO_EVENT_CAN_STATE_T coEventCanStateInd;
extern CO_CONST CO_EVENT_COMM_T coEventCommInd;
#endif /* CO_EVENT_STATIC_CAN */


/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static void serviceManagement(void);
static BOOL_T receiveMessageHandler(void);

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static BOOL_T	eventFlagMsg = { CO_FALSE };
static BOOL_T	eventFlagTimer = { CO_FALSE };
static BOOL_T	eventFlagEventHandler = { CO_FALSE };
static CO_CAN_STATE_T canState = { CO_CAN_STATE_BUS_OFF };
static CO_COMM_STATE_EVENT_T recQuState = { CO_COMM_STATE_EVENT_REC_QUEUE_EMPTY };
static CO_COMM_STATE_EVENT_T trQuState = { CO_COMM_STATE_EVENT_BUS_OFF_RECOVERY };
#ifdef CO_EVENT_CAN_CNT
static UNSIGNED8	canStateTableCnt = 0u;
static UNSIGNED8	commEventTableCnt = 0u;
static CO_EVENT_CAN_STATE_T	canStateTable[CO_EVENT_CAN_CNT];
static CO_EVENT_COMM_T		commEventTable[CO_EVENT_CAN_CNT];
#endif /* CO_EVENT_CAN_CNT */



/***************************************************************************/
/**
* \brief coCommTask - main communication task
*
* This is the main communication task for the CANopen stack.
* It has to be called cyclically by the application or signal
* driven after each received CAN message or timer event.
*
* \return void
*
*/
void coCommTask(
		void	/* no parameter */
	)
{
    /* call protocol handler */
	serviceManagement();

	/* call cob handler */

	/* call queue handler */
	icoQueueHandler();

	/* call driver handler */
 codrvCanDriverHandler();

#ifdef CO_SRDO_SUPPORTED
	icoSrdoTimerCheck();
#endif /* CO_SRDO_SUPPORTED */

}


/***************************************************************************/
/**
*
* \internal
*
* \brief serviceManagement - call canopen protocol handler
*
* handles all canopen protocol services
* like service management, timer handling, communication state changes....
*
*
* \return void
*
*/
static void serviceManagement(
		void	/* no parameter */
	)
{
	/* message handler - if new messages are available */
	if (eventFlagMsg != CO_FALSE)  {
		if (receiveMessageHandler() == CO_FALSE)  {
			eventFlagMsg = CO_FALSE;
		}
	}

	/* timer handling - if timer has elapsed */
	if (eventFlagTimer != CO_FALSE)  {
		eventFlagTimer = CO_FALSE;
		icoTimerCheck();
	}

	if (eventFlagEventHandler != CO_FALSE)  {
		eventFlagEventHandler = CO_FALSE;
		icoEventCheck();
	}

	/* communication state changes */
/*	commStateHandler();*/

}


/***************************************************************************/
/**
*
* \internal
*
* \brief receiveMessageHandler - handles new receive messages
*
* handles new message from CAN
* read it from queue handler and call approproate service function
*
* \return message available
* \retval CO_FALSE
*	no message available
* \retval CO_TRUE
*	message available
*/
static BOOL_T receiveMessageHandler(
		void	/* no parameter */
	)
{
CO_REC_DATA_T	recData;
BOOL_T		avail;

	/* get next message from Queue handler */
	avail = icoQueueGetReceiveMessage(&recData);
	if (avail == CO_FALSE)  {
		return(avail);
	}

	switch (recData.service)  {
#ifdef CO_SDO_SERVER_CNT
		case CO_SERVICE_NMT:
			icoNmtMsgHandler(&recData);
			break;
#endif /* CO_SDO_SERVER_CNT */

#ifdef CO_GFC_SUPPORTED
		case CO_SERVICE_GFC_RECEIVE:
			icoGfcHandler();
			break;
#endif /* CO_GFC_SUPPORTED */

#ifdef CO_SRDO_SUPPORTED
		case CO_SERVICE_SRDO_RECEIVE:
			icoSrdoHandler(&recData);
			break;
#endif /* CO_SRDO_SUPPORTED */

#ifdef CO_SYNC_SUPPORTED
		case CO_SERVICE_SYNC_RECEIVE:
			/* check for correct message len */
			if (recData.msg.len == 0u) {
				recData.msg.data[0] = 0u;
			}
			if (recData.msg.len < 2u)  {
				icoSyncHandler(recData.msg.data[0]);
			}
			break;
#endif /* CO_SYNC_SUPPORTED */

#ifdef CO_PDO_RECEIVE_CNT
		case CO_SERVICE_PDO_RECEIVE:
			icoPdoReceiveHandler(&recData);
			break;
#endif /* CO_PDO_RECEIVE_CNT */

#ifdef CO_SDO_SERVER_CNT
		case CO_SERVICE_ERRCTRL:
			icoNmtErrorCtrlHandler(&recData);
			break;

		case CO_SERVICE_SDO_SERVER:
			icoSdoServerHandler(&recData
# ifdef CO_SDO_NETWORKING
				, 0u
# endif /* CO_SDO_NETWORKING */
				);
			break;
#endif /* CO_SDO_SERVER_CNT */

#ifdef CO_USDO_SUPPORTED
		case CO_SERVICE_USDO_SERVER: 
			icoUsdoServerHandler(&recData);
			break;
#endif /* CO_USDO_SUPPORTED */

#ifdef CO_SDO_CLIENT_CNT
		case CO_SERVICE_SDO_CLIENT:
			icoSdoClientHandler(&recData);
			break;
#endif /* CO_SDO_CLIENT_CNT */

#ifdef CO_EMCY_CONSUMER_CNT
		case CO_SERVICE_EMCY_RECEIVE:
			icoEmcyConsumerHandler(&recData);
			break;
#endif /* CO_EMCY_CONSUMER_CNT */

#ifdef CO_PDO_TRANSMIT_CNT
		case CO_SERVICE_PDO_TRANSMIT:
			icoPdoRtrHandler(&recData);
			break;
#endif /* CO_PDO_RECEIVE_CNT */

#ifdef CO_TIME_SUPPORTED
		case CO_SERVICE_TIME_RECEIVE:
			/* check for correct message len */
			if (recData.msg.len == 6u)  {
				icoTimeHandler(&recData.msg.data[0]);
			}
			break;
#endif /* CO_TIME_SUPPORTED */

#ifdef CO_LSS_SUPPORTED
		case CO_SERVICE_LSS_S_RECEIVE:
			icoLssHandler(&recData.msg.data[0]);
			break;
#endif /* CO_LSS_SUPPORTED */

#ifdef CO_LSS_MASTER_SUPPORTED
		case CO_SERVICE_LSS_M_RECEIVE:
			icoLssMasterHandler(&recData.msg.data[0]);
			break;
#endif /* CO_LSS_MASTER_SUPPORTED */

#ifdef CO_FLYING_MASTER_SUPPORTED
		case CO_SERVICE_FLYMA:
			icoNmtFlymaHandler(&recData);
			break;
#endif /* CO_FLYMA_SUPPORTED */

#ifdef CO_GUARDING_CNT
		case CO_SERVICE_GUARDING:
			icoGuardingHandler(&recData);
			break;
#endif /* CO_GUARDING_CNT */

#ifdef CO_EVENT_SLEEP
		case CO_SERVICE_SLEEP:
			icoSleepMsgHandler(&recData);
			break;
#endif /* CO_EVENT_SLEEP */

#ifdef CO_USER_EXTENSION_SUPPORTED
		case CO_SERVICE_USER:
			icoUserHandler(&recData);
			break;
#endif /* CO_USER_EXTENSION_SUPPORTED */

#ifdef J1939_SUPPORTED
		case CO_SERVICE_J1939_RECEIVE:
			ij1939_receive(&recData);
			break;
#endif /* J1939_SUPPORTED */

		default:
			break;
	}

	return(avail);
}


/***************************************************************************/
/**
*
* \internal
*
* \brief coCommTaskSet - set signal from CAN or timer
*
* This function signals new available messages from CAN or from timer
*
* \return void
*
*/
CO_INLINE void coCommTaskSet(
		CO_COMMTASK_EVENT_T	event
	)
{
	switch (event)  {
		case CO_COMMTASK_EVENT_TIMER:
			eventFlagTimer = CO_TRUE;
			break;
		case CO_COMMTASK_EVENT_MSG_AVAIL:
			eventFlagMsg = CO_TRUE;
			break;
		case CO_COMMTASK_EVENT_NEW_EVENT:
			eventFlagEventHandler = CO_TRUE;
			break;
	}
}


/***************************************************************************/
/**
* \brief coCommStateEvent - set a new communication state
*
* This function should be called,
* if a new communication state has been reached.
* <br>
* It sets the LEDs and informs the application about the event.
*
* \return void
*
*/
void coCommStateEvent(
		CO_COMM_STATE_EVENT_T	newEvent		/**< new communication event */
	)
{
typedef enum {
	CST_CAN, CST_TRQ, CST_RECQ
} COMM_STATE_TYPE_T;

typedef struct {
	CO_COMM_STATE_EVENT_T	event;		/* comm event */
	COMM_STATE_TYPE_T		type;
	CO_CAN_STATE_T			state;		/* goto CAN state */
} CO_NV_STORAGE COMM_STATE_TABLE_T;

static const COMM_STATE_TABLE_T	commTable[COMM_STATE_TABLE_CNT] = {
	{ CO_COMM_STATE_EVENT_REC_QUEUE_FULL,	CST_RECQ,	CO_CAN_STATE_UNCHANGED},
	{ CO_COMM_STATE_EVENT_REC_QUEUE_OVERFLOW,CST_RECQ,	CO_CAN_STATE_UNCHANGED},
	{ CO_COMM_STATE_EVENT_REC_QUEUE_EMPTY,	CST_RECQ,	CO_CAN_STATE_UNCHANGED},
	{ CO_COMM_STATE_EVENT_TR_QUEUE_FULL,	CST_TRQ,	CO_CAN_STATE_UNCHANGED},
	{ CO_COMM_STATE_EVENT_TR_QUEUE_OVERFLOW,CST_TRQ,	CO_CAN_STATE_UNCHANGED},
	{ CO_COMM_STATE_EVENT_TR_QUEUE_EMPTY,	CST_TRQ, 	CO_CAN_STATE_UNCHANGED},
	{ CO_COMM_STATE_EVENT_CAN_OVERRUN,		CST_RECQ,	CO_CAN_STATE_UNCHANGED},
	{ CO_COMM_STATE_EVENT_BUS_OFF,			CST_CAN,	CO_CAN_STATE_BUS_OFF },
	{ CO_COMM_STATE_EVENT_BUS_OFF_RECOVERY, CST_CAN,	CO_CAN_STATE_BUS_OFF },
	{ CO_COMM_STATE_EVENT_BUS_ON,			CST_CAN,	CO_CAN_STATE_BUS_ON },
	{ CO_COMM_STATE_EVENT_PASSIVE,			CST_CAN,	CO_CAN_STATE_PASSIVE },
	{ CO_COMM_STATE_EVENT_ACTIVE,			CST_CAN,	CO_CAN_STATE_BUS_ON }
};
UNSIGNED8	i;
CO_CAN_STATE_T	newCanState = CO_CAN_STATE_UNCHANGED;
CO_COMM_STATE_EVENT_T newState = CO_COMM_STATE_EVENT_NONE;
#ifdef CO_EVENT_CAN_CNT
UNSIGNED8	cnt;
#endif /* CO_EVENT_CAN_CNT */

	i = 0u;
	/* search event at event table */
	while (i < COMM_STATE_TABLE_CNT)  {
		if (commTable[i].event == newEvent)  {
			break;
		}
		i++;
	}

	/* no state found ? */
	if (i == COMM_STATE_TABLE_CNT)  {
		return;
	}

	/* event changes CAN state ? */
	if (commTable[i].state != CO_CAN_STATE_UNCHANGED)  {
		newCanState = commTable[i].state;

		/* new CAN state reached ? */
		if (canState != newCanState)  {
			/* if bus-off, go to preop */
#ifdef CO_SDO_SERVER_CNT
			if (newCanState == CO_CAN_STATE_BUS_OFF)  {
				icoErrorBehavior();
			}
#endif /* CO_SDO_SERVER_CNT */

#ifdef CO_EVENT_CAN_CNT
			cnt = canStateTableCnt;
			while (cnt > 0u)  {
				cnt--;
				/* call user indication */
				canStateTable[cnt](newCanState);
			}
#endif /* CO_EVENT_CAN_CNT */
#ifdef CO_EVENT_STATIC_CAN
			coEventCanStateInd(newCanState);
#endif /* CO_EVENT_STATIC_CAN */

#ifdef CO_EVENT_LED
			/* set led */
			switch (newCanState)  {
				case CO_CAN_STATE_BUS_OFF:
					coLedSetState(CO_LED_STATE_ON, CO_TRUE);
					break;
				case CO_CAN_STATE_BUS_ON:
					coLedSetState(CO_LED_STATE_ON, CO_FALSE);
					coLedSetState(CO_LED_STATE_FLASH_1, CO_FALSE);
					break;
				case CO_CAN_STATE_PASSIVE:
					coLedSetState(CO_LED_STATE_FLASH_1, CO_TRUE);
					break;
				case CO_CAN_STATE_UNCHANGED:
				default:
					break;
			}
#endif /* CO_EVENT_LED */

		}
		canState = newCanState;
	}

	/* check for transmit queue event */
	if (commTable[i].type == CST_TRQ)  {
		if (trQuState != newEvent)  {
			trQuState = newEvent;
			newState = newEvent;
		}
	}

	/* check for receive queue event */
	if (commTable[i].type == CST_RECQ)  {
		if (recQuState != newEvent)  {
			recQuState = newEvent;
			newState = newEvent;
		}
	}

	/* call user indication */
	if (newState != CO_COMM_STATE_EVENT_NONE)  {
#ifdef CO_EVENT_CAN_CNT
		cnt = commEventTableCnt;
		while (cnt > 0u)  {
			cnt--;
			/* call user indication */
			commEventTable[cnt](newEvent);
		}
#endif /* CO_EVENT_CAN_CNT */
#ifdef CO_EVENT_STATIC_CAN
		coEventCommInd(newEvent);
#endif /* CO_EVENT_STATIC_CAN */
	}
}


/***************************************************************************/
/*
* \brief icoCommTaskVarInit - init comm task variables
*
*/
void icoCommTaskVarInit(
		void
	)
{

	{
		eventFlagMsg = CO_FALSE;
		eventFlagTimer = CO_FALSE;
		eventFlagEventHandler = CO_FALSE;

		canState = CO_CAN_STATE_BUS_OFF;
		recQuState = CO_COMM_STATE_EVENT_REC_QUEUE_EMPTY;
		trQuState = CO_COMM_STATE_EVENT_BUS_OFF_RECOVERY;
	}

#ifdef CO_EVENT_CAN_CNT
	canStateTableCnt = 0u;
	commEventTableCnt = 0u;
#endif /* CO_EVENT_CAN */
}


#ifdef CO_EVENT_CAN_CNT
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**
* \brief coEventRegister_CAN_STATE - register can state changes
*
* With this function the application can register a function which is called,
* when the CAN state was changed.<br>
* CAN states are:<br>
*	- BUS_OFF
*	- BUS_ON
*	- PASSIV
*	- UNCHANGED
*
* \return	RET_T
*
*/

RET_T coEventRegister_CAN_STATE(
		CO_EVENT_CAN_STATE_T pFunction		/**< pointer to function */
    )
{
	/* set new indication function as first at the list */
	if (canStateTableCnt >= CO_EVENT_CAN_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	canStateTable[canStateTableCnt] = pFunction;	/* save function pointer */
	canStateTableCnt++;

	return(RET_OK);
}


/***************************************************************************/
/**
* \brief coEventRegister_COMM_EVENT - register communication event changes
*
* With this function the application can register a function which is called,
* when the communication state has been changed.
*	- BUS OFF - no communication possible
*	- CAN OVERRUN - messages was lost
*	- Receice queue full - receive messages is full
*	- Receice queue overrun - receive messages was lost
*	- Transmit queue full - no more messages can be send
*	- Transmit queue overflow - transmit messages was lost
*	- Transmit queue empty - new transmit messages can be send
*
* \return RET_T
*
*/

RET_T coEventRegister_COMM_EVENT(
		CO_EVENT_COMM_T	pFunction	/**< pointer to function */
    )
{
	/* set new indication function as first at the list */
	if (commEventTableCnt >= CO_EVENT_CAN) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	commEventTable[commEventTableCnt] = pFunction;	/* save function pointer */
	commEventTableCnt++;

	return(RET_OK);
}
#endif /* CO_EVENT_CAN_CNT */
