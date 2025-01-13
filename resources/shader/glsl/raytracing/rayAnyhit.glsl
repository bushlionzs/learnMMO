/* Copyright (c) 2023, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */
#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#include "glslBase.glsl"
layout(location = 3) rayPayloadInEXT uint payloadSeed;

hitAttributeEXT vec2 attribs;

layout(binding = 3, set = 0) uniform sampler2D samplerImage;


struct GeometryNode {
	uint64_t vertexBufferDeviceAddress;
	uint64_t indexBufferDeviceAddress;
	uint vertexOffset;
    uint indexOffset;
	int textureIndexBaseColor;
	int textureIndexOcclusion;
};
layout(binding = 4, set = 0) buffer GeometryNodes { GeometryNode nodes[]; } geometryNodes;

layout(binding = 5, set = 0) uniform sampler2D textures[];

layout (std430, UPDATE_FREQ_NONE, binding = 6) readonly buffer vertexDataBufferStruct
{
	uint vertexDataBuffer_data[];
}vertexDataBuffer;


layout (std430, UPDATE_FREQ_NONE, binding = 7) readonly buffer indexDataBufferStruct
{
	uint indexDataBuffer_data[];
}indexDataBuffer;

#include "bufferreferences.glsl"
#include "geometrytypes.glsl"
#include "random.glsl"

void main()
{
	Triangle tri = unpackTriangle(gl_PrimitiveID, 112);
	GeometryNode geometryNode = geometryNodes.nodes[gl_GeometryIndexEXT];
	vec4 color = vec4(0.2, 0.0, 0.0, 1.0f);
	if(geometryNode.textureIndexBaseColor > -1)
	{
	    color = texture(textures[nonuniformEXT(geometryNode.textureIndexBaseColor)], tri.uv);
	}
	
	// If the alpha value of the texture at the current UV coordinates is below a given threshold, we'll ignore this intersection
	// That way ray traversal will be stopped and the miss shader will be invoked
	if (color.a < 0.9) {
		if(rnd(payloadSeed) > color.a) {
			ignoreIntersectionEXT;
		}
	}
}