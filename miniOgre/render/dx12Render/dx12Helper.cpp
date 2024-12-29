#include "OgreHeader.h"
#include <dxcapi.h>
#include "OgreVertexDeclaration.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreRenderTarget.h"
#include "dx12Helper.h"
#include "dx12RenderSystem.h"
#include "Dx12HardwareBuffer.h"
#include "dx12Shader.h"
#include "dx12Handles.h"
#include "dx12Texture.h"
#include "D3D12Mappings.h"
#include "OgreMeshManager.h"
#include "OgreTextureManager.h"
#include "memoryAllocator.h"

template<> DX12Helper* Ogre::Singleton<DX12Helper>::msSingleton = 0;

DX12Helper::DX12Helper(Dx12RenderSystemBase* rs)
{
	m4xMsaaState = false;
	mDx12RenderSystem = rs;
}

DX12Helper::~DX12Helper()
{

}

void DX12Helper::createBaseInfo()
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(&mDx12Device));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&mDx12Device)));
	}

	mRtvDescriptorSize = 
		mDx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = 
		mDx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = 
		mDx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(mDx12Device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m4xMsaaQuality = msQualityLevels.NumQualityLevels;

	ThrowIfFailed(mDx12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mFence)));

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(mDx12Device->CreateCommandQueue(&queueDesc, 
		IID_PPV_ARGS(&mCommandQueue)));
}

ID3D12Device* DX12Helper::getDevice()
{
	return mDx12Device.Get();
}

IDXGIFactory4* DX12Helper::getDXGIFactory()
{
	return mdxgiFactory.Get();
}

ID3D12CommandQueue* DX12Helper::getCommandQueue()
{
	return mCommandQueue.Get();
}


void DX12Helper::executeCommand(ID3D12CommandList* commandList)
{
	mCommandQueue->ExecuteCommandLists(1, &commandList);
}

void DX12Helper::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	mCurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (mFence->GetCompletedValue() < mCurrentFence)
	{
		waitFence(mCurrentFence);
	}
}

uint64_t DX12Helper::getFenceCompletedValue()
{
	return mFence->GetCompletedValue();
}

void DX12Helper::waitFence(uint64_t fence)
{
	HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
	ThrowIfFailed(mFence->SetEventOnCompletion(fence, eventHandle));
	WaitForSingleObject(eventHandle, INFINITE);
	CloseHandle(eventHandle);
}

uint64_t DX12Helper::incrFence()
{
	return ++mCurrentFence;
}

void DX12Helper::signalFence(uint64_t fence)
{
	mCommandQueue->Signal(mFence.Get(), fence);
}


uint32_t DX12Helper::getRtvDescriptorSize()
{
	return mRtvDescriptorSize;
}

uint32_t DX12Helper::getDsvDescriptorSize()
{
	return mDsvDescriptorSize;
}

uint32_t DX12Helper::getCbvSrvUavDescriptorSize()
{
	return mCbvSrvUavDescriptorSize;
}

bool DX12Helper::hasMsaa()
{
	return m4xMsaaState;
}

uint32_t DX12Helper::getMsaaQuality()
{
	return m4xMsaaQuality;
}

std::vector<ShaderResource> DX12Helper::parseShaderResource(
	ShaderStageFlags stageFlags,
	const char* byteCode,
	uint32_t byteCodeSize)
{
	std::vector<ShaderResource> resourceList;

	IDxcLibrary* pLibrary = NULL;
	DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&pLibrary));
	IDxcBlobEncoding* pBlob = NULL;
	pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)byteCode, byteCodeSize, 0, &pBlob);

	IDxcContainerReflection* pReflection;
	DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&pReflection));

	pReflection->Load(pBlob);

