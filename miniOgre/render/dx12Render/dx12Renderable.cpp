#include "OgreHeader.h"
#include "engine_struct.h"
#include "dx12Renderable.h"
#include "shader.h"
#include "OgreMaterial.h"
#include "renderSystem.h"
#include "dx12Shader.h"
#include "dx12RenderSystem.h"
#include "dx12Texture.h"
#include "OgreCamera.h"
#include "OgreTextureUnit.h"
#include "dx12TextureHandleManager.h"
#include "OgreRenderable.h"
#include "dx12Helper.h"

void FrameRenderableData::_initialise()
{
	auto device = DX12Helper::getSingleton().getDevice();
	mMaterialCB = std::make_unique< UploadBuffer<GeneralMaterialConstantBuffer>>(device, 1, true);
	mSkinnedCB = std::make_unique< UploadBuffer<SkinnedConstantBuffer>>(device, 1, true);
}

void FrameRenderableData::updateObjectCB(ICamera* cam, ObjectConstantBuffer& cb)
{
	auto itor = mCamaraDataMap.find(cam);
	if (itor != mCamaraDataMap.end())
	{
		itor->second->CopyData(0, cb);
	}
	else
	{
		auto device = DX12Helper::getSingleton().getDevice();
		auto tmp = std::make_shared<UploadBuffer<ObjectConstantBuffer>> (device, 1, true);
		mCamaraDataMap.emplace(cam, tmp);
		tmp->CopyData(0, cb);
	}
}

void FrameRenderableData::updateMaterialCB(GeneralMaterialConstantBuffer& cb)
{
	mMaterialCB->CopyData(0, cb);
}

void FrameRenderableData::updateSkinnedCB(RawData* rd)
{
	mSkinnedCB->CopyData(rd->mData, rd->mDataSize);
}

