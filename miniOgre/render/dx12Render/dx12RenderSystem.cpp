#include "OgreHeader.h"
#include "dx12RenderSystem.h"
#include "OgreMoveObject.h"
#include "OgreMaterial.h"
#include "OgreRenderable.h"
#include "dx12Renderable.h"
#include "dx12Shader.h"
#include "dx12Texture.h"
#include "OgreTextureUnit.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreViewport.h"
#include "dx12RenderTarget.h"
#include "dx12TextureHandleManager.h"
#include "dx12ShadowMap.h"
#include "dx12RenderWindow.h"
#include "dx12Helper.h"
#include "dx12Frame.h"
#include "dx12Commands.h"
#include "memoryAllocator.h"


Dx12RenderSystem::Dx12RenderSystem(HWND wnd)
{
	mRenderSystemName = "Directx12";
}


Dx12RenderSystem::~Dx12RenderSystem()
{

}

bool Dx12RenderSystem::engineInit(bool raytracing)
{
	Dx12RenderSystemBase::engineInit();
	return true;
}


void Dx12RenderSystem::_resourceLoaded()
{

}

//void Dx12RenderSystem::frameStart()
//{
//	DX12Helper::getSingleton()._submitCommandList(true);
//	mTriangleCount = 0;
//	mBatchCount = 0;
//	mLoadResCount = 0;
//	mCurrentFrame = mFrameList[mCurrFrameResourceIndex++];
//
//	mCurrFrameResourceIndex = mCurrFrameResourceIndex % FRAME_RESOURCE_COUNT;
//
//	if (mCurrentFrame->getFence() != 0 &&
//		DX12Helper::getSingleton().getFenceCompletedValue() < mCurrentFrame->getFence())
//	{
//		DX12Helper::getSingleton().waitFence(mCurrentFrame->getFence());
//	}
//
//	mCurrentFrame->resetCommandList();
//	auto commandlist = mCurrentFrame->getCommandList();
//
//	mFrameConstantBuffer.Shadow = 0;
//
//	const std::vector<ID3D12DescriptorHeap*> dhs = mDx12TextureHandleManager->getHeaps();
//	commandlist->SetDescriptorHeaps(dhs.size(), dhs.data());
//	commandlist->SetGraphicsRootSignature(mRootSignature.Get());
//
//	mCurrentPass.mRootSignature = mRootSignature.Get();
//	mCurrentPass.mPassState = PassState_Normal;
//}
//
//void Dx12RenderSystem::frameEnd()
//{
//	auto commandList = DX12Helper::getSingleton().getCurrentCommandList();
//
//	// Done recording commands.
//	ThrowIfFailed(commandList->Close());
//
//	// Add the command list to the queue for execution.
//
//	DX12Helper::getSingleton().executeCommand(commandList);
//
//	mRenderWindow->present();
//	mCurrentFrame->setFence(DX12Helper::getSingleton().incrFence());
//	DX12Helper::getSingleton().signalFence(mCurrentFrame->getFence());
//}

void Dx12RenderSystem::render(Renderable* r, RenderListType t)
{
	const std::shared_ptr<Ogre::Material>& mat = r->getMaterial();
	if (!mat)
	{
		return;
	}

	mat->load(nullptr);

	

	mCurrentPass.mRenderListType = t;
	mCurrentPass.mMaterial = mat.get();
	mCurrentPass.mShader = nullptr;
	mCurrentPass.mRenderable = r;
	mCurrentPass.mDx12RenderableData->updateCurrentFrame(mCurrentFrame->getFrameIndex());
	VertexData* vertexData = r->getVertexData();
	auto vd = vertexData->getVertexDeclaration();
	

	/*if (mCamera->getCameraType() == CameraType_Light)
	{
		mShadowShader->updateInputDesc(vd);
		ID3D12PipelineState* pso = mShadowShader->BuildPSO(&mCurrentPass);
		mCurrentFrame->getCommandList()->SetPipelineState(pso);
	}
	else*/
	{

		ID3D12PipelineState* pso = nullptr;
		mCurrentFrame->getCommandList()->SetPipelineState(pso);
	}
	

	RawData* rd = r->getSkinnedData(2);
	if (rd)
	{
		mCurrentPass.mDx12RenderableData->updateSkinedConstanctBuffer(rd);

		D3D12_GPU_VIRTUAL_ADDRESS frameAddress = 
			mCurrentPass.mDx12RenderableData->getSkinedAddress();
		mCurrentFrame->getCommandList()->SetGraphicsRootConstantBufferView(
			mSkinedConstantIndex, frameAddress);
	}

	mCurrentPass.mDx12RenderableData->buildMaterial(mCurrentPass.mMaterial);
	renderImpl(&mCurrentPass);
}

