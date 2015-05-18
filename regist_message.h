/*
 * regist_message.h
 *
 *  Created on: Mar 10, 2015
 *      Author: jimm
 */

#ifndef REGIST_MESSAGE_H_
#define REGIST_MESSAGE_H_

#include "frame/frame.h"
#include "include/msg_head.h"
#include "include/control_head.h"
#include "include/chat_msg.h"
#include "logic/chattoone_handler.h"
#include "logic/chatread_handler.h"

using namespace FRAME;

MSGMAP_BEGIN(msgmap)
ON_PROC_PCH_PMH_PMB_PU8_I32(MSGID_CHATTOONE_REQ, ControlHead, MsgHeadCS, CChatToOneReq, CChatToOneHandler, CChatToOneHandler::ChatToOne);
ON_PROC_PCH_PMH_PMB_PU8_I32(MSGID_CHATREAD_REQ, ControlHead, MsgHeadCS, CChatReadReq, CChatReadHandler, CChatReadHandler::ChatRead);
MSGMAP_END(msgmap)

#endif /* REGIST_MESSAGE_H_ */
