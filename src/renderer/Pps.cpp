// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Pps.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Hdr.h"
#include "anki/renderer/Ssao.h"
#include "anki/util/Logger.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
Pps::Pps(Renderer* r)
:	RenderingPass(r), 
	m_hdr(r), 
	m_ssao(r), 
	m_bl(r), 
	m_lf(r),
	m_sslr(r)
{}

//==============================================================================
Pps::~Pps()
{}

//==============================================================================
Error Pps::initInternal(const ConfigSet& config)
{
	m_enabled = config.get("pps.enabled");
	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	ANKI_ASSERT("Initializing PPS");

	ANKI_CHECK(m_ssao.init(config));
	ANKI_CHECK(m_hdr.init(config));
	ANKI_CHECK(m_lf.init(config));
	ANKI_CHECK(m_sslr.init(config));

	// FBO
	CommandBufferHandle cmdBuff;
	ANKI_CHECK(cmdBuff.create(&getGrManager()));

	ANKI_CHECK(
		m_r->createRenderTarget(
		m_r->getWidth(), m_r->getHeight(), GL_RGB8, 1, m_rt));

	FramebufferHandle::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	ANKI_CHECK(m_fb.create(cmdBuff, fbInit));

	// SProg
	String pps;
	String::ScopeDestroyer ppsd(&pps, getAllocator());

	ANKI_CHECK(pps.sprintf(getAllocator(),
		"#define SSAO_ENABLED %u\n"
		"#define HDR_ENABLED %u\n"
		"#define SHARPEN_ENABLED %u\n"
		"#define GAMMA_CORRECTION_ENABLED %u\n"
		"#define FBO_WIDTH %u\n"
		"#define FBO_HEIGHT %u\n",
		m_ssao.getEnabled(), 
		m_hdr.getEnabled(), 
		static_cast<U>(config.get("pps.sharpen")),
		static_cast<U>(config.get("pps.gammaCorrection")),
		m_r->getWidth(),
		m_r->getHeight()));

	ANKI_CHECK(m_frag.loadToCache(&getResourceManager(),
		"shaders/Pps.frag.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(m_r->createDrawQuadPipeline(m_frag->getGlProgram(), m_ppline));

	cmdBuff.finish();

	return ErrorCode::NONE;
}

//==============================================================================
Error Pps::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to init PPS");
	}

	return err;
}

//==============================================================================
Error Pps::run(CommandBufferHandle& cmdBuff)
{
	ANKI_ASSERT(m_enabled);
	Error err = ErrorCode::NONE;

	// First SSAO because it depends on MS where HDR depends on IS
	if(m_ssao.getEnabled())
	{
		err = m_ssao.run(cmdBuff);
		if(err) return err;
	}

	// Then SSLR because HDR depends on it
	if(m_sslr.getEnabled())
	{
		err = m_sslr.run(cmdBuff);
		if(err) return err;
	}

	if(m_hdr.getEnabled())
	{
		err = m_hdr.run(cmdBuff);
		if(err) return err;
	}

	if(m_lf.getEnabled())
	{
		err = m_lf.run(cmdBuff);
		if(err) return err;
	}

	/*Bool drawToDefaultFbo = 
		!m_r->getDbg().getEnabled() 
		&& !m_r->getIsOffscreen()
		&& m_r->getRenderingQuality() == 1.0;*/
	Bool drawToDefaultFbo = false;

	if(drawToDefaultFbo)
	{
		m_r->getDefaultFramebuffer().bind(cmdBuff, true);
		cmdBuff.setViewport(0, 0, 
			m_r->getDefaultFramebufferWidth(), 
			m_r->getDefaultFramebufferHeight());
	}
	else
	{
		m_fb.bind(cmdBuff, true);
		cmdBuff.setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	}

	m_ppline.bind(cmdBuff);

	m_r->getIs()._getRt().bind(cmdBuff, 0);

	if(m_ssao.getEnabled())
	{
		m_ssao._getRt().bind(cmdBuff, 1);
	}

	if(m_lf.getEnabled())
	{
		m_lf._getRt().bind(cmdBuff, 2);
	}
	else if(m_hdr.getEnabled())
	{
		m_hdr._getRt().bind(cmdBuff, 2);
	}

	m_r->drawQuad(cmdBuff);

	return err;
}

} // end namespace anki
