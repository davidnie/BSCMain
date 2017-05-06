/*
* co_dynod.c - contains dynamic od handling for UNSIGNED8 .. 32/64 and INTEGER8 .. 32 objects
*
* Copyright (c) 2015 emtas GmbH
*-------------------------------------------------------------------
* $Id: $
*
*
*-------------------------------------------------------------------
*/

/********************************************************************/
/**
* \file
* \brief This file implements a dynamic object dictionary for objects 
*        => 0x2000.
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#ifdef CO_DYNAMIC_OBJDIC

#include <co_datatype.h>
#include <co_odaccess.h>
#include "ico_dynod.h"

/* constant definitions
---------------------------------------------------------------------------*/

/* local defined data types
---------------------------------------------------------------------------*/
typedef struct {
	UNSIGNED16	actObjCnt;
	UNSIGNED16	maxObjCnt;
	UNSIGNED16	actDescCnt;
	UNSIGNED16	maxDescCnt;
	UNSIGNED16  objCnt;		/* max number of new objects */
	UNSIGNED16  u8Cnt;		/* actual number of U8 vars */
	UNSIGNED16  u8Max;		/* max number of U8 vars */
	UNSIGNED16  u16Cnt;		/* actual number of U16 vars */
	UNSIGNED16  u16Max;		/* max number of U16 vars */
	UNSIGNED16  u32Cnt;		/* actual number of U32 vars */
	UNSIGNED16  u32Max;		/* max number of U32 vars */
	UNSIGNED16  i8Cnt;		/* actual number of i8 vars */
	UNSIGNED16  i8Max;		/* max number of i8 vars */
	UNSIGNED16  i16Cnt;		/* actual number of i16 vars */
	UNSIGNED16  i16Max;		/* max number of i16 vars */
	UNSIGNED16  i32Cnt;		/* actual number of i32 vars */
	UNSIGNED16  i32Max;		/* max number of i32 vars */
#ifdef CO_EXTENDED_DATA_TYPES
	UNSIGNED16  u64Cnt;		/* actual number of U64 vars */
	UNSIGNED16  u64Max;		/* max number of U64 vars */
#endif /* CO_EXTENDED_DATA_TYPES */
	CO_OD_ASSIGN_T *pOdAssign;	/* object list */
	CO_OBJECT_DESC_T *pOdDesc;	/* description list */
	UNSIGNED8	**pU8;		/* pointer list of U8 */
	UNSIGNED16	**pU16;		/* pointer list of U16 */
	UNSIGNED32	**pU32;		/* pointer list of U32 */
	INTEGER8	**pI8;		/* pointer list of I8 */
	INTEGER16	**pI16;		/* pointer list of I16 */
	INTEGER32	**pI32;		/* pointer list of I32 */
#ifdef CO_EXTENDED_DATA_TYPES
	UNSIGNED64	**pU64;		/* pointer list of U64 */
#endif /* CO_EXTENDED_DATA_TYPES */
} CO_DYN_DATA_T;

/* list of external used functions, if not in headers
--------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static CO_DYN_DATA_T dynData;



/***************************************************************************/
/**
* \brief coDynOdInit - init dynamic object dictionary
*
* \retval RET_OK
*	initialisation OK
* \retval RET_EVENT_NO_RESSOURCE
*	error at malloc()
*/

