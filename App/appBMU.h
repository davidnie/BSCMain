#ifndef _APP_BMU_H
#define _APP_BMU_H
#include "data.h"
#include "applicfg.h"
#include "bmu.h"
#include "task_main.h"

#define SOC_CAL_LOW 300
#define SOC_CAL_HIGH	700
// Full Power
#define FPchg 48000 /* charge full power (W) */
#define FPdis 63000 /* discharge full power (W) */
#define CPchg 24000 /* continuous charge power limit (W) */
#define CPdis 30000 /* continuous discharge power limit (W)*/
// Time Limits
#define tcountH 300 /* 0.1s hard time limit at full power */
#define tcountS 100 /* 0.1sec soft time limit at full power */
// Cell Voltage Limits
#define VhighH 4050 /* mv high voltage hard limit */
#define VhighS 3750 /* mv high voltage soft limit */
#define VlowH 3055 /* lmv ow voltage hard limit */
#define VLowM	3150
#define VlowS 3320 /* mv ow voltage soft limit */
	// Temperature Limits
#define ThighH 600 /* 0.1C high temperature hard limit */
#define ThighS 400 /*  0.1Chigh temperature soft limit */
#define TlowH -100 /*  0.1Clow temperature hard limit */
#define TlowS 100 /*  0.1Clow temperature soft limit */
// SOC Limits
#define SOChighH 00 /* 0.1% high SOC hard limit (charge) */
#define SOChighS 300 /*0.1%  high SOC soft limit (charge) */
#define SOClowH 20 /* 0.1% low SOC hard limit (discharge) */
#define SOClowM 250
#define SOClowS 600 /* 0.1% low SOC soft limit (discharge) */
#define SOClowHc 00 /*0.1%  low SOC hard limit (charge) */
#define SOClowSc 250 /* 0.1% low SOC soft limit (charge) */


/***************************************************************************/
/* SDO (un)packing macros */

/** Returns the command specifier (cs, ccs, scs) from the first byte of the SDO
*/
#define getSDOcs(byte) (byte >> 5)

/** Returns the number of bytes without data from the first byte of the SDO. Coded in 2 bits
*/
#define getSDOn2(byte) ((byte >> 2) & 3)

/** Returns the number of bytes without data from the first byte of the SDO. Coded in 3 bits
*/
#define getSDOn3(byte) ((byte >> 1) & 7)

/** Returns the transfer type from the first byte of the SDO
*/
#define getSDOe(byte) ((byte >> 1) & 1)

/** Returns the size indicator from the first byte of the SDO
*/
#define getSDOs(byte) (byte & 1)

/** Returns the indicator of end transmission from the first byte of the SDO
*/
#define getSDOc(byte) (byte & 1)

/** Returns the toggle from the first byte of the SDO
*/
#define getSDOt(byte) ((byte >> 4) & 1)

/** Returns the index from the bytes 1 and 2 of the SDO
*/
#define getSDOindex(byte1, byte2) (((UNS16)byte2 << 8) | ((UNS16)byte1))

/** Returns the subIndex from the byte 3 of the SDO
*/
#define getSDOsubIndex(byte3) (byte3)

/** Returns the subcommand in SDO block transfer
*/
#define getSDOblockSC(byte) (byte & 3)



parseBroadMsg(Message *d, UNS8 nodeId);
extern UNS8 getStrTCStat(UNS8 sid,FAW_ID_T tcid);

#endif

