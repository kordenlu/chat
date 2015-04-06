/*
 * server_config.h
 *
 *  Created on: 2014年5月12日
 *      Author: jimm
 */

#ifndef SERVER_CONFIG_H_
#define SERVER_CONFIG_H_

#include "../../frame/frame_impl.h"
#include "../../frame/frame.h"
#include "../../common/common_api.h"
#include "../server_typedef.h"

#include <string.h>

using namespace FRAME;

class CServerConfig : public IConfig
{
public:
	CServerConfig(const char *pConfigName)
	{
		strcpy(m_szConfigFile, pConfigName);
		m_nInsidePort = 0;
		m_nOutsidePort = 0;
		m_nServerType = 0;
		m_nServerID = 0;
		m_nMaxConnCount = 0;
	}

	virtual ~CServerConfig(){};

	//初始化配置
	virtual int32_t Init();
	//卸载配置
	virtual int32_t Uninit();

	int32_t GetOutsideListenPort()
	{
		return m_nOutsidePort;
	}

	int32_t GetInsideListenPort()
	{
		return m_nInsidePort;
	}

	int32_t GetServerType()
	{
		return m_nServerType;
	}

	int32_t GetServerID()
	{
		return m_nServerID;
	}

	int32_t GetMaxConnCount()
	{
		return m_nMaxConnCount;
	}
protected:
	char		m_szConfigFile[enmMaxConfigFileNameSize];
	int32_t		m_nOutsidePort;
	int32_t		m_nInsidePort;
	int32_t		m_nServerType;
	int32_t		m_nServerID;
	int32_t		m_nMaxConnCount;
};

#endif /* SERVER_CONFIG_H_ */
