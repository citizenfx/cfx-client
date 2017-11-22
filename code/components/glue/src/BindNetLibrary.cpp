/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include <NetLibrary.h>
#include <CefOverlay.h>
#include <ICoreGameInit.h>
#include <GameInit.h>
#include <nutsnbolts.h>

#ifdef GTA_FIVE
static InitFunction initFunction([] ()
{
	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* library)
	{
		static NetLibrary* netLibrary = library;

		library->OnStateChanged.Connect([] (NetLibrary::ConnectionState curState, NetLibrary::ConnectionState lastState)
		{
			if (curState == NetLibrary::CS_ACTIVE)
			{
				ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();

				if (!gameInit->GetGameLoaded())
				{
					trace("Triggering LoadGameFirstLaunch()\n");

					gameInit->LoadGameFirstLaunch([]()
					{
						// download frame code
						Sleep(1);

						return netLibrary->AreDownloadsComplete();
					});
				}
				else
				{
					trace("Triggering ReloadGame()\n");

					gameInit->ReloadGame();
				}
			}
		});

		OnKillNetwork.Connect([=] (const char* message)
		{
			library->Disconnect(message);
		});

		OnKillNetworkDone.Connect([=]()
		{
			library->FinalizeDisconnect();
		});

		Instance<ICoreGameInit>::Get()->OnGameRequestLoad.Connect([] ()
		{
			nui::SetMainUI(false);

			nui::DestroyFrame("mpMenu");
		});
	});

	OnFirstLoadCompleted.Connect([] ()
	{
		g_gameInit.SetGameLoaded();
	});
});
#endif