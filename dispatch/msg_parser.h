/*
 * msg_parser.h
 *
 *  Created on: 2014��5��12��
 *      Author: jimm
 */

#ifndef MSG_PARSER_H_
#define MSG_PARSER_H_

#include "../../netevent/net_namespace.h"
#include "../../netevent/net_impl.h"
#include "../../netevent/net_packet.h"

#include <string.h>

using namespace NETEVENT;

class CMsgParser : public IPacketParser
{
public:

	virtual ~CMsgParser(){};

	virtual int32_t Init();

	virtual int32_t Uninit();

	virtual int32_t GetSize();

	virtual int32_t Parser(const uint8_t arrInputBuf[], const uint32_t nInputBufSize, uint8_t arrOutputBuf[], int32_t &nOutputBufSize);

};

#endif /* MSG_PARSER_H_ */
