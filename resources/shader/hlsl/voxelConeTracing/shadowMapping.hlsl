#include "common.hlsl"

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD0;
};

struct PSOutput
{
    float4 worldPos : SV_Target0;
    float4 normal   : SV_Target1;
    float4 flux     : SV_Target2;
};


float4 VSOnlyMain(VSInput input) : SV_Position
{
    float4 result;   
    result = mul(cbPerObject.gWorld, float4(input.position.xyz, 1));
    result = mul(cbPass.gViewProj, result);
    result.z *= result.w;

    return result;
}
