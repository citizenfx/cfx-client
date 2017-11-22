/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "memdbgon.h"

void NUIApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{
	// add the 'nui://' internal scheme
	registrar->AddCustomScheme("nui", true, false, false, true, false, true);
}

// null data resource functions
bool NUIApp::GetDataResource(int resourceID, void*& data, size_t& data_size)
{
	return false;
}

bool NUIApp::GetDataResourceForScale(int resource_id, ScaleFactor scale_factor, void*& data, size_t& data_size)
{
	return false;
}

bool NUIApp::GetLocalizedString(int messageID, CefString& string)
{
	string = "";
	return true;
}

void NUIApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
	CefRefPtr<CefV8Value> window = context->GetGlobal();

	window->SetValue("registerPollFunction", CefV8Value::CreateFunction("registerPollFunction", this), V8_PROPERTY_ATTRIBUTE_READONLY);
	window->SetValue("registerFrameFunction", CefV8Value::CreateFunction("registerFrameFunction", this), V8_PROPERTY_ATTRIBUTE_READONLY);
	window->SetValue("invokeNative", CefV8Value::CreateFunction("invokeNative", this), V8_PROPERTY_ATTRIBUTE_READONLY);
}

void NUIApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	command_line->AppendSwitch("enable-experimental-web-platform-features");
	command_line->AppendSwitch("in-process-gpu");
	command_line->AppendSwitch("enable-media-stream");
	command_line->AppendSwitch("use-fake-ui-for-media-stream");
	command_line->AppendSwitch("enable-speech-input");
	command_line->AppendSwitch("ignore-gpu-blacklist");
	command_line->AppendSwitch("enable-usermedia-screen-capture");
	command_line->AppendSwitchWithValue("default-encoding", "utf-8");

	if (IsWindows10OrGreater())
	{
		// this is broken pending https://crbug.com/480361
		// (WasResized breaks internal ANGLE/cc state, causing Alt-Enter to 'break' NUI)
		//command_line->AppendSwitch("disable-gpu-vsync");
		command_line->AppendSwitch("force-gpu-rasterization");
	}
}

bool NUIApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	auto handler = m_processMessageHandlers.find(message->GetName());
	bool success = false;

	if (handler != m_processMessageHandlers.end())
	{
		success = handler->second(browser, message);
	}
	else
	{
		trace("Unknown NUI process message: %s\n", message->GetName().ToString().c_str());
	}

	return success;
}

CefRefPtr<CefRenderProcessHandler> NUIApp::GetRenderProcessHandler()
{
	return this;
}

bool NUIApp::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
	auto handler = m_v8Handlers.find(name);
	bool success = false;

	if (handler != m_v8Handlers.end())
	{
		retval = handler->second(arguments, exception);

		success = true;
	}
	else
	{
		trace("Unknown NUI function: %s\n", name.ToString().c_str());
	}

	return success;
}

void NUIApp::AddProcessMessageHandler(std::string key, TProcessMessageHandler handler)
{
	m_processMessageHandlers[key] = handler;
}

void NUIApp::AddV8Handler(std::string key, TV8Handler handler)
{
	m_v8Handlers[key] = handler;
}
