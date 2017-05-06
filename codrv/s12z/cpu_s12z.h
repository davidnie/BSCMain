/*
* cpu_s12z.h
*
* Copyright (c) 2016 emtas GmbH
*-------------------------------------------------------------------
* SVN  $Date: 2016-10-27 16:13:55 +0200 (Do, 27. Okt 2016) $
* SVN  $Rev: 15789 $
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
* \brief CPU for S12Z
*/

#ifndef CPU_S12Z_H
#define CPU_S12Z_H 1


extern void SetupClock (void);

/* general hardware initialization */
void initHardware(void);

/* init CAN related hardware part */
void initCanHW(void);

/* general timer */
//RET_T codrvTimerSetup( UNSIGNED32 timerInterval);
__interrupt void codrvCanTransmitInt0(void);
__interrupt void codrvCanReceiveInt0(void);
__interrupt void codrvCanErrorInt0(void);
__interrupt void Timer_int(void);
#endif /* CPU_S12Z_H */
