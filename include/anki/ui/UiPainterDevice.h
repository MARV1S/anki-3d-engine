// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UI_UI_PAINTER_DEVICE_H
#define ANKI_UI_UI_PAINTER_DEVICE_H

#include "anki/gr/Fbo.h"
#include "anki/Math.h"

namespace anki {

class Texture;

/// This actually and FBO but with size info
class UiPainterDevice: public Fbo
{
public:
	/// Constructor
	UiPainterDevice(Texture* colorFai);

	/// @name Accessors
	/// @{
	Vec2 getSize() const;
	/// @}

private:
	Texture* colorFai;

	void create();
};

} // end namespace anki

#endif
