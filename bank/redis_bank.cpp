/*
 * redis_bank.cpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jimm
 */

#include "redis_bank.h"
#include "../../frame/frame.h"
#include "../../frame/redis_channel.h"
#include "../../frame/redissession_bank.h"
#include "../config/redis_config.h"
#include "../server_typedef.h"
#include "../dispatch/subscribe_channel.h"

using namespace FRAME;

REGIST_BANK(BANK_REDIS, CRedisBank)

//初始化
int32_t CRedisBank::Init()
{
	CRedisConfig *pRedisConfig = (CRedisConfig *)g_Frame.GetConfig(CONFIG_REDIS);

	int32_t nRedisCount = 0;
	RedisServerInfo arrRedisServerInfo[MAX_REDISSERVER_COUNT];
	nRedisCount = pRedisConfig->GetRedisServerInfo(arrRedisServerInfo, MAX_REDISSERVER_COUNT);

	for(int32_t i = 0; i < nRedisCount; ++i)
	{
		CRedisChannel *pRedisChannel = NULL;

		if(arrRedisServerInfo[i].arrChannelMode == string("subscribe"))
		{
//			CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
//			RedisSession *pSession = pRedisSessionBank->CreateSession(new CSubscribeChannel(), static_cast<RedisReply>(&CSubscribeChannel::OnRedisReply),
//					NULL);
//			pRedisChannel->AttachSession(pSession);
			pRedisChannel = new CSubscribeChannel(arrRedisServerInfo[i].nServerID, arrRedisServerInfo[i].arrServerAddress,
					arrRedisServerInfo[i].nPort, arrRedisServerInfo[i].arrChannelKey);
			g_Frame.AddRunner((CSubscribeChannel *)pRedisChannel);
		}
		else
		{
			pRedisChannel = new CRedisChannel(arrRedisServerInfo[i].nServerID, arrRedisServerInfo[i].arrServerAddress,
					arrRedisServerInfo[i].nPort, arrRedisServerInfo[i].arrChannelKey);
		}
		pRedisChannel->Connect();

		m_stRedisServerIDMap[arrRedisServerInfo[i].nServerID][arrRedisServerInfo[i].arrChannelKey] = pRedisChannel;
		m_stRedisServerMap[arrRedisServerInfo[i].arrChannelKey] = pRedisChannel;

		m_arrRedisHashTable[m_nHashTableSize++] = pRedisChannel;
	}

	return 0;
}

//卸载
int32_t CRedisBank::Uninit()
{
	return 0;
}

//获取所有redis对象
int32_t CRedisBank::GetAllRedisChannel(CRedisChannel *arrRedisChannel[], int32_t nMaxCount)
{
	int32_t nCount = 0;

	RedisServerMap::iterator it = m_stRedisServerMap.begin();
	for(; it != m_stRedisServerMap.end(); ++it)
	{
		arrRedisChannel[nCount++] = it->second;
		if(nCount >= nMaxCount - 1)
		{
			break;
		}
	}

	return nCount;
}

//获取一个redis对象，目前采取对目标uin hash到方案
CRedisChannel *CRedisBank::GetRedisChannel(uint32_t key)
{
	if(m_nHashTableSize <= 0)
	{
		return NULL;
	}

	return m_arrRedisHashTable[key % m_nHashTableSize];
}

//根据cache key获取redis对象
CRedisChannel *CRedisBank::GetRedisChannel(string strKey)
{
	CRedisChannel *pRedisChannel = NULL;
	map<string, CRedisChannel *>::iterator it = m_stRedisServerMap.find(strKey);
	if(it != m_stRedisServerMap.end())
	{
		pRedisChannel = it->second;
	}

	return pRedisChannel;
}

//根据serverid and cache key获取redis对象
CRedisChannel *CRedisBank::GetRedisChannel(int32_t nServerID, string strKey)
{
	RedisServerIDMap::iterator it = m_stRedisServerIDMap.find(nServerID);
	if(it != m_stRedisServerIDMap.end())
	{
		RedisServerMap::iterator et = it->second.find(strKey);
		if(et != it->second.end())
		{
			return et->second;
		}
	}

	return NULL;
}


