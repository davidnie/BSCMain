/*
* usr_ind.c - contains user indication functions
*
* Copyright (c) 2012-2014 emtas GmbH
*-------------------------------------------------------------------
* $Id: usr_ind.c 8218 2015-01-16 14:58:36Z boe $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \file
* \brief user indications
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include "gen_define.h"
#include <eb_energybus.h>

#ifdef BOOT
# include <fwupdate.h>
#endif

#include "battery.h"


/* constant definitions
---------------------------------------------------------------------------*/

/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
--------------------------------------------------------------------------*/

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


/***************************************************************************/
/**
* \brief indNmt - NMT indication
*
* \param state
    local nmt state
* \results
*	nothing
*/
void indNmt(
		UNSIGNED8	state
	)
{
	//printf("EB NMT state %s reached\n", ebDbgGetNmtStrg(state));
}


/***************************************************************************/
/**
* \brief indEbcMon - EbcMon indication
*
* \param
*	ebc=1, ext charger=0
*	ok = 1, not ok = 0
* \results
*	nothing
*/
void indEbcMon(
		UNSIGNED8 ebc,
		UNSIGNED8 active
	)
{
	if (ebc == 1) {
		if (active == 1)  {
			//printf("EB ind ebc: on\n");
		} else {
			//printf("EB ind ebc: off\n");
		}
	} else {
		if (ebc == 127)  {
			if (active == 1)  {
				//printf("EB ind externer Charger: on\n");
			} else {
				//printf("EB ind externer Charger: off\n");
			}
		}
	}
}


/***************************************************************************/
/**
* \brief indState - eb state changed
*
* \param
*	new state
* \results
*	nothing
*/
void indEbState(
		UNSIGNED16	state,
		UNSIGNED8 vdnNr
	)
{
	//printf("EB state changed VDN %d: %s\n", vdnNr,			ebDbgGetStateStrg(state, EB_DEV_TYPE_BATTERY));
}


/***************************************************************************/
/**
* \brief indEbCheckState - check new eb state
*
* \param
*	new state
* \results
*	state change allowed
*/
BOOL_T indEbCheckState(
		UNSIGNED16	state,
		UNSIGNED8 vdnNr		/**< virtual device number */
	)
{
BOOL_T	stateAllowed = CO_TRUE;

	//printf("EB check state VDN %d for: %s - allowed\n", vdnNr,	ebDbgGetStateStrg(state, EB_DEV_TYPE_BATTERY));

	return(stateAllowed);
}


/***************************************************************************/
/**
* \brief indBatState - new battery state
*
* \param
*	new state
* \results
*	none
*/
void indBatState(
		UNSIGNED16	state,
		UNSIGNED8 vdn
	)
{
	//printf("EB battery state VDN %d: %s\n", vdn,			ebDbgGetStateStrg(state, EB_DEV_TYPE_BATTERY));
}


/***************************************************************************/
/**
* \brief indBatCheckState - check new battery state
*
* \param
*	new state
* \results
*	state change allowed
*/
BOOL_T indBatCheckState(
		UNSIGNED16	state,
		UNSIGNED8 vdn
	)
{
BOOL_T	stateAllowed = CO_TRUE;

	//printf("EB battery check state VDN %d: %s - allowed\n", vdn,			ebDbgGetStateStrg(state, EB_DEV_TYPE_BATTERY));

	return(stateAllowed);
}


/***************************************************************************/
/**
* \brief indSdoWrite - sdo write ind
*
* \param
*	new state
* \results
*	state change allowed
*/
void indSdoWrite(
		UNSIGNED16	index,
		UNSIGNED8	subIndex
	)
{
#ifdef BOOT
UNSIGNED8 u8tmp;
RET_T retVal;
#endif

	//printf("EB SDO WRITE %x:%d\n", index, subIndex);

#ifdef BOOT
	/*
	 * call bootloader after 'stop program' command
	 * write '0' to 0x1F51:1
	 * additional you can check passwords and so on
	 */
	if((index == 0x1F51) && (subIndex == 1)) {
		retVal = coOdGetObj_u8(0x1F51, 1, &u8tmp);
		if (retVal != RET_OK) {
			return;
		}

		if (u8tmp != 0 /* Stop program */) {
			return;
		}

	 	jump2Bootloader();
	}
#endif
}



/***************************************************************************/
/**
* \brief indCanState
*
* \param event
*	CAN Bus event
*/
void indCanState(
		CO_CAN_STATE_T event
	)
{
	//printf("CAN State: %s\n", ebDbgGetCanStateStrg(event));
}


