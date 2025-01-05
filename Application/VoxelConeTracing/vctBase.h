#pragma once 

struct VoxelizationContext
{
    Ogre::RenderTarget* albedoTarget;
    Ogre::RenderTarget* normalTarget;
    Ogre::RenderTarget* worldPosTarget;
    Ogre::RenderTarget* depthTarget;

    Ogre::RenderTarget* voxelizationTarget;
    Handle<HwSampler> voxelizationSampler;

    Ogre::RenderTarget* posxTarget;
    Ogre::RenderTarget* negxTarget;
    Ogre::RenderTarget* posyTarget;
    Ogre::RenderTarget* negyTarget;
    Ogre::RenderTarget* poszTarget;
    Ogre::RenderTarget* negzTarget;
    Handle<HwBufferObject> mipmapBlockHandle;

    Ogre::RenderTarget* posxResultTarget;
    Ogre::RenderTarget* negxResultTarget;
    Ogre::RenderTarget* posyResultTarget;
    Ogre::RenderTarget* negyResultTarget;
    Ogre::RenderTarget* poszResultTarget;
    Ogre::RenderTarget* negzResultTarget;
    
    Ogre::RenderTarget* tracingResultTarget;

    RenderPipeline* renderPipeline;
};