void Dx12RenderSystem::multiRender(std::vector<Ogre::Renderable*>& objs, bool multithread)
{
	for (auto r : objs)
	{
		render(r, RenderListType_Opaque);
	}
}

OgreTexture* Dx12RenderSystem::createTextureFromFile(const std::string& name, TextureProperty* texProperty)
{
	DxDescriptorID descriptorId = consume_descriptor_handles(
		mDescriptorHeapContext.mCPUDescriptorHeaps[0], 1);
	Dx12Texture* tex = new Dx12Texture(
		name, texProperty, mCommands, descriptorId, -1);

	if (!tex->load(nullptr))
	{
		delete tex;
		return nullptr;
	}

	return tex;
}

void Dx12RenderSystem::createFrameResource()
{
	auto device = DX12Helper::getSingleton().getDevice();

	mFrameList.resize(FRAME_RESOURCE_COUNT);

	for (int32_t i = 0; i < FRAME_RESOURCE_COUNT; i++)
	{
		mFrameList[i] = new Dx12Frame();
		mFrameList[i]->_initialise(i);
	}

	mCurrentFrame = mFrameList.front();
}



RenderableData* Dx12RenderSystem::createRenderableData(Ogre::Renderable* r)
{
	return new Dx12RenderableData(this, r);
}

ID3D12GraphicsCommandList* Dx12RenderSystem::getCommandList()
{
	return mCurrentFrame->getCommandList();
}

void Dx12RenderSystem::_setViewport(ICamera* cam, Ogre::Viewport* vp)
{
	mCamera = cam;
	updateMainPassCB(cam);
	
	
	Ogre::RenderTarget* target;
	target = vp->getTarget();

	mActiveDx12RenderTarget = dynamic_cast<Dx12RenderTarget*>(target);

}

EngineType Dx12RenderSystem::getRenderType()
{
	return EngineType_Dx12;
}

void Dx12RenderSystem::clearFrameBuffer(uint32 buffers,
	const ColourValue& colour,
	float depth, uint16 stencil)
{
	
}

