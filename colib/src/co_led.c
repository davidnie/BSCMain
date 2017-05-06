/*
* co_led.c - contains LED services
*
* Copyright (c) 2012-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_led.c 15327 2016-09-23 09:00:10Z boe $
*
*
*-------------------------------------------------------------------
*
*
*/


/********************************************************************/
/**
* \brief LED handling according CiA 303-3
*
* \file co_led.c
* contains LED handling according CiA 303-3
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
#include <co_led.h>
#include "ico_indication.h"
#include "ico_led.h"

#ifdef CO_EVENT_LED

/* constant definitions
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_DYNAMIC_LED
# ifdef CO_EVENT_PROFILE_LED
#  define CO_EVENT_LED_CNT	(CO_EVENT_DYNAMIC_LED + CO_EVENT_PROFILE_LED)
# else /* CO_EVENT_PROFILE_LED */
#  define CO_EVENT_LED_CNT	(CO_EVENT_DYNAMIC_LED)
# endif /* CO_EVENT_PROFILE_LED */
#else /* CO_EVENT_DYNAMIC_LED */
# ifdef CO_EVENT_PROFILE_LED
#  define CO_EVENT_LED_CNT	(CO_EVENT_PROFILE_LED)
# endif /* CO_EVENT_PROFILE_LED */
#endif /* CO_EVENT_DYNAMIC_LED */



/* local defined data types
---------------------------------------------------------------------------*/
typedef struct {
	UNSIGNED8	blinkEntries;
	UNSIGNED8	blinkState[5];
} CO_NV_STORAGE LED_BLINKTABLE_T;

static const LED_BLINKTABLE_T	ledBlinkTables[] =  {
	{ 1u, { 0u } },			/* CO_LED_STATE_OFF */
	{ 1u, { 1u, 0u, 0u, 0u, 0u } },		/* CO_LED_STATE_FLICKERING */
	{ 3u, { 1u, 0u, 0u, 0u, 0u } },		/* CO_LED_STATE_FLASH_1 */
	{ 4u, { 1u, 1u, 0u, 0u, 0u } },		/* CO_LED_STATE_FLASH_2 */
	{ 5u, { 1u, 1u, 1u, 0u, 0u } },		/* CO_LED_STATE_FLASH_3 */
	{ 1u, { 1u, 0u, 0u, 0u, 0u } },		/* CO_LED_STATE_BLINKING */
	{ 1u, { 1u, 0u, 0u, 0u, 0u } }		/* CO_LED_STATE_ON */
};

typedef struct {
	CO_LED_STATE_T	state;
	BOOL_T			on;
	UNSIGNED8		blinkCnt;
	UNSIGNED8		blinkMaxCnt;
	const LED_BLINKTABLE_T	*blinkTable;
} CO_LED_T;


#define PRIORITY_SIZE	7u
static const CO_LED_STATE_T	priority[PRIORITY_SIZE]  = {
	CO_LED_STATE_OFF,			/**< led is off */
	CO_LED_STATE_FLICKERING,	/**< led is flickering */
	CO_LED_STATE_FLASH_3,		/**< led is flashing mode 3 */
	CO_LED_STATE_FLASH_1,		/**< led is flashing mode 1 */
	CO_LED_STATE_FLASH_2,		/**< led is flashing mode 2 */
	CO_LED_STATE_BLINKING,		/**< led is blinking */
	CO_LED_STATE_ON				/**< led is on */
};

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_STATIC_LED
extern CO_CONST CO_EVENT_LED_T coEventLedRedInd;
extern CO_CONST CO_EVENT_LED_T coEventLedGreenInd;
#endif /* CO_EVENT_STATIC_LED */


/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static void ledTimerInd(void *pPtr);
static void startLedTimer(BOOL_T	normal);
static void stopLedTimer(void);
static void ledRedInd(BOOL_T on);
static void ledGreenInd(BOOL_T on);

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static CO_LED_T		ledGreen;
static CO_LED_T		ledRed;
static BOOL_T		ledTimerOn = { CO_FALSE };
static CO_TIMER_T	ledTimer;
static UNSIGNED8	ledCnt = { 0u };
#ifdef CO_EVENT_LED_CNT
static CO_EVENT_LED_T	ledGreenTable[CO_EVENT_LED_CNT];
static CO_EVENT_LED_T	ledRedTable[CO_EVENT_LED_CNT];
static UNSIGNED8	ledGreenTableCnt = 0u;
static UNSIGNED8	ledRedTableCnt = 0u;
#endif /* CO_EVENT_LED_CNT */




/***************************************************************************/
/**
* \brief coLedSetGreen - set green led to new state
*
* Set green LED to one of the following state:
*	- OFF,
*	- FLICKERING,
*	- FLASH_1, FLASH_2, FLASH_3,
*	- BLINKING,
*	- ON
*
* \return none
*
*/

