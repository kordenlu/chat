/*
 * checkconn_handler.cpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jimm
 */

#include "checkconn_handler.h"
#include "bank/redis_bank.h"
#include "server_typedef.h"
#include "frame/frame.h"
#include "logger/logger.h"

using namespace FRAME;
using namespace LOGGER;

int32_t CCheckConnHandler::CheckConnStatus(void *pTimerData)
{
	WRITE_INFO_LOG(SERVER_NAME, "check connection status!\n");

	CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);

	CRedisChannel *arrRedisChannel[enmMaxRedisCount];
	int32_t nRedisCount = pRedisBank->GetAllRedisChannel(arrRedisChannel, enmMaxRedisCount);
	for(int32_t i = 0; i < nRedisCount; ++i)
	{
		if(!arrRedisChannel[i]->IsConnected())
		{
			arrRedisChannel[i]->Connect();
		}
	}

	return 0;
}

