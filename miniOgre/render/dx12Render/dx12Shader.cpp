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
    load(info);
}

DX12Program::~DX12Program()
{

}

bool DX12Program::load(const ShaderInfo& shaderInfo)
{
    Ogre::ShaderPrivateInfo* privateInfo =
        ShaderManager::getSingleton().getShader(shaderInfo.shaderName, EngineType_Dx12);

    const D3D_SHADER_MACRO* macro = nullptr;

    std::vector<D3D_SHADER_MACRO> macros;
    if (!shaderInfo.shaderMacros.empty())
    {
        for (auto& o : shaderInfo.shaderMacros)
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

    return true;
}

int32_t DX12Program::getRootParamIndex(const std::string& name)
{
    auto itor = mRootParamMap->find(name);
    if (itor != mRootParamMap->end())
    {
        return itor->second;
    }
    assert(false);
    return -1;
}

void DX12Program::updateInputDesc(VertexDeclaration* vDeclaration)
{
    if (!mInputDesc.empty())
    {
        return;
    }
    const VertexDeclaration::VertexElementList& elementList = vDeclaration->getElementList();

    D3d12ShaderParameters shaderInputParameters;
    ID3D12ShaderReflection* shaderReflection = nullptr;
    void* byteCode = mvsByteCode->GetBufferPointer();
    uint32_t byteCodeSize = mvsByteCode->GetBufferSize();
    HRESULT hr = D3DReflect(byteCode, byteCodeSize,
        IID_ID3D11ShaderReflection, (void**)&shaderReflection);

    D3D12_SHADER_DESC shaderDesc;
    hr = shaderReflection->GetDesc(&shaderDesc);

    shaderInputParameters.resize(shaderDesc.InputParameters);

    for (auto i = 0; i < shaderDesc.InputParameters; i++)
    {
        D3D12_SIGNATURE_PARAMETER_DESC& curParam = shaderInputParameters[i];
        shaderReflection->GetInputParameterDesc(i, &curParam);
    }

    
    uint32_t index = 0;
    for (auto& it : shaderInputParameters)
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

        uint32_t  semanticSize = strlen(it.SemanticName);
        char* data = (char*)malloc(semanticSize + 1);
        if (data)
        {
            memcpy((void*)data, it.SemanticName, semanticSize);
            data[semanticSize] = 0;
        }
        
        elem.SemanticName = data;
        elem.SemanticIndex = it.SemanticIndex;
        elem.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        elem.InstanceDataStepRate = 0;
        mInputDesc[index] = elem;
        index++;
    }
    mInputSize = index;

    ReleaseCOM(shaderReflection);
}

std::vector<ShaderResource> DX12Program::parseShaderResource(
    ShaderStageFlags stageFlags,
    void* byteCode, 
    uint32_t byteCodeSize)
{

    ID3D12ShaderReflection* shaderReflection = nullptr;

    std::vector<ShaderResource> shaderResourceList;
    if (byteCode == nullptr)
    {
        return shaderResourceList;
    }
    HRESULT hr = D3DReflect(byteCode, byteCodeSize,
        IID_ID3D11ShaderReflection, (void**)&shaderReflection);

    D3D12_SHADER_DESC shaderDesc;
    hr = shaderReflection->GetDesc(&shaderDesc);

    
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
        back.set = desc.Space;
        back.used_stages = (uint8_t)stageFlags;
    }

   

    ReleaseCOM(shaderReflection);

    return shaderResourceList;
}

void DX12Program::updateRootSignature(ID3D12RootSignature* rootSignature)
{
    mRootSignature = rootSignature;
}

void DX12Program::updateNameMapping(const std::map<std::string, uint32_t>& rootParamMap)
{
    if (mRootParamMap == nullptr)
    {
        mRootParamMap = new std::map<std::string, uint32_t>;
    }

    *mRootParamMap = rootParamMap;
}