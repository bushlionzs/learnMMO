#pragma once
#include <d3d12.h>
#include <d3d12shader.h>
#include <dx12Common.h>
#include <OgreCommon.h>
#include "engine_struct.h"
#include "shaderReflection.h"
class Dx12RenderSystem;
class VertexDeclaration;



class DX12ProgramImpl
{
public:
    
public:
    DX12ProgramImpl(const ShaderInfo& info, VertexDeclaration* decl);
    ~DX12ProgramImpl();

    

    const std::string& getVsBlob()
    {
        return mVertexByteCode;
    }

    const std::string& getGsBlob()
    {
        return mGeometryByteCode;
    }
    const std::string& getPsBlob()
    {
        return mFragByteCode;
    }

    const std::string& getComputeBlob()
    {
        return mComputeByteCode;
    }
    
    ID3D12RootSignature* getRootSignature()
    {
        return mRootSignature;
    }   

    const std::vector<D3D12_INPUT_ELEMENT_DESC>& getInputDesc()
    {
        return mInputDesc;
    }

    uint32_t getInputDescSize()
    {
        return mInputSize;
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
    bool load(const ShaderInfo& shaderInfo);
    bool loadglsl(const ShaderInfo& shaderInfo);
    bool loadhlsl(const ShaderInfo& shaderInfo);
    void parseShaderInfo();
    void updateInputDesc(VertexDeclaration* vDeclaration);
private:
    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputDesc;
    std::vector <ShaderResource> mProgramResourceList;
    uint32_t mInputSize = 0;
    std::string mVertexByteCode;
    std::string mGeometryByteCode;
    std::string mFragByteCode;
    std::string mComputeByteCode;

    ID3D12RootSignature* mRootSignature;
    std::map<std::string, DescriptorInfo> mDescriptorInfoMap;

    D3d12ShaderParameters mShaderInputParameters;
};
