/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "NUIWindow.h"

#include <d3d11.h>

#include <mutex>

class NUIWindowManager
{
private:
	std::vector<NUIWindow*> m_nuiWindows;

	std::mutex m_nuiWindowMutex;

	fwRefContainer<NUIWindow> m_rootWindow;

	ID3D11Texture2D* m_parentTexture;

public:
	void AddWindow(NUIWindow* window);

	void ForAllWindows(std::function<void(fwRefContainer<NUIWindow>)> callback);

	void RemoveWindow(NUIWindow* window);

	inline ID3D11Texture2D* GetParentTexture()
	{
		return m_parentTexture;
	}

	inline void SetParentTexture(ID3D11Texture2D* texture)
	{
		m_parentTexture = texture;
	}

public:
	inline fwRefContainer<NUIWindow> GetRootWindow() { return m_rootWindow; }

	inline void SetRootWindow(fwRefContainer<NUIWindow> window) { m_rootWindow = window; }
};

DECLARE_INSTANCE_TYPE(NUIWindowManager);