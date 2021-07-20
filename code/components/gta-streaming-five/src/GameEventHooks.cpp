#include <StdInc.h>
#include <Hooking.h>

#include <EntitySystem.h>

#include <MinHook.h>

#include <CrossBuildRuntime.h>


namespace rage
{
	class fwEvent
	{
	public:
		virtual ~fwEvent() = 0;

		virtual void m_8() = 0;

		virtual bool Equals(rage::fwEvent* other) = 0;

		virtual int32_t GetId() = 0;

		virtual int m_20() = 0;

		virtual int m_28() = 0;

		virtual bool GetArguments(void* buffer, size_t len) = 0;

		virtual bool m_38() = 0;

		virtual bool m_40(rage::fwEvent* other) = 0;
	};

	class aiEvent : public fwEvent 
	{
	public:
		fwEntity* GetSource() 
		{
			const auto vtable = *reinterpret_cast<uintptr_t*>(this);
			const auto idx = xbr::IsGameBuildOrGreater<2060>() ? 28 : 27;
			return (*reinterpret_cast<fwEntity*(**)(rage::aiEvent*)>(vtable + idx * sizeof(uintptr_t)))(this);
		}
	};
}

class CEventGroup {};

class CEventGroupPed : public CEventGroup 
{
public:
	fwEntity* GetPed() 
	{
		static const auto pedOffset = *hook::pattern("48 8B C6 48 89 BE ? ? ? ?").count(1).get(0).get<uint32_t>(6);
		return *reinterpret_cast<fwEntity**>(this + pedOffset);
	}
};

static hook::cdecl_stub<uint32_t(void*)> getScriptGuidForEntity([]() 
{
	return hook::get_pattern("48 F7 F9 49 8B 48 08 48 63 D0 C1 E0 08 0F B6 1C 11 03 D8", -0x68);
});

void*(*g_eventCall1)(void* group, void* event);
void*(*g_eventCall2)(void* group, void* event);
void*(*g_eventCall3)(void* group, void* event);

template<decltype(g_eventCall1)* TFunc, GameEventType eventType>
void* HandleEventWrap(CEventGroup* group, rage::fwEvent* event)
{
	if (event)
	{
		try
		{
			const char* eventName = typeid(*event).name();

			GameEventMetaData data = { 0 };
			data.type = eventType;
			strcpy(data.name, &eventName[6]);
			data.numArguments = 0;

			switch (eventType)
			{
			case GameEventType::NetworkEvent:
			{
				// brute-force the argument count
				// since these functions should early exit, most cost here will be VMT dispatch
				for (int i = 0; i < _countof(data.arguments); i++) 
				{
					if (event->GetArguments(data.arguments, i * sizeof(uintptr_t))) 
					{
						data.numArguments = i;
						break;
					}
				}

				break;
			}
			case GameEventType::AIEvent:
			{
				const auto ped = static_cast<CEventGroupPed*>(group)->GetPed();

				if (ped != nullptr) 
				{
					data.arguments[data.numArguments++] = getScriptGuidForEntity(ped);

					// The source entity involved in this AI event, if one exists,
					// *should* be accessible by vtable lookup.
					const auto sourceEntity = static_cast<rage::aiEvent*>(event)->GetSource();

					if (sourceEntity != nullptr) 
					{
						data.arguments[data.numArguments++] = getScriptGuidForEntity(sourceEntity);
					}
				}

				break;
			}
			}

			OnTriggerGameEvent(data);
		}
		catch (std::exception& e)
		{
		
		}
	}

	return (*TFunc)(group, event);
}

#include <sstream>

namespace rage
{
class netObject;
}

typedef void (*OnEntityTakeDmgFn)(rage::netObject*, void*, uint8_t);
static OnEntityTakeDmgFn origOnEntityTakeDmg;

static void OnEntityTakeDmg(rage::netObject* thisptr, void* dmgInfo, uint8_t unk)
{
	if (xbr::IsGameBuildOrGreater<2060>())
	{
		// Hack: 2060+ does not set the damageSource for fall damage. This is checked != 0 before sending the event
		if (*((DWORD*)dmgInfo + 8) == 0)
		{
			*((DWORD*)dmgInfo + 8) = 0xCDC174B0; // damageSource = WORLD (hash)
		}
	}
	return origOnEntityTakeDmg(thisptr, dmgInfo, unk);
}

static HookFunction hookFunction([]()
{
	MH_Initialize();

	// 8-bit event pools

	{
		auto matches = hook::pattern("83 BF ? ? 00 00 ? 75 ? 48 8B CF E8 ? ? ? ? 83 BF").count(2);

		MH_CreateHook(matches.get(0).get<void>(-0x36), HandleEventWrap<&g_eventCall1, GameEventType::AIEvent>, (void**)&g_eventCall1);

		// we can't read out any data from this event call yet
		// MH_CreateHook(matches.get(1).get<void>(-0x36), HandleEventWrap<&g_eventCall2, true>, (void**)&g_eventCall2);
	}

	{
		MH_CreateHook(hook::get_pattern("81 BF ? ? 00 00 ? ?  00 00 75 ? 48 8B CF E8", -0x36), HandleEventWrap<&g_eventCall3, GameEventType::NetworkEvent>, (void**)&g_eventCall3);
	}


	// fix for invalid damage sources in events
	uintptr_t* cNetObjPhys_vtable = hook::get_address<uintptr_t*>(hook::get_pattern<unsigned char>("88 44 24 20 E8 ? ? ? ? 33 C9 48 8D 05", 14));
	if (xbr::IsGameBuildOrGreater<2189>())
	{
		MH_CreateHook((LPVOID)cNetObjPhys_vtable[128], OnEntityTakeDmg, (void**)&origOnEntityTakeDmg);
	}
	else if (xbr::IsGameBuildOrGreater<2060>())
	{
		MH_CreateHook((LPVOID)cNetObjPhys_vtable[127], OnEntityTakeDmg, (void**)&origOnEntityTakeDmg);
	}

	MH_EnableHook(MH_ALL_HOOKS);

	/*
	OnTriggerGameEvent.Connect([](const GameEventMetaData& data)
	{
		std::stringstream argStr;
		argStr << "(" << data.numArguments << ") ";
		for (int i = 0; i < data.numArguments; i++)
		{
			argStr << " " << data.arguments[i];
		}
		trace("game event %s, args %s\n", data.name, argStr.str());
	});
	*/
});

fwEvent<const GameEventMetaData&> OnTriggerGameEvent;
