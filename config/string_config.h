/*
 * string_config.h
 *
 *  Created on: Mar 14, 2015
 *      Author: jimm
 */

#ifndef STRING_CONFIG_H_
#define STRING_CONFIG_H_

#include "../../frame/frame_impl.h"
#include "../../frame/frame.h"
#include "../server_typedef.h"
#include <string>

using namespace std;
using namespace FRAME;

class CStringConfig : public IConfig
{
	typedef map<int32_t, string>				ResultStringMap;
	typedef map<int32_t, ResultStringMap>		MsgStringMap;
public:
	CStringConfig(const char *pConfigName)
	{
		strcpy(m_szConfigFile, pConfigName);
	}

	virtual ~CStringConfig(){};

	//初始化配置
	virtual int32_t Init();
	//卸载配置
	virtual int32_t Uninit();

	string GetString(int32_t nMsgID, int32_t nResult);

protected:
	char			m_szConfigFile[enmMaxConfigFileNameSize];
	MsgStringMap	m_stMsgStringMap;
};


#endif /* STRING_CONFIG_H_ */
