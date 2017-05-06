/*
* codrv_can_generic.c - generic driver for basic can
*
* Copyright (c) 2012-2015 emtas GmbH
*-------------------------------------------------------------------
* $Id: codrv_can_generic.c 8833 2015-03-20 11:26:08Z ro $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief generic driver
*
* \file codrv_can_generic.c - generic driver for basic CAN
* \author emtas GmbH
*
* This module contains a skeleton for a basic can driver.
* It can be use to implement a new driver for CANopen library of emtas.
*
* The official small API for Filter usage is contained. But it is not 
* required for a basic CAN driver. 
*
*/

/**
* \define CO_DRV_FILTER
* This setting activates the filter functionality. But note,
* you need a lot of filter to use it effectively.
* For a slave for example for the following services:
* - NMT
* - SDO Request
* - n RPDOs
* optionally a slave can receive other nodes
* - Heart Beat (as consumer)
* - EMCY (as consumer)
* Typical the CAN controller is called FullCAN controller if it has
* for each filtered out CAN frame Id a own hardware storage (message object).
*
* It can also be a sophisticated CAN receiver, preferred with a hardware FIFO,
* with a sophisticated acceptance filter mechanism.
* 
*
* You have to set this define in gen_define.h!
*/

/**
* \define CO_DRV_GROUP_FILTER
* The group filter mechanism is a additional feature for the general
* filter mechanism. The most filter can set an acceptance mask. A 
* often used mask enable a group for all NodeIds of a specific
* command group, e.g. heartbeat consumer. In this case only one
* filter is required for 128 message identifiers.
*
* You have to set this define in gen_define.h!
*/

/**
* \define POLLING
* Often used driver internal define, e.g. during the development.
* In case this define is set, the driver don't use interrupts.
*
* You have to use it driver internal, only.
*/

/**
* \define CODRV_DEBUG
* Often used driver internal define to activate the printf()
* output for debugging. A completely correct functionality is not
* ensured, if this define is set. Please deactivate it!
*/ 

/**
* \define DEBUG_SEND_TESTMESSAGE
* Often used driver internal #define to send a transmit CAN frame
* during the initialization.
* For measurement purpose the message ID is 0x555 and the data byte
* 0x01..0x8.
* If no other CAN node is connected,
* the CAN controller will send this frame endless.
* This can be used to measure the bit time using an oscilloscope.
* Please deactivate it in production code!
*/

//only for internal use!
//#define DRIVER_TEST 1

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stddef.h>
#include <stdio.h>
#include "stm32f4xx.h" 
#include "bsp_can.h"
/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#include <co_datatype.h>
#include <co_drv.h>
#include <codrv_error.h>
#include <co_commtask.h>

#include <codrv_can_generic.h>
#ifdef DRIVER_TEST
#include <co_canopen.h>
#include <../src/ico_cobhandler.h>
#include <../src/ico_queue.h>
#endif

/* constant definitions
---------------------------------------------------------------------------*/
#define POLLING 1
#define DEBUG_SEND_TESTMESSAGE 1

/* OS related macros - default definition */
#ifdef CO_OS_SIGNAL_CAN_TRANSMIT
#else
#  define CO_OS_SIGNAL_CAN_TRANSMIT()
#endif

#ifdef CO_OS_SIGNAL_CAN_RECEIVE
#else
#  define CO_OS_SIGNAL_CAN_RECEIVE()
#endif



/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static RET_T codrvCanInitController(UNSIGNED16 bitRate);
static RET_T codrvCanTransmit(CO_CONST CO_CAN_MSG_T * pBuf);
static CO_CONST CODRV_BTR_T * codrvCanGetBtrSettings(UNSIGNED16 bitRate);

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static BOOL_T canEnabled = CO_FALSE; /**< CAN buson */
static volatile BOOL_T transmissionIsActive = CO_FALSE; /**< TX transmission active */


/** currently TX message */
static CO_CAN_MSG_T *pTxBuf = NULL;

/** CAN Controller address */
static volatile UNSIGNED32 * CO_CONST pCan = (void*)0x40006400ul; 


#ifdef CODRV_BIT_TABLE_EXTERN 
/* use an external bitrate table */
extern CO_CONST CODRV_BTR_T codrvCanBittimingTable[];
#else /* CODRV_BIT_TABLE_EXTERN */

