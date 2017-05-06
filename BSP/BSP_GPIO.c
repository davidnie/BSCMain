/***********************************************************************
filename: GPIO_BSP.C
function: hardware layer configuration and control
time :2016.09.15
auther: D.N

***********************************************************************/
#include "main.h"

/***********************************************************************

***********************************************************************/
void DIO_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC , ENABLE); 						 
			
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 |GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	/* port D*/
	
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD , ENABLE); 						 
			
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* Enable the GPIO E */
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOE , ENABLE); 						 
	/*BMU Wakeup pin out */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	/*Lowside drive 1,2,3 and 12V output poewr control set as digital ouput */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	/*high side drive 1,2,3,4,5 and output wakeup control set as digital ouput */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 |GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Enable the GPIO F*/
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOF , ENABLE); 						 
	/*general digital output 1,2,3 set as DO */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* Enable the GPIO G*/
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOG , ENABLE); 						 
			
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);


/******below are input ***************/

	/*extenal input voltage detection pin 60V/12V. set as DI */
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD, ENABLE); 						 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5| GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
		

HS1_OFF;
HS2_OFF;
HS3_OFF;
HS4_OFF;
HS5_OFF;






}

/***********************************************************************
LED D16 controlled by D2
***********************************************************************/
void LED_Blink(void)
{
	LED1_ON;
	LED_Delay(0x4fffff);
	/*====LED-OFF=======*/ 
	LED1_OFF;
	LED_Delay(0x4fffff);
	
}
/***********************************************************************
function name£ºOne_LED_ON(unsigned char led_num)
function: turn on LED
input: 
ouput: 
time: 2017.1.25
writer£º
note:
***********************************************************************/
void One_LED_ON(unsigned char led_num)
{	
	LED2_OFF;
	LED4_OFF;
	switch(led_num)
	{
		case 1:
		{
			break;
		}
		case 2:
		{
			LED2_ON;
			break;		
		}
		case 3:
		{
			break;		
		}
		case 4:
		{
			LED4_ON;
			break;		
		}
		default:
		{
			break;	
		}
	}		
}

static LED_Delay(uint32_t nCount)
{ 
  while(nCount > 0)
  { 
  	  nCount --;   
  }
}