void Dx12RenderSystem::buildRootSignature()
{
	ID3D12Device* device = DX12Helper::getSingleton().getDevice();

	CD3DX12_DESCRIPTOR_RANGE texTable0;
	texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE texTable1;
	texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);

	CD3DX12_DESCRIPTOR_RANGE texTable2;
	texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 3, 0);


	CD3DX12_ROOT_PARAMETER slotRootParameter[7];



	slotRootParameter[0].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[2].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[3].InitAsConstantBufferView(0);
	slotRootParameter[4].InitAsConstantBufferView(1);
	slotRootParameter[5].InitAsConstantBufferView(2);
	slotRootParameter[6].InitAsConstantBufferView(3);


	auto staticSamplers = GetStaticSamplers();


	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(7, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void Dx12RenderSystem::renderImpl(Dx12Pass* pass)
{
	ICamera* camera = mCamera;
	VertexData* vertexData = pass->mRenderable->getVertexData();
	IndexData* indexData = pass->mRenderable->getIndexData();
	auto vd = vertexData->getVertexDeclaration();

	
	auto commandlist = mCurrentFrame->getCommandList();


	//bind texture
	int32_t cubeIndex = pass->mDx12RenderableData->getCubeTexStartIndex();
	if (cubeIndex >= 0)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;

		commandlist->SetGraphicsRootDescriptorTable(mCubeMapIndex, gpuHandle);
	}


	int32_t startIndex = pass->mDx12RenderableData->getTexStartIndex();
	if (startIndex >= 0)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;

		commandlist->SetGraphicsRootDescriptorTable(mTextureArrayIndex, gpuHandle);
	}

	//bind constant

	pass->mDx12RenderableData->updateData(pass, camera);
	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = pass->mDx12RenderableData->getObjectAddress();
	commandlist->SetGraphicsRootConstantBufferView(mObjectConstantIndex, objCBAddress);
	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = pass->mDx12RenderableData->getMaterialAddress();
	commandlist->SetGraphicsRootConstantBufferView(mMatConstantIndex, matCBAddress);


	//draw object

	vertexData->bind(commandlist);
	if(indexData)
		indexData->bind();

	commandlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	IndexDataView* view = pass->mRenderable->getIndexView();

	if (view)
	{
		commandlist->DrawIndexedInstanced(
			view->mIndexCount,
			1, view->mIndexLocation, view->mBaseVertexLocation, 0);
		mTriangleCount += view->mIndexCount / 3;
	}
	else
	{
		commandlist->DrawInstanced(
			vertexData->getVertexCount(), 1, 0, 0);
		mTriangleCount += vertexData->getVertexCount() / 3;
	}
	mBatchCount++;
}

void Dx12RenderSystem::updateMainPassCB(Ogre::ICamera* camera)
{
	/*updateMainPassCBForTest(camera);
	return;*/
	const Ogre::Matrix4& view = camera->getViewMatrix();
	const Ogre::Matrix4& proj = camera->getProjectMatrix();
	const Ogre::Vector3& camepos = camera->getDerivedPosition();

	mFrameConstantBuffer.Shadow = 0;

	if (camera->getCameraType() == CameraType_Light)
	{
		Ogre::Matrix4 T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);
		Ogre::Matrix4 S = view * proj * T;

		mFrameConstantBuffer.ShadowTransform = S;
		mFrameConstantBuffer.Shadow = 1;
	}

	int width = mRenderWindow->getWidth();
	int height = mRenderWindow->getHeight();

	mFrameConstantBuffer.View = view;
	mFrameConstantBuffer.InvView = view.inverse();
	mFrameConstantBuffer.Proj = proj;
	mFrameConstantBuffer.InvProj = proj.inverse();
	mFrameConstantBuffer.ViewProj = view * proj;
	mFrameConstantBuffer.InvViewProj = mFrameConstantBuffer.ViewProj.inverse();
	
	mFrameConstantBuffer.EyePosW = { camepos.x, camepos.y, camepos.z };
	mFrameConstantBuffer.RenderTargetSize = Ogre::Vector2((float)width, (float)height);
	mFrameConstantBuffer.InvRenderTargetSize = Ogre::Vector2(1.0f / width, 1.0f / height);
	mFrameConstantBuffer.NearZ = 1.0f;
	mFrameConstantBuffer.FarZ = 1000.0f;
	mFrameConstantBuffer.TotalTime += Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;
	mFrameConstantBuffer.DeltaTime = Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;

	
	UploadBuffer<FrameConstantBuffer>* cb = mCurrentFrame->getCameraFrameData(camera);
	cb->CopyData(0, mFrameConstantBuffer);

	D3D12_GPU_VIRTUAL_ADDRESS frameAddress = cb->Resource()->GetGPUVirtualAddress();
	mCurrentFrame->getCommandList()->SetGraphicsRootConstantBufferView(
		mMainConstantIndex, frameAddress);
}

