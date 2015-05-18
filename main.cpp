/*
 * main.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: jimm
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include "netevent/net_handler.h"
#include "logger/logger.h"
#include "frame/frame.h"
#include "frame/cachekey_define.h"
#include "regist_message.h"
#include "dispatch/checkconn_handler.h"
#include "config/server_config.h"
#include "config/redis_config.h"
#include "server_typedef.h"

using namespace LOGGER;
using namespace NETEVENT;
using namespace FRAME;

class Initer : public IInitFrame
{
public:
	virtual int32_t InitFrame(CNetHandler *pNetHandler)
	{
		CCheckConnHandler *pCheckConnHandler = new CCheckConnHandler();
		int32_t nTimerIndex = 0;
		g_Frame.CreateTimer(static_cast<TimerProc>(&CCheckConnHandler::CheckConnStatus),
				pCheckConnHandler, NULL, 10 * MS_PER_SECOND, true, nTimerIndex);

		return 0;
	}
};

int32_t main(int32_t argc, char* argv[])
{
	srand((int)time(0));

	//启动日志线程
	CLogger::Start();

	char *szCtlAddress = NULL;
	//读取命令行参数
	if (argc > 1)
	{
		if (0 == strcasecmp((const char*)argv[1], "-s"))
		{
			szCtlAddress = argv[2];
		}
		else
		{
			printf("./service -s ctlcenter's ip -w num & or ./service &");
			exit(0);
		}

		if (0 == strcasecmp((const char*)argv[3], "-w"))
		{
			int32_t nWorkerCount = 0;
			g_Frame.SetWorkerCount(atoi(argv[4]));
		}
		else
		{
			printf("./service -s ctlcenter's ip -w num & or ./service &");
			exit(0);
		}
	}

	g_Frame.Start(SERVER_NAME, 1, szCtlAddress, 5178, new Initer());

	return 0;
}

