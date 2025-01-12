#include "base.hlsl"
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
	float3 LightColor;
};

#ifdef VULKAN
#define VKBINDING(r,s) [[vk::binding(r, s)]]
#define VKLOCATION(l) [[vk::location(l)]]
#else
#define VKBINDING(r,s)
#define VKLOCATION(l)
#endif

#ifdef PBR
Texture2D albedo_pbr           VKBINDING(0, 1): register(t0,space1);
Texture2D ao_pbr               VKBINDING(1, 1): register(t1,space1);
Texture2D normal_pbr           VKBINDING(2, 1): register(t2,space1);
Texture2D emissive_pbr         VKBINDING(3, 1): register(t3,space1);
Texture2D metal_roughness_pbr  VKBINDING(4, 1): register(t4,space1);
Texture2D roughness_pbr        VKBINDING(5, 1): register(t5,space1);
Texture2D brdflut_pbr              VKBINDING(6, 1): register(t6,space1);
TextureCube irradianceCube     VKBINDING(7, 1): register(t7,space1);
TextureCube prefilteredCube    VKBINDING(8, 1): register(t8,space1);

SamplerState albedoSampler           VKBINDING(9, 1): register(s0,space1);
SamplerState aoSampler               VKBINDING(10, 1): register(s1,space1);
SamplerState normalSampler           VKBINDING(11, 1): register(s2,space1);
SamplerState emissiveSampler         VKBINDING(12, 1): register(s3,space1);
SamplerState metalRoughnessSampler   VKBINDING(13, 1): register(s4,space1);
SamplerState roughnessSampler        VKBINDING(14, 1): register(s5,space1);
SamplerState brdflutSampler          VKBINDING(15, 1): register(s6,space1);
SamplerState irradianceSampler       VKBINDING(16, 1): register(s7,space1);
SamplerState prefilteredSampler      VKBINDING(17, 1): register(s8,space1);
#else
Texture2D first        VKBINDING(0, 1): register(t0,space1);
Texture2D second       VKBINDING(1, 1): register(t1,space1);
Texture2D third        VKBINDING(2, 1): register(t2,space1);
Texture2D gShadowMap   VKBINDING(3, 1): register(t3,space1);
TextureCube gCubeMap   VKBINDING(4, 1): register(t4,space1);

SamplerState firstSampler       VKBINDING(5, 1): register(s0,space1);
SamplerState secondSampler      VKBINDING(6, 1): register(s1,space1);
SamplerState thirdSampler       VKBINDING(7, 1): register(s2,space1);
SamplerState shadowSampler      VKBINDING(8, 1): register(s3,space1);
SamplerState cubeSampler        VKBINDING(9, 1): register(s4,space1);
#endif //PBR

// Constant data that varies per frame.

struct ObjectBlock
{
    column_major  float4x4 gWorld;
	column_major float4x4 gProjector;
	float4 diffuseColor;
};

RES(CBUFFER(ObjectBlock), cbPerObject, UPDATE_FREQ_NONE, b0, VKBINDING(0, 0));

struct PassBlock
{
    column_major float4x4 gView;
    column_major float4x4 gInvView;
    column_major float4x4 gProj;
    column_major float4x4 gInvProj;
    column_major float4x4 gViewProj;
    column_major float4x4 gInvViewProj;
	column_major float4x4 gShadowTransform;
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

RES(CBUFFER(PassBlock), cbPass, UPDATE_FREQ_NONE, b1, VKBINDING(1, 0));


#ifdef PBR
struct PbrMaterialBlock
{
    float2 u_MetallicRoughnessValues;
	float u_OcclusionStrength;
	uint alpha_mask;
    float3 u_EmissiveFactor;
	float alpha_mask_cutoff;
	float4 u_BaseColorFactor;
	float4 u_ScaleIBLAmbient;
    uint debugRenderMode;
	uint hasEmissiveMap;
    uint hasNormalMap;
    uint hasMetalRoughNessMap;
};

RES(CBUFFER(PbrMaterialBlock), pbrMaterial, UPDATE_FREQ_NONE, b2, VKBINDING(2, 0));

#else
struct MaterialBlock
{
    float4   gDiffuseAlbedo;
    float3   gFresnelR0;
    float    gRoughness;
    column_major float4x4 gTexScale;
	column_major float4x4 gTexTransform;
	uint     gDiffuseMapIndex;
	uint     MatPad0;
	uint     MatPad1;
	uint     MatPad2;
};
RES(CBUFFER(MaterialBlock), cbMaterial, UPDATE_FREQ_NONE, b2, VKBINDING(2, 0));

#endif //PBR

#ifdef SKINNED
struct SkinBlock
{
    float4x4 gBoneTransforms[200];
};

RES(CBUFFER(MaterialBlock), cbSkinned, UPDATE_FREQ_NONE, b3, VKBINDING(3, 0));

#endif
