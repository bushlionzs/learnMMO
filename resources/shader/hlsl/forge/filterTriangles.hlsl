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

STRUCT(MeshConstants)
{
	uint indexOffset;
	uint vertexOffset;
	uint materialID;
	uint twoSided;
};



STRUCT(Transform)
{
	float4x4 vp;
	float4x4 mvp;
	float4x4 projection;
	float4x4 invVP;
};

STRUCT(CullingViewPort)
{
	float2 windowSize;
	uint sampleCount;
	uint _pad0;
};

STRUCT(PerFrameVBConstantsData)
{
	Transform transform[ 5 ];
	CullingViewPort cullingViewports[ 5 ];
	uint numViewports;
};

STRUCT(FilterDispatchGroupData)
{
	uint meshIndex;
	uint indexOffset;
    uint instanceDataIndex;
	uint geometrySet_faceCount;
};



RES(CBUFFER(VBConstantBufferData), VBConstantBuffer, UPDATE_FREQ_NONE, b2, VKBINDING(2, 0));

RES(ByteBuffer, vertexDataBuffer, UPDATE_FREQ_NONE, t1, VKBINDING(1, 0));

RES(ByteBuffer, indexDataBuffer, UPDATE_FREQ_NONE, t13, VKBINDING(4, 0));



RES(Buffer(MeshConstants), meshConstantsBuffer, UPDATE_FREQ_NONE, t14, VKBINDING(5, 0));

RES(CBUFFER(PerFrameVBConstantsData), PerFrameVBConstants, UPDATE_FREQ_PER_FRAME, b1, VKBINDING(1, 1));

RES(Buffer(FilterDispatchGroupData), filterDispatchGroupDataBuffer, UPDATE_FREQ_PER_FRAME, t2, VKBINDING(3, 1));

RES(RWBuffer(uint), indirectDrawArgs, UPDATE_FREQ_NONE, u0, VKBINDING(7, 0));

RES(RWBuffer(uint), indirectDataBuffer, UPDATE_FREQ_PER_FRAME, u3, VKBINDING(6, 1));

RES(RWByteBuffer, filteredIndicesBuffer[ 5 ], UPDATE_FREQ_PER_FRAME, u4, VKUNUSED(8));


float3 LoadVertexPositionFloat3(uint vtxIndex)
{
    uint4 aa = LoadByte4(vertexDataBuffer, vtxIndex * 32);
    return asfloat(aa).xyz;
}

float4 LoadVertex(uint index)
{
    return float4(LoadVertexPositionFloat3(index), 1.0f);
}



bool FilterTriangle(uint indices[3], float4 vertices[3], bool cullBackFace, float2 windowSize, uint samples)
{
	if (cullBackFace)
	{
		float det = determinant(float3x3(vertices[0].xyw, vertices[1].xyw, vertices[2].xyw));
		if (det >= 0)
			return true;
	}

	int verticesInFrontOfNearPlane = 0;

	for (uint i = 0; i < 3; i++)
	{
		if (vertices[i].w < 0)
		{
			++verticesInFrontOfNearPlane;



			vertices[i].w *= (-1.0);
		}


		vertices[i].xy /= vertices[i].w * 2;
		vertices[i].xy += float2(0.5, 0.5);
	}



	if (verticesInFrontOfNearPlane == 3)
		return true;

	float minx = min(min(vertices[0].x, vertices[1].x), vertices[2].x);
	float miny = min(min(vertices[0].y, vertices[1].y), vertices[2].y);
	float maxx = max(max(vertices[0].x, vertices[1].x), vertices[2].x);
	float maxy = max(max(vertices[0].y, vertices[1].y), vertices[2].y);

	if ((maxx < 0) || (maxy < 0) || (minx > 1) || (miny > 1))
		return true;




	if (verticesInFrontOfNearPlane == 0)
	{
		const uint SUBPIXEL_BITS = 8;
		const uint SUBPIXEL_MASK = 0xFF;
		const uint SUBPIXEL_SAMPLES = 1U << SUBPIXEL_BITS;
		int2 minBB = int2(1 << 30, 1 << 30);
		int2 maxBB = -minBB;
		for (uint i = 0; i < 3; i++)
		{
			float2 screenSpacePositionFP = vertices[i].xy * windowSize;
			int2 screenSpacePosition = int2(screenSpacePositionFP * (SUBPIXEL_SAMPLES * samples));
			minBB = min(screenSpacePosition, minBB);
			maxBB = max(screenSpacePosition, maxBB);
		}
		{
			const uint SUBPIXEL_SAMPLE_CENTER = SUBPIXEL_SAMPLES / 2;
			const uint SUBPIXEL_SAMPLE_SIZE = SUBPIXEL_SAMPLES - 1;
			if (any( And(GreaterThan( minBB & SUBPIXEL_MASK, SUBPIXEL_SAMPLE_CENTER ),
				LessThan( maxBB - ((minBB & ~SUBPIXEL_MASK) + SUBPIXEL_SAMPLE_CENTER), SUBPIXEL_SAMPLE_SIZE ))))
			{
				return true;
			}
		}
	}


	return false;
}




