/***********************************************************************
***********************************************************************/
#include "main.h"  
OS_EVENT  *sem_RS232_rec_flag = 0;			//RS232接收完一桢数据信号量定义

extern void Task_ADC(void);

OS_STK Stk_Task_HTTP[TASK_TEST_HTTP_STK_SIZE];
OS_STK Stk_Task_LED[TASK_TEST_LED_STK_SIZE];
void I2C_Test(void);

/***********************************************************************

void Task_LED(void *pdata)

***********************************************************************/
void Task_LED(void *pdata)
{		 
   while(1)
	{
		OSTimeDlyHMSM(0, 0, 0, 100);
		//	GPIO_ToggleBits(LED1);
		OSTimeDlyHMSM(0, 0, 0, 100);
		//	GPIO_ToggleBits(LED2);
		OSTimeDlyHMSM(0, 0, 0, 100);
		//	GPIO_ToggleBits(LED3);
		OSTimeDlyHMSM(0, 0, 0, 100);
		//	GPIO_ToggleBits(LED4);
	}
}
/***********************************************************************
***********************************************************************/
void Task_HTTP(void *pdata)
{
	__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
	/* configure ethernet (GPIOs, clocks, MAC, DMA) */ 
	ETH_BSP_Config();
	LwIP_Init();

	/* Http webserver Init */
	httpd_init();
	while(1)
	{  
		LocalTime ++;
		LwIP_Periodic_Handle(LocalTime);		
		OSTimeDlyHMSM(0, 0, 0, 10);//suspend at here for 10ms
	}
}
void I2C_Test(void)
{
	unsigned int i = 0;
	unsigned char WriteBuffer[256];
	unsigned char ReadBuffer[256];
	for(i = 0;i < 256;i ++)//????????
	{
		WriteBuffer[i] = i;
	}
	I2C_Write(I2C1,ADDR_24LC02,0,WriteBuffer,sizeof(WriteBuffer));	//?EEPROM???
	I2C_Read(I2C1,ADDR_24LC02,0,ReadBuffer,sizeof(WriteBuffer));	//?EEPROM???
}
void UART4_Test(void){
}
