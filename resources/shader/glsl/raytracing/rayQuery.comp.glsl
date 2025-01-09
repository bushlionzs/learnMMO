#version 460 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_query : require
#extension GL_EXT_ray_flags_primitive_culling : require

#include "glslBase.glsl"


RES( accelerationStructureEXT , gRtScene, UPDATE_FREQ_NONE, t0, binding = 0);
layout (std430, UPDATE_FREQ_NONE, binding = 1) readonly buffer indices
{
	uint indices_data[];
};
layout (std430, UPDATE_FREQ_NONE, binding = 2) readonly buffer vertexDataBuffer
{
	uint vertexDataBuffer_data[];
};

layout (std430, UPDATE_FREQ_NONE, binding = 5) readonly buffer indexOffsets
{
	uint indexOffsets_data[];
};

RES(SamplerState, linearSampler, UPDATE_FREQ_NONE, s0, binding = 6);

RES(Tex2D(float4), materialTextures[ 256 ], UPDATE_FREQ_NONE, t7, binding = 7);

struct ShadersConfigBlock
{
	float4x4 mCameraToWorld;
	float4x4 mWorldToCamera;
	float4x4 mCameraToProjection;
	float4x4 mWorldToProjectionPrevious;
	float4x4 mWorldMatrix;
	float2 mRtInvSize;
	float2 mZ1PlaneSize;
	float mProjNear;
	float mProjFarMinusNear;
	float mRandomSeed;
	uint mFrameIndex;
	packed_float3 mLightDirection;
	uint mFramesSinceCameraMove;
	float2 mSubpixelJitter;
	uint mWidth;
	uint mHeight;
};

CBUFFER(gSettings, UPDATE_FREQ_PER_FRAME, b0, binding = 0)
{
	DATA(ShadersConfigBlock, gSettingsData, None);
};

layout(UPDATE_FREQ_NONE, binding = 8, rgba32f)  uniform RWTex2D(float4) gOutput;

float3 sampleCosineWeightedHemisphere(float2 u)
{
	float phi = 2.0f *  float(3.141592653589793)  * u.x;

	float sin_phi, cos_phi;
	sincos(phi, sin_phi, cos_phi);

	float cos_theta = sqrt(u.y);
	float sin_theta = sqrt(1.0f - cos_theta * cos_theta);

	return float3(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi);
}



float3 alignHemisphereWithNormal(float3 sampleVal, float3 normal)
{
	float3 up = normal;
	float3 right = normalize(cross(normal, float3(0.0072f, 1.0f, 0.0034f)));
	float3 forward = cross(right, up);
	return sampleVal.x * right + sampleVal.y * up + sampleVal.z * forward;
}

float PDnrand( float2 n ) {
	return frac( sin(dot(n.xy, float2(12.9898, 78.233f)))* 43758.5453 );
}
float2 PDnrand2( float2 n ) {
	return frac( sin(dot(n.xy, float2(12.9898, 78.233f)))* float2(43758.5453, 28001.8384) );
}
float3 PDnrand3( float2 n ) {
	return frac( sin(dot(n.xy, float2(12.9898, 78.233f)))* float3(43758.5453, 28001.8384, 50849.4141 ) );
}
float4 PDnrand4( float2 n ) {
	return frac( sin(dot(n.xy, float2(12.9898, 78.233f)))* float4(43758.5453, 28001.8384, 50849.4141, 12996.89) );
}


float2 uniformNoiseToTriangular(float2 n) {
	float2 orig = n*2.0-1.0;
	n = orig*rsqrt(abs(orig));
	n = max(f2(-1.0), n);
	n = n-float2(sign(orig));
	return n;
}

float3 LoadVertexPositionFloat3(uint vtxIndex)
{
    return asfloat(LoadByte4(vertexDataBuffer_data, vtxIndex * 32)).xyz;
}

float4 LoadVertex(uint index)
{
    return float4(LoadVertexPositionFloat3(index), 1.0f);
}

float2 LoadVertexUVFloat2(uint vtxIndex)
{
    return asfloat(LoadByte2(vertexDataBuffer_data, vtxIndex * 32 + 24)).xy;
}

float2 LoadTexCoord(uint index)
{
    return LoadVertexUVFloat2(index);
}

float3 LoadVertexNormalFloat3(uint vtxIndex)
{
    return asfloat(LoadByte4(vertexDataBuffer_data, vtxIndex * 32 + 12)).xyz;
}

float3 LoadNormal(uint index)
{
    return LoadVertexNormalFloat3(index);
}