/** can bittiming table */
static CO_CONST CODRV_BTR_T codrvCanBittimingTable[] = {
		/* e.g. 36MHz table, prescaler 10bit (max 1024) */
		{   10u,225u, 0, 13u, 2u }, 
		{   20u,120u, 0, 12u, 2u }, /* 86,7% */
		{   50u, 45u, 0, 13u, 2u }, /* 85% */
		{  100u, 18u, 0, 16u, 3u }, /* 85% */
		{  125u, 18u, 0, 13u, 2u },
		{  250u,  9u, 0, 13u, 2u },
		{  500u,  9u, 0,  6u, 1u }, /* only 8tq */
		{  800u,  3u, 0, 12u, 2u },
		{ 1000u,  2u, 0, 14u, 3u}, /* 83.3% */
		{0,0,0,0,0} /* last */
	};

#endif /* CODRV_BIT_TABLE_EXTERN */

/*---------------------------------------------------------------------------*/
#ifdef DEBUG_SEND_TESTMESSAGE
static void codrvSendTestMessage(void)
{
	/* CAN enable */
	/* CAN ID 0x555 */
	/* data length 8 */
	/* data 0x1..0x8 */

}
#endif /* DEBUG_SEND_TESTMESSAGE */
/*---------------------------------------------------------------------------*/



/***************************************************************************/
/**
* \brief codrvCanInit - init CAN controller
*
* This function initializes the CAN controller and configures the bitrate.
* At the end of the function, the CAN controller should be in state disabled.
*
* \return RET_T
* \retval RET_OK
*	initialization was OK
*
*/
RET_T codrvCanInit(
		UNSIGNED16	bitRate		/**< CAN bitrate */
	)
{
RET_T	retVal;

	/* some inital values */


	/* init driver */
	retVal = codrvCanInitController(bitRate);

	return(retVal);
}

/***************************************************************************/
/**
* \brief codrvCanReInit - reinit CAN controller
*
* This Function reinits the CAN controller after deactivation.
* 
* In Filter mode: After this function call all Filter are reset and must
* be reconfigured!
*
* At the end of the function, the CAN controller should be in state disabled.
*
* \param
*	bitrate	- CANopen bitrate
* \results
*	RET_T
*/

RET_T codrvCanReInit(
		UNSIGNED16	bitRate		/**< CAN bitrate */
	)
{
RET_T	retVal;

	retVal = codrvCanInitController(bitRate);

	return (retVal);
}

/***************************************************************************/
/**
* \brief codrvCanInitController - init CAN controller
*
* This Function inits the CAN controller and setup the bitrate
* After that, the CAN controller is disabled.
*
* \param
*	bitrate	- CANopen bitrate
* \results
*	RET_T
*
* \internal
*/

static RET_T codrvCanInitController(
		UNSIGNED16 bitRate
	)
{
RET_T	retVal;

	/* init req. variables */
	canEnabled = CO_FALSE;
	transmissionIsActive = CO_FALSE;

	pTxBuf = NULL;

	/* error states */
	codrvCanErrorInit();



	/* initialize CAN controller, setup timing, pin description, CAN mode ...*/


	/* set bitrate */
	retVal = codrvCanSetBitRate(bitRate);

#ifdef DEBUG_SEND_TESTMESSAGE
	codrvCanEnable();
	codrvSendTestMessage();
#endif /* DEBUG_SEND_TESTMESSAGE */

	return (retVal);

}

/***********************************************************************/
/**
* codrvCanGetBtrSettings - get pointer to the BTR value structure
*
* \internal
*
* \returns
*	pointer to an BTR table entry
*/

static CO_CONST CODRV_BTR_T * codrvCanGetBtrSettings(
		UNSIGNED16 bitRate	/**< required bitrate */
	)
{
CO_CONST CODRV_BTR_T * pBtrEntry = NULL;
UNSIGNED8 i;

	i = 0u;
	while (codrvCanBittimingTable[i].bitRate != 0u) {
		if (codrvCanBittimingTable[i].bitRate == bitRate) {
			pBtrEntry = &codrvCanBittimingTable[i];
			break;
		}
		i++;
	}

	return pBtrEntry;

}


