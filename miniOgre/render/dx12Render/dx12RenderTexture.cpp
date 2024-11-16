#include "OgreHeader.h"
#include "dx12RenderTexture.h"
#include "dx12HardwarePixelBuffer.h"
#include "dx12Helper.h"
#include "dx12Texture.h"
#include "dx12TextureHandleManager.h"
#include "dx12RenderSystem.h"

Dx12RenderTexture::Dx12RenderTexture(
	const String& name, 
	Dx12HardwarePixelBuffer* buffer,
	uint32_t zoffset)
	:RenderTexture(buffer, zoffset)
{
	mRenderSystem = (Dx12RenderSystem*)DX12Helper::getSingleton().getDx12RenderSystem();
	mViewport = { 0.0f, 0.0f, (float)mWidth, (float)mHeight, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)mWidth, (int)mHeight };
	rebind(buffer);
}

Dx12RenderTexture::~Dx12RenderTexture()
{

}

void Dx12RenderTexture::rebind(Dx12HardwarePixelBuffer* buffer)
{
	mBuffer = buffer;
	mWidth = mBuffer->getWidth();
	mHeight = mBuffer->getHeight();
	auto device = DX12Helper::getSingleton().getDevice();

	mParentTexture = buffer->getParentTexture();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = mParentTexture->getDxFormat();
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap));

	auto rtvCpuStart = mRtvHeap->GetCPUDescriptorHandleForHeapStart();


	mCpuHandleOfRenderTarget = rtvCpuStart;
	mGpuHandleOfRenderTarget = mRtvHeap->GetGPUDescriptorHandleForHeapStart();

	ID3D12Resource* res = mParentTexture->getTextureResource();
	device->CreateRenderTargetView(res, 
		&rtvDesc, mCpuHandleOfRenderTarget);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = mParentTexture->getDxFormat();
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	auto texMgr = DX12Helper::getSingleton().getDx12TextureMgr();
	mResourceViewIndex = texMgr->allocStartIndex();
	mCpuHandleOfResourceView = texMgr->getCpuHandleByIndex(mResourceViewIndex);
	mGpuHandleOfResourceView = texMgr->getGpuHandleByIndex(mResourceViewIndex);
	// Create SRV to the entire cubemap resource.
	device->CreateShaderResourceView(mParentTexture->getTextureResource(), &srvDesc, mCpuHandleOfResourceView);


	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
	dsvDescriptorHeapDesc.NumDescriptors = 1;
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	ThrowIfFailed(device->CreateDescriptorHeap(&dsvDescriptorHeapDesc,
		IID_PPV_ARGS(mDSVDescriptorHeap.ReleaseAndGetAddressOf())));

	mCpuHandleOfDepth = mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	mGpuHandleOfDepth = mDSVDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		mWidth,
		mHeight,
		1, // This depth stencil view has only one texture.
		1, // Use a single mipmap level.
		1
	);
	depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

	ThrowIfFailed(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(mDepthStencil.ReleaseAndGetAddressOf())
	));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;

	device->CreateDepthStencilView(
		mDepthStencil.Get(), &dsvDesc,
		mCpuHandleOfDepth);
}

void Dx12RenderTexture::copyContentsToMemory(
	const Box& src,
	const PixelBox& dst,
	FrameBuffer buffer)
{

}

bool Dx12RenderTexture::requiresTextureFlipping() const
{
	return false;
}

void Dx12RenderTexture::preRender(ID3D12GraphicsCommandList* cl)
{
	cl->RSSetViewports(1, &mViewport);
	cl->RSSetScissorRects(1, &mScissorRect);
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(mParentTexture->getTextureResource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	cl->ResourceBarrier(1, &barrier);

}

void Dx12RenderTexture::swapBuffers()
{
	auto cl = mRenderSystem->getCommandList();
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(mParentTexture->getTextureResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	cl->ResourceBarrier(1, &barrier);

}

D3D12_RESOURCE_STATES Dx12RenderTexture::getResourceStates()
{
	return D3D12_RESOURCE_STATE_RENDER_TARGET;
}

ID3D12Resource* Dx12RenderTexture::getCurrentBackBuffer() const
{
	return mParentTexture->getTextureResource();
}

D3D12_CPU_DESCRIPTOR_HANDLE Dx12RenderTexture::CurrentBackBufferView() const
{
	return mCpuHandleOfRenderTarget;
}

D3D12_CPU_DESCRIPTOR_HANDLE Dx12RenderTexture::DepthStencilView() const
{
	return mCpuHandleOfDepth;
}

void Dx12RenderTexture::clearFrameBuffer(uint32_t buffers,
	const Ogre::ColourValue& colour,
	float depth, uint16_t stencil)
{
	auto cl = mRenderSystem->getCommandList();
	auto backBuffer = getCurrentBackBuffer();


	auto backBufferView = CurrentBackBufferView();
	auto depthStencilView = DepthStencilView();

	// Specify the buffers we are going to render to.
	cl->OMSetRenderTargets(1, &backBufferView, true, &depthStencilView);

	// Clear the back buffer and depth buffer.
	cl->ClearRenderTargetView(backBufferView, colour.ptr(), 0, nullptr);
	cl->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}