#define DXIL_FOURCC(ch0, ch1, ch2, ch3) \
    ((uint32_t)(uint8_t)(ch0) | (uint32_t)(uint8_t)(ch1) << 8 | (uint32_t)(uint8_t)(ch2) << 16 | (uint32_t)(uint8_t)(ch3) << 24)
	UINT32                   shaderIdx;
	(pReflection->FindFirstPartKind(DXIL_FOURCC('D', 'X', 'I', 'L'), &shaderIdx));
	ID3D12ShaderReflection* d3d12reflection = NULL;
	pReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&d3d12reflection));
	D3D12_SHADER_DESC shaderDesc;
	HRESULT hr = d3d12reflection->GetDesc(&shaderDesc);
	for (auto i = 0; i < shaderDesc.BoundResources; i++)
	{
		D3D12_SHADER_INPUT_BIND_DESC desc;
		d3d12reflection->GetResourceBindingDesc(i, &desc);
		if (strcmp(desc.Name, "$Globals") == 0)
		{
			assert(false);
		}
		resourceList.emplace_back();
		auto& back = resourceList.back();
		back.name = desc.Name;
		back.reg = desc.BindPoint;
		back.size = desc.BindCount;
		back.type = desc.Type;
		back.set = desc.Space;
		back.used_stages = (uint8_t)stageFlags;
	}
	pBlob->Release();
	pLibrary->Release();
	pReflection->Release();
	d3d12reflection->Release();
	return resourceList;
}


void DX12Helper::parseInputParams(
	const char* byteCode,
	uint32_t byteCodeSize,
	D3d12ShaderParameters& shaderInputParameters)
{
	IDxcLibrary* pLibrary = NULL;
	DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&pLibrary));
	IDxcBlobEncoding* pBlob = NULL;
	pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)byteCode, byteCodeSize, 0, &pBlob);

	IDxcContainerReflection* pReflection;
	DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&pReflection));

	pReflection->Load(pBlob);

#define DXIL_FOURCC(ch0, ch1, ch2, ch3) \
    ((uint32_t)(uint8_t)(ch0) | (uint32_t)(uint8_t)(ch1) << 8 | (uint32_t)(uint8_t)(ch2) << 16 | (uint32_t)(uint8_t)(ch3) << 24)
	UINT32                   shaderIdx;
	(pReflection->FindFirstPartKind(DXIL_FOURCC('D', 'X', 'I', 'L'), &shaderIdx));
	ID3D12ShaderReflection* d3d12reflection = NULL;
	pReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&d3d12reflection));
	D3D12_SHADER_DESC shaderDesc;
	HRESULT hr = d3d12reflection->GetDesc(&shaderDesc);
	shaderInputParameters.resize(shaderDesc.InputParameters);
	D3D12_SIGNATURE_PARAMETER_DESC curParam;
	for (auto i = 0; i < shaderDesc.InputParameters; i++)
	{
		d3d12reflection->GetInputParameterDesc(i, &curParam);
		D3D12ParamDesc& desc = shaderInputParameters[i];
		strncpy(desc.name, curParam.SemanticName, sizeof(desc.name));
		desc.semanticIndex = curParam.SemanticIndex;
	}
	pBlob->Release();
	pLibrary->Release();
	pReflection->Release();
	d3d12reflection->Release();

}

DxDescriptorID DX12Helper::getSampler(
	const filament::backend::SamplerParams& params, 
	DescriptorHeap* heap)
{
	auto iter = mSamplersCache.find(params);
	if (UTILS_LIKELY(iter != mSamplersCache.end())) {
		return iter->second;
	}

	D3D12_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12Mappings::getWrapMode(params.wrapS);
	samplerDesc.AddressV = D3D12Mappings::getWrapMode(params.wrapT);
	samplerDesc.AddressW = D3D12Mappings::getWrapMode(params.wrapR);
	samplerDesc.Filter = D3D12Mappings::getFilter(params);
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = params.anisotropyLog2 == 0 ? 0.0f : (float)(1u << params.anisotropyLog2);
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12Mappings::getMaxLod(params.mipMapMode);
	samplerDesc.ComparisonFunc = D3D12Mappings::getCompareOp(params.compareFunc);

	

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	DxDescriptorID id = consume_descriptor_handles(heap, 1);
	cpuHandle = descriptor_id_to_cpu_handle(heap, id);
	mDx12Device->CreateSampler(&samplerDesc, cpuHandle);

	mSamplersCache.insert({ params, id });
	return id;
}

