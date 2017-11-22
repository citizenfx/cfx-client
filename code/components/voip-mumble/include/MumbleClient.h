/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include <ppltasks.h>

struct MumbleConnectionInfo
{
	bool isConnected;
	fwString hostname;
	uint16_t port;
	fwWString username;
};

class IMumbleClient : public fwRefCountable
{
public:
	virtual void Initialize() = 0;

	virtual concurrency::task<MumbleConnectionInfo*> ConnectAsync(const char* hostname, uint16_t port, const wchar_t* userName) = 0;

	virtual concurrency::task<void> DisconnectAsync() = 0;
};

fwRefContainer<IMumbleClient>
	#ifdef COMPILING_VOIP_MUMBLE
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	CreateMumbleClient();