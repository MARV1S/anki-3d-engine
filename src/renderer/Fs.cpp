// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Fs.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"

namespace anki {

//==============================================================================
Fs::~Fs()
{}

//==============================================================================
Error Fs::init(const ConfigSet&)
{
	CommandBufferHandle cmdb;
	ANKI_CHECK(cmdb.create(&getGrManager()));

	FramebufferHandle::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_r->getIs()._getRt();
	fbInit.m_depthStencilAttachment.m_texture = m_r->getMs()._getDepthRt();
	ANKI_CHECK(m_fb.create(cmdb,fbInit));

	cmdb.flush();

	return ErrorCode::NONE;
}

//==============================================================================
Error Fs::run(CommandBufferHandle& cmdb)
{
	Error err = ErrorCode::NONE;

	m_fb.bind(cmdb, false);

	cmdb.enableDepthTest(true);
	cmdb.setDepthWriteMask(false);
	cmdb.enableBlend(true);

	RenderableDrawer& drawer = m_r->getSceneDrawer();
	drawer.prepareDraw(RenderingStage::BLEND, Pass::COLOR, cmdb);

	Camera& cam = m_r->getSceneGraph().getActiveCamera();
	FrustumComponent& camFr = cam.getComponent<FrustumComponent>();

	auto it = camFr.getVisibilityTestResults().getRenderablesBegin();
	auto end = camFr.getVisibilityTestResults().getRenderablesEnd();
	for(; !err && it != end; ++it)
	{
		err = drawer.render(cam, *it);
	}

	if(!err)
	{
		drawer.finishDraw();

		cmdb.enableDepthTest(false);
		cmdb.setDepthWriteMask(true);
		cmdb.enableBlend(false);
	}

	return err;
}

} // end namespace anki
