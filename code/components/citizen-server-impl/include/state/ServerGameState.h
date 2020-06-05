#pragma once

#include <Client.h>
#include <LRWeakPtr.h>

#include <ServerInstanceBase.h>

#include <state/Pool.h>

#include <state/RlMessageBuffer.h>

#include <variant>

#include <array>
#include <optional>
#include <EASTL/bitset.h>
#include <EASTL/fixed_hash_map.h>
#include <shared_mutex>

#include <tbb/concurrent_unordered_map.h>
#include <thread_pool.hpp>

#define GLM_ENABLE_EXPERIMENTAL

// TODO: clang style defines/checking
#if defined(_M_IX86) || defined(_M_AMD64)
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SSE2
#define GLM_FORCE_SSE3
#endif

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

namespace fx
{
struct ScriptGuid;
}

extern CPool<fx::ScriptGuid>* g_scriptHandlePool;

namespace fx::sync
{
struct SyncEntityState;

struct SyncParseState
{
	rl::MessageBuffer buffer;
	int syncType;
	int objType;

	std::shared_ptr<SyncEntityState> entity;

	uint64_t frameIndex;
};

struct SyncUnparseState
{
	rl::MessageBuffer& buffer;
	int syncType;
	int objType;

	std::shared_ptr<Client> client;

	SyncUnparseState(rl::MessageBuffer& buffer)
		: buffer(buffer)
	{

	}
};

struct NodeBase;

using SyncTreeVisitor = std::function<bool(NodeBase&)>;

struct NodeBase
{
public:
	eastl::bitset<MAX_CLIENTS> ackedPlayers;

	uint64_t frameIndex;

	virtual bool Parse(SyncParseState& state) = 0;

	virtual bool Unparse(SyncUnparseState& state) = 0;

	virtual bool Visit(const SyncTreeVisitor& visitor) = 0;
};

struct CPlayerCameraNodeData
{
	int camMode;
	float freeCamPosX;
	float freeCamPosY;
	float freeCamPosZ;

	float cameraX;
	float cameraZ;

	float camOffX;
	float camOffY;
	float camOffZ;
};

struct CPedGameStateNodeData
{
	int curVehicle;
	int curVehicleSeat;

	int lastVehicle;
	int lastVehicleSeat;

	int lastVehiclePedWasIn;

	int curWeapon;

	inline CPedGameStateNodeData()
		: lastVehicle(-1), lastVehicleSeat(-1), lastVehiclePedWasIn(-1)
	{

	}
};

struct CVehicleAppearanceNodeData
{
	int primaryColour;
	int secondaryColour;
	int pearlColour;
	int wheelColour;
	int interiorColour;
	int dashboardColour;

	bool isPrimaryColourRGB;
	bool isSecondaryColourRGB;

	int primaryRedColour;
	int primaryGreenColour;
	int primaryBlueColour;
	
	int secondaryRedColour;
	int secondaryGreenColour;
	int secondaryBlueColour;

	int dirtLevel;
	int liveryIndex;
	int roofLiveryIndex;

	int wheelChoice;
	int wheelType;

	bool hasCustomTires;

	int windowTintIndex;

	int tyreSmokeRedColour;
	int tyreSmokeGreenColour;
	int tyreSmokeBlueColour;

	char plate[9];

	int numberPlateTextIndex;

	inline CVehicleAppearanceNodeData()
	{
		memset(plate, 0, sizeof(plate));
	}
};

struct CVehicleHealthNodeData
{
	int engineHealth;
	int petrolTankHealth;
	bool tyresFine;
	int tyreStatus[1 << 4];
	int bodyHealth;

};

struct CVehicleGameStateNodeData
{
	uint16_t occupants[32];
	eastl::bitset<32> playerOccupants;
	int radioStation;
	bool isEngineOn;
	bool isEngineStarting;
	bool handbrake;
	int defaultHeadlights;
	int headlightsColour;
	bool sirenOn;
	int lockStatus;
	int doorsOpen;
	int doorPositions[1 << 7];
	bool noLongerNeeded;
	bool lightsOn;
	bool highbeamsOn;
	bool hasBeenOwnedByPlayer;
	bool hasLock;
	int lockedPlayers;

	inline CVehicleGameStateNodeData()
	{
		memset(occupants, 0, sizeof(occupants));
	}
};

struct CEntityOrientationNodeData
{
	float rotX;
	float rotY;
	float rotZ;
};

struct CPhysicalVelocityNodeData
{
	float velX;
	float velY;
	float velZ;
};

struct CVehicleAngVelocityNodeData
{
	float angVelX;
	float angVelY;
	float angVelZ;
};

struct CPedHealthNodeData
{
	int maxHealth;
	int health;
	int armour;
	uint32_t causeOfDeath;

};

struct CPedOrientationNodeData
{
	float currentHeading;
	float desiredHeading;
};

enum ePopType
{
	POPTYPE_UNKNOWN = 0,
	POPTYPE_RANDOM_PERMANENT,
	POPTYPE_RANDOM_PARKED,
	POPTYPE_RANDOM_PATROL,
	POPTYPE_RANDOM_SCENARIO,
	POPTYPE_RANDOM_AMBIENT,
	POPTYPE_PERMANENT,
	POPTYPE_MISSION,
	POPTYPE_REPLAY,
	POPTYPE_CACHE,
	POPTYPE_TOOL
};

struct SyncTreeBase
{
public:
	virtual ~SyncTreeBase() = default;

