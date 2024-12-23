//***************************************************************************************
// Common.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

// Defaults for number of lights.
#ifdef DIRECT3D12
#include "base.hlsl"
#else
#include "base.glsl"
#endif
#ifndef MAX_NUM_DIR_LIGHTS
    #define MAX_NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

struct Light {
    float3 Strength;
    float FalloffStart; // point/spot light only
    float3 Direction;   // directional/spot light only
    float FalloffEnd;   // point/spot light only
    float3 Position;    // point light only
    float SpotPower;    // spot light only
	f4x4 viewProj;
};

#ifdef PBR
Texture2D albedo_pbr: register(t0,space1);
Texture2D ao_pbr: register(t1,space1);
Texture2D normal_pbr: register(t2,space1);
Texture2D emissive_pbr: register(t3,space1);
Texture2D metal_roughness_pbr: register(t4,space1);
Texture2D roughness_pbr: register(t5,space1);
Texture2D brdflut: register(t6,space1);
TextureCube irradianceCube: register(t7,space1);
TextureCube prefilteredCube: register(t8,space1);
#else
Texture2D first: register(t0,space1);
Texture2D second: register(t1,space1);
Texture2D third: register(t2,space1);
Texture2D gShadowMap: register(t3,space1);
TextureCube gCubeMap: register(t4,space1);

#endif //PBR



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
    Light gDirLights[MAX_NUM_DIR_LIGHTS];
	uint numDirLights;
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
