/*
 * msgdispatch_config.cpp
 *
 *  Created on: Mar 13, 2015
 *      Author: jimm
 */

#include "msgdispatch_config.h"
#include "../../logger/logger.h"
#include "../../tinyxml/tinyxml.h"
#include "../server_typedef.h"

using namespace LOGGER;

//注册到配置管理器
REGIST_CONFIG(CONFIG_MSGDISPATCH, CMsgDispatchConfig)

//初始化配置
int32_t CMsgDispatchConfig::Init()
{
	TiXmlDocument doc(m_szConfigFile);
	if (!doc.LoadFile(TIXML_ENCODING_UTF8))
	{
		WRITE_WARN_LOG(SERVER_NAME, "%s is not format utf8!\n", m_szConfigFile);
		return 1;
	}

	//获取根节点
	TiXmlElement *pRoot = doc.RootElement();
	if (NULL == pRoot)
	{
		WRITE_WARN_LOG(SERVER_NAME, "%s is not found redis node!\n", m_szConfigFile);
		return 1;
	}

	const char* pszValue = NULL;

	TiXmlElement *pNode = pRoot->FirstChildElement("param");
	if(NULL == pNode)
	{
		WRITE_WARN_LOG(SERVER_NAME, "%s is not found param!\n", m_szConfigFile);
		return 1;
	}

	while(pNode != NULL)
	{
		int32_t nMsgID = 0;
		pszValue = pNode->Attribute("msgid", &nMsgID);
		if(NULL == pszValue)
		{
			WRITE_WARN_LOG(SERVER_NAME, "%s is not found msgid node!\n", m_szConfigFile);
			return 1;
		}

		char arrChannekKey[256];
		memset(arrChannekKey, 0, sizeof(arrChannekKey));

		pszValue = pNode->Attribute("channel_key");
		if(NULL == pszValue)
		{
			WRITE_WARN_LOG(SERVER_NAME, "%s is not found channel_key node!\n", m_szConfigFile);
			return 1;
		}

		strcpy(arrChannekKey, pszValue);

		m_stMsgChannelMap[nMsgID] = string(arrChannekKey);

		pNode = pNode->NextSiblingElement();
	}

	return 0;
}

//卸载配置
int32_t CMsgDispatchConfig::Uninit()
{
	return 0;
}

//获取channelkey
string CMsgDispatchConfig::GetChannelKey(int32_t nMsgID)
{
	string strChannelKey = "";
	map<int32_t, string>::iterator it = m_stMsgChannelMap.find(nMsgID);
	if(it != m_stMsgChannelMap.end())
	{
		strChannelKey = it->second;
	}

	return strChannelKey;
}

