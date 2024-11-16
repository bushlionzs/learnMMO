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
        // Constant buffers must be a multiple of the minimum hardware
        // allocation size (usually 256 bytes).  So round up to nearest
        // multiple of 256.  We do this by adding 255 and then masking off
        // the lower 2 bytes which store all bits < 256.
        // Example: Suppose byteSize = 300.
        // (300 + 255) & ~255
        // 555 & ~255
        // 0x022B & ~0x00ff
        // 0x022B & 0xff00
        // 0x0200
        // 512
        return (byteSize + 255) & ~255;
    }

    static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);

    static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* cmdList,
        const void* initData,
        UINT64 byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);

    static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
        const std::string& content,
        const D3D_SHADER_MACRO* defines,
        const std::string& entrypoint,
        const std::string& target, 
        const std::string& sourceName);

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
};