void DX12Helper::generateMipmaps(Dx12Texture* tex)
{
	auto* rs = mDx12RenderSystem;
	ShaderInfo shaderInfo;
	shaderInfo.shaderName = "mipmap";

	if (!mMipmapHandle)
	{
		VertexDeclaration decl;
		decl.addElement(0, 0, 0, VET_FLOAT3, VES_POSITION);
		decl.addElement(0, 0, 12, VET_FLOAT3, VES_NORMAL);
		decl.addElement(0, 0, 24, VET_FLOAT2, VES_TEXTURE_COORDINATES);
		mMipmapHandle = rs->createShaderProgram(shaderInfo, &decl);
		backend::RasterState rasterState{};
		rasterState.colorWrite = true;
		rasterState.renderTargetCount = 1;
		rasterState.depthWrite = false;
		rasterState.depthTest = false;
		rasterState.pixelFormat = PF_A8B8G8R8;
		mMipmapPipelineHandle = rs->createPipeline(rasterState, mMipmapHandle);

		Ogre::TextureProperty texProperty;
		texProperty._width = 1024;
		texProperty._height = 1024;
		texProperty._tex_usage = Ogre::TextureUsage::COLOR_ATTACHMENT;
		texProperty._tex_format = PF_A8B8G8R8;

		mMipmapTarget = rs->createRenderTarget("outputTarget", texProperty);


		TextureManager::getSingleton().addTexture("outputTarget", mMipmapTarget->getTarget());

		mMipMapDescSet = rs->createDescriptorSet(mMipmapHandle, 0);

		mMipMapBlockHandle = rs->createBufferObject(
			BufferObjectBinding::BufferObjectBinding_Uniform,
			RESOURCE_MEMORY_USAGE_GPU_ONLY,
			0,
			sizeof(Ogre::Matrix4));

		
		DescriptorData descriptorData[1];

		descriptorData[0].pName = "cbPerObject";
		descriptorData[0].mCount = 1;
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[0].ppBuffers = &mMipMapBlockHandle;


		rs->updateDescriptorSet(mMipMapDescSet, 1, descriptorData);
	}


	if (tex == nullptr)
	{
		return;
	}
	DescriptorData descriptorData[2];

	descriptorData[0].pName = "first";
	descriptorData[0].mCount = 1;
	descriptorData[0].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[0].ppTextures =  (const OgreTexture**) & tex;

	descriptorData[1].pName = "firstSampler";
	descriptorData[1].mCount = 1;
	descriptorData[1].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[1].ppTextures = (const OgreTexture**)&tex;
	rs->updateDescriptorSet(mMipMapDescSet, 2, descriptorData);

	

	float aa = 1024.0f;
	Ogre::Vector3 leftop = Ogre::Vector3(0.0f, 0.0f, 0.0f);
	Ogre::Vector3 leftbottom = Ogre::Vector3(0.0f, aa, 0.0f);
	Ogre::Vector3 righttop = Ogre::Vector3(aa, 0.0f, 0.0f);
	Ogre::Vector3 rightbottom = Ogre::Vector3(aa, aa, 0.0f);
	Ogre::Vector3 normal = Ogre::Vector3(0.0f, 0.0f, 1.0f);

	std::string meshName = "mipmapMesh";

	auto mesh = MeshManager::getSingletonPtr()->createRect(
		nullptr,
		meshName,
		leftop, leftbottom, righttop, rightbottom, normal);
	auto* subMesh = mesh->getSubMesh(0);
	VertexData* vertexData = mesh->getVertexData();
	IndexData* indexData = mesh->getIndexData();

	RenderPassInfo renderPassInfo;
	renderPassInfo.renderTargetCount = 1;
	renderPassInfo.renderTargets[0].renderTarget = mMipmapTarget;
	renderPassInfo.depthTarget.depthStencil = nullptr;
	renderPassInfo.renderTargets[0].clearColour = { 1.0f, 0.0f, 0.0f, 1.0f };
	renderPassInfo.viewport = false;
	auto texWidth = tex->getWidth();
	auto texHeight = tex->getHeight();

	const uint32_t numMips = static_cast<uint32_t>(floor(log2(std::max(texWidth, texHeight))) + 1.0);

	Dx12Texture* srcTexture = (Dx12Texture*)mMipmapTarget->getTarget();

	Ogre::Matrix4 project;

	project = Ogre::Math::makeOrthoLH(
		0.0f, srcTexture->getWidth(), srcTexture->getHeight(), 0.0f, 0.1, 1000.0f);

	Ogre::Vector3 eyePos = Ogre::Vector3(0, 0, -10);
	Ogre::Matrix4 view = Ogre::Math::makeLookAtLH(eyePos, Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_Y);

	Ogre::Matrix4 viewProj = (project * view).transpose();

	rs->updateBufferObject(mMipMapBlockHandle, (const char*)&viewProj, sizeof(viewProj));

	for (uint32_t m = 1; m < numMips; m++)
	{
		uint32_t width = static_cast<float>(texWidth * std::pow(0.5f, m));
		uint32_t height = static_cast<float>(texHeight * std::pow(0.5f, m));
		rs->beginCmd();
		
		RenderTargetBarrier uavBarriers[] = {
				   {
				   mMipmapTarget,
				   RESOURCE_STATE_UNDEFINED,
				   RESOURCE_STATE_RENDER_TARGET},
		};
		rs->resourceBarrier(0, nullptr, 0, nullptr, 1, uavBarriers);
		rs->setViewport(0, 0, width, height, 0.0f, 1.0f);
		rs->setScissor(0, 0, width, height);
		rs->beginRenderPass(renderPassInfo);
		rs->bindPipeline(mMipmapHandle, mMipmapPipelineHandle, &mMipMapDescSet, 1);
		vertexData->bind(nullptr);
		indexData->bind();
		IndexDataView* indexView = subMesh->getIndexView();
		rs->drawIndexed(indexView->mIndexCount, 1,
			indexView->mIndexLocation, indexView->mBaseVertexLocation, 0);
		rs->endRenderPass(renderPassInfo);

		RenderTargetBarrier rtBarriers[] =
		{
			{
				mMipmapTarget,
				RESOURCE_STATE_RENDER_TARGET,
				RESOURCE_STATE_GENERIC_READ
			}
		};
		rs->resourceBarrier(0, nullptr, 0, nullptr, 1, rtBarriers);

		ImageCopyDesc copyRegion;

		copyRegion.srcSubresource.aspectMask = 0;
		copyRegion.srcSubresource.baseArrayLayer = 0;
		copyRegion.srcSubresource.mipLevel = 0;
		copyRegion.srcSubresource.layerCount = 1;

		copyRegion.dstSubresource.aspectMask = 0;
		copyRegion.dstSubresource.baseArrayLayer = 0;
		copyRegion.dstSubresource.mipLevel = m;
		copyRegion.dstSubresource.layerCount = 1;

		copyRegion.extent.width = width;
		copyRegion.extent.height = height;
		copyRegion.extent.depth = 1;
		rs->copyImage(tex, srcTexture, copyRegion);

		rtBarriers[0] =
		{
			mMipmapTarget,
			RESOURCE_STATE_GENERIC_READ,
			RESOURCE_STATE_RENDER_TARGET
		};
		rs->resourceBarrier(0, nullptr, 0, nullptr, 1, rtBarriers);
		rs->flushCmd(true);
	}
	
	rs->beginCmd();
}