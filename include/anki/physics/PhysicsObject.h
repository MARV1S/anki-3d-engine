// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PHYSICS_OBJECT_H
#define ANKI_PHYSICS_PHYSICS_OBJECT_H

#include "anki/physics/Common.h"

namespace anki {

/// @addtogroup physics
/// @{

/// Base of all physics objects.
class PhysicsObject
{
public:
	/// Type of the physics object.
	enum class Type: U8
	{
		COLLISION_SHAPE,
		BODY,
		JOINT,
		PLAYER_CONTROLLER,
		COUNT
	};

	PhysicsObject(Type type, PhysicsWorld* world)
	:	m_world(world),
		m_type(type)
	{
		ANKI_ASSERT(m_world);
	}

	virtual ~PhysicsObject()
	{
		ANKI_ASSERT(m_markedForDeletion == true);
	}

	Type getType() const
	{
		return m_type;
	}

	void setMarkedForDeletion();

	Bool getMarkedForDeletion() const
	{
		return m_markedForDeletion;
	}

protected:
	PhysicsWorld* m_world = nullptr;

private:
	Type m_type;
	Bool8 m_markedForDeletion = false;
};
/// @}

} // end namespace anki

#endif
