/*
 * subscribe_channel.h
 *
 *  Created on: Mar 10, 2015
 *      Author: jimm
 */

#ifndef SUBSCRIBE_CHANNEL_H_
#define SUBSCRIBE_CHANNEL_H_

#include "common/common_typedef.h"
#include "common/common_runnable.h"
#include "frame/redis_channel.h"

class CSubscribeChannel : public CRedisChannel, public IRunnable
{
public:
	CSubscribeChannel(int32_t nServerID, char *pAddress, uint16_t nPort, char *pChannelKey):
		CRedisChannel(nServerID, pAddress, nPort)
	{
		m_strChannelKey = string(pChannelKey);
		m_pSubscribeSession = NULL;
		m_nIdleCount = 0;
		m_nRestCount = 0;
	}

	virtual int32_t Init()
	{
		return 0;
	}
	virtual int32_t Uninit()
	{
		return 0;
	}
	virtual int32_t GetSize()
	{
		return sizeof(*this);
	}

	virtual int32_t OnConnected();

	virtual void OnClosed();

	virtual int32_t Run();

	virtual int32_t OnRedisReply(int32_t nResult, void *pReply, void *pSession);

protected:
	string				m_strChannelKey;
	RedisSession 		*m_pSubscribeSession;
	int32_t				m_nIdleCount;
	int32_t				m_nRestCount;
};


#endif /* SUBSCRIBE_CHANNEL_H_ */
