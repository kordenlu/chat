/*
 * string_config.cpp
 *
 *  Created on: Mar 14, 2015
 *      Author: jimm
 */

#include "string_config.h"
#include "../../logger/logger.h"
#include "../../tinyxml/tinyxml.h"
#include "../server_typedef.h"

using namespace LOGGER;

//注册到配置管理器
REGIST_CONFIG_SAFE(CONFIG_STRING, CStringConfig)

//初始化配置
int32_t CStringConfig::Init()
{
	return 0;
}

//卸载配置
int32_t CStringConfig::Uninit()
{
	return 0;
}

int32_t CStringConfig::Parser(char *pXMLString)
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

	TiXmlElement *pMsg = pRoot->FirstChildElement("msg");
	if(NULL == pMsg)
	{
		WRITE_WARN_LOG(SERVER_NAME, "%s is not found msg!\n", m_szConfigFile);
		return 1;
	}

	while(pMsg != NULL)
	{
		int32_t nMsgID = 0;
		pszValue = pMsg->Attribute("msgid", &nMsgID);
		if(NULL == pszValue)
		{
			WRITE_WARN_LOG(SERVER_NAME, "%s is not found msgid node!\n", m_szConfigFile);
			return 1;
		}

		TiXmlElement *pError = pMsg->FirstChildElement("error");
		while(pError != NULL)
		{
			int32_t nResult = 0;
			pszValue = pError->Attribute("result", &nResult);
			if(NULL == pszValue)
			{
				WRITE_WARN_LOG(SERVER_NAME, "%s is not found result node!\n", m_szConfigFile);
				return 1;
			}

			char arrString[1024];
			memset(arrString, 0, sizeof(arrString));

			pszValue = pError->Attribute("string");
			if(NULL != pszValue)
			{
				strcpy(arrString, pszValue);
			}

			m_stMsgStringMap[nMsgID][nResult] = arrString;

			pError = pError->NextSiblingElement();
		}

		pMsg = pMsg->NextSiblingElement();
	}

	return 0;
}

string CStringConfig::GetString(int32_t nMsgID, int32_t nResult)
{
	string strTips;

	MsgStringMap::iterator it = m_stMsgStringMap.find(nMsgID);
	if(it != m_stMsgStringMap.end())
	{
		ResultStringMap::iterator et = it->second.find(nResult);
		if(et != it->second.end())
		{
			strTips = et->second;
		}
	}

	return strTips;
}

