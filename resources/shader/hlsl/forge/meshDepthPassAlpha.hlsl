#include "base.hlsl"


STRUCT(ObjectUniform)
{
    float4x4 worldViewProjMat;
    uint viewID;
};


STRUCT(Transform)
{
	float4x4 vp;
	float4x4 mvp;
	float4x4 projection;
	float4x4 invVP;
};

STRUCT(CullingViewPort)
{
	float2 windowSize;
	uint sampleCount;
	uint _pad0;
};


STRUCT(PerFrameVBConstantsData)
{
	Transform transform[ 5 ];
	CullingViewPort cullingViewports[ 5 ];
	uint numViewports;
#line 90
};

struct VBConstants
{
	uint indexOffset;
	uint pad_0;
	uint pad_1;
	uint pad_2;
};

STRUCT(VBConstantBufferData)
{
    VBConstants vbConstant[ 2 ];
};

RES(ByteBuffer, vertexDataBuffer, UPDATE_FREQ_NONE, t2, VKBINDING(4, 0));

RES(Buffer(uint), indirectDataBuffer, UPDATE_FREQ_PER_FRAME, t0, VKBINDING(2, 1));


RES(CBUFFER(ObjectUniform), objectUniformBlock, UPDATE_FREQ_PER_DRAW, b0, VKBINDING(0, 3));

RES(CBUFFER(PerFrameVBConstantsData), PerFrameVBConstants, UPDATE_FREQ_PER_FRAME, b1, VKBINDING(1, 1));


RES(CBUFFER(VBConstantBufferData), VBConstantBuffer, UPDATE_FREQ_NONE, b2, VKBINDING(2, 0));

RES(Tex2D(float4), diffuseMaps[ 256U ], UPDATE_FREQ_NONE, t10, VKUNUSED(6));

RES(SamplerState, nearClampSampler, UPDATE_FREQ_NONE, s0, VKBINDING(5, 0));

float3 LoadVertexPositionFloat3(uint vtxIndex)
{
    return asfloat(LoadByte4(vertexDataBuffer, vtxIndex * 32)).xyz;
}

float4 LoadVertex(uint index)
{
    return float4(LoadVertexPositionFloat3(index), 1.0f);
}

float2 LoadVertexUVFloat2(uint vtxIndex)
{
    return asfloat(LoadByte2(vertexDataBuffer, vtxIndex * 32 + 12)).xy;
}

float2 LoadTexCoord(uint index)
{
    return LoadVertexUVFloat2(index);
}

STRUCT(PsInAlphaTested)
{
	DATA(float4, position, SV_Position);
	DATA(float2, texCoord, TEXCOORD0);
	DATA(FLAT(uint), materialID, TEXCOORD1);
};



PsInAlphaTested VS(uint vertexID : SV_VERTEXID)
{
	PsInAlphaTested Out;
    float4 vertexPos = LoadVertex(vertexID);

	Out.position = mul(objectUniformBlock.worldViewProjMat, vertexPos);
	Out.texCoord = LoadTexCoord(vertexID);

	Out.materialID = indirectDataBuffer[vertexID];

	return (Out);
}

void PS(PsInAlphaTested In )
{
    uint materialID = In.materialID;
	float4 texColor = f4(0);
#define materialID NonUniformResourceIndex(materialID)
    	texColor = SampleLvlTex2D(diffuseMaps[materialID], nearClampSampler, In.texCoord, 0);
#undef materialID

	clip(texColor.a < 0.5f ? -1 : 1);

	return;
}
