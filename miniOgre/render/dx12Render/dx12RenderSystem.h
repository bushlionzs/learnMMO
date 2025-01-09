#pragma once

#include "renderSystem.h"
#include "d3dutil.h"
#include "UploadBuffer.h"
#include "dx12Common.h"
#include "dx12RenderSystemBase.h"
class Dx12GraphicsCommandList;
class Dx12RenderTarget;
class Dx12TextureHandleManager;
class Dx12ShadowMap;
class Dx12RenderWindow;

class Dx12RenderSystem : public Dx12RenderSystemBase
{
public:
    Dx12RenderSystem(HWND wnd);
    ~Dx12RenderSystem();
    virtual bool engineInit(bool raytracing);

    virtual Ogre::OgreTexture* createTextureFromFile(const std::string& name, Ogre::TextureProperty* texProperty) override;

    virtual Handle<HwRaytracingProgram> createRaytracingProgram(
        const RaytracingShaderInfo& mShaderInfo) override;
};