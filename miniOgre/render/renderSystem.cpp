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
{
    
}

RenderSystem::~RenderSystem()
{

}

bool RenderSystem::engineInit(bool raytracing)
{
    return false;
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
    TextureProperty& texProperty)
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
    BufferDesc& desc)
{
    assert(false);
    return Handle<HwBufferObject>();
}

void RenderSystem::updateBufferObject(
    Handle<HwBufferObject> boh, 
    const char* data, 
    uint32_t size,
    uint32_t offset)
{
    assert(false);
}


Handle<HwDescriptorSet> RenderSystem::createDescriptorSet(
    Handle<HwProgram> programHandle,
    uint32_t set)
{
    assert(false);
    return Handle<HwDescriptorSet>();
}

Handle<HwDescriptorSet> RenderSystem::createDescriptorSet(
    Handle<HwComputeProgram> programHandle,
    uint32_t set)
{
    assert(false);
    return Handle<HwDescriptorSet>();
}

Handle<HwDescriptorSet> RenderSystem::createDescriptorSet(
    Handle<HwRaytracingProgram> programHandle,
    uint32_t set)
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

Handle<HwRaytracingProgram> RenderSystem::createRaytracingProgram(
    const RaytracingShaderInfo& mShaderInfo)
{
    assert(false);
    return Handle<HwRaytracingProgram>();
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



