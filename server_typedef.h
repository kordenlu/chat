/*
 * server_typedef.h
 *
 *  Created on: 2014年1月23日
 *      Author: jimm
 */

#ifndef SERVER_TYPEDEF_H_
#define SERVER_TYPEDEF_H_

#include "../common/common_typedef.h"

#define SERVER_NAME			"chat"

#define CONFIG_SERVER		"./server_config.xml"
#define CONFIG_REDIS		"./redis_config.xml"
#define CONFIG_MSGDISPATCH	"./msgdispatch_config.xml"
#define CONFIG_STRING		"./string_config.xml"

#define BANK_REDIS			"redis_bank"

typedef int32_t			ServerID;
enum
{
	enmInvalidServerID		= 0,
};

enum
{
	enmMaxConfigFileNameSize	= 1024,
	enmMaxPacketTokenSize		= 64,
	enmIPV4NetAddressSize		= 16,
	enmMaxNetNodeCount			= 32,
	enmMaxRedisCount			= 128,
};

#endif /* SERVER_TYPEDEF_H_ */