RET_T coDynOdInit(
		UNSIGNED16  objCnt,		/**< number of new objects for can line */
		UNSIGNED16  u8Cnt,		/**< number of U8 vars for can line */
		UNSIGNED16  u16Cnt,		/**< number of U16 vars for can line */
		UNSIGNED16  u32Cnt,		/**< number of U32 vars for can line */
		UNSIGNED16  i8Cnt,		/**< number of i8 vars for can line */
		UNSIGNED16  i16Cnt,		/**< number of i16 vars for can line */
		UNSIGNED16  i32Cnt		/**< number of i32 vars for can line */
#ifdef CO_EXTENDED_DATA_TYPES
		,UNSIGNED16 u64Cnt		/**< number of U64 vars for can line */
#endif /* CO_EXTENDED_DATA_TYPES */
    )
{
	memset(&dynData, 0, sizeof(CO_DYN_DATA_T));

	dynData.maxObjCnt = objCnt;
	dynData.u8Max = u8Cnt;
	dynData.u16Max = u16Cnt;
	dynData.u32Max = u32Cnt;
	dynData.i8Max = i8Cnt;
	dynData.i16Max = i16Cnt;
	dynData.i32Max = i32Cnt;
#ifdef CO_EXTENDED_DATA_TYPES
	dynData.u64Max = u64Cnt;
#endif /* CO_EXTENDED_DATA_TYPES */

	dynData.maxDescCnt = u8Cnt + u16Cnt + u32Cnt
			+ i8Cnt + i16Cnt + i32Cnt;
#ifdef CO_EXTENDED_DATA_TYPES
	dynData.maxDescCnt += u64Cnt;
#endif /* CO_EXTENDED_DATA_TYPES */

	/* get memory for od assign list */
	dynData.pOdAssign = (CO_OD_ASSIGN_T *)malloc(objCnt * sizeof(CO_OD_ASSIGN_T));
	if (dynData.pOdAssign == NULL)  {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* get memory for subindex description */
	dynData.pOdDesc = (CO_OBJECT_DESC_T*)malloc(dynData.maxDescCnt * sizeof(CO_OBJECT_DESC_T));

	/* get memory for data pointer */
	if (u8Cnt != 0u)  {
		/* get memory */
		dynData.pU8 = (UNSIGNED8 **)malloc(u8Cnt * sizeof(UNSIGNED8 *));
		if (dynData.pU8 == NULL)  {
			return(RET_EVENT_NO_RESSOURCE);
		}
	}
	if (u16Cnt != 0u)  {
		/* get memory */
		dynData.pU16 = (UNSIGNED16 **)malloc(u16Cnt * sizeof(UNSIGNED16 *));
		if (dynData.pU16 == NULL)  {
			return(RET_EVENT_NO_RESSOURCE);
		}
	}
	if (u32Cnt != 0u)  {
		/* get memory */
		dynData.pU32 = (UNSIGNED32 **)malloc(u32Cnt * sizeof(UNSIGNED32 *));
		if (dynData.pU32 == NULL)  {
			return(RET_EVENT_NO_RESSOURCE);
		}
	}
	if (i8Cnt != 0u)  {
		/* get memory */
		dynData.pI8 = (INTEGER8 **)malloc(i8Cnt * sizeof(INTEGER8 *));
		if (dynData.pI8 == NULL)  {
			return(RET_EVENT_NO_RESSOURCE);
		}
	}
	if (i16Cnt != 0u)  {
		/* get memory */
		dynData.pI16 = (INTEGER16 **)malloc(i16Cnt * sizeof(INTEGER16 *));
		if (dynData.pI16 == NULL)  {
			return(RET_EVENT_NO_RESSOURCE);
		}
	}
	if (i32Cnt != 0u)  {
		/* get memory */
		dynData.pI32 = (INTEGER32 **)malloc(i32Cnt * sizeof(INTEGER32 *));
		if (dynData.pI32 == NULL)  {
			return(RET_EVENT_NO_RESSOURCE);
		}
	}

#ifdef CO_EXTENDED_DATA_TYPES
	if (u64Cnt != 0u)  {
		/* get memory */
		dynData.pU64 = (UNSIGNED64 **)malloc(u64Cnt * sizeof(UNSIGNED64 *));
		if (dynData.pU64 == NULL)  {
			return(RET_EVENT_NO_RESSOURCE);
		}
	}
#endif /* CO_EXTENDED_DATA_TYPES */

	return(RET_OK);
}


/***************************************************************************/
/**
* \brief coDynOdRelease - release dynamic object dictionary
*
* Deinit dynamic object dictionary and release all requested memory
*
* \retval RET_OK
*	deinitialisation OK
*/
RET_T coDynOdRelease(
		void	/* no parameter */
	)
{
	/* release memory for od assign list */
	if (dynData.pOdAssign != NULL)  {
		free(dynData.pOdAssign);
	}

	/* release memory for subindex description */
	if (dynData.pOdDesc != NULL)  {
		free(dynData.pOdDesc);
	}

	/* release memory */
	if (dynData.pU8 != NULL)  {
		free(dynData.pU8);
	}
	if (dynData.pU16 != NULL)  {
		free(dynData.pU16);
	}
	if (dynData.pU32 != NULL)  {
		free(dynData.pU32);
	}
	if (dynData.pI8 != NULL)  {
		free(dynData.pI8);
	}
	if (dynData.pI16 != NULL)  {
		free(dynData.pI16);
	}
	if (dynData.pI32 != NULL)  {
		free(dynData.pI32);
	}

#ifdef CO_EXTENDED_DATA_TYPES
	if (dynData.pU64 != NULL)  {
		free(dynData.pU64);
	}
#endif /* CO_EXTENDED_DATA_TYPES */

	/* delete all data */
	memset(&dynData, 0, sizeof(CO_DYN_DATA_T));

	return(RET_OK);
}


/***************************************************************************/
/**
* \brief coDynOdAddIndex - add a new object index
*
* \retval RET_IDX_NOT_FOUND
*	index < 0x2000 are not allowed
* \retval RET_INVALID_PARAMETER
*	index already exist
* \retval RET_EVENT_NO_RESSOURCE
*	no resource available
*/

RET_T coDynOdAddIndex(
		UNSIGNED16  index,			/**< index */
		UNSIGNED8   nrOfSubs,		/**< number of subindex */
		CO_ODTYPE_T	odType			/**< variable, array, struct */
	)
{
UNSIGNED16  idx = 0xffffu;
UNSIGNED16	i;
CO_DYN_DATA_T *pDyn = &dynData;
	
	/* only index >= 0x2000 are allowed */
	if (index < 0x2000u)  {
		return(RET_IDX_NOT_FOUND);
	}

	/* check, if index is already listed */
	for (i = 0u; i < pDyn->objCnt; i++)  {
		if (pDyn->pOdAssign[i].index == index)  { /*lint !e960 Note pointer arithmetic other than array indexing used */
			idx = i;
			break;
		}
	}
	/* if it already exist, abort */
	if (idx != 0xffffu)  {
		return(RET_INVALID_PARAMETER);
	}

	/* check for free entry */
	if (pDyn->actObjCnt == pDyn->maxObjCnt)  {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* not listed */
	/* enough desc list entries available ? */
	if ((pDyn->actDescCnt + nrOfSubs) > pDyn->maxDescCnt)  {
		/* no, abort */
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* not listed, add it into sorted list */
	for (idx = pDyn->actObjCnt; idx > 0u; idx--)  {
		if (index < pDyn->pOdAssign[idx - 1u].index)  {
			memcpy(&pDyn->pOdAssign[idx], &pDyn->pOdAssign[idx - 1u],
				   sizeof(CO_OD_ASSIGN_T));
		} else {
			break;
		}
	}
	pDyn->pOdAssign[idx].index = index;
	pDyn->pOdAssign[idx].numberOfSubs = nrOfSubs;
	pDyn->pOdAssign[idx].highestSub = 0u;
	pDyn->pOdAssign[idx].odType = odType;
	pDyn->pOdAssign[idx].odDescIdx = pDyn->actDescCnt;
	pDyn->pOdDesc[pDyn->actDescCnt].subIndex = 0xffu;
	pDyn->actDescCnt += nrOfSubs;
	pDyn->actObjCnt ++;

	return(RET_OK);
}


/***************************************************************************/
/**
* \brief coDynOdAddSubIndex - add new subindex
*
* no check for to many data or duplicate subindex
*
* \retval RET_DATA_TYPE_MISMATCH
*	data type not supported (only U8, U16, U32, I8, I16, I32 allowed)
* \retval RET_IDX_NOT_FOUND
*	index not found
*/

RET_T coDynOdAddSubIndex(
		UNSIGNED16  index,			/**< index */
		UNSIGNED8   subIndex,		/**< number of subindex */
		CO_DATA_TYPE_T  dataType,	/**< data type */
		UNSIGNED16  attr,			/**< attribute */
		void        *pVar			/**< pointer to variable */
	)
{
UNSIGNED16  idx = 0xffffu;
UNSIGNED16	i;
UNSIGNED16	u16;
UNSIGNED16	cnt, max;
CO_DYN_DATA_T *pDyn = &dynData;
CO_OD_ASSIGN_T *pOdAssign;
CO_OBJECT_DESC_T *pDesc;

	/* check if pointer is != 0 */
	if (pVar == NULL) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* check for allowed data types */
	switch (dataType)  {
		case CO_DTYPE_U8_PTR:
			cnt = pDyn->u8Cnt;
			max = pDyn->u8Max;
			break;
		case CO_DTYPE_U16_PTR:
			cnt = pDyn->u16Cnt;
			max = pDyn->u16Max;
			break;
		case CO_DTYPE_U32_PTR:
			cnt = pDyn->u32Cnt;
			max = pDyn->u32Max;
			break;
		case CO_DTYPE_I8_PTR:
			cnt = pDyn->i8Cnt;
			max = pDyn->i8Max;
			break;
		case CO_DTYPE_I16_PTR:
			cnt = pDyn->i16Cnt;
			max = pDyn->i16Max;
			break;
		case CO_DTYPE_I32_PTR:
			cnt = pDyn->i32Cnt;
			max = pDyn->i32Max;
			break;
#ifdef CO_EXTENDED_DATA_TYPES
		case CO_DTYPE_U64_PTR:
			cnt = pDyn->u64Cnt;
			max = pDyn->u64Max;
			break;
#endif /* CO_EXTENDED_DATA_TYPES */
		default:
			return(RET_DATA_TYPE_MISMATCH);
	}

	/* free entry for data pointer available ? */
	if (cnt >= max)  {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* search index */
	for (i = 0u; i < pDyn->actObjCnt; i++)  {
		if (pDyn->pOdAssign[i].index == index)  {
			idx = i;
			break;
		}
	}
	if (idx == 0xffffu)  {
		return(RET_IDX_NOT_FOUND);
	}

	pOdAssign = &pDyn->pOdAssign[idx];

	/* set offset for start subindex desc */
	pDesc = &pDyn->pOdDesc[pOdAssign->odDescIdx];

    /* search subindex */
	for (i = 0u; i < pOdAssign->numberOfSubs; i++)  {
		if (subIndex < pDesc[i].subIndex)  {
			/* move all subs one to the end - if possible */
			u16 = pOdAssign->numberOfSubs - i;
			u16 -= 1u;
			memmove(&pDesc[i + 1u], &pDesc[i], u16 * sizeof(CO_OBJECT_DESC_T));
			break;
		}
	}

	/* save here my data */
	pDesc[i].subIndex = subIndex;
	pDesc[i].dType = dataType;
	pDesc[i].attr = attr | CO_ATTR_DYNOD;

	switch (dataType)  {
		case CO_DTYPE_U8_PTR:
			pDyn->pU8[cnt] = (UNSIGNED8 *)pVar;
			pDyn->u8Cnt ++;
			break;
		case CO_DTYPE_U16_PTR:
			pDyn->pU16[cnt] = (UNSIGNED16 *)pVar;
			pDyn->u16Cnt ++;
			break;
		case CO_DTYPE_U32_PTR:
			pDyn->pU32[cnt] = (UNSIGNED32 *)pVar;
			pDyn->u32Cnt ++;
			break;
		case CO_DTYPE_I8_PTR:
			pDyn->pI8[cnt] = (INTEGER8 *)pVar;
			pDyn->i8Cnt ++;
			break;
		case CO_DTYPE_I16_PTR:
			pDyn->pI16[cnt] = (INTEGER16 *)pVar;
			pDyn->i16Cnt ++;
			break;
		case CO_DTYPE_I32_PTR:
			pDyn->pI32[cnt] = (INTEGER32 *)pVar;
			pDyn->i32Cnt ++;
			break;
#ifdef CO_EXTENDED_DATA_TYPES
		case CO_DTYPE_U64_PTR:
			pDyn->pU64[cnt] = (UNSIGNED64 *)pVar;
			pDyn->u64Cnt ++;
			break;
#endif /* CO_EXTENDED_DATA_TYPES */
		default:
			break;
	}
	pDesc[i].tableIdx = cnt;

	/* setup highest subindex */
	if (subIndex > pOdAssign->highestSub)  {
		pOdAssign->highestSub = subIndex;
	}

	return(RET_OK);
}


/***************************************************************************/
/**
* \brief coDynOdSetSubIndexAddr - set new pointer for subindex
*
* set a new data pointer for a given sub index 
*
* \retval RET_DATA_TYPE_MISMATCH
*	data type not supported (only U8, U16, U32, I8, I16, I32 allowed)
* \retval RET_IDX_NOT_FOUND
*	index not found
*/

RET_T coDynOdSetSubIndexAddr(
		UNSIGNED16  index,			/**< index */
		UNSIGNED8   subIndex,		/**< number of subindex */
		CO_DATA_TYPE_T  dataType,	/**< data type */
		void        *pVar			/**< pointer to variable */
	)
{
CO_CONST CO_OBJECT_DESC_T *pDesc;
RET_T retVal;

	retVal = icoDynOdGetObjDescPtr(index, subIndex, &pDesc);
	
	if (retVal != RET_OK) {
		return(RET_IDX_NOT_FOUND);
	}
	
	if (pDesc->dType != dataType) {
		return(RET_DATA_TYPE_MISMATCH);
	}

	switch (pDesc->dType)  {
	case CO_DTYPE_U8_PTR:
		dynData.pU8[pDesc->tableIdx] = pVar; /*lint !e960 Note: pointer arithmetic other than array indexing used */
		break;
	case CO_DTYPE_U16_PTR:
		dynData.pU16[pDesc->tableIdx] = pVar; /*lint !e960 Note: pointer arithmetic other than array indexing used */
		break;
	case CO_DTYPE_U32_PTR:
		dynData.pU32[pDesc->tableIdx] = pVar; /*lint !e960 Note: pointer arithmetic other than array indexing used */
		break;
	case CO_DTYPE_I8_PTR:
		dynData.pI8[pDesc->tableIdx] = pVar; /*lint !e960 Note: pointer arithmetic other than array indexing used */
		break;
	case CO_DTYPE_I16_PTR:
		dynData.pI16[pDesc->tableIdx] = pVar; /*lint !e960 Note: pointer arithmetic other than array indexing used */
		break;
	case CO_DTYPE_I32_PTR:
		dynData.pI32[pDesc->tableIdx] = pVar; /*lint !e960 Note: pointer arithmetic other than array indexing used */
		break;
	default:
		return(RET_DATA_TYPE_MISMATCH);
		break;
	}
	
	return(RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoDynOdGetObjDescPtr - get dynod obj description pointer
*
* \return RET_T
*
*/
RET_T icoDynOdGetObjDescPtr(
		UNSIGNED16 index,
		UNSIGNED8 subIndex,
		CO_CONST CO_OBJECT_DESC_T **pDescPtr
	)
{
CO_DYN_DATA_T *pDyn = &dynData;
CO_OBJECT_DESC_T *pDesc;
UNSIGNED16	i, j;

	/* for all objects form this line */
	for (i = 0u; i < pDyn->actObjCnt; i++)  {
		/* index found ? */
		if (pDyn->pOdAssign[i].index == index)  {
			/* for all subindex */
			for (j = 0u; j < pDyn->pOdAssign[i].numberOfSubs; j++)  {
				pDesc = &pDyn->pOdDesc[pDyn->pOdAssign[i].odDescIdx];
				/* subindex found ? */
				if (pDesc[j].subIndex == subIndex)  {
					/* found */
					*pDescPtr = &pDesc[j];
					return(RET_OK);
				}
			}
			return(RET_SUBIDX_NOT_FOUND);
		}
	}

	return(RET_IDX_NOT_FOUND);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoDynOdGetObjAddr - return dynod object adresse
*
* \return RET_T
*
*/
UNSIGNED8 *icoDynOdGetObjAddr(
		CO_CONST CO_OBJECT_DESC_T	*pDesc  /* pointer for description index */
	)
{
void *pObjAddr = NULL;

	switch (pDesc->dType)  {
		case CO_DTYPE_U8_PTR:
			if (dynData.pU8 != NULL)  {
				pObjAddr = dynData.pU8[pDesc->tableIdx]; /*lint !e960 Note: pointer arithmetic other than array indexing used */
			}
			break;
		case CO_DTYPE_U16_PTR:
			if (dynData.pU16 != NULL)  {
				pObjAddr = dynData.pU16[pDesc->tableIdx]; /*lint !e960 Note: pointer arithmetic other than array indexing used */
			}
			break;
		case CO_DTYPE_U32_PTR:
			if (dynData.pU32 != NULL)  {
				pObjAddr = dynData.pU32[pDesc->tableIdx]; /*lint !e960 Note: pointer arithmetic other than array indexing used */
			}
			break;
		case CO_DTYPE_I8_PTR:
			if (dynData.pI8 != NULL)  {
				pObjAddr = dynData.pI8[pDesc->tableIdx]; /*lint !e960 Note: pointer arithmetic other than array indexing used */
			}
			break;
		case CO_DTYPE_I16_PTR:
			if (dynData.pI16 != NULL)  {
				pObjAddr = dynData.pI16[pDesc->tableIdx]; /*lint !e960 Note: pointer arithmetic other than array indexing used */
			}
			break;
		case CO_DTYPE_I32_PTR:
			if (dynData.pI32 != NULL)  {
				pObjAddr = dynData.pI32[pDesc->tableIdx]; /*lint !e960 Note: pointer arithmetic other than array indexing used */
			}
			break;
#ifdef CO_EXTENDED_DATA_TYPES
		case CO_DTYPE_U64_PTR:
			if (dynData.pU64 != NULL)  {
				pObjAddr = dynData.pU64[pDesc->tableIdx]; /*lint !e960 Note: pointer arithmetic other than array indexing used */
			}
			break;
#endif /* CO_EXTENDED_DATA_TYPES */
		default:
			break;
	}

	return((UNSIGNED8 *)pObjAddr);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoDynOdGetObjSize - return size for dynamic object
*
* \return RET_T
*
*/

UNSIGNED32 icoDynOdGetObjSize(
		CO_CONST CO_OBJECT_DESC_T	*pDesc	/* pointer for description index */
	)
{
UNSIGNED32	size = 0u;

	switch (pDesc->dType)  {
		case CO_DTYPE_U8_PTR:
		case CO_DTYPE_I8_PTR:
				size = 1u;
			break;
		case CO_DTYPE_U16_PTR:
		case CO_DTYPE_I16_PTR:
				size = 2u;
			break;
		case CO_DTYPE_U32_PTR:
		case CO_DTYPE_I32_PTR:
				size = 4u;
			break;
#ifdef CO_EXTENDED_DATA_TYPES
		case CO_DTYPE_U64_PTR:
				size = 8u;
			break;
#endif /* CO_EXTENDED_DATA_TYPES */
		default:
			break;
	}

	return(size);
}
#endif /* CO_DYNAMIC_OBJDIC */
