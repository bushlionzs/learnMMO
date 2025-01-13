#include "OgreHeader.h"
#include "dx12RayTracingShader.h"
#include "dx12RenderSystem.h"
#include "OgreVertexDeclaration.h"
#include "OgreResourceManager.h"
#include "myutils.h"
#include "d3dutil.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string_util.h>
#include "OgreMaterial.h"
#include "dx12Common.h"
#include "D3D12Mappings.h"
#include "shaderManager.h"
#include "dx12Helper.h"
#include "hlslUtil.h"
#include "DirectXRaytracingHelper.h"


DX12RayTracingProgramImpl::DX12RayTracingProgramImpl(
    const RaytracingShaderInfo& info)
{
    load(info);
    parseShaderInfo(info);
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
        String* content = ShaderManager::getSingleton().getShaderContent(shaderInfo.rayTracingShaderName);
        hlslToBin(shaderInfo.rayGenShaderName, *content, shaderInfo.rayGenEntryName,
            shaderInfo.shaderMacros, Ogre::RayGenShader, mRayTracingCode, false);
    }

    return true;
}



void DX12RayTracingProgramImpl::parseShaderInfo(const RaytracingShaderInfo& info)
{
    {
        if (!mRayTracingCode.empty())
        {
            std::vector<DX12Helper::StageFlagsInfo> flagsInfoList;
            if (!info.rayGenEntryName.empty())
            {
                flagsInfoList.push_back({ info.rayGenEntryName.c_str(), ShaderStageFlags::RAYGEN });
            }

            if (!info.rayMissEntryName.empty())
            {
                flagsInfoList.push_back({ info.rayMissEntryName.c_str(), ShaderStageFlags::RAYMISS });
            }

            if (!info.rayClosethitEntryName.empty())
            {
                flagsInfoList.push_back({ info.rayClosethitEntryName.c_str(), ShaderStageFlags::RAYCLOSETHIT });
            }

            if (!info.rayAnyHitEntryName.empty())
            {
                flagsInfoList.push_back({ info.rayAnyHitEntryName.c_str(), ShaderStageFlags::RAYANYHIT });
            }
            
            mProgramResourceList = DX12Helper::getSingleton().parseShaderResource2(flagsInfoList,
                mRayTracingCode.c_str(), mRayTracingCode.size());
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
            shaderResource.type == D3D_SIT_UAV_RWTYPED ||
            shaderResource.type == D3D_SIT_RTACCELERATIONSTRUCTURE)
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

void DX12RayTracingProgramImpl::createRaytracingPipelineStateObject(
    ID3D12Device5* prDevice, const RaytracingShaderInfo& shaderInfo)
{
    CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

    CD3DX12_DXIL_LIBRARY_SUBOBJECT* lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

    const std::string* blob = getRayTracingBlob();

    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)blob->data(), blob->size());

    lib->SetDXILLibrary(&libdxil);

    std::wstring wRayGenEntryName = dy::acsi_to_widebyte(shaderInfo.rayGenEntryName);
    std::wstring wRayClosethitEntryName = dy::acsi_to_widebyte(shaderInfo.rayClosethitEntryName);
    std::wstring wRayMissEntryName = dy::acsi_to_widebyte(shaderInfo.rayMissEntryName);

    std::vector<LPCWCHAR> exportNames;
    exportNames.push_back(wRayGenEntryName.c_str());
    exportNames.push_back(wRayClosethitEntryName.c_str());
    exportNames.push_back(wRayMissEntryName.c_str());
    {
        lib->DefineExport(wRayGenEntryName.c_str());
        lib->DefineExport(wRayClosethitEntryName.c_str());
        lib->DefineExport(wRayMissEntryName.c_str());
    }

    CD3DX12_HIT_GROUP_SUBOBJECT* hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(wRayClosethitEntryName.c_str());
    
    hitGroup->SetHitGroupExport(c_hitGroupName);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = sizeof(Ogre::Vector4) * 2;    // float4 pixelColor
    UINT attributeSize = sizeof(Ogre::Vector2);  // float2 barycentrics
    shaderConfig->Config(payloadSize, attributeSize);


    ID3D12RootSignature* rootSignature = getRootSignature();


    auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(rootSignature);

    auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();

    UINT maxRecursionDepth = 1;

    pipelineConfig->Config(maxRecursionDepth);

    // Add a state subobject for the association between shaders and the payload

    /*D3D12_STATE_SUBOBJECT* shaderPayloadAssociationObject = raytracingPipeline.CreateSubobject<D3D12_STATE_SUBOBJECT>();
    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderPayloadAssociation = {};
    shaderPayloadAssociation.NumExports = static_cast<UINT>(exportNames.size());
    shaderPayloadAssociation.pExports = exportNames.data();
    shaderPayloadAssociation.pSubobjectToAssociate = shaderPayloadAssociationObject;
    shaderPayloadAssociationObject->Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    shaderPayloadAssociationObject->pDesc = &shaderPayloadAssociation;*/

    // Create the state object.
    ThrowIfFailed(prDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&mDxrStateObject)));
}


void DX12RayTracingProgramImpl::buildShaderTables(const RaytracingShaderInfo& shaderInfo)
{
    void* rayGenShaderIdentifier;
    void* missShaderIdentifier;
    void* hitGroupShaderIdentifier;

    std::wstring wRayGenEntryName = dy::acsi_to_widebyte(shaderInfo.rayGenEntryName);
    std::wstring wRayClosethitEntryName = dy::acsi_to_widebyte(shaderInfo.rayClosethitEntryName);
    std::wstring wRayMissEntryName = dy::acsi_to_widebyte(shaderInfo.rayMissEntryName);

    auto GetShaderIdentifiers = [&](ID3D12StateObjectProperties* stateObjectProperties)
        {
            rayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(wRayGenEntryName.c_str());
            missShaderIdentifier = stateObjectProperties->GetShaderIdentifier(wRayClosethitEntryName.c_str());
            hitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(wRayMissEntryName.c_str());
        };

    // Get shader identifiers.
    UINT shaderIdentifierSize;
    {
       
        ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
        mDxrStateObject.As(&stateObjectProperties);
        rayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(wRayGenEntryName.c_str());
        missShaderIdentifier = stateObjectProperties->GetShaderIdentifier(wRayMissEntryName.c_str());
        hitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_hitGroupName);
        shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    }

    auto device = DX12Helper::getSingleton().getDevice();
    // Ray gen shader table
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable rayGenShaderTable(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
        rayGenShaderTable.push_back(ShaderRecord(rayGenShaderIdentifier, shaderIdentifierSize));
        m_rayGenShaderTable = rayGenShaderTable.GetResource();
    }

    // Miss shader table
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable missShaderTable(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
        missShaderTable.push_back(ShaderRecord(missShaderIdentifier, shaderIdentifierSize));
        m_missShaderTable = missShaderTable.GetResource();
    }

    // Hit group shader table
    {
        
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");
        hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderIdentifier, shaderIdentifierSize));
        m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
    }
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
