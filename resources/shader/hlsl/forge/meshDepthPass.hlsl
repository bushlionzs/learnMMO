#include "base.hlsl"

RES(ByteBuffer, vertexDataBuffer, UPDATE_FREQ_NONE, t2, VKBINDING(4, 0));


STRUCT(ObjectUniform)
{
    float4x4 worldViewProjMat;
    uint viewID;
};

RES(CBUFFER(ObjectUniform), objectUniformBlock, UPDATE_FREQ_PER_DRAW, b0, VKBINDING(0, 3));


float3 LoadVertexPositionFloat3(uint vtxIndex)
{
    uint4 aa = LoadByte4(vertexDataBuffer, vtxIndex * 32);
    return asfloat(aa).xyz;
}

float4 LoadVertex(uint index)
{
    return float4(LoadVertexPositionFloat3(index), 1.0f);
}



struct VertexOut
{
	float4 position    : SV_POSITION;
};

VertexOut VS(uint vertexID : SV_VERTEXID)
{
    VertexOut Out;
	float4 vertexPos = LoadVertex(vertexID);
	
	Out.position = mul(objectUniformBlock.worldViewProjMat, vertexPos);
	return Out;
}

