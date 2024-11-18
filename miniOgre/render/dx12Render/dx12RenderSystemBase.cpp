#include "OgreHeader.h"
#include "dx12RenderSystemBase.h"
#include "dx12HardwareBufferManager.h"
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
#include "dx12Buffer.h"
#include "dx12Shader.h"
#include "dx12Texture.h"
#include "dx12Handles.h"
#include "dx12RenderTarget.h"
#include "dx12Commands.h"
#include "dx12RenderTarget.h"
#include "dx12TextureHandleManager.h"
#include "dx12ShadowMap.h"
#include "dx12RenderWindow.h"
#include "dx12Helper.h"
#include "dx12Frame.h"
#include "D3D12Mappings.h"
#include "d3dutil.h"
#include "dx12RenderTarget.h"
#include "dx12SwapChain.h"
#include "memoryAllocator.h"


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
    mDx12TextureHandleManager = new Dx12TextureHandleManager(mDevice);

    mMemoryAllocator = new DxMemoryAllocator(mDevice);

	
	return true;
}

void Dx12RenderSystemBase::ready()
{

}

Ogre::RenderWindow* Dx12RenderSystemBase::createRenderWindow(
    const String& name, unsigned int width, unsigned int height,
    const NameValuePairList* miscParams)
{

    auto itor = miscParams->find("externalWindowHandle");
    if (itor == miscParams->end())
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "externalWindowHandle should be provided");
    }

    auto wnd = (HWND)StringConverter::parseSizeT(itor->second);
    mSwapChain = new DX12SwapChain(mCommands, wnd);
    mRenderWindow = new Dx12RenderWindow(mSwapChain);
    mRenderWindow->create();
    return mRenderWindow;
}
Ogre::RenderTarget* Dx12RenderSystemBase::createRenderTarget(
    const String& name,
    uint32_t width,
    uint32_t height,
    Ogre::PixelFormat format,
    uint32_t usage)
{
    TextureProperty texProperty;
    texProperty._width = width;
    texProperty._height = height;
    texProperty._tex_usage = usage;
    texProperty._tex_format = format;
    texProperty._need_mipmap = false;

    if (usage & (uint32_t)Ogre::TextureUsage::DEPTH_ATTACHMENT)
    {
        texProperty._samplerParams.wrapS = filament::backend::SamplerWrapMode::CLAMP_TO_EDGE;
        texProperty._samplerParams.wrapT = filament::backend::SamplerWrapMode::CLAMP_TO_EDGE;
        texProperty._samplerParams.wrapR = filament::backend::SamplerWrapMode::CLAMP_TO_EDGE;
    }
    Dx12RenderTarget* renderTarget = new Dx12RenderTarget(
        name, mCommands, &texProperty, mDx12TextureHandleManager);
    return renderTarget;
}

void Dx12RenderSystemBase::frameStart()
{
    bool reized = false;
    mSwapChain->acquire(reized);
}

void Dx12RenderSystemBase::frameEnd()
{

}

