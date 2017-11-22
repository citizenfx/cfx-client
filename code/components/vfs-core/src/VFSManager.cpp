/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include <VFSManager.h>

namespace vfs
{
fwRefContainer<Stream> Manager::OpenRead(const std::string& path)
{
	auto device = GetDevice(path);

	if (device.GetRef())
	{
		auto handle = device->Open(path, true);

		if (handle != INVALID_DEVICE_HANDLE)
		{
			return new Stream(device, handle);
		}
	}

	return nullptr;
}

fwRefContainer<Device> Manager::GetNativeDevice(void* nativeDevice)
{
	return nullptr;
}

fwRefContainer<Stream> OpenRead(const std::string& path)
{
	return Instance<Manager>::Get()->OpenRead(path);
}

fwRefContainer<Device> GetDevice(const std::string& path)
{
	return Instance<Manager>::Get()->GetDevice(path);
}

fwRefContainer<Device> GetNativeDevice(void* nativeDevice)
{
	return Instance<Manager>::Get()->GetNativeDevice(nativeDevice);
}

void Mount(fwRefContainer<Device> device, const std::string& path)
{
	return Instance<Manager>::Get()->Mount(device, path);
}

void Unmount(const std::string& path)
{
	return Instance<Manager>::Get()->Unmount(path);
}
}