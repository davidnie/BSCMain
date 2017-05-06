/*
* co_store.h - contains defines for store services
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_store.h 10342 2015-08-24 13:53:52Z boe $
*-------------------------------------------------------------------
*
* Changelog:
*
*
*/

/**
* \brief defines for store services
*
* \file co_store.h - contains defines for store services
*
*/

#ifndef CO_STORE_H
#define CO_STORE_H 1

#include <co_datatype.h>


/* constant */

/**
* define for store/load/restore area all
*/
#define CO_STORE_AREA_ALL	1u
/*
* define for store/load/restore area communication
*/
#define CO_STORE_AREA_COMM	2u
/*
* define for store/load/restore area application
*/
#define CO_STORE_AREA_APPL	3u


/**
* define for save command
*/
#define CO_STORE_SIGNATURE_SAVE	0x65766173ul

/**
* define for load command
*/
#define CO_STORE_SIGNATURE_LOAD	0x64616f6cul


/* datatypes */

/** \brief function pointer to save/load/clear function 
 *
 * \param subIndex - subindex parameter to point parameter area
 * 
 * \return none
 */
typedef RET_T (* CO_EVENT_STORE_T)(UNSIGNED8 subIndex);



/* function prototypes */
EXTERN_DECL RET_T	coEventRegister_LOAD_PARA(CO_EVENT_STORE_T pFunction);
EXTERN_DECL RET_T	coEventRegister_SAVE_PARA(CO_EVENT_STORE_T pFunction);
EXTERN_DECL RET_T	coEventRegister_CLEAR_PARA(CO_EVENT_STORE_T pFunction);

#endif /* CO_STORE_H */
