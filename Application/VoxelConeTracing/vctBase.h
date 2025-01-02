#pragma once 

struct VoxelizationContext
{
    Ogre::RenderTarget* albedoTarget;
    Ogre::RenderTarget* normalTarget;
    Ogre::RenderTarget* worldPosTarget;
    Ogre::RenderTarget* depthTarget;
    RenderPipeline* renderPipeline;
};