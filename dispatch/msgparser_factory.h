/*
 * msgparser_factory.h
 *
 *  Created on: 2014��5��12��
 *      Author: jimm
 */

#ifndef MSGPARSER_FACTORY_H_
#define MSGPARSER_FACTORY_H_

#include "netevent/net_namespace.h"
#include "netevent/net_impl.h"

using namespace NETEVENT;

class CMsgParserFactory : public IPacketParserFactory
{
public:
	CMsgParserFactory();

	virtual IPacketParser *Create();

	virtual void Destory(IPacketParser *pParser);

};


#endif /* MSGPARSER_FACTORY_H_ */

