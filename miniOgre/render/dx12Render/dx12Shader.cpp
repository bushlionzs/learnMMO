#include "OgreHeader.h"
#include "dx12Shader.h"
#include "dx12RenderSystem.h"
#include "OgreVertexDeclaration.h"
#include "OgreResourceManager.h"
#include "myutils.h"
#include "d3dutil.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "OgreMaterial.h"
#include "dx12Common.h"
#include "D3D12Mappings.h"
#include "shaderManager.h"
#include "dx12Helper.h"
#include "hlslUtil.h"


DX12ProgramImpl::DX12ProgramImpl(
    const ShaderInfo& info,
    VertexDeclaration* decl)
{
    load(info);
    updateInputDesc(decl);
    parseShaderInfo();
}

DX12ProgramImpl::~DX12ProgramImpl()
{

}

bool DX12ProgramImpl::load(const ShaderInfo& shaderInfo)
{
    Ogre::ShaderPrivateInfo* privateInfo =
        ShaderManager::getSingleton().getShader(shaderInfo.shaderName, EngineType_Dx12);
    std::string* shaderName = &privateInfo->vertexShaderName;
    if (privateInfo->vertexShaderName.empty())
    {
        shaderName = &privateInfo->computeShaderName;
    }
    const char* suffix = getSuffix(*shaderName);

    bool glsl = false;
    if (strcmp(suffix, ".glsl") == 0)
    {
        glsl = true;
    }
    
    if (glsl)
    {
        return loadglsl(shaderInfo);
    }

    return loadhlsl(shaderInfo);
}

bool DX12ProgramImpl::loadglsl(const ShaderInfo& shaderInfo)
{
    Ogre::ShaderPrivateInfo* privateInfo =
        ShaderManager::getSingleton().getShader(shaderInfo.shaderName, EngineType_Dx12);

    auto res = ResourceManager::getSingleton().getResource(privateInfo->vertexShaderName);

    if (res)
    {
        String* vertexContent = ShaderManager::getSingleton().getShaderContent(privateInfo->vertexShaderName);
        mVertexByteCode.clear();
        d3dUtil::CompileGlslShader(
            *vertexContent,
            shaderInfo.shaderMacros,
            privateInfo->vertexShaderEntryPoint,
            Ogre::ShaderType::VertexShader,
            res->_fullname,
            mVertexByteCode);
    }
    

    res = ResourceManager::getSingleton().getResource(privateInfo->fragShaderName);

    if (res)
    {
        String* fragContent = ShaderManager::getSingleton().getShaderContent(privateInfo->fragShaderName);
        mFragByteCode.clear();
        d3dUtil::CompileGlslShader(
            *fragContent,
            shaderInfo.shaderMacros,
            privateInfo->fragShaderEntryPoint,
            Ogre::ShaderType::PixelShader,
            res->_fullname,
            mFragByteCode);
    }
    
    res = ResourceManager::getSingleton().getResource(privateInfo->geometryShaderName);

    if (res)
    {
        String* Content = ShaderManager::getSingleton().getShaderContent(privateInfo->geometryShaderName);
        mGeometryByteCode.clear();
        d3dUtil::CompileGlslShader(
            *Content,
            shaderInfo.shaderMacros,
            privateInfo->geometryShaderEntryPoint,
            Ogre::ShaderType::GeometryShader,
            res->_fullname,
            mGeometryByteCode);
    }

    res = ResourceManager::getSingleton().getResource(privateInfo->computeShaderName);

    if (res)
    {
        String* Content = ShaderManager::getSingleton().getShaderContent(privateInfo->computeShaderName);
        mComputeByteCode.clear();
        d3dUtil::CompileGlslShader(
            *Content,
            shaderInfo.shaderMacros,
            privateInfo->computeShaderEntryPoint,
            Ogre::ShaderType::ComputeShader,
            res->_fullname,
            mComputeByteCode);
    }

    return true;
}
bool DX12ProgramImpl::loadhlsl(const ShaderInfo& shaderInfo)
{
    Ogre::ShaderPrivateInfo* privateInfo =
        ShaderManager::getSingleton().getShader(shaderInfo.shaderName, EngineType_Dx12);

    auto res = ResourceManager::getSingleton().getResource(privateInfo->vertexShaderName);
    
    if (res)
    {
        String* content = ShaderManager::getSingleton().getShaderContent(privateInfo->vertexShaderName);
        hlslToBin(shaderInfo.shaderName, *content, privateInfo->vertexShaderEntryPoint,
            shaderInfo.shaderMacros, Ogre::VertexShader, mVertexByteCode, false);
        const char* data = mVertexByteCode.c_str();
        int kk = 0;
    }
    
    res = ResourceManager::getSingleton().getResource(privateInfo->fragShaderName);
    if (res)
    {
        String* content = ShaderManager::getSingleton().getShaderContent(privateInfo->fragShaderName);
        hlslToBin(shaderInfo.shaderName, *content, privateInfo->fragShaderEntryPoint,
            shaderInfo.shaderMacros, Ogre::PixelShader, mFragByteCode, false);
    }
   

    res = ResourceManager::getSingleton().getResource(privateInfo->geometryShaderName);
    if (res)
    {
        String* content = ShaderManager::getSingleton().getShaderContent(privateInfo->geometryShaderName);
        hlslToBin(shaderInfo.shaderName, *content, privateInfo->geometryShaderEntryPoint,
            shaderInfo.shaderMacros, Ogre::GeometryShader, mGeometryByteCode, false);
    }

    res = ResourceManager::getSingleton().getResource(privateInfo->computeShaderName);
    if (res)
    {
        String* content = ShaderManager::getSingleton().getShaderContent(privateInfo->computeShaderName);
        hlslToBin(shaderInfo.shaderName, *content, privateInfo->computeShaderEntryPoint,
            shaderInfo.shaderMacros, Ogre::ComputeShader, mComputeByteCode, false);
    }
    return true;
}

