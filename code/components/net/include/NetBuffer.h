/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#ifndef COMPILING_ADHESIVE
#include <boost/type_index.hpp>
#endif

// for now!
#include <Error.h>

class 
#ifdef COMPILING_NET
	__declspec(dllexport)
#endif
	NetBuffer
{
private:
	char* m_bytes;
	size_t m_curOff;
	size_t m_length;

	bool m_end;
	bool m_bytesManaged;

public:
	NetBuffer(const char* bytes, size_t length);
	NetBuffer(size_t length);

	virtual ~NetBuffer();

	bool End();

	bool Read(void* buffer, size_t length);
	void Write(const void* buffer, size_t length);

	template<typename T>
	T Read()
	{
		T tempValue;
		if (!Read(&tempValue, sizeof(T)))
		{
#ifndef COMPILING_ADHESIVE
			GlobalError("NetBuffer::Read<%s>() failed to read %d bytes.", boost::typeindex::type_id<T>().pretty_name().c_str(), sizeof(T));
#endif
		}

		return tempValue;
	}

	template<typename T>
	void Write(T value)
	{
		Write(&value, sizeof(T));
	}

	inline const char* GetBuffer() { return m_bytes; }
	inline size_t GetLength() { return m_length; }
	inline size_t GetCurLength() { return m_curOff; }
};
