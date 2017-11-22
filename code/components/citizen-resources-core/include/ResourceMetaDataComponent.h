/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include <om/core.h>

#include <boost/optional.hpp>
#include <optional>
#include "IteratorView.h"

#ifdef COMPILING_CITIZEN_RESOURCES_CORE
#define RESOURCES_CORE_EXPORT DLL_EXPORT
#else
#define RESOURCES_CORE_EXPORT DLL_IMPORT
#endif

namespace fx
{
class Resource;

class ResourceMetaDataComponent;

class ResourceMetaDataLoader : public fwRefCountable
{
public:
	virtual boost::optional<std::string> LoadMetaData(ResourceMetaDataComponent* component, const std::string& resourcePath) = 0;
};

class RESOURCES_CORE_EXPORT ResourceMetaDataComponent : public fwRefCountable
{
private:
	Resource* m_resource;

	std::multimap<std::string, std::string> m_metaDataEntries;

	fwRefContainer<ResourceMetaDataLoader> m_metaDataLoader;

public:
	ResourceMetaDataComponent(Resource* resourceRef);

	boost::optional<std::string> LoadMetaData(const std::string& resourcePath);

	std::optional<bool> IsManifestVersionBetween(const guid_t& lowerBound, const guid_t& upperBound);

	inline Resource* GetResource()
	{
		return m_resource;
	}

	inline void SetMetaDataLoader(fwRefContainer<ResourceMetaDataLoader> loader)
	{
		m_metaDataLoader = loader;
	}

	inline auto GetEntries(const std::string& key)
	{
		return GetIteratorView(m_metaDataEntries.equal_range(key));
	}

	inline void AddMetaData(const std::string& key, const std::string& value)
	{
		m_metaDataEntries.insert({ key, value });
	}
};
}

DECLARE_INSTANCE_TYPE(fx::ResourceMetaDataComponent);
