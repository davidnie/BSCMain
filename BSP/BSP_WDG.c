/***********************************************************************
BSP_WDG.c
watch dog function
***********************************************************************/
#include "main.h"

#define WDG_RESET_TIME 1000  //ms
/***********************************************************************
name: void WDG_Init(void)
function:
input:
output:
time:
developer:
note:
***********************************************************************/

void WDG_Init(void)
{
 /*Clear all Reset Flags */
   RCC_ClearFlag();
 /*Enable write access to IWDG_PR and IWDG_RLR registers*/
 IWDG_WriteAccessCmd(0x5555);
//  IWDG->KR = 0x5555;
  /*IWDG counter clock using  pre scalar 8 - LSI/8*/
  IWDG_SetPrescaler(IWDG_PR_PR_2);
//  IWDG->PR = IWDG_PR_PR_2; /*Bit 0 in PR register*/
   /*Set counter reload value to obtain 1 sec IWDG TimeOut.*/
  IWDG_SetReload(250);
//IWDG->RLR = 250;
  /*Reloads IWDG counter with value defined in the reload register*/
  IWDG_ReloadCounter();
  //IWDG->KR = KR_KEY_RELOAD;
  IWDG_Enable();

//  IWDG->KR = KR_KEY_ENABLE;
  
}


void wdg_feed(void)
{
 /*Writing 0xAAAA in the Key register prevents watchdog reset*/
 IWDG_ReloadCounter();
 //IWDG->KR = KR_KEY_RELOAD;
}


