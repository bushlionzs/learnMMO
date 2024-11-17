#include "OgreHeader.h"
#include "dx12Shader.h"
#include "dx12RenderSystem.h"
#include "OgreVertexDeclaration.h"
#include "myutils.h"
#include "d3dutil.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "OgreMaterial.h"
#include "dx12Common.h"
#include "D3D12Mappings.h"
#include "shaderManager.h"
#include "OgreResourceManager.h"
#include "dx12Helper.h"


DX12Program::DX12Program(const ShaderInfo& info)
{
    mShaderInfo = info;
}

DX12Program::~DX12Program()
{

}

bool DX12Program::load()
{
    Ogre::ShaderPrivateInfo* privateInfo =
        ShaderManager::getSingleton().getShader(mShaderInfo.shaderName, EngineType_Dx12);

    const D3D_SHADER_MACRO* macro = nullptr;

    std::vector<D3D_SHADER_MACRO> macros;
    if (!mShaderInfo.shaderMacros.empty())
    {
        for (auto& o : mShaderInfo.shaderMacros)
        {
            macros.emplace_back();
            macros.back().Name = o.first.c_str();
            macros.back().Definition = o.second.c_str();
        }

        macros.emplace_back();
        macros.back().Name = NULL;
        macros.back().Definition = NULL;
    }

    macro = macros.data();

    auto res = ResourceManager::getSingleton().getResource(privateInfo->vertexShaderName);

    String* vertexContent = ShaderManager::getSingleton().getShaderContent(privateInfo->vertexShaderName);
    mvsByteCode = d3dUtil::CompileShader(
        *vertexContent, 
        macro, 
        privateInfo->vertexShaderEntryPoint.c_str(), 
        "vs_5_1",
        res->_fullname);

    String* fragContent = ShaderManager::getSingleton().getShaderContent(privateInfo->fragShaderName);

    res = ResourceManager::getSingleton().getResource(privateInfo->fragShaderName);
    mpsByteCode = d3dUtil::CompileShader(
        *fragContent, 
        macro, 
        privateInfo->fragShaderEntryPoint.c_str(), 
        "ps_5_1",
        res->_fullname);

    updateShaderResource(ShaderType::VertexShader);
    updateShaderResource(ShaderType::GeometryShader);
    updateShaderResource(ShaderType::PixelShader);
    return true;
}

void DX12Program::updateInputDesc(VertexDeclaration* vDeclaration)
{
    if (!mInputDesc.empty())
    {
        return;
    }
    const VertexDeclaration::VertexElementList& elementList = vDeclaration->getElementList();

    std::vector<D3D12_INPUT_ELEMENT_DESC> D3delems;

    for (auto& it : mD3d12ShaderInputParameters)
    {
        D3D12_INPUT_ELEMENT_DESC elem = {};
        for (const auto& e : elementList)
        {
            LPCSTR semanticName = D3D12Mappings::getSemanticName(e.getSemantic());
            if (strcmp(semanticName, it.SemanticName) == 0 && e.getIndex() == it.SemanticIndex)
            {
                elem.Format = D3D12Mappings::dx12GetType(e.getType());
                elem.InputSlot = e.getSource();
                elem.AlignedByteOffset = static_cast<WORD>(e.getOffset());
                break;
            }
        }

        if (elem.Format == DXGI_FORMAT_UNKNOWN)
        {
            std::throw_with_nested("No VertexElement for semantic");
        }

        elem.SemanticName = it.SemanticName;
        elem.SemanticIndex = it.SemanticIndex;
        elem.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        elem.InstanceDataStepRate = 0;

        D3delems.push_back(elem);
    }

    D3delems.swap(mInputDesc);
}

void DX12Program::updateShaderResource(Ogre::ShaderType shaderType)
{
    ID3D12ShaderReflection* shaderReflection = nullptr;

    void* byteCode = nullptr;
    uint32_t byteCodeSize = 0;

    uint8_t stageFlags = (uint8_t)ShaderStageFlags::NONE;
    switch (shaderType)
    {
    case ShaderType::VertexShader:
        if (mvsByteCode)
        {
            byteCode = mvsByteCode->GetBufferPointer();
            byteCodeSize = mvsByteCode->GetBufferSize();
            stageFlags = (uint8_t)ShaderStageFlags::VERTEX;
        }
        break;
    case ShaderType::GeometryShader:
        if (mgsByteCode)
        {
            byteCode = mgsByteCode->GetBufferPointer();
            byteCodeSize = mgsByteCode->GetBufferSize();
            stageFlags = (uint8_t)ShaderStageFlags::GEOMETRY;
        }
        break;
    case ShaderType::PixelShader:
        if (mpsByteCode)
        {
            byteCode = mpsByteCode->GetBufferPointer();
            byteCodeSize = mpsByteCode->GetBufferSize();
            stageFlags = (uint8_t)ShaderStageFlags::FRAGMENT;
        }
        break;
    }

    if (byteCode == nullptr)
    {
        return;
    }
    HRESULT hr = D3DReflect(byteCode, byteCodeSize,
        IID_ID3D11ShaderReflection, (void**)&shaderReflection);

    D3D12_SHADER_DESC shaderDesc;
    hr = shaderReflection->GetDesc(&shaderDesc);

    if (shaderType == ShaderType::VertexShader)
    {
        mD3d12ShaderInputParameters.resize(shaderDesc.InputParameters);
        mSerStrings.resize(shaderDesc.InputParameters);
        for (auto i = 0; i < shaderDesc.InputParameters; i++)
        {
            D3D12_SIGNATURE_PARAMETER_DESC& curParam = mD3d12ShaderInputParameters[i];
            shaderReflection->GetInputParameterDesc(i, &curParam);
            mSerStrings[i] = curParam.SemanticName;
            curParam.SemanticName = mSerStrings[i].c_str();
        }
    }
    std::vector<ShaderResource> shaderResourceList;
    int kk = 0;
    for (auto i = 0; i < shaderDesc.BoundResources; i++)
    {
        D3D12_SHADER_INPUT_BIND_DESC desc;
        shaderReflection->GetResourceBindingDesc(i, &desc);
        shaderResourceList.emplace_back();
        auto& back = shaderResourceList.back();
        back.name = desc.Name;
        back.reg = desc.BindPoint;
        back.size = desc.BindCount;
        back.type = desc.Type;
        back.used_stages = stageFlags;
    }

    for (auto& current : shaderResourceList)
    {
        bool have = false;
        for (auto& shaderResource : mShaderResourceList)
        {
            if (current.name == shaderResource.name)
            {
                shaderResource.used_stages |= stageFlags;
                have = true;
                break;
            }
        }

        if (!have)
        {
            mShaderResourceList.push_back(current);
        }
    }

    ReleaseCOM(shaderReflection);
}

void DX12Program::updateRootSignature(ID3D12RootSignature* rootSignature)
{
    rootSignature = rootSignature;
}
