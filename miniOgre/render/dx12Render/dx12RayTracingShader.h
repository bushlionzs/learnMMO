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



    const std::string* getRayGenBlob()
    {
        return mRayGenCode.empty() ? nullptr : &mRayGenCode;
    }

    const std::string* getRayMissBlob()
    {
        return mRayMissCode.empty() ? nullptr : &mRayMissCode;
    }
    const std::string* getClosethitBlob()
    {
        return mRayClosethitCode.empty() ? nullptr : &mRayClosethitCode;
    }

    const std::string* getAnyhitBlob()
    {
        return mRayAnyhitCode.empty() ? nullptr : &mRayAnyhitCode;
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

    const DescriptorInfo* getDescriptor(const char* descriptorName);


    uint32_t getCbvSrvUavDescCount(uint32_t set);

    uint32_t getSamplerCount(uint32_t set);
private:
    void updateSetIndex();
    bool load(const RaytracingShaderInfo& shaderInfo);
    bool loadhlsl(const RaytracingShaderInfo& shaderInfo);
    void parseShaderInfo();
private:

    std::vector <ShaderResource> mProgramResourceList;

    std::string mRayGenCode;
    std::string mRayMissCode;
    std::string mRayClosethitCode;
    std::string mRayAnyhitCode;

    ID3D12RootSignature* mRootSignature;
    std::map<std::string, DescriptorInfo> mDescriptorInfoMap;

};
