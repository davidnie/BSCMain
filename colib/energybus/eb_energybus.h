/*
* eb_energybus.h - contains defines for all services
*
* Copyright (c) 2013-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: eb_energybus.h 9850 2015-07-09 15:16:27Z boe $

*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for all services
* This header inludes defines for all services of the energybus framework.
* It can be included instead of header files of each service.
*
* \file eb_energybus.h
*	contains defines for all services
*/

#ifndef EB_ENERGYBUS_H
#define EB_ENERGYBUS_H 1


#include <eb_data.h>
#include <eb_eeprom.h>
#include <eb_fct.h>

#ifdef EB_EBC_CNT
# include <eb_ebc.h>
#endif /* EB_EBC_CNT */

#if defined(EB_MCU_CNT) || defined(EB_NETWORK_MCU_CNT)
# include <eb_mcu.h>
#endif /* EB_MCU_CNT */

#if defined(EB_HMI_CNT) || defined(EB_NETWORK_HMI_CNT)
# include <eb_hmi.h>
#endif /* EB_HMI_CNT */

#if defined(EB_BATTERY_CNT) || defined(EB_NETWORK_BATTERY_CNT)
# include <eb_battery.h>
#endif /* EB_BATTERY_CNT */

#if defined(EB_SECURITY_CNT) || defined(EB_NETWORK_SECURITY_CNT)
# include <eb_sec.h>
#endif /* EB_SECURITY_CNT */

#if defined(EB_VOLT_CONV_CNT) || defined(EB_NETWORK_VOLT_CONV_CNT)
# include <eb_voltconv.h>
#endif /* EB_VOLTCONV_CNT */

#include <eb_debug.h>


#define EB_FW_VERSION	0x020301ul

extern UNSIGNED32		emtasVendorId;
extern UNSIGNED32		emtasCANopenVersion;
extern UNSIGNED32		emtasEbFwVersion;

#endif /* EB_ENERGYBUS_H */
