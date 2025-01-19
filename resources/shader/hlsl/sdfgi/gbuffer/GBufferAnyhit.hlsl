/*
* Copyright (c) 2019-2023, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "ddgiDescriptors.hlsl"
#include "ddgiRayTracing.hlsl"



[shader("anyhit")]
void AHS_PRIMARY(inout PackedPayload payload, BuiltInTriangleIntersectionAttributes attrib)
{
    // Load the intersected mesh geometry's data
    GeometryData geometry;
    GetGeometryData(InstanceID(), GeometryIndex(), geometry);

    // Load the material
    Material material = GetMaterial(geometry);

    float alpha = material.opacity;
    if (material.alphaMode == 2)
    {
        // Load the vertices
        Vertex vertices[3];
        LoadVerticesPosUV0(InstanceID(), PrimitiveIndex(), geometry, vertices);

        // Compute texture coordinate differentials
        float2 dUVdx, dUVdy;
        ComputeUV0Differentials(vertices, WorldRayDirection(), RayTCurrent(), dUVdx, dUVdy);

        // TODO-ACM: passing ConstantBuffer<T> to functions crashes DXC HLSL->SPIRV
        //ConstantBuffer<Camera> camera = GetCamera();
        //ComputeUV0Differentials(vertices, camera, WorldRayDirection(), RayTCurrent(), dUVdx, dUVdy);

        // Interpolate the triangle's texture coordinates
        float3 barycentrics = float3((1.f - attrib.barycentrics.x - attrib.barycentrics.y), attrib.barycentrics.x, attrib.barycentrics.y);
        Vertex v = InterpolateVertexUV0(vertices, barycentrics);

        // Sample the texture
        if (material.albedoTexIdx > -1)
        {
            alpha *= GetTex2D(material.albedoTexIdx).SampleGrad(GetAnisoWrapSampler(), v.uv0, dUVdx, dUVdy).a;
        }
    }

    if (alpha < material.alphaCutoff) IgnoreHit();
}

