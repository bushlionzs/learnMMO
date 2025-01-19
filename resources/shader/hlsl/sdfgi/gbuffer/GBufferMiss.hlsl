#include "ddgiDescriptors.hlsl"
#include "ddgiRayTracing.hlsl"

// ---[ Miss Shader ]---

[shader("miss")]
void Miss(inout PackedPayload packedPayload)
{
    packedPayload.hitT = -1.f;
}