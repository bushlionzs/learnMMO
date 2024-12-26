#include "common.hlsl"
struct VertexIn
{
	float3 PosL    : POSITION;
	float4 ColorL  : COLOR0;
	float2 iTexcoord_0 : TEXCOORD0;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float4 oColor  : COLOR0;
    float2	oTexcoord_0 : TEXCOORD0;
};

VertexOut VS(VertexIn vIn)
{
	VertexOut vOut;
	float4 posW = mul(gWorld, float4(vIn.PosL, 1.0f));
	vOut.PosH = mul(gViewProj, posW);
	vOut.oColor = vIn.ColorL;
	vOut.oTexcoord_0 = vIn.iTexcoord_0;
	return vOut;
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 color = first.Sample(firstSampler, pin.oTexcoord_0)* pin.oColor;
	clip(color.a - 0.5f);
	return color;
}
