/*
* co_time.h - contains defines for time services
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_time.h 10342 2015-08-24 13:53:52Z boe $
*-------------------------------------------------------------------
*
* Changelog:
*
*
*/

/**
* \brief defines for time services
*
* \file co_time.h - contains defines for time services
*
*/

#ifndef CO_TIME_H
#define CO_TIME_H 1

#include <co_datatype.h>


/* constant */
#define CO_TIME_PRODUCER_BIT	0x40000000ul
#define CO_TIME_CONSUMER_BIT	0x80000000ul


/**
* TIME_OF_DAY structure
*/
typedef struct {
	UNSIGNED32	msec;			/**< milliseconds after midnight */
	UNSIGNED16	days;			/**< days after 1st january of 1984 */
} CO_TIME_T;


/** \brief function pointer to time function 
 *
 * \param pTime - time of day structure
 * 
 * \return void
 */
typedef void (* CO_EVENT_TIME_T)(CO_TIME_T *pTime);


/* function prototypes */
EXTERN_DECL RET_T coTimeWriteReq(CO_TIME_T const *pTimeData);
EXTERN_DECL RET_T coTimeInit(BOOL_T producer, BOOL_T consumer);
EXTERN_DECL RET_T coEventRegister_TIME(CO_EVENT_TIME_T pFunction);

#endif /* CO_TIME_H */
