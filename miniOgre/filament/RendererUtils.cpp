/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "OgreHeader.h"
#include "RendererUtils.h"

#include "filament/FEngine.h"
#include "filament/FView.h"

#include "fg/FrameGraph.h"
#include "fg/FrameGraphId.h"
#include "fg/FrameGraphResources.h"
#include "fg/FrameGraphTexture.h"

#include <filament/Viewport.h>

#include <filament/DriverEnums.h>
#include <filament/Handle.h>
#include <filament/PixelBufferDescriptor.h>
#include <filament/FVertexBuffer.h>
#include <filament/FIndexBuffer.h>

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreCamera.h>
#include <OgreRenderable.h>
#include <OgreMaterial.h>
#include <OgreTextureManager.h>
#include <OgreTextureUnit.h>
#include <utils/BitmaskEnum.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Panic.h>

#include <algorithm>
#include <utility>

#include <stddef.h>
#include <stdint.h>

namespace filament {

using namespace backend;
using namespace math;

FrameGraphId<FrameGraphTexture> RendererUtils::colorPass(
        FrameGraph& fg, const char* name, FEngine& engine, FView const& view,
        FrameGraphTexture::Descriptor const& colorBufferDesc,
        ColorPassConfig const& config) noexcept {

    struct ColorPassData {
        FrameGraphId<FrameGraphTexture> shadows;
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> stencil;
        FrameGraphId<FrameGraphTexture> ssao;
        FrameGraphId<FrameGraphTexture> ssr;    // either screen-space reflections or refractions
        FrameGraphId<FrameGraphTexture> structure;
    };

    Blackboard& blackboard = fg.getBlackboard();

    auto& colorPass = fg.addPass<ColorPassData>(name,
            [&](FrameGraph::Builder& builder, ColorPassData& data) {

                TargetBufferFlags const clearColorFlags = config.clearFlags & TargetBufferFlags::COLOR;
                TargetBufferFlags clearDepthFlags = config.clearFlags & TargetBufferFlags::DEPTH;
                TargetBufferFlags clearStencilFlags = config.clearFlags & TargetBufferFlags::STENCIL;
                uint8_t layerCount = 0;

                data.shadows = blackboard.get<FrameGraphTexture>("shadows");
                data.ssao = blackboard.get<FrameGraphTexture>("ssao");
                data.color = blackboard.get<FrameGraphTexture>("color");
                data.depth = blackboard.get<FrameGraphTexture>("depth");


                if (!data.color) {
                    data.color = builder.createTexture("Color Buffer", colorBufferDesc);
                }

                const bool canAutoResolveDepth = engine.getDriverApi().isAutoDepthResolveSupported();

                if (!data.depth) {
                    // clear newly allocated depth/stencil buffers, regardless of given clear flags
                    clearDepthFlags = TargetBufferFlags::DEPTH;
                    clearStencilFlags = config.enabledStencilBuffer ?
                            TargetBufferFlags::STENCIL : TargetBufferFlags::NONE;
                    const char* const name = config.enabledStencilBuffer ?
                             "Depth/Stencil Buffer" : "Depth Buffer";

                    bool const isES2 =
                            engine.getDriverApi().getFeatureLevel() == FeatureLevel::FEATURE_LEVEL_0;

                    TextureFormat const stencilFormat = isES2 ?
                            TextureFormat::DEPTH24_STENCIL8 : TextureFormat::DEPTH32F_STENCIL8;

                    TextureFormat const depthOnlyFormat = isES2 ?
                            TextureFormat::DEPTH24 : TextureFormat::DEPTH32F;

                    TextureFormat const format = config.enabledStencilBuffer ?
                            stencilFormat : depthOnlyFormat;

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
                    if (config.enabledStencilBuffer) {
                        data.stencil = data.depth;
                    }
                }

                
                //data.color = builder.read(data.color, FrameGraphTexture::Usage::SUBPASS_INPUT);
                

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
                builder.declareRenderPass("Color Pass Target", desc);
                blackboard["depth"] = data.depth;

            },
            [=,  &view, &engine](FrameGraphResources const& resources,
                    ColorPassData const& data, DriverApi& driver) {
                auto out = resources.getRenderPassInfo();


                // Note: here we can't use data.color's descriptor for the viewport because
                // the actual viewport might be offset when the target is the swapchain.
                // However, the width/height should be the same.
                assert_invariant(
                        out.params.viewport.width == resources.getDescriptor(data.color).width);
                assert_invariant(
                        out.params.viewport.height == resources.getDescriptor(data.color).height);

                view.prepareViewport(static_cast<filament::Viewport&>(out.params.viewport),
                        config.logicalViewport);

                

                // TODO: this should be a parameter of FrameGraphRenderPass::Descriptor
                out.params.clearStencil = 0.0f;
                out.params.clearDepth = 1.0f;
                out.params.flags.clear = TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH_AND_STENCIL;

                FrameConstantBuffer frameBuffer;

                {
                    SceneManager* sm = Ogre::Root::getSingleton().getSceneManager(MAIN_SCENE_MANAGER);
                    Ogre::Camera* camera = sm->getCamera(MAIN_CAMERA);

                    const Ogre::Matrix4& view = camera->getViewMatrix();
                    const Ogre::Matrix4& proj = camera->getProjectMatrix();
                    const Ogre::Vector3& camepos = camera->getDerivedPosition();

                    Ogre::Matrix4 invView = view.inverse();
                    Ogre::Matrix4 viewProj = view * proj;
                    Ogre::Matrix4 invProj = proj.inverse();
                    Ogre::Matrix4 invViewProj = viewProj.inverse();

                    frameBuffer.Shadow = 0;


                    frameBuffer.View = view;
                    frameBuffer.InvView = invView;
                    frameBuffer.Proj = proj;
                    frameBuffer.InvProj = invProj;
                    frameBuffer.ViewProj = viewProj;
                    frameBuffer.InvViewProj = invViewProj;
                    //mFrameConstantBuffer.ShadowTransform = mShadowTransform;
                    frameBuffer.EyePosW = camepos;


                    auto width = 1024;
                    auto height = 768;
                    frameBuffer.RenderTargetSize = Ogre::Vector2((float)width, (float)height);
                    frameBuffer.InvRenderTargetSize = Ogre::Vector2(1.0f / width, 1.0f / height);
                    frameBuffer.NearZ = 0.1f;
                    frameBuffer.FarZ = 10000.0f;
                    frameBuffer.TotalTime += Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;
                    frameBuffer.DeltaTime = Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;

                }

                
                view.commitUniforms(driver, (const char*)& frameBuffer, sizeof(FrameConstantBuffer));

                

                SceneManager* sm = Ogre::Root::getSingleton().getSceneManager(MAIN_SCENE_MANAGER);
                Ogre::Camera* cam = sm->getCamera(MAIN_CAMERA);

                /*sm = Ogre::Root::getSingleton().getSceneManager("cegui");
                cam = sm->getCamera("cegui_camera");*/

                static EngineRenderList engineRenerList;

                sm->getSceneRenderList(cam, engineRenerList);
                for (auto* r : engineRenerList.mOpaqueList)
                {
                    auto* mat = r->getMaterial().get();
                    if (mat->getResourceState() == ResourceState::READY)
                    {
                        r->updateBufferObject(cam);
                    }
                }

                

                driver.beginRenderPass(out.target, out.params);

                
                {
                    

                    PipelineState pipeline;

                    pipeline.rasterState.culling = backend::CullingMode::NONE;
                    pipeline.rasterState.depthWrite = true;
                    pipeline.rasterState.depthFunc = backend::SamplerCompareFunc::LE;
                    pipeline.rasterState.colorWrite = true;
                    utils::JobSystem::Job* loadJob = Ogre::Root::getSingleton().getLoadJob();;
     
                    auto& driveApi = engine.getDriverApi();

                    for (auto* r : engineRenerList.mOpaqueList)
                    {
                        auto* mat = r->getMaterial().get();
                        if (mat->getResourceState() == ResourceState::READY)
                        {
                            pipeline.primitiveType = PrimitiveType::TRIANGLES;
                            pipeline.program = mat->getProgram();
                            FVertexBuffer* vb = (FVertexBuffer*)r->getVertexBuffer();
                            FIndexBuffer* ib = (FIndexBuffer*)r->getIndexBuffer();
                            VertexBufferHandle vbh = vb->getHwHandle();
                            IndexBufferHandle ibh = ib->getHwHandle();;

                            pipeline.vertexBufferInfo = vb->getVertexBufferInfoHandle();

                           
                            mat->bindSamplerGroup();
                             
                            auto rbh = r->getRenderableHandle();
                            driveApi.bindUniformBuffer(0, rbh);

                            auto mbh = mat->getMaterialBufferHandle();
                            driveApi.bindUniformBuffer(2, mbh);

                            auto sbh = r->getSkinnedHandle();
                            if (sbh)
                            {
                                driveApi.bindUniformBuffer(3, sbh);
                            }
  

                            driver.bindPipeline(pipeline);
                            driver.bindRenderPrimitive(vbh, ibh);


                            IndexDataView* idv = r->getIndexView();

                            //auto indexCount = r->getIndexBuffer()->getIndexCount();

                            driver.draw2(idv->mIndexLocation, idv->mIndexCount, 1, idv->mBaseVertexLocation);
                        }
                        else
                        {
                            mat->load(loadJob);

                            mat->updateResourceState();
                        }
                    }

                }
                driver.endRenderPass();

                // color pass is typically heavy, and we don't have much CPU work left after
                // this point, so flushing now allows us to start the GPU earlier and reduce
                // latency, without creating bubbles.
                driver.flush();
            }
    );

    // when color grading is done as a subpass, the output of the color-pass is the ldr buffer
    auto output = colorPass->color;

    blackboard["color"] = output;
    return output;
}




} // namespace filament