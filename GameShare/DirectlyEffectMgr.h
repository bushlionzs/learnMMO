/*$T Common/DirectlyEffectMgr.h GC 1.140 10/10/09 10:06:29 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


/* DirectlyEffectMgr.h ����˵���� */
#pragma once
#include "StructDB.h"

class TAB::	TABFile;
class		CDirectlyEffectMgr
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
protected:
	static CDirectlyEffectMgr	*s_pDirectlyImpactMgr;

/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	static CDirectlyEffectMgr *GetMe(void)
	{
		return s_pDirectlyImpactMgr;
	}

/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	CDirectlyEffectMgr(void);
	~			CDirectlyEffectMgr(void);

	BOOL			Init(const char *lpszPathName);
	BOOL			Init(const TAB::TABFile *pDataBase);
	void			Term(void);

	SDirectImpact		*GetDirectlyImpact(uint32 dwID);
	const SDirectImpact	*GetConstDirectlyImpact(uint32 dwID) const;

/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
protected:
	TAB::TABFile * m_pDBCFileLoad;

	const TAB::TABFile * m_pDataBase;
};

/*
 =======================================================================================================================
 =======================================================================================================================
 */

inline CDirectlyEffectMgr *GetDirectlyEffectMgr(void)
{
	return CDirectlyEffectMgr::GetMe();
}