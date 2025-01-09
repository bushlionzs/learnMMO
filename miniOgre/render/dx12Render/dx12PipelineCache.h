#pragma once

#include "dx12Common.h"
#include <filament/DriverEnums.h>
#include <utils/bitset.h>
#include <utils/compiler.h>
#include <utils/Hash.h>
#include <tsl/robin_map.h>

class DX12PipelineCache
{
public:
    struct RasterState {
        D3D12_CULL_MODE       cullMode : 2;
        bool                frontFace : 2;
        bool              depthBiasEnable : 1;
        bool              blendEnable : 1;
        bool              depthWriteEnable : 1;
        bool              depthTestEnable : 1;
        D3D12_BLEND         srcColorBlendFactor : 5; // offset = 1 byte
        D3D12_BLEND         dstColorBlendFactor : 5;
        D3D12_BLEND         srcAlphaBlendFactor : 5;
        D3D12_BLEND         dstAlphaBlendFactor : 5;
        D3D12_DEPTH_WRITE_MASK colorWriteMask : 4;
        uint8_t               rasterizationSamples;    // offset = 4 bytes
        uint8_t               colorTargetCount;        // offset = 5 bytes
        D3D12_BLEND_OP         colorBlendOp : 4;        // offset = 6 bytes
        D3D12_BLEND_OP         alphaBlendOp : 4;
        D3D12_COMPARISON_FUNC    depthCompareOp;          // offset = 7 bytes
        float                 depthBiasConstantFactor; // offset = 8 bytes
        float                 depthBiasSlopeFactor;    // offset = 12 bytes
    };

    void bindLayout(ID3D12RootSignature* rootSignature) noexcept;

    ID3D12PipelineState* getPipeline();

    // Creates a new pipeline if necessary and binds it using vkCmdBindPipeline.
    void bindPipeline(ID3D12GraphicsCommandList* cb);
    void bindPipeline(ID3D12GraphicsCommandList* cb, ID3D12PipelineState* pipeline);

    // Each of the following methods are fast and do not make Vulkan calls.
    void bindProgram(
        const std::string* vertexShader,
        const std::string* geomtryShader,
        const std::string* fragShader) noexcept;
    void bindRasterState(const RasterState& rasterState) noexcept;
    void bindFormat(DXGI_FORMAT colorFormat[8], DXGI_FORMAT depthFormat);
    void bindPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) noexcept;

    void bindVertexArray(
        D3D12_INPUT_ELEMENT_DESC const* attribDesc,
        uint8_t attribDescCount);

    struct DX12PipelineCacheEntry {
        ID3D12PipelineState* handle;
        Timestamp lastUsed;
    };

    struct DX12PipelineKey {                                                          // size : offset
        const std::string* shaders[SHADER_MODULE_COUNT];                              //  24  : 0
        DXGI_FORMAT depthFormat;                                                     //  4   : 16
        DXGI_FORMAT colorFormat[8];                                                     //  4   : 20
        uint16_t topology;                                                        //  2   : 24
        uint16_t vertexAttributeCount;
        D3D12_INPUT_ELEMENT_DESC vertexAttributes[VERTEX_ATTRIBUTE_COUNT]; //  128 : 28
        RasterState rasterState;                                                  //  16  : 284
        uint32_t padding;                                                         //  4   : 300
        ID3D12RootSignature* rootSignature;                                                  //  8   : 304
    };

    using DX12PipelineHashFn = utils::hash::MurmurHashFn<DX12PipelineKey>;

    struct DX12PipelineEqual {
        bool operator()(const DX12PipelineKey& k1, const DX12PipelineKey& k2) const;
    };

    using PipelineMap = tsl::robin_map<DX12PipelineKey, DX12PipelineCacheEntry,
        DX12PipelineHashFn, DX12PipelineEqual>;
private:
    DX12PipelineCacheEntry* getOrCreatePipeline();
    DX12PipelineCacheEntry* createPipeline();
private:
    PipelineMap mPipelines;
    Timestamp mCurrentTime = 0;
    // Current requirements for the pipeline layout, pipeline, and descriptor sets.
    DX12PipelineKey mPipelineRequirements = {};

    // Current bindings for the pipeline and descriptor sets.
    DX12PipelineKey mBoundPipeline = {};
};