#include "OgreHeader.h"
#include "dx12RenderSystemBase.h"
#define USE_PIX
#if defined(USE_PIX)
#include <winpixeventruntime/pix3.h>
#endif 
#include "OgreMoveObject.h"
#include "OgreMaterial.h"
#include "OgreRenderable.h"
#include "OgreTextureUnit.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreCamera.h"
#include "OgreViewport.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreStringConverter.h"
#include "OgreResourceManager.h"
#include "dx12Buffer.h"
#include "dx12Shader.h"
#include "dx12Texture.h"
#include "dx12Handles.h"
#include "dx12RenderTarget.h"
#include "dx12Commands.h"
#include "dx12TextureHandleManager.h"
#include "dx12ShadowMap.h"
#include "dx12RenderWindow.h"
#include "dx12Helper.h"
#include "dx12Frame.h"
#include "D3D12Mappings.h"
#include "d3dutil.h"
#include "dx12SwapChain.h"
#include "memoryAllocator.h"


DescriptorHeapProperties gCpuDescriptorHeapProperties[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {
    { 1024 * 256, D3D12_DESCRIPTOR_HEAP_FLAG_NONE }, // CBV SRV UAV
    { 2048, D3D12_DESCRIPTOR_HEAP_FLAG_NONE },       // Sampler
    { 512, D3D12_DESCRIPTOR_HEAP_FLAG_NONE },        // RTV
    { 512, D3D12_DESCRIPTOR_HEAP_FLAG_NONE },        // DSV
};
#define CALC_SUBRESOURCE_INDEX(MipSlice, ArraySlice, PlaneSlice, MipLevels, ArraySize) \
    ((MipSlice) + ((ArraySlice) * (MipLevels)) + ((PlaneSlice) * (MipLevels) * (ArraySize)))


Dx12RenderSystemBase::Dx12RenderSystemBase()
    :mResourceAllocator(83886080, false)
{
	mRenderSystemName = "Directx12";
    mRenderType = EngineType_Dx12;
}


Dx12RenderSystemBase::~Dx12RenderSystemBase()
{

}



bool Dx12RenderSystemBase::engineInit()
{
	RenderSystem::engineInit();

	auto helper = new DX12Helper(this);
	helper->createBaseInfo();
    mDevice = helper->getDevice();
    mCommands = new DX12Commands(mDevice);

    mMemoryAllocator = new DxMemoryAllocator(mDevice);


    mDescriptorHeapContext.mCPUDescriptorHeaps = (DescriptorHeap**)malloc(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES * sizeof(DescriptorHeap*));
    mDescriptorHeapContext.mCbvSrvUavHeaps = (DescriptorHeap**)malloc(1 * sizeof(DescriptorHeap*));
    mDescriptorHeapContext.pSamplerHeaps = (DescriptorHeap**)malloc(1 * sizeof(DescriptorHeap*));
    for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Flags = gCpuDescriptorHeapProperties[i].mFlags;
        desc.NodeMask = 0; 
        desc.NumDescriptors = gCpuDescriptorHeapProperties[i].mMaxDescriptors;
        desc.Type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
        d3dUtil::add_descriptor_heap(mDevice, &desc, &mDescriptorHeapContext.mCPUDescriptorHeaps[i]);
    }

    for (uint32_t i = 0; i < 1; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;

        desc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        d3dUtil::add_descriptor_heap(mDevice, &desc, &mDescriptorHeapContext.mCbvSrvUavHeaps[i]);

        // Max sampler descriptor count
        desc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        d3dUtil::add_descriptor_heap(mDevice, &desc, &mDescriptorHeapContext.pSamplerHeaps[i]);
    }


    ID3D12DescriptorHeap* heaps[] =
    {
        mDescriptorHeapContext.mCbvSrvUavHeaps[0]->pHeap,
        mDescriptorHeapContext.pSamplerHeaps[0]->pHeap
    };

    D3D12_COMMAND_SIGNATURE_DESC desc = {};
    D3D12_INDIRECT_ARGUMENT_DESC arg = {};
    arg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
    desc.NumArgumentDescs = 1;
    desc.pArgumentDescs = &arg;
    desc.ByteStride = sizeof(IndirectDrawIndexArguments);
    auto hr = mDevice->CreateCommandSignature(
        &desc, NULL, IID_PPV_ARGS(&mDrawIndexCommandSignature));
	return true;
}

void Dx12RenderSystemBase::ready()
{
}

