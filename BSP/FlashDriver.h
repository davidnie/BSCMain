#ifndef __FLASHDRIVER_H 
#define __FLASHDRIVER_H 

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private function prototypes -----------------------------------------------*/
void df_read_open(uint32_t addr);
void df_write_open(uint32_t addr);
void df_read(uint8_t *buf,uint16_t size);
void df_write(uint8_t *buf,uint16_t size);
void df_read_seek(uint32_t addr);
void df_write_seek(uint32_t addr);
void df_read_close(void);
void df_write_close(void);

#endif 
/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

