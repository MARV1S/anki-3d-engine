// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/RenderingPass.h"
#include "anki/renderer/Renderer.h"
#include "anki/util/Enum.h"

namespace anki {

//==============================================================================
Timestamp RenderingPass::getGlobalTimestamp() const
{
	return m_r->getGlobalTimestamp();
}

//==============================================================================
GrManager& RenderingPass::getGrManager()
{
	return m_r->_getGrManager();
}

//==============================================================================
const GrManager& RenderingPass::getGrManager() const
{
	return m_r->_getGrManager();
}

//==============================================================================
HeapAllocator<U8>& RenderingPass::getAllocator()
{
	return m_r->_getAllocator();
}

//==============================================================================
ResourceManager& RenderingPass::getResourceManager()
{
	return m_r->_getResourceManager();
}

//==============================================================================
Error BlurringRenderingPass::initBlurring(
	Renderer& r, U width, U height, U samples, F32 blurringDistance)
{
	GrManager& gl = getGrManager();
	CommandBufferHandle cmdb;
	ANKI_CHECK(cmdb.create(&gl));

	Array<String, 2> pps;
	String::ScopeDestroyer ppsd0(&pps[0], getAllocator());
	String::ScopeDestroyer ppsd1(&pps[1], getAllocator());

	ANKI_CHECK(
		pps[1].sprintf(getAllocator(),
		"#define HPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(%f)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES %u\n", 
		blurringDistance, height, samples));

	ANKI_CHECK(
		pps[0].sprintf(getAllocator(),
		"#define VPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(%f)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES %u\n",
		blurringDistance, width, samples));

	for(U i = 0; i < 2; i++)
	{
		Direction& dir = m_dirs[i];

		ANKI_CHECK(r.createRenderTarget(width, height, GL_RGB8, 1, dir.m_rt));

		// Set to bilinear because the blurring techniques take advantage of 
		// that
		dir.m_rt.setFilter(cmdb, TextureHandle::Filter::LINEAR);

		// Create FB
		FramebufferHandle::Initializer fbInit;
		fbInit.m_colorAttachmentsCount = 1;
		fbInit.m_colorAttachments[0].m_texture = dir.m_rt;
		ANKI_CHECK(dir.m_fb.create(cmdb, fbInit));

		ANKI_CHECK(dir.m_frag.loadToCache(&getResourceManager(),
			"shaders/VariableSamplingBlurGeneric.frag.glsl", 
			pps[i].toCString(), "r_"));

		ANKI_CHECK(r.createDrawQuadPipeline(
			dir.m_frag->getGlProgram(), dir.m_ppline));
	}

	cmdb.finish();

	return ErrorCode::NONE;
}

//==============================================================================
Error BlurringRenderingPass::runBlurring(
	Renderer& r, CommandBufferHandle& cmdb)
{
	// H pass input
	m_dirs[enumToValue(DirectionEnum::VERTICAL)].m_rt.bind(cmdb, 1); 

	// V pass input
	m_dirs[enumToValue(DirectionEnum::HORIZONTAL)].m_rt.bind(cmdb, 0); 

	for(U32 i = 0; i < m_blurringIterationsCount; i++)
	{
		// hpass
		m_dirs[enumToValue(DirectionEnum::HORIZONTAL)].m_fb.bind(cmdb, true);
		m_dirs[enumToValue(DirectionEnum::HORIZONTAL)].m_ppline.bind(cmdb);
		r.drawQuad(cmdb);

		// vpass
		m_dirs[enumToValue(DirectionEnum::VERTICAL)].m_fb.bind(cmdb, true);
		m_dirs[enumToValue(DirectionEnum::VERTICAL)].m_ppline.bind(cmdb);
		r.drawQuad(cmdb);
	}

	return ErrorCode::NONE;
}

} // end namespace anki

