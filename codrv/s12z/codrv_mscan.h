/*
* codrv_mscan.h
*
* Copyright (c) 2012 emtas GmbH
*-------------------------------------------------------------------
* SVN  $Date: 2016-10-27 16:13:55 +0200 (Do, 27. Okt 2016) $
* SVN  $Rev: 15789 $
* SVN  $Author: ro $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*/

/********************************************************************/
/**
* \file
* \brief mscan can driver
*
*/

#ifndef CODRV_MSCAN_H
#define CODRV_MSCAN_H 1

/** CANCTL0 */
#define MSCAN_CANCTL0        0
#define MSCAN_CANCTL0_INITRQ (1 << 0)
#define MSCAN_CANCTL0_SLPRQ  (1 << 1)
#define MSCAN_CANCTL0_WUPE   (1 << 2)
#define MSCAN_CANCTL0_TIME   (1 << 3)
#define MSCAN_CANCTL0_SYNCH  (1 << 4)
#define MSCAN_CANCTL0_CSWAI  (1 << 5)
#define MSCAN_CANCTL0_RXACT  (1 << 6)
#define MSCAN_CANCTL0_RXFRM  (1 << 7)

/** CANCTL1 */
#define MSCAN_CANCTL1         1
#define MSCAN_CANCTL1_INITAK  (1 << 0)
#define MSCAN_CANCTL1_SLPAK   (1 << 1)
#define MSCAN_CANCTL1_WUPM    (1 << 2)
#define MSCAN_CANCTL1_BORM    (1 << 3)
#define MSCAN_CANCTL1_LISTEN  (1 << 4)
#define MSCAN_CANCTL1_LOOPB   (1 << 5)
#define MSCAN_CANCTL1_CLKSRC  (1 << 6)
#define MSCAN_CANCTL1_CANE    (1 << 7)


/** CANBTR0 */
#define MSCAN_CANBTR0		2
#define MSCAN_CANBTR0_BRP_MASK  0x3F
#define MSCAN_CANBTR0_SJW_MASK  0xC0


/** CANBTR1 */
#define MSCAN_CANBTR1		  3
#define MSCAN_CANBTR1_TSEG1_MASK  0x0F
#define MSCAN_CANBTR1_TSEG2_MASK  0x70
#define MSCAN_CANBTR1_SAMP        (1 << 7)


/** CANRFLG */
#define MSCAN_CANRFLG	     4
#define MSCAN_CANRFLG_RXF    (1 << 0)
#define MSCAN_CANRFLG_OVRIF  (1 << 1)
#define MSCAN_CANRFLG_BUSOFF (1 << 2 | 1 << 3)
#define MSCAN_CANRFLG_TX_PASSIVE (1 << 3)
#define MSCAN_CANRFLG_RX_PASSIVE (1 << 5)
#define MSCAN_CANRFLG_TSTAT_MASK  0x0C
#define MSCAN_CANRFLG_RSTAT_MASK  0x30
#define MSCAN_CANRFLG_CSCIF  (1 << 6)
#define MSCAN_CANRFLG_WUPIF  (1 << 7)
#define MSCAN_CANRFLG_ALLERRORS (0xFE)


/** CANRIER */
#define MSCAN_CANRIER        5
#define MSCAN_CANRIER_RXFIE  (1 << 0)
#define MSCAN_CANRIER_OVRIE  (1 << 1)
#define MSCAN_CANRIER_TSTAT_MASK  0x0C
#define MSCAN_CANRIER_RSTAT_MASK  0x30
#define MSCAN_CANRIER_CSCIE  (1 << 6)
#define MSCAN_CANRIER_WUPIE  (1 << 7)


/** CANTFLG */
#define MSCAN_CANTFLG	    6
#define MSCAN_CANTFLG_TXE0  (1 << 0)
#define MSCAN_CANTFLG_TXE1  (1 << 1)
#define MSCAN_CANTFLG_TXE2  (1 << 2)


/** CANTIER */
#define MSCAN_CANTIER         7
#define MSCAN_CANTIER_TXEIE0  (1 << 0)
#define MSCAN_CANTIER_TXEIE1  (1 << 1)
#define MSCAN_CANTIER_TXEIE2  (1 << 2)


