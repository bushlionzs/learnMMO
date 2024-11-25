#include <OgreHeader.h>
#include "memoryAllocator.h"
#include <Direct3D12MemoryAllocator.h>
#include <IMemory.h>
#include "d3dutil.h"

static void add_descriptor_heap(
    ID3D12Device* pDevice, 
    const D3D12_DESCRIPTOR_HEAP_DESC* pDesc, 
    DescriptorHeap** ppDescHeap)
{
    uint32_t numDescriptors = pDesc->NumDescriptors;

    // Keep 32 aligned for easy remove
    numDescriptors = d3dUtil::round_up(numDescriptors, 32);

    const size_t sizeInBytes = (numDescriptors / 32) * sizeof(uint32_t);

    DescriptorHeap* pHeap = (DescriptorHeap*)tf_calloc(1, sizeof(*pHeap) + sizeInBytes);
    pHeap->pFlags = (uint32_t*)(pHeap + 1);
    pHeap->pDevice = pDevice;

    initMutex(&pHeap->mMutex);

    D3D12_DESCRIPTOR_HEAP_DESC desc = *pDesc;
    desc.NumDescriptors = numDescriptors;

    CHECK_HRESULT(pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap->pHeap)));

    pHeap->mStartCpuHandle = pHeap->pHeap->GetCPUDescriptorHandleForHeapStart();
    if (desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
    {
        pHeap->mStartGpuHandle = pHeap->pHeap->GetGPUDescriptorHandleForHeapStart();
    }
    pHeap->mNumDescriptors = desc.NumDescriptors;
    pHeap->mType = desc.Type;
    pHeap->mDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(pHeap->mType);

    *ppDescHeap = pHeap;
}

DxMemoryAllocator::DxMemoryAllocator(ID3D12Device* device)
{
    return;
    D3D12MA::ALLOCATOR_DESC desc = {};
    desc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
    desc.pDevice = device;
    //desc.pAdapter = pRenderer->pGpu->mDx.pGpu;

    D3D12MA::ALLOCATION_CALLBACKS allocationCallbacks = {};
    allocationCallbacks.pAllocate = [](size_t size, size_t alignment, void*) { return tf_memalign(alignment, size); };
    allocationCallbacks.pFree = [](void* ptr, void*) { tf_free(ptr); };
    desc.pAllocationCallbacks = &allocationCallbacks;
    CHECK_HRESULT(D3D12MA::CreateAllocator(&desc, &mAllocator));

    
}

void return_descriptor_handles_unlocked(DescriptorHeap* pHeap, DxDescriptorID handle, uint32_t count)
{
    if (D3D12_DESCRIPTOR_ID_NONE == handle || !count)
    {
        return;
    }

    for (uint32_t id = handle; id < handle + count; ++id)
    {
        const uint32_t i = id / 32;
        const uint32_t mask = ~(1 << (id % 32));
        pHeap->pFlags[i] &= mask;
    }

    pHeap->mUsedDescriptors -= count;
}

D3D12_CPU_DESCRIPTOR_HANDLE descriptor_id_to_cpu_handle(
    DescriptorHeap* pHeap, DxDescriptorID id)
{
    return { pHeap->mStartCpuHandle.ptr + id * pHeap->mDescriptorSize };
}

D3D12_GPU_DESCRIPTOR_HANDLE descriptor_id_to_gpu_handle(
    DescriptorHeap* pHeap, DxDescriptorID id)
{
    return { pHeap->mStartGpuHandle.ptr + id * pHeap->mDescriptorSize };
}


void return_descriptor_handles(DescriptorHeap* pHeap, DxDescriptorID handle, uint32_t count)
{
    MutexLock lock(pHeap->mMutex);
    return_descriptor_handles_unlocked(pHeap, handle, count);
}

DxDescriptorID consume_descriptor_handles(DescriptorHeap* pHeap, uint32_t descriptorCount)
{
    if (!descriptorCount)
    {
        return D3D12_DESCRIPTOR_ID_NONE;
    }

    MutexLock lock(pHeap->mMutex);

    DxDescriptorID result = D3D12_DESCRIPTOR_ID_NONE;
    DxDescriptorID firstResult = D3D12_DESCRIPTOR_ID_NONE;
    uint32_t       foundCount = 0;

    for (uint32_t i = 0; i < pHeap->mNumDescriptors / 32; ++i)
    {
        const uint32_t flag = pHeap->pFlags[i];
        if (UINT32_MAX == flag)
        {
            return_descriptor_handles_unlocked(pHeap, firstResult, foundCount);
            foundCount = 0;
            result = D3D12_DESCRIPTOR_ID_NONE;
            firstResult = D3D12_DESCRIPTOR_ID_NONE;
            continue;
        }

        for (int32_t j = 0, mask = 1; j < 32; ++j, mask <<= 1)
        {
            if (!(flag & mask))
            {
                pHeap->pFlags[i] |= mask;
                result = i * 32 + j;

                ASSERT(result != D3D12_DESCRIPTOR_ID_NONE && "Out of descriptors");

                if (D3D12_DESCRIPTOR_ID_NONE == firstResult)
                {
                    firstResult = result;
                }

                ++foundCount;
                ++pHeap->mUsedDescriptors;

                if (foundCount == descriptorCount)
                {
                    return firstResult;
                }
            }
            // Non contiguous. Start scanning again from this point
            else if (foundCount)
            {
                return_descriptor_handles_unlocked(pHeap, firstResult, foundCount);
                foundCount = 0;
                result = D3D12_DESCRIPTOR_ID_NONE;
                firstResult = D3D12_DESCRIPTOR_ID_NONE;
            }
        }
    }

    ASSERT(result != D3D12_DESCRIPTOR_ID_NONE && "Out of descriptors");
    return firstResult;
}