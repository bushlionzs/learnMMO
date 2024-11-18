#include "OgreHeader.h"
#include "OgreRoot.h"
#include "pass.h"
#include "engine_struct.h"
#include "renderSystem.h"
#include "OgreSceneManager.h"
#include "OgreRenderable.h"
#include "OgreMaterial.h"
#include "OgreRenderTarget.h"
#include "OgreTextureManager.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"

class StandardRenderPass : public PassBase
{
public:
	StandardRenderPass(RenderPassInput& input)
	{
		mPassInput = input;
		RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
		auto& ogreConfig = ::Root::getSingleton().getEngineConfig();
		mFrameBufferObjectList.resize(ogreConfig.swapBufferCount);
		for (auto i = 0; i < ogreConfig.swapBufferCount; i++)
		{
			mFrameBufferObjectList[i] =
				rs->createBufferObject(
					BufferObjectBinding::BufferObjectBinding_Uniform, 
					RESOURCE_MEMORY_USAGE_GPU_ONLY,
					0, sizeof(mFrameConstantBuffer));
		}
		auto width = ogreConfig.width;
		auto height = ogreConfig.height;
		mFrameConstantBuffer.RenderTargetSize =
			Ogre::Vector2((float)width,
				(float)height);
		mFrameConstantBuffer.InvRenderTargetSize =
			Ogre::Vector2(1.0f / width, 1.0f / height);
		mFrameConstantBuffer.NearZ = 0.1f;
		mFrameConstantBuffer.FarZ = 10000.0f;
	}

	virtual void execute(RenderSystem* rs)
	{
		auto& ogreConfig = ::Root::getSingleton().getEngineConfig();
		auto& info = mRenderPassInfo;
		auto cam = mPassInput.cam;
		auto sceneManager = mPassInput.sceneMgr;
		info.renderTargetCount = 1;
		info.renderTargets[0].renderTarget = mPassInput.color;
		info.depthTarget.depthStencil = mPassInput.depth;
		info.renderTargets[0].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
		//info.renderTargets[0].clearColour = { 0.678431f, 0, 0, 1.000000000f };
		float depthValue = 1.0f;
		if (ogreConfig.reverseDepth)
		{
			depthValue = 0.0f;
		}
		info.depthTarget.clearValue = { depthValue, 0.0f };

		static EngineRenderList engineRenerList;
		sceneManager->getSceneRenderList(cam, engineRenerList, false);
		auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		for (auto r : engineRenerList.mOpaqueList)
		{
			Ogre::Material* mat = r->getMaterial().get();
			
			if (!mat->isLoaded())
			{
				mat->load(nullptr);
			}
			if (r->createFrameResource())
			{
				for (auto i = 0; i < ogreConfig.swapBufferCount; i++)
				{
					FrameResourceInfo* resourceInfo = r->getFrameResourceInfo(i);
					auto frameHandle = mFrameBufferObjectList[i];
					rs->updateDescriptorSetBuffer(resourceInfo->zeroSet, 1, &frameHandle, 1);
					rs->updateDescriptorSetBuffer(resourceInfo->zeroShadowSet, 1, &frameHandle, 1);
				}
			}
			
			r->updateFrameResource(frameIndex);
		}
		rs->beginRenderPass(info);
		for (auto r : engineRenerList.mOpaqueList)
		{
			Ogre::Material* mat = r->getMaterial().get();
			auto flags = mat->getMaterialFlags();
			if (flags & MATERIAL_FLAG_ALPHA_TESTED)
			{
				continue;
			}
			auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
			FrameResourceInfo* resourceInfo = r->getFrameResourceInfo(frameIndex);
			Handle<HwDescriptorSet> descriptorSet[2];
			descriptorSet[0] = resourceInfo->zeroSet;
			descriptorSet[1] = resourceInfo->firstSet;

			auto programHandle = mat->getProgram();
			auto piplineHandle = mat->getPipeline();
			rs->bindPipeline(programHandle, piplineHandle, descriptorSet, 2);


			VertexData* vertexData = r->getVertexData();
			IndexData* indexData = r->getIndexData();
			vertexData->bind(nullptr);
			indexData->bind();
			IndexDataView* view = r->getIndexView();
			rs->drawIndexed(view->mIndexCount, 1,
				view->mIndexLocation, view->mBaseVertexLocation, 0);
		}

		for (auto r : engineRenerList.mOpaqueList)
		{
			Ogre::Material* mat = r->getMaterial().get();
			auto flags = mat->getMaterialFlags();
			if (flags & MATERIAL_FLAG_ALPHA_TESTED)
			{
				auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
				FrameResourceInfo* resourceInfo = r->getFrameResourceInfo(frameIndex);
				Handle<HwDescriptorSet> descriptorSet[2];
				descriptorSet[0] = resourceInfo->zeroSet;
				descriptorSet[1] = resourceInfo->firstSet;

				auto programHandle = mat->getProgram();
				auto piplineHandle = mat->getPipeline();
				rs->bindPipeline(programHandle, piplineHandle, descriptorSet, 2);


				VertexData* vertexData = r->getVertexData();
				IndexData* indexData = r->getIndexData();
				vertexData->bind(nullptr);
				indexData->bind();
				IndexDataView* view = r->getIndexView();
				rs->drawIndexed(view->mIndexCount, 1,
					view->mIndexLocation, view->mBaseVertexLocation, 0);
			}
			
		}
		rs->endRenderPass(info);

		
	}
	virtual void update(float delta)
	{
		//updateFrameData(mPassInput.cam, nullptr);
	}
private:
	void updateFrameData(ICamera* camera, ICamera* light)
	{
		RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
		const Ogre::Matrix4& view = camera->getViewMatrix();
		const Ogre::Matrix4& proj = camera->getProjectMatrix();
		const Ogre::Vector3& camepos = camera->getDerivedPosition();
		Ogre::Matrix4 invView = view.inverse();
		Ogre::Matrix4 viewProj = proj * view;
		Ogre::Matrix4 invProj = proj.inverse();
		Ogre::Matrix4 invViewProj = viewProj.inverse();

		mFrameConstantBuffer.View = view.transpose();
		mFrameConstantBuffer.InvView = invView.transpose();
		mFrameConstantBuffer.Proj = proj.transpose();
		mFrameConstantBuffer.InvProj = invProj.transpose();
		mFrameConstantBuffer.ViewProj = viewProj.transpose();
		mFrameConstantBuffer.InvViewProj = invViewProj.transpose();

		mFrameConstantBuffer.EyePosW = camepos;

		if (light)
		{
			mFrameConstantBuffer.ShadowTransform = (light->getProjectMatrix() * light->getViewMatrix()).transpose();
			mFrameConstantBuffer.Shadow = 1;

			mFrameConstantBuffer.numDirLights = 1;
			mFrameConstantBuffer.directionLights[0].lightViewProject =
				(light->getProjectMatrix() * light->getViewMatrix()).transpose();
			mFrameConstantBuffer.directionLights[0].Direction = Ogre::Vector3(-100.0f, -100.0f, 0.0f);
		}
		else
		{
			mFrameConstantBuffer.Shadow = 0;
			mFrameConstantBuffer.directionLights[0].Direction = Ogre::Vector3(0.739942074, 0.642787576, 0.198266909);
			mFrameConstantBuffer.directionLights[0].Direction.normalise();
		}


		mFrameConstantBuffer.TotalTime += Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;
		mFrameConstantBuffer.DeltaTime = Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;

		auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();

		rs->updateBufferObject(mFrameBufferObjectList[frameIndex], 
			(const char*)&mFrameConstantBuffer, sizeof(mFrameConstantBuffer));

	}
private:
	RenderPassInput mPassInput;
	RenderPassInfo mRenderPassInfo;
	FrameConstantBuffer mFrameConstantBuffer;
	std::vector<Handle<HwBufferObject>> mFrameBufferObjectList;
};