	virtual void Parse(SyncParseState& state) = 0;

	virtual bool Unparse(SyncUnparseState& state) = 0;

	virtual void Visit(const SyncTreeVisitor& visitor) = 0;

	// accessors
public:
	virtual void GetPosition(float* posOut) = 0;

	virtual CPlayerCameraNodeData* GetPlayerCamera() = 0;

	virtual CPedGameStateNodeData* GetPedGameState() = 0;

	virtual CVehicleGameStateNodeData* GetVehicleGameState() = 0;

	virtual CVehicleAppearanceNodeData* GetVehicleAppearance() = 0;

	virtual CPedHealthNodeData* GetPedHealth() = 0;

	virtual CVehicleHealthNodeData* GetVehicleHealth() = 0;

	virtual CPedOrientationNodeData* GetPedOrientation() = 0;

	virtual CEntityOrientationNodeData* GetEntityOrientation() = 0;

	virtual CVehicleAngVelocityNodeData* GetAngVelocity() = 0;

	virtual CPhysicalVelocityNodeData* GetVelocity() = 0;

	virtual void CalculatePosition() = 0;

	virtual bool GetPopulationType(ePopType* popType) = 0;

	virtual bool GetModelHash(uint32_t* modelHash) = 0;

	virtual bool GetScriptHash(uint32_t* scriptHash) = 0;
};

enum class NetObjEntityType
{
	Automobile = 0,
	Bike = 1,
	Boat = 2,
	Door = 3,
	Heli = 4,
	Object = 5,
	Ped = 6,
	Pickup = 7,
	PickupPlacement = 8,
	Plane = 9,
	Submarine = 10,
	Player = 11,
	Trailer = 12,
	Train = 13
};

struct SyncEntityState
{
	using TData = std::variant<int, float, bool, std::string>;

	std::shared_mutex clientMutex;
	std::weak_ptr<fx::Client> client;
	NetObjEntityType type;
	eastl::bitset<MAX_CLIENTS> ackedCreation;
	eastl::bitset<MAX_CLIENTS> didDeletion;
	uint32_t timestamp;
	uint64_t frameIndex;
	uint64_t lastFrameIndex;
	uint16_t uniqifier;
	uint32_t creationToken;

	std::chrono::milliseconds lastReceivedAt;

	std::array<std::chrono::milliseconds, MAX_CLIENTS> lastResends{};
	std::array<std::chrono::milliseconds, MAX_CLIENTS> lastSyncs{};

	std::shared_ptr<SyncTreeBase> syncTree;

	std::shared_mutex guidMutex;
	ScriptGuid* guid;
	uint32_t handle;

	bool deleting;

	SyncEntityState();

	SyncEntityState(const SyncEntityState&) = delete;

	virtual ~SyncEntityState();
};
}

namespace fx
{
struct ScriptGuid
{
	enum class Type
	{
		Undefined = 0,
		Entity,
		TempEntity
	};

	Type type;
	union
	{
		struct
		{
			uint32_t handle;
		} entity;

		struct
		{
			uint32_t creationToken;
		} tempEntity;
	};

	void* reference;

	inline void* operator new(size_t s)
	{
		return g_scriptHandlePool->New();
	}

	inline void operator delete(void* ptr)
	{
		g_scriptHandlePool->Delete((fx::ScriptGuid*)ptr);
	}
};

struct EntityCreationState
{
	// TODO: allow resending in case the target client disappears
	uint32_t creationToken;
	uint32_t clientIdx;
	fx::ScriptGuid* scriptGuid;

	EntityCreationState()
		: creationToken(0), clientIdx(0), scriptGuid(0)
	{

	}
};

struct AckPacketWrapper
{
	rl::MessageBuffer& ackPacket;
	std::function<void()> flush;

	inline explicit AckPacketWrapper(rl::MessageBuffer& ackPacket)
		: ackPacket(ackPacket)
	{
		
	}

	template<typename TData>
	inline void Write(int length, TData data)
	{
		ackPacket.Write(length, data);
	}
};

static constexpr const int MaxObjectId = (1 << 16) - 1;

struct GameStateClientData : public sync::ClientSyncDataBase
{
	rl::MessageBuffer ackBuffer{ 16384 };
	std::set<int> objectIds;

	std::mutex selfMutex;

	std::weak_ptr<sync::SyncEntityState> playerEntity;
	std::optional<int> playerId;

	bool syncing;

	glm::mat4x4 viewMatrix;

	eastl::fixed_hash_map<uint64_t, eastl::bitset<MaxObjectId>, 150> idsForGameState;

	eastl::bitset<MaxObjectId> pendingRemovals;

	std::weak_ptr<fx::Client> client;

	std::map<int, int> playersToSlots;
	std::map<int, int> slotsToPlayers;

