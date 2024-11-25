#pragma once
#include "dx12Common.h"
namespace D3D12MA
{
    class Allocator;
}

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL    ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN ((D3D12_GPU_VIRTUAL_ADDRESS)-1)
#define D3D12_REQ_CONSTANT_BUFFER_SIZE    (D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16u)
#define D3D12_DESCRIPTOR_ID_NONE          ((int32_t)-1)

#define MAX_COMPILE_ARGS                  64


class DxMemoryAllocator
{
public:
    DxMemoryAllocator(ID3D12Device* device);
private:
    D3D12MA::Allocator* mAllocator;
};

D3D12_CPU_DESCRIPTOR_HANDLE descriptor_id_to_cpu_handle(
    DescriptorHeap* pHeap, DxDescriptorID id);

D3D12_GPU_DESCRIPTOR_HANDLE descriptor_id_to_gpu_handle(
    DescriptorHeap* pHeap, DxDescriptorID id);


void return_descriptor_handles_unlocked(DescriptorHeap* pHeap, DxDescriptorID handle, uint32_t count);

void return_descriptor_handles(DescriptorHeap* pHeap, DxDescriptorID handle, uint32_t count);

DxDescriptorID consume_descriptor_handles(DescriptorHeap* pHeap, uint32_t descriptorCount);