PassBase* createStandardRenderPass(RenderPassInput& input)
{
	return new StandardRenderPass(input);
}

class UserDefineRenderPass : public PassBase
{
public:
	UserDefineRenderPass(
		RenderPassCallback renderCallback,
		UpdatePassCallback updateCallback)
	{
		mRenderCallback = renderCallback;
		mUpdateCallback = updateCallback;
	}

	void execute(RenderSystem* rs)
	{
		mRenderCallback(mRenderPassInfo);
	}

	void update(float delta)
	{
		mUpdateCallback(delta);
	}
private:
	RenderPassInfo mRenderPassInfo;
	RenderPassCallback mRenderCallback;
	UpdatePassCallback mUpdateCallback;
};
PassBase* createUserDefineRenderPass(
	RenderPassCallback renderCallback,
	UpdatePassCallback updateCallback)
{
	return new UserDefineRenderPass(renderCallback, updateCallback);
}

class ComputePass : public PassBase
{
public:
	ComputePass(ComputePassCallback userCallback,
		UpdatePassCallback updateCallback)
	{
		mCallback = userCallback;
		mUpdateCallback = updateCallback;
	}

	void execute(RenderSystem* rs)
	{
		mCallback(mComputePassInfo);
	}

	void update(float delta)
	{
		if(mUpdateCallback)
			mUpdateCallback(delta);
	}
private:
	ComputePassInfo mComputePassInfo;
	ComputePassCallback mCallback;
	UpdatePassCallback  mUpdateCallback;
};

PassBase* createComputePass(
	ComputePassCallback userCallback,
	UpdatePassCallback updateCallback)
{
	return new ComputePass(userCallback, updateCallback);
}