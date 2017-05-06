/*
* co_cobhandler.c - contains functions for cob handling
*
* Copyright (c) 2012-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_cobhandler.c 12751 2016-03-10 09:45:22Z phi $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief Functions for COB handling
*
* \file co_cobhandler.c contains functions for cob handling
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stddef.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>
#include <co_datatype.h>
#include <co_timer.h>
#include <co_drv.h>
#include "ico_cobhandler.h"
#include "ico_queue.h"

/* constant definitions
---------------------------------------------------------------------------*/

/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
#ifdef CO_DRV_FILTER
static RET_T icoCobFilterSet(COB_REFERENZ_T cobRef);
#endif /* CO_DRV_FILTER */

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static COB_REFERENZ_T	cobRefCnt = { 0u };
static CO_COB_T			cobData[CO_COB_CNT];
#ifdef CO_COB_SORTED_LIST
static COB_REFERENZ_T	cobSortedList[CO_COB_CNT];
#endif /* CO_COB_SORTED_LIST */


#ifdef CO_DRV_FILTER
/***************************************************************************/
/**
*
* \internal
*
* \brief icoCobFilterSet - configure filter
*
* \return RET_T
*
*/
static RET_T icoCobFilterSet(
		COB_REFERENZ_T	cobRef
	)
{
RET_T retval = RET_OK;
BOOL_T	enable;

		/* special case: change to TRANSMIT without RTR
	 	 * disable it temporary, call filter function, enable it again
	 	*/
	if ((cobData[cobRef].type == CO_COB_TYPE_TRANSMIT)
	 	&& (cobData[cobRef].canCob.rtr == CO_FALSE))  
	{
		/* save cob enable state */
		enable = cobData[cobRef].canCob.enabled;
		cobData[cobRef].canCob.enabled = CO_FALSE;
		/* inform driver - disable filter */
		retval = codrvCanSetFilter(&cobData[cobRef].canCob);
		/* restore enable state */
		cobData[cobRef].canCob.enabled = enable;
	} else 
		/* special case: change to Receive with RTR Request
	 	 * -> receive data frames
	 	*/

	if ((cobData[cobRef].type == CO_COB_TYPE_RECEIVE)
	 	&& (cobData[cobRef].canCob.rtr == CO_TRUE))  
	{	
		/* receive data frame */
		cobData[cobRef].canCob.rtr = CO_FALSE;
		retval = codrvCanSetFilter(&cobData[cobRef].canCob);
		cobData[cobRef].canCob.rtr = CO_TRUE; /* restore old value */

	} else
	{
		/* inform driver */
		retval = codrvCanSetFilter(&cobData[cobRef].canCob);
	}

	return retval;
}
#endif /* CO_DRV_FILTER */


/***************************************************************************/
/**
*
* \internal
*
* \brief icoCobCreate
*
* \return COB_REFERENZ_T (not line depending!)
*
*/
COB_REFERENZ_T icoCobCreate(
		CO_COB_TYPE_T	cobType,			/* cob type */
		CO_SERVICE_T	service,			/* service type */
		UNSIGNED16		serviceNr			/* service number */
	)
{
COB_REFERENZ_T	cobRef;

	/* cob(s) available ? */
	if (cobRefCnt >= CO_COB_CNT)  {
		return(0xffffu);
	}

	cobRef = cobRefCnt;
	cobRefCnt++;

	/* fill into cob structure */
	cobData[cobRef].cobNr = cobRef + 1u;
	cobData[cobRef].type = cobType;
	cobData[cobRef].canCob.enabled = CO_FALSE;
	cobData[cobRef].canCob.canId = CO_COB_INVALID;
	cobData[cobRef].canCob.ignore = 0u;
	cobData[cobRef].canCob.canChan = 0xFFFFu;
	cobData[cobRef].service = service;
	cobData[cobRef].serviceNr = serviceNr;
	cobData[cobRef].inhibit = 0u;

	return(cobRef);
}