auto updateResourceList = [](std::vector <ShaderResource>& programResourceList,
    std::vector <ShaderResource>& resourceList, ShaderStageFlags stageFlags)
    {
        for (auto& current : resourceList)
        {
            bool have = false;
            for (auto& shaderResource : programResourceList)
            {
                if (current.name == shaderResource.name)
                {
                    shaderResource.used_stages |= (uint8_t)stageFlags;
                    have = true;
                    break;
                }
            }

            if (!have)
            {
                programResourceList.push_back(current);
            }
        }
    };

void DX12ProgramImpl::parseShaderInfo()
{
    {
        if (!mVertexByteCode.empty())
        {
            auto resourceList = DX12Helper::getSingleton().parseShaderResource(ShaderStageFlags::VERTEX,
                mVertexByteCode.c_str(), mVertexByteCode.size());
            updateResourceList(mProgramResourceList, resourceList, ShaderStageFlags::VERTEX);
        }
    }

    {

        if (!mGeometryByteCode.empty())
        {
            auto resourceList = DX12Helper::getSingleton().parseShaderResource(ShaderStageFlags::GEOMETRY,
                mGeometryByteCode.c_str(), mGeometryByteCode.size());
            updateResourceList(mProgramResourceList, resourceList, ShaderStageFlags::GEOMETRY);
        }
    }

    {
        if (!mFragByteCode.empty())
        {
            auto resourceList = DX12Helper::getSingleton().parseShaderResource(ShaderStageFlags::FRAGMENT,
                mFragByteCode.c_str(), mFragByteCode.size());
            updateResourceList(mProgramResourceList, resourceList, ShaderStageFlags::FRAGMENT);
        }
    }

    {
        if (!mComputeByteCode.empty())
        {
            auto resourceList = DX12Helper::getSingleton().parseShaderResource(ShaderStageFlags::COMPUTE,
                mComputeByteCode.c_str(), mComputeByteCode.size());
            updateResourceList(mProgramResourceList, resourceList, ShaderStageFlags::COMPUTE);
        }
    }

    std::sort(mProgramResourceList.begin(), mProgramResourceList.end(),
        [](const ShaderResource& a, const ShaderResource& b) {
            return a.set > b.set;
        });

    for (uint32_t i = 0; i < 4; i++)
    {
        uint32_t index = 0;
        uint32_t samplerIndex = 0;
        for (auto& shaderSource : mProgramResourceList)
        {
            if (shaderSource.set == i)
            {
                if (shaderSource.type == D3D_SIT_SAMPLER)
                {
                    shaderSource.set_index = samplerIndex;
                    samplerIndex += shaderSource.size;
                }
                else
                {
                    shaderSource.set_index = index;
                    if (shaderSource.size == 5)
                    {
                        int kk = 0;
                    }
                    index += shaderSource.size;
                }
                
            }
        }
    }
    D3D12_ROOT_PARAMETER1      rootParams[D3D12_MAX_ROOT_COST] = {};
    UINT rootParamCount = 0;
    constexpr uint32_t         kMaxResourceTableSize = 32;
    static constexpr uint32_t kMaxLayoutCount = 16;
    D3D12_DESCRIPTOR_RANGE1    cbvSrvUavRange[kMaxLayoutCount][kMaxResourceTableSize] = {};
    D3D12_DESCRIPTOR_RANGE1    samplerRange[kMaxLayoutCount][kMaxResourceTableSize] = {};
    uint32_t index = 0;
    for (auto& shaderResource : mProgramResourceList)
    {
        if (shaderResource.type == D3D_SIT_TEXTURE ||
            shaderResource.type == D3D_SIT_CBUFFER ||
            shaderResource.type == D3D_SIT_SAMPLER ||
            shaderResource.type == D3D_SIT_BYTEADDRESS ||
            shaderResource.type == D3D_SIT_UAV_RWBYTEADDRESS||
            shaderResource.type == D3D_SIT_STRUCTURED ||
            shaderResource.type == D3D_SIT_UAV_RWSTRUCTURED)
        {
            d3dUtil::create_descriptor_table(shaderResource.size,
                &shaderResource, cbvSrvUavRange[index], &rootParams[rootParamCount]);
            
        }
        else
        {
            assert(false);
        }

        index++;
        auto& DescriptorInfo = mDescriptorInfoMap[shaderResource.name];
        DescriptorInfo.pName = shaderResource.name.c_str();
        DescriptorInfo.mRootIndex = rootParamCount;
        DescriptorInfo.mType = shaderResource.type;
        DescriptorInfo.mSet = shaderResource.set;
        DescriptorInfo.mSetIndex = shaderResource.set_index;
        rootParamCount++;
    }

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    
    if (mVertexByteCode.empty())
    {
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    }
    else
    {
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    }
    if (mFragByteCode.empty())
    {
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
    }
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    rootSigDesc.Desc_1_1.NumParameters = rootParamCount;
    rootSigDesc.Desc_1_1.pParameters = rootParams;
    rootSigDesc.Desc_1_1.NumStaticSamplers = 0;
    rootSigDesc.Desc_1_1.pStaticSamplers = nullptr;
    rootSigDesc.Desc_1_1.pStaticSamplers = nullptr;
    rootSigDesc.Desc_1_1.Flags = rootSignatureFlags;


    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSigDesc,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);
    auto device = DX12Helper::getSingleton().getDevice();
    ThrowIfFailed(device->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&mRootSignature)));
}


