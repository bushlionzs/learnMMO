#pragma once

#include "OgreRenderWindow.h"
#include "d3dutil.h"
#include "dx12RenderTarget.h"
#include "dx12RenderTargetHelper.h"
class Dx12RenderSystem;
class Dx12ShadowMap;
class Dx12RenderWindow : public Ogre::RenderWindow, public Dx12RenderTarget
{
public:
	Dx12RenderWindow(Dx12RenderSystem* rs);
	~Dx12RenderWindow();

	virtual void create(const String& name, unsigned int width, unsigned int height,
		bool fullScreen, const NameValuePairList* miscParams);

	virtual void destroy();

	virtual void resize(unsigned int width, unsigned int height);

	virtual bool requiresTextureFlipping() const;
	void createSwapChain();
	void swapBuffers();
	D3D12_RESOURCE_STATES getResourceStates();
	void refreshMainWindow();
	virtual ID3D12Resource* getCurrentBackBuffer()const;
	virtual D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	virtual D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

	virtual void preRender(ID3D12GraphicsCommandList* cl);
	virtual void clearFrameBuffer(uint32_t buffers,
		const Ogre::ColourValue& colour,
		float depth, uint16_t stencil);
	void present();
private:
	HWND mWnd;
	Dx12RenderSystem* mRenderSystem;
	static const uint32_t mSwapChainBufferCount = 2;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[mSwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	uint32_t mCurrBackBufferIndex = 0;

	//for msaa
	Dx12RenderTargetHelper mMsaaRenderTarget;
	ColourValue mBackgroundColor;

	D3D12_CPU_DESCRIPTOR_HANDLE mCurrentRtv;
	D3D12_CPU_DESCRIPTOR_HANDLE mCurrentDst;

};