// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_DRAWER_H
#define ANKI_RENDERER_DRAWER_H

#include "anki/util/StdTypes.h"
#include "anki/util/Ptr.h"
#include "anki/resource/RenderingKey.h"
#include "anki/scene/Forward.h"
#include "anki/Gr.h"

namespace anki {

// Forward
class Renderer;

/// @addtogroup renderer
/// @{

/// The rendering stage
enum class RenderingStage: U8
{
	MATERIAL,
	BLEND
};

/// It includes all the functions to render a Renderable
class RenderableDrawer
{
	friend class SetupRenderableVariableVisitor;

public:
	static const U32 MAX_UNIFORM_BUFFER_SIZE = 1024 * 1024 * 2;

	/// The one and only constructor
	ANKI_USE_RESULT Error create(Renderer* r);

	void prepareDraw(
		RenderingStage stage, Pass pass, CommandBufferHandle& cmdBuff);

	ANKI_USE_RESULT Error render(
		SceneNode& frsn,
		VisibleNode& visible);

	void finishDraw();

private:
	Renderer* m_r;
	BufferHandle m_uniformBuff;
	U8* m_uniformBuffMapAddr = nullptr;

	/// @name State
	/// @{
	CommandBufferHandle m_cmdBuff;
	U8* m_uniformPtr;

	/// Used to calc if the uni buffer is big enough. Zero it per swap buffers
	U32 m_uniformsUsedSize; 
	U32 m_uniformsUsedSizeFrame;

	RenderingStage m_stage;
	Pass m_pass;
	/// @}

	void setupUniforms(
		VisibleNode& visibleNode, 
		RenderComponent& renderable,
		FrustumComponent& fr,
		F32 flod);
};

/// @}

} // end namespace anki

#endif
