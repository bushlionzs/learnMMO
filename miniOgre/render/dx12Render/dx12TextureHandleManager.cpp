#include <OgreHeader.h>
#include "dx12TextureHandleManager.h"

Dx12TextureHandleManager::Dx12TextureHandleManager(ID3D12Device* device)
{
	mDx12Device = device;

	mContainerCount = 10000;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = mContainerCount;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	for (uint32_t i = 0; i < mContainerCount; i++)
	{
		mHandleContainer.insert(i);
	}

	mHeaps.push_back(mSrvDescriptorHeap.Get());
	mCbvSrvUavDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

int32_t Dx12TextureHandleManager::allocStartIndex(int32_t hCount)
{
	if (mFreeIndex + hCount > mContainerCount)
	{
		assert(false);
		return -1;
	}

	int32_t start = mFreeIndex;
	mFreeIndex += hCount;
	return start;
}


CD3DX12_CPU_DESCRIPTOR_HANDLE Dx12TextureHandleManager::getCpuHandleByIndex(int32_t index)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	hDescriptor.Offset(index, mCbvSrvUavDescriptorSize);

	return hDescriptor;
}
CD3DX12_GPU_DESCRIPTOR_HANDLE Dx12TextureHandleManager::getGpuHandleByIndex(int32_t index)
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	hDescriptor.Offset(index, mCbvSrvUavDescriptorSize);

	return hDescriptor;
}