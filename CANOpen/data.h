/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __data_h__
#define __data_h__

#ifdef __cplusplus
extern "C" {
#endif

/* declaration of CO_Data type let us include all necessary headers
 struct struct_CO_Data can then be defined later
 */
typedef struct struct_CO_Data CO_Data;

#include "applicfg.h"
#include "def.h"
#include "can.h"
#include "objdictdef.h"
#include "objacces.h"
#ifdef CO_ENABLE_LSS
#include "lss.h"
#endif

/**
 * @ingroup od
 * @brief This structure contains all necessary informations to define a CANOpen node 
 */
struct struct_CO_Data {
	int a;
};


#endif /* __data_h__ */


