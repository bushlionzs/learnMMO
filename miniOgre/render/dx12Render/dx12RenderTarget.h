#pragma once
#include "dx12Common.h"
#include "OgreColourValue.h"
#include "OgreRenderTarget.h"
#include "dx12Texture.h"

class DX12SwapChain;
class DX12Commands;
class Dx12TextureHandleManager;

class Dx12RenderTarget: public Ogre::RenderTarget
{
public:
	Dx12RenderTarget(
		const std::string& name,
		DX12Commands* commands,
		TextureProperty* texProperty);
	Dx12RenderTarget(DX12SwapChain* swapChain, bool depth = false);
	~Dx12RenderTarget();


	virtual Dx12Texture* getTarget();
	

	virtual bool useMsaa() 
	{
		return false;
	}

	DX12SwapChain* getSwapChain()
	{
		return mSwapChain;
	}
private:
	Dx12Texture* mTarget = nullptr;

	DX12SwapChain* mSwapChain = nullptr;
	bool mDepth = false;
};