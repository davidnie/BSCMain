/*
* ico_sdo.h - contains internal defines for SDO
*
* Copyright (c) 2012-2016 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_sdo.h 13645 2016-04-28 12:56:51Z boe $

*-------------------------------------------------------------------
*
* Changelog:
*
*
*/

/**
* \file
* \brief dataty type header
*/

#ifndef ICO_SDO_H
#define ICO_SDO_H 1

/* constants */
#define CO_SDO_CS_ABORT				0x80u
#define CO_SDO_CCS_MASK				0xe0u	/* SDO CCS Mask */
#define CO_SDO_CCS_TOGGLE_BIT		0x10u
#define CO_SDO_CCS_CONT_BIT			0x01u
#define CO_SDO_CCS_FD_CONT_BIT		0x01u
#define CO_SDO_CCS_LEN_MASK			0x0eu
#define CO_SDO_CCS_WRITE_TYPE_MASK	0x03u
#define CO_SDO_CCS_DOWNLOAD_INIT	0x20u	/* init download */
#define CO_SDO_CCS_DOWNLOAD_SEGMENT	0x00u	/* segment download */
#define CO_SDO_CCS_UPLOAD_INIT		0x40u	/* init upload */
#define CO_SDO_CCS_UPLOAD_SEGMENT	0x60u	/* segment upload */
#define CO_SDO_CCS_BLOCK_UPLOAD		0xa0u	/* block upload */
#define CO_SDO_CCS_BLOCK_DOWNLOAD	0xc0u	/* block download */
#define CO_SDO_CCS_NET_IND			0xe0u	/* SDO Network Indikation */
#define CO_SDO_CCS_FD_UPLOAD_INIT	0x40u	/* fd upload init */
#define CO_SDO_CCS_FD_UPLOAD_SEGMENT 0x60u	/* fd upload segment */
#define CO_SDO_CCS_FD_DOWNLOAD_INIT	0x20u	/* fd init download */

#define CO_SDO_CCS_BLOCK_UL_CRC		0x04u	/* bit for CRC */
#define CO_SDO_CCS_BLOCK_DL_CRC		0x04u	/* bit for CRC */
#define CO_SDO_CCS_BLOCK_DL_SIZE	0x02u	/* bit size indicated */

/* FD byte 0 */
#define CO_SDO_CCS_FD_MSUB			0x01u	/* multiple subindex */
#define CO_SDO_CCS_FD_MFL_MASK		0x0fu	/* max frame len as DLC */

/* client subcommand */
#define CO_SDO_CCS_BLOCK_SC_MASK	0x03u	/* block subcommand mask */
#define CO_SDO_CCS_BLOCK_SC_UL_INIT	0x00u	/* block upload init */
#define CO_SDO_CCS_BLOCK_SC_UL_END	0x01u	/* block upload end */
#define CO_SDO_CCS_BLOCK_SC_UL_CON	0x02u	/* block upload confirmation */
#define CO_SDO_CCS_BLOCK_SC_UL_BLK	0x03u	/* block upload blk */
#define CO_SDO_CCS_BLOCK_CS_MASK	0x01u	/* block subcommand mask */
#define CO_SDO_CCS_BLOCK_CS_DL_INIT	0x00u	/* block download init */
#define CO_SDO_CCS_BLOCK_CS_DL_END	0x01u	/* block download end */
#define CO_SDO_CCS_BLOCK_DL_LAST	0x80u	/* last flag */

#define CO_SDO_SCS_UPLOAD_INIT		0x40u	/* init upload */
#define CO_SDO_SCS_UPLOAD_SEGMENT	0x00u	/* segment upload */
#define CO_SDO_SCS_DOWNLOAD_INIT	0x60u	/* init download */
#define CO_SDO_SCS_DOWNLOAD_SEGMENT	0x20u	/* segment download */
#define CO_SDO_SCS_LEN_INDICATED	0x01u	/* len indicated */
#define CO_SDO_SCS_EXPEDITED		0x02u	/* expedited transfer */
#define CO_SDO_SCS_CONT_FLAG		0x01u	/* cont flag */
#define CO_SDO_SCS_LEN_MASK			0x0eu	/* lenght mask */
#define CO_SDO_SCS_BLOCK_UPLOAD		0xc0u	/* block upload */
#define CO_SDO_SCS_BLOCK_DOWNLOAD	0xa0u	/* block download */
#define CO_SDO_SCS_FD_UPLOAD_INIT	0x40u	/* fd init upload */
#define CO_SDO_SCS_FD_EXPEDITED		0x02u	/* fd expedited transfer */
#define CO_SDO_SCS_FD_CONT_FLAG		0x01u	/* cont flag */
#define CO_SDO_SCS_NET_IND			0xe0u	/* SDO Network Indikation */

