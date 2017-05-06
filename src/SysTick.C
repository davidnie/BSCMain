/***********************************************************************
file name: SysTick.c
make time: 2017.4.1
authe:
note: 
***********************************************************************/
#include "main.h"
#include "sysTick.h"
 __IO uint32_t TimingDelay;
extern __IO uint32_t LocalTime ;
#define SYSTEMTICK_PERIOD_MS  10

  /* Setup SysTick Timer for 1 msec interrupts.
     ------------------------------------------
    1. The SysTick_Config() function is a CMSIS function which configure:
       - The SysTick Reload register with value passed as function parameter.
       - Configure the SysTick IRQ priority to the lowest value (0x0F).
       - Reset the SysTick Counter register.
       - Configure the SysTick Counter clock source to be Core Clock Source (HCLK).
       - Enable the SysTick Interrupt.
       - Start the SysTick Counter.
    
    2. You can change the SysTick Clock source to be HCLK_Div8 by calling the
       SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8) just after the
       SysTick_Config() function call. The SysTick_CLKSourceConfig() is defined
       inside the misc.c file.

    3. You can change the SysTick IRQ priority by calling the
       NVIC_SetPriority(SysTick_IRQn,...) just after the SysTick_Config() function 
       call. The NVIC_SetPriority() is defined inside the core_cm3.h file.

    4. To adjust the SysTick time base, use the following formula:
                            
         Reload Value = SysTick Counter Clock (Hz) x  Desired Time base (s)
    
       - Reload Value is the parameter to be passed for SysTick_Config() function
       - Reload Value should not exceed 0xFFFFFF
   */
/***********************************************************************
***********************************************************************/
void SysTick_Configuration(void)
{
	if (SysTick_Config(SystemCoreClock / 1680))
	{ 
		/* Capture error */ 
		while (1);
	}
}
  /**
	* @brief  Updates the system local time
	* @param  None
	* @retval None
	*/
  void Time_Update(void)
  {
	//LocalTime += SYSTEMTICK_PERIOD_MS;
  }



/***********************************************************************
function name £ºTimingDelay_Decrement(void)
***********************************************************************/
void TimingDelay_Decrement(void)
{
	if (TimingDelay != 0x00)
	{ 
		TimingDelay--;
	}
}
uint32_t timeDiff(uint32_t t1,uint32_t t2){
	uint32_t diff;
	if(t2>=t1) diff=t2-t1;
	else diff=t1-t2+0xFFFFFFFF;
	return diff;
}
uint8_t isDelayExp( uint32_t delayTime, uint32_t startTime )
{
    uint32_t tempTick;
    tempTick = OSTimeGet();
    if ( TickGetDiff( tempTick, startTime) >  delayTime )
    {
        return 1;
    }
    return 0;
}

