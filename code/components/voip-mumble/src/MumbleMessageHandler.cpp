/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "MumbleMessageHandler.h"

MumbleMessageHandlerBase* MumbleMessageHandlerBase::ms_messageHandlerRegistry[MAX_MUMBLE_MESSAGE + 1];

MumbleMessageHandlerBase::MumbleMessageHandlerBase(MumbleMessageType messageType)
{
	ms_messageHandlerRegistry[(int)messageType] = this;
	m_messageType = messageType;
}

MumbleMessageHandlerBase* MumbleMessageHandlerBase::GetHandlerFor(MumbleMessageType type)
{
	return ms_messageHandlerRegistry[(int)type];
}

/*MumbleMessageHandler::MumbleMessageHandler(MumbleMessageType messageType, HandlerType handler)
	: MumbleMessageHandlerBase(messageType), m_handler(handler)
{

}*/

void MumbleMessageHandler::HandleMessage(const uint8_t* message, size_t length)
{
	m_handler(message, length);
}