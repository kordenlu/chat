/*
 * msgparser_factory.cpp
 *
 *  Created on: 2014��5��13��
 *      Author: jimm
 */

#include "msgparser_factory.h"
#include "msg_parser.h"
#include "../../common/common_memmgt.h"

CMsgParserFactory::CMsgParserFactory()
{
}

IPacketParser *CMsgParserFactory::Create()
{
	return NEW(CMsgParser);
}

void CMsgParserFactory::Destory(IPacketParser *pParser)
{
	DELETE(pParser);
}
