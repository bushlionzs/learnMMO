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
    vec3 Strength;
    float FalloffStart; // point/spot light only
    vec3 Direction;   // directional/spot light only
    float FalloffEnd;   // point/spot light only
    vec3 Position;    // point light only
    float SpotPower;    // spot light only
	mat4 viewProj;
};


float textureProj(vec4 shadowCoord, sampler2D shadowMap,  vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st + off).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = 0.1;
		}
	}
	return shadow;
}

float calcShadowFactor(vec4 shadowPosH, sampler2D shadowMap, float shadowMapSize)
{
    // Complete projection by doing division by w.
    vec3 shadowPosNDC = shadowPosH.xyz / shadowPosH.w;

    // Depth in NDC space.
    float depth = shadowPosNDC.z;

    vec2 texelSize = vec2(1.0) / shadowMapSize;

    float percentLit = 0.0;
    vec2 offsets[9] = vec2[]
    (
        vec2(-texelSize.x,  -texelSize.y), vec2(0.0,  -texelSize.y), vec2(texelSize.x,  -texelSize.y),
        vec2(-texelSize.x, 0.0),           vec2(0.0, 0.0),           vec2(texelSize.x, 0.0),
        vec2(-texelSize.x,  texelSize.y), vec2(0.0,  texelSize.y), vec2(texelSize.x,  texelSize.y)
    );

    for(int i = 0; i < 9; ++i)
    {
        percentLit += textureProj(shadowPosH , shadowMap, offsets[i]);
    }

    return percentLit / 9.0;
}


layout(binding = 0, std140) uniform ObjectUniforms {
    mat4 gWorld;
    mat4 gProjector;
} cbPerObject;

layout(binding = 1, std140) uniform FrameUniforms {
    mat4 gView;
    mat4 gInvView;
    mat4 gProj;
    mat4 gInvProj;
    mat4 gViewProj;
    mat4 gInvViewProj;
	mat4 gShadowTransform;
    vec3 gEyePosW;
    uint gUseShadow;
    vec2 gRenderTargetSize;
    vec2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
	Light gDirLights[MAX_NUM_DIR_LIGHTS];
	uint numDirLights;
} cbPass;
#ifdef PBR
layout(binding = 2, std140) uniform MaterialUniforms {
    //some constance value;
    vec2 u_MetallicRoughnessValues;
	float u_OcclusionStrength;
	uint alpha_mask;
    vec3 u_EmissiveFactor;
	float alpha_mask_cutoff;
	vec4 u_BaseColorFactor;
	vec4 u_ScaleIBLAmbient;
	uint debugRenderMode;
	uint hasEmissiveMap;
    uint hasNormalMap;
    uint hasMetalRoughNessMap;
} pbrMaterial;
#else
layout(binding = 2, std140) uniform MaterialUniforms {
    vec4   gDiffuseAlbedo;
    vec3   gFresnelR0;
    float  gRoughness;
    mat4   gTexScale;
	mat4   gTexTransform;
	uint   gDiffuseMapIndex;
	uint   MatPad0;
	uint   MatPad1;
	uint   MatPad2;
} cbMaterial;
#endif// PBR

layout(binding = 3, std140) uniform SkinnedUniforms {
    mat4 gBoneTransforms[100];
} cbSkinned;

#ifdef PBR
layout (set=1, binding = 0) uniform sampler2D albedo_pbr;
layout (set=1, binding = 1) uniform sampler2D ao_pbr;
layout (set=1, binding = 2) uniform sampler2D normal_pbr;
layout (set=1, binding = 3) uniform sampler2D emissive_pbr;
layout (set=1, binding = 4) uniform sampler2D metal_roughness_pbr;
layout (set=1, binding = 5) uniform sampler2D roughness_pbr;
layout (set=1, binding = 6) uniform sampler2D brdflut;

layout (set=1, binding = 7) uniform samplerCube irradianceCube;
layout (set=1, binding = 8) uniform samplerCube prefilteredCube;
#else
layout(set=1, binding = 0) uniform sampler2D first;
layout (set=1, binding = 1) uniform sampler2D second;
layout (set=1, binding = 2) uniform sampler2D third;
layout (set=1, binding = 3) uniform sampler2D gShadowMap;
layout (set=1, binding = 4) uniform samplerCube gCubeMap;
layout(set=1, binding = 5) uniform sampler baseSampler;
#endif //PBR