/* Copyright (c) 2023, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

struct Vertex
{
  vec3 pos;
  vec3 normal;
  vec2 uv;
};

struct Triangle {
	Vertex vertices[3];
	vec3 normal;
	vec2 uv;
};

vec4 LoadVertexPosition(uint vtxIndex, uint offset)
{
    uvec4 aa = LoadByte4(vertexDataBuffer.vertexDataBuffer_data, vtxIndex * 32 + offset);
    return asfloat(aa).xyzw;
}


uint LoadIndex(uint index)
{
    uint aa = LoadByte(indexDataBuffer.indexDataBuffer_data, index * 4);
	return aa;
}

// This function will unpack our vertex buffer data into a single triangle and calculates uv coordinates
Triangle unpackTriangle(uint index, int vertexSize) {
	Triangle tri;
	uint triIndex = index * 3;

	GeometryNode geometryNode = geometryNodes.nodes[gl_GeometryIndexEXT];

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
	return tri;
}