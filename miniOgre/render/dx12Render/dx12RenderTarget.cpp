#include "OgreHeader.h"
#include "dx12RenderTarget.h"
#include "dx12Texture.h"
#include "dx12SwapChain.h"

Dx12RenderTarget::Dx12RenderTarget(
	const std::string& name,
	DX12Commands* commands,
	TextureProperty* texProperty,
	Dx12TextureHandleManager* mgr)
{
	mName = name;
	mTarget = new Dx12Texture(mName, texProperty, commands, mgr);
	mTarget->load(nullptr);
	mWidth = mTarget->getWidth();
	mHeight = mTarget->getHeight();
	mSwapChain = nullptr;
	mDepth = false;
}

Dx12RenderTarget::Dx12RenderTarget(DX12SwapChain* swapChain, bool depth)
{
	mSwapChain = swapChain;
	mDepth = depth;

	mWidth = mSwapChain->getWidth();
	mHeight = mSwapChain->getHeight();
}

Dx12RenderTarget::~Dx12RenderTarget()
{

}

Dx12Texture* Dx12RenderTarget::getTarget()
{
	if (mSwapChain)
	{
		if (mDepth)
		{
			return mSwapChain->getDepthTexture();
		}
		return mSwapChain->getCurrentColor();
	}
	return mTarget;
}