void coLedSetGreen(
		CO_LED_STATE_T	newLedState			/**< new led state */
	)
{
	/* disable flickering */
	if (ledGreen.state == CO_LED_STATE_FLICKERING)  {
		/* disable timer */
		stopLedTimer();

		/* reset red led */
		ledRed.state = CO_LED_STATE_OFF;
	}

	/* set state */
	ledGreen.state = newLedState;

	ledGreen.blinkCnt = 0u;
	ledGreen.blinkTable = &ledBlinkTables[newLedState];
	ledGreen.blinkMaxCnt = ledGreen.blinkTable->blinkEntries;

	/* is off, return */
	if (newLedState == CO_LED_STATE_OFF)  {
		if (ledRed.state == CO_LED_STATE_OFF)  {
			stopLedTimer();
		}

		/* if timer is off, call user indication immediately */
		if (ledTimerOn == CO_FALSE)  {
			ledGreenInd(CO_FALSE);
		}

		return;
	}

	/* start timer if not active */
	if (ledTimerOn != CO_TRUE)  {
		if (newLedState == CO_LED_STATE_FLICKERING)  {
			startLedTimer(CO_FALSE);
		} else {
			startLedTimer(CO_TRUE);
		}
	}
}



/***************************************************************************/
/**
* \brief coLedSetRed - set red led to new state
*
* Set led LED to one of the following state:
*	OFF,
*	FLICKERING,
*	FLASH_1, FLASH_2, FLASH_3,
*	BLINKING,
*	ON
*
* \return none
*
*/

void coLedSetRed(
		CO_LED_STATE_T	newLedState			/**< new led state */
	)
{
	/* disable flickering */
	if (ledRed.state == CO_LED_STATE_FLICKERING)  {
		/* disable timer */
		stopLedTimer();

		/* reset red led */
		ledGreen.state = CO_LED_STATE_OFF;
	}

	/* set state */
	ledRed.state = newLedState;

	ledRed.blinkCnt = 0u;
	ledRed.blinkTable = &ledBlinkTables[newLedState];
	ledRed.blinkMaxCnt = ledRed.blinkTable->blinkEntries;

	/* is off, return */
	if (newLedState == CO_LED_STATE_OFF)  {
		if (ledGreen.state == CO_LED_STATE_OFF)  {
			stopLedTimer();
		}

		/* if timer is off, call user indication immediately */
		if (ledTimerOn == CO_FALSE)  {
			ledRedInd(CO_FALSE);
		}

		return;
	}

	/* start timer if not active */
	if (ledTimerOn != CO_TRUE)  {
		if (newLedState == CO_LED_STATE_FLICKERING)  {
			startLedTimer(CO_FALSE);
		} else {
			startLedTimer(CO_TRUE);
		}
	}

}


