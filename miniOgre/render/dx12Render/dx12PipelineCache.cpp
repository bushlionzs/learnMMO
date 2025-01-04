#include <OgreHeader.h>
#include "dx12PipelineCache.h"
#include "dx12Helper.h"
#include "D3D12Mappings.h"

void DX12PipelineCache::bindLayout(ID3D12RootSignature* rootSignature) noexcept
{
    mPipelineRequirements.rootSignature = rootSignature;
}

ID3D12PipelineState* DX12PipelineCache::getPipeline()
{
    DX12PipelineCacheEntry* cacheEntry = getOrCreatePipeline();
    return cacheEntry->handle;
}

// Creates a new pipeline if necessary and binds it using vkCmdBindPipeline.
void DX12PipelineCache::bindPipeline(ID3D12GraphicsCommandList* cb)
{

}

void DX12PipelineCache::bindPipeline(
    ID3D12GraphicsCommandList* cb,
    ID3D12PipelineState* pipeline)
{

}

// Each of the following methods are fast and do not make Vulkan calls.
void DX12PipelineCache::bindProgram(
    const std::string* vertexShader,
    const std::string* geomtryShader,
    const std::string* fragShader) noexcept
{
    mPipelineRequirements.shaders[0] = vertexShader;
    mPipelineRequirements.shaders[1] = geomtryShader;
    mPipelineRequirements.shaders[2] = fragShader;
}
void DX12PipelineCache::bindRasterState(const RasterState& rasterState) noexcept
{
    mPipelineRequirements.rasterState = rasterState;
}

void DX12PipelineCache::bindFormat(DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat)
{
    mPipelineRequirements.colorFormat = colorFormat;
    mPipelineRequirements.depthFormat = depthFormat;
}

void DX12PipelineCache::bindPrimitiveTopology(
    D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) noexcept
{
    mPipelineRequirements.topology = topology;
}

void DX12PipelineCache::bindVertexArray(
    D3D12_INPUT_ELEMENT_DESC const* attribDesc,
    uint8_t attribDescCount)
{
    for (auto i = 0; i < attribDescCount; i++)
    {
        mPipelineRequirements.vertexAttributes[i] = attribDesc[i];
    }

    mPipelineRequirements.vertexAttributeCount = attribDescCount;
}

DX12PipelineCache::DX12PipelineCacheEntry* DX12PipelineCache::getOrCreatePipeline()
{
    if (auto pipelineIter = mPipelines.find(mPipelineRequirements);
        pipelineIter != mPipelines.end()) {
        auto& pipeline = pipelineIter.value();
        pipeline.lastUsed = mCurrentTime;
        return &pipeline;
    }
    auto ret = createPipeline();
    ret->lastUsed = mCurrentTime;
    return ret;
}

DX12PipelineCache::DX12PipelineCacheEntry* DX12PipelineCache::createPipeline()
{
    ID3D12Device* device = DX12Helper::getSingleton().getDevice();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));


    psoDesc.InputLayout = 
    { 
        &mPipelineRequirements.vertexAttributes[0], 
        (UINT)mPipelineRequirements.vertexAttributeCount 
    };
    psoDesc.pRootSignature = mPipelineRequirements.rootSignature;

    const std::string* vsblob = mPipelineRequirements.shaders[0];
    if (vsblob)
    {
        psoDesc.VS =
        {
            reinterpret_cast<const BYTE*>(vsblob->c_str()),
            vsblob->size()
        };
    }
    
    const std::string* gsblob = mPipelineRequirements.shaders[1];
    if (gsblob)
    {
        psoDesc.GS =
        {
            reinterpret_cast<const BYTE*>(gsblob->c_str()),
            gsblob->size()
        };
    }

    const std::string* psblob = mPipelineRequirements.shaders[2];
    if (psblob)
    {
        psoDesc.PS =
        {
            reinterpret_cast<const BYTE*>(psblob->c_str()),
            psblob->size()
        };
    }
    
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

    psoDesc.RasterizerState.CullMode = mPipelineRequirements.rasterState.cullMode;
    
    psoDesc.RasterizerState.FrontCounterClockwise = true;
    psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    psoDesc.RasterizerState.MultisampleEnable = FALSE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = (D3D12_PRIMITIVE_TOPOLOGY_TYPE)mPipelineRequirements.topology;
    psoDesc.NumRenderTargets = mPipelineRequirements.rasterState.colorTargetCount;
    for(auto i = 0;  i < psoDesc.NumRenderTargets; i++)
    {
        psoDesc.RTVFormats[i] = mPipelineRequirements.colorFormat;
    }
    
    psoDesc.SampleDesc.Count = mPipelineRequirements.rasterState.rasterizationSamples; 
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.DSVFormat = mPipelineRequirements.depthFormat;

    
    D3D12_DEPTH_STENCIL_DESC depthDSS{};
    depthDSS.DepthEnable = mPipelineRequirements.rasterState.depthWriteEnable;
    depthDSS.DepthWriteMask = mPipelineRequirements.rasterState.colorWriteMask;
    depthDSS.DepthFunc = mPipelineRequirements.rasterState.depthCompareOp;
    depthDSS.StencilEnable = FALSE;
    depthDSS.StencilReadMask = 0;
    depthDSS.StencilWriteMask = 0;
    psoDesc.DepthStencilState = depthDSS;
    
   
    if (mPipelineRequirements.rasterState.blendEnable)
    {
        D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
        auto& rasterState = mPipelineRequirements.rasterState;
        transparencyBlendDesc.BlendEnable = true;
        transparencyBlendDesc.LogicOpEnable = false;
        transparencyBlendDesc.SrcBlend = rasterState.srcColorBlendFactor;
        transparencyBlendDesc.DestBlend = rasterState.dstColorBlendFactor;
        transparencyBlendDesc.BlendOp = rasterState.colorBlendOp;
        transparencyBlendDesc.SrcBlendAlpha = rasterState.srcAlphaBlendFactor;
        transparencyBlendDesc.DestBlendAlpha = rasterState.dstAlphaBlendFactor;
        transparencyBlendDesc.BlendOpAlpha = rasterState.alphaBlendOp;
        transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
        transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        psoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
    }
    ID3D12PipelineState* pso = nullptr;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));

    DX12PipelineCacheEntry cacheEntry = {};
    cacheEntry.handle = pso;
    cacheEntry.lastUsed = mCurrentTime;
    return &mPipelines.emplace(mPipelineRequirements, cacheEntry).first.value();
}

bool DX12PipelineCache::DX12PipelineEqual::operator()(const DX12PipelineKey& k1,
    const DX12PipelineKey& k2) const {
    return 0 == memcmp((const void*)&k1, (const void*)&k2, sizeof(k1));
}