/***********************************************************************/
/**
* \brief codrvCanSetBitRate - set CAN Bitrate
*
* This function sets the CAN Bitrate to the given value.
* Changing the Bitrate is only allowed, if the CAN controller is in reset.
* The state at the start of the function is unknown, 
* so the CAN controller should be switch to state reset.
*
* At the end of the function the CAN controller should be stay in state reset.
*
* \return RET_T
* \retval RET_OK
*	setting of Bitrate was OK
*
*/
RET_T codrvCanSetBitRate(
		UNSIGNED16	bitRate		/**< CAN Bitrate in kbit/s */
	)
{
CO_CONST CODRV_BTR_T * pBtrEntry;

UNSIGNED32 pre;
UNSIGNED32 seg1;
UNSIGNED32 seg2;

	/* stop CAN controller */
	(void)codrvCanDisable();

	/* get bittiming values */
	pBtrEntry = codrvCanGetBtrSettings(bitRate);

	if (pBtrEntry == NULL) {
		/* if Bitrate not supported */
		return(RET_DRV_WRONG_BITRATE); 
	}

	pre = pBtrEntry->pre; 
	seg1 = pBtrEntry->seg1 + pBtrEntry->prop; 
	seg2 = pBtrEntry->seg2; 


	/* setup timing registers for the given Bitrate */
	(void)pre;
	(void)seg1;
	(void)seg2;


    return(RET_OK);
}


/***********************************************************************/
/**
* \brief codrvCanEnable - enable CAN controller
*
* This function enables the CAN controller.
* At this point the enable bit is set. Typically the CAN controller
* requests 11 recessive bits to go in active mode.
* This will be checked later outside of this function.
*
* \return RET_T
* \retval RET_OK
*	CAN controller, enabled was set
*
*/
RET_T codrvCanEnable(
		void
	)
{
RET_T	retVal = RET_OK;

	/* if error is occurred */
	/* retVal = RET_DRV_ERROR; */


	/* enable CAN controller */


	/* Error active is later checked */
	/* later: canEnabled = CO_TRUE; */

	return(retVal);
}


/***********************************************************************/
/**
* \brief codrvCanDisable - disable CAN controller
*
* This function disables the CAN controller.  The function waits
* for the CAN controller being disabled. Code calling this function
* typically expects that after returning the CAN controller is in Init mode.
*
* But note, the time the CAN controller needs to enter the Init mode
* can be as long as the duration of one CAN frame.
*
* \return RET_T
* \retval RET_OK
*	CAN controller is set to be disabled
*
*/
RET_T codrvCanDisable(
		void
	)
{
RET_T	retVal = RET_OK;

	/* if error is occurred */
	/* retVal = RET_DRV_ERROR; */
	
	/* disable CAN controller */


	canEnabled = CO_FALSE;

	return(retVal);
}

#ifdef CO_DRV_FILTER
#error "Filter not supported, yet!"
/***********************************************************************/
/**
* codrvCanSetFilter - activate and configure the receive filter
*
* Depend of the COB entry's the driver specific filter will 
* be configured. 
*
* 
*
* \retval RET_OK
*	OK
* \retval RET_INVALID_PARAMETER
*	invalid COB reference
* \retval RET_DRV_ERROR
*	filter cannot be set, e.g. no free entry
*
*/

RET_T codrvCanSetFilter(
		CO_CAN_COB_T * pCanCob /**< COB reference */
	)
{
	return(RET_OK);
}
#endif /* CO_DRV_FILTER */

/***********************************************************************/
/**
* \brief codrvCanStartTransmission - start can transmission if not active
*
* Transmission of CAN messages should be interrupt driven.
* If a message was sent, the Transmit Interrupt is called
* and the next message can be transmitted.
* To start the transmission of the first message,
* this function is called from the CANopen stack.
*
* The easiest way to implement this function is
* to trigger the transmit interrupt, 
* but only of the transmission is not already active.
*
* \return RET_T
* \retval RET_OK
*	start transmission was successful
*
*/
RET_T codrvCanStartTransmission(
		void
	)
{
	/* if can is not enabled, return with error */
	if (canEnabled != CO_TRUE)  {
	//nie	return(RET_DRV_ERROR);
	}

	if (transmissionIsActive == CO_FALSE)  {
		/* trigger transmit interrupt */
#ifdef POLLING
		codrvCanTransmitInterrupt();
#else
		/* enable global interrupt pending bit to call interrupt */
		codrvCanSetTxInterrupt();
#endif

	}

	return(RET_OK);
}
 

