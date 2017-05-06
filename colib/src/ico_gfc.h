/*
* ico_gfc.h - contains defines for global failsafe services
*
* Copyright (c) 2015-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_gfc.h 12657 2016-03-04 16:03:45Z boe $

*-------------------------------------------------------------------
*
* Changelog:
*
*
*/

/**
* \file
* \brief data type header
*/

#ifndef ICO_GFC_H
#define ICO_GFC_H 1


/* datatypes */

/* function prototypes */

void	icoGfcReset(void);
void	icoGfcSetDefaultValue(void);
void	*icoGfcGetObjectAddr(void);
RET_T	icoGfcCheckObjLimit_u8(UNSIGNED8	value);
void	icoGfcHandler(void);
void	icoGfcVarInit(void);

#endif /* ICO_GFC_H */

