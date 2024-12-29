#include "base.hlsl"



struct VBConstants
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

RES(RWBuffer(uint), indirectDrawArgs, UPDATE_FREQ_NONE, u0, VKBINDING(0, 0));

RES(CBUFFER(VBConstantBufferData), VBConstantBuffer, UPDATE_FREQ_NONE, b2, VKBINDING(2, 0));



void ClearIndirectDrawArgsBuffers(const uint dispatchThreadIdX)
{
	if(dispatchThreadIdX == 0)
	{
         for (uint view = 0; view <  5 ; ++view)
         {
            for (uint geom = 0; geom <  2 ; ++geom)
            {
               indirectDrawArgs[ (((((view) * 2 ) + (geom)) * 8 ) + (0)) ] = 0;
               indirectDrawArgs[ (((((view) * 2 ) + (geom)) * 8 ) + (1)) ] = 1;
               indirectDrawArgs[ (((((view) * 2 ) + (geom)) * 8 ) + (2)) ] = VBConstantBuffer.vbConstant[geom].indexOffset  ;
               indirectDrawArgs[ (((((view) * 2 ) + (geom)) * 8 ) + (3)) ] = 0;
               indirectDrawArgs[ (((((view) * 2 ) + (geom)) * 8 ) + (4)) ] = 0;
            }
         }
    }
}

NUM_THREADS(1, 1, 1)
void CS(uint3 threadID : SV_DISPATCHTHREADID)
{
    ClearIndirectDrawArgsBuffers(threadID.x);
}