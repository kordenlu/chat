/*
 * chatread_handler.cpp
 *
 *  Created on: 2015年4月11日
 *      Author: jimm
 */

#include "chatread_handler.h"
#include "../../common/common_datetime.h"
#include "../../common/common_api.h"
#include "../../frame/frame.h"
#include "../../frame/server_helper.h"
#include "../../frame/redissession_bank.h"
#include "../../logger/logger.h"
#include "../../include/cachekey_define.h"
#include "../../include/control_head.h"
#include "../../include/typedef.h"
#include "../../include/sync_msg.h"
#include "../config/msgdispatch_config.h"
#include "../config/string_config.h"
#include "../server_typedef.h"
#include "../bank/redis_bank.h"

using namespace LOGGER;
using namespace FRAME;

int32_t CChatReadHandler::ChatRead(ICtlHead *pCtlHead, IMsgHead *pMsgHead, IMsgBody *pMsgBody, uint8_t *pBuf, int32_t nBufSize)
{
	ControlHead *pControlHead = dynamic_cast<ControlHead *>(pCtlHead);
	if(pControlHead == NULL)
	{
		return 0;
	}

	MsgHeadCS *pMsgHeadCS = dynamic_cast<MsgHeadCS *>(pMsgHead);
	if(pMsgHeadCS == NULL)
	{
		return 0;
	}

	CChatReadReq *pChatReadReq = dynamic_cast<CChatReadReq *>(pMsgBody);
	if(pChatReadReq == NULL)
	{
		return 0;
	}

	UserBlackList *pUserBlackList = (UserBlackList *)g_Frame.GetConfig(USER_BLACKLIST);

	CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
	RedisSession *pSession = pRedisSessionBank->CreateSession(this, static_cast<RedisReply>(&CChatReadHandler::OnSessionGetUserUnreadMsgCount),
			static_cast<TimerProc>(&CChatReadHandler::OnRedisSessionTimeout));
	UserSession *pSessionData = new(pSession->GetSessionData()) UserSession();
	pSessionData->m_stCtlHead = *pControlHead;
	pSessionData->m_stMsgHeadCS = *pMsgHeadCS;

	uint8_t *pMsgBuf = pBuf + pControlHead->GetSize();
	int32_t nMsgSize = nBufSize - pControlHead->GetSize();

	CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
	CRedisChannel *pUnreadMsgChannel = pRedisBank->GetRedisChannel(USER_UNREADMSGLIST);
	pUnreadMsgChannel->Multi();
	pUnreadMsgChannel->ZAdd(NULL, itoa(pMsgHeadCS->m_nDstUin), "%ld %b", pControlHead->m_nTimeStamp, pMsgBuf, nMsgSize);
	pUnreadMsgChannel->ZCount(NULL, itoa(pMsgHeadCS->m_nDstUin));
	pUnreadMsgChannel->Exec(pSession);

	uint8_t arrRespBuf[MAX_MSG_SIZE];

	CMsgDispatchConfig *pMsgDispatchConfig = (CMsgDispatchConfig *)g_Frame.GetConfig(CONFIG_MSGDISPATCH);
	CRedisChannel *pRespChannel = pRedisBank->GetRedisChannel(pMsgDispatchConfig->GetChannelKey(MSGID_CHATREAD_RESP));

	CChatReadResp stChatReadResp;
	stChatReadResp.m_nResult = CChatReadResp::enmResult_OK;

	MsgHeadCS stMsgHeadCS;
	stMsgHeadCS.m_nMsgID = MSGID_CHATREAD_RESP;
	stMsgHeadCS.m_nSeq = pMsgHeadCS->m_nSeq;
	stMsgHeadCS.m_nDstUin = pMsgHeadCS->m_nSrcUin;

	uint16_t nTotalSize = CServerHelper::MakeMsg(pControlHead, &stMsgHeadCS, &stChatReadResp, arrRespBuf, sizeof(arrRespBuf));
	pRespChannel->RPush(NULL, (char *)arrRespBuf, nTotalSize);

	g_Frame.Dump(pControlHead, &stMsgHeadCS, &stChatReadResp, "send ");

	return 0;
}

