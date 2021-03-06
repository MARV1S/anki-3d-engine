// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Bl.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ProgramResource.h"

namespace anki {

#if 0
//==============================================================================
void Bl::init(const ConfigSet& initializer)
{
	enabled = initializer.pps.bl.enabled;
	blurringIterationsNum = initializer.pps.bl.blurringIterationsNum;
	sideBlurFactor = initializer.pps.bl.sideBlurFactor;

	if(!enabled)
	{
		return;
	}

	// Horizontal
	//
	try
	{
		Renderer::createFai(r->getWidth(), r->getHeight(), GL_RGB, GL_RGB,
			GL_FLOAT, blurFai);

		hBlurFbo.create();
		hBlurFbo.setColorAttachments({&blurFai});
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create horizontal blur "
			"post-processing stage FBO") << e;
	}

	hBlurSProg.load(ProgramResource::createSrcCodeToCache(
		"shaders/PpsBlurGeneric.glsl", "#define HPASS\n").c_str());

	// Vertical
	//
	try
	{
		vBlurFbo.create();
		vBlurFbo.setColorAttachments({&r->getPps().getPostPassFai()});
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create vertical blur "
			"post-processing stage FBO") << e;
	}

	vBlurSProg.load(ProgramResource::createSrcCodeToCache(
		"shaders/PpsBlurGeneric.glsl", "#define VPASS\n").c_str());

	// Side blur
	//
	try
	{
		sideBlurFbo.create();
		sideBlurFbo.setColorAttachments({&r->getMs().getFai0()});
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create side blur "
			"post-processing stage FBO") << e;
	}

	sideBlurMap.load("engine-rsrc/side-blur.png");
	sideBlurSProg.load("shaders/PpsSideBlur.glsl");
}

//==============================================================================
void Bl::runSideBlur()
{
	if(sideBlurFactor == 0.0)
	{
		return;
	}

	sideBlurFbo.bind();
	sideBlurSProg->bind();

	GlStateSingleton::get().enable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	sideBlurSProg->findUniformVariable("tex").set(
		static_cast<const Texture&>(*sideBlurMap));
	sideBlurSProg->findUniformVariable("factor").set(sideBlurFactor);

	r->drawQuad();
}

//==============================================================================
void Bl::runBlur()
{
	GlStateSingleton::get().disable(GL_BLEND);

	// Setup programs here. Reverse order
	vBlurSProg->bind();
	vBlurSProg->findUniformVariable("img").set(blurFai);
	vBlurSProg->findUniformVariable("msNormalFai").set(
		r->getMs().getFai0());
	vBlurSProg->findUniformVariable("imgDimension").set(
		float(r->getHeight()));

	hBlurSProg->bind();
	hBlurSProg->findUniformVariable("img").set(
		r->getPps().getPostPassFai());
	hBlurSProg->findUniformVariable("msNormalFai").set(
		r->getMs().getFai0());
	hBlurSProg->findUniformVariable("imgDimension").set(
		float(r->getWidth()));

	for(uint32_t i = 0; i < blurringIterationsNum; i++)
	{
		// hpass
		hBlurFbo.bind();
		hBlurSProg->bind();
		r->drawQuad();

		// vpass
		vBlurFbo.bind();
		vBlurSProg->bind();
		r->drawQuad();
	}
}

//==============================================================================
void Bl::run()
{
	if(!enabled)
	{
		return;
	}

	runSideBlur();
	runBlur();
}
#endif

} // end namespace anki
