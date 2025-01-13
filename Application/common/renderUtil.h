#pragma once
#include <OgreRenderable.h>
#include <OgreCommon.h>
#include <DriverBase.h>
#include <engine_struct.h>

class GameCamera;
struct UserDefineShader;
using RenderableInitCallback = std::function< void(uint32_t frameIndex, Renderable* r)>;
using RenderableUpdateCallback = std::function< void(Renderable* r)>;
using RenderableBindCallback = std::function< void(uint32_t frameIndex, Renderable*r)>;
using RenderableDrawCallback = std::function< void(uint32_t frameIndex, Renderable* r)>;

struct UserDefineShader
{
    RenderableInitCallback initCallback;
    RenderableDrawCallback drawCallback;
    RenderableBindCallback bindCallback;
};

struct FrameResourceInfo
{
    Handle<HwDescriptorSet> zeroSet;
    Handle<HwDescriptorSet> firstSet;
    Handle<HwDescriptorSet> zeroShadowSet;
    Handle<HwBufferObject>  modelObjectHandle;
    Handle<HwBufferObject>  matObjectHandle;
    Handle<HwBufferObject>  skinObjectHandle;
    bool update;
};

void initFrameResource(uint32_t frameIndex, Renderable* r);

void updateFrameResource(uint32_t frameIndex, Renderable* r);

void updateMaterialInfo(Renderable* r, bool updateTexture);


void renderScene(
    ICamera* cam,
    SceneManager* sceneManager,
    RenderPassInfo& renderPassInfo,
    UserDefineShader* userDefineShader);

void renderScene(
    ICamera* cam,
    const std::vector<Renderable*>& renderList,
    RenderPassInfo& renderPassInfo,
    UserDefineShader* userDefineShader);

void updateFrameData(
    ICamera* camera,
    ICamera* light,
    FrameConstantBuffer& frameConstantBuffer);

struct BaseVertex
{
    Ogre::Vector3 position;
    Ogre::Vector3 normal;
};

bool createManualMesh(
    const std::string& name,
    std::vector<BaseVertex>& vertices,
    std::vector<uint16_t>& indices);