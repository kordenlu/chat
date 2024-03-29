/*
 * chattoone_handler.cpp
 *
 *  Created on: 2015年3月30日
 *      Author: jimm
 */

#include "chattoone_handler.h"
#include "common/common_datetime.h"
#include "common/common_api.h"
#include "common/common_codeengine.h"
#include "frame/frame.h"
#include "frame/server_helper.h"
#include "frame/redissession_bank.h"
#include "frame/cachekey_define.h"
#include "logger/logger.h"
#include "include/control_head.h"
#include "include/typedef.h"
#include "config/string_config.h"
#include "server_typedef.h"
#include "bank/redis_bank.h"

using namespace LOGGER;
using namespace FRAME;

int32_t CChatToOneHandler::ChatToOne(ICtlHead *pCtlHead, IMsgHead *pMsgHead, IMsgBody *pMsgBody, uint8_t *pBuf, int32_t nBufSize)
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

	if((pControlHead->m_nUin == 0) || (pControlHead->m_nUin != pMsgHeadCS->m_nSrcUin))
	{
		CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
		CRedisChannel *pClientRespChannel = pRedisBank->GetRedisChannel(pControlHead->m_nGateRedisAddress, pControlHead->m_nGateRedisPort);

		return CServerHelper::KickUser(pControlHead, pMsgHeadCS, pClientRespChannel, KickReason_NotLogined);
	}

	CChatToOneReq *pChatToOneReq = dynamic_cast<CChatToOneReq *>(pMsgBody);
	if(pChatToOneReq == NULL)
	{
		return 0;
	}

	CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
	RedisSession *pSession = pRedisSessionBank->CreateSession(this, static_cast<RedisReply>(&CChatToOneHandler::OnSessionExistInBlackList),
			static_cast<TimerProc>(&CChatToOneHandler::OnRedisSessionTimeout));
	UserSession *pSessionData = new(pSession->GetSessionData()) UserSession();
	pSessionData->m_stCtlHead = *pControlHead;
	pSessionData->m_stMsgHeadCS = *pMsgHeadCS;
	pSessionData->m_nChatMsgType = pChatToOneReq->m_nChatMsgType;
	pSessionData->m_nMsgSize = pMsgHeadCS->m_nTotalSize;
	memcpy(pSessionData->m_arrMsg, pBuf + pControlHead->m_nHeadSize, nBufSize - pControlHead->m_nHeadSize);
	CServerHelper::Split((uint8_t *)(pChatToOneReq->m_strChatMsg.c_str()), pChatToOneReq->m_strChatMsg.size(),
			pSessionData->m_arrChatApns, pSessionData->m_nChatApnsSize, sizeof(pSessionData->m_arrChatApns));

	uint32_t nOffset = pSessionData->m_nMsgSize - sizeof(pChatToOneReq->m_nCurTime);
	CCodeEngine::Encode(pSessionData->m_arrMsg, pSessionData->m_nMsgSize, nOffset, (int32_t)CDateTime::CurrentDateTime().Seconds());

	CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
	CRedisChannel *pRedisChannel = pRedisBank->GetRedisChannel(UserBlackList::servername, pMsgHeadCS->m_nDstUin);
	pRedisChannel->ZScore(pSession, CServerHelper::MakeRedisKey(UserBlackList::keyname, pMsgHeadCS->m_nDstUin), pMsgHeadCS->m_nSrcUin);

	return 0;
}

