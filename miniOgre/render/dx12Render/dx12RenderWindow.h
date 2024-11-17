#pragma once

#include "OgreRenderWindow.h"

class DX12SwapChain;
class Dx12RenderTarget;
class Dx12RenderWindow : public Ogre::RenderWindow
{
public:
	Dx12RenderWindow();
	~Dx12RenderWindow();

	virtual void create(DX12SwapChain* swapChain);
	virtual Ogre::PixelFormat getColorFormat();
	virtual Ogre::RenderTarget* getColorTarget();
	virtual Ogre::RenderTarget* getDepthTarget();
private:
	DX12SwapChain* mSwapChain;
	Dx12RenderTarget* mColorTarget;
	Dx12RenderTarget* mDepthTarget;

};