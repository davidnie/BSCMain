/*
* co_emcy.h - contains defines for emcy services
*
* Copyright (c) 2012-2014 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_emcy.h 12546 2016-02-29 10:26:29Z boe $

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

#ifndef ICO_EMCY_H
#define ICO_EMCY_H 1

/* datatypes */

/* function prototypes */

void	icoEmcyReset(void);
void	icoEmcyProducerSetDefaultValue(void);
RET_T	icoEmcyCheckObjLimitCobid(UNSIGNED16 index, UNSIGNED8 subIndex,
			UNSIGNED32 canId);
RET_T	icoEmcyCheckObjLimitHist(UNSIGNED16 index, UNSIGNED8 subIndex,
			UNSIGNED8 u8);
void	*icoEmcyGetObjectAddr(UNSIGNED16 idx, UNSIGNED8	subIndex);
RET_T	icoEmcyWriteReq(UNSIGNED16 emcyErrCode, const UNSIGNED8 *pAddErrorBytes);
RET_T	icoEmcyObjChanged(UNSIGNED16 index, UNSIGNED8 subIndex);
void	icoEmcyConsumerHandler(const CO_REC_DATA_T *pRecData);
void	icoEmcyConsumerReset(void);
void	icoEmcyConsumerSetDefaultValue(void);
void	icoEmcyVarInit(UNSIGNED8 *pEmcyErrHistList, UNSIGNED8 *pEmcyConsList);

#endif /* ICO_EMCY_H */

