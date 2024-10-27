#pragma once 
#include <DriverBase.h>
#include <OgreCommon.h>
#include <engine_struct.h>
class GameCamera;
class RenderSystem;
namespace Ogre
{
	class RenderWindow;
	class SceneManager;
	class ICamera;
	class OgreTexture;
	class RenderTarget;
}

struct RenderPassInput
{
	Ogre::RenderTarget* color = nullptr;
	Ogre::RenderTarget* depth = nullptr;
	Ogre::SceneManager* sceneMgr = nullptr;
	Ogre::ICamera* cam = nullptr;
	Ogre::Light* light = nullptr;
	Ogre::RenderTarget* shadowMapTarget = nullptr;
	bool shadowPass = false;
};




using RenderPassCallback = std::function< void(RenderPassInfo& info)>;
using ComputePassCallback = std::function< void(ComputePassInfo& info)>;
using UpdatePassCallback = std::function<void(float delta)>;

class PassBase
{
public:
	virtual void execute(RenderSystem* rs) = 0;
	virtual void update(float delta) {}
};

class RenderPipeline
{
public:
	virtual void addRenderPass(PassBase* pass) = 0;
};

PassBase* createStandardRenderPass(RenderPassInput& input);

PassBase* createUserDefineRenderPass(
	RenderPassCallback renderCallback, 
	UpdatePassCallback updateCallback);

PassBase* createComputePass(
	ComputePassCallback userCallback,
	UpdatePassCallback updateCallback);
