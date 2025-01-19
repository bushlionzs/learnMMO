#include "base.hlsl"

struct VoxelizationBlock
{
    float4x4 WorldVoxelCube;
    float4x4 ViewProjection;
    float4x4 ShadowViewProjection;
    float WorldVoxelScale;
};

RES(CBUFFER(VoxelizationBlock), VoxelizationCB, UPDATE_FREQ_NONE, b0, VKBINDING(0, 0));

struct ObjectBlock
{
    float4x4 gWorld;
	float4x4 gProjector;
	float4 diffuseColor;
};

RES(CBUFFER(ObjectBlock), cbPerObject, UPDATE_FREQ_NONE, b1, VKBINDING(1, 0));

struct VS_IN
{
    VKLOCATION(0) float3 position : POSITION;
    VKLOCATION(1) float3 normal   : NORMAL;
};

struct GS_IN
{
    VKLOCATION(0) float4 position : SV_POSITION;
    VKLOCATION(1) float3 normal   : NORMAL;
};

struct PS_IN
{
    float4 position : SV_POSITION;
    float3 voxelPos : VOXEL_POSITION;
};

RWTexture3D<float4> outputTexture VKBINDING(2,0): register(u0);
Texture2D<float> shadowBuffer VKBINDING(3,0): register(t0);

SamplerState PcfShadowMapSampler VKBINDING(4,0): register(s0);

GS_IN VSMain(VS_IN input)
{
    GS_IN output = (GS_IN) 0;
    
    output.position = mul(cbPerObject.gWorld, float4(input.position.xyz, 1));
    return output;
}

[maxvertexcount(3)]
void GSMain(triangle GS_IN input[3], inout TriangleStream<PS_IN> OutputStream)
{
    PS_IN output[3];
    output[0] = (PS_IN) 0;
    output[1] = (PS_IN) 0;
    output[2] = (PS_IN) 0;
    
    float3 p1 = input[1].position.rgb - input[0].position.rgb;
    float3 p2 = input[2].position.rgb - input[0].position.rgb;
    float3 n = abs(normalize(cross(p1, p2)));
       
	float axis = max(n.x, max(n.y, n.z));
    
    [unroll]
    for (uint i = 0; i < 3; i++)
    {
        output[0].voxelPos = input[i].position.xyz / VoxelizationCB.WorldVoxelScale * 2.0f;
        output[1].voxelPos = input[i].position.xyz / VoxelizationCB.WorldVoxelScale * 2.0f;
        output[2].voxelPos = input[i].position.xyz / VoxelizationCB.WorldVoxelScale * 2.0f;
        if (axis == n.z)
            output[i].position = float4(output[i].voxelPos.x, output[i].voxelPos.y, 0, 1);
        else if (axis == n.x)
            output[i].position = float4(output[i].voxelPos.y, output[i].voxelPos.z, 0, 1);
        else
            output[i].position = float4(output[i].voxelPos.x, output[i].voxelPos.z, 0, 1);
    
        //output[i].normal = input[i].normal;
        OutputStream.Append(output[i]);
    }
    OutputStream.RestartStrip();
}

float3 VoxelToWorld(float3 pos)
{
    float3 result = pos;
    result *= VoxelizationCB.WorldVoxelScale;

    return result * 0.5f;
}


void PSMain(PS_IN input)
{
    uint width;
    uint height;
    uint depth;
    
    outputTexture.GetDimensions(width, height, depth);
    float3 voxelPos = input.voxelPos.rgb;
    voxelPos.y = -voxelPos.y; 
    
    int3 finalVoxelPos = width * float3(0.5f * voxelPos + float3(0.5f, 0.5f, 0.5f));
    float4 colorRes = float4(cbPerObject.diffuseColor.rgb, 1.0f);
    voxelPos.y = -voxelPos.y; 
    
    float4 worldPos = float4(VoxelToWorld(voxelPos), 1.0f);
    float4 lightSpacePos = mul(VoxelizationCB.ShadowViewProjection, worldPos);
    float4 shadowcoord = lightSpacePos / lightSpacePos.w;
    shadowcoord.rg = shadowcoord.rg * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
	
	
	float4 depthColor = shadowBuffer.Sample(PcfShadowMapSampler, shadowcoord.xy);
	
	float shadow;
	if(depthColor.r <= shadowcoord.z)
	{
	    shadow = 0.0f;
	}
	else
	{
	    shadow = 1.0f;
	}


    outputTexture[finalVoxelPos] = colorRes * float4(shadow, shadow, shadow, 1.0f);
}