/***************************************************************************/
/**
*
* \internal
*
* \brief icoCobSet - set cob data 
*
*
* \return RET_T
*
*/
RET_T icoCobSet(
		COB_REFERENZ_T	cobRef,			/* cob reference */
		UNSIGNED32		cobId,			/* cob-id */
		CO_COB_RTR_T	rtr,			/* rtr flag */
		UNSIGNED8		len				/* data len */
	)
{
CO_COB_T	*pCob;

	if (cobRef > (cobRefCnt))  {
		return(RET_NO_COB_AVAILABLE);
	}

	pCob = &cobData[cobRef];
	if ((cobId & CO_COB_VALID_MASK) == CO_COB_INVALID)  {
		pCob->canCob.enabled = CO_FALSE;
	} else {

		if (len > CO_CAN_MAX_DATA_LEN)  {
			return(RET_SDO_INVALID_VALUE);
		}
		pCob->canCob.enabled = CO_TRUE;
	}

	pCob->canCob.canId = cobId & CO_COB_ID_MASK;

	if ((cobId & CO_COB_29BIT_MASK) == CO_COB_29BIT)  {
		pCob->canCob.extended = CO_TRUE;
	} else {
		pCob->canCob.extended = CO_FALSE;
	}

	if (rtr == CO_COB_RTR_NONE)  {
		pCob->canCob.rtr = CO_FALSE;
	} else {
		pCob->canCob.rtr = CO_TRUE;
	}

	pCob->len = len;

#ifdef CO_DRV_FILTER
	{
	RET_T retval;

		retval = icoCobFilterSet(cobRef);
		if (retval != RET_OK) {
			return retval;
		}
	}
#endif /* CO_DRV_FILTER */

#ifdef CO_COB_SORTED_LIST
	{
	UNSIGNED16 cobCnt;
	UNSIGNED16 cnt = 0u;
	UNSIGNED16 tmpCobNbr = 0u;// CO_COB_T* pTmpCob = NULL;
		cobCnt = CO_COB_CNT;
		cnt = 0u;
		/* First see if cob already used */
		for (cnt; cnt < cobCnt; cnt++) {
			if (cobSortedList[cnt] == pCob->cobNr) {
				/* yes already used */
				break;
			}
		}
		/* if already used delete it */
		for (cnt; cnt < cobCnt; cnt++) {
			if ((cnt + 1u) == cobCnt) {
				cobSortedList[cnt] = 0u;
			} else {
				cobSortedList[cnt] = cobSortedList[cnt + 1u];
			}
		}
		cnt = 0u;
		/* put new cob in sorted list */
		for (cnt; cnt < cobCnt; cnt++ ) {
			if (cobSortedList[cnt] == 0u) {
				cobSortedList[cnt] = pCob->cobNr;
				break;
			}

			if (pCob->canCob.canId < cobData[cobSortedList[cnt] - 1u].canCob.canId) {
				tmpCobNbr = cobSortedList[cnt];
				cobSortedList[cnt] = pCob->cobNr;
				pCob = &cobData[tmpCobNbr - 1u];
			}
		}
	}
#endif /* CO_COB_SORTED_LIST */
	return(RET_OK);
}


/******************************************************************/
/**
*
* \internal
*
* \brief icoCobChangeType - change cob type
*
*
* \return RET_T
*
*/
RET_T icoCobChangeType(
		COB_REFERENZ_T	cobRef,			/* cob reference */
		CO_COB_TYPE_T	cobType			/* new cob type */
	)
{
	if (cobRef > (cobRefCnt))  {
		return(RET_NO_COB_AVAILABLE);
	}

	cobData[cobRef].type = cobType;

#ifdef CO_DRV_FILTER
	{
	RET_T retval;

		retval = icoCobFilterSet(cobRef);
		if (retval != RET_OK) {
			return retval;
		}
	}
#endif /* CO_DRV_FILTER */

	return(RET_OK);
}


/******************************************************************/
/**
*
* \internal
*
* \brief icoCobSetInhibit - set inhibit time
*
*
* \return RET_T
*
*/
RET_T icoCobSetInhibit(
		COB_REFERENZ_T	cobRef,			/* cob reference */
		UNSIGNED16	inhibit				/* inhibit time */
	)
{
	if (cobRef > (cobRefCnt))  {
		return(RET_NO_COB_AVAILABLE);
	}

	cobData[cobRef].inhibit = inhibit;

	/* delete old messages from inhibit list if new time = 0 */
	if (inhibit == 0u)  {
		icoQueueDeleteInhibit(cobRef);
	}

	return(RET_OK);
}


RET_T icoCobSetIgnore(
	COB_REFERENZ_T	cobRef,			/* cob reference */
	UNSIGNED32		mask			/* ignore mask */
	)
{
	if (cobRef > (cobRefCnt)) {
		return(RET_NO_COB_AVAILABLE);
	}

	cobData[cobRef].canCob.ignore = mask;

	return(RET_OK);
}


#ifdef CO_CANFD_SUPPORTED
/***************************************************************************/
/**
*
* \internal
*
* \brief icoCobSetLen - update len information for COB
*
*
* \return RET_T
*
*/
RET_T icoCobSetLen(
		COB_REFERENZ_T	cobRef,			/* cob reference */
		UNSIGNED8		len				/* data len */
	)
{
	if (cobRef > (cobRefCnt))  {
		return(RET_NO_COB_AVAILABLE);
	}

	if (len > CO_CAN_MAX_DATA_LEN)  {
		return(RET_SDO_INVALID_VALUE);
	}

	cobData[cobRef].len = len;

	return(RET_OK);
}
#endif /* CO_CANFD_SUPPORTED */


