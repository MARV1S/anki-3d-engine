// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/collision/Sphere.h"
#include "anki/collision/Plane.h"
#include "anki/collision/Aabb.h"

namespace anki {

//==============================================================================
F32 Sphere::testPlane(const Plane& p) const
{
	F32 dist = p.test(m_center);

	F32 out = dist - m_radius;
	if(out > 0)
	{
		// Do nothing
	}
	else
	{
		out = dist + m_radius;
		if(out < 0)
		{
			// Do nothing
		}
		else
		{
			out = 0.0;
		}
	}

	return out;
}

//==============================================================================
Sphere Sphere::getTransformed(const Transform& trf) const
{
	Sphere newSphere;

	newSphere.m_center = trf.transform(m_center);
	newSphere.m_radius = m_radius * trf.getScale();
	return newSphere;
}

//==============================================================================
Sphere Sphere::getCompoundShape(const Sphere& b) const
{
	const Sphere& a = *this;

	/// TODO test this
	/*
	Vec3 centerDiff = b.center - a.center;
	F32 radiusDiff = b.radius - a.radius;
	Vec3 radiusDiffSqr = radiusDiff * radiusDiff;
	F32 lenSqr = centerDiff.getLengthSquared();

	if(radiusDiffSqrt >= 0.0)
	{
		if(radiusDiff >= 0.0)
		{
			return b;
		}
		else
		{
			return a;
		}
	}
	else
	{
		F32 l = sqrt(lenSqr);
		F32 t = (l + b.radius - a.radius) / (2.0 * l);
		return Sphere(a.center + t * centerDiff, (l + a.radius + b.radius) /
			2.0);
	}
	*/

	Vec4 c = b.getCenter() - a.getCenter(); // Vector from one center to the
	                                        // other
	F32 cLen = c.getLength();

	if(cLen + b.getRadius() < a.getRadius())
	{
		return a;
	}
	else if(cLen + a.getRadius() < b.getRadius())
	{
		return b;
	}

	Vec4 bnorm = c / cLen;

	Vec4 ca = (-bnorm) * a.getRadius() + a.getCenter();
	Vec4 cb = (bnorm) * b.getRadius() + b.getCenter();

	return Sphere((ca + cb) / 2.0, (ca - cb).getLength() / 2.0);
}

//==============================================================================
void Sphere::computeAabb(Aabb& aabb) const
{
	aabb.setMin(m_center - m_radius);
	aabb.setMax(m_center + m_radius);
}

//==============================================================================
void Sphere::setFromPointCloud(
	const void* buff, U count, PtrSize stride, PtrSize buffSize)
{
	// Calc min/max
	Vec4 min(Vec3(MAX_F32), 0.0);
	Vec4 max(Vec3(MIN_F32), 0.0);
	
	iteratePointCloud(buff, count, stride, buffSize, [&](const Vec3& pos)
	{
		for(U j = 0; j < 3; j++)
		{
			if(pos[j] > max[j])
			{
				max[j] = pos[j];
			}
			else if(pos[j] < min[j])
			{
				min[j] = pos[j];
			}
		}
	});

	m_center = (min + max) * 0.5; // average

	// max distance between center and the vec3 arr
	F32 maxDist = MIN_F32;

	iteratePointCloud(buff, count, stride, buffSize, [&](const Vec3& pos)
	{
		F32 dist = (Vec4(pos, 0.0) - m_center).getLengthSquared();
		if(dist > maxDist)
		{
			maxDist = dist;
		}
	});

	m_radius = sqrt(maxDist);
}

//==============================================================================
Vec4 Sphere::computeSupport(const Vec4& dir) const
{
	return m_center + dir.getNormalized() * m_radius;
}

} // end namespace anki
