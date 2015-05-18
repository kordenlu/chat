/*
 * redis_config.h
 *
 *  Created on: Mar 6, 2015
 *      Author: jimm
 */

#ifndef REDIS_CONFIG_H_
#define REDIS_CONFIG_H_

#include "frame/frame_impl.h"
#include "frame/frame.h"
#include "server_typedef.h"
#include <string.h>

using namespace FRAME;

#define MAX_REDISSERVER_COUNT		64

struct RedisServerInfo
{
	RedisServerInfo()
	{
		memset(arrServerName, 0, sizeof(arrServerName));
		nServerID = 0;
		memset(arrServerAddress, 0, sizeof(arrServerAddress));
		nPort = 0;
		memset(arrChannelKey, 0, sizeof(arrChannelKey));
		memset(arrChannelMode, 0, sizeof(arrChannelMode));
	}
	char			arrServerName[256];
	int32_t 		nServerID;
	char			arrServerAddress[128];
	uint16_t		nPort;
	char			arrChannelKey[256];
	char			arrChannelMode[256];
};

class CRedisConfig : public IConfig
{
public:
	CRedisConfig(const char *pConfigName)
	{
		strcpy(m_szConfigFile, pConfigName);
		m_nRedisCount = 0;
	}

	virtual ~CRedisConfig(){};

	//初始化配置
	virtual int32_t Init();
	//卸载配置
	virtual int32_t Uninit();

	virtual int32_t Parser(char *pXMLString);

	int32_t GetRedisServerInfo(RedisServerInfo arrRedisServerInfo[], int32_t nMaxCount);

protected:
	char			m_szConfigFile[enmMaxConfigFileNameSize];
	int32_t			m_nRedisCount;
	RedisServerInfo	m_arrRedisServerInfo[MAX_REDISSERVER_COUNT];
};

#endif /* REDIS_CONFIG_H_ */
