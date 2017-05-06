/*
* co_nmtslave.c - contains NMT slave services
*
* Copyright (c) 2016-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_nmtslave.c 14542 2016-07-15 08:48:49Z boe $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief NMT slave services
*
* \file co_nmtslave.c
* contains NMT slave services for self starting devices
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>	/*lint !e766 depends on configuration */

#ifndef CO_NMT_MASTER
# ifdef CO_SELF_STARTING_DEVICE

#include <co_datatype.h>
#include <co_drv.h>
#include <co_timer.h>
#include <co_odaccess.h>
#include <co_nmt.h>
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_nmt.h"

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

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static UNSIGNED32	nmtStartup = { 0u };


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**
* \internal
*
* \brief icoNmtGetObjectAddr - get Nmt object address
*
* \return none
*
*/
void *icoNmtGetObjectAddr(
		UNSIGNED16	index,			/* index */
		UNSIGNED8	subIndex		/* subindex */
	)
{
void	*pAddr = NULL;

	switch (index)  {
		case 0x1f80u:
			pAddr = (void *)&nmtStartup;
			break;
		default:
			break;
	}

	return(pAddr);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoNmtObjChanged - Nmt object changed
*
*
* \return RET_T
*
*/
RET_T icoNmtObjChanged(
		UNSIGNED16	index,			/* index */
		UNSIGNED8	subIndex		/* subindex */
	)
{
RET_T	retVal = RET_OK;

	switch (index)  {
		case 0x1f80:
		    if ((nmtStartup & ~CO_NMT_STARTUP_BIT_STARTITSELF) != 0u)  {
				icoNmtSetDefaultValue();
				retVal = RET_PARAMETER_INCOMPATIBLE;
			}
			break;
		default:
			break;
	}
	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoNmtSetDefaultValue - set predefined connection set
*
* \return none
*
*/
void icoNmtSetDefaultValue(
		void	/* no parameter */
	)
{
RET_T	retVal;
UNSIGNED32	u32;

	retVal = coOdGetDefaultVal_u32(0x1f80u, 0u, &u32);
	if (retVal == RET_OK)  {
		nmtStartup = u32;
	}
}

/***************************************************************************/
/**
* \internal
*
* \brief icoNmtApplyObj - test and applies the object 0x1f80
*
* \return none
*
*/
void icoNmtApplyObj(
		void	/* no parameter */
)
{
UNSIGNED32 selfStartUp;
RET_T ret = RET_OK;

	ret = coOdGetObj_u32(0x1f80, 0u, &selfStartUp);
	if (ret == RET_OK) {
		if ((selfStartUp & CO_NMT_STARTUP_BIT_STARTITSELF) == 0u) {
			icoNmtStateChange(CO_NMT_STATE_OPERATIONAL, CO_TRUE);
		}
	}
}


# endif /* CO_SELF_STARTING_DEVICE */
#endif /* !CO_NMT_MASTER */

