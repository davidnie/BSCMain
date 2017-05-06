/*
* driver for mscan
*
* Copyright (c) 2012-2015 emtas GmbH
*-------------------------------------------------------------------
* $Id: codrv_mscan.c 15789 2016-10-27 14:13:55Z ro $
*
*
*-------------------------------------------------------------------
*
*/


/********************************************************************/
/**
* \brief   msCAN CAN driver (e.g HCS12z)
*
* \file codrv_mscan.c - driver for MSCAN V3
* \author emtas GmbH
*
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

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>
#include <co_datatype.h>
#include <co_drv.h>
#include <co_commtask.h>
#include "codrv_mscan.h"
#include "codrv_error.h"

#ifdef DRIVER_TEST
#include <co_canopen.h>
#include <../src/ico_cobhandler.h>
#include <../src/ico_queue.h>
#endif

/* constant definitions
---------------------------------------------------------------------------*/
#define CAN_NR  0

#define SJW     1   /* must be 1..4 */

//#define POLLING 1
//#define DEBUG_SEND_TESTMESSAGE 1

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
void coCanDriverTransmitInterrupt(void);

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
static BOOL_T canEnabled = CO_FALSE;
static volatile BOOL_T transmissionIsActive = { CO_FALSE }; /**< TX transmission active */

/** current TX message */
static CO_CAN_MSG_T *pTxBuf = NULL;

/** CAN Controller address */
//#define CAN_ADDR  (0x0140 + (CAN_NR * 0x40) + ((CAN_NR >> 2) * 0x40))
#define CAN_ADDR  (0x0800)
static volatile UNSIGNED8 * const pCan = (UNSIGNED8 *) CAN_ADDR; 


#ifdef CODRV_BIT_TABLE_EXTERN 
/* use an external bitrate table */
extern CO_CONST CODRV_BTR_T codrvCanBittimingTable[];
#else /* CODRV_BIT_TABLE_EXTERN */

/** can bittiming table */
static CO_CONST CODRV_BTR_T codrvCanBittimingTable[] = {
		/* e.g. 16MHz table, prescaler 10bit (max 1024) */
		{   20u, 50u, 0, 13u, 2u },		
		{   50u, 20u, 0, 13u, 2u },
		{  125u,  8u, 0, 13u, 2u },
		{  250u,  4u, 0, 13u, 2u },
		{  500u,  2u, 0, 13u, 2u },
		{  800u,  1u, 0, 17u, 2u },
		{ 1000u,  1u, 0, 13u, 2u },
		{    0u,0u,0u,0u,0u}		/* last */
	};
#endif /* CODRV_BIT_TABLE_EXTERN */


