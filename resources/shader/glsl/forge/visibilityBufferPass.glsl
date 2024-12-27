#version 450 core
#extension GL_GOOGLE_include_directive : require

#include "glslBase.glsl"

#ifdef VERTEX_SHADER
layout (std430, UPDATE_FREQ_NONE, binding = 3) readonly buffer vertexDataBufferStruct
{
	uint vertexDataBuffer_data[];
}vertexDataBuffer;

float3 LoadVertexPositionFloat3(uint vtxIndex)
{
    return asfloat(LoadByte4(vertexDataBuffer.vertexDataBuffer_data, vtxIndex * 32)).xyz;
}

float4 LoadVertex(uint index)
{
    return float4(LoadVertexPositionFloat3(index), 1.0f);
}

CBUFFER(objectUniformBlockStruct, UPDATE_FREQ_PER_DRAW, b0, binding = 0)
{
    DATA(float4x4, worldViewProjMat, None);
    DATA(uint, viewID, None);
}objectUniformBlock;


void main()
{
	const uint vertexID = uint(gl_VertexIndex);
	float4 vertexPos = LoadVertex(vertexID);
	gl_Position = objectUniformBlock.worldViewProjMat * vertexPos;
}
#endif //VERTEX_SHADER

#ifdef FRAGMENT_SHADER
layout(location = 0) out(float4) out_float4;
void main()
{
	const uint primitiveID = uint(gl_PrimitiveID);
	out_float4 = unpackUnorm4x8(  (((( 0 ) & 0x00000003 ) << 30 ) | (((primitiveID) & 0x3FFFFFFF ) << 0 )) );
}
#endif //FRAGMENT_SHADER