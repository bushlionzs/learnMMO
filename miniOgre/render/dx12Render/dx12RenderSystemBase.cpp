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
#include "dx12Buffer.h"
#include "dx12Shader.h"
#include "dx12Texture.h"
#include "dx12Handles.h"
#include "dx12Commands.h"
#include "dx12RenderTarget.h"
#include "dx12TextureHandleManager.h"
#include "dx12ShadowMap.h"
#include "dx12RenderWindow.h"
#include "dx12Helper.h"
#include "dx12Frame.h"
#include "D3D12Mappings.h"
#include "d3dutil.h"

Dx12RenderSystemBase::Dx12RenderSystemBase()
    :mResourceAllocator(83886080, false)
{
	mRenderSystemName = "Directx12";
}


Dx12RenderSystemBase::~Dx12RenderSystemBase()
{

}

bool Dx12RenderSystemBase::engineInit()
{
	RenderSystem::engineInit();
	new Dx12HardwareBufferManager();

	auto helper = new DX12Helper(this);
	helper->createBaseInfo();

	
	return true;
}

void Dx12RenderSystemBase::ready()
{

}

void Dx12RenderSystemBase::beginRenderPass(RenderPassInfo& renderPassInfo)
{
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

void Dx12RenderSystemBase::present()
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

    auto* cmdList = mDX12Commands->get();
    bo->unlock(cmdList);
}

Handle<HwBufferObject> Dx12RenderSystemBase::createBufferObject(
    uint32_t bindingType,
    uint32_t bufferCreationFlags,
    uint32_t byteCount,
    const char* debugName)
{
    Handle<HwBufferObject> boh = mResourceAllocator.allocHandle<DX12BufferObject>();

    DX12BufferObject* bufferObject = mResourceAllocator.construct<DX12BufferObject>(
        boh, byteCount, bufferCreationFlags);

    return boh;
}

void Dx12RenderSystemBase::updateBufferObject(
    Handle<HwBufferObject> boh,
    const char* data,
    uint32_t size)
{
    DX12BufferObject* bo = mResourceAllocator.handle_cast<DX12BufferObject*>(boh);
    auto* cmdList = mDX12Commands->get();
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
    program->load();
    program->updateInputDesc(decl);

    const std::vector <ShaderResource>& shaderResourceList = program->getShaderResourceList();
    D3D12_ROOT_PARAMETER1      rootParams[D3D12_MAX_ROOT_COST] = {};
    UINT rootParamCount = 0;

    for (auto& shaderResource : shaderResourceList)
    {
        if (shaderResource.type == D3D_SIT_SAMPLER)
        {
            continue;
        }

        if (shaderResource.type == D3D_SIT_TEXTURE)
        {
            D3D12_DESCRIPTOR_RANGE1 range{};
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            range.BaseShaderRegister = shaderResource.reg;
            range.NumDescriptors = shaderResource.size;
            d3dUtil::create_descriptor_table(shaderResource.size,
                &shaderResource, &range, &rootParams[rootParamCount]);
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
    //mDX12PipelineCache.bindLayout(pipelineLayout);

    const auto& inputList = dx12Program->getInputDesc();
    mDX12PipelineCache.bindVertexArray(inputList.data(), inputList.size());

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
    uint32_t numBufferBarriers,
    BufferBarrier* pBufferBarriers,
    uint32_t textureBarrierCount,
    TextureBarrier* pTextureBarriers,
    uint32_t numRtBarriers,
    RenderTargetBarrier* pRtBarriers
)
{
}


void Dx12RenderSystemBase::beginCmd() 
{
}

void Dx12RenderSystemBase::flushCmd(bool waitCmd)
{
}




