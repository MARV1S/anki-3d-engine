// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Visibility.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/scene/LensFlareComponent.h"
#include "anki/scene/Light.h"
#include "anki/renderer/Renderer.h"
#include "anki/util/Logger.h"

namespace anki {

//==============================================================================
// VisibilityTestTask                                                          =
//==============================================================================

//==============================================================================
class VisibilityTestTask: public Threadpool::Task
{
public:
	U m_nodesCount = 0;
	SceneGraph* m_scene = nullptr;
	SceneNode* m_frustumableSn = nullptr;
	SceneFrameAllocator<U8> m_alloc;

	VisibilityTestResults* m_cameraVisible; // out

	/// Test a frustum component
	ANKI_USE_RESULT Error test(SceneNode& testedNode, Bool testingLight, 
		U32 threadId, PtrSize threadsCount);

	/// Do the tests
	Error operator()(U32 threadId, PtrSize threadsCount)
	{
		return test(*m_frustumableSn, false, threadId, threadsCount);
	}
};

//==============================================================================
Error VisibilityTestTask::test(SceneNode& testedNode, Bool testingLight, 
	U32 threadId, PtrSize threadsCount)
{
	ANKI_ASSERT(testingLight == 
		(testedNode.tryGetComponent<LightComponent>() != nullptr));

	Error err = ErrorCode::NONE;

	FrustumComponent& testedFr = 
		testedNode.getComponent<FrustumComponent>();

	// Allocate visible
	VisibilityTestResults* visible = 
		m_alloc.newInstance<VisibilityTestResults>();
	if(visible == nullptr) return ErrorCode::OUT_OF_MEMORY;

	// Init visible
	FrustumComponent::VisibilityStats stats = testedFr.getLastVisibilityStats();
	
	if(!testingLight)
	{
		// For camera be conservative
		stats.m_renderablesCount /= threadsCount;
		stats.m_lightsCount /= threadsCount;
	}

	err = visible->create(
		m_alloc, stats.m_renderablesCount, stats.m_lightsCount, 4);
	if(err)	return err;

	// Chose the test range and a few other things
	PtrSize start, end;
	if(!testingLight)
	{
		choseStartEnd(threadId, threadsCount, m_nodesCount, start, end);
		m_cameraVisible = visible;
	}
	else
	{
		// Is light
		start = 0;
		end = m_nodesCount;
		testedFr.setVisibilityTestResults(visible);
	}

	// Iterate range of nodes
	err = m_scene->iterateSceneNodes(start, end, [&](SceneNode& node) -> Error
	{
		Error err = ErrorCode::NONE;

		FrustumComponent* fr = node.tryGetComponent<FrustumComponent>();
		
		// Skip if it is the same
		if(ANKI_UNLIKELY(&testedFr == fr))
		{
			return ErrorCode::NONE;
		}

		VisibleNode visibleNode;
		visibleNode.m_node = &node;

		// Test all spatial components of that node
		struct SpatialTemp
		{
			SpatialComponent* m_sp;
			U8 m_idx;
		};
		Array<SpatialTemp, ANKI_GL_MAX_SUB_DRAWCALLS> sps;

		U spIdx = 0;
		U count = 0;
		err = node.iterateComponentsOfType<SpatialComponent>(
			[&](SpatialComponent& sp)
		{
			if(testedFr.insideFrustum(sp))
			{
				// Inside
				ANKI_ASSERT(spIdx < MAX_U8);
				sps[count++] = SpatialTemp{&sp, static_cast<U8>(spIdx)};

				sp.enableBits(testingLight 
					? SpatialComponent::Flag::VISIBLE_LIGHT 
					: SpatialComponent::Flag::VISIBLE_CAMERA);
			}

			++spIdx;

			return ErrorCode::NONE;
		});

		if(count == 0)
		{
			return err;
		}

		// Sort spatials
		Vec4 origin = testedFr.getFrustumOrigin();
		std::sort(sps.begin(), sps.begin() + count, 
			[origin](const SpatialTemp& a, const SpatialTemp& b) -> Bool
		{
			Vec4 spa = a.m_sp->getSpatialOrigin();
			Vec4 spb = b.m_sp->getSpatialOrigin();

			F32 dist0 = origin.getDistanceSquared(spa);
			F32 dist1 = origin.getDistanceSquared(spb);

			return dist0 < dist1;
		});

		// Update the visibleNode
		ANKI_ASSERT(count < MAX_U8);
		visibleNode.m_spatialsCount = count;
		visibleNode.m_spatialIndices = m_alloc.newArray<U8>(count);
		if(visibleNode.m_spatialIndices == nullptr)
		{
			return ErrorCode::OUT_OF_MEMORY;
		}

		for(U i = 0; i < count; i++)
		{
			visibleNode.m_spatialIndices[i] = sps[i].m_idx;
		}

		// Do something with the result
		RenderComponent* r = node.tryGetComponent<RenderComponent>();
		if(testingLight)
		{
			if(r && r->getCastsShadow())
			{
				err = visible->moveBackRenderable(m_alloc, visibleNode);
			}
		}
		else
		{
			if(r)
			{
				err = visible->moveBackRenderable(m_alloc, visibleNode);
			}

			LightComponent* l = node.tryGetComponent<LightComponent>();
			if(!err && l)
			{
				err = visible->moveBackLight(m_alloc, visibleNode);

				if(!err && l->getShadowEnabled() && fr)
				{
					err = test(node, true, 0, 0);
				}
			}

			LensFlareComponent* lf = node.tryGetComponent<LensFlareComponent>();
			if(!err && lf)
			{
				err = visible->moveBackLensFlare(m_alloc, visibleNode);
				ANKI_ASSERT(visibleNode.m_node);
			}
		}

		return err;
	}); // end for

	return err;
}

//==============================================================================
// VisibilityTestResults                                                       =
//==============================================================================

//==============================================================================
Error VisibilityTestResults::create(
	SceneFrameAllocator<U8> alloc,
	U32 renderablesReservedSize,
	U32 lightsReservedSize,
	U32 lensFlaresReservedSize)
{
	Error err = m_renderables.create(alloc, renderablesReservedSize);
	
	if(!err)
	{
		err = m_lights.create(alloc, lightsReservedSize);
	}

	if(!err)
	{
		err = m_flares.create(alloc, lensFlaresReservedSize);
	}

	return err;
}

//==============================================================================
Error VisibilityTestResults::moveBack(
	SceneFrameAllocator<U8> alloc, Container& c, U32& count, VisibleNode& x)
{
	Error err = ErrorCode::NONE;

	if(count + 1 > c.getSize())
	{
		// Need to grow
		U newSize = (c.getSize() != 0) ? c.getSize() * 2 : 2;
		err = c.resize(alloc, newSize);
	}

	if(!err)
	{
		c[count++] = x;
	}

	return err;
}

//==============================================================================
// doVisibilityTests                                                           =
//==============================================================================

//==============================================================================
Error doVisibilityTests(SceneNode& fsn, SceneGraph& scene, Renderer& r)
{
	FrustumComponent& fr = fsn.getComponent<FrustumComponent>();

	//
	// Do the tests in parallel
	//
	Threadpool& threadPool = scene._getThreadpool();
	VisibilityTestTask jobs[Threadpool::MAX_THREADS];
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].m_nodesCount = scene.getSceneNodesCount();
		jobs[i].m_scene = &scene;
		jobs[i].m_frustumableSn = &fsn;
		jobs[i].m_alloc = scene.getFrameAllocator();

