#include "base.hlsl"




STRUCT(ObjectUniform)
{
    float4x4 worldViewProjMat;
    uint viewID;
};


STRUCT(PsInOpaque)
{
	DATA(float4, position, SV_Position);
#line 57
};



RES(ByteBuffer, vertexDataBuffer, UPDATE_FREQ_NONE, t2, VKBINDING(4, 0));
RES(CBUFFER(ObjectUniform), objectUniformBlock, UPDATE_FREQ_PER_DRAW, b0, VKBINDING(0, 3));


float3 LoadVertexPositionFloat3(uint vtxIndex)
{
    return asfloat(LoadByte4(vertexDataBuffer, vtxIndex * 32)).xyz;
}

float4 LoadVertex(uint index)
{
    return float4(LoadVertexPositionFloat3(index), 1.0f);
}

float4 unpackUnorm4x8(uint p)
{
	return float4(float(p & 0x000000FF) / 255.0,
		float((p & 0x0000FF00) >> 8) / 255.0,
		float((p & 0x00FF0000) >> 16) / 255.0,
		float((p & 0xFF000000) >> 24) / 255.0);
}





PsInOpaque VS(uint vertexID : SV_VERTEXID)
{
	PsInOpaque Out;

    float4 vertexPos = LoadVertex(vertexID);
    Out.position = mul(objectUniformBlock.worldViewProjMat, vertexPos);

    return (Out);
}



float4 PS(PsInOpaque In, uint primitiveID : SV_PRIMITIVEID ): SV_TARGET
{
	float4 Out;
	Out = unpackUnorm4x8(  (((( 0 ) & 0x00000003 ) << 30 ) | (((primitiveID) & 0x3FFFFFFF ) << 0 )) );
	return (Out);
}
