/*
* co_sleep.h - contains defines for sleep services
*
* Copyright (c) 2013-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_sleep.h 10342 2015-08-24 13:53:52Z boe $
*
*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for sleep services
*
* \file co_sleep.h - contains defines for sleep services
*
*/

#ifndef CO_SLEEP_H
#define CO_SLEEP_H 1

#include <co_datatype.h>


/* datatypes */

/**
* SLEEP states 
*/
typedef enum {
	CO_SLEEP_MODE_CHECK,		/**< check if sleep mode is possible */
	CO_SLEEP_MODE_OBJECTION,	/**< slave has send an objection */
	CO_SLEEP_MODE_PREPARE,		/**< automatic start sleep mode */
	CO_SLEEP_MODE_SILENT,		/**< sleep mode silent */
	CO_SLEEP_MODE_DOZE,			/**< sleep mode doze */
	CO_SLEEP_MODE_REQUEST_SLEEP	/**< sleep mode reuqest sleep */
} CO_SLEEP_MODE_T;



/** \brief function pointer to sleep event function 
 * \param sleep mode
 * \param node id
 * 
 * \retval 0 - ok
 * \retval !=0 - error
 */
typedef UNSIGNED8 (* CO_EVENT_SLEEP_T)(CO_SLEEP_MODE_T, UNSIGNED8); /*lint !e960 customer specific parameter names */



/* function prototypes */
EXTERN_DECL void	coSleepModeStart(UNSIGNED16 waitTime);
EXTERN_DECL RET_T	coEventRegister_SLEEP(CO_EVENT_SLEEP_T pFunction);
EXTERN_DECL BOOL_T	coSleepModeActive(void);
EXTERN_DECL void	coSleepModeCheck(UNSIGNED16 waitTime);
EXTERN_DECL void	coSleepAwake(BOOL_T master, UNSIGNED8 status,
					UNSIGNED8 reason, UNSIGNED16 repeatTime);
EXTERN_DECL void	coSleepWakeUp(BOOL_T master, UNSIGNED8 status,
					UNSIGNED8 reason, UNSIGNED16 repeatTime);

EXTERN_DECL void	coSleepRequestSleep(void);

#endif /* CO_SLEEP_H */