Ogre::RenderWindow* Dx12RenderSystemBase::createRenderWindow(
    const CreateWindowDesc& desc)
{
    auto wnd = (HWND)StringConverter::parseSizeT(desc.windowHandle);
    mSwapChain = new DX12SwapChain(mCommands, wnd, desc.srgb);
    mRenderWindow = new Dx12RenderWindow(mSwapChain);
    mRenderWindow->create();
    return mRenderWindow;
}
Ogre::RenderTarget* Dx12RenderSystemBase::createRenderTarget(
    const String& name,
    TextureProperty& texProperty)
{
    D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    DxDescriptorID targetId = -1;
    if (texProperty._tex_usage & (uint32_t)Ogre::TextureUsage::DEPTH_ATTACHMENT)
    {
        texProperty._samplerParams.wrapS = filament::backend::SamplerWrapMode::CLAMP_TO_EDGE;
        texProperty._samplerParams.wrapT = filament::backend::SamplerWrapMode::CLAMP_TO_EDGE;
        texProperty._samplerParams.wrapR = filament::backend::SamplerWrapMode::CLAMP_TO_EDGE;
        type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        targetId = consume_descriptor_handles(
            mDescriptorHeapContext.mCPUDescriptorHeaps[type], 1);
    }
    else if (texProperty._tex_usage & (uint32_t)Ogre::TextureUsage::COLOR_ATTACHMENT)
    {
        type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        targetId = consume_descriptor_handles(
            mDescriptorHeapContext.mCPUDescriptorHeaps[type], 1);
    }
    else
    {
        //WRITEABLE
    }

    DxDescriptorID descriptorId = consume_descriptor_handles(
        mDescriptorHeapContext.mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1);
    
    Dx12RenderTarget* renderTarget = new Dx12RenderTarget(
        name, mCommands, &texProperty, descriptorId, targetId);
    return renderTarget;
}

void Dx12RenderSystemBase::frameStart()
{
    bool reized = false;
    mSwapChain->acquire(reized);

    auto* cl = mCommands->get();
    ID3D12DescriptorHeap* heaps[] = 
    { 
        mDescriptorHeapContext.mCbvSrvUavHeaps[0]->pHeap,
        mDescriptorHeapContext.pSamplerHeaps[0]->pHeap
    };
    cl->SetDescriptorHeaps(2, heaps);
}

void Dx12RenderSystemBase::frameEnd()
{
}

void Dx12RenderSystemBase::present()
{
    mSwapChain->present();
}

void Dx12RenderSystemBase::copyImage(
    Ogre::RenderTarget* dst,
    Ogre::RenderTarget* src,
    ImageCopyDesc& desc)
{
    Dx12Texture* dstTexture = (Dx12Texture*)dst->getTarget();
    Dx12Texture* srcTexture  = (Dx12Texture*)src->getTarget();

    copyImage(dstTexture, srcTexture, desc);
}

void Dx12RenderSystemBase::copyImage(
    Dx12Texture* dstTexture,
    Dx12Texture* srcTexture,
    ImageCopyDesc& desc)
{
    auto width = desc.extent.width;
    auto height = desc.extent.height;
    auto depth = desc.extent.depth;
    auto* cl = mCommands->get();

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = srcTexture->getResource();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = D3D12CalcSubresource(
        desc.srcSubresource.mipLevel, 0, 0, 1, 1);

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = dstTexture->getResource();
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

    // 设置SubresourceIndex为目标CubeMap的特定面
    // CubeMap面的索引顺序是：+X, -X, +Y, -Y, +Z, -Z
    uint32_t MipLevels = static_cast<UINT>(floor(log2(std::max(dstTexture->getWidth(), dstTexture->getHeight())))) + 1;
    UINT dstSubresource = D3D12CalcSubresource(
        desc.dstSubresource.mipLevel,
        desc.dstSubresource.baseArrayLayer, 0, MipLevels, 1);
    dstLocation.SubresourceIndex = dstSubresource;

    // 执行拷贝
    CD3DX12_BOX srcBox(0, 0, width, height); // 定义源纹理的矩形区域
    cl->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &srcBox);
}

void Dx12RenderSystemBase::setViewport(
    float x, float y, float width, float height, float minDepth, float maxDepth)
{
    D3D12_VIEWPORT viewport;
    
    viewport.TopLeftX = x;
    viewport.TopLeftY = y;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = minDepth;
    viewport.MaxDepth = maxDepth;

    auto* cl = mCommands->get();
    cl->RSSetViewports(1, &viewport);
}

