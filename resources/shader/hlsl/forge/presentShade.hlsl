#include "base.hlsl"

STRUCT(PsIn)
{
	DATA(float4, Position, SV_Position);
	DATA(float2, texCoord, TEXCOORD0);
};
PsIn VS(uint vertexID : SV_VERTEXID)
{
	
	float4 position;
	position.x = (vertexID == 2) ? 3.0 : -1.0;
	position.y = (vertexID == 0) ? -3.0 : 1.0;
	position.zw = f2(1.0);
	
	
	PsIn Out;
	Out.Position = position;
    Out.texCoord = position.xy * float2(0.5, -0.5) + 0.5;
	return Out;
}

STRUCT(PsOut)
{
    DATA(float4, FinalColor, SV_Target0);
};

RES(Tex2D(float4), SourceTexture, UPDATE_FREQ_NONE, t0, VKBINDING(0,0));
RES(SamplerState, repeatBillinearSampler, UPDATE_FREQ_NONE, s1, VKBINDING(1,0));

PsOut PS(PsIn In)
{
	PsOut Out;
	Out.FinalColor = SampleTex2D(SourceTexture, repeatBillinearSampler, In.texCoord);
	return Out;
}
