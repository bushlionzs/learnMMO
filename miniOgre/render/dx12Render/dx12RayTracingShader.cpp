#include "OgreHeader.h"
#include "dx12RayTracingShader.h"
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


DX12RayTracingProgramImpl::DX12RayTracingProgramImpl(
    const RaytracingShaderInfo& info)
{
    load(info);
    parseShaderInfo();
}

DX12RayTracingProgramImpl::~DX12RayTracingProgramImpl()
{

}

bool DX12RayTracingProgramImpl::load(const RaytracingShaderInfo& shaderInfo)
{
    return loadhlsl(shaderInfo);
}


bool DX12RayTracingProgramImpl::loadhlsl(const RaytracingShaderInfo& shaderInfo)
{
    {
        String* content = ShaderManager::getSingleton().getShaderContent(shaderInfo.rayGenShaderName);
        hlslToBin(shaderInfo.rayGenShaderName, *content, shaderInfo.rayGenEntryName,
            shaderInfo.shaderMacros, Ogre::RayGenShader, mRayGenCode, false);
    }
    
    {
        String* content = ShaderManager::getSingleton().getShaderContent(shaderInfo.rayMissShaderName);
        hlslToBin(shaderInfo.rayMissShaderName, *content, shaderInfo.rayGenEntryName,
            shaderInfo.shaderMacros, Ogre::MissShader, mRayMissCode, false);
    }

    
    
    {
        String* content = ShaderManager::getSingleton().getShaderContent(shaderInfo.rayClosethitShaderName);
        hlslToBin(shaderInfo.rayClosethitShaderName, *content, shaderInfo.rayGenEntryName,
            shaderInfo.shaderMacros, Ogre::ClosestHitShader, mRayClosethitCode, false);
    }

    
    {
        String* content = ShaderManager::getSingleton().getShaderContent(shaderInfo.rayAnyHitShaderName);
        hlslToBin(shaderInfo.rayAnyHitShaderName, *content, shaderInfo.rayGenEntryName,
            shaderInfo.shaderMacros, Ogre::AnyHitShader, mRayAnyhitCode, false);
    }

   
    return true;
}

static auto updateResourceList = [](std::vector <ShaderResource>& programResourceList,
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

void DX12RayTracingProgramImpl::parseShaderInfo()
{
    {
        if (!mRayGenCode.empty())
        {
            auto resourceList = DX12Helper::getSingleton().parseShaderResource(ShaderStageFlags::RAYGEN,
                mRayGenCode.c_str(), mRayGenCode.size());
            updateResourceList(mProgramResourceList, resourceList, ShaderStageFlags::RAYGEN);
        }
    }

    {

        if (!mRayMissCode.empty())
        {
            auto resourceList = DX12Helper::getSingleton().parseShaderResource(ShaderStageFlags::RAYMISS,
                mRayMissCode.c_str(), mRayMissCode.size());
            updateResourceList(mProgramResourceList, resourceList, ShaderStageFlags::RAYMISS);
        }
    }

    {
        if (!mRayClosethitCode.empty())
        {
            auto resourceList = DX12Helper::getSingleton().parseShaderResource(ShaderStageFlags::RAYCLOSETHIT,
                mRayClosethitCode.c_str(), mRayClosethitCode.size());
            updateResourceList(mProgramResourceList, resourceList, ShaderStageFlags::RAYCLOSETHIT);
        }
    }

    {
        if (!mRayAnyhitCode.empty())
        {
            auto resourceList = DX12Helper::getSingleton().parseShaderResource(ShaderStageFlags::RAYANYHIT,
                mRayAnyhitCode.c_str(), mRayAnyhitCode.size());
            updateResourceList(mProgramResourceList, resourceList, ShaderStageFlags::RAYANYHIT);
        }
    }

    std::sort(mProgramResourceList.begin(), mProgramResourceList.end(),
        [](const ShaderResource& a, const ShaderResource& b) {
            return a.set > b.set;
        });

    updateSetIndex();

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
            shaderResource.type == D3D_SIT_UAV_RWBYTEADDRESS ||
            shaderResource.type == D3D_SIT_STRUCTURED ||
            shaderResource.type == D3D_SIT_UAV_RWSTRUCTURED ||
            shaderResource.type == D3D_SIT_UAV_RWTYPED)
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

std::vector<ShaderResource> DX12RayTracingProgramImpl::parseShaderResource(
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

void DX12RayTracingProgramImpl::updateRootSignature(ID3D12RootSignature* rootSignature)
{
    mRootSignature = rootSignature;
}

const DescriptorInfo* DX12RayTracingProgramImpl::getDescriptor(const char* descriptorName)
{
    auto itor = mDescriptorInfoMap.find(descriptorName);
    if (itor != mDescriptorInfoMap.end())
    {
        return &itor->second;
    }

    return nullptr;
}

void DX12RayTracingProgramImpl::updateSetIndex()
{
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

                    index += shaderSource.size;

                    if (shaderSource.type == D3D_SIT_UAV_RWTYPED)
                    {
                        index += D3D12_MAX_MIPMAP_COUNT;
                    }
                }
            }
        }
    }
}

uint32_t DX12RayTracingProgramImpl::getCbvSrvUavDescCount(uint32_t set)
{
    uint32_t count = 0;
    for (auto& shaderSource : mProgramResourceList)
    {
        if (shaderSource.set == set)
        {
            if (shaderSource.type != D3D_SIT_SAMPLER)
            {
                count += shaderSource.size;

                if (shaderSource.type == D3D_SIT_UAV_RWTYPED)
                {
                    count += D3D12_MAX_MIPMAP_COUNT;
                }
            }
        }
    }

    return count;
}

uint32_t DX12RayTracingProgramImpl::getSamplerCount(uint32_t set)
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