/** CANTARQ */
#define MSCAN_CANTARQ         8
#define MSCAN_CANTARQ_ABTRQ0  (1 << 0)
#define MSCAN_CANTARQ_ABTRQ1  (1 << 1)
#define MSCAN_CANTARQ_ABTRQ2  (1 << 2)


/** CANTAAK */
#define MSCAN_CANTAAK         9
#define MSCAN_CANTAAK_ABTAK0  (1 << 0)
#define MSCAN_CANTAAK_ABTAK1  (1 << 1)
#define MSCAN_CANTAAK_ABTAK2  (1 << 2)


/** CANTBSEL */
#define MSCAN_CANTBSEL      10
#define MSCAN_CANTBSEL_TX0  (1 << 0)
#define MSCAN_CANTBSEL_TX1  (1 << 1)
#define MSCAN_CANTBSEL_TX2  (1 << 2)


/** CANIDAC */
#define MSCAN_CANIDAC        11
#define MSCAN_CANIDAC_IDHIT0  (1 << 0)
#define MSCAN_CANIDAC_IDHIT1  (1 << 1)
#define MSCAN_CANIDAC_IDHIT2  (1 << 2)
#define MSCAN_CANIDAC_IDAM0   (1 << 4)
#define MSCAN_CANIDAC_IDAM1   (1 << 5)

/** CANRXERR */
#define MSCAN_CANRXERR        14

/** CANTXERR */
#define MSCAN_CANTXERR        15

/** CANIDAR */
#define MSCAN_CANIDAR0        16
#define MSCAN_CANIDAR1        17
#define MSCAN_CANIDAR2        18
#define MSCAN_CANIDAR3        19
#define MSCAN_CANIDAR4        24
#define MSCAN_CANIDAR5        25
#define MSCAN_CANIDAR6        26
#define MSCAN_CANIDAR7        27 

/** CANIDMR */
#define MSCAN_CANIDMR0        20
#define MSCAN_CANIDMR1        21
#define MSCAN_CANIDMR2        22
#define MSCAN_CANIDMR3        23
#define MSCAN_CANIDMR4        28
#define MSCAN_CANIDMR5        29
#define MSCAN_CANIDMR6        30 
#define MSCAN_CANIDMR7        31 


/********************************************************************/

/** ID RX registers */
#define MSCAN_CANRXIDR0       32
#define MSCAN_CANRXIDR1       33
#define MSCAN_CANRXIDR2       34
#define MSCAN_CANRXIDR3       35

/** data RX registers */
#define MSCAN_CANRXDSR0       36
#define MSCAN_CANRXDSR1       37
#define MSCAN_CANRXDSR2       38
#define MSCAN_CANRXDSR3       39
#define MSCAN_CANRXDSR4       40
#define MSCAN_CANRXDSR5       41
#define MSCAN_CANRXDSR6       42
#define MSCAN_CANRXDSR7       43


/* DLC RX registers */
#define MSCAN_CANRXDLR        44


/********************************************************************/

/** ID TX registers */
#define MSCAN_CANTXIDR0       48
#define MSCAN_CANTXIDR1       49
#define MSCAN_CANTXIDR2       50
#define MSCAN_CANTXIDR3       51

/** data TX registers */
#define MSCAN_CANTXDSR0       52
#define MSCAN_CANTXDSR1       53
#define MSCAN_CANTXDSR2       54
#define MSCAN_CANTXDSR3       55
#define MSCAN_CANTXDSR4       56
#define MSCAN_CANTXDSR5       57
#define MSCAN_CANTXDSR6       58
#define MSCAN_CANTXDSR7       59


/* DLC TX registers */
#define MSCAN_CANTXDLR        60

/********************************************************************/

/** IDR1 */
#define MSCAN_IDR1_IDE_EXT  (1 << 3)
#define MSCAN_IDR1_RTR_BASE  (1 << 4)


/** IDR3 */
#define MSCAN_IDR3_RTR_EXT  (1 << 0)


/** DLR */
#define MSCAN_DLR_DLC_MASK  0x0F

/* interrupt entries 
-------------------------------------------------------------------*/
extern void codrvCanTransmitInterrupt(void);
extern void codrvCanErrorInterrupt( void);
extern void codrvCanReceiveInterrupt( void);


#endif /* CODRV_MSCAN_H */