D3D12_GPU_VIRTUAL_ADDRESS FrameRenderableData::getObjectAddress(ICamera* cam)
{
	auto itor = mCamaraDataMap.find(cam);
	return itor->second->Resource()->GetGPUVirtualAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS FrameRenderableData::getMaterialAddress()
{
	return mMaterialCB->Resource()->GetGPUVirtualAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS FrameRenderableData::getSkinnedAddress()
{
	return mSkinnedCB->Resource()->GetGPUVirtualAddress();
}

Dx12RenderableData::Dx12RenderableData(Dx12RenderSystem* engine, Ogre::Renderable* r)
	:RenderableData(r)
{
	mEngine = engine;
	ID3D12Device* device = DX12Helper::getSingleton().getDevice();
	mFrameRenderableDatas.resize(FRAME_RESOURCE_COUNT);

	for (auto& frame : mFrameRenderableDatas)
	{
		frame._initialise();
	}
}

D3D12_GPU_VIRTUAL_ADDRESS Dx12RenderableData::getObjectAddress()
{
	return mCurrentFrameData->getObjectAddress(mCurrentCamera);
}

D3D12_GPU_VIRTUAL_ADDRESS Dx12RenderableData::getMaterialAddress()
{
	return mCurrentFrameData->getMaterialAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS Dx12RenderableData::getSkinedAddress()
{
	return mCurrentFrameData->getSkinnedAddress();
}

void Dx12RenderableData::updateObjectConstantBuffer(Ogre::ICamera* cam)
{
	mCurrentFrameData->updateObjectCB(cam, mObjectConstantBuffer);
}

void Dx12RenderableData::updateMaterialConstantBuffer(Ogre::ICamera* cam)
{
	mCurrentFrameData->updateMaterialCB(mMaterialConstantBuffer);
}

void Dx12RenderableData::updatePbrMaterialConstantBuffer(Ogre::ICamera* cam)
{
	
}

void Dx12RenderableData::updateSkinedConstanctBuffer(RawData* rd)
{
	mCurrentFrameData->updateSkinnedCB(rd);
}

void Dx12RenderableData::buildMaterial(Ogre::Material* mat)
{
	if (mat->isPbr())
	{
		buildPbrMaterial(mat);
	}
	else
	{
		buildCommonMaterial(mat);
	}
}

void Dx12RenderableData::updateCurrentFrame(int32_t index)
{
	mCurrentFrameData = &mFrameRenderableDatas[index];
}

void Dx12RenderableData::updateData(Dx12Pass* pass, ICamera* cam)
{
	mCurrentCamera = cam;
	const Ogre::Matrix4& model = pass->mRenderable->getModelMatrix();

	mObjectConstantBuffer.world = model.transpose();

	
	{
		const Ogre::Matrix4& view = cam->getViewMatrix();
		const Ogre::Matrix4& proj = cam->getProjectMatrix();

		mObjectConstantBuffer.projector = pass->mRenderable->getProjectorMatrix();
	}

	const PbrMaterialConstanceBuffer& mcb = pass->mMaterial->getPbrMatInfo();

	if (pass->mMaterial->isPbr())
	{
		
	}
	else
	{
		if (pass->mMaterial->getTextureUnitCount())
		{
			auto unit = pass->mMaterial->getTextureUnit(0);
			mMaterialConstantBuffer.TexTransform =
				unit->getTextureTransform().transpose();
		}
		
	}
	

	updateObjectConstantBuffer(cam);
	updateMaterialConstantBuffer(cam);

}

void Dx12RenderableData::buildPbrMaterial(Ogre::Material* mat)
{
	if (mTexStartIndex >= 0 || mCubeTexStartIndex >= 0)
	{
		return;
	}
	auto texs = mat->getAllTexureUnit();

	int32_t texCount = 0;
	for (auto& tex : texs)
	{
		Dx12Texture* current = (Dx12Texture*)tex->getRaw();

		if (current->isCubeTexture())
		{
			mCubeTexCount++;
		}
		else
		{
			mTexCount++;
		}
	}

	if (mCubeTexCount > 0)
	{
		Dx12TextureHandleManager* mgr;
		mCubeTexStartIndex = mgr->allocStartIndex(mCubeTexCount);
		if (mCubeTexStartIndex < 0)
		{
			assert(false);
		}
		int32_t offset = 0;
		for (auto& tex : texs)
		{
			Dx12Texture* current = (Dx12Texture*)tex->getRaw();

			if (current->isCubeTexture())
			{
				TextureProperty* texProperty = current->getTextureProperty();
				updatePbrTextureIndex(texProperty, offset);
				offset++;
			}
		}
	}
	if (mTexCount > 0)
	{
		Dx12TextureHandleManager* mgr;
		mTexStartIndex = mgr->allocStartIndex(mTexCount);
		if (mTexStartIndex < 0)
		{
			assert(false);
		}
		int32_t offset = 0;
		for (auto& tex : texs)
		{
			Dx12Texture* current = (Dx12Texture*)tex->getRaw();
			TextureProperty* texProperty = current->getTextureProperty();
			if (current->isCubeTexture())
			{
				updatePbrTextureIndex(texProperty, offset);
				offset++;
			}
		}
	}
}

void Dx12RenderableData::buildCommonMaterial(Ogre::Material* mat)
{
	auto texs = mat->getAllTexureUnit();

	int32_t texCount = 0;
	for (auto& tex : texs)
	{
		Dx12Texture* current = (Dx12Texture*)tex->getRaw();
		if (current->isCubeTexture())
		{
			mCubeTexCount++;
		}
		else
		{
			mTexCount++;
		}
	}

	if (mCubeTexCount > 0)
	{
		Dx12TextureHandleManager* mgr;

		if (mCubeTexStartIndex < 0)
		{
			mCubeTexStartIndex = mgr->allocStartIndex(mCubeTexCount);
		}
		
		int32_t offset = 0;
		for (auto& tex : texs)
		{
			Dx12Texture* current = (Dx12Texture*)tex->getRaw();

			if (current->isCubeTexture())
			{
				offset++;
			}
		}
	}
	if (mTexCount > 0)
	{
		Dx12TextureHandleManager* mgr;
		if (mTexStartIndex < 0)
		{
			mTexStartIndex = mgr->allocStartIndex(mTexCount);
		}
		
		int32_t offset = 0;
		for (auto& tex : texs)
		{
			Dx12Texture* current = (Dx12Texture*)tex->getRaw();
			bool has = false;
			if (!current->isCubeTexture())
			{
				offset++;
				has = true;
			}
			if (!has)
			{
				int kk = 0;
			}
		}
	}
	else
	{
		int kk = 0;
	}
}

//TextureType_AmbientOcclusion = 2,
//TextureType_NormalMap = 3,
//TextureType_Emissive = 4,
//TextureType_MetalRoughness = 5,
//TextureType_IBL_Diffuse = 6,
//TextureType_IBL_Specular = 7
void Dx12RenderableData::updatePbrTextureIndex(Ogre::TextureProperty* texProperty, uint32_t index)
{
}

int32_t Dx12RenderableData::getTexStartIndex()
{
	return mTexStartIndex;
}

int32_t Dx12RenderableData::getCubeTexStartIndex()
{
	return mCubeTexStartIndex;
}