void Dx12RenderSystemBase::setScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    D3D12_RECT scissorRect;
    scissorRect = { (LONG)x, (LONG)y, (LONG)width, (LONG)height };

    auto* cl = mCommands->get();
    cl->RSSetScissorRects(1, &scissorRect);
}
void Dx12RenderSystemBase::beginRenderPass(RenderPassInfo& renderPassInfo)
{
    auto* cl = mCommands->get();

    uint32_t width = 0;
    uint32_t height = 0;

    bool hasColor = false;
    float* ptr = (float*)&renderPassInfo.renderTargets->clearColour;
    D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle[8];
    for (auto i = 0; i < renderPassInfo.renderTargetCount; i++)
    {
        Dx12RenderTarget* colorTarget = (Dx12RenderTarget*)renderPassInfo.renderTargets[i].renderTarget;
        auto* tex = colorTarget->getTarget();
        DxDescriptorID srcid = tex->getTargetDescriptorId();
        DescriptorHeap* heap = mDescriptorHeapContext.mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
        auto cpuHandle = descriptor_id_to_cpu_handle(heap, srcid);
        renderTargetHandle[i] = cpuHandle;
        cl->ClearRenderTargetView(cpuHandle, ptr, 0, nullptr);
        hasColor = true;
    }
    bool hasDepth = false;
    D3D12_CPU_DESCRIPTOR_HANDLE depthHandle;
    if (renderPassInfo.depthTarget.depthStencil)
    {
        Dx12RenderTarget* depthTarget = (Dx12RenderTarget*)renderPassInfo.depthTarget.depthStencil;
        auto* tex = depthTarget->getTarget();
        DxDescriptorID srcid = tex->getTargetDescriptorId();
        depthHandle = descriptor_id_to_cpu_handle(mDescriptorHeapContext.mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV], srcid);
        cl->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH,
            renderPassInfo.depthTarget.clearValue.depth, renderPassInfo.depthTarget.clearValue.stencil, 0, nullptr);

        hasDepth = true;
    }

    if (hasColor)
    {
        Dx12RenderTarget* colorTarget = (Dx12RenderTarget*)renderPassInfo.renderTargets[0].renderTarget;
        width = colorTarget->getWidth();
        height = colorTarget->getHeight();
    }
    else if (hasDepth)
    {
        Dx12RenderTarget* depthTarget = (Dx12RenderTarget*)renderPassInfo.depthTarget.depthStencil;
        width = depthTarget->getWidth();
        height = depthTarget->getHeight();
    }

    cl->OMSetRenderTargets(renderPassInfo.renderTargetCount, renderTargetHandle, 
        FALSE, hasDepth?&depthHandle:NULL);

    if (renderPassInfo.viewport)
    {
        D3D12_VIEWPORT viewport;
        D3D12_RECT scissorRect;

        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        scissorRect = { 0, 0, (LONG)width, (LONG)height };
        cl->RSSetViewports(1, &viewport);
        cl->RSSetScissorRects(1, &scissorRect);
    }
    
}

void Dx12RenderSystemBase::endRenderPass(RenderPassInfo& renderPassInfo)
{
}

void Dx12RenderSystemBase::bindPipeline(
    Handle<HwProgram> programHandle,
    Handle<HwPipeline> pipelineHandle,
    const Handle<HwDescriptorSet>* descSets,
    uint32_t setCount)
{
    DX12Pipeline* dx12Pipeline = mResourceAllocator.handle_cast<DX12Pipeline*>(pipelineHandle);
    ID3D12PipelineState* pso = dx12Pipeline->getPipeline();
    DX12Program* dx12Program = mResourceAllocator.handle_cast<DX12Program*>(programHandle);
    DX12ProgramImpl* dx12ProgramImpl = dx12Program->getProgramImpl();
    ID3D12GraphicsCommandList* cl = mCommands->get();

    auto rootSignature = dx12ProgramImpl->getRootSignature();
    cl->SetGraphicsRootSignature(rootSignature);
    cl->SetPipelineState(pso);
    cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    for (uint32_t i = 0; i < setCount; i++)
    {
        if (!descSets[i])
            continue;
        DX12DescriptorSet* dset = mResourceAllocator.handle_cast<DX12DescriptorSet*>(descSets[i]);
        std::vector<const DescriptorInfo*> descriptorInfos = dset->getDescriptorInfos();
        auto cbvSrvUavHandle = dset->getCbvSrvUavHandle();
        auto samplerHandle = dset->getSamplerHandle();
        for (auto descriptorInfo : descriptorInfos)
        {
            if (descriptorInfo->mType == D3D_SIT_SAMPLER)
            {
                auto gpuHandle = descriptor_id_to_gpu_handle(
                    mDescriptorHeapContext.pSamplerHeaps[0], samplerHandle + descriptorInfo->mSetIndex);
                cl->SetGraphicsRootDescriptorTable(descriptorInfo->mRootIndex, gpuHandle);
            }
            else
            {
                auto gpuHandle = descriptor_id_to_gpu_handle(
                    mDescriptorHeapContext.mCbvSrvUavHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], cbvSrvUavHandle + descriptorInfo->mSetIndex);
                cl->SetGraphicsRootDescriptorTable(descriptorInfo->mRootIndex, gpuHandle);
                int kk = 0;
            }
            
        }
    }
}

void Dx12RenderSystemBase::drawIndexed(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    uint32_t vertexOffset,
    uint32_t firstInstance)
{
    ID3D12GraphicsCommandList* cl = mCommands->get();
    cl->DrawIndexedInstanced(
        indexCount,
        1, firstIndex, vertexOffset, firstInstance);
}

void Dx12RenderSystemBase::draw(uint32_t vertexCount, uint32_t firstVertex)
{
    ID3D12GraphicsCommandList* cl = mCommands->get();
    cl->DrawInstanced(vertexCount, 1, firstVertex, 0);
}

