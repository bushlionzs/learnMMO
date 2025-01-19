#include <OgreHeader.h>
#include "d3dutil.h"
#include <DriverEnums.h>
#include "dx12Common.h"
#include "OgreResourceManager.h"
#include <platform_file.h>
#include "D3D12Mappings.h"
#include "memoryAllocator.h"
#include "glslUtil.h"
#include "hlslUtil.h"
#include <comdef.h>
#include <fstream>
#include <array>

class CustomD3DInclude : public ID3DInclude {
public:
    // 构造函数和析构函数
    CustomD3DInclude() {}
    virtual ~CustomD3DInclude() {}



    // ID3DInclude methods
    STDMETHODIMP Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override {

        ResourceInfo* resInfo = ResourceManager::getSingleton().getResource(pFileName);

        std::string content;
        get_file_content(resInfo->_fullname.c_str(), content);

        // 将内容复制到堆上，并返回指针和大小
        char* data = new char[content.size() + 1];
        memcpy(data, content.c_str(), content.size() + 1);

        *ppData = data;
        *pBytes = static_cast<UINT>(content.size());

        return S_OK;
    }

    STDMETHODIMP Close(LPCVOID pData) override {
        char* data = (char*)pData;
        delete[] data;
        return S_OK;
    }
};

std::wstring AnsiToWString(const std::string& str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

std::wstring AnsiToWString(const char* str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str, -1, buffer, 512);
    return std::wstring(buffer);
}


DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
    ErrorCode(hr),
    FunctionName(functionName),
    Filename(filename),
    LineNumber(lineNumber)
{
}

std::wstring DxException::ToString()const
{
    // Get the string description of the error code.
    _com_error err(ErrorCode);
    std::wstring msg = AnsiToWString(err.ErrorMessage());

    return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}

Microsoft::WRL::ComPtr<ID3DBlob> d3dUtil::LoadBinary(const std::wstring& filename)
{
    std::ifstream fin(filename, std::ios::binary);

    fin.seekg(0, std::ios_base::end);
    std::ifstream::pos_type size = (int)fin.tellg();
    fin.seekg(0, std::ios_base::beg);

    Microsoft::WRL::ComPtr<ID3DBlob> blob;
    ThrowIfFailed(D3DCreateBlob(size, blob.GetAddressOf()));

    fin.read((char*)blob->GetBufferPointer(), size);
    fin.close();

    return blob;
}

Microsoft::WRL::ComPtr<ID3D12Resource> d3dUtil::CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
    Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    // Create the actual default buffer resource.
    ThrowIfFailed(device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

    // In order to copy CPU memory data into our default buffer, we need to create
    // an intermediate upload heap. 
    heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


    // Describe the data we want to copy into the default buffer.
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
    // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
    // the intermediate upload heap data will be copied to mBuffer.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    cmdList->ResourceBarrier(1, &barrier);
    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    cmdList->ResourceBarrier(1, &barrier);

    // Note: uploadBuffer has to be kept alive after the above function calls because
    // the command list has not been executed yet that performs the actual copy.
    // The caller can Release the uploadBuffer after it knows the copy has been executed.


    return defaultBuffer;
}

ID3DBlob* d3dUtil::CompileShader(
    const std::string& content,
    const D3D_SHADER_MACRO* defines,
    const std::string& entrypoint,
    const std::string& target,
    const std::string& sourceName)
{
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = S_OK;

    ID3DBlob* byteCode = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;
    CustomD3DInclude includer;
    hr = D3DCompile2(content.c_str(), content.size(), sourceName.c_str(), defines, &includer,
        entrypoint.c_str(), target.c_str(), compileFlags, 0, 0, nullptr, 0, &byteCode, &errors);

    if (errors != nullptr)
        OutputDebugStringA((char*)errors->GetBufferPointer());

    ThrowIfFailed(hr);

    return byteCode;
}

void d3dUtil::CompileGlslShader(
    const std::string& content,
    const std::vector<std::pair<std::string, std::string>>& shaderMacros,
    const std::string& entrypoint,
    Ogre::ShaderType shaderType,
    const std::string& sourceName,
    std::string& blob)
{
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    
    std::string hlslContent;
    translateToHlsl(sourceName, content, entrypoint, shaderMacros, shaderType, hlslContent);
    
    hlslToBin(sourceName, hlslContent, entrypoint,
        shaderMacros, nullptr, shaderType, blob, false);

}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers()
{
    // Applications usually only need a handful of samplers.  So just define them all up front
    // and keep them available as part of the root signature.  

    const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
        0, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
        1, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
        2, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
        3, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
        4, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
        0.0f,                             // mipLODBias
        8);                               // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
        5, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
        0.0f,                              // mipLODBias
        8);                                // maxAnisotropy
    const CD3DX12_STATIC_SAMPLER_DESC shadow(
        6, // shaderRegister
        D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
        0.0f,                               // mipLODBias
        16,                                 // maxAnisotropy
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);
    return {
        pointWrap, pointClamp,
        linearWrap, linearClamp,
        anisotropicWrap, anisotropicClamp, shadow };
}

