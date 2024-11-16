#include "OgreHeader.h"
#include "dx12RenderWindow.h"
#include "OgreStringConverter.h"
#include "dx12Helper.h"
#include "dx12RenderSystem.h"
#include "dx12Frame.h"
#include "OgreStringConverter.h"
#include "dx12ShadowMap.h"

Dx12RenderWindow::Dx12RenderWindow(Dx12RenderSystem* rs)
{
	mWnd = 0;
	mRenderSystem = rs;
	mBackgroundColor = ColourValue::Black;
}

Dx12RenderWindow::~Dx12RenderWindow()
{

}

void Dx12RenderWindow::create(const String& name, unsigned int width, unsigned int height,
	bool fullScreen, const NameValuePairList* miscParams)
{
	auto itor = miscParams->find("externalWindowHandle");
	if (itor == miscParams->end())
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "externalWindowHandle should be provided");
	}

	mWnd = (HWND)StringConverter::parseSizeT(itor->second);

	itor = miscParams->find("backGroundColor");
	if (itor != miscParams->end())
	{
		ColourValue value;
		if (StringConverter::parse(itor->second, value))
		{
			mBackgroundColor = value;
		}
	}

	

	auto device = DX12Helper::getSingleton().getDevice();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = mSwapChainBufferCount;
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

	resize(width, height);
}


void Dx12RenderWindow::createSwapChain()
{
	// Release the previous swapchain we will be recreating.
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mWidth;
	sd.BufferDesc.Height = mHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DX12Helper::getSingleton().getBackBufferFormat();
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = mSwapChainBufferCount;
	sd.OutputWindow = mWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(DX12Helper::getSingleton().getDXGIFactory()->CreateSwapChain(
		DX12Helper::getSingleton().getCommandQueue(),
		&sd,
		mSwapChain.GetAddressOf()));
}


D3D12_RESOURCE_STATES Dx12RenderWindow::getResourceStates()
{
	return D3D12_RESOURCE_STATE_PRESENT;
}

void Dx12RenderWindow::destroy()
{

}

void Dx12RenderWindow::resize(unsigned int width, unsigned int height)
{
	if (width == mWidth && height == mHeight)
	{
		return;
	}
	refreshMainWindow();
	
	// Release the previous resources we will be recreating.
	for (int i = 0; i < mSwapChainBufferCount; ++i)
		mSwapChainBuffer[i].Reset();
	mDepthStencilBuffer.Reset();

	createSwapChain();
	// Resize the swap chain.
	ThrowIfFailed(mSwapChain->ResizeBuffers(
		mSwapChainBufferCount,
		mWidth, mHeight,
		DX12Helper::getSingleton().getBackBufferFormat(),
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	mCurrBackBufferIndex = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < mSwapChainBufferCount; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
		DX12Helper::getSingleton().getDevice()->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, DX12Helper::getSingleton().getRtvDescriptorSize());
	}

	// Create the depth/stencil buffer and view.
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

	auto device = DX12Helper::getSingleton().getDevice();
	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optClear,
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	device->CreateDepthStencilView(mDepthStencilBuffer.Get(), nullptr, DepthStencilView());

	

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mWidth);
	mScreenViewport.Height = static_cast<float>(mHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, (LONG)mWidth, (LONG)mHeight };

	mMsaaRenderTarget.createResource(mWidth, mHeight, mBackgroundColor, false);
}

ID3D12Resource* Dx12RenderWindow::getCurrentBackBuffer()const
{
	return mSwapChainBuffer[mCurrBackBufferIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Dx12RenderWindow::CurrentBackBufferView()const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
		mCurrBackBufferIndex,
		DX12Helper::getSingleton().getRtvDescriptorSize());
}


D3D12_CPU_DESCRIPTOR_HANDLE Dx12RenderWindow::DepthStencilView()const
{
	return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void Dx12RenderWindow::preRender(ID3D12GraphicsCommandList* cl)
{
	auto cam = mRenderSystem->getCamera();

	
}

void Dx12RenderWindow::clearFrameBuffer(uint32_t buffers,
	const Ogre::ColourValue& colour,
	float depth, uint16_t stencil)
{
	auto cl = mRenderSystem->getCommandList();
	auto cam = mRenderSystem->getCamera();
	

	ID3D12Resource* renderTarget = mMsaaRenderTarget.getRenderTarget();
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		renderTarget,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	cl->ResourceBarrier(1, &barrier);
	cl->ClearRenderTargetView(mCurrentRtv, colour.ptr(), 0, nullptr);
	
	
	cl->ClearDepthStencilView(mCurrentDst, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Dx12RenderWindow::present()
{
	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBufferIndex = (mCurrBackBufferIndex + 1) % mSwapChainBufferCount;
	auto cl = mRenderSystem->getCommandList();
}


void Dx12RenderWindow::swapBuffers()
{
	ID3D12Resource* renderTarget = mMsaaRenderTarget.getRenderTarget();
	ID3D12Resource* depthStencil = mMsaaRenderTarget.getDepthStencil();
	auto backBuffer = getCurrentBackBuffer();
	auto cl = mRenderSystem->getCommandList();
	D3D12_RESOURCE_BARRIER barriers[2] =
	{
		CD3DX12_RESOURCE_BARRIER::Transition(
			renderTarget,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_COPY_SOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer,
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_COPY_DEST)
	};

	cl->ResourceBarrier(2, barriers);
	cl->CopyResource(backBuffer, renderTarget);
	/*cl->ResolveSubresource(
		backBuffer,
		0, renderTarget, 0,
		DX12Helper::getSingleton().getBackBufferFormat());*/
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		backBuffer,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PRESENT);
	cl->ResourceBarrier(1, &barrier);
}

bool Dx12RenderWindow::requiresTextureFlipping() const
{
	return false;
}

void Dx12RenderWindow::refreshMainWindow()
{
	RECT rect;
	::GetClientRect(mWnd, &rect);
	mWidth = rect.right - rect.left;
	mHeight = rect.bottom - rect.top;
}