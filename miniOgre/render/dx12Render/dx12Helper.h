#pragma once

#include "OgreSingleton.h"
#include "UploadBuffer.h"
#include "dx12Common.h"


class Dx12RenderSystemBase;
class Dx12Shader;
struct DX12Sampler;
class Dx12Texture;


class DX12Helper : public Ogre::Singleton<DX12Helper>
{
public:
	DX12Helper(Dx12RenderSystemBase* rs);
	~DX12Helper();

	void createBaseInfo();

	ID3D12Device* getDevice();
	IDXGIFactory4* getDXGIFactory();
	ID3D12CommandQueue* getCommandQueue();
	void executeCommand(ID3D12CommandList* commandList);

	void FlushCommandQueue();

	uint64_t getFenceCompletedValue();
	void waitFence(uint64_t fence);
	uint64_t incrFence();
	void signalFence(uint64_t fence);

	uint32_t getRtvDescriptorSize();
	uint32_t getDsvDescriptorSize();
	uint32_t getCbvSrvUavDescriptorSize();

	bool hasMsaa();
	uint32_t getMsaaQuality();

	Dx12RenderSystemBase* getDx12RenderSystem()
	{
		return mDx12RenderSystem;
	}


	std::vector<ShaderResource> parseShaderResource(
		ShaderStageFlags stageFlags,
		const char* byteCode,
		uint32_t byteCodeSize);

	void parseInputParams(const char* byteCode,
		uint32_t byteCodeSize,
		D3d12ShaderParameters& parameters);


	DxDescriptorID getSampler(
		const filament::backend::SamplerParams& params);

	void generateMipmaps(Dx12Texture* tex);

	DescriptorHeapContext* getHeapContext()
	{
		return &mDescriptorHeapContext;
	}
private:
	ComPtr<ID3D12Device> mDevice;
	ComPtr<IDXGIFactory4> mdxgiFactory;
    ComPtr<ID3D12Fence> mFence;
	ComPtr<ID3D12CommandQueue> mCommandQueue;
	uint32_t mRtvDescriptorSize = 0;
	uint32_t mDsvDescriptorSize = 0;
	uint32_t mCbvSrvUavDescriptorSize = 0;

	uint64_t mCurrentFence = 0;


	bool      m4xMsaaState = false;    // 4X MSAA enabled
	uint32_t  m4xMsaaQuality = 0;      // quality level of 4X MSAA

	Dx12RenderSystemBase* mDx12RenderSystem;
	Handle<HwProgram> mMipmapHandle;
	Handle<HwPipeline> mMipmapPipelineHandle;
	Handle<HwDescriptorSet> mMipMapDescSet;
	Handle<HwBufferObject> mMipMapBlockHandle;
	Handle<HwSampler> mMipMapSamplerHandle;
	Ogre::RenderTarget* mMipmapTarget;
	tsl::robin_map<SamplerParams, DxDescriptorID, SamplerParams::Hasher, SamplerParams::EqualTo> mSamplersCache;


	DescriptorHeapContext mDescriptorHeapContext;
};