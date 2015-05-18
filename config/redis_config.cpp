/*
 * redis_config.cpp
 *
 *  Created on: Mar 6, 2015
 *      Author: jimm
 */

#include "redis_config.h"
#include "tinyxml/tinyxml.h"
#include "logger/logger.h"
#include "server_typedef.h"

using namespace LOGGER;

//注册到配置管理器
REGIST_CONFIG_SAFE(CONFIG_REDIS, CRedisConfig)

//初始化配置
int32_t CRedisConfig::Init()
{
	return 0;
}

//卸载配置
int32_t CRedisConfig::Uninit()
{
	return 0;
}

int32_t CRedisConfig::Parser(char *pXMLString)
{
	TiXmlDocument doc;
	doc.Parse(pXMLString, 0, TIXML_ENCODING_UTF8);

	//获取根节点
	TiXmlElement *pRoot = doc.RootElement();
	if (NULL == pRoot)
	{
		WRITE_WARN_LOG(SERVER_NAME, "%s is not found redis node!\n", m_szConfigFile);
		return 1;
	}

	const char* pszValue = NULL;

	TiXmlElement *pNode = pRoot->FirstChildElement("node");
	if(NULL == pNode)
	{
		WRITE_WARN_LOG(SERVER_NAME, "%s is not found node!\n", m_szConfigFile);
		return 1;
	}

	while(pNode != NULL)
	{
		char arrServerName[64];
		memset(arrServerName, 0, sizeof(arrServerName));

		pszValue = pNode->Attribute("server_name");
		if(NULL == pszValue)
		{
			WRITE_WARN_LOG(SERVER_NAME, "%s is not found server_name node!\n", m_szConfigFile);
			return 1;
		}

		strcpy(arrServerName, pszValue);

		int32_t nServerID = 0;
		pszValue = pNode->Attribute("server_id", &nServerID);
		if(NULL == pszValue)
		{
			WRITE_WARN_LOG(SERVER_NAME, "%s is not found server_id node!\n", m_szConfigFile);
			return 1;
		}

		char arrServerAddress[64];
		memset(arrServerAddress, 0, sizeof(arrServerAddress));

		pszValue = pNode->Attribute("server_address");
		if(NULL == pszValue)
		{
			WRITE_WARN_LOG(SERVER_NAME, "%s is not found server_address node!\n", m_szConfigFile);
			return 1;
		}

		strcpy(arrServerAddress, pszValue);

		int32_t nServerPort = 0;
		pszValue = pNode->Attribute("server_port", &nServerPort);
		if(NULL == pszValue)
		{
			WRITE_WARN_LOG(SERVER_NAME, "%s is not found server_port node!\n", m_szConfigFile);
			return 1;
		}

		char arrChannelKey[256];
		memset(arrChannelKey, 0, sizeof(arrChannelKey));

		pszValue = pNode->Attribute("channel_key");
		if(NULL != pszValue)
		{
			strcpy(arrChannelKey, pszValue);
		}

		strcpy(m_arrRedisServerInfo[m_nRedisCount].arrServerName, arrServerName);
		m_arrRedisServerInfo[m_nRedisCount].nServerID = nServerID;
		strcpy(m_arrRedisServerInfo[m_nRedisCount].arrServerAddress, arrServerAddress);
		m_arrRedisServerInfo[m_nRedisCount].nPort = nServerPort;
		strcpy(m_arrRedisServerInfo[m_nRedisCount].arrChannelKey, arrChannelKey);
		++m_nRedisCount;

		pNode = pNode->NextSiblingElement();
	}

	return 0;
}

int32_t CRedisConfig::GetRedisServerInfo(RedisServerInfo arrRedisServerInfo[], int32_t nMaxCount)
{
	int32_t nCount = 0;
	for(int32_t i = 0; i < m_nRedisCount; ++i)
	{
		arrRedisServerInfo[nCount++] = m_arrRedisServerInfo[i];
		if(nCount >= nMaxCount - 1)
		{
			break;
		}
	}

	return nCount;
}


