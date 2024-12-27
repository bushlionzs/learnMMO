#version 450 core
#extension GL_GOOGLE_include_directive : require
#include "glslBase.glsl"
#ifdef VERTEX_SHADER

layout(location = 0) out vec2 outScreenPos;
void main()
{
    const uint vertexID = uint(gl_VertexIndex);
	
	vec4 position;
	
	position.x = (vertexID == 2 ? 3.0 : -1.0);
	position.y = (vertexID == 0 ? -3.0 : 1.0);
	position.zw = vec2(0, 1);
	gl_Position = position;
    outScreenPos = position.xy;
}
#endif //VERTEX_SHADER

#ifdef FRAGMENT_SHADER
RES(Tex2D(float4), IndexTexture[10], UPDATE_FREQ_NONE, t20, binding = 20);
RES(Depth2D(float), DepthAtlasTexture, UPDATE_FREQ_NONE, t30, binding = 21);
RES(Tex2D(float), PrerenderLodClampTexture, UPDATE_FREQ_NONE, t32, binding = 23);
RES(Tex2D(float), ESMShadowTexture, UPDATE_FREQ_NONE, t33, binding = 24);
RES(Tex2D(float4), vbPassTexture, UPDATE_FREQ_NONE, t18, binding = 18);
//RES(Tex2D(uint), ScreenSpaceShadowTexture, UPDATE_FREQ_NONE, t36, binding = 33);
RES(Tex2D(float4), diffuseMaps[ 256U ], UPDATE_FREQ_NONE, t40, binding = 25);
RES(Tex2D(float4), normalMaps[ 256U ], UPDATE_FREQ_NONE, t296, binding = 26);
RES(Tex2D(float4), specularMaps[ 256U ], UPDATE_FREQ_NONE, t552, binding = 27);

RES(SamplerComparisonState, ShadowCmpSampler, UPDATE_FREQ_NONE, s4, binding = 11);
RES(SamplerState, clampBorderNearSampler, UPDATE_FREQ_NONE, s3, binding = 10);
RES(SamplerState, clampMiplessLinearSampler, UPDATE_FREQ_NONE, s2, binding = 8);
RES(SamplerState, clampMiplessNearSampler, UPDATE_FREQ_NONE, s1, binding = 9);
RES(SamplerState, textureSampler, UPDATE_FREQ_NONE, s0, binding = 7);

CBUFFER(VBConstantBufferStruct, UPDATE_FREQ_NONE, b2, binding = 2)
{
    DATA(VBConstants, vbConstant[ 2 ], None);
}VBConstantBuffer;

layout (std430, UPDATE_FREQ_NONE, binding = 35) readonly buffer vertexDataBufferStruct
{
	uint vertexDataBuffer_data[];
}vertexDataBuffer;

layout (std430, UPDATE_FREQ_NONE, binding = 6) readonly buffer meshConstantsBufferStruct
{
	MeshConstants meshConstantsBuffer_data[];
}meshConstantsBuffer;

CBUFFER(PerFrameVBConstantsStruct, UPDATE_FREQ_PER_FRAME, b1, binding = 1)
{
	DATA(Transform, transform[ 5 ], None);
	DATA(CullingViewPort, cullingViewports[ 5 ], None);
	DATA(uint, numViewports, None);
}PerFrameVBConstants;


CBUFFER(lightUniformBlockStruct, UPDATE_FREQ_PER_FRAME, b1, binding = 15)
{
    DATA(float4x4, mLightViewProj, None);
    DATA(float4, lightPosition, None);
    DATA(float4, lightColor, None);
	DATA(float4, mLightUpVec, None);
	DATA(float4, mTanLightAngleAndThresholdValue, None);
	DATA(float3, mLightDir, None);
}lightUniformBlock;

