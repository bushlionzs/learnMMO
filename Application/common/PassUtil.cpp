#include <OgreHeader.h>
#include "PassUtil.h"
#include <fg/FrameGraph.h>
#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreRoot.h>
#include <OgreRenderable.h>
#include <OgreMaterial.h>
#include <renderSystem.h>
#include <SHADER.H>

FrameGraphId<FrameGraphTexture> PassUtil::colorPass(
	FrameGraph& fg, const char* name, 
	FrameGraphTexture::Descriptor const& colorBufferDesc,
	PassConfig const& config) noexcept
{
    struct ColorPassData {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> stencil;
        FrameGraphId<FrameGraphTexture> dependency;
    };

    Blackboard& blackboard = fg.getBlackboard();

    auto& colorPass = fg.addPass<ColorPassData>(name,
        [&](FrameGraph::Builder& builder, ColorPassData& data) {

            TargetBufferFlags const clearColorFlags = TargetBufferFlags::COLOR;
            TargetBufferFlags clearDepthFlags = TargetBufferFlags::DEPTH;
            TargetBufferFlags clearStencilFlags = TargetBufferFlags::STENCIL;
            uint8_t layerCount = 0;

            data.color = blackboard.get<FrameGraphTexture>("color");
            data.depth = blackboard.get<FrameGraphTexture>("depth");

            if (!data.color) {
                data.color = builder.createTexture("Color Buffer", colorBufferDesc);

                auto& aa = fg.getDescriptor(data.color);
                int kk = 0;
            
            }

            const bool canAutoResolveDepth = false;

            if (!data.depth) {
                // clear newly allocated depth/stencil buffers, regardless of given clear flags
                clearDepthFlags = TargetBufferFlags::DEPTH;
                clearStencilFlags = TargetBufferFlags::STENCIL;
                const char* const name = "Depth/Stencil Buffer";

                TextureFormat const stencilFormat = TextureFormat::DEPTH32F_STENCIL8;

                TextureFormat const depthOnlyFormat =  TextureFormat::DEPTH32F;

                TextureFormat const format = stencilFormat;

                data.depth = builder.createTexture(name, {
                        .width = colorBufferDesc.width,
                        .height = colorBufferDesc.height,
                        .depth = colorBufferDesc.depth,
                        .samples = canAutoResolveDepth ? colorBufferDesc.samples : uint8_t(config.msaa),
                        .type = colorBufferDesc.type,
                        .format = format,
                    });
                    data.stencil = data.depth;
            }


      
            data.color = builder.read(data.color, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
            data.depth = builder.read(data.depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

            data.color = builder.write(data.color, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
            data.depth = builder.write(data.depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

            FrameGraphRenderPass::Descriptor desc = {
                    .attachments = {.color = { data.color, data.output },
                    .depth = data.depth,
                    .stencil = data.stencil },
                    .clearColor = config.clearColor,
                    .samples = config.msaa,
                    .clearFlags = clearColorFlags | clearDepthFlags | clearStencilFlags };


            builder.declareRenderPass("Color Pass Target", desc);
            blackboard["depth"] = data.depth;


        },
        [=](FrameGraphResources const& resources,
            ColorPassData const& data, DriverApi& driver) {

                auto out = resources.getRenderPassInfo();
                RenderSystem* rs = config.rs;
                RenderPassInfo passInfo;
                passInfo.renderTargetCount = 1;
                passInfo.renderTargets[0].renderTarget = config.target->getColorTarget();
                passInfo.renderTargets[0].clearColour = { 0.0, 0.0, 0.0, 1.0 };
                passInfo.depthTarget.depthStencil = config.target->getDepthTarget();
                passInfo.depthTarget.clearValue = { 1.0f, 0.0f };

                rs->beginRenderPass(passInfo);
                SceneManager* sm = config.scene;
                Ogre::Camera* cam = config.cam;
                
                static EngineRenderList engineRenerList;
                sm->getSceneRenderList(cam, engineRenerList, false);
                
                rs->endRenderPass(passInfo);
        }
    );

    // when color grading is done as a subpass, the output of the color-pass is the ldr buffer
    auto output = colorPass->color;

    blackboard["color"] = output;
    return output;
}


FrameGraphId<FrameGraphTexture> PassUtil::renderTexturePass(
    FrameGraph& fg, const char* name,
    FrameGraphTexture::Descriptor const& colorBufferDesc,
    PassConfig const& config) noexcept
{
    struct TexturePassData {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> stencil;
    };

    Blackboard& blackboard = fg.getBlackboard();

    auto& colorPass = fg.addPass<TexturePassData>(name,
        [&](FrameGraph::Builder& builder, TexturePassData& data) {

            TargetBufferFlags const clearColorFlags = TargetBufferFlags::COLOR;
            TargetBufferFlags clearDepthFlags = TargetBufferFlags::DEPTH;
            TargetBufferFlags clearStencilFlags = TargetBufferFlags::STENCIL;
            uint8_t layerCount = 0;


            if (!data.color) {
                data.color = builder.createTexture("Color Buffer", colorBufferDesc);
            }

            

            
            const bool canAutoResolveDepth = false;

            if (!data.depth) {
                // clear newly allocated depth/stencil buffers, regardless of given clear flags
                clearDepthFlags = TargetBufferFlags::DEPTH;
                clearStencilFlags = TargetBufferFlags::STENCIL;
                const char* const name = "Depth/Stencil Buffer";

                TextureFormat const format = TextureFormat::DEPTH32F_STENCIL8;

                data.depth = builder.createTexture(name, {
                        .width = colorBufferDesc.width,
                        .height = colorBufferDesc.height,
                        // If the color attachment requested MS, we assume this means the MS
                        // buffer must be kept, and for that reason we allocate the depth
                        // buffer with MS as well.
                        // On the other hand, if the color attachment was allocated without
                        // MS, no need to allocate the depth buffer with MS; Either it's not
                        // multi-sampled or it is auto-resolved.
                        // One complication above is that some backends don't support
                        // depth auto-resolve, in which case we must allocate the depth
                        // buffer with MS and manually resolve it (see "Resolved Depth Buffer"
                        // pass).
                        .depth = colorBufferDesc.depth,
                        .samples = canAutoResolveDepth ? colorBufferDesc.samples : uint8_t(config.msaa),
                        .type = colorBufferDesc.type,
                        .format = format,
                    });
                data.stencil = data.depth;
            }

            // We set a "read" constraint on these attachments here because we need to preserve them
            // when the color pass happens in several passes (e.g. with SSR)
            data.color = builder.read(data.color, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
            data.depth = builder.read(data.depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

            data.color = builder.write(data.color, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
            data.depth = builder.write(data.depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);


            

            FrameGraphRenderPass::Descriptor desc = {
                    .attachments = {.color = { data.color, data.output },
                    .depth = data.depth,
                    .stencil = data.stencil },
                    .clearColor = config.clearColor,
                    .samples = config.msaa,
                    .clearFlags = clearColorFlags | clearDepthFlags | clearStencilFlags };

           

        },
        [=](FrameGraphResources const& resources,
            TexturePassData const& data, DriverApi& driver) {
                auto out = resources.getRenderPassInfo();


                // Note: here we can't use data.color's descriptor for the viewport because
                // the actual viewport might be offset when the target is the swapchain.
                // However, the width/height should be the same.
                assert_invariant(
                    out.params.viewport.width == resources.getDescriptor(data.color).width);
                assert_invariant(
                    out.params.viewport.height == resources.getDescriptor(data.color).height);

                // TODO: this should be a parameter of FrameGraphRenderPass::Descriptor
                out.params.clearStencil = 0.0f;
                out.params.clearDepth = 1.0f;
                out.params.flags.clear = TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH_AND_STENCIL;


                SceneManager* sm = config.scene;
                Ogre::Camera* cam = config.cam;


                static EngineRenderList engineRenerList;

                sm->getSceneRenderList(cam, engineRenerList, false);
                
        }
    );

    // when color grading is done as a subpass, the output of the color-pass is the ldr buffer
    auto output = colorPass->color;
    
    return output;
}


FrameGraphId<FrameGraphTexture> PassUtil::guiPass(
    FrameGraph& fg, const char* name,
    FrameGraphTexture::Descriptor const& colorBufferDesc,
    PassConfig const& config) noexcept
{
    struct GuiPassData {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> stencil;
    };

    Blackboard& blackboard = fg.getBlackboard();

    auto& colorPass = fg.addPass<GuiPassData>(name,
        [&](FrameGraph::Builder& builder, GuiPassData& data) {
            data.color = blackboard.get<FrameGraphTexture>("color");
            data.depth = blackboard.get<FrameGraphTexture>("depth");

            // We set a "read" constraint on these attachments here because we need to preserve them
            // when the color pass happens in several passes (e.g. with SSR)
            data.color = builder.read(data.color, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
            data.depth = builder.read(data.depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

            data.color = builder.write(data.color, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
            data.depth = builder.write(data.depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);
        },
        [=](FrameGraphResources const& resources,
            GuiPassData const& data, DriverApi& driver) {

                Ogre::Camera* cam = config.cam;       
        }
    );

    // when color grading is done as a subpass, the output of the color-pass is the ldr buffer
    auto output = colorPass->color;

    blackboard["color"] = output;
    return output;
}