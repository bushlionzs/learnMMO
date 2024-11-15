#include "OgreHeader.h"
#include "renderSystem.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreRenderTarget.h"
#include "OgreViewport.h"
#include <fg/FrameGraph.h>
#include <filament/DriverApiForward.h>
#include <filament/CommandStream.h>
#include <math/vec4.h>

RenderSystem::RenderSystem()
    :mPerRenderPassArena("", 1024 * 10)
{
    
}

RenderSystem::~RenderSystem()
{

}

bool RenderSystem::engineInit(bool raytracing)
{
    return false;
}

void RenderSystem::render(FrameGraphPassCallback cb)
{
    filament::ArenaScope rootArena(mPerRenderPassArena);
    auto* rootJob = mJobSystem.setRootJob(mJobSystem.createJob());
    renderJob(rootArena, cb);
    mJobSystem.runAndWait(rootJob);
}

class DummyDriver : public Driver
{
public:
    virtual void purge() noexcept
    {

    }

    virtual ShaderModel getShaderModel()const noexcept
    {
        return ShaderModel::DESKTOP;
    }

    virtual Dispatcher getDispatcher()const noexcept
    {
        static Dispatcher dispatcher;
        return dispatcher;
    }

    virtual void debugCommandBegin(CommandStream* cmds,
        bool synchronous, const char* methodName) noexcept
    {

    }

    virtual void debugCommandEnd(CommandStream* cmds,
        bool synchronous, const char* methodName) noexcept
    {

    }


};

void RenderSystem::renderJob(filament::ArenaScope& arena, FrameGraphPassCallback cb)
{
    static CircularBuffer buf(1000);
    backend::Driver* driver = (backend::Driver*)&buf;
    Engine::Config config;
    DriverApi* driverApi = nullptr;
    static ResourceAllocator* resourceAllocator = new ResourceAllocator(config, *driverApi);
    
    FrameGraph fg(*resourceAllocator);

    auto colorPassOutput = cb(fg);

    FrameGraphId<FrameGraphTexture> input = colorPassOutput;
    auto& blackboard = fg.getBlackboard();
    Handle<HwRenderTarget> viewRenderTarget;
    
    filament::Viewport vp;
    auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
    vp.width = ogreConfig.width;
    vp.height = ogreConfig.height;
    
    const filament::math::float4 clearColor = {};
    TargetBufferFlags attachmentMask = TargetBufferFlags::COLOR0;
    TargetBufferFlags keepOverrideStartFlags = TargetBufferFlags::NONE;
    TargetBufferFlags keepOverrideEndFlags = TargetBufferFlags::NONE;
    TargetBufferFlags clearFlags = TargetBufferFlags::NONE;
    FrameGraphId<FrameGraphTexture> const fgViewRenderTarget = fg.import("viewRenderTarget", {
            .attachments = attachmentMask,
            .viewport = vp,
            .clearColor = clearColor,
            .samples = 0,
            .clearFlags = clearFlags,
            .keepOverrideStart = keepOverrideStartFlags,
            .keepOverrideEnd = keepOverrideEndFlags
        }, viewRenderTarget);

    fg.forwardResource(fgViewRenderTarget, input);

    fg.present(fgViewRenderTarget);

    fg.compile();

    
    
    filament::backend::DriverApi api(*driver, buf);
    fg.execute(api);
}

OgreTexture* RenderSystem::createTextureFromFile(
    const std::string& name,
    Ogre::TextureProperty* texProperty)
{
    return nullptr;
}

Ogre::OgreTexture* RenderSystem::createTexture(Ogre::TextureProperty* texProperty)
{
    return nullptr;
}

Ogre::RenderTarget* RenderSystem::createRenderTarget(
    const String& name, 
    uint32_t width, 
    uint32_t height, 
    Ogre::PixelFormat format, 
    uint32_t textureUsage)
{
    return nullptr;
}


void RenderSystem::beginRenderPass(RenderPassInfo& renderPassInfo)
{

}

void RenderSystem::endRenderPass(RenderPassInfo& renderPassInfo)
{

}

void RenderSystem::beginComputePass(
    ComputePassInfo& computePassInfo)
{

}

void RenderSystem::endComputePass()
{

}

void RenderSystem::dispatchComputeShader()
{

}

void RenderSystem::present()
{

}

Handle<HwBufferObject> RenderSystem::createBufferObject(
    uint32_t bindingType,
    uint32_t bufferCreationFlags,
    uint32_t byteCount,
    const char* debugName)
{
    assert(false);
    return Handle<HwBufferObject>();
}

void RenderSystem::updateBufferObject(
    Handle<HwBufferObject> boh, 
    const char* data, 
    uint32_t size)
{
    assert(false);
}

Handle<HwDescriptorSetLayout> RenderSystem::createDescriptorSetLayout(DescriptorSetLayout& info)
{
    assert(false);
    return Handle<HwDescriptorSetLayout>();
}

Handle<HwDescriptorSetLayout> RenderSystem::getDescriptorSetLayout(Handle<HwProgram> programHandle, uint32_t index)
{
    assert(false);
    return Handle<HwDescriptorSetLayout>();
}

Handle<HwDescriptorSet> RenderSystem::createDescriptorSet(Handle<HwDescriptorSetLayout> dslh)
{
    assert(false);
    return Handle<HwDescriptorSet>();
}

Handle<HwPipelineLayout> RenderSystem::createPipelineLayout(std::array<Handle<HwDescriptorSetLayout>, 4>& layouts)
{
    assert(false);
    return Handle<HwPipelineLayout>();
}

Handle<HwProgram> RenderSystem::createShaderProgram(const ShaderInfo& mShaderInfo, VertexDeclaration* decl)
{
    assert(false);
    return Handle<HwProgram>();
}

Handle<HwDescriptorSetLayout> RenderSystem::getDescriptorSetLayout(
    Handle<HwComputeProgram> programHandle,
    uint32_t set)
{
    assert(false);
    return Handle<HwDescriptorSetLayout>();
}

Handle<HwRaytracingProgram> RenderSystem::createRaytracingProgram(
    const ShaderInfo& mShaderInfo)
{
    assert(false);
    return Handle<HwRaytracingProgram>();
}

Handle<HwDescriptorSetLayout> RenderSystem::getDescriptorSetLayout(
    Handle<HwRaytracingProgram> programHandle, uint32_t set)
{
    assert(false);
    return Handle<HwDescriptorSetLayout>();
}

Handle<HwSampler> RenderSystem::createTextureSampler(filament::backend::SamplerParams& samplerParams)
{
    assert(false);
    return Handle<HwSampler>();
}

Handle<HwComputeProgram> RenderSystem::createComputeProgram(const ShaderInfo& shaderInfo)
{
    assert(false);
    return Handle<HwComputeProgram>();
}

Handle<HwPipeline> RenderSystem::createPipeline(
    backend::RasterState& rasterState,
    Handle<HwProgram>& program)
{
    assert(false);
    return Handle<HwPipeline>();
}

void RenderSystem::bindDescriptorSet(
    Handle<HwDescriptorSet> dsh,
    uint8_t setIndex,
    backend::DescriptorSetOffsetArray&& offsets)
{
    assert(false);
}

void RenderSystem::updateDescriptorSetBuffer(
    Handle<HwDescriptorSet> dsh,
    backend::descriptor_binding_t binding,
    backend::BufferObjectHandle* boh,
    uint32_t handleCount)
{
    
}