void Dx12RenderSystemBase::drawIndexedIndirect(
    Handle<HwBufferObject> drawBuffer,
    uint32_t offset,
    uint32_t drawCount,
    uint32_t stride
)
{
    
    ID3D12GraphicsCommandList* cl = mCommands->get();

    DX12BufferObject* bufferObject =
        mResourceAllocator.handle_cast<DX12BufferObject*>(drawBuffer);
    cl->ExecuteIndirect(mDrawIndexCommandSignature, drawCount, bufferObject->getResource(), offset, NULL, 0);
}

void Dx12RenderSystemBase::beginComputePass(
    ComputePassInfo& computePassInfo)
{
    ID3D12GraphicsCommandList* cl = mCommands->get();

    DX12ComputeProgram* program = 
        mResourceAllocator.handle_cast<DX12ComputeProgram*>(computePassInfo.programHandle);

    cl->SetPipelineState(program->getPSO());
    cl->SetComputeRootSignature(program->getProgramImpl()->getRootSignature());
    for (auto& ds : computePassInfo.descSets)
    {
        DX12DescriptorSet* dset = mResourceAllocator.handle_cast<DX12DescriptorSet*>(ds);
        std::vector<const DescriptorInfo*> descriptorInfos = dset->getDescriptorInfos();
        auto cbvSrvUavHandle = dset->getCbvSrvUavHandle();
        auto samplerHandle = dset->getSamplerHandle();
        for (auto descriptorInfo : descriptorInfos)
        {
            if (descriptorInfo->mType == D3D_SIT_SAMPLER)
            {
                auto gpuHandle = descriptor_id_to_gpu_handle(
                    mDescriptorHeapContext.pSamplerHeaps[0], samplerHandle + descriptorInfo->mSetIndex);
                cl->SetComputeRootDescriptorTable(descriptorInfo->mRootIndex, gpuHandle);
            }
            else
            {
                auto gpuHandle = descriptor_id_to_gpu_handle(
                    mDescriptorHeapContext.mCbvSrvUavHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], cbvSrvUavHandle + descriptorInfo->mSetIndex);
                cl->SetComputeRootDescriptorTable(descriptorInfo->mRootIndex, gpuHandle);
            }
        }
    }

    cl->Dispatch(computePassInfo.computeGroup.x, 
        computePassInfo.computeGroup.y, computePassInfo.computeGroup.z);
}

void Dx12RenderSystemBase::endComputePass()
{

}

void Dx12RenderSystemBase::dispatchComputeShader()
{
}

void Dx12RenderSystemBase::pushGroupMarker(const char* maker) 
{
#if defined(USE_PIX)
    ID3D12GraphicsCommandList* cl = mCommands->get();
    PIXBeginEvent(cl, PIX_COLOR((BYTE)(255), (BYTE)(255), (BYTE)0), maker);
#endif
}
void Dx12RenderSystemBase::popGroupMarker() 
{
#if defined(USE_PIX)
    ID3D12GraphicsCommandList* cl = mCommands->get();
    PIXEndEvent(cl);
#endif
}

void Dx12RenderSystemBase::bindVertexBuffer(
    Handle<HwBufferObject> bufHandle, 
    uint32_t binding,
    uint32_t vertexSize)
{
    DX12BufferObject* bo = mResourceAllocator.handle_cast<DX12BufferObject*>(bufHandle);
    auto* cl = mCommands->get();

    D3D12_VERTEX_BUFFER_VIEW vbv;
    vbv.BufferLocation = bo->getGPUVirtualAddress();
    vbv.StrideInBytes = vertexSize;
    vbv.SizeInBytes = bo->getByteCount();
    cl->IASetVertexBuffers(binding, 1, &vbv);
}

void Dx12RenderSystemBase::bindIndexBuffer(Handle<HwBufferObject> bufHandle, uint32_t indexSize) 
{
    DX12BufferObject* bo = mResourceAllocator.handle_cast<DX12BufferObject*>(bufHandle);
    auto* cl = mCommands->get();
    D3D12_INDEX_BUFFER_VIEW ibv;
    ibv.BufferLocation = bo->getGPUVirtualAddress();
    ibv.Format = indexSize==2? DXGI_FORMAT_R16_UINT: DXGI_FORMAT_R32_UINT;
    ibv.SizeInBytes = bo->getByteCount();
    cl->IASetIndexBuffer(&ibv);
}

void* Dx12RenderSystemBase::lockBuffer(
    Handle<HwBufferObject> boh, uint32_t offset, uint32_t numBytes)
{
    DX12BufferObject* bo = mResourceAllocator.handle_cast<DX12BufferObject*>(boh);
    return bo->lock(offset, numBytes);
}

void Dx12RenderSystemBase::unlockBuffer(Handle<HwBufferObject> boh)
{
    DX12BufferObject* bo = mResourceAllocator.handle_cast<DX12BufferObject*>(boh);

    auto* cmdList = mCommands->get();
    bo->unlock(cmdList);
}

Handle<HwBufferObject> Dx12RenderSystemBase::createBufferObject(
    BufferDesc& desc)
{
    Handle<HwBufferObject> boh = mResourceAllocator.allocHandle<DX12BufferObject>();
    DxDescriptorID id = consume_descriptor_handles(
        mDescriptorHeapContext.mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1);
    DX12BufferObject* bufferObject = mResourceAllocator.construct<DX12BufferObject>(
        boh, &mDescriptorHeapContext, desc, id);
    return boh;
}

