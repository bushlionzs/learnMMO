#pragma once

#include "OgreSingleton.h"
#include "UploadBuffer.h"
#include "dx12Common.h"


class Dx12RenderSystemBase;
class Dx12TextureHandleManager;
class Dx12HardwareBuffer;
class Dx12Shader;

class DX12Helper : public Ogre::Singleton<DX12Helper>
{
public:
	DX12Helper(Dx12RenderSystemBase* rs);
	~DX12Helper();

	void createBaseInfo();

	ID3D12Device* getDevice();
	IDXGIFactory4* getDXGIFactory();
	ID3D12CommandQueue* getCommandQueue();
	Dx12TextureHandleManager* getDx12TextureMgr();
	void executeCommand(ID3D12CommandList* commandList);

	void FlushCommandQueue();

	uint64_t getFenceCompletedValue();
	void waitFence(uint64_t fence);
	uint64_t incrFence();
	void signalFence(uint64_t fence);
	DXGI_FORMAT getBackBufferFormat();
	DXGI_FORMAT getDepthStencilFormat();
	uint32_t getRtvDescriptorSize();
	uint32_t getDsvDescriptorSize();
	uint32_t getCbvSrvUavDescriptorSize();

	bool hasMsaa();
	uint32_t getMsaaQuality();

	Dx12RenderSystemBase* getDx12RenderSystem()
	{
		return mDx12RenderSystem;
	}
	ID3D12GraphicsCommandList* getCurrentCommandList();
	ID3D12GraphicsCommandList* getMipmapCommandList();
	Dx12HardwareBuffer* getMipmapVetexBuffer();
	ID3D12RootSignature* getMipmapRootSignature();
	UploadBuffer<ObjectConstantBuffer>* getMipmapFrameCB();
	ID3D12PipelineState* getMipmapPipelineState();
	void _incrMipmapCommandList();
	void _submitCommandList(bool force = false);
	void _createMipmapPrepare();
private:
	
private:
	ComPtr<ID3D12Device> mDx12Device;
	ComPtr<IDXGIFactory4> mdxgiFactory;
    ComPtr<ID3D12Fence> mFence;
	ComPtr<ID3D12CommandQueue> mCommandQueue;
	uint32_t mRtvDescriptorSize = 0;
	uint32_t mDsvDescriptorSize = 0;
	uint32_t mCbvSrvUavDescriptorSize = 0;

	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	uint64_t mCurrentFence = 0;


	bool      m4xMsaaState = false;    // 4X MSAA enabled
	uint32_t  m4xMsaaQuality = 0;      // quality level of 4X MSAA

	Dx12RenderSystemBase* mDx12RenderSystem;

	//for directx12 mipmap
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureMipmap = nullptr;
	Dx12HardwareBuffer* mMipmapVertexBuffer;
	Dx12Shader* mMipmapShader;
	std::shared_ptr<UploadBuffer<ObjectConstantBuffer>> mMipmapFrameCB;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mMipmapCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mMipmapCommandList;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mMipmapPipelineState;
	uint32_t mMipmapCommandCount = 0;
};