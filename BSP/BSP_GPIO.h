/***********************************************************************
filename£ºBSP_GPIO.h
function: initlize and name GPIO
note:
***********************************************************************/
#ifndef _LED_H_
#define _LED_H_

#include "config.h"

	#define LED1			GPIOC , GPIO_Pin_8
	#define LED1_ON 		GPIO_ResetBits(GPIOC , GPIO_Pin_8)
	#define LED1_OFF 		GPIO_SetBits(GPIOC , GPIO_Pin_8)
	#define LED1_Toggle		GPIO_ToggleBits(GPIOC , GPIO_Pin_8);
#ifdef EVAL_BOARD_Z
	#define LED1			GPIOC , GPIO_Pin_9
	#define LED1_ON 		GPIO_ResetBits(GPIOC , GPIO_Pin_9)
	#define LED1_OFF 		GPIO_SetBits(GPIOC , GPIO_Pin_9)
#endif
#define SC_POWER			GPIO_ReadOutputDataBit(GPIOD , GPIO_Pin_2)
#define SC_POWER_OFF 		GPIO_ResetBits(GPIOD , GPIO_Pin_2)
#define SC_POWER_ON 		GPIO_SetBits(GPIOD , GPIO_Pin_2)

#define LED2			GPIOE , GPIO_Pin_12
#define LED2_ON 		GPIO_ResetBits(GPIOG , GPIO_Pin_12)
#define LED2_OFF 		GPIO_SetBits(GPIOG , GPIO_Pin_12)
#define LED4			GPIOG , GPIO_Pin_12
#define LED4_ON 		GPIO_ResetBits(GPIOG , GPIO_Pin_12)
#define LED4_OFF 		GPIO_SetBits(GPIOG , GPIO_Pin_12)

#define LS1			GPIOE , GPIO_Pin_1
#define LS1_STAT	GPIO_ReadOutputDataBit(LS1)
#define LS1_ON 		GPIO_SetBits(GPIOE , GPIO_Pin_1)
#define LS1_OFF 		GPIO_ResetBits(GPIOE , GPIO_Pin_1)
#define LS2			GPIOE , GPIO_Pin_2
#define LS2_STAT	GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_2)
#define LS2_ON 		GPIO_SetBits(GPIOE , GPIO_Pin_2)
#define LS2_OFF 		GPIO_ResetBits(GPIOE , GPIO_Pin_2)
#define LS3			GPIOE , GPIO_Pin_3
#define HS3_STAT	GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_3)
#define LS3_ON 		GPIO_SetBits(GPIOE , GPIO_Pin_3)
#define LS3_OFF 		GPIO_ResetBits(GPIOE , GPIO_Pin_3)
#define PWR_12V			GPIOE , GPIO_Pin_4
#define GET_PWR_12V_STAT GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_4)
#define PWR_12V_ON 		GPIO_ResetBits(GPIOE , GPIO_Pin_4)
#define PWR_12V_OFF 		GPIO_SetBits(GPIOE , GPIO_Pin_4)
#define HS1			GPIOE , GPIO_Pin_10
#define HS1_STAT	GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_10)
#define HS1_ON 		GPIO_ResetBits(GPIOE , GPIO_Pin_10)
#define HS1_OFF 		GPIO_SetBits(GPIOE , GPIO_Pin_10)
#define HS2			GPIOE , GPIO_Pin_11
#define HS2_STAT	GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_11)
#define HS2_ON 		GPIO_ResetBits(GPIOE , GPIO_Pin_11)
#define HS2_OFF 		GPIO_SetBits(GPIOE , GPIO_Pin_11)
#define HS3			GPIOE , GPIO_Pin_12
#define HS3_STAT	GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_12)
#define HS3_ON 		GPIO_ResetBits(GPIOE , GPIO_Pin_12)
#define HS3_OFF 		GPIO_SetBits(GPIOE , GPIO_Pin_12)
#define HS4			GPIOE , GPIO_Pin_13
#define HS4_STAT	GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_13)
#define HS4_ON 		GPIO_ResetBits(GPIOE , GPIO_Pin_13)
#define HS4_OFF 		GPIO_SetBits(GPIOE , GPIO_Pin_13)
#define HS5			GPIOE , GPIO_Pin_14
#define HS5_STAT	GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_14)
#define HS5_ON 		GPIO_ResetBits(GPIOE , GPIO_Pin_14)
#define HS5_OFF 		GPIO_SetBits(GPIOE , GPIO_Pin_14)
#define BMU_WAKE_STAT			GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_15)
#define BMU_WAKE_HIGH 		GPIO_SetBits(GPIOE , GPIO_Pin_15)
#define BMU_WAKE_LOW		GPIO_ResetBits(GPIOE , GPIO_Pin_15)

#define ISODO_1			GPIOF , GPIO_Pin_0
#define ISODO_ON_1 		GPIO_SetBits(GPIOF , GPIO_Pin_0)
#define ISODO_OFF_1 	GPIO_ResetBits(GPIOF , GPIO_Pin_0)
#define ISODO_2			GPIOF , GPIO_Pin_1
#define ISODO_ON_2 		GPIO_SetBits(GPIOF , GPIO_Pin_1)
#define ISODO_OFF_2 	GPIO_ResetBits(GPIOF , GPIO_Pin_1)
#define ISODO_3			GPIOF , GPIO_Pin_2
#define ISODO_ON_3 		GPIO_SetBits(GPIOF , GPIO_Pin_2)
#define ISODO_OFF_3 	GPIO_ResetBits(GPIOF , GPIO_Pin_2)


#define DRY_SIG_1		GPIOE , GPIO_Pin_7
#define DRY_SIG_2 		GPIOE , GPIO_Pin_8
#define DI_WAKE_BY_CHG		GPIO_ReadInputDataBit(GPIOD , GPIO_Pin_3)
#define DI_60V_1		GPIOF , GPIO_Pin_3
#define DI_60V_2 		GPIOF , GPIO_Pin_4
#define DI_12V_1		GPIOF , GPIO_Pin_5
#define DI_12V_2 		GPIOF , GPIO_Pin_6



#ifdef PRO_TKS
#define PWR_24V_ON    HS3_ON
#define PWR_24V_OFF	 HS3_OFF
#define HEATER_CTRL_1_ON	HS1_ON
#define HEATER_CTRL_1_OFF	HS1_OFF
#define HEATER_CTRL_2_ON	HS2_ON
#define HEATER_CTRL_2_OFF	HS2_OFF
MCU_RELAY_MAIN_POS

#endif

#ifdef PRO_FORK
#define HEATER_CTRL_1_ON	LS1_ON
#define HEATER_CTRL_1_OFF	LS1_OFF
#define HEATER_CTRL_2_ON	LS2_ON
#define HEATER_CTRL_2_OFF	LS2_OFF
#define AC_PRESENT DI_12V_1

#endif


void GPIO_Configuration(void);
void LED_Blink(void);
void One_LED_ON(unsigned char led_num);
static LED_Delay(uint32_t nCount);

#endif
