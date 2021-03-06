// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_INPUT_INPUT_SDL_H
#define ANKI_INPUT_INPUT_SDL_H

#include "anki/input/KeyCode.h"
#include <SDL_keycode.h>
#include <unordered_map>

namespace anki {

/// SDL input implementation
class InputImpl
{
public:
	std::unordered_map<
		SDL_Keycode, 
		KeyCode, 
		std::hash<SDL_Keycode>,
		std::equal_to<SDL_Keycode>,
		HeapAllocator<std::pair<const SDL_Keycode, KeyCode>>> m_sdlToAnki;

	InputImpl(HeapAllocator<std::pair<const SDL_Keycode, KeyCode>>& alloc)
	:	m_sdlToAnki(10, std::hash<SDL_Keycode>(), std::equal_to<SDL_Keycode>(),
			alloc)
	{}
};

} // end namespace anki

#endif

