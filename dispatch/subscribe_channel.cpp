/*
 * subscribe_channel.cpp
 *
 *  Created on: Mar 10, 2015
 *      Author: jimm
 */

#include "subscribe_channel.h"
#include "hiredis/hiredis.h"
#include "server_typedef.h"
#include "logger/logger.h"
#include "frame/frame.h"
#include "frame/redissession_bank.h"
#include "common/common_codeengine.h"
#include <string.h>

using namespace FRAME;
using namespace LOGGER;

int32_t CSubscribeChannel::OnConnected()
{
	CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
	m_pSubscribeSession = pRedisSessionBank->CreateSession(this, static_cast<RedisReply>(&CSubscribeChannel::OnRedisReply),
			NULL);

	return 0;
}

void CSubscribeChannel::OnClosed()
{
	if(m_pSubscribeSession != NULL)
	{
		CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
		pRedisSessionBank->DestroySession(m_pSubscribeSession);
	}
}

int32_t CSubscribeChannel::Run()
{
	int32_t nHasTask = 0;
	if(m_nRestCount > 0)
	{
		--m_nRestCount;
	}
	else
	{
		LPop(m_pSubscribeSession, m_strChannelKey.c_str());
		nHasTask = 1;
	}

	return nHasTask;
}

int32_t CSubscribeChannel::OnRedisReply(int32_t nResult, void *pReply, void *pSession)
{
	redisReply *pRedisReply = (redisReply *)pReply;
	if(nResult != 0)
	{
		WRITE_WARN_LOG(SERVER_NAME, "recv redis reply error!{%s}\n", pRedisReply->str);
		return 0;
	}

	if(pRedisReply->type == REDIS_REPLY_STRING)
	{
		uint8_t *pBuf = (uint8_t *)pRedisReply->str;
		int32_t nBufSize = pRedisReply->len;
		if(nBufSize <= 0)
		{
			WRITE_WARN_LOG(SERVER_NAME, "bufsize is wrong!\n");
			return 0;
		}

		uint8_t nHeadSize = 0;
		uint32_t nOffset = sizeof(uint16_t);
		CCodeEngine::Decode(pBuf, nBufSize, nOffset, nHeadSize);

		uint16_t nMsgID = 0;
		nOffset = (nHeadSize + sizeof(uint16_t));

		CCodeEngine::Decode(pBuf, nBufSize, nOffset, nMsgID);

		int32_t nRet = g_Frame.FrameCallBack(nMsgID, pBuf, nBufSize);
		if(nRet == -1)
		{
			WRITE_WARN_LOG(SERVER_NAME, "frame call back return fail!\n");
		}
		else if(nRet == 1)
		{
			WRITE_WARN_LOG(SERVER_NAME, "it's not found msg handler!{msgid=0x%04x}\n", nMsgID);
		}
		else if(nRet == 2)
		{
			WRITE_WARN_LOG(SERVER_NAME, "decode msg failed!{msgid=0x%04x}\n", nMsgID);
		}

		m_nIdleCount = 0;
	}
	else
	{
		m_nIdleCount++;
		if(m_nIdleCount >= 1)
		{
			m_nRestCount = m_nIdleCount;
			m_nIdleCount = 0;
		}
	}

	return 0;
}
