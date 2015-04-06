/*
 * redis_bank.h
 *
 *  Created on: Mar 7, 2015
 *      Author: jimm
 */

#ifndef REDIS_BANK_H_
#define REDIS_BANK_H_

#include "../../frame/frame_impl.h"
#include "../../frame/redis_channel.h"
#include <string>
#include <map>
using namespace std;
using namespace FRAME;

#define MAX_REDISHASH_COUNT		1024

class CRedisBank : public IBank
{
	typedef map<string, CRedisChannel *>	RedisServerMap;
	typedef map<int32_t, RedisServerMap>	RedisServerIDMap;
public:
	CRedisBank()
	{
		m_nHashTableSize = 0;
	}

	//初始化
	virtual int32_t Init();
	//卸载
	virtual int32_t Uninit();
	//获取所有redis对象
	int32_t GetAllRedisChannel(CRedisChannel *arrRedisChannel[], int32_t nMaxCount);
	//获取一个redis对象，目前采取对目标uin hash到方案
	CRedisChannel *GetRedisChannel(uint32_t key);
	//根据cache key获取redis对象
	CRedisChannel *GetRedisChannel(string strKey);
	//根据serverid and cache key获取redis对象
	CRedisChannel *GetRedisChannel(int32_t nServerID, string strKey);
protected:
	RedisServerMap		m_stRedisServerMap;
	RedisServerIDMap	m_stRedisServerIDMap;
	int32_t				m_nHashTableSize;
	CRedisChannel		*m_arrRedisHashTable[MAX_REDISHASH_COUNT];
};


#endif /* REDIS_BANK_H_ */
