/***********************************************************************
文件名称：
功    能：
编写时间：
编 写 人：
注    意：
***********************************************************************/
 
#ifndef _MAIN_H_
#define _MAIN_H_


//添加必要的头文件
#include "config.h"
#include "stm32f4xx.h" 
#include "stm32f4x7_eth.h"
#include "stm32f4x7_eth_bsp.h"
#include "stm32f4xx_spi.h"

#include "lwip/tcp.h"
#include "httpd.h"
#include "STDIO.h"

#include "M_Type_P.h"
#include "M_CMSIS_P.h" 
#include "M_Global.h"
#include "Task_Startup.h"
#include "Bsp_Global.h"
#include "app_inc.h"

#include "BSP_GPIO.h"   
#include "BSP_ADC.h"
#include "DMA.h"
#include "NVIC.h"
#include "sci.h"
//#include "canfestival.h"
#include "globalVar.h"
#include "bmu.h"
#include "24C02.H"
#include "Task_AVChg.h"
#include "SST25VF016B.h"
#include "FlashDriver.h"
#include "wdg.h"
#include "config.h"
//#include "Task_SysCheck.h"
//ET Module版本号
#define M_VERSION	100


#define M_DEV_MCU   1
 #define RMII_MODE  1
void M_Global_init(void);


//////////////////////////////////////////////////////////////////////////
/////////////////////////以下包含不同模块的头文件/////////////////////////
//////////////////////////////////////////////////////////////////////////

//软延时函数
#include "M_Delay_P.h"

void Delay(uint32_t nCount);


#endif
