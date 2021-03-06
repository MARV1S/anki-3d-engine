// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UI_UI_COMMON_H
#define ANKI_UI_UI_COMMON_H

#include "anki/Math.h"
#include "anki/util/Allocator.h"

namespace anki {

/// @addtogroup ui
/// @{

/// Color
typedef Vec4 UiColor;

/// 2D point
typedef Vec2 UiPoint;

/// 2D position
typedef Vec2 UiPosition;

/// Rectangle
struct UiRect
{
	UiPosition min;
	UiPosition max;

	UiRect(UiPosition min_, UiPosition max_)
		: min(min_), max(max_)
	{
		ANKI_ASSERT(min < max);
	}
};

/// @}

} // end namespace anki

#endif
