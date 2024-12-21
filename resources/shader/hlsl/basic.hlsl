#include "common.hlsl"
struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
#ifdef USETANGENT
	float3 TangentL : TANGENT;
#endif
	float2 TexC    : TEXCOORD;
#ifdef SKINNED
    float4 BoneWeights : BLENDWEIGHT;
    uint4 BoneIndices  : BLENDINDICES;
#endif
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float4 ShadowPosH : POSITION0;
    float3 PosW    : POSITION1;
    float3 NormalW : NORMAL;
#ifdef USETANGENT
	float3 TangentW : TANGENT;
#endif
	float2 TexC    : TEXCOORD;
};

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
	
	#ifdef SKINNED
    float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    weights[0] = vIn.BoneWeights.x;
    weights[1] = vIn.BoneWeights.y;
    weights[2] = vIn.BoneWeights.z;
    weights[3] = vIn.BoneWeights.w;
    //weights[3]  =1.0f - weights[0] - weights[1] - weights[2];
    float3 posL = float3(0.0f, 0.0f, 0.0f);
    float3 normalL = float3(0.0f, 0.0f, 0.0f);
#ifdef USETANGENT
    float3 tangentL = float3(0.0f, 0.0f, 0.0f);
#endif
    for(int i = 0; i < 4; ++i)
    {
        // Assume no nonuniform scaling when transforming normals, so 
        // that we do not have to use the inverse-transpose.

		posL += weights[i] * mul(gBoneTransforms[vIn.BoneIndices[i]], float4(vIn.PosL, 1.0f)).xyz;
        normalL += weights[i] * mul((float3x3)gBoneTransforms[vIn.BoneIndices[i]], vIn.NormalL);
		
#ifdef USETANGENT
        tangentL += weights[i] * mul(vIn.TangentL.xyz, (float3x3)gBoneTransforms[vIn.BoneIndices[i]]);
#endif
    }

    vIn.PosL = posL;
    vIn.NormalL = normalL;
#ifdef USETANGENT
    vIn.TangentL.xyz = tangentL;
#endif
#endif
    float4 posW = mul(gWorld, float4(vIn.PosL, 1.0f));
    //vOut.PosH = mul(gWorldViewProj, float4(vIn.PosL, 1.0f));
	vOut.PosH = mul(gViewProj, posW);
	//vOut.PosH = float4(vIn.PosL, 1.0f);
    vOut.PosW = posW.xyz;
    vOut.NormalW = mul((float3x3) gWorld, vIn.NormalL);
#ifdef USETANGENT
	vOut.TangentW = mul((float3x3)gWorld, vIn.TangentL);
#endif
    vOut.TexC = mul(gTexTransform, float4(vIn.TexC, 0.0f, 1.0f)).xy;
	vOut.ShadowPosH = mul(gShadowTransform, posW);
    return vOut;
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 diffuseAlbedo = first.Sample(gsamAnisotropicWrap, pin.TexC) * gDiffuseAlbedo;
	clip(diffuseAlbedo.a - 0.5f);
	return diffuseAlbedo;
	pin.NormalW = normalize(pin.NormalW);

    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // Light terms.
	//float4 gAmbientLight = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 ambient = gAmbientLight*diffuseAlbedo;
    const float shininess = 1.0f - gRoughness;
	float3 fresnel = float3(0.01f, 0.01f, 0.01f); 
    Material mat = { diffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = float3(0.0f, 1.0f, 1.0f);
	shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH);
	if(shadowFactor[0] < 0.1)
	{
		//return float4(0.0f, 1.0f, 0.0f, 1.0f);
	}
    float4 directLight = ComputeLighting(mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor);
    float4 litColor = ambient + directLight;

    // Common convention to take alpha from diffuse material.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}