/// Creates a root descriptor table parameter from the input table layout for root signature version 1_1
void d3dUtil::create_descriptor_table(
    uint32_t numDescriptors, 
    const ShaderResource* shaderResource,
    D3D12_DESCRIPTOR_RANGE1* pRange,
    D3D12_ROOT_PARAMETER1* pRootParam)
{
    pRootParam->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    uint8_t stageCount = 0;
    for (uint32_t i = 0; i < 1; ++i)
    {
        const ShaderResource* res = shaderResource;
        pRange[i].BaseShaderRegister = res->reg;
        pRange[i].RegisterSpace = res->set;
        pRange[i].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
        pRange[i].NumDescriptors = numDescriptors;
        pRange[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        pRange[i].RangeType = D3D12Mappings::getRangeType(res->type);
        stageCount |= res->used_stages;
    }
    pRootParam->ShaderVisibility = D3D12Mappings::getShaderVisibility(stageCount);
    pRootParam->DescriptorTable.NumDescriptorRanges = 1;
    pRootParam->DescriptorTable.pDescriptorRanges = pRange;
}

/// Creates a root descriptor / root constant parameter for root signature version 1_1
void d3dUtil::create_root_descriptor(
    const ShaderResource* shaderResource,
    D3D12_ROOT_PARAMETER1* pRootParam)
{
    pRootParam->ShaderVisibility = D3D12Mappings::getShaderVisibility(shaderResource->used_stages);
    pRootParam->ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    pRootParam->Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC;
    pRootParam->Descriptor.ShaderRegister = shaderResource->reg;
    pRootParam->Descriptor.RegisterSpace = shaderResource->set;
}

void d3dUtil::create_root_constant(
    const ShaderResource* shaderResource,
    D3D12_ROOT_PARAMETER1* pRootParam)
{
    pRootParam->ShaderVisibility = D3D12Mappings::getShaderVisibility(shaderResource->used_stages);
    pRootParam->ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    pRootParam->Constants.Num32BitValues = shaderResource->size;
    pRootParam->Constants.ShaderRegister = shaderResource->reg;
    pRootParam->Constants.RegisterSpace = shaderResource->set;
}



void d3dUtil::copy_descriptor_handle(
    DescriptorHeap* pSrcHeap,
    DxDescriptorID srcId,
    DescriptorHeap* pDstHeap,
    DxDescriptorID dstId
)
{
    ASSERT(pSrcHeap->mType == pDstHeap->mType);
    D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = descriptor_id_to_cpu_handle(pSrcHeap, srcId);
    D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = descriptor_id_to_cpu_handle(pDstHeap, dstId);
    pSrcHeap->pDevice->CopyDescriptorsSimple(1, dstHandle, srcHandle, pSrcHeap->mType);
}

void d3dUtil::add_descriptor_heap(
    ID3D12Device* pDevice, 
    const D3D12_DESCRIPTOR_HEAP_DESC* pDesc, 
    DescriptorHeap** ppDescHeap)
{
    uint32_t numDescriptors = pDesc->NumDescriptors;

    // Keep 32 aligned for easy remove
    numDescriptors = round_up(numDescriptors, 32);

    const size_t sizeInBytes = (numDescriptors / 32) * sizeof(uint32_t);
    DescriptorHeap* pHeap;
    uint32_t allocSize = sizeof(*pHeap) + sizeInBytes;
    pHeap = (DescriptorHeap*)malloc(allocSize);
    memset(pHeap, 0, allocSize);
    pHeap->pFlags = (uint32_t*)(pHeap + 1);
    pHeap->pDevice = pDevice;

    initMutex(&pHeap->mMutex);

    D3D12_DESCRIPTOR_HEAP_DESC desc = *pDesc;
    desc.NumDescriptors = numDescriptors;

    CHECK_HRESULT(pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap->pHeap)));

    pHeap->mStartCpuHandle = pHeap->pHeap->GetCPUDescriptorHandleForHeapStart();
    //if (desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
    {
        pHeap->mStartGpuHandle = pHeap->pHeap->GetGPUDescriptorHandleForHeapStart();
    }
    pHeap->mNumDescriptors = desc.NumDescriptors;
    pHeap->mType = desc.Type;
    pHeap->mDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(pHeap->mType);

    *ppDescHeap = pHeap;
}

