/*
* co_odaccess.h - contains defines for od access
*
* Copyright (c) 2012-2016 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_odaccess.h 15350 2016-09-23 13:30:24Z phi $
*
*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for OD access
*
* \file co_odaccess.h - contains defines for object dictionary access
*/

#ifndef OD_ACCESS_H
#define OD_ACCESS_H 1

#include <co_datatype.h>


/* datatypes */

/*lint -save -e961 empty Semaphore calls without OS */

/**
* function call to lock object dictionary
*/
#ifndef CO_OS_LOCK_OD
# define CO_OS_LOCK_OD()
#endif /* CO_OS_LOCK_OD */

/**
* function call to unlock object dictionary
*/
#ifndef CO_OS_UNLOCK_OD
# define CO_OS_UNLOCK_OD()
#endif /* CO_OS_LOCK_OD */

/*lint -restore */

/**
* object attributes 
*/
#define CO_ATTR_READ		0x0001u	/**< object readable */
#define CO_ATTR_WRITE		0x0002u	/**< object writeable */
#define CO_ATTR_NUM			0x0004u	/**< object is numeric */
#define CO_ATTR_MAP			0x0008u	/**< object can be mapped into a PDO */
#define CO_ATTR_MAP_TR		0x0010u	/**< object can be mapped into a transmit PDO */
#define CO_ATTR_MAP_REC		0x0020u	/**< object can be mapped into a receive PDO */
#define CO_ATTR_DEFVAL		0x0040u	/**< object has a default value */
#define CO_ATTR_LIMIT		0x0080u	/**< object has limits */
#define CO_ATTR_DYNOD		0x0100u	/**< object is a dynamic created object */
#define CO_ATTR_STORE		0x0200u	/**< object is supposed to be stored */


/**
* Object type 
*/
typedef enum {
	CO_ODTYPE_VAR,		/**< variable */
	CO_ODTYPE_ARRAY,	/**< array */
	CO_ODTYPE_STRUCT	/**< structure */
} CO_ODTYPE_T;
 
/**
* object datatypes
*/
typedef enum {
	CO_DTYPE_BOOL_VAR,	CO_DTYPE_BOOL_CONST,	CO_DTYPE_BOOL_PTR,
	CO_DTYPE_U8_VAR,	CO_DTYPE_U8_CONST,	CO_DTYPE_U8_PTR,
	CO_DTYPE_U16_VAR,	CO_DTYPE_U16_CONST,	CO_DTYPE_U16_PTR,
	CO_DTYPE_U24_VAR,	CO_DTYPE_U24_CONST,	CO_DTYPE_U24_PTR,
	CO_DTYPE_U32_VAR,	CO_DTYPE_U32_CONST,	CO_DTYPE_U32_PTR,
	CO_DTYPE_U40_VAR,	CO_DTYPE_U40_CONST,	CO_DTYPE_U40_PTR,
	CO_DTYPE_U48_VAR,	CO_DTYPE_U48_CONST,	CO_DTYPE_U48_PTR,
	CO_DTYPE_U64_VAR,	CO_DTYPE_U64_CONST,	CO_DTYPE_U64_PTR,

	CO_DTYPE_I8_VAR,	CO_DTYPE_I8_CONST,	CO_DTYPE_I8_PTR,
	CO_DTYPE_I16_VAR,	CO_DTYPE_I16_CONST,	CO_DTYPE_I16_PTR,
	CO_DTYPE_I32_VAR,	CO_DTYPE_I32_CONST,	CO_DTYPE_I32_PTR,

	CO_DTYPE_VS_PTR,	CO_DTYPE_VS_CONST,
	CO_DTYPE_OS_PTR,	CO_DTYPE_DOMAIN,
	CO_DTYPE_U8_SDO_SERVER,	CO_DTYPE_U32_SDO_SERVER,
	CO_DTYPE_U8_SDO_CLIENT,	CO_DTYPE_U32_SDO_CLIENT,
	CO_DTYPE_U8_TPDO,	CO_DTYPE_U16_TPDO,	CO_DTYPE_U32_TPDO,
	CO_DTYPE_U8_RPDO,	CO_DTYPE_U16_RPDO,	CO_DTYPE_U32_RPDO,
	CO_DTYPE_U8_TMAP,	CO_DTYPE_U32_TMAP,
	CO_DTYPE_U8_RMAP,	CO_DTYPE_U32_RMAP,
	CO_DTYPE_U8_ERRCTRL,CO_DTYPE_U16_ERRCTRL,	CO_DTYPE_U32_ERRCTRL,
	CO_DTYPE_U8_SYNC,	CO_DTYPE_U32_SYNC,
	CO_DTYPE_U8_EMCY,	CO_DTYPE_U16_EMCY,	CO_DTYPE_U32_EMCY,
	CO_DTYPE_U16_SRD,	CO_DTYPE_U32_SRD,
	CO_DTYPE_U8_NMT,	CO_DTYPE_U16_NMT,	CO_DTYPE_U32_NMT,
	CO_DTYPE_U16_FLYMA,
	CO_DTYPE_U8_NETWORK, CO_DTYPE_U16_NETWORK, CO_DTYPE_U32_NETWORK,
	CO_DTYPE_U8_SRDO,	CO_DTYPE_U16_SRDO,	CO_DTYPE_U32_SRDO,
	CO_DTYPE_U32_TIME,
	CO_DTYPE_U32_STORE,
	CO_DTYPE_R32_VAR,	CO_DTYPE_R32_CONST, CO_DTYPE_R32_PTR,
	CO_DTYPE_U8_GFC
} CO_DATA_TYPE_T;


