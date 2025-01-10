#pragma once
#include <string>
#include "dx12Common.h"

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

class d3dUtil
{
public:

    static bool IsKeyDown(int vkeyCode);

    static std::string ToString(HRESULT hr);

    static UINT CalcConstantBufferByteSize(UINT byteSize)
    {
        return (byteSize + 255) & ~255;
    }

    static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);

    static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* cmdList,
        const void* initData,
        UINT64 byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);

    static ID3DBlob* CompileShader(
        const std::string& content,
        const D3D_SHADER_MACRO* defines,
        const std::string& entrypoint,
        const std::string& target, 
        const std::string& sourceName);

    static void CompileGlslShader(
        const std::string& content,
        const std::vector<std::pair<std::string, std::string>>& shaderMacros,
        const std::string& entrypoint,
        Ogre::ShaderType shaderType,
        const std::string& sourceName,
        std::string&blob);

    static void create_descriptor_table(
        uint32_t numDescriptors,
        const ShaderResource* shaderResource,
        D3D12_DESCRIPTOR_RANGE1* pRange,
        D3D12_ROOT_PARAMETER1* pRootParam);

    static void create_root_descriptor(
        const ShaderResource* shaderResource,
        D3D12_ROOT_PARAMETER1* pRootParam);

    static void create_root_constant(
       const  ShaderResource* shaderResource,
        D3D12_ROOT_PARAMETER1* pRootParam);

    static void copy_descriptor_handle(
        DescriptorHeap* pSrcHeap,
        DxDescriptorID srcId,
        DescriptorHeap* pDstHeap,
        DxDescriptorID dstId
    );

    static inline constexpr uint32_t round_up(uint32_t value, uint32_t multiple) 
              { return ((value + multiple - 1) / multiple) * multiple; }

    static void add_descriptor_heap(
        ID3D12Device* pDevice,
        const D3D12_DESCRIPTOR_HEAP_DESC* pDesc,
        DescriptorHeap** ppDescHeap);
};
