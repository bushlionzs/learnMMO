#include "OgreHeader.h"
#include "dx12RenderWindow.h"
#include "OgreStringConverter.h"
#include "dx12Helper.h"
#include "dx12RenderSystemBase.h"
#include "dx12RenderTarget.h"
#include "dx12SwapChain.h"

Dx12RenderWindow::Dx12RenderWindow(DX12SwapChain* swapChain):
    mSwapChain(swapChain),
    mColorTarget(nullptr),
    mDepthTarget(nullptr)
{
	
}

Dx12RenderWindow::~Dx12RenderWindow()
{

}

void Dx12RenderWindow::create()
{
    mWidth = mSwapChain->getWidth();
    mHeight = mSwapChain->getHeight();

    mColorTarget = new Dx12RenderTarget(mSwapChain);
    mDepthTarget = new Dx12RenderTarget(mSwapChain, true);
}

Ogre::PixelFormat Dx12RenderWindow::getColorFormat()
{
    return mColorTarget->getTarget()->getFormat();
}

Ogre::RenderTarget* Dx12RenderWindow::getColorTarget()
{
    return mColorTarget;
}

Ogre::RenderTarget* Dx12RenderWindow::getDepthTarget()
{
    return mDepthTarget;
}