/** \brief function pointer to object changed function 
 * \param index - object index
 * \param subindex - object subindex
 * 
 * \return RET_T
 */
typedef RET_T (* CO_EVENT_OBJECT_CHANGED_FCT_T)(UNSIGNED16, UNSIGNED8); /*lint !e960 customer specific parameter names */


/**
* object description
*/
typedef struct {
	UNSIGNED8		subIndex;		/**< subindex */
	CO_DATA_TYPE_T	dType;			/**< datatype and table (var, const, ptr) */
	UNSIGNED16		tableIdx;		/**< index at var table (descibed by DATA_TYPE_T typ) */
	UNSIGNED16		attr;			/**< object attributes like e.g. acccess rights */
	UNSIGNED16		defValIdx;		/**< index at const table for default value */
#ifdef CO_CONFIG_LIMIT_CHECK
	UNSIGNED16		limitMinIdx;	/**< index at const table for min limit val */
	UNSIGNED16		limitMaxIdx;	/**< index at const table for max limit val */
#endif /* CO_CONFIG_LIMIT_CHECK */
} CO_NV_STORAGE CO_OBJECT_DESC_T;


/**
* object dictionary
*/
typedef struct {
	UNSIGNED16	index;				/**< object index */
	UNSIGNED8	numberOfSubs;		/**< number of subindex */
	UNSIGNED8	highestSub;			/**< highest used subindex */
	CO_ODTYPE_T	odType;				/**< variable, array, struct */
	UNSIGNED16	odDescIdx;			/**< index in object_description table */
} CO_NV_STORAGE CO_OD_ASSIGN_T;


/* data variables from OD */
typedef struct {
	CO_CONST UNSIGNED8	 *odConst_u8;
	CO_CONST UNSIGNED16 *odConst_u16;
	CO_CONST UNSIGNED32 *odConst_u32;
	CO_CONST REAL32	*odConst_r32;
#ifdef CO_EXTENDED_DATA_TYPES
	CO_CONST UNSIGNED24	*odConst_u24;
	CO_CONST UNSIGNED40	*odConst_u40;
	CO_CONST UNSIGNED48	*odConst_u48;
	CO_CONST UNSIGNED64	*odConst_u64;
#endif /* CO_EXTENDED_DATA_TYPES */

	UNSIGNED8	*odVar_u8;
	UNSIGNED16	*odVar_u16;
	UNSIGNED32	*odVar_u32;
	REAL32		*odVar_r32;
#ifdef CO_EXTENDED_DATA_TYPES
	UNSIGNED24	*odVar_u24;
	UNSIGNED40	*odVar_u40;
	UNSIGNED48	*odVar_u48;
	UNSIGNED64	*odVar_u64;
#endif /* CO_EXTENDED_DATA_TYPES */

	CO_CONST INTEGER8	*odConst_i8;
	CO_CONST INTEGER16	*odConst_i16;
	CO_CONST INTEGER32	*odConst_i32;
	INTEGER8	*odVar_i8;
	INTEGER16	*odVar_i16;
	INTEGER32	*odVar_i32;
	UNSIGNED8	* CO_CONST *odPtr_u8;
	UNSIGNED16	* CO_CONST *odPtr_u16;
	UNSIGNED32	* CO_CONST *odPtr_u32;
#ifdef CO_EXTENDED_DATA_TYPES
	UNSIGNED24	* CO_CONST *odPtr_u24;
	UNSIGNED40	* CO_CONST *odPtr_u40;
	UNSIGNED48	* CO_CONST *odPtr_u48;
	UNSIGNED64	* CO_CONST *odPtr_u64;
#endif /* CO_EXTENDED_DATA_TYPES */

	INTEGER8	* CO_CONST *odPtr_i8;
	INTEGER16	* CO_CONST *odPtr_i16;
	INTEGER32	* CO_CONST *odPtr_i32;
	REAL32	        * CO_CONST *odPtr_r32;
	CO_CONST VIS_STRING	*odConstVisString;
	CO_CONST UNSIGNED32	*odConstVisStringLen;
	VIS_STRING			*odVisString;
	UNSIGNED32			*odVisStringLen;
	OCTET_STRING		**odOctetString;
	UNSIGNED32			*odOctetStringLen;
	CO_DOMAIN_PTR 		*domainPtr;
	UNSIGNED32			*domainLen;
} CO_NV_STORAGE CO_OD_DATA_VARIABLES_T;



