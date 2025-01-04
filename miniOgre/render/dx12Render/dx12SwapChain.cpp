#include <OgreHeader.h>
#include "dx12SwapChain.h"
#include "OgreRoot.h"
#include "dx12Helper.h"
#include "dx12RenderTarget.h"
#include "dx12RenderSystemBase.h"
#include "D3D12Mappings.h"
#include "memoryAllocator.h"

DX12SwapChain::DX12SwapChain(DX12Commands* commands, HWND hWnd, bool srgb)
{
    mCommands = commands;
	mHwnd = hWnd;

	mColorFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	mDepthFormat = DXGI_FORMAT_D32_FLOAT;

	if (srgb)
	{
		mColorFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	}

	mCurrentFrameIndex = 0;

	createSwapChain2(srgb);
}

void DX12SwapChain::present()
{
	mCommands->flush(false);
	ThrowIfFailed(mSwapChain3->Present(0, 0));
}

void DX12SwapChain::acquire(bool& reized)
{
	mCurrentFrameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
}

Dx12Texture* DX12SwapChain::getDepthTexture()
{
	return mDepth;
}

Dx12Texture* DX12SwapChain::getCurrentColor()
{
	return mColors[mCurrentFrameIndex];
}

void DX12SwapChain::createSwapChain2(bool srgb)
{
	auto device = DX12Helper::getSingleton().getDevice();
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	mWidth = ogreConfig.width;
	mHeight = ogreConfig.height;

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = ogreConfig.swapBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(device->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(device->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));

	DXGI_SWAP_CHAIN_DESC1 desc;
	desc.Width = mWidth;
	desc.Height = mHeight;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.Stereo = false;
	desc.SampleDesc.Count = 1; // If multisampling is needed, we'll resolve it later
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = ogreConfig.swapBufferCount;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	IDXGISwapChain1* swapchain;
	// Note: Swap chain uses queue to perform flush.
	IDXGIFactory4* pDXGIFactory = DX12Helper::getSingleton().getDXGIFactory();
	ThrowIfFailed(pDXGIFactory->CreateSwapChainForHwnd(
		mCommands->getCommandQueue(),
		mHwnd,
		&desc, NULL, NULL,
		&swapchain));

	ThrowIfFailed(pDXGIFactory->MakeWindowAssociation(mHwnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapchain->QueryInterface(IID_PPV_ARGS(&mSwapChain3)));
	swapchain->Release();
	DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
	
	//mSwapChain3->SetMaximumFrameLatency(1);
	mSwapChain3->SetColorSpace1(colorSpace);
	mColors.resize(ogreConfig.swapBufferCount);
	DescriptorHeapContext* context = DX12Helper::getSingleton().getHeapContext();
	TextureProperty texProperty;
	texProperty._tex_usage = Ogre::TextureUsage::COLOR_ATTACHMENT;
	texProperty._width = ogreConfig.width;
	texProperty._height = ogreConfig.height;
	texProperty._tex_format = D3D12Mappings::getPixelFormat(mColorFormat);

	struct DescriptorHeap* rtvHeap = context->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
	auto descriptors = consume_descriptor_handles(rtvHeap, ogreConfig.swapBufferCount);
	
	for (UINT i = 0; i < ogreConfig.swapBufferCount; i++)
	{
		ID3D12Resource* res;
		ThrowIfFailed(mSwapChain3->GetBuffer(i, IID_PPV_ARGS(&res)));

		/*D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle =
			descriptor_id_to_cpu_handle(rtvHeap, descriptors + i);
		device->CreateRenderTargetView(res, &rtvDesc, cpuHandle);*/

		mColors[i] = new Dx12Texture("colorTarget", &texProperty, mCommands, res);
	}



	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mWidth;
	depthStencilDesc.Height = mHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = mDepthFormat;
	depthStencilDesc.SampleDesc.Count = DX12Helper::getSingleton().hasMsaa() ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = DX12Helper::getSingleton().hasMsaa() ?
		(DX12Helper::getSingleton().getMsaaQuality() - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mDepthFormat;
	optClear.DepthStencil.Depth = ogreConfig.reverseDepth ? 0.0f : 1.0f;
	optClear.DepthStencil.Stencil = 0;


	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ID3D12Resource* depth;
	ThrowIfFailed(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optClear,
		IID_PPV_ARGS(&depth)));
	struct DescriptorHeap* dsvHeap = context->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV];
	auto descriptorId = consume_descriptor_handles(dsvHeap, 1);
	//D3D12_CPU_DESCRIPTOR_HANDLE depthHandle =
	//	descriptor_id_to_cpu_handle(dsvHeap, descriptorId);
	//// Create descriptor to mip level 0 of entire resource using the format of the resource.
	//device->CreateDepthStencilView(depth, nullptr, depthHandle);


	texProperty._tex_usage = Ogre::TextureUsage::DEPTH_ATTACHMENT;
	texProperty._tex_format = D3D12Mappings::getPixelFormat(mDepthFormat);

	mDepth = new Dx12Texture(std::string("depthTarget"), &texProperty, mCommands, depth);
}