/******************************************************************/
/**
* \internal
*
* \brief icoCobGet - get a cob rerenced by cobref
*
*
* \return cob
*
*/
UNSIGNED32 icoCobGet(
		COB_REFERENZ_T	cobRef			/* cob reference */
	)
{
UNSIGNED32	cobId;

	if (cobRef > (cobRefCnt))  {
		return(0ul);
	}

	cobId = cobData[cobRef].canCob.canId;
	if (cobData[cobRef].canCob.extended == CO_TRUE)  {
		cobId |= CO_COB_29BIT;
	}
	if (cobData[cobRef].canCob.enabled == CO_FALSE)  {
		cobId |= CO_COB_INVALID;
	}

	return(cobId);
}


/******************************************************************/
/**
*
* \internal
*
* \brief icoCobEnable - enable a cob
*
*
* \return RET_T
*
*/
RET_T icoCobEnable(
		COB_REFERENZ_T	cobRef			/* cob reference */
	)
{
	if (cobRef > (cobRefCnt))  {
		return(RET_NO_COB_AVAILABLE);
	}
	cobData[cobRef].canCob.enabled = CO_TRUE;

#ifdef CO_DRV_FILTER
	{
	RET_T retval;

		retval = icoCobFilterSet(cobRef);
		if (retval != RET_OK) {
			return retval;
		}
	}
#endif /* CO_DRV_FILTER */

	return(RET_OK);
}


/******************************************************************/
/**
*
* \internal
*
* \brief icoCobDisable - disable a cob
*
*
* \return RET_T
*
*/
RET_T icoCobDisable(
		COB_REFERENZ_T	cobRef			/* cob reference */
	)
{
	if (cobRef > (cobRefCnt))  {
		return(RET_NO_COB_AVAILABLE);
	}

	cobData[cobRef].canCob.enabled = CO_FALSE;

#ifdef CO_DRV_FILTER
	{
	RET_T retval;

		retval = icoCobFilterSet(cobRef);
		if (retval != RET_OK) {
			return retval;
		}
	}
#endif /* CO_DRV_FILTER */

	return(RET_OK);
}


/******************************************************************/
/**
* \internal
*
* \brief icoCobGetPointer - get a pointer to cob
*
*
* \return CO_COB_T
*	pointer to cob data
*/
CO_COB_T	*icoCobGetPointer(
		COB_REFERENZ_T	cobRef			/* cob reference */
	)
{
	if (cobRef > (cobRefCnt))  {
		return(NULL);
	}

	return(&cobData[cobRef]);
}


/******************************************************************/
/**
*
* \internal
*
* \brief icoCobCheck - get cob structure for given can id
*
*
* \return RET_T
*
*/
CO_COB_T	*icoCobCheck(
		const CO_CAN_COB_T	*pCanCob		/* can cob data */
	)
{
CO_COB_T	*pCob = NULL;
UNSIGNED32	mask;

#ifdef CO_COB_SORTED_LIST
UNSIGNED16	left = 0u;
UNSIGNED16	right = cobRefCnt - 1u;
UNSIGNED16	middle = 0u;
BOOL_T		found = CO_FALSE;
BOOL_T		breakLoop = CO_FALSE;

	while (left <= right) {
		/* l + r / 2 */
		middle = (left + right) >> 1;
		pCob = &cobData[cobSortedList[middle] - 1u];
		if ((pCanCob->canId == pCob->canCob.canId)
			&& (pCanCob->extended == pCob->canCob.extended)
			&& (CO_TRUE == pCob->canCob.enabled)) {
			/* receive cob or transmit_rtr */
			if ((CO_COB_TYPE_RECEIVE == pCob->type)
				|| ((CO_COB_TYPE_TRANSMIT == pCob->type)
					&& (pCanCob->rtr == CO_TRUE)
					&& (pCanCob->rtr == pCob->canCob.rtr))) {
				/*lint !e960 Note: pointer arithmetic other than array indexing used */
				found = CO_TRUE;
				breakLoop = CO_TRUE;	/* break; */
			}
		}
		if (pCob->canCob.canId > pCanCob->canId) { /*lint !e960 Note: pointer arithmetic other than array indexing used */
			if (middle == 0u) {
				breakLoop = CO_TRUE;	/* break; */
			}
			right = middle - 1u;
		}
		else {
			left = middle + 1u;
		}
		/* finish loop ? */
		if (breakLoop == CO_TRUE) {
			break;
		}
	}

	if (found == CO_TRUE) {
		return(pCob);
	}

#else /* CO_COB_SORTED_LIST */
UNSIGNED16	cnt;

	for (cnt = 0u; cnt < cobRefCnt; cnt++)  {
		pCob = &cobData[cnt];
		mask = pCob->canCob.ignore;

		if (((pCanCob->canId & ~mask) == (pCob->canCob.canId & ~mask) ) 
		 && (pCanCob->extended == pCob->canCob.extended)
		 && (CO_TRUE == pCob->canCob.enabled))  {
			/* receive cob or transmit_rtr */
			if ((CO_COB_TYPE_RECEIVE == pCob->type) 
			 || (  (CO_COB_TYPE_TRANSMIT == pCob->type)
			 	&& (pCanCob->rtr == CO_TRUE)
			 	&& (pCanCob->rtr == pCob->canCob.rtr)))  {
				return(pCob);
			}
		}
	}
#endif /* CO_COB_SORTED_LIST */
	return(NULL);
}