/***********************************************************************/
/**
* \brief codrvCanTransmit - transmit can message
*
* This function writes a new message to the CAN controller and transmits it.
* Normally called from Transmit Interrupt
*
* \return RET_T
* \retval RET_OK
*	Transmission was OK
*
*/
static RET_T codrvCanTransmit(
		CO_CONST CO_CAN_MSG_T * pBuf		/**< pointer to data */
	)
{
RET_T	retVal = RET_OK;
/*
	printf("< TX 0x%03lx:%d:%02x%02x%02x%02x%02x%02x%02x%02x\n",
		pBuf->canCob.canId,
		(pBuf->canCob.rtr) ? -pBuf->len : pBuf->len ,
		pBuf->data[0],
		pBuf->data[1],
		pBuf->data[2],
		pBuf->data[3],
		pBuf->data[4],
		pBuf->data[5],
		pBuf->data[6],
		pBuf->data[7]);


*/
	/* write message to the CAN controller */
	/* 
	message.id = pBuf->canCob.canId;
	message.length = pBuf->len;
	memcpy(message.data, pBuf->data, message.length);
	*/

	/* transmit it */

  CAN1_WriteData(pBuf->canCob.canId,(unsigned char *)pBuf->data,(unsigned char)pBuf->len,0);
	transmissionIsActive = CO_TRUE;

	return(retVal);
}


/***********************************************************************/
/**
* \brief codrvCanDriverTransmitInterrupt - can driver transmit interrupt
*
* This function is called, after a message was transmitted.
*
* As first, inform stack about message transmission.
* Get the next message from the transmit buffer,
* write it to the CAN controller
* and transmit it.
*
* \return void
*
*/
void codrvCanTransmitInterrupt(
		void
	)
{


	/* inform stack about transmitted message */
	if (pTxBuf != NULL)  {
		coQueueMsgTransmitted(pTxBuf);
		pTxBuf = NULL;

		/* signal transmitted message */
		CO_OS_SIGNAL_CAN_TRANSMIT();
	}

	/* get next message from transmit queue */
	pTxBuf = coQueueGetNextTransmitMessage();
	if (pTxBuf != NULL)  {
		/* and transmit it */
		(void)codrvCanTransmit(pTxBuf);
	}
	transmissionIsActive = CO_FALSE;

}


/***********************************************************************/
/**
* \brief codrvCanReceiveInterrupt - can driver receive interrupt
*
* This function is called, if a new message was received.
* As first get the pointer to the receive buffer
* and save the message there.
* Then set the buffer as filled and inform the lib about new data.
*
* \return void
*
*/
void codrvCanReceiveInterrupt(CanRxMsg *canMsg)
{
CO_CAN_MSG_T *pRecBuf;
UNSIGNED16 j;
CAN_ERROR_FLAGS_T * pError;

/* --- Simulation ---- */
typedef struct {
	UNSIGNED32	id;
	UNSIGNED8	len;
	UNSIGNED8	data[8];
} CAN_SIMU_T;

	/* get receive buffer */
	pRecBuf = coQueueGetReceiveBuffer();
	if (pRecBuf == NULL)  {
		/* error, no buffer available */
		/* release mailbox */

		return;
	}

	
	/* save message at buffer */
	pRecBuf->canCob.canId = canMsg->StdId;
	pRecBuf->len = canMsg->DLC;
	for (j = 0u; j < pRecBuf->len; j++) {
		pRecBuf->data[j] = canMsg->Data[j];
	}

	pRecBuf->canCob.extended = CO_FALSE;
	pRecBuf->canCob.rtr = CO_FALSE;

	
	/* is overrun situation? */
	/*
		pError = codrvCanErrorGetFlags();
		pError->canErrorRxOverrun = CO_TRUE;
	*/

	/* release mailbox */

	/* set buffer filled */
	coQueueReceiveBufferIsFilled();

	/* inform stack about new data */
	coCommTaskSet(CO_COMMTASK_EVENT_MSG_AVAIL);

	/* signal received message */
	CO_OS_SIGNAL_CAN_RECEIVE();

}


