/*
* Copyright (c) 2019-2023, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef RTXGI_DDGI_COMMON_HLSL
#define RTXGI_DDGI_COMMON_HLSL

#include "ddgiCommon.hlsl"
#include "Platform.hlsl"
#include "Defines.h"
#include "DDGIRootConstants.h"
#include "DDGIVolumeDescGPU.h"

//------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------

// Bindless resource implementation type
#define RTXGI_BINDLESS_TYPE_RESOURCE_ARRAYS 0
#define RTXGI_BINDLESS_TYPE_DESCRIPTOR_HEAP 1

// Texture formats (matches EDDGIVolumeTextureFormat)
#define RTXGI_DDGI_VOLUME_TEXTURE_FORMAT_U32 0
#define RTXGI_DDGI_VOLUME_TEXTURE_FORMAT_F16 1
#define RTXGI_DDGI_VOLUME_TEXTURE_FORMAT_F16x2 2
#define RTXGI_DDGI_VOLUME_TEXTURE_FORMAT_F16x4 3
#define RTXGI_DDGI_VOLUME_TEXTURE_FORMAT_F32 4
#define RTXGI_DDGI_VOLUME_TEXTURE_FORMAT_F32x2 5
#define RTXGI_DDGI_VOLUME_TEXTURE_FORMAT_F32x4 6

// The number of fixed rays that are used by probe relocation and classification.
// These rays directions are always the same to produce temporally stable results.
#define RTXGI_DDGI_NUM_FIXED_RAYS 32

// Probe classification states
#define RTXGI_DDGI_PROBE_STATE_ACTIVE 0     // probe shoots rays and may be sampled by a front facing surface or another probe (recursive irradiance)
#define RTXGI_DDGI_PROBE_STATE_INACTIVE 1   // probe doesn't need to shoot rays, it isn't near a front facing surface

// Volume movement types
#define RTXGI_DDGI_VOLUME_MOVEMENT_TYPE_DEFAULT 0
#define RTXGI_DDGI_VOLUME_MOVEMENT_TYPE_SCROLLING 1

//------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------

bool IsVolumeMovementScrolling(DDGIVolumeDescGPU volume)
{
    return (volume.movementType == RTXGI_DDGI_VOLUME_MOVEMENT_TYPE_SCROLLING);
}


static const float RTXGI_PI = 3.1415926535897932f;
static const float RTXGI_2PI = 6.2831853071795864f;

//------------------------------------------------------------------------
// Math Helpers
//------------------------------------------------------------------------

/**
 * Returns the largest component of the vector.
 */
float RTXGIMaxComponent(float3 a)
{
    return max(a.x, max(a.y, a.z));
}

/**
 * Returns either -1 or 1 based on the sign of the input value.
 * If the input is zero, 1 is returned.
 */
float RTXGISignNotZero(float v)
{
    return (v >= 0.f) ? 1.f : -1.f;
}

/**
 * 2-component version of RTXGISignNotZero.
 */
float2 RTXGISignNotZero(float2 v)
{
    return float2(RTXGISignNotZero(v.x), RTXGISignNotZero(v.y));
}

//------------------------------------------------------------------------
// Sampling Helpers
//------------------------------------------------------------------------

/**
 * Computes a low discrepancy spherically distributed direction on the unit sphere,
 * for the given index in a set of samples. Each direction is unique in
 * the set, but the set of directions is always the same.
 */
float3 RTXGISphericalFibonacci(float sampleIndex, float numSamples)
{
    const float b = (sqrt(5.f) * 0.5f + 0.5f) - 1.f;
    float phi = RTXGI_2PI * frac(sampleIndex * b);
    float cosTheta = 1.f - (2.f * sampleIndex + 1.f) * (1.f / numSamples);
    float sinTheta = sqrt(saturate(1.f - (cosTheta * cosTheta)));

    return float3((cos(phi) * sinTheta), (sin(phi) * sinTheta), cosTheta);
}

//------------------------------------------------------------------------
// Format Conversion Helpers
//------------------------------------------------------------------------

/**
 * Return the given float value as an unsigned integer within the given numerical scale.
 */
uint RTXGIFloatToUint(float v, float scale)
{
    return (uint)floor(v * scale + 0.5f);
}

/**
 * Pack a float3 into a 32-bit unsigned integer.
 * All channels use 10 bits and 2 bits are unused.
 * Compliment of RTXGIUintToFloat3().
 */
uint RTXGIFloat3ToUint(float3 input)
{
    return (RTXGIFloatToUint(input.r, 1023.f)) | (RTXGIFloatToUint(input.g, 1023.f) << 10) | (RTXGIFloatToUint(input.b, 1023.f) << 20);
}

/**
 * Unpack a packed 32-bit unsigned integer to a float3.
 * Compliment of RTXGIFloat3ToUint().
 */
float3 RTXGIUintToFloat3(uint input)
{
    float3 output;
    output.x = (float)(input & 0x000003FF) / 1023.f;
    output.y = (float)((input >> 10) & 0x000003FF) / 1023.f;
    output.z = (float)((input >> 20) & 0x000003FF) / 1023.f;
    return output;
}

//------------------------------------------------------------------------
// Quaternion Helpers
//------------------------------------------------------------------------

/**
 * Rotate vector v with quaternion q.
 */
float3 RTXGIQuaternionRotate(float3 v, float4 q)
{
    float3 b = q.xyz;
    float b2 = dot(b, b);
    return (v * (q.w * q.w - b2) + b * (dot(v, b) * 2.f) + cross(b, v) * (q.w * 2.f));
}

/**
 * Quaternion conjugate.
 * For unit quaternions, conjugate equals inverse.
 * Use this to create a quaternion that rotates in the opposite direction.
 */
float4 RTXGIQuaternionConjugate(float4 q)
{
    return float4(-q.xyz, q.w);
}

//------------------------------------------------------------------------
// Luminance Helper
//------------------------------------------------------------------------

/**
 * Convert Linear RGB value to Luminance
 */
float RTXGILinearRGBToLuminance(float3 rgb)
{
    const float3 LuminanceWeights = float3(0.2126, 0.7152, 0.0722);
    return dot(rgb, LuminanceWeights);
}
#endif // RTXGI_DDGI_COMMON_HLSL
