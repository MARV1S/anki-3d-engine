// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_VISIBILITY_TEST_RESULTS_H
#define ANKI_SCENE_VISIBILITY_TEST_RESULTS_H

#include "anki/scene/Common.h"
#include "anki/collision/Forward.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/RenderComponent.h"
#include "anki/util/NonCopyable.h"

namespace anki {

// Forward
class Renderer;

/// @addtogroup Scene
/// @{

/// Visibility test type
enum VisibilityTest
{
	VT_RENDERABLES = 1 << 0,
	VT_ONLY_SHADOW_CASTERS = 1 << 1,
	VT_LIGHTS = 1 << 2
};

/// Visible by
enum VisibleBy
{
	VB_NONE = 0,
	VB_CAMERA = 1 << 0,
	VB_LIGHT = 1 << 1
};

/// Visible node pointer with some more info
/// @note Keep this structore as small as possible
class VisibleNode
{
public:
	SceneNode* m_node = nullptr;
	/// An array of the visible spatials
	U8* m_spatialIndices = nullptr;
	U8 m_spatialsCount = 0;

	VisibleNode()
	{}

	VisibleNode(const VisibleNode& other)
	{
		*this = other;
	}

	VisibleNode& operator=(const VisibleNode& other)
	{
		m_node = other.m_node;
		m_spatialIndices = other.m_spatialIndices;
		m_spatialsCount = other.m_spatialsCount;
		return *this;
	}

	U8 getSpatialIndex(U i)
	{
		ANKI_ASSERT(m_spatialsCount != 0 && i < m_spatialsCount);
		return m_spatialIndices[i];
	}
};

/// Its actually a container for visible entities. It should be per frame
class VisibilityTestResults
{
public:
	using Container = DArray<VisibleNode>;

	~VisibilityTestResults()
	{
		ANKI_ASSERT(0 && "It's supposed to be deallocated on frame start");
	}

	ANKI_USE_RESULT Error create(
		SceneFrameAllocator<U8> alloc,
		U32 renderablesReservedSize,
		U32 lightsReservedSize,
		U32 lensFlaresReservedSize);

	void prepareMerge()
	{
		ANKI_ASSERT(m_renderablesCount == 0 
			&& m_lightsCount == 0 
			&& m_flaresCount == 0);
		m_renderablesCount = m_renderables.getSize();
		m_lightsCount = m_lights.getSize();
		m_flaresCount = m_flares.getSize();
	}

	VisibleNode* getRenderablesBegin()
	{
		return (m_renderablesCount) ? &m_renderables[0] : nullptr;
	}

	VisibleNode* getRenderablesEnd()
	{
		return (m_renderablesCount) 
			? (&m_renderables[0] + m_renderablesCount) : nullptr;
	}

	VisibleNode* getLightsBegin()
	{
		return (m_lightsCount) ? &m_lights[0] : nullptr;
	}

	VisibleNode* getLightsEnd()
	{
		return (m_lightsCount) ? (&m_lights[0] + m_lightsCount) : nullptr;
	}

	VisibleNode* getLensFlaresBegin()
	{
		return (m_flaresCount) ? &m_flares[0] : nullptr;
	}

	VisibleNode* getLensFlaresEnd()
	{
		return (m_flaresCount) ? (&m_flares[0] + m_flaresCount) : nullptr;
	}

	U32 getRenderablesCount() const
	{
		return m_renderablesCount;
	}

	U32 getLightsCount() const
	{
		return m_lightsCount;
	}

	U32 getLensFlaresCount() const
	{
		return m_flaresCount;
	}

	ANKI_USE_RESULT Error moveBackRenderable(
		SceneFrameAllocator<U8> alloc, VisibleNode& x)
	{
		return moveBack(alloc, m_renderables, m_renderablesCount, x);
	}

	ANKI_USE_RESULT Error moveBackLight(
		SceneFrameAllocator<U8> alloc, VisibleNode& x)
	{
		return moveBack(alloc, m_lights, m_lightsCount, x);
	}

	ANKI_USE_RESULT Error moveBackLensFlare(
		SceneFrameAllocator<U8> alloc, VisibleNode& x)
	{
		return moveBack(alloc, m_flares, m_flaresCount, x);
	}

private:
	Container m_renderables;
	Container m_lights;
	Container m_flares;
	U32 m_renderablesCount = 0;
	U32 m_lightsCount = 0;
	U32 m_flaresCount = 0;

	ANKI_USE_RESULT Error moveBack(SceneFrameAllocator<U8> alloc, 
		Container& c, U32& count, VisibleNode& x);
};

/// Sort spatial scene nodes on distance
class DistanceSortFunctor
{
public:
	Vec4 m_origin;

	Bool operator()(const VisibleNode& a, const VisibleNode& b)
	{
		ANKI_ASSERT(a.m_node && b.m_node);

		F32 dist0 = m_origin.getDistanceSquared(
			a.m_node->getComponent<SpatialComponent>().getSpatialOrigin());
		F32 dist1 = m_origin.getDistanceSquared(
			b.m_node->getComponent<SpatialComponent>().getSpatialOrigin());

		return dist0 < dist1;
	}
};

/// Sort renderable scene nodes on material
class MaterialSortFunctor
{
public:
	Bool operator()(const VisibleNode& a, const VisibleNode& b)
	{
		ANKI_ASSERT(a.m_node && b.m_node);

		return a.m_node->getComponent<RenderComponent>().getMaterial()
			< b.m_node->getComponent<RenderComponent>().getMaterial();
	}
};

/// Thread job to short scene nodes by distance
class DistanceSortJob: public Threadpool::Task					
{
public:
	U32 m_nodesCount;
	VisibilityTestResults::Container::Iterator m_nodes;
	Vec4 m_origin;

	Error operator()(U32 /*threadId*/, PtrSize /*threadsCount*/)
	{
		DistanceSortFunctor comp;
		comp.m_origin = m_origin;
		std::sort(m_nodes, m_nodes + m_nodesCount, comp);
		return ErrorCode::NONE;
	}
};

/// Thread job to short renderable scene nodes by material
class MaterialSortJob: public Threadpool::Task
{
public:
	U32 m_nodesCount;
	VisibilityTestResults::Container::Iterator m_nodes;

	Error operator()(U32 /*threadId*/, PtrSize /*threadsCount*/)
	{
		std::sort(m_nodes, m_nodes + m_nodesCount, MaterialSortFunctor());
		return ErrorCode::NONE;
	}
};

/// Do visibility tests bypassing portals 
ANKI_USE_RESULT Error doVisibilityTests(
	SceneNode& frustumable, 
	SceneGraph& scene, 
	Renderer& renderer);

/// @}

} // end namespace anki

#endif
