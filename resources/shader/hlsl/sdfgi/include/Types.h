/*
* Copyright (c) 2019-2023, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef TYPES_H
#define TYPES_H



enum COMPOSITE_USE_FLAGS
{
	COMPOSITE_FLAG_USE_NONE = 0,
	COMPOSITE_FLAG_USE_RTAO = 0x1,
	COMPOSITE_FLAG_USE_DDGI = 0x2
};

enum COMPOSITE_SHOW_FLAGS
{
	COMPOSITE_FLAG_SHOW_NONE = 0,
	COMPOSITE_FLAG_SHOW_RTAO = 0x1,
	COMPOSITE_FLAG_SHOW_DDGI_INDIRECT = 0x2,
	COMPOSITE_FLAG_SHOW_DDGI_VOLUME_PROBES = 0x4,
	COMPOSITE_FLAG_SHOW_DDGI_VOLUME_TEXTURES = 0x8
};

enum POSTPROCESS_USE_FLAGS
{
	POSTPROCESS_FLAG_USE_NONE = 0,
	POSTPROCESS_FLAG_USE_EXPOSURE = 0x1,
	POSTPROCESS_FLAG_USE_TONEMAPPING = 0x2,
	POSTPROCESS_FLAG_USE_DITHER = 0x4,
	POSTPROCESS_FLAG_USE_GAMMA = 0x8,
};

struct Payload
{                                         // Byte Offset
	float3  albedo;                       // 12
	float   opacity;                      // 16
	float3  worldPosition;                // 28
	float   metallic;                     // 32
	float3  normal;                       // 44
	float   roughness;                    // 48
	float3  shadingNormal;                // 60
	float   hitT;                         // 64
	uint    hitKind;                      // 68
};

struct PackedPayload
{                                  // Byte Offset        Data Format
	float  hitT;                   // 0                  HitT
	float3 worldPosition;          // 4               X: World Position X
								   // 8               Y: World Position Y
								   // 12              Z: World Position Z
	uint4  packed0;                // 16              X: 16: Albedo R          16: Albedo G
								   //                 Y: 16: Albedo B          16: Normal X
								   //                 Z: 16: Normal Y          16: Normal Z
								   //                 W: 16: Metallic          16: Roughness
	uint3  packed1;                // 32              X: 16: ShadingNormal X   16: ShadingNormal Y
								   //                 Y: 16: ShadingNormal Z   16: Opacity
								   //                 Z: 16: Hit Kind          16: Unused
								   // 44
};

struct ProbeVisualizationPayload
{
	float  hitT;
	float3 worldPosition;
	int    instanceIndex;
	uint   volumeIndex;
	uint   instanceOffset;
};

struct MinimalPayload
{
	float3 radiance;
	float  hitT;
};

struct Vertex
{
	float3 position;
	float3 normal;
	float4 tangent;     // w stores bitangent direction
	float2 uv0;
};

struct GeometryData
{
	uint vertexOffset;
    uint indexOffset;
};

struct Camera
{
	float3 position;
	float  aspect;
	float3 up;
	float  fov;
	float3 right;
	float  tanHalfFovY;
	float3 forward;
	float  pad0;
	float2 resolution;
	float  pad1;
};

struct Light
{
	uint    type;                // 0: directional, 1: spot, 2: point (don't really need type on gpu with implicit placement)
	float3  direction;           // Directional / Spot
	float   power;
	float3  position;            // Spot / Point
	float   radius;              // Spot / Point
	float3  color;
	float   umbraAngle;          // Spot
	float   penumbraAngle;       // Spot
	float2  pad0;
};

struct Material
{
	float3 albedo;                  // RGB [0-1]
	float  opacity;                 // [0-1]
	float3 emissiveColor;           // RGB [0-1]
	float  roughness;               // [0-1]
	float  metallic;                // [0-1]
	int    alphaMode;               // 0: Opaque, 1: Blend, 2: Masked
	float  alphaCutoff;             // [0-1]
	int    doubleSided;             // 0: false, 1: true
	int    albedoTexIdx;            // RGBA [0-1]
	int    roughnessMetallicTexIdx; // R: Occlusion, G: Roughness, B: Metallic
	int    normalTexIdx;            // Tangent space XYZ
	int    emissiveTexIdx;          // RGB [0-1]
};

struct AppConsts
{
	uint   frameNumber;    // updated every frame, used for random number generation
	float3 skyRadiance;
};

struct PathTraceConsts
{
	float rayNormalBias;
	float rayViewBias;
	uint  numBounces;
	uint  samplesPerPixel;
};

struct LightingConsts
{
	uint hasDirectionalLight;   // -1: no directional light
	uint numPointLights;        // point lights start at index 1
	uint numSpotLights;         // spot lights start at 1 + numPointLights
	uint lightingPad0;
};

struct RTAOConsts
{
	float rayLength;
	float rayNormalBias;
	float rayViewBias;
	float power;
	float filterDistanceSigma;
	float filterDepthSigma;
	uint  filterBufferWidth;
	uint  filterBufferHeight;
	float filterDistKernel0;
	float filterDistKernel1;
	float filterDistKernel2;
	float filterDistKernel3;
	float filterDistKernel4;
	float filterDistKernel5;
};

struct CompositeConsts
{
	uint useFlags;
	uint showFlags;

};

struct PostProcessConsts
{
	uint  useFlags;
	float exposure;
};

struct DDGIVisConsts
{
	// Probe Visualization
	uint  instanceOffset;   // Offset of the current volume's sphere instances in the acceleration structure's TLAS instances
	uint  probeType;        // 0: irradiance | 1: distance
	float probeRadius;      // world-space value
	float distanceDivisor;  // divisor that normalizes the displayed distance values

	// Probe Textures Visualization
	float rayDataTextureScale;
	float irradianceTextureScale;
	float distanceTextureScale;
	float probeDataTextureScale;
	float probeVariabilityTextureScale;
	float probeVariabilityTextureThreshold;
};

struct GlobalConstants             // Added directly to the Root Signature (D3D12) or VkPipelineLayout Push Constants (Vulkan)
{
	// Note: although nested structs can be used in D3D12 root constants, they *may not* be used in Vulkan push constants.
	// Due to this constraint, we declare GPU-side global constants and provide accessor functions (in Descriptors.hlsl)
	// to distinguish these as global values.

	// App Constants
	uint   app_frameNumber;
	float3 app_skyRadiance;

	// Path Tracing Constants
	float  pt_rayNormalBias;
	float  pt_rayViewBias;
	uint   pt_numBounces;
	uint   pt_samplesPerPixel;

	// Lighting Constants
	uint   lighting_hasDirectionalLight;   // -1: no directional light
	uint   lighting_numPointLights;        // point lights start at index 1
	uint   lighting_numSpotLights;         // spot lights start at 1 + numPointLights
	uint   lighting_pad;

	// RTAO constants
	float  rtao_rayLength;
	float  rtao_rayNormalBias;
	float  rtao_rayViewBias;
	float  rtao_power;
	float  rtao_filterDistanceSigma;
	float  rtao_filterDepthSigma;
	uint   rtao_filterBufferWidth;
	uint   rtao_filterBufferHeight;
	float  rtao_filterDistKernel0;
	float  rtao_filterDistKernel1;
	float  rtao_filterDistKernel2;
	float  rtao_filterDistKernel3;
	float  rtao_filterDistKernel4;
	float  rtao_filterDistKernel5;
	uint2  rtao_pad;

	// Composite Constants
	uint   composite_useFlags;
	uint   composite_showFlags;
	uint2  composite_pad;

	// Post Process Constants
	uint   post_useFlags;
	float  post_exposure;
	uint2  post_pad;

	// DDGI Visualization Constants
	uint   ddgivis_instanceOffset;
	uint   ddgivis_probeType;
	float  ddgivis_probeRadius;
	float  ddgivis_distanceDivisor;
	float  ddgivis_rayDataTextureScale;
	float  ddgivis_irradianceTextureScale;
	float  ddgivis_distanceTextureScale;
	float  ddgivis_probeDataTextureScale;
	float  ddgivis_probeVariabilityTextureScale;
	float  ddgivis_probeVariabilityTextureThreshold;
	uint2  ddgivis_pad;

#ifdef __spirv__
	// DDGIRootConstants
	uint   ddgi_volumeIndex;
	uint2  ddgi_pad0;
	uint   ddgi_reductionInputSizeX;
	uint   ddgi_reductionInputSizeY;
	uint   ddgi_reductionInputSizeZ;
	uint2  ddgi_pad1;
#endif
};

#endif // TYPES_H