void Dx12RenderSystemBase::updateBufferObject(
    Handle<HwBufferObject> boh,
    const char* data,
    uint32_t size)
{
    DX12BufferObject* bo = mResourceAllocator.handle_cast<DX12BufferObject*>(boh);
    auto* cmdList = mCommands->get();
    bo->copyData(cmdList, data, size);
}

Handle<HwDescriptorSetLayout> Dx12RenderSystemBase::getDescriptorSetLayout(
    Handle<HwProgram> programHandle, uint32_t set)
{
    return Handle<HwDescriptorSetLayout>();
}


Handle<HwPipelineLayout> Dx12RenderSystemBase::createPipelineLayout(
    std::array<Handle<HwDescriptorSetLayout>, 4>& layouts)
{
    return Handle<HwPipelineLayout>();
}

Handle<HwProgram> Dx12RenderSystemBase::createShaderProgram(
    const ShaderInfo& shaderInfo, VertexDeclaration* decl)
{
    Handle<HwProgram> programHandle = mResourceAllocator.allocHandle<DX12Program>();

    DX12Program* program = mResourceAllocator.construct<DX12Program>(programHandle,
        shaderInfo, decl);

    return programHandle;
}


Handle<HwSampler> Dx12RenderSystemBase::createTextureSampler(
    filament::backend::SamplerParams& samplerParams)
{
    Handle<HwSampler> samplerHandle = mResourceAllocator.allocHandle<HwSampler>();
    DescriptorHeap* heap;
    Dx12RenderSystemBase* rs = DX12Helper::getSingleton().getDx12RenderSystem();
    struct DescriptorHeap** heaps = rs->getCPUDescriptorHeaps();
    heap = heaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER];
    DxDescriptorID id = DX12Helper::getSingleton().getSampler(samplerParams, heap);
    DX12Sampler* sampler = mResourceAllocator.construct<DX12Sampler>(samplerHandle, id);
    return samplerHandle;
}

Handle<HwComputeProgram> Dx12RenderSystemBase::createComputeProgram(
    const ShaderInfo& shaderInfo)
{
    Handle<HwComputeProgram> program = mResourceAllocator.allocHandle<DX12ComputeProgram>();
    DX12ComputeProgram* computeProgram = mResourceAllocator.construct<DX12ComputeProgram>(program, shaderInfo);
    DX12ProgramImpl* pImpl = computeProgram->getProgramImpl();
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = pImpl->getRootSignature();
    const std::string* blob = pImpl->getComputeBlob();
    psoDesc.CS = { reinterpret_cast<const BYTE*>(blob->c_str()), blob->size()};
    ID3D12PipelineState* pipelineState;
    auto hr = mDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
    assert_invariant(hr == S_OK);
    computeProgram->updatePSO(pipelineState);
    return program;
}

Handle<HwDescriptorSet> Dx12RenderSystemBase::createDescriptorSet(
    Handle<HwComputeProgram> programHandle,
    uint32_t set)
{
    Handle<HwDescriptorSet> dsh = mResourceAllocator.allocHandle<DX12DescriptorSet>();
    DX12ComputeProgram* computeProgram = 
        mResourceAllocator.handle_cast<DX12ComputeProgram*>(programHandle);

    DX12ProgramImpl* programImpl = computeProgram->getProgramImpl();
    DX12DescriptorSet* dx12DescSet = mResourceAllocator.construct<DX12DescriptorSet>(
        dsh, programImpl, set);

    uint32_t cbvSrvUavDescCount = programImpl->getCbvSrvUavDescCount(set);

    DxDescriptorID cbvSrvUavHandle = consume_descriptor_handles(mDescriptorHeapContext.mCbvSrvUavHeaps[0], cbvSrvUavDescCount);
    dx12DescSet->updateCbvSrvUavHandle(cbvSrvUavHandle, cbvSrvUavDescCount);

    uint32_t samplerCount = programImpl->getSamplerCount(set);

    if (samplerCount > 0)
    {
        DxDescriptorID samplerHandle = consume_descriptor_handles(mDescriptorHeapContext.pSamplerHeaps[0], samplerCount);
        dx12DescSet->updateSamplerHandle(samplerHandle, samplerCount);
    }
    return dsh;
}

