#include "OgreHeader.h"
#include "VulkanRenderSystem.h"
#include "OgreRenderable.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreMaterial.h"
#include "OgreRoot.h"
#include "VulkanRenderTarget.h"
#include "VulkanMappings.h"
#include <VulkanPipelineCache.h>
#include <VulkanHandles.h>
#include <VulkanPipelineLayoutCache.h>
#include "VulkanTexture.h"
#include "VulkanTools.h"
#include "VulkanHelper.h"
#include "VulkanWindow.h"



VulkanRenderSystem::VulkanRenderSystem(HWND wnd)
{
    bluevk::initialize();

    mRenderSystemName = "Vulkan";
    mRenderType = EngineType_Vulkan;
        
}

VulkanRenderSystem::~VulkanRenderSystem()
{

}

bool VulkanRenderSystem::engineInit(bool raytracing)
{
    VulkanRenderSystemBase::engineInit(raytracing);
    

    return true;
}














