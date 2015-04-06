/*
 * msgdispatch_config.h
 *
 *  Created on: Mar 13, 2015
 *      Author: jimm
 */

#ifndef MSGDISPATCH_CONFIG_H_
#define MSGDISPATCH_CONFIG_H_

#include "../../frame/frame_impl.h"
#include "../../frame/frame.h"
#include "../server_typedef.h"
#include <string.h>
#include <map>

using namespace std;
using namespace FRAME;

class CMsgDispatchConfig : public IConfig
{
public:
	CMsgDispatchConfig(const char *pConfigName)
	{
		strcpy(m_szConfigFile, pConfigName);
	}

	virtual ~CMsgDispatchConfig(){};

	//初始化配置
	virtual int32_t Init();
	//卸载配置
	virtual int32_t Uninit();
	//获取channelkey
	string GetChannelKey(int32_t nMsgID);

protected:
	char					m_szConfigFile[enmMaxConfigFileNameSize];
	map<int32_t, string>	m_stMsgChannelMap;
};

#endif /* MSGDISPATCH_CONFIG_H_ */
