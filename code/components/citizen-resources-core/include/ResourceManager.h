/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

//#define CPPREST_FORCE_PPLX 1
#define _PPLTASK_ASYNC_LOGGING 0
#include <pplx/pplxtasks.h>

#include <Resource.h>
#include <ResourceMounter.h>

#include <ComponentHolder.h>

#include <msgpack.hpp>

#ifdef COMPILING_CITIZEN_RESOURCES_CORE
#define RESOURCES_CORE_EXPORT DLL_EXPORT
#else
#define RESOURCES_CORE_EXPORT DLL_IMPORT
#endif

namespace fx
{
namespace fx_internal
{
template<typename T>
struct Unserializer
{
	static T Unserialize(const std::string& retval)
	{
		if (retval.empty())
		{
			return T();
		}

		auto unpacked = msgpack::unpack(retval.c_str(), retval.size());
		auto objects = unpacked.get().as<std::vector<msgpack::object>>();

		return objects[0].as<T>();
	}
};

template<>
struct Unserializer<void>
{
	static void Unserialize(const std::string& retval)
	{

	}
};
}

class ResourceManager : public fwRefCountable, public ComponentHolderImpl<ResourceManager>
{
public:
	//
	// Adds a resource to the resource manager from the passed resource URI.
	//
	virtual pplx::task<fwRefContainer<Resource>> AddResource(const std::string& uri) = 0;

	//
	// Gets the mounter that is responsible for handling a particular resource URI.
	//
	virtual fwRefContainer<ResourceMounter> GetMounterForUri(const std::string& uri) = 0;

	//
	// Obtains a reference to the resource with the passed identity string.
	//
	virtual fwRefContainer<Resource> GetResource(const std::string& identifier) = 0;

	//
	// Iterates over all registered resources.
	//
	virtual void ForAllResources(const std::function<void(fwRefContainer<Resource>)>& function) = 0;

	//
	// Stops and unloads all registered resources.
	//
	virtual void ResetResources() = 0;

	//
	// Registers a resource mounter.
	//
	virtual void AddMounter(fwRefContainer<ResourceMounter> mounter) = 0;

	//
	// Remove and destroys a resource.
	//
	virtual void RemoveResource(fwRefContainer<Resource> resource) = 0;

	//
	// For use in resource mounters, creates a resource with the passed identity.
	//
	virtual fwRefContainer<Resource> CreateResource(const std::string& resourceName) = 0;

	//
	// Executes a single tick for the resource manager.
	//
	virtual void Tick() = 0;

	//
	// Makes this resource manager the current resource manager.
	//
	virtual void MakeCurrent() = 0;

private:
	struct pass
	{
		template<typename ...T> pass(T...) {}
	};

public:
	//
	// Calls a formatted function reference.
	//
	template<typename TRet, typename... TArg>
	inline TRet CallReference(const std::string& functionReference, const TArg&... args)
	{
		msgpack::sbuffer buf;
		msgpack::packer<msgpack::sbuffer> packer(buf);

		// pack the argument pack as array
		packer.pack_array(sizeof...(args));
		pass{ (packer.pack(args), 0)... };

		std::string retval = CallReferenceInternal(functionReference, std::string(buf.data(), buf.size()));

		return fx_internal::Unserializer<TRet>::Unserialize(retval);
	}

private:
	virtual std::string CallReferenceInternal(const std::string& functionReference, const std::string& argsSerialized) = 0;

public:
	static void RESOURCES_CORE_EXPORT SetCallRefCallback(const std::function<std::string(const std::string&, const std::string&)>& refCallback);

public:
	fwEvent<> OnTick;

public:
	//
	// An event to add components to a newly-initializing resource manager's instance registry.
	//
	static RESOURCES_CORE_EXPORT fwEvent<ResourceManager*> OnInitializeInstance;

	//
	// Gets the current resource manager.
	//
	static RESOURCES_CORE_EXPORT ResourceManager* GetCurrent();
};

RESOURCES_CORE_EXPORT ResourceManager* CreateResourceManager();
}

DECLARE_INSTANCE_TYPE(fx::ResourceManager);
