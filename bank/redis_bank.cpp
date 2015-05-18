/*
 * redis_bank.cpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jimm
 */

#include "redis_bank.h"
#include "frame/frame.h"
#include "frame/redis_channel.h"
#include "frame/redissession_bank.h"
#include "config/redis_config.h"
#include "server_typedef.h"
#include "dispatch/subscribe_channel.h"

#include <stdio.h>

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

		if(strlen(arrRedisServerInfo[i].arrChannelKey) > 0)
		{
			pRedisChannel = new CSubscribeChannel(arrRedisServerInfo[i].nServerID, arrRedisServerInfo[i].arrServerAddress,
					arrRedisServerInfo[i].nPort, arrRedisServerInfo[i].arrChannelKey);

			g_Frame.AddRunner((CSubscribeChannel *)pRedisChannel);

			RedisChannelMap *pChannelMap = NULL;
			if(m_stSubscribeKindMap.find(arrRedisServerInfo[i].arrServerName) == m_stSubscribeKindMap.end())
			{
				pChannelMap = new RedisChannelMap();
				m_stSubscribeKindMap[arrRedisServerInfo[i].arrServerName] = pChannelMap;
			}
			else
			{
				pChannelMap = m_stSubscribeKindMap[arrRedisServerInfo[i].arrServerName];
			}
			(*pChannelMap)[arrRedisServerInfo[i].nServerID] = pRedisChannel;
//			m_stSubscribeKindMap[arrRedisServerInfo[i].arrServerName][arrRedisServerInfo[i].nServerID] = pRedisChannel;
		}
		else
		{
			pRedisChannel = new CRedisChannel(arrRedisServerInfo[i].nServerID, arrRedisServerInfo[i].arrServerAddress,
					arrRedisServerInfo[i].nPort);

			RedisChannelMap *pChannelMap = NULL;
			if(m_stRedisKindMap.find(arrRedisServerInfo[i].arrServerName) == m_stRedisKindMap.end())
			{
				pChannelMap = new RedisChannelMap();
				m_stRedisKindMap[arrRedisServerInfo[i].arrServerName] = pChannelMap;
			}
			else
			{
				pChannelMap = m_stRedisKindMap[arrRedisServerInfo[i].arrServerName];
			}
			(*pChannelMap)[arrRedisServerInfo[i].nServerID] = pRedisChannel;

			char arrTuple[64];
			sprintf(arrTuple, "%s:%d", arrRedisServerInfo[i].arrServerAddress, arrRedisServerInfo[i].nPort);
			m_stChannelTupleMap[arrTuple] = pRedisChannel;

			RedisChannelArray *pChannelArray = NULL;
			if(m_stRedisChannelSet.find(arrRedisServerInfo[i].arrServerName) == m_stRedisChannelSet.end())
			{
				pChannelArray = new RedisChannelArray();
				m_stRedisChannelSet[arrRedisServerInfo[i].arrServerName] = pChannelArray;
			}
			else
			{
				pChannelArray = m_stRedisChannelSet[arrRedisServerInfo[i].arrServerName];
			}
			pChannelArray->m_arrRedisChannel[pChannelArray->m_nChannelCount++] = pRedisChannel;
		}

		pRedisChannel->Connect();
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

	RedisKindMap::iterator it = m_stRedisKindMap.begin();
	for(; it != m_stRedisKindMap.end(); ++it)
	{
		RedisChannelMap *pRedisChannelMap = it->second;
		for(RedisChannelMap::iterator jt = pRedisChannelMap->begin(); jt != pRedisChannelMap->end(); ++jt)
		{
			arrRedisChannel[nCount++] = jt->second;
			if(nCount >= nMaxCount - 1)
			{
				return nCount;
			}
		}
	}

	it = m_stSubscribeKindMap.begin();
	for(; it != m_stSubscribeKindMap.end(); ++it)
	{
		RedisChannelMap *pRedisChannelMap = it->second;
		for(RedisChannelMap::iterator jt = pRedisChannelMap->begin(); jt != pRedisChannelMap->end(); ++jt)
		{
			arrRedisChannel[nCount++] = jt->second;
			if(nCount >= nMaxCount - 1)
			{
				return nCount;
			}
		}
	}

	return nCount;
}

//获取一个redis对象，目前采取对目标uin hash到方案
CRedisChannel *CRedisBank::GetRedisChannel(const char *szKindName, int64_t nKey)
{
	CRedisChannel *pRedisChannel = NULL;
	RedisChannelSet::iterator it = m_stRedisChannelSet.find(szKindName);
	if(it != m_stRedisChannelSet.end())
	{
		RedisChannelArray *pChannelArray = it->second;
		int32_t nIndex = nKey % pChannelArray->m_nChannelCount;
		pRedisChannel = pChannelArray->m_arrRedisChannel[nIndex];
	}

	return pRedisChannel;
}

CRedisChannel *CRedisBank::GetRedisChannel(const char *szKindName, const char *szKey)
{
	return GetRedisChannel(szKindName, atoi64(szKey));
}

//根据cache key获取redis对象
CRedisChannel *CRedisBank::GetRedisChannel(uint32_t nGateRedisAddress, uint16_t nGateRedisPort)
{
	CRedisChannel *pRedisChannel = NULL;

	char arrTuple[64];
	sprintf(arrTuple, "%s:%d", inet_ntoa_f(nGateRedisAddress), nGateRedisPort);
	ChannelTupleMap::iterator it = m_stChannelTupleMap.find(arrTuple);
	if(it != m_stChannelTupleMap.end())
	{
		pRedisChannel = it->second;
	}

	return pRedisChannel;
}


