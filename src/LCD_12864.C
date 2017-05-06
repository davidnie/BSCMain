/***********************************************************************
文件名称：LCD_12864.C
功    能：
编写时间：2013.4.25
编 写 人：
注    意：
***********************************************************************/
#include "main.h"

/*************LCD io口配置******************/
void LCD_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Enable GPIOD and GPIOE clocks */
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);  
	                    
	/* PE.00(D0), PE.01(D1), PE.02(D2), PE.03(D3), PE.04(D4), PE.05(D5), PE.06(D6), PE.07(D7), PE.08(D8)
	 PE.09(D9), PE.10(D10), PE.11(D11), PE.12(D12), PE.13(D13), PE.14(D14), PE.15(D15)   */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_OType_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	/* PD.13(RS), PD.14(WR), PD.15(RD) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}
//写数据
void WriteDataLCD(unsigned char WDLCD)
{
	ReadStatusLCD(); //检测忙 
	LCD_RS_SET;
	LCD_RW_CLR;
	GPIOE->ODR = WDLCD;
	LCD_E_SET;
	LCD_E_SET;
	LCD_E_SET;
	LCD_E_CLR;
}

//写指令
void WriteCommandLCD(unsigned char WCLCD,unsigned char BuysC) //BuysC为0时忽略忙检测
{
	if (BuysC) ReadStatusLCD(); //根据需要检测忙 
	LCD_RS_CLR;
	LCD_RW_CLR; 
	GPIOE->ODR = WCLCD;
	LCD_E_SET; 
	LCD_E_SET;
	LCD_E_SET;
	LCD_E_CLR;  
}

//读状态
unsigned char ReadStatusLCD(void)
{

	GPIO_InitTypeDef GPIO_InitStructure; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
// 	GPIOE->BSRRH = 0x44444444;
// 	GPIOE->BSRRL = 0x44444444;

	LCD_RS_CLR;
	LCD_RW_SET; 
	LCD_E_SET;
	while (GPIO_ReadInputData(GPIOE) & Busy) //检测忙信号
	{
		;
	}
	LCD_E_CLR;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
// 	GPIOE->BSRRH = 0x33333333;
// 	GPIOE->BSRRL = 0x33333333;
	return(GPIO_ReadInputData(GPIOE));
}

void LCDInit(void) //LCM初始化
{
	LCD_Configuration();
	WriteCommandLCD(0x30,1); //显示模式设置,开始要求每次检测忙信号
	WriteCommandLCD(0x01,1); //显示清屏
	WriteCommandLCD(0x06,1); // 显示光标移动设置
	WriteCommandLCD(0x0C,1); // 显示开及光标设置
}
void LCDClear(void) //清屏
{
	WriteCommandLCD(0x01,1); //显示清屏
	WriteCommandLCD(0x34,1); // 显示光标移动设置
	WriteCommandLCD(0x30,1); // 显示开及光标设置
}
void LCDFlash(void)	//闪烁效果
{
	WriteCommandLCD(0x08,1); //显示清屏
	Delay400Ms();
	WriteCommandLCD(0x0c,1); // 显示开及光标设置
	Delay400Ms();
	WriteCommandLCD(0x08,1); //显示清屏
	Delay400Ms();
	WriteCommandLCD(0x0c,1); // 显示开及光标设置
	Delay400Ms();
	WriteCommandLCD(0x08,1); //显示清屏
	Delay400Ms();
}
//按指定位置显示一个字符
void DisplayOneChar(unsigned char X, unsigned char Y, unsigned char DData)
{
	if(Y<1)
	{
		Y=1;
	}
	
	if(Y>4)
	{
		Y=4;
	}
	X &= 0x0F; //限制X不能大于16，Y不能大于1
	switch(Y)
	{
		case 1:
		{
			X|=0X80;
			break;
		}
		case 2:
		{
			X|=0X90;
			break;
		}
		case 3:
		{
			X|=0X88;
			break;
		}
		case 4:
		{
			X|=0X98;
			break;
		}
		default :
		{
			break;
		}
	}
	WriteCommandLCD(X, 0); //这里不检测忙信号，发送地址码
	WriteDataLCD(DData);
	Delay5Ms();
}

//按指定位置显示一串字符
void DisplayListChar(unsigned char X, unsigned char Y, unsigned char  *DData)
{
	unsigned char ListLength,X2;
	ListLength = 0;
	X2=X;
	if(Y<1)
	{
		Y=1;
	}
	if(Y>4)
	{
		Y=4;
	}
	X &= 0x0F; //限制X不能大于16，Y在1-4之内
	switch(Y)
	{
		case 1:
		{
			X2|=0X80;
			break;	//根据行数来选择相应地址
		}
		case 2:
		{
			X2|=0X90;
			break;
		}
		case 3:
		{	
			X2|=0X88;
			break;
		}
		case 4:
		{
			X2|=0X98;
			break;
		}
		default :
		{
			break;
		}
	}
	WriteCommandLCD(X2, 1); //发送地址码
	while (DData[ListLength]>=0x20) //若到达字串尾则退出
	{
		if (X <= 0x0F) //X坐标应小于0xF
		{
			WriteDataLCD(DData[ListLength]); //
			ListLength++;
			X++;
			Delay5Ms();
		}
	}
}

//图形显示122*32
void DisplayImage (unsigned char *DData)
{
	unsigned char x,y,i;
	unsigned int tmp=0;
	for(i=0;i<9;)
	{		//分两屏，上半屏和下半屏，因为起始地址不同，需要分开
		for(x=0;x<32;x++)
		{				//32行
			WriteCommandLCD(0x34,1);
			WriteCommandLCD((0x80+x),1);//列地址
			WriteCommandLCD((0x80+i),1);	//行地址，下半屏，即第三行地址0X88
			WriteCommandLCD(0x30,1);		
			for(y=0;y<16;y++)
			{	
				WriteDataLCD(DData[tmp+y]);//读取数据写入LCD
			}
			tmp+=16;		
		}
		i+=8;
	}
	WriteCommandLCD(0x36,1);	//扩充功能设定
	WriteCommandLCD(0x30,1);
}

//5ms延时
void Delay5Ms(void)
{
	unsigned int TempCyc = 5552 * 2;
	while(TempCyc--);
}

//400ms延时
void Delay400Ms(void)
{
	unsigned char TempCycA = 5 * 2;
	unsigned int TempCycB;
	while(TempCycA--)
	{
		TempCycB = 7269;
		while(TempCycB--);
	}
}
