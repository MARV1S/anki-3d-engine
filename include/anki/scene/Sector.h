// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_SECTOR_H
#define ANKI_SCENE_SECTOR_H

#include "anki/scene/Common.h"
#include "anki/scene/Visibility.h"
#include "anki/Collision.h"

namespace anki {

// Forward
class SceneNode;
class SceneGraph;
class Sector;
class SectorGroup;
class Renderer;

/// @addtogroup Scene
/// @{

/// 2 way Portal
struct Portal
{
	Array<Sector*, 2> sectors;
	Obb shape;
	Bool8 open;

	Portal();
};

/// A sector. It consists of an octree and some portals
class Sector
{
	friend class SectorGroup;

public:
	/// Used to reserve some space on the portals vector to save memory
	static const U AVERAGE_PORTALS_PER_SECTOR = 3;

	/// Default constructor
	Sector(SectorGroup* group, const Aabb& box);

	const Aabb& getAabb() const
	{
		return aabb;
	}

	const SectorGroup& getSectorGroup() const
	{
		return *group;
	}
	SectorGroup& getSectorGroup()
	{
		return *group;
	}

	U8 getVisibleByMask() const
	{
		return visibleBy;
	}

	/// Called when a node was moved or a change in shape happened
	Bool placeSceneNode(SceneNode* sp);

private:
	SectorGroup* group; ///< Know your father
	SceneVector<Portal*> portals;
	U8 visibleBy;
	Aabb aabb;

	/// Sector does not take ownership of the portal
	void addNewPortal(Portal* portal);

	/// Remove a Portal from the portals container
	void removePortal(Portal* portal);
};

/// Sector group. This is supposed to represent the whole scene
class SectorGroup
{
public:
	/// Default constructor
	SectorGroup(SceneGraph* scene);

	/// Destructor
	~SectorGroup();

	/// @name Accessors
	/// @{
	const SceneGraph& getSceneGraph() const
	{
		return *scene;
	}
	SceneGraph& getSceneGraph()
	{
		return *scene;
	}

	const SceneVector<Portal*>& getPortals() const
	{
		return portals;
	}

	const SceneVector<Sector*>& getSectors() const
	{
		return sectors;
	}
	/// @}

	/// Called when a node was moved or a change in shape happened. The node 
	/// must be Spatial
	void placeSceneNode(SceneNode* sp);

	/// XXX
	void doVisibilityTests(SceneNode& fr, VisibilityTest test, Renderer* r);

	/// The owner of the pointer is the sector group
	Sector* createNewSector(const Aabb& aabb);

	/// The owner of the pointer is the sector group
	Portal* createNewPortal(Sector* a, Sector* b, const Obb& collisionShape);

private:
	SceneGraph* scene; ///< Keep it here to access various allocators
	SceneVector<Sector*> sectors;
	SceneVector<Portal*> portals;

	void doVisibilityTestsInternal(SceneNode& fr, VisibilityTest test,
		Renderer* r, VisibleBy visibleBy);
};
/// @}

} // end namespace anki

#endif
