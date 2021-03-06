// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Common code for all fragment shaders of BS
#define DEFAULT_FLOAT_PRECISION mediump

#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/MsBsCommon.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

layout(location = 1) flat in float inAlpha;

#if PASS == COLOR
layout(location = 0) out vec4 outColor;
#	define outColor_DEFINED
#endif

#if PASS == COLOR
#	define texture_DEFINED
#endif

//==============================================================================
#define getAlpha_DEFINED
float getAlpha()
{
	return inAlpha;
}

//==============================================================================
#define getPointCoord_DEFINED
#define getPointCoord() gl_PointCoord

//==============================================================================
#if PASS == COLOR
#	define writeGBuffer_DEFINED
void writeGBuffer(in vec4 color)
{
	outColor = color;
}
#endif

//==============================================================================
#if PASS == COLOR
#	define particleAlpha_DEFINED
void particleAlpha(in sampler2D tex, in float alpha)
{
	vec4 color = texture(tex, gl_PointCoord);
	color.a *= alpha;
	writeGBuffer(color);
}
#endif

//==============================================================================
#if PASS == COLOR
#	define particleSoftTextureAlpha_DEFINED
void particleSoftTextureAlpha(in sampler2D depthMap, in sampler2D tex, 
	in float alpha)
{
	const vec2 screenSize = vec2(
		1.0 / float(ANKI_RENDERER_WIDTH), 
		1.0 / float(ANKI_RENDERER_HEIGHT));

	float depth = texture(depthMap, gl_FragCoord.xy * screenSize).r;

	float delta = depth - gl_FragCoord.z;
	float softalpha = clamp(delta * 50.0, 0.0, 1.0);

	vec4 color = texture(tex, gl_PointCoord);
	color.a *= alpha * softalpha;

	writeGBuffer(color);
}
#endif

//==============================================================================
#if PASS == COLOR
#	define particleSoftColorAlpha_DEFINED
void particleSoftColorAlpha(in sampler2D depthMap, in vec3 icolor, 
	in float alpha)
{
	const vec2 screenSize = vec2(
		1.0 / float(ANKI_RENDERER_WIDTH), 
		1.0 / float(ANKI_RENDERER_HEIGHT));

	float depth = texture(depthMap, gl_FragCoord.xy * screenSize).r;

	float delta = depth - gl_FragCoord.z;
	float softalpha = clamp(delta * 50.0, 0.0, 1.0);

	vec2 pix = (1.0 - abs(gl_PointCoord * 2.0 - 1.0));
	float roundFactor = pix.x * pix.y;

	vec4 color;
	color.rgb = icolor;
	color.a = alpha * softalpha * roundFactor;
	writeGBuffer(color);
}
#endif

//==============================================================================
#if PASS == COLOR
#	define fog_DEFINED
void fog(in sampler2D depthMap, in vec3 color, in float fogScale)
{
	const vec2 screenSize = vec2(
		1.0 / float(ANKI_RENDERER_WIDTH), 
		1.0 / float(ANKI_RENDERER_HEIGHT));

	vec2 texCoords = gl_FragCoord.xy * screenSize;
	float depth = texture(depthMap, texCoords);
	float zNear = 0.2;
	float zFar = 200.0;
	float linearDepth = (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));

	float depth2 = gl_FragCoord.z;
	float linearDepth2 = (2.0 * zNear) / (zFar + zNear - depth2 * (zFar - zNear));

	float diff = linearDepth - linearDepth2;

	//writeGBuffer(vec4(vec3(diff * fogScale), 1.0));
	writeGBuffer(vec4(color, diff * fogScale));
}
#endif
