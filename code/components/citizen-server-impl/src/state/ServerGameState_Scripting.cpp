#include <StdInc.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>
#include <state/ServerGameState.h>

#include <ResourceManager.h>
#include <ScriptEngine.h>

#include <ScriptSerialization.h>

static InitFunction initFunction([]()
{
	auto makeEntityFunction = [](auto fn, uintptr_t defaultValue = 0)
	{
		return [=](fx::ScriptContext& context)
		{
			// get the current resource manager
			auto resourceManager = fx::ResourceManager::GetCurrent();

			// get the owning server instance
			auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

			// get the server's game state
			auto gameState = instance->GetComponent<fx::ServerGameState>();

			// parse the client ID
			auto id = context.GetArgument<uint32_t>(0);

			if (!id)
			{
				context.SetResult(defaultValue);
				return;
			}

			auto entity = gameState->GetEntity(id);

			if (!entity)
			{
				throw std::runtime_error(va("Tried to access invalid entity: %d", id));

				context.SetResult(defaultValue);
				return;
			}

			context.SetResult(fn(context, entity));
		};
	};

	struct scrVector
	{
		float x;
		int pad;
		float y;
		int pad2;
		float z;
		int pad3;
	};

	fx::ScriptEngine::RegisterNativeHandler("DOES_ENTITY_EXIST", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		// parse the client ID
		auto id = context.GetArgument<uint32_t>(0);

		if (!id)
		{
			context.SetResult(false);
			return;
		}

		auto entity = gameState->GetEntity(id);

		if (!entity)
		{
			context.SetResult(false);
			return;
		}

		context.SetResult(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_ENTITY_FROM_NETWORK_ID", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		// parse the client ID
		auto id = context.GetArgument<uint32_t>(0);

		if (!id)
		{
			context.SetResult(0);
			return;
		}

		auto entity = gameState->GetEntity(0, id);

		if (!entity)
		{
			context.SetResult(0);
			return;
		}

		context.SetResult(gameState->MakeScriptHandle(entity));
	});

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_ENTITY_OWNER", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		int retval = -1;
		auto entry = entity->client.lock();

		if (entry)
		{
			retval = entry->GetNetId();
		}

		return retval;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_COORDS", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		float position[3];
		entity->syncTree->GetPosition(position);

		scrVector resultVec = { 0 };
		resultVec.x = position[0];
		resultVec.y = position[1];
		resultVec.z = position[2];

		return resultVec;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_VELOCITY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		scrVector resultVec = { 0 };

		auto v = entity->syncTree->GetVelocity();

		if (v)
		{
			resultVec.x = v->velX;
			resultVec.y = v->velY;
			resultVec.z = v->velZ;
		}

		return resultVec;
	}));

	static const float pi = 3.14159265358979323846f;

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_ROTATION_VELOCITY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto av = entity->syncTree->GetAngVelocity();

		scrVector resultVec = { 0 };

		if (av)
		{
			resultVec.x = av->angVelX;
			resultVec.y = av->angVelY;
			resultVec.z = av->angVelZ;
		}

		return resultVec;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_ROTATION", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		scrVector resultVec = { 0 };

		if (entity->type == fx::sync::NetObjEntityType::Player || entity->type == fx::sync::NetObjEntityType::Ped)
		{
			resultVec.x = 0.0f;
			resultVec.y = 0.0f;

			auto pn = entity->syncTree->GetPedOrientation();

			if (pn)
			{
				resultVec.z = pn->currentHeading * 180.0 / pi;
			}
		}
		else
		{
			auto en = entity->syncTree->GetEntityOrientation();

			if (en)
			{
				resultVec.x = en->rotX * 180.0 / pi;
				resultVec.y = en->rotY * 180.0 / pi;
				resultVec.z = en->rotZ * 180.0 / pi;
			}
		}

		return resultVec;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_HEADING", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		float heading = 0.0f;

		if (entity->type == fx::sync::NetObjEntityType::Player || entity->type == fx::sync::NetObjEntityType::Ped)
		{
			auto pn = entity->syncTree->GetPedOrientation();

			if (pn)
			{
				heading = pn->currentHeading * 180.0 / pi;
			}
		}
		else
		{
			auto en = entity->syncTree->GetEntityOrientation();

			if (en)
			{
				heading = en->rotZ * 180.0 / pi;
			}
		}

		return (heading < 0) ? 360.0f + heading : heading;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_POPULATION_TYPE", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		fx::sync::ePopType popType = fx::sync::POPTYPE_UNKNOWN;
		entity->syncTree->GetPopulationType(&popType);

		return popType;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_MODEL", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		uint32_t model = 0;
		entity->syncTree->GetModelHash(&model);

		return model;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_SCRIPT", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		uint32_t script = 0;
		if (entity->syncTree->GetScriptHash(&script))
		{
			static std::string scriptName;
			scriptName.clear();

			auto resourceManager = fx::ResourceManager::GetCurrent();
			resourceManager->ForAllResources([script](const fwRefContainer<fx::Resource>& resource)
			{
				if (scriptName.empty())
				{
					std::string subName = resource->GetName();

					if (subName.length() > 63)
					{
						subName = subName.substr(0, 63);
					}

					if (HashString(subName.c_str()) == script)
					{
						scriptName = resource->GetName();
					}
				}
			});

			if (!scriptName.empty())
			{
				return scriptName.c_str();
			}
		}

		return (const char*)nullptr;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_TYPE", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		switch (entity->type)
		{
		case fx::sync::NetObjEntityType::Automobile:
		case fx::sync::NetObjEntityType::Bike:
		case fx::sync::NetObjEntityType::Boat:
		case fx::sync::NetObjEntityType::Heli:
		case fx::sync::NetObjEntityType::Plane:
		case fx::sync::NetObjEntityType::Submarine:
		case fx::sync::NetObjEntityType::Trailer:
		case fx::sync::NetObjEntityType::Train:
			return 2;
		case fx::sync::NetObjEntityType::Ped:
		case fx::sync::NetObjEntityType::Player:
			return 1;
		case fx::sync::NetObjEntityType::Object:
		case fx::sync::NetObjEntityType::Door:
		case fx::sync::NetObjEntityType::Pickup:
			return 3;
		default:
			return 0;
		}
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_SYNC_ENTITY_LOCKDOWN_MODE", [](fx::ScriptContext& context)
	{
		std::string_view sv = context.CheckArgument<const char*>(0);

		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		if (sv == "strict")
		{
			gameState->SetEntityLockdownMode(fx::EntityLockdownMode::Strict);
		}
		else if (sv == "relaxed")
		{
			gameState->SetEntityLockdownMode(fx::EntityLockdownMode::Relaxed);
		}
		else if (sv == "inactive")
		{
			gameState->SetEntityLockdownMode(fx::EntityLockdownMode::Inactive);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_NETWORK_ID_FROM_ENTITY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->handle & 0xFFFF;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_HASH_KEY", [](fx::ScriptContext& context)
	{
		context.SetResult(HashString(context.GetArgument<const char*>(0)));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_RADIO_STATION_INDEX", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		return vn ? vn->radioStation : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_LIGHTS_STATE", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 2)
		{
			auto vn = entity->syncTree->GetVehicleGameState();

			*context.GetArgument<int*>(1) = vn ? vn->lightsOn : false;
			*context.GetArgument<int*>(2) = vn ? vn->highbeamsOn : false;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_IS_VEHICLE_ENGINE_RUNNING", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		return vn ? vn->isEngineOn : false;
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_ENGINE_STARTING", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		return vn ? vn->isEngineStarting : false;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HANDBRAKE", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		return vn ? vn->handbrake : false;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HEADLIGHTS_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		int headlightsColour = 0;

		auto vn = entity->syncTree->GetVehicleGameState();

		if (vn)
		{
			if (vn->defaultHeadlights)
			{
				headlightsColour = vn->headlightsColour;
			}
		}

		return headlightsColour;
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_SIREN_ON", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		return vn ? vn->sirenOn : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DOOR_STATUS", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		if (!vn)
		{
			return 0;
		}

		int doorStatus = 0;
		int doorsOpen = vn->doorsOpen;

		if (context.GetArgumentCount() > 1 && doorsOpen)
		{
			doorStatus = vn->doorPositions[context.GetArgument<int>(1)];
		}

		return doorStatus;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DOOR_LOCK_STATUS", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		return vn ? vn->lockStatus : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DOORS_LOCKED_FOR_PLAYER", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		if (!vn)
		{
			return false;
		}

		bool lockedForPlayer = false;
		int hasLock = vn->hasLock;

		if (hasLock)
		{
			int lockedPlayers = vn->lockedPlayers;

			if (lockedPlayers == -1) // ALL
			{
				lockedForPlayer = true;
			}
		}

		return lockedForPlayer;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ENGINE_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleHealth();

		return vn ? vn->engineHealth : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_PETROL_TANK_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleHealth();

		return vn ? vn->petrolTankHealth : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_TYRE_BURST", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleHealth();

		if (!vn)
		{
			return false;
		}

		bool tyreBurst = false;
		bool wheelsFine = vn->tyresFine;

		if (!wheelsFine && context.GetArgumentCount() > 1)
		{
			int tyreID = context.GetArgument<int>(1);
			bool completely = context.GetArgument<bool>(2);

			int tyreStatus = vn->tyreStatus[tyreID];

			if (completely && tyreStatus == 2 || !completely && tyreStatus == 1)
			{
				tyreBurst = true;
			}
		}

		return tyreBurst;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_BODY_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleHealth();

		return vn ? vn->bodyHealth : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_MAX_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto pn = entity->syncTree->GetPedHealth();

		return pn ? pn->maxHealth : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_ARMOUR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto pn = entity->syncTree->GetPedHealth();

		return pn ? pn->armour : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_CAUSE_OF_DEATH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto pn = entity->syncTree->GetPedHealth();

		return pn ? pn->causeOfDeath : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_DESIRED_HEADING", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (entity->type == fx::sync::NetObjEntityType::Player || entity->type == fx::sync::NetObjEntityType::Ped)
		{
			auto pn = entity->syncTree->GetPedOrientation();
			float heading = pn->desiredHeading * 180.0 / pi;
			return (heading < 0) ? 360.0f + heading : heading;
		}

		return 0.0f;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_MAX_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		switch (entity->type)
		{
		case fx::sync::NetObjEntityType::Player:
		case fx::sync::NetObjEntityType::Ped:
		{
			auto pn = entity->syncTree->GetPedHealth();
			return pn->maxHealth;
		}
		default:
			return 0;
		}
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		switch (entity->type)
		{
		case fx::sync::NetObjEntityType::Player:
		case fx::sync::NetObjEntityType::Ped:
		{
			auto pn = entity->syncTree->GetPedHealth();
			return pn->health;
		}
		default:
			return 0;
		}
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_COLOURS", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 2)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = vn ? vn->primaryColour : 0;
			*context.GetArgument<int*>(2) = vn ? vn->secondaryColour : 0;

		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_EXTRA_COLOURS", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 2)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = vn ? vn->pearlColour : 0;
			*context.GetArgument<int*>(2) = vn ? vn->wheelColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_INTERIOR_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 1)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = vn ? vn->interiorColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 1)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = vn ? vn->dashboardColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CUSTOM_PRIMARY_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 3)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = (vn && vn->isPrimaryColourRGB) ? vn->primaryRedColour : 0;
			*context.GetArgument<int*>(2) = (vn && vn->isPrimaryColourRGB) ? vn->primaryGreenColour : 0;
			*context.GetArgument<int*>(3) = (vn && vn->isPrimaryColourRGB) ? vn->primaryBlueColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CUSTOM_SECONDARY_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 3)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = (vn && vn->isSecondaryColourRGB) ? vn->secondaryRedColour : 0;
			*context.GetArgument<int*>(2) = (vn && vn->isSecondaryColourRGB) ? vn->secondaryGreenColour : 0;
			*context.GetArgument<int*>(3) = (vn && vn->isSecondaryColourRGB) ? vn->secondaryBlueColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_IS_VEHICLE_PRIMARY_COLOUR_CUSTOM", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return vn ? vn->isPrimaryColourRGB : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return vn ? vn->isSecondaryColourRGB : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_LIVERY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return vn ? vn->liveryIndex : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ROOF_LIVERY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return vn ? vn->roofLiveryIndex : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DIRT_LEVEL", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return vn ? vn->dirtLevel : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_TYPE", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return vn ? vn->wheelType : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WINDOW_TINT", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return (vn)
			? ((vn->windowTintIndex == 255) ? -1 : vn->windowTintIndex)
			: 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_TYRE_SMOKE_COLOR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 2)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = vn ? vn->tyreSmokeRedColour : 0;
			*context.GetArgument<int*>(2) = vn ? vn->tyreSmokeGreenColour : 0;
			*context.GetArgument<int*>(3) = vn ? vn->tyreSmokeBlueColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NUMBER_PLATE_TEXT", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		if (vn)
		{
			return (const char*)vn->plate;
		}
		
		return (const char*)"";
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		if (vn)
		{
			return vn->numberPlateTextIndex;
		}

		return 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("HAS_VEHICLE_BEEN_OWNED_BY_PLAYER", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		if (vn)
		{
			return vn->hasBeenOwnedByPlayer;
		}

		return false;
	}));

	fx::ScriptEngine::RegisterNativeHandler("HAS_ENTITY_BEEN_MARKED_AS_NO_LONGER_NEEDED", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		if (vn)
		{
			return vn->noLongerNeeded;
		}

		return false;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ALL_VEHICLES", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		std::vector<int> entityList;
		std::shared_lock<std::shared_mutex> lock(gameState->m_entityListMutex);

		for (auto& entity : gameState->m_entityList)
		{
			if (entity && (entity->type == fx::sync::NetObjEntityType::Automobile ||
				entity->type == fx::sync::NetObjEntityType::Bike ||
				entity->type == fx::sync::NetObjEntityType::Boat ||
				entity->type == fx::sync::NetObjEntityType::Heli ||
				entity->type == fx::sync::NetObjEntityType::Plane ||
				entity->type == fx::sync::NetObjEntityType::Submarine ||
				entity->type == fx::sync::NetObjEntityType::Trailer ||
				entity->type == fx::sync::NetObjEntityType::Train))
			{
				entityList.push_back(gameState->MakeScriptHandle(entity));
			}
		}

		context.SetResult(fx::SerializeObject(entityList));
	});

	fx::ScriptEngine::RegisterNativeHandler("DELETE_ENTITY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto resourceManager = fx::ResourceManager::GetCurrent();
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		gameState->DeleteEntity(entity);

		return 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_SELECTED_PED_WEAPON", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto node = entity->syncTree->GetPedGameState();

		return uint32_t(node ? node->curWeapon : 0);
	}));
});
