/*
* codrv_can_generic.h
*
* Copyright (c) 2012 emtas GmbH
*-------------------------------------------------------------------
* SVN  $Date: 2015-01-12 14:00:30 +0100 (Mo, 12. Jan 2015) $
* SVN  $Rev: 8086 $
* SVN  $Author: ro $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \file
* \brief register definition for generic driver
*
*/

#ifndef CODRV_CAN_GENERIC_H
#define CODRV_CAN_GENERIC_H 1

/* Register offsets */
#define XXCAN_STATUS 	0x0000

#define XXCAN_MAX_FILTER	15

/** STATUS */
#define	XXCAN_STATUS_ENABLE 	(1ul << 0)

/* global prototypes, that not in co_drv.h */
void codrvCanReceiveInterrupt(CanRxMsg *canMsg);
void codrvCanTransmitInterrupt(void);

/* extern required functions */
extern void codrvCanSetTxInterrupt(void);

#endif /* CODRV_CAN_GENERIC_H */