Handle<HwPipeline> Dx12RenderSystemBase::createPipeline(
    backend::RasterState& rasterState,
    Handle<HwProgram>& program
)
{
    Handle<HwPipeline> pipelineHandle = mResourceAllocator.allocHandle<DX12Pipeline>();

    DX12Pipeline* dx12Pipeline = mResourceAllocator.construct<DX12Pipeline>(pipelineHandle);
    DX12Program* dx12Program = mResourceAllocator.handle_cast<DX12Program*>(program);
    DX12ProgramImpl* dx12ProgramImpl = dx12Program->getProgramImpl();
    DX12PipelineCache::RasterState dx12RasterState;

    dx12RasterState.cullMode = D3D12Mappings::getCullMode(rasterState.culling);
    dx12RasterState.frontFace = FALSE;
    dx12RasterState.depthBiasEnable = FALSE;


    dx12RasterState.blendEnable = rasterState.hasBlending();

    dx12RasterState.depthWriteEnable = rasterState.depthWrite?TRUE:FALSE;
    dx12RasterState.depthTestEnable = rasterState.depthTest;
    dx12RasterState.srcColorBlendFactor = D3D12Mappings::getBlendFactor(rasterState.blendFunctionSrcRGB);
    dx12RasterState.dstColorBlendFactor = D3D12Mappings::getBlendFactor(rasterState.blendFunctionDstRGB);
    dx12RasterState.srcAlphaBlendFactor = D3D12Mappings::getBlendFactor(rasterState.blendFunctionSrcAlpha);
    dx12RasterState.dstAlphaBlendFactor = D3D12Mappings::getBlendFactor(rasterState.blendFunctionDstAlpha);
    dx12RasterState.colorBlendOp = D3D12Mappings::getBlendOp(rasterState.blendEquationRGB);
    dx12RasterState.alphaBlendOp = D3D12Mappings::getBlendOp(rasterState.blendEquationAlpha);

    dx12RasterState.colorWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    dx12RasterState.rasterizationSamples = DX12Helper::getSingleton().hasMsaa() ? 4 : 1;
    dx12RasterState.colorTargetCount = rasterState.renderTargetCount;
    dx12RasterState.depthCompareOp = D3D12Mappings::getComparisonFunc(rasterState.depthFunc);
    dx12RasterState.depthBiasConstantFactor = 0.0f;
    dx12RasterState.depthBiasSlopeFactor = 0.0f;
    PixelFormat format = (PixelFormat)rasterState.pixelFormat;
    if (format == PF_UNKNOWN)
    {
        format = mRenderWindow->getColorFormat();
    }

    DXGI_FORMAT colorFormat = D3D12Mappings::_getPF(format);
    mDX12PipelineCache.bindFormat(colorFormat, DXGI_FORMAT_D32_FLOAT);

    const std::string* vsCode = dx12ProgramImpl->getVsBlob();
    const std::string* gsCode = dx12ProgramImpl->getGsBlob();
    const std::string* psCode = dx12ProgramImpl->getPsBlob();
    mDX12PipelineCache.bindProgram(
        vsCode,
        gsCode,
        psCode);
    mDX12PipelineCache.bindRasterState(dx12RasterState);
    mDX12PipelineCache.bindPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    auto* rootSignature = dx12ProgramImpl->getRootSignature();
    mDX12PipelineCache.bindLayout(rootSignature);

    const auto& inputList = dx12ProgramImpl->getInputDesc();
    auto inputListSize = inputList.size();
    mDX12PipelineCache.bindVertexArray(inputList.data(), inputListSize);
    
    ID3D12PipelineState* pipeline = mDX12PipelineCache.getPipeline();
    dx12Pipeline->updatePipeline(pipeline);

    return pipelineHandle;
}

Handle<HwDescriptorSet> Dx12RenderSystemBase::createDescriptorSet(
    Handle<HwProgram> programHandle,
    uint32_t set)
{
    Handle<HwDescriptorSet> dsh = mResourceAllocator.allocHandle<DX12DescriptorSet>();

    DX12Program* dx12Program = mResourceAllocator.handle_cast<DX12Program*>(programHandle);
    DX12ProgramImpl* dx12ProgramImpl = dx12Program->getProgramImpl();

    DX12DescriptorSet* dx12DescSet = mResourceAllocator.construct<DX12DescriptorSet>(dsh, dx12ProgramImpl, set);

    uint32_t cbvSrvUavDescCount = dx12ProgramImpl->getCbvSrvUavDescCount(set);

    DxDescriptorID cbvSrvUavHandle = consume_descriptor_handles(mDescriptorHeapContext.mCbvSrvUavHeaps[0], cbvSrvUavDescCount);
    dx12DescSet->updateCbvSrvUavHandle(cbvSrvUavHandle, cbvSrvUavDescCount);

    uint32_t samplerCount = dx12ProgramImpl->getSamplerCount(set);

    if (samplerCount > 0)
    {
        DxDescriptorID samplerHandle = consume_descriptor_handles(mDescriptorHeapContext.pSamplerHeaps[0], samplerCount);
        dx12DescSet->updateSamplerHandle(samplerHandle, samplerCount);
    }
    return dsh;
}

