#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "glslBase.glsl"
layout(location = 0) rayPayloadInEXT vec3 hitValue;
layout(location = 2) rayPayloadEXT bool shadowed;
hitAttributeEXT vec2 attribs;

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;

CBUFFER(UBO, UPDATE_FREQ_NONE, b2, binding = 2)
{
    mat4 viewInverse;
	mat4 projInverse;
	float4 lightPos;
	int vertexSize;
}ubo;



layout (std430, UPDATE_FREQ_NONE, binding = 3) readonly buffer vertexDataBufferStruct
{
	uint vertexDataBuffer_data[];
}vertexDataBuffer;


layout (std430, UPDATE_FREQ_NONE, binding = 4) readonly buffer indexDataBufferStruct
{
	uint indexDataBuffer_data[];
}indexDataBuffer;

struct Vertex
{
  vec3 pos;
  vec3 normal;
  vec4 color;
 };



struct GeometryNode {
	uint64_t vertexBufferDeviceAddress;
	uint64_t indexBufferDeviceAddress;
	uint vertexOffset;
    uint indexOffset;
	int textureIndexBaseColor;
	int textureIndexOcclusion;
};




layout (std430, UPDATE_FREQ_NONE, binding = 5) readonly buffer geometryNodesStruct
{
	GeometryNode geometryNodesBuffer_data[];
}geometryNodes;

float4 LoadVertexPosition(uint vtxIndex, uint offset)
{
    uint4 aa = LoadByte4(vertexDataBuffer.vertexDataBuffer_data, vtxIndex * 32 + offset);
    return asfloat(aa).xyzw;
}


uint3 LoadIndex(uint index)
{
    uint3 aa = LoadByte3(indexDataBuffer.indexDataBuffer_data, index * 4);
	return aa;
}


Vertex unpack(uint index)
{

	float4 d0 = LoadVertexPosition(index, 0);
	float4 d1 = LoadVertexPosition(index, 16);

	Vertex v;
	v.pos = d0.xyz;
	v.normal = vec3(d0.w, d1.x, d1.y);
	v.color = vec4(0.5, 0.0, 0.0, 1.0);

	return v;
}

void main()
{
    uint geometryIndex = gl_GeometryIndexEXT;
	
	uint indexOffset = geometryNodes.geometryNodesBuffer_data[geometryIndex].indexOffset;
	
	uint index_ = 3 * gl_PrimitiveID + indexOffset;
	
	uint3 index = LoadIndex(index_);
	
	uint vertexOffset = geometryNodes.geometryNodesBuffer_data[geometryIndex].vertexOffset;
	
	Vertex v0 = unpack(index.x + vertexOffset);
	Vertex v1 = unpack(index.y + vertexOffset);
	Vertex v2 = unpack(index.z + vertexOffset);

	// Interpolate normal
	const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
	vec3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);

	// Basic lighting
	vec3 lightVector = normalize(ubo.lightPos.xyz);
	float dot_product = max(dot(lightVector, normal), 0.2);
	hitValue = v0.color.rgb * dot_product;
 
	// Shadow casting
	float tmin = 0.001;
	float tmax = 10000.0;
	vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	shadowed = true;  
	// Trace shadow ray and offset indices to match shadow hit/miss shader group indices
	traceRayEXT(topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT, 0xFF, 0, 0, 1, origin, tmin, lightVector, tmax, 2);
	if (shadowed) {
		hitValue *= 0.3;
	}
}