int32_t CChatReadHandler::OnSessionGetUserUnreadMsgCount(int32_t nResult, void *pReply, void *pSession)
{
	redisReply *pRedisReply = (redisReply *)pReply;
	RedisSession *pRedisSession = (RedisSession *)pSession;
	UserSession *pUserSession = (UserSession *)pRedisSession->GetSessionData();

	bool bIsSyncNoti = false;
	do
	{
		if(pRedisReply->type == REDIS_REPLY_ERROR)
		{
			break;
		}

		if(pRedisReply->type == REDIS_REPLY_ARRAY)
		{
			redisReply *pReplyElement = pRedisReply->element[1];
			if(pReplyElement->type == REDIS_REPLY_INTEGER)
			{
				if(pReplyElement->integer <= 1)
				{
					bIsSyncNoti = true;
					break;
				}
			}
		}
	}while(0);

	if(bIsSyncNoti)
	{
		pRedisSession->SetHandleRedisReply(static_cast<RedisReply>(&CChatReadHandler::OnSessionGetUserSessionInfo));

		UserSessionInfo *pUserSessionInfo = (UserSessionInfo *)g_Frame.GetConfig(USER_SESSIONINFO);
		CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
		CRedisChannel *pUserSessionChannel = pRedisBank->GetRedisChannel(USER_SESSIONINFO);
		pUserSessionChannel->HMGet(pRedisSession, itoa(pUserSession->m_stMsgHeadCS.m_nDstUin), "%s %s %s %s", pUserSessionInfo->clientaddress,
				pUserSessionInfo->clientport, pUserSessionInfo->sessionid, pUserSessionInfo->gateid);
	}
	else
	{
		CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
		pRedisSessionBank->DestroySession(pRedisSession);
	}

	return 0;
}

int32_t CChatReadHandler::OnSessionGetUserSessionInfo(int32_t nResult, void *pReply, void *pSession)
{
	redisReply *pRedisReply = (redisReply *)pReply;
	RedisSession *pRedisSession = (RedisSession *)pSession;
	UserSession *pUserSession = (UserSession *)pRedisSession->GetSessionData();

	ControlHead stCtlHead;
	stCtlHead.m_nUin = pUserSession->m_stMsgHeadCS.m_nDstUin;

	bool bGetSessionSuccess = true;
	do
	{
		if(pRedisReply->type == REDIS_REPLY_ERROR)
		{
			bGetSessionSuccess = false;
			break;
		}

		if(pRedisReply->type == REDIS_REPLY_ARRAY)
		{
			redisReply *pReplyElement = pRedisReply->element[0];
			if(pReplyElement->type != REDIS_REPLY_NIL)
			{
				stCtlHead.m_nClientAddress = atoi(pReplyElement->str);
			}
			else
			{
				bGetSessionSuccess = false;
				break;
			}

			pReplyElement = pRedisReply->element[1];
			if(pReplyElement->type != REDIS_REPLY_NIL)
			{
				stCtlHead.m_nClientPort = atoi(pReplyElement->str);
			}
			else
			{
				bGetSessionSuccess = false;
				break;
			}

			pReplyElement = pRedisReply->element[2];
			if(pReplyElement->type != REDIS_REPLY_NIL)
			{
				stCtlHead.m_nSessionID = atoi(pReplyElement->str);
			}
			else
			{
				bGetSessionSuccess = false;
				break;
			}

			pReplyElement = pRedisReply->element[3];
			if(pReplyElement->type != REDIS_REPLY_NIL)
			{
				stCtlHead.m_nGateID = atoi(pReplyElement->str);
			}
			else
			{
				bGetSessionSuccess = false;
				break;
			}
		}
		else
		{
			bGetSessionSuccess = false;
			break;
		}
	}while(0);

	if(bGetSessionSuccess)
	{
		MsgHeadCS stMsgHeadCS;
		stMsgHeadCS.m_nMsgID = MSGID_STATUSSYNC_NOTI;
		stMsgHeadCS.m_nDstUin = pUserSession->m_stMsgHeadCS.m_nDstUin;

		CStatusSyncNoti stStatusSyncNoti;

		uint8_t arrRespBuf[MAX_MSG_SIZE];

		CMsgDispatchConfig *pMsgDispatchConfig = (CMsgDispatchConfig *)g_Frame.GetConfig(CONFIG_MSGDISPATCH);
		CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
		CRedisChannel *pPushClientChannel = pRedisBank->GetRedisChannel(stCtlHead.m_nGateID, pMsgDispatchConfig->GetChannelKey(MSGID_STATUSSYNC_NOTI));
		if(pPushClientChannel != NULL)
		{
			uint16_t nTotalSize = CServerHelper::MakeMsg(&stCtlHead, &stMsgHeadCS, &stStatusSyncNoti, arrRespBuf, sizeof(arrRespBuf));
			pPushClientChannel->RPush(NULL, (char *)arrRespBuf, nTotalSize);
		}
	}

	CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
	pRedisSessionBank->DestroySession(pRedisSession);

	return 0;
}