void Dx12RenderSystemBase::present()
{
    mSwapChain->present();
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
        auto cpuHandle = tex->getCpuHandle();
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
        depthHandle = tex->getCpuHandle();
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

void Dx12RenderSystemBase::endRenderPass(RenderPassInfo& renderPassInfo)
{
}

void Dx12RenderSystemBase::bindPipeline(
    Handle<HwProgram> programHandle,
    Handle<HwPipeline> pipelineHandle,
    Handle<HwDescriptorSet>* descSets,
    uint32_t setCount)
{
}


void Dx12RenderSystemBase::copyImage(Ogre::RenderTarget* dst, Ogre::RenderTarget* src) 
{
}

void Dx12RenderSystemBase::drawIndexed(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    uint32_t vertexOffset,
    uint32_t firstInstance)
{
}

void Dx12RenderSystemBase::draw(uint32_t vertexCount, uint32_t firstVertex)
{

}

void Dx12RenderSystemBase::drawIndexedIndirect(
    Handle<HwBufferObject> drawBuffer,
    uint32_t offset,
    uint32_t drawCount,
    uint32_t stride
)
{
}

void Dx12RenderSystemBase::beginComputePass(
    ComputePassInfo& computePassInfo)
{

}

void Dx12RenderSystemBase::endComputePass()
{

}

void Dx12RenderSystemBase::dispatchComputeShader()
{
}

void Dx12RenderSystemBase::pushGroupMarker(const char* maker) 
{
}
void Dx12RenderSystemBase::popGroupMarker() 
{
}

Ogre::OgreTexture* Dx12RenderSystemBase::generateCubeMap(
    const std::string& name,
    Ogre::OgreTexture* environmentCube,
    Ogre::PixelFormat format,
    int32_t dim,
    CubeType type)
{
    return nullptr;
}
Ogre::OgreTexture* Dx12RenderSystemBase::generateBRDFLUT(const std::string& name)
{
    return nullptr;
}
void Dx12RenderSystemBase::bindVertexBuffer(Handle<HwBufferObject> bufHandle, uint32_t binding)
{
}
void Dx12RenderSystemBase::bindIndexBuffer(Handle<HwBufferObject> bufHandle, uint32_t indexSize) 
{
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
    BufferObjectBinding bindingType,
    ResourceMemoryUsage memoryUsage,
    uint32_t bufferCreationFlags,
    uint32_t byteCount,
    const char* debugName)
{
    Handle<HwBufferObject> boh = mResourceAllocator.allocHandle<DX12BufferObject>();

    DX12BufferObject* bufferObject = mResourceAllocator.construct<DX12BufferObject>(
        boh, mMemoryAllocator, bindingType, memoryUsage, bufferCreationFlags, byteCount);

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

Handle<HwDescriptorSet> Dx12RenderSystemBase::createDescriptorSet(
    Handle<HwDescriptorSetLayout> dslh)
{
    return Handle <HwDescriptorSet>();
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
        shaderInfo);

    program->updateInputDesc(decl);

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
    
    std::vector <ShaderResource> programResourceList;
   
    {
        ID3DBlob* blob = program->getVsBlob();
        if (blob)
        {
            auto resourceList = DX12Program::parseShaderResource(ShaderStageFlags::VERTEX,
                blob->GetBufferPointer(), blob->GetBufferSize());
            updateResourceList(programResourceList, resourceList, ShaderStageFlags::VERTEX);
        }
    }

    {
        ID3DBlob* blob = program->getGsBlob();
        if (blob)
        {
            auto resourceList = DX12Program::parseShaderResource(ShaderStageFlags::GEOMETRY,
                blob->GetBufferPointer(), blob->GetBufferSize());
            updateResourceList(programResourceList, resourceList, ShaderStageFlags::GEOMETRY);
        }
    }
    
    {
        ID3DBlob* blob = program->getPsBlob();
        if (blob)
        {
            auto resourceList = DX12Program::parseShaderResource(ShaderStageFlags::FRAGMENT,
                blob->GetBufferPointer(), blob->GetBufferSize());
            updateResourceList(programResourceList, resourceList, ShaderStageFlags::FRAGMENT);
        }
    }

    D3D12_ROOT_PARAMETER1      rootParams[D3D12_MAX_ROOT_COST] = {};
    UINT rootParamCount = 0;
    D3D12_DESCRIPTOR_RANGE1 range[32];
    for (auto& shaderResource : programResourceList)
    {
        if (shaderResource.type == D3D_SIT_SAMPLER)
        {
            continue;
        }

        if (shaderResource.type == D3D_SIT_TEXTURE)
        {
            d3dUtil::create_descriptor_table(shaderResource.size,
                &shaderResource, range, &rootParams[rootParamCount]);
        }
        else if (shaderResource.type == D3D_SIT_CBUFFER)
        {
            d3dUtil::create_root_descriptor(&shaderResource, &rootParams[rootParamCount]);
        }
        else
        {
            assert(false);
        }

        rootParamCount++;
    }

    auto staticSamplers = GetStaticSamplers();

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    rootSigDesc.Desc_1_1.NumParameters = rootParamCount;
    rootSigDesc.Desc_1_1.pParameters = rootParams;
    rootSigDesc.Desc_1_1.NumStaticSamplers = staticSamplers.size();
    rootSigDesc.Desc_1_1.pStaticSamplers = staticSamplers.data();
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
    ID3D12RootSignature* rootSignature;
    ThrowIfFailed(mDevice->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)));

    program->updateRootSignature(rootSignature);

    return programHandle;
}