/***************************************************************************/
/**
* \brief indErrorHandler - Error ind
*
* \param event
*	error event
*/
void indErrorHandler(
		CO_COMM_STATE_EVENT_T event
	)
{
	//printf("EB Comminication state change: %s\n", ebDbgGetCommStateStrg(event));
}


#ifdef EB_NETWORK_SECURITY_CNT
/***************************************************************************/
/**
* \brief indSecurity - security event
*
* \param
*	new event
* \results
*	event was executed
*/
BOOL_T indSecurity(
		EB_SEC_EVENT_T	event,
		UNSIGNED16		devType,
		UNSIGNED8		vdn
	)
{
//printf("indSecurity event %x\n", event);
	switch (event)  {
		case EB_SEC_EVENT_LOCK:
//printf("******* BATTERY LOCKED *****\n");
			/* if it takes some time, call the function later */
			ebSecLocked(vdn, CO_TRUE);

			break;

		case EB_SEC_EVENT_UNLOCK:
//printf("******* BATTERY UNLOCKED *****\n");
			/* if it takes some time, call the function later */
			ebSecLocked(vdn, CO_FALSE);

			break;

		default:
			break;
	}

	return(CO_TRUE);
}
#endif /* EB_NETWORK_SECURITY_CNT */


/***************************************************************************/
/**
* \brief
*
* \internal
*
*/
EB_RET_T eepromWriteInd(
		EB_EEPROM_DATA_T type,
		UNSIGNED16 number,
		UNSIGNED16 len,
		CO_CONST UNSIGNED8 *src
	)
{
#ifdef TARGET_LINUX
FILE	*fd;
char	*fname;
UNSIGNED16	offs;

	switch (type)  {
		case EB_EEPROM_SECURITY_KEYS:
			fname = "eeprom_sec_keys.bin";
			offs = len * number;
			break;
		default:
			return(EB_RET_ERROR);
	}

	//printf("** eepromWriteInd: load %s, num: %d offs: 0x%x, len: %d\n", fname, number, offs, len);

	fd = fopen(fname, "r+b");
	if (fd == NULL)  {
		fd = fopen(fname, "wb");
		if (fd == NULL)  {
			return(EB_RET_ERROR);
		}
		fwrite(src, 1, 512, fd);
		fclose(fd);
	}
	fd = fopen(fname, "r+b");
	fseek(fd, offs, SEEK_SET);
	fwrite(src, 1, len, fd);
	fclose(fd);
#endif /* TARGET_LINUX */

	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \brief
*
* \internal
*
*/
EB_RET_T eepromReadInd(
		EB_EEPROM_DATA_T type,
		UNSIGNED16 number,
		UNSIGNED16 len,
		UNSIGNED8 *dst
	)
{
#ifdef TARGET_LINUX
FILE	*fd;
char	*fname;
UNSIGNED16	offs;

	switch (type)  {
		case EB_EEPROM_SECURITY_KEYS:
			fname = "eeprom_sec_keys.bin";
			offs = len * number;
			break;
		default:
			return(EB_RET_ERROR);
	}


	fd = fopen(fname, "rb");
	if (fd == NULL)  {
		return(EB_RET_ERROR);
	}

	fseek(fd, offs, SEEK_SET);
	fread(dst, 1, len, fd);
	fclose(fd);
#endif /* TARGET_LINUX */

	return(EB_RET_OK);
}




#ifdef CO_SLEEP_454
/***************************************************************************/
/**
* \brief iebSleepInd
*
* \internal
*
*/
UNSIGNED8 sleepInd(
		CO_SLEEP_MODE_T	mode
	)
{
return 1;
	

	switch (mode)  {
		case CO_SLEEP_MODE_CHECK:
			break;
		case CO_SLEEP_MODE_PREPARE:
			break;
		case CO_SLEEP_MODE_SILENT:
			break;
		case CO_SLEEP_MODE_DOZE:

/* disable CAN */
/* disable timer */

#ifdef TARGET_LINUX
/* works only for can4linux, not for socketCan */
{
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <can4linux.h>

int canfd;
canmsg_t msg;
int ret = -1;

	/* open std can device */
	canfd = open(C4L_DEVICE, O_RDONLY);
	/* wait until next message */
	while (ret < 0)  {
		ret = read(canfd, &msg, 1);
	}
	close(canfd);
}
#endif
			/* start wakeup */
			/* init CAN */

			ebSleepAwake(); /* wakeup command */

			break;
		default:
	}

	return(0);
}
#endif /* CO_SLEEP_454 */
