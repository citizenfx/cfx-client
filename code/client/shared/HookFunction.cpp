/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "HookFunction.h"

static HookFunctionBase* g_hookFunctions;

void HookFunctionBase::Register()
{
	m_next = g_hookFunctions;
	g_hookFunctions = this;
}

void HookFunctionBase::RunAll()
{
	for (auto func = g_hookFunctions; func; func = func->m_next)
	{
		func->Run();
	}
}

static RuntimeHookFunction* g_runtimeHookFunctions;

void RuntimeHookFunction::Register()
{
	m_next = g_runtimeHookFunctions;
	g_runtimeHookFunctions = this;
}

void RuntimeHookFunction::Run(const char* key)
{
	for (auto func = g_runtimeHookFunctions; func; func = func->m_next)
	{
		if (func->m_key == key)
		{
			func->m_function();
		}
	}
}