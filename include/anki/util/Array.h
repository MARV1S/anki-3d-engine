// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PtrSizeTIL_ARRAY_H
#define ANKI_PtrSizeTIL_ARRAY_H

#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util_containers
/// @{

/// Like std::array but with some additions
template<typename T, PtrSize N>
class Array
{
public:
	using Value = T;
	using Iterator = Value*;
	using ConstIterator = const Value*;
	using Reference = Value&;
	using ConstReference = const Value&;

	// STL compatible
	using iterator = Iterator;
	using const_iterator = ConstIterator;
	using reference = Reference;
	using const_reference = ConstReference;

	Value m_data[N];

	Reference operator[](const PtrSize n)
	{
		ANKI_ASSERT(n < N);
		return m_data[n];
	}

	ConstReference operator[](const PtrSize n) const
	{
		ANKI_ASSERT(n < N);
		return m_data[n];
	}

	Iterator getBegin()
	{
		return &m_data[0];
	}

	ConstIterator getBegin() const
	{
		return &m_data[0];
	}

	Iterator getEnd()
	{
		return &m_data[0] + N;
	}

	ConstIterator getEnd() const
	{
		return &m_data[0] + N;
	}

	Reference getFront() 
	{
		return m_data[0];
	}

	ConstReference getFront() const
	{
		return m_data[0];
	}

	Reference getBack() 
	{
		return m_data[N - 1];
	}

	ConstReference getBack() const
	{
		return m_data[N - 1];
	}

	/// Make it compatible with the C++11 range based for loop
	Iterator begin()
	{
		return getBegin();
	}

	/// Make it compatible with the C++11 range based for loop
	ConstIterator begin() const
	{
		return getBegin();
	}

	/// Make it compatible with the C++11 range based for loop
	Iterator end()
	{
		return getEnd();
	}

	/// Make it compatible with the C++11 range based for loop
	ConstIterator end() const
	{
		return getEnd();
	}

	/// Make it compatible with STL
	Reference front() 
	{
		return getFront();
	}

	/// Make it compatible with STL
	ConstReference front() const
	{
		return getFront();
	}

	/// Make it compatible with STL
	Reference back() 
	{
		return getBack;
	}

	/// Make it compatible with STL
	ConstReference back() const
	{
		return getBack();
	}

	static constexpr PtrSize getSize()
	{
		return N;
	}

	/// Make it compatible with STL
	static constexpr PtrSize size()
	{
		return N;
	}
};

/// 2D Array. @code Array2d<X, 10, 2> a; @endcode is equivelent to 
/// @code X a[10][2]; @endcode
template<typename T, PtrSize I, PtrSize J>
using Array2d = Array<Array<T, J>, I>;

/// 3D Array. @code Array3d<X, 10, 2, 3> a; @endcode is equivelent to 
/// @code X a[10][2][3]; @endcode
template<typename T, PtrSize I, PtrSize J, PtrSize K>
using Array3d = Array<Array<Array<T, K>, J>, I>;

/// @}

} // end namespace anki

#endif