/* function prototypes */

void coOdInitOdPtr(
		const CO_OD_ASSIGN_T	*pOdAssing,
		UNSIGNED16		 		odCnt,
		const CO_OBJECT_DESC_T	*pObjdesc,
		UNSIGNED16				 descCnt,
		CO_EVENT_OBJECT_CHANGED_FCT_T	*pEventPtr,
		const CO_OD_DATA_VARIABLES_T *pOdVarPointers
);



EXTERN_DECL RET_T coOdGetObj_u32(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED32 *pObj);
EXTERN_DECL RET_T coOdGetObj_u16(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED16 *pObj);
EXTERN_DECL RET_T coOdGetObj_u8(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED8 *pObj);
EXTERN_DECL RET_T coOdGetObj_i32(UNSIGNED16 index,
					UNSIGNED8 subIndex, INTEGER32 *pObj);
EXTERN_DECL RET_T coOdGetObj_i16(UNSIGNED16 index,
					UNSIGNED8 subIndex, INTEGER16 *pObj);
EXTERN_DECL RET_T coOdGetObj_i8(UNSIGNED16 index,
					UNSIGNED8 subIndex, INTEGER8 *pObj);
EXTERN_DECL RET_T coOdGetObj_r32(UNSIGNED16 index,
					UNSIGNED8 subIndex, REAL32 *pObj);
#ifdef CO_EXTENDED_DATA_TYPES
EXTERN_DECL RET_T coOdGetObj_u24(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED24 *pObj);
EXTERN_DECL RET_T coOdGetObj_u40(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED40 *pObj);
EXTERN_DECL RET_T coOdGetObj_u48(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED48 *pObj);
EXTERN_DECL RET_T coOdGetObj_u64(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED64 *pObj);
#endif /* CO_EXTENDED_DATA_TYPES */
EXTERN_DECL UNSIGNED16 coOdGetObjAttribute(CO_CONST CO_OBJECT_DESC_T *pObjDesc);
EXTERN_DECL RET_T coOdGetObjDescPtr(UNSIGNED16 index,
					UNSIGNED8 subIndex, CO_CONST CO_OBJECT_DESC_T ** pDescPtr);
EXTERN_DECL UNSIGNED32 coOdGetObjSize(CO_CONST CO_OBJECT_DESC_T *pDesc);

EXTERN_DECL RET_T coOdPutObj_u32(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED32 newVal);
EXTERN_DECL RET_T coOdPutObj_u16(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED16 newVal);
EXTERN_DECL RET_T coOdPutObj_u8(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED8 newVal);
EXTERN_DECL RET_T coOdPutObj_i32(UNSIGNED16 index,
					UNSIGNED8 subIndex, INTEGER32 newVal);
EXTERN_DECL RET_T coOdPutObj_i16(UNSIGNED16 index,
					UNSIGNED8 subIndex, INTEGER16 newVal);
