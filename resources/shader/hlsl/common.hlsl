//***************************************************************************************
// Common.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

// Defaults for number of lights.
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
	float4x4 viewProj;
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
Texture2D first        [[vk::binding(0, 1)]]: register(t0,space1);
Texture2D second       [[vk::binding(1, 1)]]: register(t1,space1);
Texture2D third        [[vk::binding(2, 1)]]: register(t2,space1);
Texture2D gShadowMap   [[vk::binding(3, 1)]]: register(t3,space1);
TextureCube gCubeMap   [[vk::binding(4, 1)]]: register(t4,space1);

#endif //PBR



SamplerState firstSampler       [[vk::binding(5, 1)]]: register(s0,space1);
SamplerState secondSampler      [[vk::binding(6, 1)]]: register(s1,space1);
SamplerState thirdSampler       [[vk::binding(7, 1)]]: register(s2,space1);
SamplerState shadowSampler      [[vk::binding(8, 1)]]: register(s3,space1);
SamplerState cubeSampler        [[vk::binding(9, 1)]]: register(s4,space1);

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
