/*
 * checkconn_handler.h
 *
 *  Created on: Mar 7, 2015
 *      Author: jimm
 */

#ifndef CHECKCONN_HANDLER_H_
#define CHECKCONN_HANDLER_H_

#include "../../common/common_object.h"

class CCheckConnHandler : public CBaseObject
{
public:
	CCheckConnHandler()
	{
		Init();
	}

	virtual int32_t Init()
	{
		return 0;
	}

	virtual int32_t Uninit()
	{
		return 0;
	}

	virtual int32_t GetSize()
	{
		return sizeof(*this);
	}

	int32_t CheckConnStatus(void *pTimerData);
};


#endif /* CHECKCONN_HANDLER_H_ */
