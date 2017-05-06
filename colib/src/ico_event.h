/*
* ico_event.h - contains internal defines for event handling
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_event.h 9666 2015-06-29 13:48:37Z boe $

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

#ifndef ICO_EVENT_H
#define ICO_EVENT_H 1


/* datatypes */


/**
* event structure 
*/
typedef struct co_event {
	struct co_event	*pNext;			/**< pointer to next event */
	void			(*pFct)(void *para);/**< pointer to own function */
	void			*pData;			/**< pointer for own data */
} xEvent;
typedef struct co_event	CO_EVENT_T;


/** \brief function pointer to event indication
 * \param pFct - pointer to timer up function
 * 
 * \return void
 */
typedef void (* CO_EVENT_FCT_T)(void *); /*lint !e960 customer specific parameter names */


/* function prototypes */

void	icoEventCheck(void);
void	icoEventInit(void);
RET_T	icoEventStart(CO_EVENT_T *pEvent,
			CO_EVENT_FCT_T	ptrToFct, void *pData);
BOOL_T	icoEventIsActive(CO_CONST CO_EVENT_T *pEvent);

#endif /* ICO_EVENT_H */
