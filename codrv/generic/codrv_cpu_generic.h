/*
* codrv_cpu_generic.h
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
* \brief CPU driver part for a generic driver
*
*
*
*/

#ifndef CODRV_CPU_GENERIC_H
#define CODRV_CPU_GENERIC_H 1


/* general hardware initialization */
void codrvHardwareInit(void);

/* init CAN related hardware part */
void codrvHardwareCanInit(void);

#endif /* CODRV_CPU_GENERIC_H */
