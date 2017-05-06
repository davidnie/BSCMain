/***********************************************************************
filename: BSP_global.c
function:  configuration all device

***********************************************************************/
#include "main.h"

/*
 
 */
void BSP_Init(void)
{
	DIO_Configuration();
	I2C_Configuration();
	CAN1_Configuration();
//	CAN2_Configuration();
	ADC_Configuration();
	UART4_Configuration(); //for external charger
	UART1_Configuration(); //for wifi module
	//RS232_DMA_Init();
	UART4_DMA_Init();
	SPI_FLASH_Init();
	
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	 /*enable RNG */
	 RNG_Cmd(ENABLE);
	WDG_Init();
		NVIC_Configuration();
		SC_POWER_ON;
	BMU_WAKE_LOW;
}