NUM_THREADS(8, 8, 1)
void main()
{
	const uint2 tid = uint2(gl_GlobalInvocationID);

	if (tid.x >= Get(gSettingsData).mWidth || tid.y >= Get(gSettingsData).mHeight)
	{
		return;
	}

	uint2 launchDim = uint2(Get(gSettingsData).mWidth, Get(gSettingsData).mHeight);

	float2 crd = float2(tid) + Get(gSettingsData).mSubpixelJitter;
	float2 dims = float2(launchDim);

	float2 d = ((crd / dims) * 2.f - 1.f);
	d.y *= -1;

	float3 direction = normalize(float3(d * Get(gSettingsData).mZ1PlaneSize, 1.0));
	float3 directionWS = mul(float3x3(Get(gSettingsData).mCameraToWorld[0].xyz, Get(gSettingsData).mCameraToWorld[1].xyz, Get(gSettingsData).mCameraToWorld[2].xyz), direction);

	RayDesc ray;
	ray.TMin = 0.001;
	ray.TMax = 10000;
	ray.Direction = directionWS;
	ray.Origin = getCol(Get(gSettingsData).mCameraToWorld, 3).xyz + directionWS * Get(gSettingsData).mProjNear;

	const uint maxBounce = 2;
	float3 surfaceAlbedo[maxBounce];
	float3 bounceRadiance[maxBounce];

	UNROLL
	for (uint b = 0; b < maxBounce; ++b)
	{
		bounceRadiance[b] = f3(0.0f);
		surfaceAlbedo[b] = f3(0.0f);
	}

	const  uint  traversalFlags =  0  |  0  |  0 ;

	float4x4 worldMatrix = Get(gSettingsData).mWorldMatrix;

	UNROLL
	for (uint bounce = 0; bounce < maxBounce; ++bounce)
	{
		surfaceAlbedo[bounce] = f3(0.0f);
		bounceRadiance[bounce] = f3(0.0f);

        float3 uvw;
		float3 position;
		float3 normal;
		float2 uv;

		uint geometryIndex;
		uint triangleIndex;
		uint indexOffset;
        float2 barycentricCoords;

		uint i0;
        uint i1;
        uint i2;

		float4 matColor;

        rayQueryEXT hit; rayQueryInitializeEXT(hit, Get(gRtScene), gl_RayFlagsSkipAABBEXT , 0xFF, ray.Origin, ray.TMin, ray.Direction, ray.TMax); bool hitHasHitCandidates = rayQueryProceedEXT(hit);
		LOOP
		while (hitHasHitCandidates )
		{
			if ( (rayQueryGetIntersectionTypeEXT(hit, false) == gl_RayQueryCandidateIntersectionTriangleEXT) )
            {
				geometryIndex =  (rayQueryGetIntersectionGeometryIndexEXT(hit, false)) ;
                triangleIndex =  (rayQueryGetIntersectionPrimitiveIndexEXT(hit, false)) ;
                barycentricCoords =  (rayQueryGetIntersectionBarycentricsEXT(hit, false)) ;
				indexOffset = LoadByte(indexOffsets_data, geometryIndex << 2);


                uvw.yz = barycentricCoords;
                uvw.x = 1.0f - uvw.y - uvw.z;

                uint4 i012 = LoadByte4(indices_data, ((3 * triangleIndex + indexOffset) << 2));
                i0 = i012[0];
                i1 = i012[1];
                i2 = i012[2];

                float2 uv012[3] = {
					LoadTexCoord(i0),
				    LoadTexCoord(i1),
				    LoadTexCoord(i2)
				};

                uv = uvw.x * uv012[0] + uvw.y * uv012[1] + uvw.z * uv012[2];

#define CASE_LIST REPEAT_HUNDRED(0) REPEAT_HUNDRED(100) REPEAT_TEN(200) REPEAT_TEN(210) REPEAT_TEN(220) REPEAT_TEN(230) REPEAT_TEN(240) CASE(250) CASE(251) CASE(252) CASE(253) CASE(254) CASE(255) 
#define NonUniformResourceIndexBlock(geometryIndex) \
                matColor = SampleLvlTex2D(Get(materialTextures)[geometryIndex], Get(linearSampler), uv, 0).rgba; \

#if VK_EXT_DESCRIPTOR_INDEXING_ENABLED
	NonUniformResourceIndexBlock(nonuniformEXT(geometryIndex))
#elif VK_FEATURE_TEXTURE_ARRAY_DYNAMIC_INDEXING_ENABLED
	NonUniformResourceIndexBlock(geometryIndex)
#else
#define CASE(id) case id: NonUniformResourceIndexBlock(id) break;
	switch(geometryIndex) {CASE_LIST};
#undef CASE
#endif
#undef NonUniformResourceIndexBlock
#undef CASE_LIST

                if (matColor.a > 0.5)
                {
                    rayQueryConfirmIntersectionEXT(hit); ;
                }
            }

			hitHasHitCandidates = rayQueryProceedEXT(hit); ;
        }
		;

        if (! (rayQueryGetIntersectionTypeEXT(hit, true) != gl_RayQueryCommittedIntersectionNoneEXT) )
        {
            float3 skyColor = float3(0.3f, 0.6f, 1.2f);
            surfaceAlbedo[bounce] = f3(1.0f);
            bounceRadiance[bounce] = skyColor;
            break;
        }

		uint hitGeometryIndex =  (rayQueryGetIntersectionGeometryIndexEXT(hit, true)) ;
		uint hitTriangleIndex =  (rayQueryGetIntersectionPrimitiveIndexEXT(hit, true)) ;
		float2 hitBarycentricCoords =  (rayQueryGetIntersectionBarycentricsEXT(hit, true)) ;
		if (hitGeometryIndex != geometryIndex || hitTriangleIndex != triangleIndex || hitBarycentricCoords.x != barycentricCoords.x || hitBarycentricCoords.y != barycentricCoords.y)
		{
			triangleIndex = hitTriangleIndex;
			barycentricCoords = hitBarycentricCoords;
			geometryIndex = hitGeometryIndex;
			indexOffset = LoadByte(indexOffsets_data, geometryIndex << 2);

		    uvw.yz = barycentricCoords;
            uvw.x = 1.0f - uvw.y - uvw.z;

            uint4 i012 = LoadByte4(indices_data, ((3 * triangleIndex + indexOffset) << 2));
            i0 = i012[0];
            i1 = i012[1];
            i2 = i012[2];

            float2 uv012[3] = {
				LoadTexCoord(i0),
				LoadTexCoord(i1),
				LoadTexCoord(i2)
			};

            uv = uvw.x * uv012[0] + uvw.y * uv012[1] + uvw.z * uv012[2];

#define CASE_LIST REPEAT_HUNDRED(0) REPEAT_HUNDRED(100) REPEAT_TEN(200) REPEAT_TEN(210) REPEAT_TEN(220) REPEAT_TEN(230) REPEAT_TEN(240) CASE(250) CASE(251) CASE(252) CASE(253) CASE(254) CASE(255) 
#define NonUniformResourceIndexBlock(geometryIndex) \
            matColor = SampleLvlTex2D(Get(materialTextures)[geometryIndex], Get(linearSampler), uv, 0).rgba; \

#if VK_EXT_DESCRIPTOR_INDEXING_ENABLED
	NonUniformResourceIndexBlock(nonuniformEXT(geometryIndex))
#elif VK_FEATURE_TEXTURE_ARRAY_DYNAMIC_INDEXING_ENABLED
	NonUniformResourceIndexBlock(geometryIndex)
#else
#define CASE(id) case id: NonUniformResourceIndexBlock(id) break;
	switch(geometryIndex) {CASE_LIST};
#undef CASE
#endif
#undef NonUniformResourceIndexBlock
#undef CASE_LIST
		}

        float4 position012[3] =
		{
			LoadVertex(i0 * 3),
			LoadVertex(i1 * 3),
			LoadVertex(i2 * 3)
		};
		float3 normal012[3] = {
			LoadNormal(i0),
		    LoadNormal(i1),
		    LoadNormal(i2)
		};
        position = uvw.x * position012[0].xyz + uvw.y * position012[1].xyz + uvw.z * position012[2].xyz;
        position = mul(worldMatrix, float4(position, 1.0f)).xyz;
        normal = normalize(uvw.x * normal012[0].xyz + uvw.y * normal012[1].xyz + uvw.z * normal012[2].xyz);
        surfaceAlbedo[bounce] = matColor.rgb;

		float3 rayOrigin = position + normal * 0.001;


		{
			float3 lightDir = Get(gSettingsData).mLightDirection;

			RayDesc shadowRay;
			shadowRay.Origin = rayOrigin;
			shadowRay.Direction = lightDir;
			shadowRay.TMin = 0.001;
			shadowRay.TMax = 10000;

			rayQueryEXT shadowHit; rayQueryInitializeEXT(shadowHit, Get(gRtScene), gl_RayFlagsSkipAABBEXT | gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsTerminateOnFirstHitEXT , 0xFF, shadowRay.Origin, shadowRay.TMin, shadowRay.Direction, shadowRay.TMax); bool shadowHitHasHitCandidates = rayQueryProceedEXT(shadowHit);
			LOOP
			while (shadowHitHasHitCandidates )
			{
				if ( (rayQueryGetIntersectionTypeEXT(shadowHit, false) == gl_RayQueryCandidateIntersectionTriangleEXT) )
				{
					uint triangleIndex =  (rayQueryGetIntersectionPrimitiveIndexEXT(shadowHit, false)) ;
					float2 barycentricCoords =  (rayQueryGetIntersectionBarycentricsEXT(shadowHit, false)) ;
					uint geometryIndex =  (rayQueryGetIntersectionGeometryIndexEXT(shadowHit, false)) ;
					uint indexOffset = LoadByte(indexOffsets_data, geometryIndex << 2);


					float3 uvw;
					uvw.yz = barycentricCoords;
					uvw.x = 1.0f - uvw.y - uvw.z;

					uint4 i012 = LoadByte4(indices_data, ((3 * triangleIndex + indexOffset) << 2));
					uint i0 = i012[0];
					uint i1 = i012[1];
					uint i2 = i012[2];

					float2 uv012[3] = {
						LoadTexCoord(i0),
				        LoadTexCoord(i1),
				        LoadTexCoord(i2)
					};

					float2 uv = uvw.x * uv012[0] + uvw.y * uv012[1] + uvw.z * uv012[2];

					float alpha;
#define CASE_LIST REPEAT_HUNDRED(0) REPEAT_HUNDRED(100) REPEAT_TEN(200) REPEAT_TEN(210) REPEAT_TEN(220) REPEAT_TEN(230) REPEAT_TEN(240) CASE(250) CASE(251) CASE(252) CASE(253) CASE(254) CASE(255) 
#define NonUniformResourceIndexBlock(geometryIndex) \
					alpha = SampleLvlTex2D(Get(materialTextures)[geometryIndex], Get(linearSampler), uv, 0).a; \

#if VK_EXT_DESCRIPTOR_INDEXING_ENABLED
	NonUniformResourceIndexBlock(nonuniformEXT(geometryIndex))
#elif VK_FEATURE_TEXTURE_ARRAY_DYNAMIC_INDEXING_ENABLED
	NonUniformResourceIndexBlock(geometryIndex)
#else
#define CASE(id) case id: NonUniformResourceIndexBlock(id) break;
	switch(geometryIndex) {CASE_LIST};
#undef CASE
#endif
#undef NonUniformResourceIndexBlock
#undef CASE_LIST

					if (alpha > 0.5)
					{
						rayQueryConfirmIntersectionEXT(shadowHit); ;
					}
				}

				shadowHitHasHitCandidates = rayQueryProceedEXT(shadowHit); ;
			}


			if (! (rayQueryGetIntersectionTypeEXT(shadowHit, true) != gl_RayQueryCommittedIntersectionNoneEXT) )
			{
				float3 lightSample = 5.0f * saturate(dot(lightDir, normal)) * surfaceAlbedo[bounce] *  float(0.318309886183790) ;
				bounceRadiance[bounce] += lightSample;
			}
		}

		float2 pixelUV = (float2(tid) + f2(0.5)) / float2(launchDim.xy);
		float2 sampleVal = uniformNoiseToTriangular(PDnrand2(pixelUV.xy + Get(gSettingsData).mRandomSeed + bounce)) * 0.5 + 0.5;
		float3 sampleDirLocal = sampleCosineWeightedHemisphere(sampleVal);
		float3 sampleDir = alignHemisphereWithNormal(sampleDirLocal, normal);

		ray.Origin = rayOrigin;
		ray.Direction = sampleDir;
	}

	UNROLL
	for (int i = int(maxBounce) - 2; i >= 0; --i)
	{
		bounceRadiance[i] += bounceRadiance[i + 1] * surfaceAlbedo[i];
	}

	float3 payloadRadiance = bounceRadiance[0];

	float4 accumulatedRadiance = f4(0.0f);
	if (Get(gSettingsData).mFrameIndex == 0)
	{
		accumulatedRadiance = float4(payloadRadiance, 1.0);
	}
	else
	{
		accumulatedRadiance = LoadRWTex2D(Get(gOutput), tid);

		accumulatedRadiance.w += 1.0;
		accumulatedRadiance.rgb += (payloadRadiance - accumulatedRadiance.rgb) / accumulatedRadiance.w;
	}

	Write2D(Get(gOutput), tid, accumulatedRadiance);
}