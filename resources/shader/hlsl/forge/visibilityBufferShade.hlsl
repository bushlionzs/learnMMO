#include "base.hlsl"

STRUCT(PsIn)
{
	DATA(float4, Position, SV_Position);
	DATA(float2, ScreenPos, TEXCOORD0);
#line 30
};

PsIn VS( uint vertexID : SV_VERTEXID )
{
	//INIT_MAIN;
	PsIn Out;


	Out.Position.x = (vertexID == 2 ? 3.0 : -1.0);
	Out.Position.y = (vertexID == 0 ? -3.0 : 1.0);
	Out.Position.zw = float2(0, 1);
	Out.ScreenPos = Out.Position.xy;
	return (Out);
}

STRUCT(VBConstants)
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

STRUCT(MeshConstants)
{
	uint indexOffset;
	uint vertexOffset;
	uint materialID;
	uint twoSided;
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

STRUCT(PerFrameVBConstantsData)
{
	Transform transform[ 5 ];
	CullingViewPort cullingViewports[ 5 ];
	uint numViewports;
};

STRUCT(LightUniformBlock)
{
    float4x4 mLightViewProj;
    float4 lightPosition;
    float4 lightColor;
	float4 mLightUpVec;
	float4 mTanLightAngleAndThresholdValue;
	float3 mLightDir;
};


STRUCT(CameraUniformBlock)
{
    float4x4 View;
    float4x4 Project;
    float4x4 ViewProject;
    float4x4 InvView;
	float4x4 InvProj;
	float4x4 InvViewProject;
	float4 mCameraPos;
	float mNear;
	float mFar;
	float mFarNearDiff;
	float mFarNear;
	float2 mTwoOverRes;
	float2 mWindowSize;
	float4 mDeviceZToWorldZ;
};


STRUCT(ObjectUniform)
{
    float4x4 WorldViewProjMat;
};

STRUCT(RenderSettingUniform)
{
    float4 WindowDimension;
    int ShadowType;
};

STRUCT(ESMInputConstantData)
{
    float mEsmControl;
};

STRUCT(SSSConfig)
{
	uint mEnabled;
};

RES(CBUFFER(VBConstantBufferData), VBConstantBuffer, UPDATE_FREQ_NONE, b2, VKBINDING(0, 0));
RES(ByteBuffer, vertexDataBuffer, UPDATE_FREQ_NONE, t0, VKBINDING(1, 0));
RES(Buffer(MeshConstants), meshConstantsBuffer, UPDATE_FREQ_NONE, t6, VKBINDING(2, 0));


RES(Depth2D(float), DepthAtlasTexture, UPDATE_FREQ_NONE, t30, VKBINDING(5, 0));
RES(Tex2D(float), PrerenderLodClampTexture, UPDATE_FREQ_NONE, t32, VKBINDING(6, 0));
RES(Tex2D(float), ESMShadowTexture, UPDATE_FREQ_NONE, t33, VKBINDING(7, 0));
RES(Tex2D(float4), vbPassTexture, UPDATE_FREQ_NONE, t18, VKBINDING(8, 0));
//RES(Tex2D(uint), ScreenSpaceShadowTexture, UPDATE_FREQ_NONE, t36, VKBINDING(9, 0));

RES(SamplerComparisonState, ShadowCmpSampler, UPDATE_FREQ_NONE, s4, VKBINDING(10, 0));
RES(SamplerState, clampBorderNearSampler, UPDATE_FREQ_NONE, s3, VKBINDING(11, 0));
RES(SamplerState, clampMiplessLinearSampler, UPDATE_FREQ_NONE, s2, VKBINDING(12, 0));
RES(SamplerState, clampMiplessNearSampler, UPDATE_FREQ_NONE, s1, VKBINDING(13, 0));
RES(SamplerState, textureSampler, UPDATE_FREQ_NONE, s0, VKBINDING(14, 0));


RES(Tex2D(float4), IndexTexture[10], UPDATE_FREQ_NONE, t20, VKUNUSED(20));
RES(Tex2D(float4), diffuseMaps[ 256U ], UPDATE_FREQ_NONE, t40, VKUNUSED(40));
RES(Tex2D(float4), normalMaps[ 256U ], UPDATE_FREQ_NONE, t296, VKUNUSED(296));
RES(Tex2D(float4), specularMaps[ 256U ], UPDATE_FREQ_NONE, t552, VKUNUSED(552));



//////

RES(CBUFFER(PerFrameVBConstantsData), PerFrameVBConstants, UPDATE_FREQ_PER_FRAME, b1, VKBINDING(0, 1));
RES(CBUFFER(LightUniformBlock), lightUniformBlock, UPDATE_FREQ_PER_FRAME, b1, VKBINDING(1, 1));
RES(CBUFFER(CameraUniformBlock), cameraUniformBlock, UPDATE_FREQ_PER_FRAME, b3, VKBINDING(2, 1));
RES(CBUFFER(ObjectUniform), objectUniformBlock, UPDATE_FREQ_PER_FRAME, b0, VKBINDING(3, 1));
RES(CBUFFER(RenderSettingUniform), renderSettingUniformBlock, UPDATE_FREQ_PER_FRAME, b4, VKBINDING(4, 1));
RES(CBUFFER(ESMInputConstantData), ESMInputConstants, UPDATE_FREQ_PER_FRAME, b5, VKBINDING(5, 1));
RES(CBUFFER(SSSConfig), SSSEnabled, UPDATE_FREQ_PER_FRAME, b8, VKBINDING(6, 1));
RES(ByteBuffer, filteredIndexBuffer, UPDATE_FREQ_PER_FRAME, t4, VKBINDING(7, 1));
RES(Buffer(uint), indirectDataBuffer, UPDATE_FREQ_PER_FRAME, t5, VKBINDING(8, 1));


float3 LoadVertexPositionFloat3(uint vtxIndex)
{
    return asfloat(LoadByte4(vertexDataBuffer, vtxIndex * 32)).xyz;
}

float3 LoadVertex(uint index)
{
    return LoadVertexPositionFloat3(index);
}

float2 LoadVertexUVFloat2(uint vtxIndex)
{
    return asfloat(LoadByte2(vertexDataBuffer, vtxIndex * 32 + 24)).xy;
}

float2 LoadTexCoord(uint index)
{
    return LoadVertexUVFloat2(index);
}

float3 LoadVertexNormalFloat3(uint vtxIndex)
{
    return asfloat(LoadByte4(vertexDataBuffer, vtxIndex * 32 + 12)).xyz;
}

float3 LoadNormal(uint index)
{
    return LoadVertexNormalFloat3(index);
}

uint packUnorm4x8(float4 v)
{
	uint4 UNorm = uint4(round(saturate(v) * 255.0));
	return (0x000000FF & UNorm.x) | ((UNorm.y << 8) & 0x0000FF00) | ((UNorm.z << 16) & 0x00FF0000) | ((UNorm.w << 24) & 0xFF000000);
}


#include "glslFunc.glsl"

STRUCT(PsOut)
{
    DATA(float4, FinalColor, SV_Target0);
};

PsOut PS( PsIn In, uint i : SV_SAMPLEINDEX )
{	
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

	uint index0 = LoadByte(filteredIndexBuffer, triIdx0 << 2);
	uint index1 = LoadByte(filteredIndexBuffer, triIdx1 << 2);
	uint index2 = LoadByte(filteredIndexBuffer, triIdx2 << 2);
	

	float3 v0pos = LoadVertex(index0);
	float3 v1pos = LoadVertex(index1);
	float3 v2pos = LoadVertex(index2);

	float4 pos0 = mul(objectUniformBlock.WorldViewProjMat, float4(v0pos, 1));
	float4 pos1 = mul(objectUniformBlock.WorldViewProjMat, float4(v1pos, 1));
	float4 pos2 = mul(objectUniformBlock.WorldViewProjMat, float4(v2pos, 1));

	float4 wPos0 = mul(cameraUniformBlock.InvViewProject, pos0);
	float4 wPos1 = mul(cameraUniformBlock.InvViewProject, pos1);
	float4 wPos2 = mul(cameraUniformBlock.InvViewProject, pos2);

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



	uint materialID = indirectDataBuffer[index0];
    

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

	bool isTwoSided = (geomSet ==  1 ) && (meshConstantsBuffer[materialID].twoSided == 1);
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


    PsOut Out;
    Out.FinalColor = float4(shadedColor.xyz, 1.0f);
	
	return Out;
}
