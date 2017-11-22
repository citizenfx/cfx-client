/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include <ICoreGameInit.h>

class
#ifdef COMPILING_GTA_CORE_FIVE
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	FiveGameInit : public ICoreGameInit
{
private:
	bool m_gameLoaded;

public:
	virtual bool GetGameLoaded() override;

	virtual void KillNetwork(const wchar_t* errorString) override;

	virtual bool TryDisconnect() override;

	virtual void SetPreventSavePointer(bool* preventSaveValue) override;

	virtual void LoadGameFirstLaunch(bool(*callBeforeLoad)()) override;

	virtual void ReloadGame() override;

	virtual bool TriggerError(const char* message) override;

	void SetGameLoaded();
};

extern
#ifdef COMPILING_GTA_CORE_FIVE
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	FiveGameInit g_gameInit;

extern
#ifdef COMPILING_GTA_CORE_FIVE
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	fwEvent<const char*> OnKillNetwork;

extern
	#ifdef COMPILING_GTA_CORE_FIVE
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	fwEvent<> OnKillNetworkDone;