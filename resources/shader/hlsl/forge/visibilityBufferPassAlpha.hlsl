#include "base.hlsl"

STRUCT(PsInOpaque)
{
	DATA(float4, position, SV_Position);
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




RES(Buffer(uint), indirectDataBuffer, UPDATE_FREQ_PER_FRAME, t0, VKBINDING(2, 1));

RES(ByteBuffer, vertexDataBuffer, UPDATE_FREQ_NONE, t3, VKBINDING(3, 0));

RES(CBUFFER(VBConstantBufferData), VBConstantBuffer, UPDATE_FREQ_NONE, b2, VKBINDING(2, 0));


RES(Tex2D(float4), diffuseMaps[ 256U ], UPDATE_FREQ_NONE, t11, VKUNUSED(6));

RES(SamplerState, nearClampSampler, UPDATE_FREQ_NONE, s0, VKBINDING(5, 0));



STRUCT(ObjectUniform)
{
    float4x4 worldViewProjMat;
    uint viewID;
};
RES(CBUFFER(ObjectUniform), objectUniformBlock, UPDATE_FREQ_PER_DRAW, b0, VKBINDING(0, 3));

STRUCT(PsInAlphaTested)
{
	DATA(float4, position, SV_Position);
	DATA(float2, texCoord, TEXCOORD0);
	DATA(FLAT(uint), materialID, TEXCOORD1);
#line 64
};



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
    return asfloat(LoadByte2(vertexDataBuffer, vtxIndex * 32 + 24)).xy;
}

float2 LoadTexCoord(uint index)
{
    return LoadVertexUVFloat2(index);
}



float4 unpackUnorm4x8(uint p)
{
	return float4(float(p & 0x000000FF) / 255.0,
		float((p & 0x0000FF00) >> 8) / 255.0,
		float((p & 0x00FF0000) >> 16) / 255.0,
		float((p & 0xFF000000) >> 24) / 255.0);
}

PsInAlphaTested VS(uint vertexID : SV_VERTEXID)
{
    //INIT_MAIN;
    PsInAlphaTested Out;
    float4 vertexPos = LoadVertex(vertexID);

    Out.position = mul(objectUniformBlock.worldViewProjMat, vertexPos);
    Out.texCoord = LoadTexCoord(vertexID);

    Out.materialID = indirectDataBuffer[vertexID];

    return (Out);
}

float4 PS( PsInAlphaTested In, uint primitiveID : SV_PRIMITIVEID ): SV_TARGET
{
	//INIT_MAIN;
	float4 Out;

	uint materialID = In.materialID;
	float4 texColor = f4(0);
#define materialID NonUniformResourceIndex(materialID)
    	texColor = SampleLvlTex2D(diffuseMaps[materialID], nearClampSampler, In.texCoord, 0);
#undef materialID

	clip(texColor.a < 0.5f ? -1 : 1);
	Out = unpackUnorm4x8( (((( 1 ) & 0x00000003 ) << 30 ) | (((primitiveID) & 0x3FFFFFFF ) << 0 )) );
	return (Out);
}
