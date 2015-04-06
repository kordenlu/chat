/*
 * main.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: jimm
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include "../netevent/net_namespace.h"
#include "../netevent/net_impl.h"
#include "../netevent/net_typedef.h"
#include "../netevent/net_reactor.h"
#include "../netevent/net_handler.h"
#include "../netevent/net_acceptor.h"
#include "../netevent/net_connector.h"
#include "../logger/logger.h"
#include "../frame/frame.h"
#include "../include/cachekey_define.h"
#include "regist_message.h"
#include "dispatch/msgparser_factory.h"
#include "dispatch/msg_handler.h"
#include "dispatch/checkconn_handler.h"
#include "config/server_config.h"
#include "config/redis_config.h"
#include "server_typedef.h"

using namespace LOGGER;
using namespace NETEVENT;
using namespace FRAME;

//注册到配置管理器
REGIST_CONFIG(USER_BASEINFO, UserBaseInfo)
REGIST_CONFIG(USER_BLACKLIST, UserBlackList)
REGIST_CONFIG(USER_UNREADMSGLIST, UserUnreadMsgList)

int32_t InitNetAndTimer(CNetHandler *pNetHandler)
{
	CServerConfig *pServerConfig = (CServerConfig *)g_Frame.GetConfig(CONFIG_SERVER);
	if(pServerConfig == NULL)
	{
		return 1;
	}

	//创建包解析器工厂
	IPacketParserFactory *pMsgParserFactory = new CMsgParserFactory();
	CMsgHandler *pMsgHandler = new CMsgHandler();

	CAcceptor *pMsgAcceptor = new CAcceptor(pNetHandler, pMsgParserFactory, pMsgHandler);
	pMsgAcceptor->Bind(pServerConfig->GetOutsideListenPort());

	CCheckConnHandler *pCheckConnHandler = new CCheckConnHandler();
	int32_t nTimerIndex = 0;
	g_Frame.CreateTimer(static_cast<TimerProc>(&CCheckConnHandler::CheckConnStatus),
			pCheckConnHandler, NULL, 10 * MS_PER_SECOND, true, nTimerIndex);

	return 0;
}

int32_t main()
{
	//启动日志线程
	CLogger::Start();

	//创建网络事件处理器
	CNetHandler *pNetHandler = new CNetHandler();
	pNetHandler->CreateReactor();

	g_Frame.AddRunner(pNetHandler);

	if(g_Frame.Init(SERVER_NAME) != 0)
	{
		return 0;
	}

	InitNetAndTimer(pNetHandler);

	while(true)
	{
		g_Frame.Run();
	}

	g_Frame.Uninit();

	return 0;
}