void DX12ProgramImpl::updateInputDesc(VertexDeclaration* vDeclaration)
{
    if (!mInputDesc.empty())
    {
        return;
    }

    if (vDeclaration == nullptr)
    {
        return;
    }
    const VertexDeclaration::VertexElementList& elementList = vDeclaration->getElementList();

    DX12Helper::getSingleton().parseInputParams(mVertexByteCode.c_str(), mVertexByteCode.size(), mShaderInputParameters);
    

    
    uint32_t index = 0;
    for (auto& it : mShaderInputParameters)
    {
        D3D12_INPUT_ELEMENT_DESC elem = {};
        for (const auto& e : elementList)
        {
            LPCSTR semanticName = D3D12Mappings::getSemanticName(e.getSemantic());
            if (strcmp(semanticName, it.name) == 0 && e.getIndex() == it.semanticIndex)
            {
                elem.Format = D3D12Mappings::dx12GetType(e.getType());
                elem.InputSlot = e.getSource();
                elem.AlignedByteOffset = static_cast<WORD>(e.getOffset());
                break;
            }
        }

        if (elem.Format == DXGI_FORMAT_UNKNOWN)
        {
            assert(false);
        } 
        elem.SemanticName = it.name;
        elem.SemanticIndex = it.semanticIndex;
        elem.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        elem.InstanceDataStepRate = 0;
        mInputDesc.push_back(elem);
        index++;
    }
    mInputSize = index;
}

std::vector<ShaderResource> DX12ProgramImpl::parseShaderResource(
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
        if (strcmp(desc.Name, "$Globals") == 0)
        {
            continue;
        }
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

void DX12ProgramImpl::updateRootSignature(ID3D12RootSignature* rootSignature)
{
    mRootSignature = rootSignature;
}

const DescriptorInfo* DX12ProgramImpl::getDescriptor(const char* descriptorName)
{
    auto itor = mDescriptorInfoMap.find(descriptorName);
    if (itor != mDescriptorInfoMap.end())
    {
        return &itor->second;
    }

    return nullptr;
}

uint32_t DX12ProgramImpl::getCbvSrvUavDescCount(uint32_t set)
{
    uint32_t count = 0;
    for (auto& shaderSource : mProgramResourceList)
    {
        if (shaderSource.set == set && shaderSource.type != D3D_SIT_SAMPLER)
        {
            count += shaderSource.size;
        }
    }

    return count;
}

uint32_t DX12ProgramImpl::getSamplerCount(uint32_t set)
{
    uint32_t count = 0;
    for (auto& shaderSource : mProgramResourceList)
    {
        if (shaderSource.set == set && shaderSource.type == D3D_SIT_SAMPLER)
        {
            count += shaderSource.size;
        }
    }

    return count;
}
