/*
 * FreeModbus Libary: BARE Demo Application
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: demo.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "main.h"
#include "mb.h"
#include "mbport.h"
#include "bmu.h"
#include "task_main.h"
/* ----------------------- Defines ------------------------------------------*/
#define mainMB_TASK_PRIORITY    ( tskIDLE_PRIORITY + 3 )
#define REG_INPUT_START         30000
#define REG_INPUT_NREGS         4
#define REG_HOLDING_START       40001
#define REG_HOLDING_NREGS       150
#define REG_HOLDING_START_2       40201 //read MODULE infromation 20byte per module
#define REG_HOLDING_NREGS_2      200

#define DISCRETE_START         10000
#define DISCRETE_NREGS         32
#define COIL_START       20000
#define COIL_NREGS       256

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];

static USHORT   usDiscreteStart = DISCRETE_START;
static UCHAR   usDiscreteBuf[DISCRETE_NREGS/8];
static USHORT   usCoilStart = COIL_START;
static UCHAR   usCoilBuf[COIL_NREGS/8];
static void mbUpdateReg(void);
void fillMbReg(USHORT dstAddr,USHORT * ptData,UCHAR len);

void mbtcpd_init(void);
/* ----------------------- Start implementation -----------------------------*/

void Task_HTTP_d(void *pdata)
{
//nie	__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
	/* configure ethernet (GPIOs, clocks, MAC, DMA) */ 
	ETH_BSP_Config();
	LwIP_Init();

	/* Http webserver Init */
	httpd_init();
	while(1)
	{  
//nie		LocalTime ++;
//nie		LwIP_Periodic_Handle(LocalTime);		
		OSTimeDlyHMSM(0, 0, 0, 10);//suspend at here for 10ms
	}
}
int mbtcp_task( void *pdata )
{
    eMBErrorCode    eStatus;
	//enable ethernet
	ETH_BSP_Config();
	LwIP_Init();
	//mbtcpd_init();

    /* Enable the Modbus Protocol Stack. */
	eMBTCPInit( 502 );
    eStatus = eMBEnable(  );

    for( ;; )
    {
        ( void )eMBPoll(  );

        /* Here we simply count the number of poll cycles. */
		mbUpdateReg();
		OSTimeDlyHMSM(0, 0, 0, 10);//suspend at here for 10ms
    }
}

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_HOLDING_START ) &&
        ( (usAddress + usNRegs) <= (REG_HOLDING_START + REG_HOLDING_NREGS )) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {
            /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
                *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] >> 8 );
                *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;

            /* Update current register values with new values from the
           		  * protocol stack. */
        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
        }
    }
	
    else if( ( usAddress >= REG_HOLDING_START_2 ) &&
        ( usAddress + usNRegs <= REG_HOLDING_START_2 + REG_HOLDING_NREGS_2 ) ) //read module infomration 20reg per mod
    {
    	UINT16 modbuf[160];
		int mid;
		for(mid=0;mid<8;mid++){
			memcpy(&modbuf[mid*20],&bmu[mid].cv[0],28);
			modbuf[mid*20+15]=bmu[mid].ct[0]/10;
			modbuf[mid*20+16]=bmu[mid].ct[1]/10;
			modbuf[mid*20+17]=bmu[mid].ct[2]/10;
		}
        iRegIndex = ( int )( usAddress - REG_HOLDING_START_2 );
        if(eMode==MB_REG_READ){
            while( usNRegs > 0 )
            {
                *pucRegBuffer++ = ( UCHAR ) ( modbuf[iRegIndex] >> 8 );
                *pucRegBuffer++ = ( UCHAR ) ( modbuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
         }
    }
	else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
	UCHAR bid,Bid,bufId,wid,rid;
	UCHAR bit_value;
	usAddress--;

    if( ( usAddress >= COIL_START) &&
        ( usAddress + usNCoils <= COIL_START + COIL_NREGS) )
    {
        iRegIndex = ( int )( usAddress - usCoilStart);
        switch ( eMode )
        {
            /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
			/*get first coil posistion*/
			bid=(usAddress-COIL_START)%8;
			Bid=(usAddress-COIL_START)/8;
			bufId=0;
			wid=0;
	      while( usNCoils > 0 )
	      {
	      	bit_value=((usCoilBuf[Bid]>>bid)&0x1);
			if(bit_value>0){
		        *pucRegBuffer |= 1<<wid;
			}else{
		        *pucRegBuffer &= ~(1<<wid);
			}
	       	usNCoils--;
				bid++;
				wid++;
				if(bid>=8){
					bid=0;
					Bid++;
				}
				if((wid%8)==0){
					wid=0;
					pucRegBuffer++;
				}
					
            }
            break;

            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
			bid=(usAddress-COIL_START)%8;
			Bid=(usAddress-COIL_START)/8;
			bufId=0;
			rid=0;
            while( usNCoils > 0 )
            {	
            	bit_value=(((*pucRegBuffer)>>rid)&0x01);
				if(bit_value>0){
	            	usCoilBuf[Bid]|=bit_value<<bid;
				}else{
	            	usCoilBuf[Bid]&=~(1<<bid);
				}
                //usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                //usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                //iRegIndex++;
                usNCoils--;
				bid++;
				rid++;
				if((bid%8)==0){
					bid=0;
					Bid++;
				}
				if(rid>=8){
					rid=0;
					pucRegBuffer++;
				}
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
	UCHAR bid,Bid,bufId,wid,rid;
	UCHAR bit_value;
	usAddress--;

    if( ( usAddress >= DISCRETE_START) &&
        ( usAddress + usNDiscrete <= DISCRETE_START + DISCRETE_NREGS) )
    {
        iRegIndex = ( int )( usAddress - usCoilStart);
			/*get first coil posistion*/
			bid=(usAddress-DISCRETE_START)%8;
			Bid=(usAddress-DISCRETE_START)/8;
			bufId=0;
			wid=0;
	      while( usNDiscrete > 0 )
	      {
	      	bit_value=((usDiscreteBuf[Bid]>>bid)&0x1);
			if(bit_value>0){
		        *pucRegBuffer |= 1<<wid;
			}else{
		        *pucRegBuffer &= ~(1<<wid);
			}
	       	usNDiscrete--;
				bid++;
				wid++;
				if(bid>=8){
					bid=0;
					Bid++;
				}
				if((wid%8)==0){
					wid=0;
					pucRegBuffer++;
				}
					
            }
             }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}
//below add by d
void mbtcpd_init(void)
{
  struct tcp_pcb *pcb;

//  pcb = tcp_new();
//  tcp_bind(pcb, IP_ADDR_ANY, 80);
 // pcb = tcp_listen(pcb);
 // tcp_accept(pcb, http_accept);
  xMBTCPPortInit(502);
}
static void mbUpdateReg(void){
	UCHAR sid;
	for(sid=0;sid<NUM_STR;sid++){
		usRegHoldingBuf[sid*8]=strInfo[sid].s_cv_max;
		usRegHoldingBuf[sid*8+1]=strInfo[sid].s_cv_min;
	}
	//coil read. 0x01 return contactor status. start at 10000
	usDiscreteBuf[0]=getRelayStatus(RL_MAIN_POS);
	usDiscreteBuf[0]|=getRelayStatus(RL_MAIN_NEG)<<1;
	usDiscreteBuf[0]|=getRelayStatus(RL_PCHG)<<2;
	usDiscreteBuf[0]|=getRelayStatus(RL_CHG_POS)<<3;
	usDiscreteBuf[0]|=getRelayStatus(RL_DCDC)<<4;
	usDiscreteBuf[0]|=getRelayStatus(RL_HEATER_1)<<5;
	usDiscreteBuf[0]|=getRelayStatus(RL_HEATER_2)<<6;
	usDiscreteBuf[1]=getSStrRelayStatus(0);
	usDiscreteBuf[1]|=getSStrRelayStatus(1)<<1;
	usDiscreteBuf[1]|=getSStrRelayStatus(2)<<2;
	usDiscreteBuf[1]|=getSStrRelayStatus(3)<<3;
	usDiscreteBuf[1]|=getSStrRelayStatus(4)<<4;
	usDiscreteBuf[1]|=getSStrRelayStatus(5)<<5;
	usDiscreteBuf[1]|=getSStrRelayStatus(6)<<6;
	usDiscreteBuf[1]|=getSStrRelayStatus(7)<<7;
}
void fillMbReg(USHORT dstAddr,USHORT * ptData,UCHAR len){
	USHORT *addr;
	addr=&usRegHoldingBuf[dstAddr];
	memcpy((USHORT *)&usRegHoldingBuf[dstAddr],ptData,len);

}
	
