#pragma once
#include <d3d12.h>
#include <dx12Common.h>
#include <OgreCommon.h>
#include "engine_struct.h"
#include "dx12Shader.h"
class Dx12RenderSystem;
class VertexDeclaration;



class DX12RayTracingProgramImpl:public DX12ProgramBase
{
public:

public:
    DX12RayTracingProgramImpl(const RaytracingShaderInfo& info);
    ~DX12RayTracingProgramImpl();


    const std::string* getRayTracingBlob()
    {
        return mRayTracingCode.empty() ? nullptr:&mRayTracingCode;
    }
    
    ID3D12RootSignature* getRootSignature()
    {
        return mRootSignature;
    }

    static std::vector<ShaderResource> parseShaderResource(
        ShaderStageFlags stageFlags,
        void* byteCode,
        uint32_t byteCodeSize);


    void updateRootSignature(ID3D12RootSignature* rootSignature);

    void createRaytracingPipelineStateObject(ID3D12Device5* prDevice, const RaytracingShaderInfo& info);

    void buildShaderTables(const RaytracingShaderInfo& info);
    const DescriptorInfo* getDescriptor(const char* descriptorName);


    uint32_t getCbvSrvUavDescCount(uint32_t set);

    uint32_t getSamplerCount(uint32_t set);

    ID3D12Resource* getMissShaderTable()
    {
        return m_missShaderTable.Get();
    }

    ID3D12Resource* getHitGroupShaderTable()
    {
        return m_hitGroupShaderTable.Get();
    }

    ID3D12Resource* getRayGenShaderTable()
    {
        return m_rayGenShaderTable.Get();
    }

    ID3D12StateObject* getDxrStateObject()
    {
        return mDxrStateObject.Get();
    }
private:
    void updateSetIndex();
    bool load(const RaytracingShaderInfo& shaderInfo);
    bool loadhlsl(const RaytracingShaderInfo& shaderInfo);
    void parseShaderInfo(const RaytracingShaderInfo& info);
private:

    std::vector <ShaderResource> mProgramResourceList;

    std::string mRayTracingCode;

    ID3D12RootSignature* mRootSignature;


    RaytracingShaderInfo mRaytracingShaderInfo;

    ComPtr<ID3D12StateObject> mDxrStateObject;

    ComPtr<ID3D12Resource> m_missShaderTable;
    ComPtr<ID3D12Resource> m_hitGroupShaderTable;
    ComPtr<ID3D12Resource> m_rayGenShaderTable;

    const wchar_t* c_hitGroupName = L"MyHitGroup";
};