/***********************************************************************/
/**
* \brief codrvCanErrorHandler - local Error handler
*
* This function polls the current state of the CAN controller
* and checks explicitly all situation that are not signalled
* within the interrupts.
*
* To be called outside of interrupts!
* Typically called in codrvCanDriverHandler().
*
* \return void
*
*/

static void codrvCanErrorHandler(void)
{
CAN_ERROR_FLAGS_T * pError;

	pError = codrvCanErrorGetFlags();
	(void)pError; /* remove unused warning */


		/* busoff */
		/*	pError->canNewState = Error_Busoff; */

		/* error passive */
		/*	pError->canNewState = Error_Passive; */

		/* error active */
		/*	pError->canNewState = Error_Active; */


	/* correct possible Errors -> CAN has deactivated the transmission */
		/* transmissionIsActive = CO_FALSE; */

		/* CAN was deactivated */
		/* canEnabled = CO_FALSE; */

		/* CAN is active, now */
		/* canEnabled = CO_TRUE;*/


		/* change: CAN deactive -> active => check for buffered transmissions */
		/* (void)codrvCanStartTransmission(); */

	if (canEnabled == CO_TRUE) {
		/* check for stopped transmissions */
		if ((transmissionIsActive == CO_FALSE) && (pTxBuf != NULL)) {
			/* transmission aborted, e.g. busoff, 
		     * discard message -> is done within the tx interrupt
			*/
			(void)codrvCanStartTransmission();
		}
	}
}


/***********************************************************************/
/**
* \brief codrvCanDriverHandler - can driver handler
*
* This function is cyclically called from the CANopen stack
* to get the current CAN state
* (BUS_OFF, PASSIVE, ACTIVE).
*
* If a bus off event has occurred,
* this function should try to get to bus on again
* (activate the CAN controller).
*
* \return void
*
*/
void codrvCanDriverHandler(
		void
	)
{
	/* check current state */
	codrvCanErrorHandler();

	/* inform stack about the state changes during two handler calls */
	(void)codrvCanErrorInformStack();



    return;
}


#ifdef DRIVER_TEST
/***********************************************************************/
/* Transmit Tests
* req. 1 additional COB -> CANopen Device Designer!
*
* 11bit Dataframe id 0x123
* 11bit RTR id 0x234
* 29bit dataframe in 0x345
* 29bit RTR id 0x456
*/
/***********************************************************************/
static COB_REFERENZ_T	drvCob = 0xffffu;
RET_T transmitFrames(void)
{
static char trData[8] = {1,2,3,4,5,6,7,8};
static volatile RET_T			retVal;

	if (drvCob == 0xffffu)  {
		drvCob = icoCobCreate(CO_COB_TYPE_TRANSMIT, CO_SERVICE_CAN_DEBUG, 0u);
		if (drvCob == 0xffffu)  {
			return(RET_EVENT_NO_RESSOURCE);
		}
	}

	icoCobChangeType(drvCob, CO_COB_TYPE_TRANSMIT);
	retVal = icoCobSet(drvCob, 0x123 , CO_COB_RTR_NONE, 1);
	retVal = icoTransmitMessage(drvCob, (CO_TRANS_DATA_T *)&trData, 0u);

	icoCobChangeType(drvCob, CO_COB_TYPE_RECEIVE);
	retVal = icoCobSet(drvCob, 0x234 , CO_COB_RTR, 2);
	retVal = icoTransmitMessage(drvCob, (CO_TRANS_DATA_T *)&trData, 0u);

	icoCobChangeType(drvCob, CO_COB_TYPE_TRANSMIT);
	retVal = icoCobSet(drvCob, 0x345 + CO_COB_29BIT , CO_COB_RTR_NONE, 3);
	retVal = icoTransmitMessage(drvCob, (CO_TRANS_DATA_T *)&trData, 0u);

	icoCobChangeType(drvCob, CO_COB_TYPE_RECEIVE);
	retVal = icoCobSet(drvCob, 0x456 + CO_COB_29BIT, CO_COB_RTR, 4);
	retVal = icoTransmitMessage(drvCob, (CO_TRANS_DATA_T *)&trData, 0u);

	return (RET_OK);
}
#endif
