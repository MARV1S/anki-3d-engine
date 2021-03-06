// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_RTTI_H
#define ANKI_UTIL_RTTI_H

#include "anki/util/Assert.h"

namespace anki {

/// @addtogroup util_private
/// @{
template<typename T>
struct ExtractType
{
	using Type = T;
};

template<typename T>
struct ExtractType<const T&>
{
	using Type = T;
};

template<typename T>
struct ExtractType<T&>
{
	using Type = T;
};

template<typename T>
struct ExtractType<const T*>
{
	using Type = T;
};

template<typename T>
struct ExtractType<T*>
{
	using Type = T;
};
/// @}

/// @addtogroup util_misc
/// @{

/// Check if a class is of certain type.
template<typename TTo, typename TFrom>
inline Bool isa(TFrom& c)
{
	return TTo::classof(c);
}

/// Check if a class is of certain type.
template<typename TTo, typename TFrom>
inline Bool isa(TFrom* c)
{
	return TTo::classof(*c);
}

/// Custom dynamic cast.
template<typename TTo, typename TFrom>
inline TTo dcast(TFrom& c)
{
	ANKI_ASSERT(isa<typename ExtractType<TTo>::Type>(c));
	return static_cast<TTo>(c);
}

/// Custom dynamic cast.
template<typename TTo, typename TFrom>
inline TTo dcast(TFrom* c)
{
	ANKI_ASSERT(isa<typename ExtractType<TTo>::Type>(c));
	return static_cast<TTo>(c);
}
/// @}

} // end namespace anki

#endif