/*---------------------------------------------------------------------------*/
#ifdef DEBUG_SEND_TESTMESSAGE
static void codrvSendTestMessage(void)
{
    pCan[MSCAN_CANCTL0] &= ~(MSCAN_CANCTL0_INITRQ | MSCAN_CANCTL0_SLPRQ);       
    while ((pCan[MSCAN_CANCTL1] & MSCAN_CANCTL1_INITAK) != 0);
    while ((pCan[MSCAN_CANCTL1] & MSCAN_CANCTL1_SLPAK) != 0);

	/* wait until Tx buffer released */
	while((pCan[MSCAN_CANTFLG] & MSCAN_CANTFLG_TXE0) == 0);	

	/* we only use TX buffer 0 for now */
    pCan[MSCAN_CANTBSEL] = MSCAN_CANTBSEL_TX0;  

	/* id of test message is 0x555 */
	pCan[MSCAN_CANTXIDR0] = (UNSIGNED8)(0x555u >> 3);
	pCan[MSCAN_CANTXIDR1] = (UNSIGNED8)(0x555u << 5);
	
	/* set data bytes of test message */
    pCan[MSCAN_CANTXDSR0] = 0x01;
    pCan[MSCAN_CANTXDSR1] = 0x02;
    pCan[MSCAN_CANTXDSR2] = 0x03;
    pCan[MSCAN_CANTXDSR3] = 0x04;
    pCan[MSCAN_CANTXDSR4] = 0x05;
    pCan[MSCAN_CANTXDSR5] = 0x06;
    pCan[MSCAN_CANTXDSR6] = 0x07;
    pCan[MSCAN_CANTXDSR7] = 0x08;

	/* set length of test message */
    pCan[MSCAN_CANTXDLR] = 8;		

	/* initiate CAN transmission */              
    pCan[MSCAN_CANTFLG]  = MSCAN_CANTFLG_TXE0;			
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
		UNSIGNED16		bitRate		/**< CAN bitrate */
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
*
*/

RET_T codrvCanReInit(
		UNSIGNED16		bitRate		/**< CAN bitrate */
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
		UNSIGNED16	bitRate		/* Bitrate */
	)
{
RET_T	retVal = RET_OK;

	/* init req. variables */
	canEnabled = CO_FALSE;
	transmissionIsActive = CO_FALSE;
	pTxBuf = NULL;

	/* error states */
	codrvCanErrorInit();

	/* initialize CAN controller, setup timing, pin description, CAN mode ...*/

	/* reset wupif to prevent sleep mode request blocking */
	pCan[MSCAN_CANRFLG] = 0u;

    if ((pCan[MSCAN_CANCTL1] & MSCAN_CANCTL1_INITAK) == 0u)  { 
		/* Sleepmode -> wait for recessiv bus 
		* reset all other settings
		*/
		pCan[MSCAN_CANCTL0] = MSCAN_CANCTL0_SLPRQ;
		while ((pCan[MSCAN_CANCTL1] & MSCAN_CANCTL1_SLPAK) == 0u) { } 
		
		/* set INITRQ, this will also set INITAK */
		pCan[MSCAN_CANCTL0] |= MSCAN_CANCTL0_INITRQ;

		/* wait for init mode to occur */
		while ((pCan[MSCAN_CANCTL1] & MSCAN_CANCTL1_INITAK) == 0u) { } 
 	}

	/* Set CANE just in case this is the first time after reset 
	 * CAN clock => oscillator clock
	 * Automatic bus-off recovery
	 */
	pCan[MSCAN_CANCTL1] = MSCAN_CANCTL1_CANE;//TMB - this selects the Oscillator clock
//	pCan[MSCAN_CANCTL1] = MSCAN_CANCTL1_CANE | MSCAN_CANCTL1_CLKSRC;//TMB - this selects the bus clock

	/* configure the Receiver - receive all 
     *----------------------------------------------------------------*/
 
	/* we are going to use two 32-bit acceptance filters: */
	pCan[MSCAN_CANIDAC] = 0x00u;

	/* standard id and RTR */
	pCan[MSCAN_CANIDMR0] = 0xff;
	pCan[MSCAN_CANIDMR1] = 0xe0 | MSCAN_IDR1_RTR_BASE;
	pCan[MSCAN_CANIDMR2] = 0x00;
	pCan[MSCAN_CANIDMR3] = 0x00;

	/* extended id and RTR */
	pCan[MSCAN_CANIDMR4] = 0xff;
	pCan[MSCAN_CANIDMR5] = 0xff;
	pCan[MSCAN_CANIDMR6] = 0xff;
	pCan[MSCAN_CANIDMR7] = 0xff;


	/* set bitrate */
	retVal = codrvCanSetBitRate(bitRate);

	/* exit Init Mode */
	pCan[MSCAN_CANCTL0] &= ~MSCAN_CANCTL0_INITRQ;          

	/* wait until Normal Mode is established    */
	while((pCan[MSCAN_CANCTL1] & MSCAN_CANCTL1_INITAK) != 0u) ;

	/* use (only) TX buffer 0 */
	pCan[MSCAN_CANTBSEL] = MSCAN_CANTBSEL_TX0;

#ifdef DEBUG_SEND_TESTMESSAGE
	codrvCanEnable();
	codrvSendTestMessage();
#endif /* DEBUG_SEND_TESTMESSAGE */

	return(retVal);
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
		UNSIGNED16 bitRate	/* required bitrate */
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
		UNSIGNED16	bitRate	/**< bit rate */
	)
{
CO_CONST CODRV_BTR_T * pBtrEntry;
UNSIGNED8 seg1;
UNSIGNED8 seg2;

	/* stop CAN controller */
	(void)codrvCanDisable();

	/* get bittiming values */
	pBtrEntry = codrvCanGetBtrSettings(bitRate);
	if (pBtrEntry == NULL) {
		/* if Bitrate not supported */
		return(RET_DRV_WRONG_BITRATE); 
	}

	seg1 = pBtrEntry->seg1 + pBtrEntry->prop - 1; 
	seg2 = pBtrEntry->seg2 - 1; 

	/* set up timing parameters for 125kbps bus speed and sample
     * point at 87.5% (complying with CANopen recommendations):
     * fOSC = 16MHz; prescaler = 8 -> 1tq = (16MHz / 8)^-1 = 0.5µs
     * tBIT = tSYNCSEG + tSEG1 + tSEG2 = 1tq + 13tq + 2tq = 16tq = 8µs
     * fBUS = tBIT^-1 = 125kbps
    */
	pCan[MSCAN_CANBTR0] = (UNSIGNED8)(pBtrEntry->pre - 1u) | ((SJW - 1) << 6); 
	pCan[MSCAN_CANBTR1] = seg1 | (seg2 << 4);   

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

	/* leave initalization mode */
	pCan[MSCAN_CANCTL0] &= ~MSCAN_CANCTL0_INITRQ;       
      
  /* wait until Normal Mode is established    */
   while((pCan[MSCAN_CANCTL1] & MSCAN_CANCTL1_INITAK) != 0) ;
 
#ifdef POLLING
#else /* POLLING */
	/* enable receive interrupt  */
	pCan[MSCAN_CANRIER] |= MSCAN_CANRIER_RXFIE ;
	/* enalbe status/error interrupt */
	pCan[MSCAN_CANRIER] |= MSCAN_CANRIER_CSCIE  | MSCAN_CANRIER_RSTAT_MASK 
					       | MSCAN_CANRIER_TSTAT_MASK;
#endif /* POLLING */

	canEnabled = CO_TRUE;

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

	/* set INITRQ, this will also set INITAK - abort transmission */
	pCan[MSCAN_CANCTL0] |= MSCAN_CANCTL0_INITRQ;		

	/* wait for init mode to occur */
    while ((pCan[MSCAN_CANCTL1] & MSCAN_CANCTL1_INITAK) == 0u) { }
	
	canEnabled = CO_FALSE;

	return(retVal);
}


/***********************************************************************/
/**
* \brief codrvCanStartTransmission - start can transmission if not active
*
* Transmission of CAN messages should be interrupt driven.
* If a message was sent, the Transmit Interrupt is called
* and the next message can be transmitted.
* To start the transmission of the first message,
* this function is called from the stack.
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
		return(RET_DRV_ERROR);
	}

	if (transmissionIsActive == CO_FALSE)  {
#ifdef CODRV_CAN_POLLING
		/* trigger transmit interrupt */
		coCanDriverTransmitInterrupt();
#else
		/* enable/set tx interrupt */
     	pCan[MSCAN_CANTIER] |= MSCAN_CANTIER_TXEIE0;
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
		CO_CONST CO_CAN_MSG_T *pBuf		/**< pointer to data */
	)
{
RET_T retVal = RET_OK;

    /* check - Tx buffer 0 released */
	if ((pCan[MSCAN_CANTFLG] & MSCAN_CANTFLG_TXE0) == 0) {
		return RET_DRV_BUSY;
	}
	
	/* we use (only) TX buffer 0 */
    pCan[MSCAN_CANTBSEL] = MSCAN_CANTBSEL_TX0;  

	if (pBuf->canCob.extended == CO_TRUE)  {
		/* extended id */
		pCan[MSCAN_CANTXIDR0] = (UNSIGNED8)(pBuf->canCob.canId >> 21);
		pCan[MSCAN_CANTXIDR1] = (pBuf->canCob.canId >> 13) & 0xe0;
		pCan[MSCAN_CANTXIDR1] |= (pBuf->canCob.canId >> 15) & 0x07;
		pCan[MSCAN_CANTXIDR2] = (UNSIGNED8)(pBuf->canCob.canId >> 7);
		pCan[MSCAN_CANTXIDR3] = (pBuf->canCob.canId << 1) & 0xfe;

		/* set extended sign */
		pCan[MSCAN_CANTXIDR1] |= (MSCAN_IDR1_IDE_EXT | MSCAN_IDR1_RTR_BASE);

		/* rtr ? */
		if (pBuf->canCob.rtr == CO_TRUE)  {
			pCan[MSCAN_CANTXIDR3] |= MSCAN_IDR3_RTR_EXT;
		}
	} else {

		/* set base CAN id */
		pCan[MSCAN_CANTXIDR0] = (UNSIGNED8)(pBuf->canCob.canId >> 3);
		pCan[MSCAN_CANTXIDR1] = (UNSIGNED8)(pBuf->canCob.canId << 5);

		/* rtr ? */
		if (pBuf->canCob.rtr == CO_TRUE)  {
			pCan[MSCAN_CANTXIDR1] |= MSCAN_IDR1_RTR_BASE;
		}
	}

	if (pBuf->canCob.rtr == CO_FALSE)  {
		/* copy data byte by byte */
		pCan[MSCAN_CANTXDSR0] = pBuf->data[0];
		pCan[MSCAN_CANTXDSR1] = pBuf->data[1];
		pCan[MSCAN_CANTXDSR2] = pBuf->data[2];
		pCan[MSCAN_CANTXDSR3] = pBuf->data[3];
		if (pBuf->len > 4)  {
			pCan[MSCAN_CANTXDSR4] = pBuf->data[4];
			pCan[MSCAN_CANTXDSR5] = pBuf->data[5];
			pCan[MSCAN_CANTXDSR6] = pBuf->data[6];
			pCan[MSCAN_CANTXDSR7] = pBuf->data[7];
		}
	}

	/* set data length code */
	pCan[MSCAN_CANTXDLR] = pBuf->len;		

	/* initiate transmission*/    
	pCan[MSCAN_CANTFLG]  = MSCAN_CANTFLG_TXE0;			

	transmissionIsActive = CO_TRUE;

#ifdef CODRV_CAN_POLLING
#else
	/* enable/set tx interrupt */
	pCan[MSCAN_CANTIER] |= MSCAN_CANTIER_TXEIE0;
#endif
	
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
#ifdef CODRV_CAN_POLLING
#else
     /* disable CAN TX interrupt */
     pCan[MSCAN_CANTIER] &= ~MSCAN_CANTIER_TXEIE0;

#endif

    /* check - Tx buffer 0 transmitted?  */
	if ((pCan[MSCAN_CANTFLG] & MSCAN_CANTFLG_TXE0) == 0u) {
		return;
	}


	/* transmission was realised */
	transmissionIsActive = CO_FALSE;

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

    return;
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
*
* \return void
*/
void codrvCanReceiveInterrupt(
		void
	)
{
CO_CAN_MSG_T *pRecBuf;
BOOL_T extended = CO_FALSE;
BOOL_T rtr      = CO_FALSE;

    /* check if there has been something received */
    if ((pCan[MSCAN_CANRFLG] & MSCAN_CANRFLG_RXF) == 0)  {
	   return;
    }
	
	/* get empty receive buffer from queue */
	pRecBuf = coQueueGetReceiveBuffer();
    if (pRecBuf == NULL)  {
		/* clear RX flag */
    	pCan[MSCAN_CANRFLG] = MSCAN_CANRFLG_RXF;   
        return;
    }

	/* set length information */
    pRecBuf->len = pCan[MSCAN_CANRXDLR] & MSCAN_DLR_DLC_MASK;


	/* ext/std id ? */
	if ((pCan[MSCAN_CANRXIDR1] & MSCAN_IDR1_IDE_EXT) != 0u)  {
		pRecBuf->canCob.extended = CO_TRUE;
		/* extended id - check RTR */
		if ((pCan[MSCAN_CANRXIDR3] & MSCAN_IDR3_RTR_EXT) != 0u)  {
			/* RTR */
			pRecBuf->canCob.rtr = CO_TRUE;
		} else {
			pRecBuf->canCob.rtr = CO_FALSE;
		}
 
		pRecBuf->canCob.canId = 
			  (((UNSIGNED32)pCan[MSCAN_CANRXIDR0] << 21) & 0x1fe00000)
			| (((UNSIGNED32)pCan[MSCAN_CANRXIDR1] << 13) & 0x001c0000)
			| (((UNSIGNED32)pCan[MSCAN_CANRXIDR1] << 15) & 0x00038000)
			| (((UNSIGNED32)pCan[MSCAN_CANRXIDR2] <<  7) & 0x00007f80)
			| (((UNSIGNED32)pCan[MSCAN_CANRXIDR3] >>  1) & 0x0000007f);
    
	} else {
		/* standard id */
		pRecBuf->canCob.extended = CO_FALSE;
		if ((pCan[MSCAN_CANRXIDR1] & MSCAN_IDR1_RTR_BASE) != 0)  {
			/* RTR */
			pRecBuf->canCob.rtr = CO_TRUE;
		} else {
			pRecBuf->canCob.rtr = CO_FALSE;
		}
		/* copy ID */
		pRecBuf->canCob.canId = ((UNSIGNED16)pCan[MSCAN_CANRXIDR0] << 3) 
									| ((UNSIGNED16)pCan[MSCAN_CANRXIDR1] >> 5);
	}

	if (pRecBuf->canCob.rtr == CO_FALSE)  {
    
		/* copy all data */
		pRecBuf->data[0] = pCan[MSCAN_CANRXDSR0];
		pRecBuf->data[1] = pCan[MSCAN_CANRXDSR1];
		pRecBuf->data[2] = pCan[MSCAN_CANRXDSR2];
		pRecBuf->data[3] = pCan[MSCAN_CANRXDSR3];
		if (pRecBuf->len > 4)  {
			pRecBuf->data[4] = pCan[MSCAN_CANRXDSR4];
			pRecBuf->data[5] = pCan[MSCAN_CANRXDSR5];
			pRecBuf->data[6] = pCan[MSCAN_CANRXDSR6];
			pRecBuf->data[7] = pCan[MSCAN_CANRXDSR7];
		}
	}

    /* clear RX flag */
    pCan[MSCAN_CANRFLG] = MSCAN_CANRFLG_RXF;      
	
	/* set buffer filled */
	coQueueReceiveBufferIsFilled();

	/* inform stack about new data */
	coCommTaskSet(CO_COMMTASK_EVENT_MSG_AVAIL);

	/* signal received message */
	CO_OS_SIGNAL_CAN_RECEIVE();
}


/***********************************************************************/
/**
* \brief codrvCanErrorInterrupt - can driver error interrupt
*
* \return void
*
*/

void codrvCanErrorInterrupt(
		void
	)
{
CAN_ERROR_FLAGS_T       *pError;
UNSIGNED8 errorReg;

	pError = codrvCanErrorGetFlags();

	errorReg = pCan[MSCAN_CANRFLG];

	if ((errorReg & MSCAN_CANRFLG_OVRIF) != 0) {
		/* CAN OVERRUN */
		pError->canErrorRxOverrun = CO_TRUE;
	}

    /* check for error */
	if ((errorReg & MSCAN_CANRFLG_CSCIF) != 0) {
		if ((errorReg & MSCAN_CANRFLG_BUSOFF) == MSCAN_CANRFLG_BUSOFF) {
			/* CAN BUS OFF */
			pError->canErrorBusoff = CO_TRUE;
		} else
		if ((errorReg & MSCAN_CANRFLG_RX_PASSIVE) != 0) {
			/* CAN error passive */
			pError->canErrorPassive = CO_TRUE;
		} else
		if ((errorReg & MSCAN_CANRFLG_TX_PASSIVE) != 0) {
			/* CAN error passive */
			pError->canErrorPassive = CO_TRUE;
		}
	}

	/* reset all errors */
	pCan[MSCAN_CANRFLG] = errorReg;
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
UNSIGNED8 errorReg;

	pError = codrvCanErrorGetFlags();
	errorReg = pCan[MSCAN_CANRFLG];

	/* check for error */
	if ((errorReg & MSCAN_CANRFLG_BUSOFF) == MSCAN_CANRFLG_BUSOFF) {
	   /* CAN BUS OFF */
		pError->canNewState = Error_Busoff;
	} else  {
		if (((errorReg & MSCAN_CANRFLG_RX_PASSIVE) != 0)
		 || ((errorReg & MSCAN_CANRFLG_TX_PASSIVE) != 0)) {
		   /* CAN error passive */
  			pError->canNewState = Error_Passive;
		} else {
			/* error active */
			pError->canNewState = Error_Active;
	 	}
	}

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
		void	/* no parameter */
	)
{
	/* check current state */
	codrvCanErrorHandler();

	/* inform stack about the state changes during two handler calls */
	(void)codrvCanErrorInformStack();

#ifdef POLLING
	codrvCanReceiveInterrupt();
	codrvCanTransmitInterrupt();
#endif

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

