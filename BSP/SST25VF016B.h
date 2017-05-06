/****************************************Copyright (c)**************************************************
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File Name: 			SST25VF016B.h
** Last modified Date: 	2010-10-29
** Last Version: 		1.0
** Descriptions: 		SPI flash �������� 
**
**------------------------------------------------------------------------------------------------------
** Created 	 By: 		AVRman
** Created date: 		2010-10-29
** Version: 1.0
** Descriptions: 		First version
**
**------------------------------------------------------------------------------------------------------
** Modified by: 	
** Modified date: 	
** Version:
** Descriptions:  	
**
********************************************************************************************************/

#ifndef __SST25VF016B_H 
#define __SST25VF016B_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* �������SST25VF016B��������� */
typedef enum ERTYPE{Sec1,Sec8,Sec16,Chip} ErType;  
typedef enum IDTYPE{Manu_ID,Dev_ID,Jedec_ID} idtype;

/* Private define ------------------------------------------------------------*/
#define MAX_ADDR		0x1FFFFF	/* ����оƬ�ڲ�����ַ */
#define	SEC_MAX     	511         /* ������������� */
#define SEC_SIZE		0x1000      /* ������С	*/

/* ��,���ڶ���SST25VF016B��Ƭѡ���� */
#define RCC_APB2Periph_GPIO_SPI_FLASH_CS      RCC_APB2Periph_GPIOB
#define SPI_FALSH_CS_PORT                     GPIOB 
#define SPI_FALSH_CS_PIN                      GPIO_Pin_0 

// #define SPI_FLASH_CS_LOW()       GPIO_ResetBits(GPIOB, GPIO_Pin_0)
// #define SPI_FLASH_CS_HIGH()      GPIO_SetBits(GPIOB, GPIO_Pin_0)

/* Private function prototypes -----------------------------------------------*/
void  SPI_FLASH_Init(void);
uint8_t SSTF016B_RD(uint32_t Dst, uint8_t* RcvBufPt ,uint32_t NByte);
uint8_t SSTF016B_RdID(idtype IDType,uint32_t* RcvbufPt);
uint8_t SSTF016B_WR(uint32_t Dst, uint8_t* SndbufPt,uint32_t NByte);

uint8_t SSTF016B_Erase(uint32_t sec1, uint32_t sec2);
void SPI_FLASH_Test(void);
void df_write_open(uint32_t addr);
void df_write(uint8_t *buf,uint16_t size);
void df_read(uint8_t *buf,uint16_t size);
void df_read_open(uint32_t addr);
void df_write_close(void);

#endif
/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