Handle<HwDescriptorSetLayout> Dx12RenderSystemBase::getDescriptorSetLayout(
    Handle<HwComputeProgram> programHandle, uint32_t set)
{
    return Handle <HwDescriptorSetLayout>();
}

Handle<HwDescriptorSetLayout> Dx12RenderSystemBase::getDescriptorSetLayout(
    Handle<HwRaytracingProgram> programHandle, uint32_t set)
{
    return Handle <HwDescriptorSetLayout>();
}

Handle<HwSampler> Dx12RenderSystemBase::createTextureSampler(
    filament::backend::SamplerParams& samplerParams)
{
    return Handle <HwSampler>();
}

Handle<HwComputeProgram> Dx12RenderSystemBase::createComputeProgram(
    const ShaderInfo& shaderInfo)
{
    return Handle <HwComputeProgram>();
}

Handle<HwPipeline> Dx12RenderSystemBase::createPipeline(
    backend::RasterState& rasterState,
    Handle<HwProgram>& program
)
{
    Handle<HwPipeline> pipelineHandle = mResourceAllocator.allocHandle<DX12Pipeline>();

    DX12Pipeline* dx12Pipeline = mResourceAllocator.construct<DX12Pipeline>(pipelineHandle);
    DX12Program* dx12Program = mResourceAllocator.handle_cast<DX12Program*>(program);
    DX12PipelineCache::RasterState dx12RasterState;

    dx12RasterState.cullMode = D3D12Mappings::getCullMode(rasterState.culling);
    dx12RasterState.frontFace = FALSE;
    dx12RasterState.depthBiasEnable = FALSE;


    dx12RasterState.blendEnable = rasterState.hasBlending();

    dx12RasterState.depthWriteEnable = rasterState.depthWrite;
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
    mDX12PipelineCache.bindFormat(D3D12Mappings::_getPF(format), DXGI_FORMAT_D32_FLOAT);
    mDX12PipelineCache.bindProgram(
        dx12Program->getVsBlob(),
        dx12Program->getGsBlob(),
        dx12Program->getPsBlob());
    mDX12PipelineCache.bindRasterState(dx12RasterState);
    mDX12PipelineCache.bindPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    auto* rootSignature = dx12Program->getRootSignature();
    mDX12PipelineCache.bindLayout(rootSignature);

    const auto& inputList = dx12Program->getInputDesc();
    auto inputListSize = dx12Program->getInputDescSize();
    mDX12PipelineCache.bindVertexArray(inputList.data(), inputListSize);

    ID3D12PipelineState* pipeline = mDX12PipelineCache.getPipeline();
    dx12Pipeline->updatePipeline(pipeline);

    return pipelineHandle;
}

void Dx12RenderSystemBase::bindDescriptorSet(
    Handle<HwDescriptorSet> dsh,
    uint8_t setIndex,
    backend::DescriptorSetOffsetArray&& offsets)
{

}

void Dx12RenderSystemBase::updateDescriptorSetBuffer(
    Handle<HwDescriptorSet> dsh,
    backend::descriptor_binding_t binding,
    backend::BufferObjectHandle* boh,
    uint32_t handleCount)
{

}

void Dx12RenderSystemBase::updateDescriptorSetTexture(
    Handle<HwDescriptorSet> dsh,
    backend::descriptor_binding_t binding,
    OgreTexture** tex,
    uint32_t count,
    TextureBindType type)
{
}

void Dx12RenderSystemBase::updateDescriptorSetSampler(
    Handle<HwDescriptorSet> dsh,
    backend::descriptor_binding_t binding,
    Handle<HwSampler> samplerHandle) 
{
}

void Dx12RenderSystemBase::updateDescriptorSetSampler(
    Handle<HwDescriptorSet> dsh,
    backend::descriptor_binding_t binding,
    OgreTexture* tex) 
{
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
}

void Dx12RenderSystemBase::flushCmd(bool waitCmd)
{
}



