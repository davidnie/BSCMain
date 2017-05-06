/***********************************************************************
name: timer.h
make time: 2013.4.25
auther: 
note:
***********************************************************************/
#ifndef _TIMER_H_
#define _TIMER_H_

void TIM_Configuration(void);
#define SECONDS( a )        ((long)a * (long)1000)
#define MILISECOND(a)		((long)a * (long)1)
#define MS10th(a)		((long)a * (long)10)

#endif

