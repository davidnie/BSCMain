/*
* eb_debug.h - debug functions for framework
*
* Copyright (c) 2014 emtas GmbH
*-------------------------------------------------------------------
* $Id: eb_ebc.c 5662 2014-04-16 14:35:11Z boe $
*
*
*-------------------------------------------------------------------
*
*
*/

const char *ebDbgGetCanStateStrg(UNSIGNED8	canState);
const char *ebDbgGetNmtStrg(UNSIGNED8 nmtState);
const char *ebDbgGetCommStateStrg(UNSIGNED8	nmtState);
const char *ebDbgGetDeviceStrg(UNSIGNED8 devType);
const char *ebDbgGetStateStrg(UNSIGNED16 state, UNSIGNED8 devType);
#ifdef EB_EBC_CNT
const char *ebDbgGetEbcHandlingStrg(EBC_HANDLING_STATE_T event);
const char *ebDbgGetEbcChargeStrg(EBC_CHARGE_EVENT_STATE_T event);
#endif /* EB_EBC_CNT */
