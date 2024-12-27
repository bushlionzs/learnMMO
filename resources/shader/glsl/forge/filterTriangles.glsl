#version 450 core
#extension GL_GOOGLE_include_directive : require

#include "glslBase.glsl"



layout (UPDATE_FREQ_NONE, binding = 0)  buffer indirectDrawArgsStruct
{
	uint indirectDrawArgs_data[];
}indirectDrawArgs;

layout(UPDATE_FREQ_NONE, binding = 2) uniform VBConstantBufferStruct
{
    VBConstants vbConstant[2];
}VBConstantBuffer;

layout (std430, UPDATE_FREQ_NONE, binding = 1) readonly buffer vertexDataBufferStruct
{
	uint vertexDataBuffer_data[];
}vertexDataBuffer;


layout (std430, UPDATE_FREQ_NONE, binding = 4) readonly buffer indexDataBufferStruct
{
	uint indexDataBuffer_data[];
}indexDataBuffer;

layout (std430, UPDATE_FREQ_NONE, binding = 5) readonly buffer meshConstantsBufferStruct
{
	MeshConstants meshConstantsBuffer_data[];
}meshConstantsBuffer;


layout(UPDATE_FREQ_PER_FRAME, binding = 1) uniform PerFrameVBConstantsStruct
{
    Transform transform[5];
	CullingViewPort cullingViewports[5];
	uint numViewports;
}PerFrameVBConstants;

layout (std430, UPDATE_FREQ_PER_FRAME, binding = 3) readonly buffer filterDispatchGroupDataBufferStruct
{
	FilterDispatchGroupData filterDispatchGroupDataBuffer_data[];
}filterDispatchGroupDataBuffer;


layout (std430, UPDATE_FREQ_PER_FRAME, binding = 6)  buffer indirectDataBufferStruct
{
	uint indirectDataBuffer_data[];
}indirectDataBuffer;

layout (std430, UPDATE_FREQ_PER_FRAME, binding = 8)  buffer filteredIndicesBufferStruct
{
	uint _data[];
} filteredIndicesBuffer[ 5 ];

float3 LoadVertexPositionFloat3(uint vtxIndex)
{
    uint4 aa = LoadByte4(vertexDataBuffer.vertexDataBuffer_data, vtxIndex * 32);
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




shared uint workGroupOutputSlot[ 5 ];
shared uint workGroupIndexCount[ 5 ];
shared FilterDispatchGroupData filterDispatchGroupData;
NUM_THREADS( 256 , 1, 1)
void main()
{
	{
	    vec3 pos = LoadVertexPositionFloat3(0);
		vec3 pos2 = LoadVertexPositionFloat3(11);
	}
    const uvec3 inGroupId = uvec3(gl_LocalInvocationID);
	const uvec3 groupId = uvec3(gl_WorkGroupID);
	
	if (inGroupId.x == 0)
	{
		for (uint i = 0; i < PerFrameVBConstants.numViewports; ++i)
			AtomicStore(workGroupIndexCount[i], 0u);

		filterDispatchGroupData = filterDispatchGroupDataBuffer.filterDispatchGroupDataBuffer_data[groupId.x];
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
	uint batchInputIndexOffset = (meshConstantsBuffer.meshConstantsBuffer_data[batchMeshIndex].indexOffset + filterDispatchGroupData.indexOffset);
	uint vertexOffset = meshConstantsBuffer.meshConstantsBuffer_data[batchMeshIndex].vertexOffset;
	bool twoSided = (meshConstantsBuffer.meshConstantsBuffer_data[batchMeshIndex].twoSided == 1);

	uint indices[3] = { 0, 0, 0 };
	if (inGroupId.x < batchFaceCount)
	{
		indices[0] = vertexOffset + LoadByte(indexDataBuffer.indexDataBuffer_data, (inGroupId.x * 3 + 0 + batchInputIndexOffset) << 2);
		indices[1] = vertexOffset + LoadByte(indexDataBuffer.indexDataBuffer_data, (inGroupId.x * 3 + 1 + batchInputIndexOffset) << 2);
		indices[2] = vertexOffset + LoadByte(indexDataBuffer.indexDataBuffer_data, (inGroupId.x * 3 + 2 + batchInputIndexOffset) << 2);


		indirectDataBuffer.indirectDataBuffer_data[indices[0]] = batchMeshIndex;
		indirectDataBuffer.indirectDataBuffer_data[indices[1]] = batchMeshIndex;
		indirectDataBuffer.indirectDataBuffer_data[indices[2]] = batchMeshIndex;

		float4 vert[3] =
		{
			LoadVertex(indices[0]),
			LoadVertex(indices[1]),
			LoadVertex(indices[2])
		};
		
		for (uint i = 0; i < PerFrameVBConstants.numViewports; ++i)
		{
			float4x4 worldViewProjection = PerFrameVBConstants.transform[i].mvp;

			vec4 vertices[3] =
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
			AtomicAdd(indirectDrawArgs.indirectDrawArgs_data[indirectDrawIndex], workGroupIndexCount[i], workGroupOutputSlot[i]);
		}
	}

	GroupMemoryBarrier();

	for (uint j = 0; j < PerFrameVBConstants.numViewports; ++j)
	{
		if (!cull[j])
		{
			uint outputIndex = (AtomicLoad(workGroupOutputSlot[j]) + threadOutputSlot[j]);


			StoreByte(filteredIndicesBuffer[j]._data, ( ( VBConstantBuffer.vbConstant[batchGeomSet].indexOffset )  + outputIndex + 0) << 2, indices[0]);
			StoreByte(filteredIndicesBuffer[j]._data, ( ( VBConstantBuffer.vbConstant[batchGeomSet].indexOffset )  + outputIndex + 1) << 2, indices[1]);
			StoreByte(filteredIndicesBuffer[j]._data, ( ( VBConstantBuffer.vbConstant[batchGeomSet].indexOffset )  + outputIndex + 2) << 2, indices[2]);
		}
	}
}