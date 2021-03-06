// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_RESOURCE_POINTER_H
#define ANKI_RESOURCE_RESOURCE_POINTER_H

#include "anki/resource/Common.h"
#include "anki/util/Assert.h"
#include "anki/util/Atomic.h"
#include <cstring>

namespace anki {

// Forward
class ResourceManager;

/// @addtogroup resource
/// @{

/// Special smart pointer that points to resource classes.
///
/// It looks like auto_ptr but the main difference is that when its out of scope
/// it tries to unload the resource.
template<typename Type, typename TResourceManager>
class ResourcePointer
{
public:
	using Value = Type; ///< Resource type

	/// Default constructor
	ResourcePointer()
	{}

	/// Copy constructor
	ResourcePointer(const ResourcePointer& b)
	{
		copy(b);
	}

	~ResourcePointer()
	{
		reset();
	}

	const Value& operator*() const
	{
		ANKI_ASSERT(m_cb != nullptr);
		return m_cb->m_resource;
	}

	Value& operator*()
	{
		ANKI_ASSERT(m_cb != nullptr);
		return m_cb->m_resource;
	}

	const Value* operator->() const
	{
		ANKI_ASSERT(m_cb != nullptr);
		return &m_cb->m_resource;
	}

	Value* operator->()
	{
		ANKI_ASSERT(m_cb != nullptr);
		return &m_cb->m_resource;
	}

	const Value* get() const
	{
		ANKI_ASSERT(m_cb != nullptr);
		return &m_cb->m_resource;
	}

	Value* get()
	{
		ANKI_ASSERT(m_cb != nullptr);
		return &m_cb->m_resource;
	}

	CString getResourceName() const
	{
		ANKI_ASSERT(m_cb != nullptr);
		return &m_cb->m_uuid[0];
	}

	U32 getReferenceCount() const
	{
		ANKI_ASSERT(m_cb != nullptr);
		return m_cb->m_refcount.load();
	}
	
	/// Copy
	ResourcePointer& operator=(const ResourcePointer& b)
	{
		copy(b);
		return *this;
	}

	Bool operator==(const ResourcePointer& b) const
	{
		ANKI_ASSERT(m_cb != nullptr);
		ANKI_ASSERT(b.m_cb != nullptr);
		return std::strcmp(&m_cb->m_uuid[0], &b.m_cb->m_uuid[0]) == 0; 
	}

	/// Load the resource using the resource manager
	ANKI_USE_RESULT Error load(
		const CString& filename, TResourceManager* resources);

	template<typename... TArgs>
	ANKI_USE_RESULT Error loadToCache(
		TResourceManager* resources, TArgs&&... args);

	Bool isLoaded() const
	{
		return m_cb != nullptr;
	}

private:
	/// Control block
	class ControlBlock
	{
	public:
		ControlBlock(ResourceAllocator<U8>& alloc)
		:	m_resource(alloc)
		{}

		Type m_resource;
		Atomic<U32> m_refcount = {1};
		TResourceManager* m_resources = nullptr;
		char m_uuid[1]; ///< This is part of the UUID
	};

	ControlBlock* m_cb = nullptr;

	void reset();

	/// If this empty and @a b empty then unload. If @a b has something then
	/// unload this and load exactly what @b has. In everything else do nothing
	void copy(const ResourcePointer& b);
};

/// @}

} // end namespace anki

#include "anki/resource/ResourcePointer.inl.h"

#endif