	std::vector<std::tuple<uint16_t, std::chrono::milliseconds, bool>> relevantEntities;

	GameStateClientData()
		: syncing(false)
	{

	}

	void FlushAcks();

	void MaybeFlushAcks();
};

enum class EntityLockdownMode
{
	Inactive,
	Relaxed,
	Strict
};

class ServerGameState : public fwRefCountable, public fx::IAttached<fx::ServerInstanceBase>
{
private:
	using ThreadPool = tp::ThreadPool;

public:
	ServerGameState();

	void Tick(fx::ServerInstanceBase* instance);

	void UpdateWorldGrid(fx::ServerInstanceBase* instance);

	void UpdateEntities();

	void ParseGameStatePacket(const std::shared_ptr<fx::Client>& client, const std::vector<uint8_t>& packetData);

	virtual void AttachToObject(fx::ServerInstanceBase* instance) override;

	void HandleClientDrop(const std::shared_ptr<fx::Client>& client);

	void SendObjectIds(const std::shared_ptr<fx::Client>& client, int numIds);

	void ReassignEntity(uint32_t entityHandle, const std::shared_ptr<fx::Client>& targetClient);

	void DeleteEntity(const std::shared_ptr<sync::SyncEntityState>& entity);

	inline EntityLockdownMode GetEntityLockdownMode()
	{
		return m_entityLockdownMode;
	}

	inline void SetEntityLockdownMode(EntityLockdownMode mode)
	{
		m_entityLockdownMode = mode;
	}

	inline float GetEntityScopeRadius()
	{
		return m_entityScopeRadius;
	}

	inline void SetEntityScopeRadius(float scopeRadius)
	{
		m_entityScopeRadius = scopeRadius;
	}

	uint32_t MakeScriptHandle(const std::shared_ptr<sync::SyncEntityState>& ptr);

	std::tuple<std::unique_lock<std::mutex>, std::shared_ptr<GameStateClientData>> ExternalGetClientData(const std::shared_ptr<fx::Client>& client);

private:
	void ProcessCloneCreate(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket, AckPacketWrapper& ackPacket);

	void ProcessCloneRemove(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket, AckPacketWrapper& ackPacket);

	void ProcessCloneSync(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket, AckPacketWrapper& ackPacket);

	void ProcessCloneTakeover(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket);

	bool ProcessClonePacket(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket, int parsingType, uint16_t* outObjectId, uint16_t* outUniqifier);

	void OnCloneRemove(const std::shared_ptr<sync::SyncEntityState>& entity, const std::function<void()>& doRemove);

	void RemoveClone(const std::shared_ptr<Client>& client, uint16_t objectId);

	void ParseClonePacket(const std::shared_ptr<fx::Client>& client, net::Buffer& buffer);

	void ParseAckPacket(const std::shared_ptr<fx::Client>& client, net::Buffer& buffer);

	bool ValidateEntity(const std::shared_ptr<sync::SyncEntityState>& entity);

public:
	std::shared_ptr<sync::SyncEntityState> GetEntity(uint8_t playerId, uint16_t objectId);
	std::shared_ptr<sync::SyncEntityState> GetEntity(uint32_t handle);

	fwEvent<std::shared_ptr<sync::SyncEntityState>> OnEntityCreate;

private:
	fx::ServerInstanceBase* m_instance;

	std::unique_ptr<ThreadPool> m_tg;

	// as bitset is not thread-safe
	std::mutex m_objectIdsMutex;

	eastl::bitset<MaxObjectId> m_objectIdsSent;
	eastl::bitset<MaxObjectId> m_objectIdsUsed;
	eastl::bitset<MaxObjectId> m_objectIdsStolen;

	uint64_t m_frameIndex;

	struct WorldGridEntry
	{
		uint8_t sectorX;
		uint8_t sectorY;
		uint8_t slotID;

		WorldGridEntry()
		{
			sectorX = 0;
			sectorY = 0;
			slotID = -1;
		}
	};

	struct WorldGridState
	{
		WorldGridEntry entries[12];
	};

	WorldGridState m_worldGrid[MAX_CLIENTS];

	struct WorldGridOwnerIndexes
	{
		uint8_t slots[256][256];

		inline WorldGridOwnerIndexes()
		{
			memset(slots, 0xFF, sizeof(slots));
		}
	};

	WorldGridOwnerIndexes m_worldGridAccel;

	void SendWorldGrid(void* entry = nullptr, const std::shared_ptr<fx::Client>& client = {});

	bool MoveEntityToCandidate(const std::shared_ptr<sync::SyncEntityState>& entity, const std::shared_ptr<fx::Client>& client);

//private:
public:
	std::vector<LRWeakPtr<sync::SyncEntityState>> m_entitiesById;

	std::list<std::shared_ptr<sync::SyncEntityState>> m_entityList;
	std::shared_mutex m_entityListMutex;

	EntityLockdownMode m_entityLockdownMode;

	float m_entityScopeRadius;
};

std::shared_ptr<sync::SyncTreeBase> MakeSyncTree(sync::NetObjEntityType objectType);
}

DECLARE_INSTANCE_TYPE(fx::ServerGameState);
