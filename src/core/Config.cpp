// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/core/Config.h"

namespace anki {

//==============================================================================
Config::Config()
{
	//
	// Renderer
	// 

	// Ms
	newOption("ms.ez.enabled", false);
	newOption("ms.ez.maxObjectsToDraw", 10);

	// Is
	newOption("is.sm.enabled", true);
	newOption("is.sm.poissonEnabled", true);
	newOption("is.sm.bilinearEnabled", true);
	newOption("is.sm.resolution", 512);
	newOption("is.sm.maxLights", 4);

	newOption("is.groundLightEnabled", true);
	newOption("is.maxPointLights", 384);
	newOption("is.maxSpotLights", 16);
	newOption("is.maxSpotTexLights", 8);
	newOption("is.maxLightsPerTile", 16);

	// Pps
	newOption("pps.hdr.enabled", true);
	newOption("pps.hdr.renderingQuality", 0.5);
	newOption("pps.hdr.blurringDist", 1.0);
	newOption("pps.hdr.samples", 5);
	newOption("pps.hdr.blurringIterationsCount", 1);
	newOption("pps.hdr.exposure", 4.0);

	newOption("pps.ssao.enabled", true);
	newOption("pps.ssao.renderingQuality", 0.3);
	newOption("pps.ssao.blurringIterationsCount", 1);

	newOption("pps.sslr.enabled", true);
	newOption("pps.sslr.renderingQuality", 0.2);
	newOption("pps.sslr.blurringIterationsCount", 1);

	newOption("pps.bl.enabled", true);
	newOption("pps.bl.blurringIterationsCount", 1);
	newOption("pps.bl.sideBlurFactor", 1.0);

	newOption("pps.lf.enabled", true);
	newOption("pps.lf.maxSpritesPerFlare", 8);
	newOption("pps.lf.maxFlares", 16);

	newOption("pps.enabled", true);
	newOption("pps.sharpen", true);
	newOption("pps.gammaCorrection", true);

	// Dbg
	newOption("dbg.enabled", false);

	// Globals
	newOption("width", 0);
	newOption("height", 0);
	newOption("renderingQuality", 1.0); // Applies only to MainRenderer
	newOption("lodDistance", 10.0); // Distance that used to calculate the LOD
	newOption("samples", 1);
	newOption("tilesXCount", 30);
	newOption("tilesYCount", 20);
	newOption("tessellation", true);
	newOption("sceneFrameAllocatorSize", 1024 * 1024);

	newOption("offscreen", false);

	//
	// Resource
	//

	newOption("maxTextureSize", 1024 * 1024);
	newOption("textureAnisotropy", 8);

	//
	// Window
	//
	newOption("glminor", 4);
	newOption("glmajor", 4);
	newOption("fullscreenDesktopResolution", false);
	newOption("debugContext", 
#if ANKI_DEBUG == 1
		true
#else
		false
#endif
		);
}

//==============================================================================
Config::~Config()
{}

} // end namespace anki