CBUFFER(cameraUniformBlockStruct, UPDATE_FREQ_PER_FRAME, b3, binding = 14)
{
    DATA(float4x4, View, None);
    DATA(float4x4, Project, None);
    DATA(float4x4, ViewProject, None);
    DATA(float4x4, InvView, None);
	DATA(float4x4, InvProj, None);
	DATA(float4x4, InvViewProject, None);
	DATA(float4, mCameraPos, None);
	DATA(float, mNear, None);
	DATA(float, mFar, None);
	DATA(float, mFarNearDiff, None);
	DATA(float, mFarNear, None);
	DATA(float2, mTwoOverRes, None);
	DATA(float2, mWindowSize, None);
	DATA(float4, mDeviceZToWorldZ, None);
}cameraUniformBlock;

CBUFFER(objectUniformBlockStruct, UPDATE_FREQ_PER_FRAME, b0, binding = 13)
{
    DATA(float4x4, WorldViewProjMat, None);
}objectUniformBlock;

CBUFFER(renderSettingUniformBlockStruct, UPDATE_FREQ_PER_FRAME, b4, binding = 16)
{
    DATA(float4, WindowDimension, None);
    DATA(int, ShadowType, None);
}renderSettingUniformBlock;

CBUFFER(ESMInputConstantsStruct, UPDATE_FREQ_PER_FRAME, b5, binding = 17)
{
    DATA(float, mEsmControl, None);
}ESMInputConstants;

CBUFFER(SSSEnabledStruct, UPDATE_FREQ_PER_FRAME, b8, binding = 19)
{
	DATA(uint, mSSSEnabled, None);
}SSSEnabled;





layout (std430, UPDATE_FREQ_PER_FRAME, binding = 4) readonly buffer filteredIndexBufferStruct
{
	uint filteredIndexBuffer_data[];
}filteredIndexBuffer;

layout (std430, UPDATE_FREQ_PER_FRAME, binding = 5) readonly buffer indirectDataBufferStruct
{
	uint indirectDataBuffer_data[];
}indirectDataBuffer;


#include "glslFunc.glsl"

float3 LoadVertexPositionFloat3(uint vtxIndex)
{
    return asfloat(LoadByte4(vertexDataBuffer.vertexDataBuffer_data, vtxIndex * 32)).xyz;
}

float3 LoadVertex(uint index)
{
    return LoadVertexPositionFloat3(index);
}

float2 LoadVertexUVFloat2(uint vtxIndex)
{
    return asfloat(LoadByte2(vertexDataBuffer.vertexDataBuffer_data, vtxIndex * 32 + 24)).xy;
}

float2 LoadTexCoord(uint index)
{
    return LoadVertexUVFloat2(index);
}

float3 LoadVertexNormalFloat3(uint vtxIndex)
{
    return asfloat(LoadByte4(vertexDataBuffer.vertexDataBuffer_data, vtxIndex * 32 + 12)).xyz;
}

float3 LoadNormal(uint index)
{
    return LoadVertexNormalFloat3(index);
}

STRUCT(PsIn)
{
	DATA(float4, Position, SV_Position);
	DATA(float2, ScreenPos, TEXCOORD0);
};