		threadPool.assignNewTask(i, &jobs[i]);
	}

	Error err = threadPool.waitForAllThreadsToFinish();
	if(err)	return err;

	//
	// Combine results
	//

	// Count the visible scene nodes to optimize the allocation of the 
	// final result
	U32 renderablesSize = 0;
	U32 lightsSize = 0;
	U32 lensFlaresSize = 0;
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		renderablesSize += jobs[i].m_cameraVisible->getRenderablesCount();
		lightsSize += jobs[i].m_cameraVisible->getLightsCount();
		lensFlaresSize += jobs[i].m_cameraVisible->getLensFlaresCount();
	}

	// Allocate
	VisibilityTestResults* visible = 
		scene.getFrameAllocator().newInstance<VisibilityTestResults>();
	if(visible == nullptr)	return ErrorCode::OUT_OF_MEMORY;

	err = visible->create(
		scene.getFrameAllocator(), 
		renderablesSize, 
		lightsSize,
		lensFlaresSize);
	if(err)	return err;

	visible->prepareMerge();

	if(renderablesSize == 0)
	{
		ANKI_LOGW("No visible renderables");
	}

	// Append thread results
	VisibleNode* renderables = visible->getRenderablesBegin();
	VisibleNode* lights = visible->getLightsBegin();
	VisibleNode* lensFlares = visible->getLensFlaresBegin();
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		VisibilityTestResults& from = *jobs[i].m_cameraVisible;

		U rCount = from.getRenderablesCount();
		U lCount = from.getLightsCount();
		U lfCount = from.getLensFlaresCount();

		if(rCount > 0)
		{
			memcpy(renderables,
				from.getRenderablesBegin(),
				sizeof(VisibleNode) * rCount);

			renderables += rCount;
		}

		if(lCount > 0)
		{
			memcpy(lights,
				from.getLightsBegin(),
				sizeof(VisibleNode) * lCount);

			lights += lCount;
		}

		if(lfCount > 0)
		{
			memcpy(lensFlares,
				from.getLensFlaresBegin(),
				sizeof(VisibleNode) * lfCount);

			lensFlares += lfCount;
		}
	}

	// Set the frustumable
	fr.setVisibilityTestResults(visible);

	//
	// Sort
	//

	// The lights
	DistanceSortJob dsjob;
	dsjob.m_nodes = visible->getLightsBegin();
	dsjob.m_nodesCount = visible->getLightsCount();
	dsjob.m_origin = fr.getFrustumOrigin();
	threadPool.assignNewTask(0, &dsjob);

	// The rest of the jobs are dummy
	for(U i = 1; i < threadPool.getThreadsCount(); i++)
	{
		threadPool.assignNewTask(i, nullptr);
	}

	// Sort the renderables in the main thread
	DistanceSortFunctor dsfunc;
	dsfunc.m_origin = fr.getFrustumOrigin();
	std::sort(
		visible->getRenderablesBegin(), visible->getRenderablesEnd(), dsfunc);

	err = threadPool.waitForAllThreadsToFinish();

	return err;
}

} // end namespace anki
