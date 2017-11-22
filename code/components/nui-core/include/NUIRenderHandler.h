/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include "NUIClient.h"
#include <include/cef_render_handler.h>

class NUIRenderHandler : public CefRenderHandler
{
public:
	NUIRenderHandler(NUIClient* client);

private:
	NUIClient* m_owner;

	// used to prevent infinite recursion when redrawing the popup rectangle
	bool m_paintingPopup;

	CefRect m_popupRect;

protected:
	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

	virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) override;

	virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;

	virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;

	virtual void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser, const CefRange& selected_range, const RectList& character_bounds) override;

private:
	void PaintView(const RectList& dirtyRects, const void* buffer, int width, int height);

	void PaintPopup(const void* buffer, int width, int height);

	void UpdatePopup();

	IMPLEMENT_REFCOUNTING(NUIRenderHandler);
};
