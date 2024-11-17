#include <OgreHeader.h>
#include "dx12SwapChain.h"
#include "OgreRoot.h"
#include "dx12Helper.h"
#include "dx12RenderTarget.h"
#include "dx12RenderSystemBase.h"
#include "D3D12Mappings.h"

DX12SwapChain::DX12SwapChain(DX12Commands* commands, HWND hWnd)
{
    mCommands = commands;
	mHwnd = hWnd;
	createSwapChain();
}

void DX12SwapChain::present()
{
	mCommands->flush(false);
	ThrowIfFailed(mSwapChain->Present(0, 0));
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

void DX12SwapChain::createSwapChain()
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

	mSwapChain.Reset();
	
	auto format = DX12Helper::getSingleton().getBackBufferFormat();
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = ogreConfig.width;
	sd.BufferDesc.Height = ogreConfig.height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = format;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = ogreConfig.swapBufferCount;
	sd.OutputWindow = mHwnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(DX12Helper::getSingleton().getDXGIFactory()->CreateSwapChain(
		mCommands->getCommandQueue(),
		&sd,
		mSwapChain.GetAddressOf()));

	ThrowIfFailed(mSwapChain->ResizeBuffers(
		ogreConfig.swapBufferCount,
		mWidth, mHeight,
		format,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	mColors.resize(ogreConfig.swapBufferCount);
	auto* rs = DX12Helper::getSingleton().getDx12RenderSystem();
	Dx12TextureHandleManager* mgr = rs->getTextureHandleManager();
	TextureProperty texProperty;
	texProperty._tex_usage = Ogre::TextureUsage::COLOR_ATTACHMENT;
	texProperty._width = ogreConfig.width;
	texProperty._height = ogreConfig.height;
	texProperty._tex_format = D3D12Mappings::getPixelFormat(format);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < ogreConfig.swapBufferCount; i++)
	{
		ID3D12Resource* res;
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&res)));
		DX12Helper::getSingleton().getDevice()->CreateRenderTargetView(res, nullptr, rtvHeapHandle);
		
		mColors[i] = new Dx12Texture("colorTarget", &texProperty, mCommands, res, rtvHeapHandle);

		rtvHeapHandle.Offset(1, DX12Helper::getSingleton().getRtvDescriptorSize());
	}


	
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mWidth;
	depthStencilDesc.Height = mHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DX12Helper::getSingleton().getDepthStencilFormat();
	depthStencilDesc.SampleDesc.Count = DX12Helper::getSingleton().hasMsaa() ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = DX12Helper::getSingleton().hasMsaa() ?
		(DX12Helper::getSingleton().getMsaaQuality() - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DX12Helper::getSingleton().getDepthStencilFormat();
	optClear.DepthStencil.Depth = 1.0f;
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

	D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = 
		mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	device->CreateDepthStencilView(depth, nullptr, depthHandle);


	texProperty._tex_usage = Ogre::TextureUsage::DEPTH_ATTACHMENT;
	texProperty._tex_format = D3D12Mappings::getPixelFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);

	mDepth = new Dx12Texture(std::string("colorTarget"), &texProperty, mCommands, depth, depthHandle);
}