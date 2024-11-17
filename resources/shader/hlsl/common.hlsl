//***************************************************************************************
// Common.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightingUtil.hlsl"

TextureCube gCubeMap[2] : register(t0);
Texture2D gShadowMap : register(t2);
// An array of textures, which is only supported in shader model 5.1+.  Unlike Texture2DArray, the textures
// in this array can be different sizes and formats, making it more flexible than texture arrays.
Texture2D gTextureArray[9] : register(t3);



SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
	float4x4 gProjector;
	float4x4 gWorldViewProj;
};

// Constant data that varies per material.
cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
	float4x4 gShadowTransform;
    float3 gEyePosW;
    uint gShadow;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;


    Light gDirectionLights[MaxLightsEach];
	Light gPointLights[MaxLightsEach];
	Light gSpotLights[MaxLightsEach];
	
};

#ifdef PBR
cbuffer pbrMaterial : register(b2)
{
    //some constance value;
    float2 u_MetallicRoughnessValues;
	float u_OcclusionStrength;
	float pad1;
    float3 u_EmissiveFactor;
	float pad2;
	float4 u_BaseColorFactor;
	float4 u_ScaleIBLAmbient;
    float4x4 gTexScale;
	float4x4 gTexTransform;
};
#else
cbuffer cbMaterial : register(b2)
{
	float4   gDiffuseAlbedo;
    float3   gFresnelR0;
    float    gRoughness;
    float4x4 gTexScale;
	float4x4 gTexTransform;
	uint     gDiffuseMapIndex;
	uint     MatPad0;
	uint     MatPad1;
	uint     MatPad2;
};
#endif //PBR

#ifdef SKINNED
cbuffer cbSkinned : register(b3)
{
    float4x4 gBoneTransforms[200];
};
#endif

float4 ComputeLighting(Material mat,
                       float3 pos, float3 normal, float3 toEye,
                       float3 shadowFactor)
{
    float3 result = 0.0f;

    int i = 0;

#if (NUM_DIR_LIGHTS > 0)
    for(i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        result += shadowFactor[i] * ComputeDirectionalLight(gDirectionLights[i], mat, normal, toEye);
    }
#endif

#if (NUM_POINT_LIGHTS > 0)
    for(i = 0; i < NUM_POINT_LIGHTS; ++i)
    {
        result += ComputePointLight(gPointLights[i], mat, pos, normal, toEye);
    }
#endif

#if (NUM_SPOT_LIGHTS > 0)
    for(i = 0; i < 0NUM_SPOT_LIGHTS; ++i)
    {
        result += ComputeSpotLight(gSpotLights[i], mat, pos, normal, toEye);
    }
#endif 

    return float4(result, 0.0f);
}

float CalcShadowFactor(float4 shadowPosH)
{
    // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;

    // Depth in NDC space.
    float depth = shadowPosH.z;
	

    uint width, height, numMips;
    gShadowMap.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float)width;
	

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    };
	

    [unroll]
    for(int i = 0; i < 9; ++i)
    {
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
            shadowPosH.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
}
