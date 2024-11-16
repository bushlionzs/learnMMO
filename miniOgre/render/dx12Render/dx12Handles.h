#pragma once
#include <DriverBase.h>
#include "dx12Common.h"


struct DX12BufferObject : public HwBufferObject {
    DX12BufferObject(
        uint32_t byteCount,
        uint32_t bufferCreationFlags);
    void copyData(ID3D12GraphicsCommandList* cmdList, const char* data, uint32_t size);
    D3D12_GPU_VIRTUAL_ADDRESS getGPUVirtualAddress();

    void* lock(uint32_t offset, uint32_t numBytes);
    void unlock(ID3D12GraphicsCommandList* cmdList);
private:
    BufferUsage usage;
    BufferObjectBinding bindingType;

    D3D12_RANGE mRange;
    ComPtr<ID3D12Resource> BufferGPU = nullptr;
    ComPtr<ID3D12Resource> BufferUploader = nullptr;
};

struct DX12Pipeline : public HwPipeline
{
    DX12Pipeline() {}
    ~DX12Pipeline() {}

    void updatePipeline(ID3D12PipelineState* pipeline)
    {
        mPipeline = pipeline;
    }

    ID3D12PipelineState* getPipeline()
    {
        return mPipeline;
    }
private:
    ID3D12PipelineState* mPipeline;
};

struct DX12DescriptorSetLayout : public HwDescriptorSetLayout
{
public:
    DX12DescriptorSetLayout() {}
    ~DX12DescriptorSetLayout() {}

    void updateRootSignature(ID3D12RootSignature* rootSignature)
    {
        mRootSignature = rootSignature;
    }

    ID3D12RootSignature* getRootSignature()
    {
        return mRootSignature;
    }
private:
    ID3D12RootSignature* mRootSignature;
};