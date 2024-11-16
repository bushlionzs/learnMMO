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

    bool load();

    ID3DBlob* getVsBlob()
    {
        return mvsByteCode.Get();
    }

    ID3DBlob* getGsBlob()
    {
        return mgsByteCode.Get();
    }
    ID3DBlob* getPsBlob()
    {
        return mpsByteCode.Get();
    }
    
    const std::vector<D3D12_INPUT_ELEMENT_DESC>& getInputDesc()
    {
        return mInputDesc;
    }

    const std::vector <ShaderResource>& getShaderResourceList()
    {
        return mShaderResourceList;
    }

    void updateInputDesc(VertexDeclaration* vDeclaration);
    void updateShaderResource(Ogre::ShaderType shaderType);
    void updateRootSignature(ID3D12RootSignature* rootSignature);
private:
    std::vector<std::string> mSerStrings;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputDesc;

    ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

    ComPtr<ID3DBlob> mvsByteCode = nullptr;
    ComPtr<ID3DBlob> mgsByteCode = nullptr;
    ComPtr<ID3DBlob> mpsByteCode = nullptr;
    ComPtr<ID3D12RootSignature> rootSignature;
    std::vector<ShaderResource> mShaderResourceList;

    uint32_t mObjectCBSize = 0;

    D3d12ShaderParameters mD3d12ShaderInputParameters;

    ShaderInfo mShaderInfo;

};
