#pragma once
#include <functional>
#include <filament/Engine.h>
#include <fg/FrameGraphTexture.h>
#include <fg/FrameGraphId.h>
#include <fg/FrameGraph.h>
#include <renderSystem.h>
class GameCamera;
class RenderSystem;
namespace Ogre
{
    class RenderWindow;
    class SceneManager;
}
using namespace filament::backend;
using namespace filament;


class PassBase;
using SetupCallback = std::function<void(RenderSystem*, Ogre::RenderWindow*, Ogre::SceneManager*, GameCamera*)>;
using CleanupCallback = std::function<void()>;
using UpdateCallback = std::function<void(float)>;

struct AppInfo
{
    SetupCallback setup;
	FrameGraphPassCallback  fgpass;
    CleanupCallback cleanup;
    UpdateCallback update;
    bool enableRayTracing = false;
    bool useCEGUI = false;
    bool useSRGB = false;
};



