/****************************************Copyright (c)**************************************************                         
---------------------------------------------------------------------------------------------------
** Created by:			David Nie
** Created date:		2010-10-29
** Version:				1.0
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:	
** Version:
** Descriptions:		
********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "main.h"
/* Private variables ---------------------------------------------------------*/
static uint32_t CurReadAddr;	/* current read address */
static uint32_t CurWriteAddr;	/* current write address	*/

/*******************************************************************************
* Function Name  : df_read_open
* Description    : read initlize function
* Input          : - addr: set read address
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void df_read_open(uint32_t addr)
{
  CurReadAddr=addr;	/* current read address	*/
}


/*******************************************************************************
* Function Name  : df_write_open
* Description    : write initlize
* Input          : - addr: write address after open
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void df_write_open(uint32_t addr)
{
CurWriteAddr=addr;	/* set write address */
}

/*******************************************************************************
* Function Name  : df_read
* Description    : read flash 
* Input          : - buf: pointer of data will write to
*                  - size: read size
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void df_read(uint8_t *buf,uint16_t size)
{
  if( CurReadAddr + size <= MAX_ADDR )
  {
    SSTF016B_RD(CurReadAddr,buf,size);
    CurReadAddr+=size;
  }
}

/*******************************************************************************
* Function Name  : df_write
* Description    : write data to flash
* Input          : - buf: pointer of data will wirte to flash
*                  - size: write size
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void df_write(uint8_t *buf,uint16_t size)
{
  if( CurWriteAddr + size <= MAX_ADDR )
  {
    SSTF016B_WR(CurWriteAddr,buf,size);
    CurWriteAddr+=size;
  }
}

/*******************************************************************************
* Function Name  : df_read_seek
* Description    : change read address
* Input          : - addr: new address
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void df_read_seek(uint32_t addr)
{
 df_read_close();
 df_read_open(addr);
}

/*******************************************************************************
* Function Name  : df_write_seek
* Description    : change write to address
* Input          : - addr: new address
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void df_write_seek(uint32_t addr)
{
 df_write_close();
 df_write_open(addr); 
}

/*******************************************************************************
* Function Name  : df_read_close
* Description    : None
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void df_read_close(void)
{
 /*no action */
}

/*******************************************************************************
* Function Name  : df_write_close
* Description    : None
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void df_write_close(void)
{
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
