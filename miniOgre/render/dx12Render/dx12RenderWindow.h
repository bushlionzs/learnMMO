#pragma once

#include "OgreRenderWindow.h"

class DX12SwapChain;
class Dx12RenderTarget;
class Dx12RenderWindow : public Ogre::RenderWindow
{
public:
	Dx12RenderWindow(DX12SwapChain* swapChain);
	~Dx12RenderWindow();

	virtual void create();
	virtual Ogre::PixelFormat getColorFormat();
	virtual Ogre::RenderTarget* getColorTarget();
	virtual Ogre::RenderTarget* getDepthTarget();
private:
	DX12SwapChain* mSwapChain;
	Dx12RenderTarget* mColorTarget;
	Dx12RenderTarget* mDepthTarget;

};