EXTERN_DECL RET_T coOdPutObj_i8(UNSIGNED16 index,
					UNSIGNED8 subIndex, INTEGER8 newVal);
EXTERN_DECL RET_T coOdPutObj_r32(UNSIGNED16 index,
					UNSIGNED8 subIndex, REAL32 newVal);
#ifdef CO_EXTENDED_DATA_TYPES
EXTERN_DECL RET_T coOdPutObj_u24(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED24 newVal);
EXTERN_DECL RET_T coOdPutObj_u40(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED40 newVal);
EXTERN_DECL RET_T coOdPutObj_u48(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED48 newVal);
EXTERN_DECL RET_T coOdPutObj_u64(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED64 newVal);
#endif /* CO_EXTENDED_DATA_TYPES */

EXTERN_DECL void *coOdGetObjAddr(UNSIGNED16 index,
					UNSIGNED8 subIndex);

EXTERN_DECL RET_T coOdGetDefaultVal_u32(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED32	*pDefVal);
EXTERN_DECL RET_T coOdGetDefaultVal_u16(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED16 *pDefVal);
EXTERN_DECL RET_T coOdGetDefaultVal_u8(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED8 *pDefVal);


EXTERN_DECL RET_T coOdSetCobid(UNSIGNED16 index,
					UNSIGNED8 subIndex, UNSIGNED32	newCobId);
EXTERN_DECL RET_T coOdDomainAddrSet(UNSIGNED16 index,
					UNSIGNED8 subIndex, CO_DOMAIN_PTR pAddr, UNSIGNED32 size);
EXTERN_DECL RET_T coOdVisStringSet(UNSIGNED16 index,
					UNSIGNED8 subIndex, VIS_STRING	pAddr, UNSIGNED32 size);

EXTERN_DECL RET_T coEventRegister_OBJECT_CHANGED(
					CO_EVENT_OBJECT_CHANGED_FCT_T, UNSIGNED16, UNSIGNED8);

#ifdef CO_DYNAMIC_OBJDIC
EXTERN_DECL RET_T coDynOdInit(UNSIGNED16 objCnt,
					UNSIGNED16 u8Cnt, UNSIGNED16 u16Cnt, UNSIGNED16 u32Cnt,
					UNSIGNED16 i8Cnt, UNSIGNED16 i16Cnt, UNSIGNED16  i32Cnt
# ifdef CO_EXTENDED_DATA_TYPES
					,UNSIGNED16 u64Cnt
# endif /* CO_EXTENDED_DATA_TYPES */
					);
EXTERN_DECL RET_T coDynOdRelease(void);
EXTERN_DECL RET_T coDynOdAddIndex(UNSIGNED16 index,
					UNSIGNED8 nrOfSubs, CO_ODTYPE_T	odType);
EXTERN_DECL RET_T coDynOdAddSubIndex(UNSIGNED16 index,
					UNSIGNED8 subIndex, CO_DATA_TYPE_T dataType,
					UNSIGNED16 attr, void *pVar);
EXTERN_DECL RET_T coDynOdSetSubIndexAddr(UNSIGNED16 index, 
					UNSIGNED8 subIndex, CO_DATA_TYPE_T dataType, void *pVar);
#endif /* CO_DYNAMIC_OBJDIC */

#ifdef CO_STORE_SUPPORTED
EXTERN_DECL UNSIGNED32 coOdGetObjStoreFlagCnt(void);

EXTERN_DECL RET_T coOdGetObjStoreFlag(UNSIGNED32 idx,
					UNSIGNED16* pIndex, UNSIGNED8* pSubIndex);
#endif /* CO_STORE_SUPPORTED */

EXTERN_DECL CO_INLINE BOOL_T coNumMemcpyPack(void *pDest, CO_CONST void *pSrc,
					UNSIGNED32 size, UNSIGNED16 numeric, UNSIGNED8 packOffset);
EXTERN_DECL CO_INLINE void coNumMemcpyUnpack(void *pDest, CO_CONST void *pSrc,
					UNSIGNED32 size, UNSIGNED16 numeric, UNSIGNED8 packOffset);
EXTERN_DECL CO_INLINE BOOL_T coNumMemcpy(void *pDest, CO_CONST void *pSrc,
					UNSIGNED32	size, UNSIGNED16 numeric);

#endif /* OD_ACCESS_H */