groupshared uint workGroupOutputSlot[ 5 ];
groupshared uint workGroupIndexCount[ 5 ];
groupshared FilterDispatchGroupData filterDispatchGroupData;
NUM_THREADS( 256 , 1, 1)
void CS( uint3 inGroupId : SV_GROUPTHREADID, uint3 groupId : SV_GROUPID )
{
	if (inGroupId.x == 0)
	{
		for (uint i = 0; i < PerFrameVBConstants.numViewports; ++i)
			AtomicStore(workGroupIndexCount[i], 0u);

		filterDispatchGroupData = filterDispatchGroupDataBuffer[groupId.x];
	}
	
	GroupMemoryBarrier();

	bool cull[ 5 ];
	uint threadOutputSlot[ 5 ];

	for (uint i = 0; i < PerFrameVBConstants.numViewports; ++i)
	{
		threadOutputSlot[i] = 0;
		cull[i] = true;
	}

	uint batchMeshIndex = filterDispatchGroupData.meshIndex;
	uint batchGeomSet = ((filterDispatchGroupData.geometrySet_faceCount &  0x3 ) >>  0 );
	uint batchFaceCount = ((filterDispatchGroupData.geometrySet_faceCount &  0x1FFC ) >>  2 );
	uint batchInputIndexOffset = (meshConstantsBuffer[batchMeshIndex].indexOffset + filterDispatchGroupData.indexOffset);
	uint vertexOffset = meshConstantsBuffer[batchMeshIndex].vertexOffset;
	bool twoSided = (meshConstantsBuffer[batchMeshIndex].twoSided == 1);

	uint indices[3] = { 0, 0, 0 };
	if (inGroupId.x < batchFaceCount)
	{
		indices[0] = vertexOffset + LoadByte(indexDataBuffer, (inGroupId.x * 3 + 0 + batchInputIndexOffset) << 2);
		indices[1] = vertexOffset + LoadByte(indexDataBuffer, (inGroupId.x * 3 + 1 + batchInputIndexOffset) << 2);
		indices[2] = vertexOffset + LoadByte(indexDataBuffer, (inGroupId.x * 3 + 2 + batchInputIndexOffset) << 2);


		indirectDataBuffer[indices[0]] = batchMeshIndex;
		indirectDataBuffer[indices[1]] = batchMeshIndex;
		indirectDataBuffer[indices[2]] = batchMeshIndex;

		float4 vert[3] =
		{
			LoadVertex(indices[0]),
			LoadVertex(indices[1]),
			LoadVertex(indices[2])
		};
		
		for (uint i = 0; i < PerFrameVBConstants.numViewports; ++i)
		{
			float4x4 worldViewProjection = PerFrameVBConstants.transform[i].mvp;

			float4 vertices[3] =
			{
				mul(worldViewProjection, vert[0]),
				mul(worldViewProjection, vert[1]),
				mul(worldViewProjection, vert[2])
			};

			CullingViewPort viewport = PerFrameVBConstants.cullingViewports[i];
			cull[i] = FilterTriangle(indices, vertices, !twoSided, viewport.windowSize, viewport.sampleCount);
			if (!cull[i])
				AtomicAdd(workGroupIndexCount[i], 3, threadOutputSlot[i]);
		}
	}
	
	
	GroupMemoryBarrier();

	if (inGroupId.x == 0)
	{
		for (uint i = 0; i < PerFrameVBConstants.numViewports; ++i)
		{
			uint indirectDrawIndex =  (((((i) * 2 ) + (batchGeomSet)) * 8 ) + (0)) ;
			AtomicAdd(indirectDrawArgs[indirectDrawIndex], workGroupIndexCount[i], workGroupOutputSlot[i]);
		}
	}

	GroupMemoryBarrier();

	for (uint j = 0; j < PerFrameVBConstants.numViewports; ++j)
	{
		if (!cull[j])
		{
			uint outputIndex = (AtomicLoad(workGroupOutputSlot[j]) + threadOutputSlot[j]);


			StoreByte(filteredIndicesBuffer[j], ( ( VBConstantBuffer.vbConstant[batchGeomSet].indexOffset )  + outputIndex + 0) << 2, indices[0]);
			StoreByte(filteredIndicesBuffer[j], ( ( VBConstantBuffer.vbConstant[batchGeomSet].indexOffset )  + outputIndex + 1) << 2, indices[1]);
			StoreByte(filteredIndicesBuffer[j], ( ( VBConstantBuffer.vbConstant[batchGeomSet].indexOffset )  + outputIndex + 2) << 2, indices[2]);
		}
	}
}