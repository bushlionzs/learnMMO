#pragma once
#include <DriverBase.h>
#include "dx12Common.h"
class DxMemoryAllocator;

struct DX12BufferObject : public HwBufferObject {
    DX12BufferObject(
        DescriptorHeapContext* context,
        BufferObjectBinding bufferObjectBinding,
        ResourceMemoryUsage memoryUsage,
        uint32_t bufferCreationFlags,
        uint32_t byteCount,
        DxDescriptorID id
        );
    void copyData(ID3D12GraphicsCommandList* cmdList, const char* data, uint32_t size);
    D3D12_GPU_VIRTUAL_ADDRESS getGPUVirtualAddress();
    D3D12_CPU_DESCRIPTOR_HANDLE getGpuHandle()
    {
        return mGpuHandle;
    }
    ResourceMemoryUsage getMemoryUsage()
    {
        return mMemoryUsage;
    }

    BufferObjectBinding getBufferObjectBinding()
    {
        return mBufferObjectBinding;
    }

    void* lock(uint32_t offset, uint32_t numBytes);
    void unlock(ID3D12GraphicsCommandList* cmdList);

    ID3D12Resource* getResource()
    {
        return BufferGPU.Get();
    }

    DxDescriptorID getDescriptorID()
    {
        return mDescriptorID;
    }

    uint32_t getByteCount()
    {
        return mByteCount;
    }
private:
    DxMemoryAllocator* mAllocator;
    BufferObjectBinding mBufferObjectBinding;
    ResourceMemoryUsage mMemoryUsage;
    BufferObjectBinding bindingType;
    D3D12_RANGE mRange;
    ComPtr<ID3D12Resource> BufferGPU = nullptr;
    ComPtr<ID3D12Resource> BufferUploader = nullptr;

    D3D12_CPU_DESCRIPTOR_HANDLE mGpuHandle;

    DxDescriptorID mDescriptorID;
    DescriptorHeapContext* mDescriptorHeapContext;

    uint32_t mByteCount;
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

class DX12ProgramImpl;
class VertexDeclaration;
struct DX12Program : public HwProgram
{
public:
    DX12Program(const ShaderInfo& info, VertexDeclaration* decl);

    DX12ProgramImpl* getProgramImpl()
    {
        return mProgramImpl;
    }
private:
    DX12ProgramImpl* mProgramImpl;
};

struct DX12ComputeProgram : public HwComputeProgram
{
public:
    DX12ComputeProgram(const ShaderInfo& info);
    ~DX12ComputeProgram();

    DX12ProgramImpl* getProgramImpl()
    {
        return mProgramImpl;
    }
private:
    DX12ProgramImpl* mProgramImpl;
};

struct DX12Sampler : public HwSampler
{
public:
    DX12Sampler(DxDescriptorID id)
        :mDescriptorID(id)
    {

    }

    DxDescriptorID getDescriptorID()
    {
        return mDescriptorID;
    }
private:
    DxDescriptorID mDescriptorID;
};

struct DX12DescriptorSet : public HwDescriptorSet
{
public:
    DX12DescriptorSet(DX12ProgramImpl* program, uint32_t set);
    ~DX12DescriptorSet();


    DX12ProgramImpl* getProgram()
    {
        return mProgram;
    }

    DxDescriptorID getCbvSrvUavHandle()
    {
        return mCbvSrvUavHandle;
    }

    void updateCbvSrvUavHandle(DxDescriptorID cbvSrvUavHandle, uint32_t cbvSrvUavDescCount)
    {
        mCbvSrvUavHandle = cbvSrvUavHandle;

        mCbvSrvUavDescCount = cbvSrvUavDescCount;
    }

    DxDescriptorID getSamplerHandle()
    {
        return mSamplerHandle;
    }
    void updateSamplerHandle(DxDescriptorID samplerHanlde, uint32_t samplerCount)
    {
        mSamplerHandle = samplerHanlde;
        mSamplerCount = samplerCount;
    }

    void addDescriptroInfo(const DescriptorInfo* descriptroInfo)
    {
        mDescriptorInfos.push_back(descriptroInfo);
    }

    std::vector<const DescriptorInfo*> getDescriptorInfos()
    {
        return mDescriptorInfos;
    }
private:
    DX12ProgramImpl* mProgram;
    uint32_t mSet;

    DxDescriptorID       mCbvSrvUavHandle;
    uint32_t mCbvSrvUavDescCount;

    DxDescriptorID mSamplerHandle;
    uint32_t mSamplerCount;
    std::vector<const DescriptorInfo*> mDescriptorInfos;
};