/******************************************************************/
/**
*
* \internal
*
* \brief icoCobDisableAll - disable all cobs
*
*
* \return none
*
*/
void icoCobDisableAll(
		void	/* no parameter */
	)
{
COB_REFERENZ_T	i;

	for (i = 0u; i < cobRefCnt; i++)  {
		(void)icoCobDisable(i);
	}
}


#ifdef CO_CANFD_SUPPORTED
/******************************************************************/
/**
*
* \internal
*
* \brief icoGetCanMsgLen - return msg len from DLC
*
*
* \return message len in bytes
*
*/
UNSIGNED8 icoGetCanMsgLen(
		UNSIGNED8	dlc			/* DLC */
	)
{
UNSIGNED8	len;

	if (dlc < 9)  {
		return(dlc);
	}

	switch (dlc)  {
		case 9:	 len = 12; break;
		case 10: len = 16; break;
		case 11: len = 20; break;
		case 12: len = 24; break;
		case 13: len = 32; break;
		case 14: len = 48; break;
		case 15: len = 64; break;
		default: break;
	}

	return(len);
}


/******************************************************************/
/**
*
* \internal
*
* \brief icoGetCanTransferLen - return used transfer len for CAN-FD message
*
*
* \return used transfer len in bytes
*
*/
UNSIGNED8 icoGetCanTransferLen(
		UNSIGNED8	len				/* number of bytes */
	)
{
UNSIGNED8	i;
static const UNSIGNED8	t[7] = { 12, 16, 20, 24, 32, 48, 64 };  
UNSIGNED8	transferLen = len;

	if (len > 8)  {
		i = 0;
		while (len > t[i])  {
			i++;
		}
		transferLen = t[i];
	}

	return(transferLen);
}
#endif /* CO_CANFD_SUPPORTED */


/******************************************************************/
/**
*
* \internal
*
* \brief icoCheckRestrictedCobs
*
*
* \return BOOL_T
*
*/
BOOL_T icoCheckRestrictedCobs(
		UNSIGNED32	canId,			/* new cobid */
		UNSIGNED32	exceptFirst,	/* allow cobid range from */
		UNSIGNED32	exceptLast		/* allow cobid range to */
	)
{
typedef struct {
	UNSIGNED32	first;
	UNSIGNED32	last;
} CO_NV_STORAGE RESTRICTED_COBS_T;
static const RESTRICTED_COBS_T	restrictCobs[] = {
	{ 0x000u,	0x000u },
	{ 0x001u,	0x07fu },
	{ 0x101u,	0x180u },
	{ 0x581u,	0x5ffu },
	{ 0x601u,	0x67fu },
	{ 0x6e0u,	0x6ffu },
	{ 0x701u,	0x77fu },
	{ 0x780u,	0x7ffu },
};
UNSIGNED8	i;

	for (i = 0u; i < (sizeof(restrictCobs) / sizeof(RESTRICTED_COBS_T)); i++)  {
		if ((canId >= restrictCobs[i].first)
		 && (canId <= restrictCobs[i].last))  {
			if ((canId < exceptFirst) || (canId > exceptLast))  {
				return(CO_TRUE);
			}
		}
	}

	return(CO_FALSE);
}


/******************************************************************/
/**
*
* \internal
*
* \brief icoCobHandlerInit - init cob handler variables
*
*
* \return BOOL_T
*
*/
void icoCobHandlerVarInit(
	)
{

	{
		cobRefCnt = 0u;

	}
#ifdef CO_COB_SORTED_LIST
	{
	UNSIGNED16		cnt = 0u;
		for (cnt = 0; cnt < CO_COB_CNT; cnt++) {
			cobSortedList[cnt] = 0u;
		}
	}
#endif /* CO_COB_SORTED_LIST */
}