void Dx12RenderSystemBase::updateDescriptorSet(
    Handle<HwDescriptorSet> dsh,
    uint32_t count,
    const DescriptorData* pParams
)
{
    DX12DescriptorSet* dx12DescSet = mResourceAllocator.handle_cast<DX12DescriptorSet*>(dsh);

    DX12ProgramImpl* dx12ProgramImpl = dx12DescSet->getProgram();

    DxDescriptorID cbvSrvUavHandle = dx12DescSet->getCbvSrvUavHandle();
    for (auto i = 0; i < count; i++)
    {
        const DescriptorData* pParam = pParams + i;
        const DescriptorInfo* descriptroInfo = dx12ProgramImpl->getDescriptor(pParam->pName);
        if (descriptroInfo == nullptr)
        {
            continue;
        }
        dx12DescSet->addDescriptroInfo(descriptroInfo);
        assert(descriptroInfo);
        const uint32_t       arrayCount = std::max(1U, pParam->mCount);

        switch (descriptroInfo->mType)
        {
        case D3D_SIT_TEXTURE:
        {
            for (uint32_t arr = 0; arr < arrayCount; ++arr)
            {
                Dx12Texture* dx12Texture = (Dx12Texture*)pParam->ppTextures[arr];
                auto srcid = dx12Texture->getDescriptorId();
                d3dUtil::copy_descriptor_handle(
                    mDescriptorHeapContext.mCPUDescriptorHeaps[0],
                    srcid,
                    mDescriptorHeapContext.mCbvSrvUavHeaps[0],
                    cbvSrvUavHandle + descriptroInfo->mSetIndex + arr
                    );
            }
        }
            
            break;
        case D3D_SIT_CBUFFER:
        case D3D_SIT_UAV_RWSTRUCTURED:
        case D3D_SIT_BYTEADDRESS:
        case D3D_SIT_STRUCTURED:
        case D3D_SIT_UAV_RWBYTEADDRESS:
        {
            for (uint32_t arr = 0; arr < arrayCount; ++arr)
            {
                auto& boh = pParam->ppBuffers[arr];
                DX12BufferObject* bo = mResourceAllocator.handle_cast<DX12BufferObject*>(boh);
                DxDescriptorID srcId = bo->getDescriptorID();
                d3dUtil::copy_descriptor_handle(
                    mDescriptorHeapContext.mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV],
                    srcId,
                    mDescriptorHeapContext.mCbvSrvUavHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV],
                    cbvSrvUavHandle + descriptroInfo->mSetIndex + arr
                );
            }
        }
            break;
        case D3D_SIT_SAMPLER:
        {
            DxDescriptorID samplerHandle = dx12DescSet->getSamplerHandle();
            for (uint32_t arr = 0; arr < arrayCount; ++arr)
            {
                if (pParam->descriptorType == DESCRIPTOR_TYPE_SAMPLER)
                {
                    DX12Sampler* sampler = mResourceAllocator.handle_cast<DX12Sampler*>(pParam->ppSamplers[arr]);
                    DxDescriptorID srcId = sampler->getDescriptorID();
                    d3dUtil::copy_descriptor_handle(
                        mDescriptorHeapContext.mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER],
                        srcId,
                        mDescriptorHeapContext.pSamplerHeaps[0],
                        samplerHandle + descriptroInfo->mSetIndex + arr
                    );
                }
                else
                {
                    Dx12Texture* dx12Texture = (Dx12Texture*)pParam->ppTextures[arr];

                    DxDescriptorID srcId = dx12Texture->getSamplerDescriptorID();

                    d3dUtil::copy_descriptor_handle(
                        mDescriptorHeapContext.mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER],
                        srcId,
                        mDescriptorHeapContext.pSamplerHeaps[0],
                        samplerHandle + descriptroInfo->mSetIndex + arr
                    );
                }
                
                
            }
        }
            break;
        default:
            assert(false);
            break;
        }
        
    }
}

