// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_EVENT_SCENE_AMBIENT_COLOR_EVENT_H
#define ANKI_EVENT_SCENE_AMBIENT_COLOR_EVENT_H

#include "anki/event/Event.h"
#include "anki/Math.h"

namespace anki {

/// @addtogroup event
/// @{

/// Change the scene color
class SceneAmbientColorEvent: public Event
{
public:
	/// Create
	ANKI_USE_RESULT Error create(
		EventManager* manager, F32 startTime, F32 duration,
		const Vec4& finalColor);

	/// Implements Event::update
	ANKI_USE_RESULT Error update(F32 prevUpdateTime, F32 crntTime);

private:
	Vec4 m_originalColor; ///< Original scene color. The constructor sets it
	Vec4 m_finalColor;
};
/// @}

} // end namespace anki

#endif
