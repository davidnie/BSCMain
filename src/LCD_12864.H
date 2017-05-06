/**********************lcd 128648*************************/
#ifndef _LCD_12864_H_
#define _LCD_12864_H_

#define LCD_RS_SET    GPIO_SetBits(GPIOD , GPIO_Pin_13);
#define LCD_RS_CLR    GPIO_ResetBits(GPIOD , GPIO_Pin_13);

#define LCD_RW_SET    GPIO_SetBits(GPIOD , GPIO_Pin_14);
#define LCD_RW_CLR    GPIO_ResetBits(GPIOD , GPIO_Pin_14);

#define LCD_E_SET     GPIO_SetBits(GPIOD , GPIO_Pin_15);
#define LCD_E_CLR     GPIO_ResetBits(GPIOD , GPIO_Pin_15);

#define Busy    0x80 //用于检测LCD状态字中的Busy标识

void WriteDataLCD(unsigned char WDLCD);
void WriteCommandLCD(unsigned char WCLCD,unsigned char BuysC);
unsigned char ReadDataLCD(void);
unsigned char ReadStatusLCD(void);
void LCDInit(void);
void LCDClear(void);
void LCDFlash(void);
void DisplayOneChar(unsigned char X, unsigned char Y, unsigned char DData);
void DisplayListChar(unsigned char X, unsigned char Y, unsigned char  *DData);
void DisplayImage (unsigned char *DData);
void Delay5Ms(void);
void Delay400Ms(void);
#endif