int32_t CChatToOneHandler::OnSessionExistInBlackList(int32_t nResult, void *pReply, void *pSession)
{
	redisReply *pRedisReply = (redisReply *)pReply;
	RedisSession *pRedisSession = (RedisSession *)pSession;
	UserSession *pUserSession = (UserSession *)pRedisSession->GetSessionData();

	CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
	CStringConfig *pStringConfig = (CStringConfig *)g_Frame.GetConfig(CONFIG_STRING);

	CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
	CRedisChannel *pRespChannel = pRedisBank->GetRedisChannel(pUserSession->m_stCtlHead.m_nGateRedisAddress, pUserSession->m_stCtlHead.m_nGateRedisPort);
	if(pRespChannel == NULL)
	{
		WRITE_WARN_LOG(SERVER_NAME, "it's not found redis channel by msgid!{msgid=%d, srcuin=%u, dstuin=%u}\n", MSGID_CHATTOONE_RESP,
				pUserSession->m_stMsgHeadCS.m_nSrcUin, pUserSession->m_stMsgHeadCS.m_nDstUin);
		pRedisSessionBank->DestroySession(pRedisSession);
		return 0;
	}

	uint8_t arrRespBuf[MAX_MSG_SIZE];

	CChatToOneResp stChatToOneResp;
	stChatToOneResp.m_nResult = CChatToOneResp::enmResult_OK;

	bool bSuccessSend = true;
	do
	{
		if(pRedisReply->type == REDIS_REPLY_ERROR)
		{
			bSuccessSend = false;
			break;
		}

		if(pRedisReply->type != REDIS_REPLY_NIL)
		{
			bSuccessSend = false;
			break;
		}
	}while(0);

	MsgHeadCS stMsgHeadCS;
	stMsgHeadCS.m_nMsgID = MSGID_CHATTOONE_RESP;
	stMsgHeadCS.m_nSeq = pUserSession->m_stMsgHeadCS.m_nSeq;
	stMsgHeadCS.m_nSrcUin = pUserSession->m_stMsgHeadCS.m_nSrcUin;
	stMsgHeadCS.m_nDstUin = pUserSession->m_stMsgHeadCS.m_nDstUin;

	uint16_t nTotalSize = CServerHelper::MakeMsg(&pUserSession->m_stCtlHead, &stMsgHeadCS, &stChatToOneResp, arrRespBuf, sizeof(arrRespBuf));
	pRespChannel->RPush(NULL, CServerHelper::MakeRedisKey(ClientResp::keyname, pUserSession->m_stCtlHead.m_nGateID), (char *)arrRespBuf, nTotalSize);

	g_Frame.Dump(&pUserSession->m_stCtlHead, &stMsgHeadCS, &stChatToOneResp, "send ");

	if(bSuccessSend)
	{
		pRedisSession->SetHandleRedisReply(static_cast<RedisReply>(&CChatToOneHandler::OnSessionGetUserUnreadMsgCount));

		CRedisChannel *pUnreadMsgChannel = pRedisBank->GetRedisChannel(UserUnreadMsgList::servername, pUserSession->m_stMsgHeadCS.m_nDstUin);
		pUnreadMsgChannel->Multi();
		pUnreadMsgChannel->ZAdd(NULL, CServerHelper::MakeRedisKey(UserUnreadMsgList::keyname, pUserSession->m_stMsgHeadCS.m_nDstUin), "%ld %b",
				pUserSession->m_stCtlHead.m_nTimeStamp, pUserSession->m_arrMsg, (size_t)pUserSession->m_nMsgSize);
		pUnreadMsgChannel->ZCard(NULL, CServerHelper::MakeRedisKey(UserUnreadMsgList::keyname, pUserSession->m_stMsgHeadCS.m_nDstUin));
		pUnreadMsgChannel->Exec(pRedisSession);

		RedisSession *pGetPhoneTypeSession = pRedisSessionBank->CreateSession(this, static_cast<RedisReply>(&CChatToOneHandler::OnSessionGetPhoneType),
				static_cast<TimerProc>(&CChatToOneHandler::OnRedisSessionTimeout));
		UserSession *pSessionData = new(pGetPhoneTypeSession->GetSessionData()) UserSession();
		*pSessionData = *pUserSession;
		pSessionData->m_nSessionType = UserSession::enmSessionType_PushApns;

		CRedisChannel *pRedisChannel = pRedisBank->GetRedisChannel(UserBaseInfo::servername, pUserSession->m_stMsgHeadCS.m_nDstUin);
		pRedisChannel->HMGet(pGetPhoneTypeSession, CServerHelper::MakeRedisKey(UserBaseInfo::keyname, pUserSession->m_stMsgHeadCS.m_nDstUin),
				"%s", UserBaseInfo::phonetype);
	}
	else
	{
		pRedisSessionBank->DestroySession(pRedisSession);
	}

	return 0;
}

int32_t CChatToOneHandler::OnSessionGetPhoneType(int32_t nResult, void *pReply, void *pSession)
{
	redisReply *pRedisReply = (redisReply *)pReply;
	RedisSession *pRedisSession = (RedisSession *)pSession;
	UserSession *pUserSession = (UserSession *)pRedisSession->GetSessionData();

	CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);

	uint8_t nPhoneType = 255;
	do
	{
		if(pRedisReply->type == REDIS_REPLY_ARRAY)
		{
			redisReply *pReplyElement = pRedisReply->element[0];
			if(pReplyElement->type != REDIS_REPLY_NIL)
			{
				nPhoneType = atoi(pReplyElement->str);
			}
		}
	}while(0);

	if(nPhoneType == enmPhoneType_IPhone)
	{
		if(pUserSession->m_nChatMsgType == CChatToOneReq::enmChatMsgType_Voice)
		{
			pUserSession->m_nChatApnsSize = 12;
			memcpy(pUserSession->m_arrChatApns, "发来语音", pUserSession->m_nChatApnsSize);
		}
		else if(pUserSession->m_nChatMsgType == CChatToOneReq::enmChatMsgType_Pic)
		{
			pUserSession->m_nChatApnsSize = 12;
			memcpy(pUserSession->m_arrChatApns, "发来图片", pUserSession->m_nChatApnsSize);
		}

		CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
		CRedisChannel *pPushApnsChannel = pRedisBank->GetRedisChannel(PushApns::servername, pUserSession->m_stMsgHeadCS.m_nDstUin);
		CServerHelper::PushToAPNS(pPushApnsChannel, pUserSession->m_stMsgHeadCS.m_nSrcUin, pUserSession->m_stMsgHeadCS.m_nDstUin,
				pUserSession->m_stMsgHeadCS.m_nMsgID, pUserSession->m_arrChatApns, pUserSession->m_nChatApnsSize);
	}

	pRedisSessionBank->DestroySession(pRedisSession);
	return 0;
}

