#pragma once

#include <engine_struct.h>
#include "pass.h"
#include "game_camera.h"
#include "DriverBase.h"

struct ShadersConfigBlock
{
	Ogre::Matrix4 mCameraToWorld;
	Ogre::Matrix4 mWorldToCamera;
	Ogre::Matrix4 mCameraToProjection;
	Ogre::Matrix4 mWorldToProjectionPrevious;
	Ogre::Matrix4 mWorldMatrix;
	Ogre::Vector2 mRtInvSize;
	Ogre::Vector2 mZ1PlaneSize;
	float mProjNear;
	float mProjFarMinusNear;
	float mRandomSeed;
	uint32_t mFrameIndex;
	Ogre::Vector3 mLightDirection;
	uint32_t mFramesSinceCameraMove;
	Ogre::Vector2 mSubpixelJitter;
	uint32_t mWidth;
	uint32_t mHeight;
};

struct PathTracingData
{
	Ogre::Matrix4   mHistoryProjView;
	Ogre::Vector3 mHistoryLightDirection;
	uint32_t   mFrameIndex;
	uint32_t   mHaltonIndex;
	uint32_t   mLastCameraMoveFrame;
	Ogre::Matrix4   mWorldToCamera;
	Ogre::Matrix4   mProjMat;
	Ogre::Matrix4   mProjectView;
	Ogre::Matrix4   mCameraToWorld;
	float  mProjNear;
	float  mProjFarMinusNear;
	Ogre::Vector2 mZ1PlaneSize;
	float  mRandomSeed;
};

struct PropData
{
	uint32_t  mMaterialCount = 0;
	Ogre::Matrix4      mWorldMatrix;
};

class RayTracingApp
{
public:
	struct FrameInfo
	{
		Handle<HwBufferObject> genConfigBuffer;

		//for compute shader
		Handle <HwDescriptorSet> zeroDescriptorSet;
		Handle <HwDescriptorSet> firstDescriptorSet;

		//for present pass
		Handle <HwDescriptorSet> zeroDescriptorSetOfPresent;
	};
	RayTracingApp();
	~RayTracingApp();
	void setup(
		RenderPipeline* renderPipeline,
		RenderSystem* renderSystem,
		Ogre::RenderWindow* renderWindow,
		Ogre::SceneManager* sceneManager,
		GameCamera* gameCamera);
	void update(float delta);

	FrameInfo* getFrameInfo(uint32_t i)
	{
		return &mFrameInfoList[i];
	}
private:
	std::vector<FrameInfo> mFrameInfoList;

	Handle<HwBufferObject> indexOffsetsBuffer;

	PathTracingData  mPathTracingData = {};

	Ogre::Vector3 mLightDirection = Ogre::Vector3(0.2f, 1.8f, 0.1f);

	PropData sanMiguelProp;
	GameCamera* mGameCamera;

};