#define CO_SDO_SCS_BLOCK_SS_DL_INIT	0x00u	/* block download init */
#define CO_SDO_SCS_BLOCK_SS_UL_END	0x01u	/* block upload end */
#define CO_SDO_SCS_BLOCK_SS_DL_MASK	0x03u	/* block download mask */
#define CO_SDO_SCS_BLOCK_SS_DL_ACQ	0x02u	/* block download ACQ */
#define CO_SDO_SCS_BLOCK_SS_DL_END	0x01u	/* block download End */
#define CO_SDO_SCS_BLOCK_UL_SIZE	0x02u	/* len indicated */
#define CO_SDO_SCS_BLOCK_UL_CRC		0x04u	/* block upload CRC */
#define CO_SDO_SCS_BLOCK_UL_LAST	0x80u	/* last flag */
#define CO_SDO_SCS_BLOCK_DL_CRC		0x04u	/* block download CRC */
#define CO_SDO_SCS_BLOCK_DL_LAST	0x80u	/* last flag */

/* FD byte 4 */
#define CO_SDO_SCS_FD4_LEN_INDICATED 0x02u	/* len indicated */
#define CO_SDO_CCS_FD4_LEN_INDICATED 0x02u	/* len indicated */

#define CO_SDO_SCS_FD_HDR_LEN		8u		/* len of header without len */
#define CO_SDO_SCS_FD_SIZE_LEN		4u		/* len of size information */

/* datatypes */

/* function prototypes */

#ifdef CO_SDO_NETWORKING
void	icoSdoServerHandler(const CO_REC_DATA_T *pRecData,
			CO_SDO_SERVER_T *pLocalReqSdo);
#else /* CO_SDO_NETWORKING */
void	icoSdoServerHandler(const CO_REC_DATA_T *pRecData);
#endif /* CO_SDO_NETWORKING */
void	*icoSdoGetObjectAddr(UNSIGNED16 sdoNr, UNSIGNED8 subIndex);

RET_T	icoSdoCheckObjLimitNode(UNSIGNED16 sdoNr);
RET_T	icoSdoCheckObjLimitCobId(UNSIGNED16 sdoNr, UNSIGNED8 subIndex, UNSIGNED32 canId);
RET_T	icoSdoObjChanged(UNSIGNED16 sdoNr, UNSIGNED8 subIndex);

void	icoSdoServerReset(void);
void	icoSdoServerSetDefaultValue(void);
void	icoSdoClientReset(void);
void	icoSdoClientSetDefaultValue(void);

void	icoSdoClientHandler(const CO_REC_DATA_T *pRecData);
void	*icoSdoClientGetObjectAddr(UNSIGNED8 sdoNr, UNSIGNED8 subIndex);
RET_T	icoSdoClientCheckObjLimitNode(UNSIGNED16 sdoNr);
RET_T	icoSdoClientCheckObjLimitCobId(UNSIGNED16 sdoNr, UNSIGNED8	subIndex,
			UNSIGNED32	canId);
RET_T	icoSdoClientObjChanged(UNSIGNED16 sdoNr, UNSIGNED8 subIndex);

UNSIGNED16 icoSdoCrcCalc(UNSIGNED16 crc, const UNSIGNED8 *pData, UNSIGNED32 len);
void	icoSdoClientQueueInd(UNSIGNED8	sdoNr, UNSIGNED16 index,
			UNSIGNED8 subIndex, UNSIGNED32	result);

#endif /* ICO_SDO_H */
