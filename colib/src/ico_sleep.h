/*
* ico_sleep.h - contains internal defines for SLEEP
*
* Copyright (c) 2013-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_sleep.h 8127 2015-01-13 06:53:49Z boe $

*-------------------------------------------------------------------
*
* Changelog:
*
*
*/

/**
* \file
* \brief dataty type header
*/

#ifndef ICO_SLEEP_H
#define ICO_SLEEP_H 1


/* datatypes */



/* function prototypes */

void	icoSleepReset(UNSIGNED8 master, UNSIGNED8 nodeId);
RET_T	icoSleepInit(UNSIGNED8 master);
void	icoSleepMsgHandler(CO_CONST CO_REC_DATA_T *pRec);
void	icoSleepVarInit(void);

#endif /* ICO_SLEEP_H */
