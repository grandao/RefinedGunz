#pragma once

#include "MQuestConst.h"
#include "MTime.h"
#include "MQuestItem.h"

class DBQuestCachingData
{
public:
	DBQuestCachingData() : m_dwLastUpdateTime(GetGlobalTimeMS()), m_nPlayCount(0), m_bEnableUpdate(false), m_nShopTradeCount(0),
		m_pObject(0), m_nRewardCount(0)
	{
	}

	~DBQuestCachingData()
	{
	}

	bool IsRequestUpdate()
	{
		if ((MAX_PLAY_COUNT < m_nPlayCount) || (MAX_ELAPSE_TIME < GetUpdaetElapse()) ||
			(MAX_SHOP_TRADE_COUNT < m_nShopTradeCount) || (MAX_REWARD_COUNT < m_nRewardCount) ||
			m_bEnableUpdate)
			return m_bEnableUpdate = true;

		return m_bEnableUpdate = false;
	}

	bool IsRequestUpdateWhenLogout()
	{
		return ((0 < (m_nShopTradeCount + m_nRewardCount)) || m_bEnableUpdate);
	}

	void IncreasePlayCount(const int nCount = 1);
	void IncreaseShopTradeCount(const int nCount = 1);
	void IncreaseRewardCount(const int nCount = 1);
	bool CheckUniqueItem(MQuestItem* pQuestItem);
	void Reset();

	u64 GetUpdaetElapse()
	{
#ifdef _DEBUG
		char szTemp[100] = { 0 };
		auto t = GetGlobalTimeMS();
		auto a = t - m_dwLastUpdateTime;
		sprintf_safe(szTemp, "Update Elapse %d %d\n", GetGlobalTimeMS() - m_dwLastUpdateTime, a);
		mlog(szTemp);
#endif
		return GetGlobalTimeMS() - m_dwLastUpdateTime;
	}

	void SetEnableUpdateState(const bool bState) { m_bEnableUpdate = bState; }
	void SetCharObject(MMatchObject* pObject) { m_pObject = pObject; }

	bool DoUpdateDBCharQuestItemInfo();

private:
	MMatchObject*	m_pObject;				// DB업데이트때 데이터를 가져오기 위해서 저장해 놓은 포인터.
	u64				m_dwLastUpdateTime;		// 업데이트가 적용되면 같이 갱신됨. 
	int				m_nPlayCount;			// 게임횟수는 게임에 들어가서 하는 모든 행동에 관계없이 완료를 해야 1번 적용됨. 
	int				m_nShopTradeCount;		// 상점에서의 퀘스트 아이템 거래 횟수.
	bool			m_bEnableUpdate;		// 현재 상태. 업데이트가 가능하면 true임.
	int				m_nRewardCount;			// 현제까지 보상 받은 횟수.
};