Texture2D first: register(t0);
SamplerState firstSampler        : register(s0);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 iTexcoord_0 : TEXCOORD0;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float2	oTexcoord_0 : TEXCOORD0;
};

VertexOut vs(VertexIn vIn)
{
	VertexOut vOut;
	
	vOut.PosH = mul(gWorldViewProj, float4(vIn.PosL, 1.0f));
	vOut.oTexcoord_0 = vIn.iTexcoord_0;
	return vOut;
}

float4 ps(VertexOut pin) : SV_Target
{
	return first.SampleLevel(firstSampler, pin.oTexcoord_0, 0.0f);
}
