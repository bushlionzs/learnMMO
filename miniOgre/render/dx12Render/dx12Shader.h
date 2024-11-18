#pragma once
#include <d3d12.h>
#include <d3d12shader.h>
#include <dx12Common.h>
#include <OgreCommon.h>
#include "engine_struct.h"
#include "shaderReflection.h"
class Dx12RenderSystem;
class VertexDeclaration;



class DX12Program: public HwProgram
{
public:
    typedef std::vector<D3D12_SIGNATURE_PARAMETER_DESC> D3d12ShaderParameters;
public:
    DX12Program(const ShaderInfo& info);
    // activate the shader

    ~DX12Program();

    

    ID3DBlob* getVsBlob()
    {
        return mvsByteCode;
    }

    ID3DBlob* getGsBlob()
    {
        return mgsByteCode;
    }
    ID3DBlob* getPsBlob()
    {
        return mpsByteCode;
    }
    
    ID3D12RootSignature* getRootSignature()
    {
        return mRootSignature;
    }   

    const std::array<D3D12_INPUT_ELEMENT_DESC, 10>& getInputDesc()
    {
        return mInputDesc;
    }

    uint32_t getInputDescSize()
    {
        return mInputSize;
    }

    int32_t getRootParamIndex(const std::string& name);

    static std::vector<ShaderResource> parseShaderResource(
        ShaderStageFlags stageFlags,
        void* byteCode, 
        uint32_t byteCodeSize);
    void updateInputDesc(VertexDeclaration* vDeclaration);

    void updateRootSignature(ID3D12RootSignature* rootSignature);
    void updateNameMapping(const std::map<std::string, uint32_t>& rootParamMap);
private:
    bool load(const ShaderInfo& info);
private:
    std::array<D3D12_INPUT_ELEMENT_DESC, 10> mInputDesc;
    uint32_t mInputSize = 0;
    
    ID3DBlob* mvsByteCode = nullptr;
    ID3DBlob* mgsByteCode = nullptr;
    ID3DBlob* mpsByteCode = nullptr;
    ID3D12RootSignature* mRootSignature;

    uint32_t mObjectCBSize = 0;
    std::map<std::string, uint32_t>* mRootParamMap = nullptr;
};