int32_t CChatToOneHandler::OnSessionGetUserUnreadMsgCount(int32_t nResult, void *pReply, void *pSession)
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
		pRedisSession->SetHandleRedisReply(static_cast<RedisReply>(&CChatToOneHandler::OnSessionGetUserSessionInfo));

		CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
		CRedisChannel *pUserSessionChannel = pRedisBank->GetRedisChannel(UserSessionInfo::servername, pUserSession->m_stMsgHeadCS.m_nDstUin);
		pUserSessionChannel->HMGet(pRedisSession, CServerHelper::MakeRedisKey(UserSessionInfo::keyname, pUserSession->m_stMsgHeadCS.m_nDstUin),
				"%s %s %s %s %s %s", UserSessionInfo::clientaddress, UserSessionInfo::clientport, UserSessionInfo::sessionid,
				UserSessionInfo::gateid, UserSessionInfo::gateredisaddress, UserSessionInfo::gateredisport);
	}
	else
	{
		CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
		pRedisSessionBank->DestroySession(pRedisSession);
	}

	return 0;
}

int32_t CChatToOneHandler::OnSessionGetUserSessionInfo(int32_t nResult, void *pReply, void *pSession)
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

			pReplyElement = pRedisReply->element[4];
			if(pReplyElement->type != REDIS_REPLY_NIL)
			{
				stCtlHead.m_nGateRedisAddress = atoi(pReplyElement->str);
			}
			else
			{
				bGetSessionSuccess = false;
				break;
			}

			pReplyElement = pRedisReply->element[5];
			if(pReplyElement->type != REDIS_REPLY_NIL)
			{
				stCtlHead.m_nGateRedisPort = atoi(pReplyElement->str);
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
		CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
		CRedisChannel *pPushClientChannel = pRedisBank->GetRedisChannel(stCtlHead.m_nGateRedisAddress, stCtlHead.m_nGateRedisPort);

		CServerHelper::SendSyncNoti(pPushClientChannel, &stCtlHead, pUserSession->m_stMsgHeadCS.m_nDstUin);
	}

	CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
	pRedisSessionBank->DestroySession(pRedisSession);

	return 0;
}

int32_t CChatToOneHandler::OnRedisSessionTimeout(void *pTimerData)
{
	CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
	RedisSession *pRedisSession = (RedisSession *)pTimerData;
	UserSession *pUserSession = (UserSession *)pRedisSession->GetSessionData();

	if(pUserSession->m_nSessionType == UserSession::enmSessionType_Other)
	{
		CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
		CRedisChannel *pRespChannel = pRedisBank->GetRedisChannel(pUserSession->m_stCtlHead.m_nGateRedisAddress, pUserSession->m_stCtlHead.m_nGateRedisPort);
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

		CChatToOneResp stChatToOneResp;
		stChatToOneResp.m_nResult = CChatToOneResp::enmResult_Unknown;
		stChatToOneResp.m_strTips = pStringConfig->GetString(stMsgHeadCS.m_nMsgID, stChatToOneResp.m_nResult);

		uint16_t nTotalSize = CServerHelper::MakeMsg(&pUserSession->m_stCtlHead, &stMsgHeadCS, &stChatToOneResp, arrRespBuf, sizeof(arrRespBuf));
		pRespChannel->RPush(NULL, CServerHelper::MakeRedisKey(ClientResp::keyname, pUserSession->m_stCtlHead.m_nGateID), (char *)arrRespBuf, nTotalSize);

		g_Frame.Dump(&pUserSession->m_stCtlHead, &stMsgHeadCS, &stChatToOneResp, "send ");
	}

	pRedisSessionBank->DestroySession(pRedisSession);
	return 0;
}