/***************************************************************************/
/**
* \brief coLedSetState - set led state
*
* Set the error led to special state
*	OFF,
*	FLICKERING,
*	FLASH_1, FLASH_2, FLASH_3,
*	BLINKING,
*	ON
*
* All states are saved. Only the highest prior state is displayed.
* If the highest state is reset, the next state is displayed.
*
* \return none
*
*/
void coLedSetState(
		CO_LED_STATE_T	newState,		/**< new state */
		BOOL_T			on				/**< set state to on/off */
	)
{
static UNSIGNED32	saveStates = 0ul;
UNSIGNED8	i;

	/* set state on/off */
	if (on == CO_TRUE)  {
		saveStates |= (1ul << (UNSIGNED32)newState);
	} else {
		saveStates &= ~(1ul << (UNSIGNED32)newState);
	}

	if (saveStates == 0u)  {
		coLedSetRed(CO_LED_STATE_OFF);
		return;
	}

	/* set led to the highest priority state */
	i = PRIORITY_SIZE;
	while (i > 0u)  {
		i--;
		if ((saveStates & (1ul << (UNSIGNED32)priority[i])) != 0ul)  {
			/* state found, set led */
			coLedSetRed(priority[i]);
			break;
		}
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief startLedTimer - start led timer
*
* Timer interval depends on the parameter normal:<br>
* If normal == CO_TRUE, use 200 msec<br>
* If normal == CO_FALSE, use 50 msec
*
*
* \return none
*
*/
static void startLedTimer(
		BOOL_T	normal				/* normal/fast */
	)
{
UNSIGNED32	takt;

	if (normal == CO_TRUE)  {
		/* 200 msec */
		takt = 200000ul;
	} else  {
		takt = 50000ul;
	}

	(void)coTimerStart(&ledTimer, takt, ledTimerInd, NULL, CO_TIMER_ATTR_ROUNDUP_CYCLIC);	/*lint !e960 function identifier used without '&' or parenthesized parameter list */

	ledTimerOn = CO_TRUE;
	ledCnt = 0u;
}


/***************************************************************************/
/**
* \internal
*
* \brief stopLedTimer - stop led timer
*
*
* \return none
*
*/
static void stopLedTimer(
		void	/* no parameter */
	)
{
	(void)coTimerStop(&ledTimer);

	ledTimerOn = CO_FALSE;
	ledCnt = 0u;
}


/***************************************************************************/
/*
* \internal
*
* \brief ledTimerInd - led timer routine
*
*
* \return none
*
*/
static void ledTimerInd(
		void			*pPtr
	)
{
BOOL_T		green = ledGreen.on;
BOOL_T		red = ledRed.on;
CO_LED_T	*pGreen = &ledGreen;
CO_LED_T	*pRed = &ledRed;
(void)pPtr;

	if ((ledCnt & 1u) == 0u)  {
		/* switch off green led, except state is on */
		if (pGreen->state != CO_LED_STATE_ON)  {
			green = CO_FALSE;
		}
		/* work red led */
		if (pRed->blinkTable->blinkState[pRed->blinkCnt] != 0u)  {
			red = CO_TRUE;
		}
		pRed->blinkCnt++;
		if (pRed->blinkCnt == pRed->blinkMaxCnt)  {
			pRed->blinkCnt = 0u;
		}
	} else {
		/* switch off red led */
		if (pRed->state != CO_LED_STATE_ON)  {
			red = CO_FALSE;
		}

		/* work green led */
		if (pGreen->blinkTable->blinkState[pGreen->blinkCnt] != 0u)  {
			green = CO_TRUE;
		}
		pGreen->blinkCnt++;
		if (pGreen->blinkCnt == pGreen->blinkMaxCnt)  {
			pGreen->blinkCnt = 0u;
		}
	}

	if (pGreen->on != green)  {
		/* call user indication */
		ledGreenInd(green);
		pGreen->on = green;
	}
	if (pRed->on != red)  {
		/* call user indication */
		ledRedInd(red);
		pRed->on = red;
	}

	ledCnt++;
}


/***************************************************************************/
/*
* \internal
*
* \brief ledRedInd - call user indication
*
*
* \return none
*
*/
static void ledRedInd(
		BOOL_T	on
	)
{
#ifdef CO_EVENT_LED_CNT
UNSIGNED8	cnt;

	cnt = ledRedTableCnt;
	while (cnt > 0u)  {
		cnt--;
		/* call user indication */
		ledRedTable[cnt](on);
	}
#endif /* CO_EVENT_LED_CNT */

#ifdef CO_EVENT_STATIC_LED
	coEventLedRedInd(on);
#endif /* CO_EVENT_STATIC_LED */
}


/***************************************************************************/
/*
* \internal
*
* \brief ledRedInd - call user indication
*
*
* \return none
*
*/
static void ledGreenInd(
		BOOL_T	on
	)
{
#ifdef CO_EVENT_LED_CNT
UNSIGNED8	cnt;

	cnt = ledGreenTableCnt;
	while (cnt > 0u)  {
		cnt--;
		/* call user indication */
		ledGreenTable[cnt](on);
	}
#endif /* CO_EVENT_LED_CNT */

#ifdef CO_EVENT_STATIC_LED
	coEventLedGreenInd(on);
#endif /* CO_EVENT_STATIC_LED */
}


#ifdef CO_EVENT_LED_CNT
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**
* \brief coEventRegister_LED_GREEN - register for green LED
*
* \return RET_T
*/

RET_T coEventRegister_LED_GREEN(
		CO_EVENT_LED_T pFunction	/**< pointer to function */
    )
{

	/* set new indication function as first at the list */
	if (ledGreenTableCnt >= CO_EVENT_LED) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	ledGreenTable[ledGreenTableCnt] = pFunction;	/* save function pointer */
	ledGreenTableCnt++;

	/* set actual led state */
	{
		ledGreenInd(ledRed.on);
	}

	return(RET_OK);
}


/***************************************************************************/
/**
* \brief coEventRegister_LED_RED - register for red LED

* Register application function for controlling of LED state
*
* \return RET_T
*
*/

RET_T coEventRegister_LED_RED(
		CO_EVENT_LED_T pFunction	/**< pointer to function */
    )
{

	/* set new indication function as first at the list */
	if (ledRedTableCnt >= CO_EVENT_LED) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	ledRedTable[ledRedTableCnt] = pFunction;	/* save function pointer */
	ledRedTableCnt++;

	/* set actual led state */
	{
		ledRedInd(ledRed.on);
	}

	return(RET_OK);
}
#endif /* CO_EVENT_LED_CNT */


/***************************************************************************/
/*
* \internal
*
* \brief icoLedVarInit - init LED variables
*
* Init LED variables
*
* \return none
*
*/
void icoLedVarInit(
		void
	)
{

#ifdef CO_EVENT_LED_CNT
	ledGreenTableCnt = 0u;
	ledRedTableCnt = 0u;
#endif /* CO_EVENT_LED_CNT */

	{
		ledTimerOn = CO_FALSE;
		ledCnt = 0u;
	}
}


/***************************************************************************/
/*
* \brief coLedInit - init LED functionality
*
* Init LED functionality
*
* \return none
*
*/
void coLedInit(
		void	/* no parameter */
	)
{
	coLedSetGreen(CO_LED_STATE_OFF);
	coLedSetRed(CO_LED_STATE_OFF);
}

#endif /* CO_EVENT_LED */
