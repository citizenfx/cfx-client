/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include <GameInit.h>

#include <GlobalEvents.h>
#include <nutsnbolts.h>

#include <gameSkeleton.h>

#include <Hooking.h>

FiveGameInit g_gameInit;

fwEvent<const char*> OnKillNetwork;
fwEvent<> OnKillNetworkDone;

void AddCustomText(const char* key, const char* value);

static hook::cdecl_stub<void(int unk, uint32_t* titleHash, uint32_t* messageHash, uint32_t* subMessageHash, int flags, bool, int8_t, void*, void*, bool, bool)> setWarningMessage([] ()
{
	return hook::get_call<void*>(hook::get_call(hook::pattern("57 41 56 41 57 48 83 EC 50 4C 63 F2").count(1).get(0).get<char>(0xAC)) + 0x6D);
});

static hook::cdecl_stub<int(bool, int)> getWarningResult([] ()
{
	return hook::get_call(hook::pattern("33 D2 33 C9 E8 ? ? ? ? 48 83 F8 04 0F 84").count(1).get(0).get<void>(4));
});

static bool g_showWarningMessage;
static std::string g_warningMessage;

void FiveGameInit::KillNetwork(const wchar_t* errorString)
{
	if (errorString == (wchar_t*)1)
	{
		OnKillNetwork(nullptr);
	}
	else
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		std::string smallReason = converter.to_bytes(errorString);

		if (!g_showWarningMessage)
		{
			g_warningMessage = smallReason;
			g_showWarningMessage = true;

			SetData("warningMessage", g_warningMessage);

			OnKillNetwork(smallReason.c_str());
		}
	}
}

bool FiveGameInit::GetGameLoaded()
{
	return m_gameLoaded;
}

void FiveGameInit::SetGameLoaded()
{
	m_gameLoaded = true;
}

void FiveGameInit::SetPreventSavePointer(bool* preventSaveValue)
{

}

bool FiveGameInit::TryDisconnect()
{
	return true;
}

bool FiveGameInit::TriggerError(const char* message)
{
	return (!OnTriggerError(message));
}

static hook::cdecl_stub<void(rage::InitFunctionType)> gamerInfoMenu_init([]()
{
	return hook::get_pattern("83 F9 08 75 3F 53 48 83 EC 20 48 83 3D", 0);
});

static hook::cdecl_stub<void(rage::InitFunctionType)> gamerInfoMenu__shutdown([]()
{
	return hook::get_pattern("83 F9 08 75 46 53 48 83 EC 20 48 83", 0);
});

static void(*g_origLoadMultiplayerTextChat)();

static HookFunction hookFunction([]()
{
	// really bad pattern pointing to switch-to-netgame
	auto location = hook::get_pattern("E8 ? ? ? ? B9 08 00 00 00 E8 ? ? ? ? E8", 15);

	hook::set_call(&g_origLoadMultiplayerTextChat, location);
	hook::nop(location, 5);

	// disable gamer info menu shutdown (testing/temp dbg for blocking loads on host/join)
	//hook::return_function(hook::get_pattern("83 F9 08 75 46 53 48 83 EC 20 48 83", 0));

	// force SET_GAME_PAUSES_FOR_STREAMING (which makes collision loading, uh, blocking) to be off
	hook::put<uint8_t>(hook::get_pattern("74 20 84 C9 74 1C 84 DB 74 18", 0), 0xEB);
});

static InitFunction initFunction([] ()
{
	OnGameFrame.Connect([] ()
	{
		if (g_showWarningMessage)
		{
			g_showWarningMessage = false;

			OnMsgConfirm();
		}
	});

	OnKillNetworkDone.Connect([]()
	{
		gamerInfoMenu__shutdown(rage::INIT_SESSION);
	});

	rage::OnInitFunctionEnd.Connect([] (rage::InitFunctionType type)
	{
		if (type == rage::INIT_SESSION)
		{
			// early-init the mp_gamer_info menu (doing this when switching will cause blocking LoadObjectsNow calls)
			gamerInfoMenu_init(rage::INIT_SESSION);

			// also early-load MULTIPLAYER_TEXT_CHAT gfx, this changed sometime between 323 and 505
			// and also causes a blocking load.
			g_origLoadMultiplayerTextChat();

			g_gameInit.SetGameLoaded();
		}
	});

	Instance<ICoreGameInit>::Set(&g_gameInit);
});
