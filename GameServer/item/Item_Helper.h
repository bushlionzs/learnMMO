/*$T MapServer/Server/Item/Item_Helper.h GC 1.140 10/10/07 10:07:22 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


#ifndef _ITEM_HELPER_H_
#define _ITEM_HELPER_H_
#include "TypeDefine.h"

struct STSerialHelper
{
	uint32	m_SN;

	STSerialHelper();;
	STSerialHelper(uint32 uiSN);;
	STSerialHelper (char *pStr);

	/*
	===============================================================================================================
	===============================================================================================================
	*/
	void SetClass(eITEM_CLASS rh);

	/*
	===============================================================================================================
	===============================================================================================================
	*/
	uint32 GetItemType();

	/*
	===============================================================================================================
	===============================================================================================================
	*/
	void SetItemType(uint32 uiType);
	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	BOOL GetItemTypeStruct(SItemType &it);

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	SItemType GetItemTypeStruct();

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	eITEM_CLASS GetItemClass();


	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	uint32 GetItemIndex();

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	void SetItemIndex(uint32 uiIndex);
};

#endif
