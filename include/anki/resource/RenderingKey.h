// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_RENDERING_KEY_H
#define ANKI_RESOURCE_RENDERING_KEY_H

#include "anki/util/StdTypes.h"
#include "anki/util/Assert.h"

namespace anki {

/// The AnKi passes
enum class Pass: U8
{
	COLOR, ///< For MS
	DEPTH, ///< For shadows
	COUNT
};

/// A key that consistst of the rendering pass and the level of detail
class RenderingKey
{
public:
	Pass m_pass;
	U8 m_lod;
	Bool8 m_tessellation;

	explicit RenderingKey(Pass pass, U8 lod, Bool tessellation)
	:	m_pass(pass), 
		m_lod(lod), 
		m_tessellation(tessellation)
	{}

	RenderingKey()
	:	RenderingKey(Pass::COLOR, 0, false)
	{}

	RenderingKey(const RenderingKey& b)
	:	RenderingKey(b.m_pass, b.m_lod, b.m_tessellation)
	{}
};

/// The hash function
class RenderingKeyHasher
{
public:
	PtrSize operator()(const RenderingKey& key) const
	{
		return (U8)key.m_pass | (key.m_lod << 8) | (key.m_tessellation << 16);
	}
};

/// The collision evaluation function
class RenderingKeyEqual
{
public:
	Bool operator()(const RenderingKey& a, const RenderingKey& b) const
	{
		return a.m_pass == b.m_pass && a.m_lod == b.m_lod 
			&& a.m_tessellation == b.m_tessellation;
	}
};

} // end namespace anki

#endif
