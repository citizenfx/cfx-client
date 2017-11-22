/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "ConsoleHost.h"
#include "ConsoleHostImpl.h"
#include "InputHook.h"
#include <thread>
#include <condition_variable>

#include <CoreConsole.h>

static std::thread g_consoleThread;
static std::once_flag g_consoleInitialized;
bool g_consoleFlag;
extern int g_scrollTop;
extern int g_bufferHeight;

static InitFunction initFunction([] ()
{
	console::CoreAddPrintListener([](ConsoleChannel channel, const char* msg)
	{
		ConHost::Print(0, msg);
	});

	static ConsoleCommand quitCommand("quit", []()
	{
		TerminateProcess(GetCurrentProcess(), -1);
	});

	InputHook::OnWndProc.Connect([] (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& lresult)
	{
		if (g_consoleFlag)
		{
			static bool g_consoleClosing = false;

			// should the console be closed?
			if (wParam == VK_F8)
			{
				if (msg == WM_KEYDOWN)
				{
					g_consoleClosing = true;
					return;
				}

				if (g_consoleClosing)
				{
					if (msg == WM_KEYUP)
					{
						g_consoleClosing = false;
						g_consoleFlag = false;

						return;
					}
				}
			}
		}
		else
		{
			// check if the console should be opened
			if (msg == WM_KEYUP && wParam == VK_F8)
			{
				g_consoleFlag = true;

				pass = false;
				lresult = 0;

				return;
			}
		}
	}, -10);
});