void Dx12RenderSystemBase::resourceBarrier(
    uint32_t numBufferBarriers, BufferBarrier* pBufferBarriers, 
    uint32_t numTextureBarriers, TextureBarrier* pTextureBarriers, 
    uint32_t numRtBarriers, RenderTargetBarrier* pRtBarriers
)
{
    D3D12_RESOURCE_BARRIER* barriers =
        (D3D12_RESOURCE_BARRIER*)alloca((numBufferBarriers + numTextureBarriers + numRtBarriers) * sizeof(D3D12_RESOURCE_BARRIER));
    uint32_t transitionCount = 0;

    for (uint32_t i = 0; i < numBufferBarriers; ++i)
    {
        BufferBarrier* pTransBarrier = &pBufferBarriers[i];
        D3D12_RESOURCE_BARRIER* pBarrier = &barriers[transitionCount];

        DX12BufferObject* bo = mResourceAllocator.handle_cast<DX12BufferObject*>(pTransBarrier->buffer);

        auto memoryUsage = bo->getMemoryUsage();
        auto bufferObjectType = bo->getBufferObjectBinding();
        // Only transition GPU visible resources.
        // Note: General CPU_TO_GPU resources have to stay in generic read state. They are created in upload heap.
        // There is one corner case: CPU_TO_GPU resources with UAV usage can have state transition. And they are created in custom heap.
        if (memoryUsage == RESOURCE_MEMORY_USAGE_GPU_ONLY || memoryUsage == RESOURCE_MEMORY_USAGE_GPU_TO_CPU ||
            (memoryUsage == RESOURCE_MEMORY_USAGE_CPU_TO_GPU && (bufferObjectType & BufferObjectBinding_Storge)))
        {
            // if (!(pBuffer->mCurrentState & pTransBarrier->mNewState) && pBuffer->mCurrentState != pTransBarrier->mNewState)
            if (RESOURCE_STATE_UNORDERED_ACCESS == pTransBarrier->mCurrentState &&
                RESOURCE_STATE_UNORDERED_ACCESS == pTransBarrier->mNewState)
            {
                pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                pBarrier->UAV.pResource = bo->getResource();
                ++transitionCount;
            }
            else
            {
                pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                if (pTransBarrier->mBeginOnly)
                {
                    pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
                }
                else if (pTransBarrier->mEndOnly)
                {
                    pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
                }
                pBarrier->Transition.pResource = bo->getResource();
                pBarrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                pBarrier->Transition.StateBefore = D3D12Mappings::util_to_dx12_resource_state(pTransBarrier->mCurrentState);
                pBarrier->Transition.StateAfter = D3D12Mappings::util_to_dx12_resource_state(pTransBarrier->mNewState);

                ++transitionCount;
            }
        }
    }

    for (uint32_t i = 0; i < numTextureBarriers; ++i)
    {
        TextureBarrier* pTrans = &pTextureBarriers[i];
        D3D12_RESOURCE_BARRIER* pBarrier = &barriers[transitionCount];
        Dx12Texture* pTexture = (Dx12Texture*)pTrans->pTexture;

        auto texMipLevel = pTexture->getMipLevel();

        if (RESOURCE_STATE_UNORDERED_ACCESS == pTrans->mCurrentState && RESOURCE_STATE_UNORDERED_ACCESS == pTrans->mNewState)
        {
            pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            pBarrier->UAV.pResource = pTexture->getResource();
            ++transitionCount;
        }
        else
        {
            pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            if (pTrans->mBeginOnly)
            {
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
            }
            else if (pTrans->mEndOnly)
            {
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
            }
            pBarrier->Transition.pResource = pTexture->getResource();
            pBarrier->Transition.Subresource = pTrans->mSubresourceBarrier
                ? CALC_SUBRESOURCE_INDEX(pTrans->mMipLevel, pTrans->mArrayLayer, 0, texMipLevel, 1)
                : D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            if (pTrans->mAcquire)
                pBarrier->Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
            else
                pBarrier->Transition.StateBefore = D3D12Mappings::util_to_dx12_resource_state(pTrans->mCurrentState);

            if (pTrans->mRelease)
                pBarrier->Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
            else
                pBarrier->Transition.StateAfter = D3D12Mappings::util_to_dx12_resource_state(pTrans->mNewState);

            ++transitionCount;
        }
    }

    for (uint32_t i = 0; i < numRtBarriers; ++i)
    {
        RenderTargetBarrier* pTrans = &pRtBarriers[i];
        D3D12_RESOURCE_BARRIER* pBarrier = &barriers[transitionCount];
        Dx12Texture* pTexture = (Dx12Texture*)pTrans->pRenderTarget->getTarget();
        auto texMipLevel = pTexture->getMipLevel();
        if (RESOURCE_STATE_UNORDERED_ACCESS == pTrans->mCurrentState && RESOURCE_STATE_UNORDERED_ACCESS == pTrans->mNewState)
        {
            pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            pBarrier->UAV.pResource = pTexture->getResource();
            ++transitionCount;
        }
        else
        {
            pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            if (pTrans->mBeginOnly)
            {
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
            }
            else if (pTrans->mEndOnly)
            {
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
            }
            pBarrier->Transition.pResource = pTexture->getResource();
            pBarrier->Transition.Subresource = pTrans->mSubresourceBarrier
                ? CALC_SUBRESOURCE_INDEX(pTrans->mMipLevel, pTrans->mArrayLayer, 0, texMipLevel, 1)
                : D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            if (pTrans->mAcquire)
                pBarrier->Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
            else
                pBarrier->Transition.StateBefore = D3D12Mappings::util_to_dx12_resource_state(pTrans->mCurrentState);

            if (pTrans->mRelease)
                pBarrier->Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
            else
                pBarrier->Transition.StateAfter = D3D12Mappings::util_to_dx12_resource_state(pTrans->mNewState);

            ++transitionCount;
        }
    }

    if (transitionCount)
    {
        auto* cl = mCommands->get();
        cl->ResourceBarrier(transitionCount, barriers);
    }
}


void Dx12RenderSystemBase::beginCmd() 
{
    auto* cl = mCommands->get();
    ID3D12DescriptorHeap* heaps[] =
    {
        mDescriptorHeapContext.mCbvSrvUavHeaps[0]->pHeap,
        mDescriptorHeapContext.pSamplerHeaps[0]->pHeap
    };
    cl->SetDescriptorHeaps(2, heaps);

}

void Dx12RenderSystemBase::flushCmd(bool waitCmd)
{
    mCommands->flush(true);
}



