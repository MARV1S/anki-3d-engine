// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_CONTACT_POINT_H
#define ANKI_COLLISION_CONTACT_POINT_H

#include "anki/Math.h"

namespace anki {

/// @addtogroup collision
/// @{

/// Collision test contact point
class ContactPoint
{
public:
	Vec4 m_position;
	Vec4 m_normal;
	F32 m_depth;
};

/// @}

} // end namespace anki

#endif

