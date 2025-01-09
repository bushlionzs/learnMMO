#include "OgreHeader.h"
#include "dx12RenderSystem.h"
#include "OgreMoveObject.h"
#include "OgreMaterial.h"
#include "OgreRenderable.h"
#include "dx12Renderable.h"
#include "dx12Shader.h"
#include "dx12Texture.h"
#include "OgreTextureUnit.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreViewport.h"
#include "dx12RenderTarget.h"
#include "dx12TextureHandleManager.h"
#include "dx12ShadowMap.h"
#include "dx12RenderWindow.h"
#include "dx12Helper.h"
#include "dx12Frame.h"
#include "dx12Commands.h"
#include "memoryAllocator.h"


Dx12RenderSystem::Dx12RenderSystem(HWND wnd)
{
	mRenderSystemName = "Directx12";
}


Dx12RenderSystem::~Dx12RenderSystem()
{

}

bool Dx12RenderSystem::engineInit(bool raytracing)
{
	Dx12RenderSystemBase::engineInit();
	return true;
}

OgreTexture* Dx12RenderSystem::createTextureFromFile(const std::string& name, TextureProperty* texProperty)
{
	Dx12Texture* tex = new Dx12Texture(
		name, texProperty, mCommands, true);

	if (!tex->load(nullptr))
	{
		delete tex;
		return nullptr;
	}

	return tex;
}

Handle<HwRaytracingProgram> Dx12RenderSystem::createRaytracingProgram(
	const RaytracingShaderInfo& mShaderInfo)
{

}

