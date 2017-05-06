/*
* co_led.h - contains defines for nmt services
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_led.h 15304 2016-09-22 15:28:37Z boe $
*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for usage of LED CiA 303
*
* \file co_led.h - contains defines for usage of LED CiA 303
*
*/

#ifndef CO_LED_H
#define CO_LED_H 1

#include <co_datatype.h>


/* datatypes */

/**
* LED states
*/
typedef enum {
	CO_LED_STATE_OFF = 0,			/**< led is off */
	CO_LED_STATE_FLICKERING = 1,	/**< led is flickering */
	CO_LED_STATE_FLASH_1 = 2,		/**< led is flashing mode 1 */
	CO_LED_STATE_FLASH_2 = 3,		/**< led is flashing mode 2 */
	CO_LED_STATE_FLASH_3 = 4,		/**< led is flashing mode 3 */
	CO_LED_STATE_BLINKING = 5,		/**< led is blinking */
	CO_LED_STATE_ON = 6				/**< led is on */
} CO_LED_STATE_T;


/** \brief function pointer to LED indication function 
 * \param led_state - set led on/off
 * 
 * \return void
 */
typedef void (* CO_EVENT_LED_T)(BOOL_T); /*lint !e960 customer specific parameter names */


/* function prototypes */

EXTERN_DECL void coLedInit(void);
EXTERN_DECL void coLedSetGreen(CO_LED_STATE_T newLedState);
EXTERN_DECL void coLedSetRed(CO_LED_STATE_T	newLedState);
EXTERN_DECL void coLedSetState(CO_LED_STATE_T newState, BOOL_T on);
EXTERN_DECL RET_T coEventRegister_LED_GREEN(CO_EVENT_LED_T pFunction);
EXTERN_DECL RET_T coEventRegister_LED_RED(CO_EVENT_LED_T pFunction);

#endif /* CO_LED_H */
