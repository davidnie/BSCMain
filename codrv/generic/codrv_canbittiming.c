/*
* codrv_canbittiming.c - CAN Bittiming tables 
*
* Copyright (c) 2012 emtas GmbH
*-------------------------------------------------------------------
* $Id: codrv_canbittiming.c 8086 2015-01-12 13:00:30Z ro $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief 
*
* \file 
* \author emtas GmbH
*
* This module contains different bittiming tables.
*
*/
/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#include <co_datatype.h>
#include <co_drv.h>

#ifdef CODRV_BIT_TABLE_EXTERN 


/*
* 50 MHz table
*
* Samplepoint is not on 87.5%.
*/

#ifdef CODRV_CANCLOCK_50MHZ
CO_CONST CODRV_BTR_T codrvCanBittimingTable[] = {
		/* 50MHz table, prescaler 6bit (max 64) + BRPE 4bit == 1024 */
# ifdef CODRV_CANCLOCK_PRE_10BIT
		{    10u,250u, 0, 16u, 3u }, /* !! 85% */
		{    20u,125u, 0, 16u, 3u }, /* !! 85% */
# endif
		{    50u, 50u, 0, 16u, 3u }, /* !! 85% */
		{   100u, 25u, 0, 16u, 3u }, /* !! 85% */
		{   125u, 25u, 0, 13u, 2u }, 
		{   250u, 10u, 0, 16u, 3u }, /* !! 85% */
		{   500u,  5u, 0, 16u, 3u }, /* !! */
		{  1000u,  5u, 0, 7u, 2u }, /* !! 80% */
		{0,0,0,0} /* last */
	};
#endif

#ifdef CODRV_CANCLOCK_36MHZ
CO_CONST CODRV_BTR_T codrvCanBittimingTable[] = {
		/* 36MHz table, prescaler 10bit (max 1024) */
# ifdef CODRV_CANCLOCK_PRE_10BIT
		{   10u,225u, 0, 13u, 2u }, 
		{   20u,120u, 0, 12u, 2u }, /* 86,7% */
# endif
		{   50u, 45u, 0, 13u, 2u }, /* 85% */
		{  100u, 18u, 0, 16u, 3u }, /* 85% */
		{  125u, 18u, 0, 13u, 2u },
		{  250u,  9u, 0, 13u, 2u },
		{  500u,  9u, 0,  6u, 1u }, /* only 8tq */
		{  800u,  3u, 0, 12u, 2u },
		{ 1000u,  2u, 0, 14u, 3u}, /* 83.3% */
		{0,0,0,0} /* last */
	};
#endif

#ifdef CODRV_CANCLOCK_20MHZ
CO_CONST CODRV_BTR_T codrvCanBittimingTable[] = {
		/* 20MHz table, prescaler 6bit (max 64) + BRPE 4bit == 1024 */
# ifdef CODRV_CANCLOCK_PRE_10BIT
		{    10u,125u, 0, 13u, 2u }, 
# endif
		{    20u, 50u, 0, 16u, 3u }, /* !! 85% */
		{    50u, 25u, 0, 13u, 2u }, 
		{   100u, 10u, 0, 16u, 3u }, /* !! 85% */
		{   125u, 10u, 0, 13u, 2u }, 
		{   250u,  5u, 0, 13u, 2u },
		{   500u,  2u, 0, 16u, 3u }, /* !! 85% */
		{  1000u,  1u, 0, 16u, 3u }, /* !! 85% */
		{0,0,0,0} /* last */
	};
#endif

#ifdef CODRV_CANCLOCK_16MHZ
CO_CONST CODRV_BTR_T codrvCanBittimingTable[] = {
		/* 16MHz table, prescaler 6bit (max 64) + BRPE 4bit == 1024 */
# ifdef CODRV_CANCLOCK_PRE_10BIT
		{    10u,100u, 0, 13u, 2u }, 
# endif
		{    20u, 50u, 0, 13u, 2u }, 
		{    50u, 20u, 0, 13u, 2u }, 
		{   100u, 10u, 0, 13u, 2u }, 
		{   125u,  8u, 0, 13u, 2u }, 
		{   250u,  4u, 0, 13u, 2u },
		{   500u,  2u, 0, 13u, 2u }, 
		{  1000u,  1u, 0, 13u, 2u }, 
		{0,0,0,0} /* last */
	};
#endif



#endif /* CODRV_BIT_TABLE_EXTERN */
