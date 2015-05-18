/*
 * redis_bank.h
 *
 *  Created on: Mar 7, 2015
 *      Author: jimm
 */

#ifndef REDIS_BANK_H_
#define REDIS_BANK_H_

#include "frame/frame_impl.h"
#include "frame/redis_channel.h"
#include <string>
#include <map>
using namespace std;
using namespace FRAME;

#define MAX_REDISHASH_COUNT		1024

class CRedisBank : public IBank
{
	struct RedisChannelArray
	{
		RedisChannelArray()
		{
			m_nChannelCount = 0;
			for(int32_t i = 0; i < MAX_REDISHASH_COUNT; ++i)
			{
				m_arrRedisChannel[i] = NULL;
			}
		}
		int32_t			m_nChannelCount;
		CRedisChannel	*m_arrRedisChannel[MAX_REDISHASH_COUNT];
	};

	typedef map<int32_t, CRedisChannel *>	RedisChannelMap;
	typedef map<string, RedisChannelMap *>	RedisKindMap;
	typedef map<string, RedisChannelArray *>	RedisChannelSet;
	typedef map<string, CRedisChannel *>	ChannelTupleMap;
public:
	CRedisBank()
	{
	}

	//初始化
	virtual int32_t Init();
	//卸载
	virtual int32_t Uninit();
	//获取所有redis对象
	int32_t GetAllRedisChannel(CRedisChannel *arrRedisChannel[], int32_t nMaxCount);
	//获取一个redis对象，目前采取对目标uin hash到方案
	CRedisChannel *GetRedisChannel(const char *szKindName, int64_t nKey);
	//
	CRedisChannel *GetRedisChannel(const char *szKindName, const char *szKey);
	//根据cache key获取redis对象
	CRedisChannel *GetRedisChannel(uint32_t nGateRedisAddress, uint16_t nGateRedisPort);
protected:
	RedisKindMap		m_stRedisKindMap;
	RedisKindMap		m_stSubscribeKindMap;
	ChannelTupleMap		m_stChannelTupleMap;
	RedisChannelSet		m_stRedisChannelSet;
};


#endif /* REDIS_BANK_H_ */
