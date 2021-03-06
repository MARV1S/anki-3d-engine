// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/event/Event.h"
#include "anki/resource/Resource.h"

namespace anki {

/// @addtogroup event
/// @{

/// Event controled by animation resource
class AnimationEvent: public Event
{
public:
	ANKI_USE_RESULT Error create(
		EventManager* manager, 
		const AnimationResourcePointer& anim, 
		SceneNode* movableSceneNode);

	/// Implements Event::update
	ANKI_USE_RESULT Error update(F32 prevUpdateTime, F32 crntTime) override;

private:
	AnimationResourcePointer m_anim;
};
/// @}

} // end namespace anki
