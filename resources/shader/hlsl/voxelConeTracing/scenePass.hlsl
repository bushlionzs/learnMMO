
#include "common.hlsl"



struct VertexIn
{
	VKLOCATION(0) float3 position : POSITION;
    VKLOCATION(1) float3 normal   : NORMAL;
	VKLOCATION(3) float2 texUV    : TEXCOORD;
};



struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float4 outWorldPos : POSITION;
	float3 outNormal: NORMAL;
	float2 outUV: TEXCOORD;
};

VertexOut VS(VertexIn vIn) 
{
    VertexOut Out;
    Out.outWorldPos = mul(cbPerObject.gWorld , float4(vIn.position, 1.0));
    Out.PosH = mul(cbPass.gViewProj , Out.outWorldPos);    
    Out.outNormal = mul((float3x3)cbPerObject.gWorld , vIn.normal);
    Out.outUV = vIn.texUV;
	return Out;
}


struct PSOutput
{
	float4 color : SV_Target0;
	float4 normal : SV_Target1;
	float4 worldpos : SV_Target2;
	float4 color2 : SV_Target3;
};

PSOutput PS(VertexOut pin)
{
	PSOutput Out;
	//Out.color = first.Sample(firstSampler, pin.outUV);
    Out.color = cbPerObject.diffuseColor;
	Out.color2 = cbPerObject.diffuseColor;
	Out.normal = float4(pin.outNormal, 1.0f);
    Out.worldpos = pin.outWorldPos;
	return Out;
}