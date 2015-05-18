/*
 * msg_handler.cpp
 *
 *  Created on: 2014��2��20��
 *      Author: Administrator
 */

#include "logger/logger.h"
#include "frame/frame_msghandle.h"
#include "frame/frame_define.h"
#include "frame/frame.h"
#include "server_typedef.h"
#include "msg_handler.h"
#include "include/account_msg.h"

using namespace LOGGER;
using namespace FRAME;

int32_t CMsgHandler::OnOpened(IIOSession *pIoSession)
{
	WRITE_INFO_LOG(SERVER_NAME, "new session!{peeraddress=%s, peerport=%d, sessionid=%u}\n",
			pIoSession->GetPeerAddressStr(), pIoSession->GetPeerPort(), pIoSession->GetSessionID());

	return 0;
}

int32_t CMsgHandler::OnRecved(IIOSession *pIoSession, uint8_t *pBuf, uint32_t nBufSize)
{
	return 0;
}

int32_t CMsgHandler::OnSent(IIOSession *pIoSession, uint8_t *pBuf, uint32_t nBufSize)
{
	return 0;
}

int32_t CMsgHandler::OnClosed(IIOSession *pIoSession)
{
	WRITE_INFO_LOG(SERVER_NAME, "session closed!{peeraddress=%s, peerport=%d, sessionid=%u}\n",
			pIoSession->GetPeerAddressStr(), pIoSession->GetPeerPort(), pIoSession->GetSessionID());

	return 0;
}

int32_t CMsgHandler::OnError(IIOSession *pIoSession)
{
	WRITE_INFO_LOG(SERVER_NAME, "session error!{peeraddress=%s, peerport=%d, sessionid=%u}\n",
			pIoSession->GetPeerAddressStr(), pIoSession->GetPeerPort(), pIoSession->GetSessionID());

	return 0;
}

int32_t CMsgHandler::OnTimeout(IIOSession *pIoSession)
{
	WRITE_INFO_LOG(SERVER_NAME, "session timeout!{peeraddress=%s, peerport=%d, sessionid=%u}\n",
			pIoSession->GetPeerAddressStr(), pIoSession->GetPeerPort(), pIoSession->GetSessionID());

	return 0;
}

