#pragma once

#include "OgreRenderWindow.h"
#include "VulkanRenderTarget.h"
#include "VulkanSwapChain.h"


class VulkanFrame;


class VulkanWindow : public Ogre::RenderWindow
{
public:
	VulkanWindow();
	~VulkanWindow();

	virtual void create(VulkanSwapChain* swapChain);
	virtual Ogre::PixelFormat getColorFormat();
	virtual Ogre::RenderTarget* getColorTarget();
	virtual Ogre::RenderTarget* getDepthTarget();

private:
	VulkanSwapChain* mSwapChain;

	Ogre::VulkanRenderTarget* mDepthTarget;

	Ogre::VulkanRenderTarget* mColorTarget;
};