int32_t CChatReadHandler::OnRedisSessionTimeout(void *pTimerData)
{
	CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
	RedisSession *pRedisSession = (RedisSession *)pTimerData;
	UserSession *pUserSession = (UserSession *)pRedisSession->GetSessionData();

	CMsgDispatchConfig *pMsgDispatchConfig = (CMsgDispatchConfig *)g_Frame.GetConfig(CONFIG_MSGDISPATCH);
	CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
	CRedisChannel *pRespChannel = pRedisBank->GetRedisChannel(pMsgDispatchConfig->GetChannelKey(MSGID_CHATTOONE_RESP));
	if(pRespChannel == NULL)
	{
		WRITE_WARN_LOG(SERVER_NAME, "it's not found redis channel by msgid!{msgid=%d, srcuin=%u, dstuin=%u}\n", MSGID_CHATTOONE_RESP,
				pUserSession->m_stMsgHeadCS.m_nSrcUin, pUserSession->m_stMsgHeadCS.m_nDstUin);
		pRedisSessionBank->DestroySession(pRedisSession);
		return 0;
	}

	CStringConfig *pStringConfig = (CStringConfig *)g_Frame.GetConfig(CONFIG_STRING);

	uint8_t arrRespBuf[MAX_MSG_SIZE];

	MsgHeadCS stMsgHeadCS;
	stMsgHeadCS.m_nMsgID = MSGID_CHATTOONE_RESP;
	stMsgHeadCS.m_nSeq = pUserSession->m_stMsgHeadCS.m_nSeq;
	stMsgHeadCS.m_nSrcUin = pUserSession->m_stMsgHeadCS.m_nSrcUin;
	stMsgHeadCS.m_nDstUin = pUserSession->m_stMsgHeadCS.m_nDstUin;

	CChatReadResp stChatReadResp;
	stChatReadResp.m_nResult = CChatReadResp::enmResult_Unknown;

	uint16_t nTotalSize = CServerHelper::MakeMsg(&pUserSession->m_stCtlHead, &stMsgHeadCS, &stChatReadResp, arrRespBuf, sizeof(arrRespBuf));
	pRespChannel->RPush(NULL, (char *)arrRespBuf, nTotalSize);

	g_Frame.Dump(&pUserSession->m_stCtlHead, &stMsgHeadCS, &stChatReadResp, "send ");

	pRedisSessionBank->DestroySession(pRedisSession);
	return 0;
}