#include <DirectXMath.h>
using namespace DirectX;
void Dx12RenderSystem::updateMainPassCBForTest(ICamera* camera)
{
	/*if (camera->getCameraType() == CameraType_Light)
	{
		
		UpdateShadowPassCBForTest(camera);
		return;
	}

	Ogre::Vector3 camepos = Ogre::Vector3(0, 0, 20);

	XMVECTOR lightPos = XMVectorSet(-2, 2, -10, 0.0f);
	XMVECTOR targetPos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX view = XMMatrixLookAtLH(lightPos, targetPos, lightUp);
	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.785, 1.33, 1, 200);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	mFrameConstantBuffer.Shadow = 0;

	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.View, view);
	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.InvView, invView);
	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.Proj, proj);
	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.InvProj, invProj);
	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.ViewProj, viewProj);
	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.InvViewProj, invViewProj);

	int width = mRenderWindow->getWidth();
	int height = mRenderWindow->getHeight();
	
	mFrameConstantBuffer.EyePosW = { camepos.x, camepos.y, camepos.z};
	mFrameConstantBuffer.RenderTargetSize = Ogre::Vector2((float)width, (float)height);
	mFrameConstantBuffer.InvRenderTargetSize = Ogre::Vector2(1.0f / width, 1.0f / height);
	mFrameConstantBuffer.NearZ = 1.0f;
	mFrameConstantBuffer.FarZ = 1000.0f;
	mFrameConstantBuffer.TotalTime += Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;
	mFrameConstantBuffer.DeltaTime = Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;
	mFrameConstantBuffer.AmbientLight = ColourValue(0.45f, 0.45f, 0.45f, 1.0f);
	mFrameConstantBuffer.directionLights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	mFrameConstantBuffer.directionLights[0].Strength = { 0.6f, 0.6f, 0.6f };


	UploadBuffer<FrameConstantBuffer>* cb = mCurrentFrame->getCameraFrameData(camera);
	cb->CopyData(0, mFrameConstantBuffer);

	D3D12_GPU_VIRTUAL_ADDRESS frameAddress = cb->Resource()->GetGPUVirtualAddress();
	mCurrentFrame->getCommandList()->SetGraphicsRootConstantBufferView(
		mMainConstantIndex, frameAddress);*/
}

void Dx12RenderSystem::UpdateShadowPassCBForTest(Ogre::ICamera* camera)
{
	/*Ogre::Vector3 camepos = Ogre::Vector3(-20.8166504, 20.8166504, -20.8166504);
	XMVECTOR lightPos = XMVectorSet(-20.8166504, 20.8166504, -20.8166504, 0.0f);
	XMVECTOR targetPos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX view = XMMatrixLookAtLH(lightPos, targetPos, lightUp);
	XMMATRIX proj = XMMatrixOrthographicOffCenterLH(-20, 20, -20, 20, 2, 60);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	mFrameConstantBuffer.Shadow = 1;

	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = view * proj * T;
	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.ShadowTransform, S);
	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.View, view);
	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.InvView, invView);
	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.Proj, proj);
	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.InvProj, invProj);
	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.ViewProj, viewProj);
	XMStoreFloat4x4((XMFLOAT4X4*)&mFrameConstantBuffer.InvViewProj, invViewProj);

	int width = 1024;
	int height = 1024;

	mFrameConstantBuffer.EyePosW = { camepos.x, camepos.y, camepos.z };
	mFrameConstantBuffer.RenderTargetSize = Ogre::Vector2((float)width, (float)height);
	mFrameConstantBuffer.InvRenderTargetSize = Ogre::Vector2(1.0f / width, 1.0f / height);
	mFrameConstantBuffer.NearZ = 1.0f;
	mFrameConstantBuffer.FarZ = 1000.0f;
	mFrameConstantBuffer.TotalTime += Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;
	mFrameConstantBuffer.DeltaTime = Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;



	UploadBuffer<FrameConstantBuffer>* cb = mCurrentFrame->getCameraFrameData(camera);
	cb->CopyData(0, mFrameConstantBuffer);

	D3D12_GPU_VIRTUAL_ADDRESS frameAddress = cb->Resource()->GetGPUVirtualAddress();
	mCurrentFrame->getCommandList()->SetGraphicsRootConstantBufferView(
		mMainConstantIndex, frameAddress);*/
}