layout(location = 0) in float2 inScreenPos;
layout (location = 0) out float4 outColor;
void main()
{
    PsIn In;
	In.Position = float4(float4(gl_FragCoord.xyz, 1.0f / gl_FragCoord.w));
	In.ScreenPos = inScreenPos;
    const uint i = uint(gl_SampleID);
	
	float4 visRaw = LoadTex2D(vbPassTexture, clampMiplessLinearSampler, uint2(In.Position.xy), 0);
	
	uint geomSetPrimID = packUnorm4x8(visRaw);
	
	if (geomSetPrimID == ~0u)
	{
		discard;
	}
	
	uint primitiveID = (geomSetPrimID >>  0 ) &  0x3FFFFFFF ;
	uint geomSet = (geomSetPrimID >>  30 ) &  0x00000003 ;

	uint triIdx0 =  ( VBConstantBuffer.vbConstant[geomSet].indexOffset )  + (primitiveID * 3 + 0);
	uint triIdx1 =  ( VBConstantBuffer.vbConstant[geomSet].indexOffset )  + (primitiveID * 3 + 1);
	uint triIdx2 =  ( VBConstantBuffer.vbConstant[geomSet].indexOffset )  + (primitiveID * 3 + 2);

	uint index0 = LoadByte(filteredIndexBuffer.filteredIndexBuffer_data, triIdx0 << 2);
	uint index1 = LoadByte(filteredIndexBuffer.filteredIndexBuffer_data, triIdx1 << 2);
	uint index2 = LoadByte(filteredIndexBuffer.filteredIndexBuffer_data, triIdx2 << 2);
	

	float3 v0pos = LoadVertex(index0);
	float3 v1pos = LoadVertex(index1);
	float3 v2pos = LoadVertex(index2);

	float4 pos0 = mul(objectUniformBlock.WorldViewProjMat, float4(v0pos, 1));
	float4 pos1 = mul(objectUniformBlock.WorldViewProjMat, float4(v1pos, 1));
	float4 pos2 = mul(objectUniformBlock.WorldViewProjMat, float4(v2pos, 1));

	float4 wPos0 = cameraUniformBlock.InvViewProject * pos0;
	float4 wPos1 = mul(cameraUniformBlock.InvViewProject,pos1);
	float4 wPos2 = mul(cameraUniformBlock.InvViewProject,pos2);

	float2 two_over_windowsize = cameraUniformBlock.mTwoOverRes;
	
	BarycentricDeriv derivativesOut = CalcFullBary(pos0,pos1,pos2,In.ScreenPos,two_over_windowsize);



	float interpolatedW = dot(float3(pos0.w, pos1.w, pos2.w),derivativesOut.m_lambda);


	float zVal = interpolatedW * getElem(cameraUniformBlock.Project, 2, 2) + getElem(cameraUniformBlock.Project, 3, 2);




	float3 WorldPos = mul(cameraUniformBlock.InvViewProject, float4(In.ScreenPos * interpolatedW, zVal, interpolatedW)).xyz;


	f3x2 texCoords = make_f3x2_cols(
			LoadTexCoord(index0),
			LoadTexCoord(index1),
			LoadTexCoord(index2) 
	);


	float3 positionDX = mul(cameraUniformBlock.InvViewProject, float4((In.ScreenPos+two_over_windowsize.x/2) * interpolatedW, zVal, interpolatedW)).xyz;
	float3 positionDY = mul(cameraUniformBlock.InvViewProject, float4((In.ScreenPos+two_over_windowsize.y/2) * interpolatedW, zVal, interpolatedW)).xyz;

	derivativesOut = CalcRayBary(wPos0.xyz,wPos1.xyz,wPos2.xyz,WorldPos,positionDX,positionDY,
												cameraUniformBlock.mCameraPos.xyz);



	uint materialID = indirectDataBuffer.indirectDataBuffer_data[index0];
    

	GradientInterpolationResults results = Interpolate2DWithDeriv(derivativesOut,texCoords);

	float2 texCoordDX = results.dx;
	float2 texCoordDY = results.dy;
	float2 texCoord = results.interp;




	float4 normalMapRG = f4(0);
	float4 diffuseColor = f4(0);
	float4 specularColor = f4(0);
#define CASE_LIST REPEAT_HUNDRED(0) REPEAT_HUNDRED(100) REPEAT_TEN(200) REPEAT_TEN(210) REPEAT_TEN(220) REPEAT_TEN(230) REPEAT_TEN(240) CASE(250) CASE(251) CASE(252) CASE(253) CASE(254) CASE(255) 
#define NonUniformResourceIndexBlock(materialID) \
		normalMapRG = SampleGradTex2D(normalMaps[materialID], textureSampler, texCoord, texCoordDX, texCoordDY); \
		diffuseColor = SampleGradTex2D(diffuseMaps[materialID], textureSampler, texCoord, texCoordDX, texCoordDY); \
		specularColor = SampleGradTex2D(specularMaps[materialID], textureSampler, texCoord, texCoordDX, texCoordDY); \

#if VK_EXT_DESCRIPTOR_INDEXING_ENABLED
	NonUniformResourceIndexBlock(nonuniformEXT(materialID))
#elif VK_FEATURE_TEXTURE_ARRAY_DYNAMIC_INDEXING_ENABLED
	NonUniformResourceIndexBlock(materialID)
#else
#define CASE(id) case id: NonUniformResourceIndexBlock(id) break;
	switch(materialID) {CASE_LIST};
#undef CASE
#endif
#undef NonUniformResourceIndexBlock
#undef CASE_LIST

	float3 reconstructedNormalMap;
	reconstructedNormalMap.xy = normalMapRG.ga * 2.f - 1.f;
	reconstructedNormalMap.z = sqrt(saturate(1.f - dot(reconstructedNormalMap.xy, reconstructedNormalMap.xy)));


	float3x3 normals = make_f3x3_rows(
		LoadNormal(index0),
		LoadNormal(index1),
		LoadNormal(index2)
	);
	float3 normal = normalize(InterpolateWithDeriv_float3x3(derivativesOut, normals));;


	f3x3 wPositions = make_f3x3_cols(
			wPos0.xyz,
			wPos1.xyz,
			wPos2.xyz
	);

	DerivativesOutput wPosDer = Cal3DDeriv(derivativesOut, wPositions);
	DerivativesOutput uvDer = { float3(results.dx, 0.0), float3(results.dy, 0.0) };
	normal = perturb_normal(reconstructedNormalMap, normal, wPosDer, uvDer);

	float shadowFactor = 1.0f;
	uint ShadowType = 1;
	if(ShadowType ==  0 )
	{
		float4 posLS = mul(lightUniformBlock.mLightViewProj, float4(WorldPos.xyz, 1.0));
		posLS /= posLS.w;
		posLS.y *= -1;
		posLS.xy = posLS.xy * 0.5 + f2(0.5);

		//shadowFactor = calcESMShadowFactor(posLS, ESMShadowTexture, clampMiplessLinearSampler, ESMInputConstants.mEsmControl);
	}
	


	float Roughness = clamp(specularColor.a, 0.05f, 0.99f);
	float Metallic = specularColor.b;

	float3 camPos = cameraUniformBlock.mCameraPos.xyz;

	float3 ViewVec = normalize(camPos.xyz - WorldPos.xyz);

	bool isTwoSided = (geomSet ==  1 ) && (meshConstantsBuffer.meshConstantsBuffer_data[materialID].twoSided == 1);
	bool isBackFace = false;

	if(isTwoSided && dot(normal, ViewVec) < 0.0)
	{

		normal = -normal;
		isBackFace = true;
	}

	float3 lightDir = -lightUniformBlock.mLightDir;

	

	float3 HalfVec = normalize(ViewVec + lightDir);
	float NoV = saturate(dot(normal, ViewVec));

	float NoL = dot(normal, lightDir);


	NoL = (isTwoSided ? abs(NoL) : saturate(NoL));

	float3 shadedColor;


	float3 F0 = f3(0.08);
	float3 SpecularColor = lerp(F0, diffuseColor.rgb, Metallic);
	float3 DiffuseColor = lerp(diffuseColor.rgb, f3(0.0), Metallic);

	shadedColor = calculateIllumination(
		    normal,
		    ViewVec,
			HalfVec,
			NoL,
			NoV,
			camPos.xyz,
			lightDir.xyz,
			WorldPos,
			DiffuseColor,
			SpecularColor,
			Roughness,
			Metallic,
			isBackFace,
			true,
			shadowFactor);

	shadedColor = shadedColor * lightUniformBlock.lightColor.rgb * lightUniformBlock.lightColor.a * NoL;

	float ambientIntencity = 0.05f;
    float3 ambient = diffuseColor.rgb * ambientIntencity;

	shadedColor += ambient;


    outColor = float4(shadedColor.xyz, 1.0f);
}
#endif //FRAGMENT_SHADER