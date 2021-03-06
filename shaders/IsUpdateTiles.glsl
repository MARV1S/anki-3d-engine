// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Compute shader that bins lights to the tiles

#pragma anki start computeShader

// Set number of work items
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#pragma anki include "shaders/IsCommon.glsl"

//
// Data
//

// The tilegrid input buffer
layout(std140, binding = 0) buffer tilegridBuffer
{
	Tilegrid tilegrid;
};

// The lights input buffers
layout(std140, binding = 1) buffer lightsBuffer
{
	Lights lights;
}; 

// The output buffer
layout(std430, binding = 2) buffer tilesBuffer
{
	Tile tiles[TILES_Y_COUNT][TILES_X_COUNT];
};

// Buffers with atomic counters for each tile
layout(std430, binding = 1) buffer pointLightsCountBuffer
{
	atomic_uint pointLightsCount[TILES_Y_COUNT][TILES_X_COUNT];
};
layout(std430, binding = 2) buffer spotLightsCountBuffer
{
	atomic_uint spotLightsCount[TILES_Y_COUNT][TILES_X_COUNT];
};
layout(std430, binding = 3) buffer spotTexLightsCountBuffer
{
	atomic_uint spotTexLightsCount[TILES_Y_COUNT][TILES_X_COUNT];
};

//
// Functions
//

// Return true if spere is in the half space defined by the plane
bool pointLightInsidePlane(in PointLight light, in Plane plane)
{	
	// Result of the plane test. Distance of 
	float dist = dot(plane.normalOffset.xyz, light.posRadius.xyz) 
		- plane.normalOffset.w;

	return dist >= 0.0;
}

// Spot light inside plane
bool spotLightInsidePlane(in SpotLight light, in Plane plane)
{
	if(pointLightInsidePlane(light.lightBase, plane))
	{
		return true;
	}

	for(uint i = 0U; i < 4; i++)
	{
		float dist = dot(plane.normalOffset.xyz, light.extendPoints[i].xyz) 
			- plane.normalOffset.w;

		if(dist >= 0.0)
		{
			return true;
		}
	}
	return false;
}

// Spot tex light inside plane
bool spotTexLightInsidePlane(in SpotTexLight light, in Plane plane)
{
	return pointLightInsidePlane(light.spotLightBase, plane);
}

// main
void main()
{
	// First the point lights
	for(uint i = 0U; i < lights.count.x; i++)
	{
		
	}
}
