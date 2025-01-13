// Copyright 2020 Google LLC
#include "base.hlsl"
RaytracingAccelerationStructure topLevelAS VKBINDING(0, 0): register(t0);
RWTexture2D<float4> image VKBINDING(1, 0): register(u1);

struct UBO
{
	float4x4 viewInverse;
	float4x4 projInverse;
	float4 lightPos;
	int vertexSize;
};

RES(CBUFFER(UBO), ubo, UPDATE_FREQ_NONE, b2, VKBINDING(2, 0));

StructuredBuffer<float4> vertices VKBINDING(3, 0): register(t3);
StructuredBuffer<uint> indices VKBINDING(4, 0): register(t4);

struct GeometryNode {
	uint64_t vertexBufferDeviceAddress;
	uint64_t indexBufferDeviceAddress;
	uint vertexOffset;
    uint indexOffset;
	int textureIndexBaseColor;
	int textureIndexOcclusion;
};

RES(Buffer(GeometryNode), geometryNodes, UPDATE_FREQ_NONE, t5, VKBINDING(5, 0));

RES(ByteBuffer, vertexDataBuffer, UPDATE_FREQ_NONE, t6, VKBINDING(6, 0));

RES(ByteBuffer, indexDataBuffer, UPDATE_FREQ_NONE, t7, VKBINDING(7, 0));

struct Payload
{
	[[vk::location(0)]] float4 hitValue;
	[[vk::location(1)]] bool shadowed;
};

struct Attributes
{
  float2 bary;
};

[shader("raygeneration")]
void rayGenMain()
{
	uint3 LaunchID = DispatchRaysIndex();
	uint3 LaunchSize = DispatchRaysDimensions();

	const float2 pixelCenter = float2(LaunchID.xy) + float2(0.5, 0.5);
	const float2 inUV = pixelCenter/float2(LaunchSize.xy);
	float2 d = inUV * 2.0 - 1.0;
	float4 target = mul(ubo.projInverse, float4(d.x, d.y, 1, 1));

	RayDesc rayDesc;
	rayDesc.Origin = mul(ubo.viewInverse, float4(0,0,0,1)).xyz;
	rayDesc.Direction = mul(ubo.viewInverse, float4(normalize(target.xyz), 0)).xyz;
	rayDesc.TMin = 0.001;
	rayDesc.TMax = 10000.0;

	Payload payload;
	TraceRay(topLevelAS, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, rayDesc, payload);

	image[int2(LaunchID.xy)] = payload.hitValue;
}


struct Vertex
{
  float3 pos;
  float3 normal;
  float2 uv;
  float4 color;

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

[shader("closesthit")]
void closethitMain(inout Payload payload, in Attributes attribs)
{
    Triangle tri = unpackTriangle(gl_PrimitiveID);
	
	
	Vertex v0 = tri.vertices[0];

	// Interpolate normal
	const float3 barycentricCoords = float3(1.0f - attribs.bary.x - attribs.bary.y, attribs.bary.x, attribs.bary.y);
	float3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);

	// Basic lighting
	float3 lightVector = normalize(ubo.lightPos.xyz);
	float dot_product = max(dot(lightVector, normal), 0.2);
	payload.hitValue = float4(v0.color.rgb * dot_product, 0.0f);
	
	return;
	RayDesc rayDesc;
	rayDesc.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
	rayDesc.Direction = lightVector;
	rayDesc.TMin = 0.001;
	rayDesc.TMax = 100.0;

	payload.shadowed = true;
	// Offset indices to match shadow hit/miss index
	TraceRay(topLevelAS, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xff, 0, 0, 1, rayDesc, payload);
	if (payload.shadowed) {
		payload.hitValue *= 0.3;
	}
}

[shader("miss")]
void missMain(inout Payload p)
{
    p.hitValue = float4(0.0, 0.2, 0.2, 0.0f);
}

[shader("miss")]
void shadowMissmain(inout Payload payload)
{
	payload.shadowed = false;
}