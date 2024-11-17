#include "OgreHeader.h"
#include "dx12RenderWindow.h"
#include "OgreStringConverter.h"
#include "dx12Helper.h"
#include "dx12RenderSystemBase.h"
#include "dx12Frame.h"
#include "OgreStringConverter.h"
#include "dx12RenderTarget.h"
#include "dx12SwapChain.h"

Dx12RenderWindow::Dx12RenderWindow():
    mSwapChain(nullptr),
    mColorTarget(nullptr),
    mDepthTarget(nullptr)
{
	
}

Dx12RenderWindow::~Dx12RenderWindow()
{

}

void Dx12RenderWindow::create(DX12SwapChain* swapChain)
{
    mSwapChain = swapChain;

    mWidth = mSwapChain->getWidth();
    mHeight = mSwapChain->getHeight();

    mColorTarget = new Dx12RenderTarget(swapChain);
    mDepthTarget = new Dx12RenderTarget(swapChain, true);
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