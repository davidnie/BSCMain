/***********************************************************************
name: SysTick.h
function:
make time: 2013.4.25
author: 
note:
***********************************************************************/

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

void SysTick_Configuration(void);
void Delay(__IO uint32_t nTime);
void TimingDelay_Decrement(void);
uint8_t isDelayExp( uint32_t delayTime, uint32_t startTime );

extern __IO uint32_t LocalTime ;
#define TickGetDiff(a,b) ((b <= a ? a - b : ((uint32_t)0xFFFFFFFF - b) + a))
#define SECONDS( a )        ((long)a * (long)1000)
#define MILISECOND(a)		((long)a * (long)1)
#define MS10th(a)		((long)a * (long)10)
#endif 
