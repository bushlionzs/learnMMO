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
	int frame;
}ubo;



layout (std430, UPDATE_FREQ_NONE, binding = 3) readonly buffer vertexDataBufferStruct
{
	uint vertexDataBuffer_data[];
}vertexDataBuffer;


layout (std430, UPDATE_FREQ_NONE, binding = 4) readonly buffer indexDataBufferStruct
{
	uint indexDataBuffer_data[];
}indexDataBuffer;


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
    uint4 aa = LoadByte4(vertexDataBuffer.vertexDataBuffer_data, vtxIndex * ubo.vertexSize + offset);
    return asfloat(aa).xyzw;
}


uint LoadIndex(uint index)
{
    uint aa = LoadByte(indexDataBuffer.indexDataBuffer_data, index * 4);
	return aa;
}

struct Vertex
{
  vec3 pos;
  vec3 normal;
  vec2 uv;
  vec4 color;
};

struct Triangle {
	Vertex vertices[3];
	vec3 normal;
	vec2 uv;
};

Triangle unpackTriangle(uint index) {
	Triangle tri;
	uint triIndex = index * 3;

	GeometryNode geometryNode = geometryNodes.geometryNodesBuffer_data[gl_GeometryIndexEXT];

    triIndex += geometryNode.indexOffset;
	// Unpack vertices
	// Data is packed as vec4 so we can map to the glTF vertex structure from the host side
	// We match vkglTF::Vertex: pos.xyz+normal.x, normalyz+uv.xy
	// glm::vec3 pos;
	// glm::vec3 normal;
	// glm::vec2 uv;
	// ...
	for (uint i = 0; i < 3; i++) {
		uint vtxIndex = LoadIndex(triIndex + i);
		vtxIndex += geometryNode.vertexOffset;
		vec4 d0 = LoadVertexPosition(vtxIndex, 0);
		vec4 d1 = LoadVertexPosition(vtxIndex, 16);
		tri.vertices[i].pos = d0.xyz;
		tri.vertices[i].normal = vec3(d0.w, d1.xy);
		tri.vertices[i].uv = d1.zw;
	}
	// Calculate values at barycentric coordinates
	vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
	tri.uv = tri.vertices[0].uv * barycentricCoords.x + tri.vertices[1].uv * barycentricCoords.y + tri.vertices[2].uv * barycentricCoords.z;
	tri.normal = tri.vertices[0].normal * barycentricCoords.x + tri.vertices[1].normal * barycentricCoords.y + tri.vertices[2].normal * barycentricCoords.z;
	tri.vertices[0].color = vec4(1.0, 1.0, 1.0, 1.0);
	return tri;
}

void main()
{
	Triangle tri = unpackTriangle(gl_PrimitiveID);
	
	
	Vertex v0 = tri.vertices[0];


	// Basic lighting
	vec3 lightVector = normalize(ubo.lightPos.xyz);
	float dot_product = max(dot(lightVector, tri.normal), 0.2);

	hitValue = v0.color.rgb * dot_product;

	// Shadow casting
	float tmin = 0.001;
	float tmax = 10000.0;
	vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	shadowed = true;  
	// Trace shadow ray and offset indices to match shadow hit/miss shader group indices
	uint rayFlags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
	traceRayEXT(topLevelAS, rayFlags, 0xFF, 0, 0, 1, origin, tmin, lightVector, tmax, 2);
	if (shadowed) {
		hitValue *= 0.3;
	}
}