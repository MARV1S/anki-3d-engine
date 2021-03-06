// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PHYSICS_DRAWER_H
#define ANKI_PHYSICS_PHYSICS_DRAWER_H

#include "anki/physics/Common.h"
#include "anki/util/Bitset.h"

// Forward
class NewtonBody;

namespace anki {

/// @addtogroup physics
/// @{

/// Physics debug drawer interface.
class PhysicsDrawer
{
public:
	/// Draw a line.
	virtual void drawLines(
		const Vec3* lines,
		const U32 linesCount,
		const Vec4& color) = 0;

	void drawWorld(const PhysicsWorld& world);

	void setDrawAabbs(Bool draw)
	{
		m_drawAabbs = draw;
	}

	Bool getDrawAabbs() const
	{
		return m_drawAabbs;
	}

	void setDrawCollision(Bool draw)
	{
		m_drawCollision = draw;
	}

	Bool getDrawCollision() const
	{
		return m_drawCollision;
	}

private:
	Bool8 m_drawAabbs = true;
	Bool8 m_drawCollision = true;

	void drawAabb(const NewtonBody* body);
	void drawCollision(const NewtonBody* body);

	static void drawGeometryCallback(void* userData, 
		int vertexCount, const dFloat* const faceVertec, int id);
};
/// @}

} // end namespace anki

#endif

