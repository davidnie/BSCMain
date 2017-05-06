/*
* co_timer.h - contains defines for timer
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_timer.h 10342 2015-08-24 13:53:52Z boe $
*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for timer
*
* \file co_timer.h - contains defines for timer
*/

#ifndef CO_TIMER_H
#define CO_TIMER_H 1


/* datatypes */

/**
* timer attributes
*/
typedef enum {
	CO_TIMER_ATTR_ROUNDUP,			/**< round up given timer value */
	CO_TIMER_ATTR_ROUNDUP_CYCLIC,	/**< round up and start timer again */
	CO_TIMER_ATTR_ROUNDDOWN,		/**< round down given timer value */
	CO_TIMER_ATTR_ROUNDDOWN_CYCLIC	/**< round down and start timer again */
} CO_TIMER_ATTR_T;


/** \brief function pointer to Timer indication
 * \param pFct - pointer to timer up function
 * 
 * \return void
 */
typedef void (* CO_TIMER_FCT_T)(void *); /*lint !e960 customer specific parameter names */


/**
* timer structure 
*/
typedef struct co_timer {
	struct co_timer	*pNext;			/**< pointer to next timer */
	UNSIGNED32		actTicks;		/**< actual timer ticks */
	UNSIGNED32		ticks;			/**< calculated timer ticks */
	CO_TIMER_FCT_T	pFct;			/**< pointer to own function */
	void			*pData;			/**< pointer for own data */
	CO_TIMER_ATTR_T	attr;			/**< timer attributes */
} xTimer;
typedef struct co_timer	CO_TIMER_T;


/* function prototypes */

EXTERN_DECL void	coTimerInit(UNSIGNED32 timerVal);
EXTERN_DECL RET_T	coTimerStart(CO_TIMER_T *pTimer,
				UNSIGNED32 timerTime, CO_TIMER_FCT_T pFct, void *pData,
				CO_TIMER_ATTR_T timerAttributes);
EXTERN_DECL RET_T	coTimerStop(CO_CONST CO_TIMER_T *pTimer);
EXTERN_DECL BOOL_T	coTimerIsActive(CO_CONST CO_TIMER_T *pTimer);
EXTERN_DECL void	coTimerTick(void);
EXTERN_DECL void	coTimerAttrChange(CO_TIMER_T *pTimer,
				CO_TIMER_ATTR_T timerAttributes);

#endif /* CO_TIMER_H */
