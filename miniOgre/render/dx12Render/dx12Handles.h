#pragma once
#include <DriverBase.h>
#include "dx12Common.h"
class DxMemoryAllocator;

struct DX12BufferObject : public HwBufferObject {
    DX12BufferObject(
        DxMemoryAllocator* allocator,
        BufferObjectBinding bufferObjectBinding,
        ResourceMemoryUsage memoryUsage,
        uint32_t bufferCreationFlags,
        uint32_t byteCount
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
        assert(false);
        return mDescriptorID;
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

struct DX12DescriptorSetLayout : public HwDescriptorSetLayout
{
public:
    DX12DescriptorSetLayout():mRootSignature(nullptr){}
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


struct DX12DescriptorSet : public HwDescriptorSet
{
public:
    DX12DescriptorSet();
    ~DX12DescriptorSet();

    void updateInfo(Handle<HwProgram> programHandle,
        uint32_t set)
    {
        mProgramHandle = programHandle;
        mSet = set;
    }


    Handle<HwProgram> getProgramHandle()
    {
        return mProgramHandle;
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
private:
    Handle<HwProgram> mProgramHandle;
    uint32_t mSet;

    DxDescriptorID       mCbvSrvUavHandle;
    uint32_t mCbvSrvUavDescCount;
};