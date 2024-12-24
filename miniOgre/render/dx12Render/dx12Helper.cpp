#include "OgreHeader.h"
#include "dx12Helper.h"
#include "dx12RenderSystem.h"
#include "Dx12HardwareBuffer.h"
#include "dx12Shader.h"
#include "dx12Handles.h"
#include "D3D12Mappings.h"
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
	msQualityLevels.Format = mBackBufferFormat;
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

Dx12TextureHandleManager* DX12Helper::getDx12TextureMgr()
{
	return nullptr;
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

DXGI_FORMAT DX12Helper::getBackBufferFormat()
{
	return mBackBufferFormat;
}

DXGI_FORMAT DX12Helper::getDepthStencilFormat()
{
	return mDepthStencilFormat;
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

ID3D12GraphicsCommandList* DX12Helper::getCurrentCommandList()
{
	return nullptr;
}

ID3D12GraphicsCommandList* DX12Helper::getMipmapCommandList()
{
	return mMipmapCommandList.Get();
}

Dx12HardwareBuffer* DX12Helper::getMipmapVetexBuffer()
{
	return mMipmapVertexBuffer;
}

ID3D12RootSignature* DX12Helper::getMipmapRootSignature()
{
	return mRootSignatureMipmap.Get();
}

UploadBuffer<ObjectConstantBuffer>* DX12Helper::getMipmapFrameCB()
{
	return mMipmapFrameCB.get();
}

ID3D12PipelineState* DX12Helper::getMipmapPipelineState()
{
	return mMipmapPipelineState.Get();
}

void DX12Helper::_incrMipmapCommandList()
{
	mMipmapCommandCount++;
}

void DX12Helper::_submitCommandList(bool force)
{
	bool execute = false;
	if (mMipmapCommandCount > 5)
	{
		execute = true;
	}
	else
	{
		if (force & mMipmapCommandCount > 0)
		{
			execute = true;
		}
	}

	if (execute)
	{
		ID3D12GraphicsCommandList* cl = mMipmapCommandList.Get();
		cl->Close();
		ID3D12CommandList* dummy = (ID3D12CommandList*)cl;
		mCommandQueue->ExecuteCommandLists(1, &dummy);

		FlushCommandQueue();

		mMipmapCmdListAlloc->Reset();
		mMipmapCommandList->Reset(mMipmapCmdListAlloc.Get(), nullptr);

		mMipmapCommandCount = 0;

		
	}
}

void DX12Helper::_createMipmapPrepare()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	CD3DX12_DESCRIPTOR_RANGE texTable0;
	texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(mDx12Device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignatureMipmap)));

	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	psoDesc.pRootSignature = mRootSignatureMipmap.Get();

	
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.FrontCounterClockwise = true;
	psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
	psoDesc.RasterizerState.MultisampleEnable = true;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;

	ThrowIfFailed(mDx12Device.Get()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mMipmapPipelineState)));

	ThrowIfFailed(mDx12Device.Get()->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(mMipmapCmdListAlloc.GetAddressOf())));

	ThrowIfFailed(mDx12Device.Get()->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		mMipmapCmdListAlloc.Get(),
		nullptr,
		IID_PPV_ARGS(mMipmapCommandList.GetAddressOf())));



	struct MipmapVertex
	{
		Ogre::Vector3 _vertex;
		Ogre::Vector2 _coord;
	};
	mMipmapVertexBuffer = new Dx12HardwareBuffer(
		HardwareBuffer::VERTEX_BUFFER,
		sizeof(MipmapVertex),
		6,
		0
	);

	float d = 1.0f;
	std::array<MipmapVertex, 6> vertices;
	vertices[0]._vertex = Ogre::Vector3(-d, d, 0);
	vertices[0]._coord = Ogre::Vector2(0, 0);
	vertices[1]._vertex = Ogre::Vector3(d, d, 0);
	vertices[1]._coord = Ogre::Vector2(1, 0);
	vertices[2]._vertex = Ogre::Vector3(-d, -d, 0);
	vertices[2]._coord = Ogre::Vector2(0, 1);
	vertices[3]._vertex = Ogre::Vector3(-d, -d, 0);
	vertices[3]._coord = Ogre::Vector2(0, 1);
	vertices[4]._vertex = Ogre::Vector3(d, d, 0);
	vertices[4]._coord = Ogre::Vector2(1, 0);
	vertices[5]._vertex = Ogre::Vector3(d, -d, 0);
	vertices[5]._coord = Ogre::Vector2(1, 1);
	_incrMipmapCommandList();
	mMipmapVertexBuffer->_copydataimpl(mMipmapCommandList.Get(),
		vertices.data(), sizeof(MipmapVertex) * 6);
	_submitCommandList(true);
	mMipmapFrameCB = std::make_shared<UploadBuffer<ObjectConstantBuffer>>(mDx12Device.Get(), 1, true);

	ObjectConstantBuffer dummy;

	Ogre::Matrix4 proj;
	proj = Ogre::Math::makeOrthoRH(-1, 1, -1, 1, 1, 1000);

	Ogre::Matrix4 view = Ogre::Math::makeViewMatrix(
		Ogre::Vector3(0, 0, 15), Ogre::Quaternion::IDENTITY);

	mMipmapFrameCB->CopyData(0, dummy);
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