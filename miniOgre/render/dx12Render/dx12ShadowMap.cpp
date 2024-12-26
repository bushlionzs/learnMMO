#include "OgreHeader.h"
#include "dx12ShadowMap.h"
#include "dx12TextureHandleManager.h"
#include "dx12Helper.h"
#include "dx12RenderSystem.h"

Dx12ShadowMap::Dx12ShadowMap(
	uint32_t width,
	uint32_t height)
{
	md3dDevice = DX12Helper::getSingleton().getDevice();
	//mDx12TextureHandleManager = DX12Helper::getSingleton().getDx12RenderSystem()->getTextureHandleManager();
	mWidth = width;
	mHeight = height;

	mViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)height };

	buildResource();
}

Dx12ShadowMap::~Dx12ShadowMap()
{

}

uint32_t Dx12ShadowMap::width()const
{
	return mWidth;
}

uint32_t Dx12ShadowMap::height()const
{
	return mHeight;
}

ID3D12Resource* Dx12ShadowMap::resource()
{
	return mShadowMap.Get();
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Dx12ShadowMap::srvHandle()const
{
	return mCpuSrvHandle;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Dx12ShadowMap::gpuSrvHandle()const
{
	return mGpuSrvHandle;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Dx12ShadowMap::dsvHandle()const
{
	return mCpuDsvHandle;
}

D3D12_VIEWPORT Dx12ShadowMap::viewport()const
{
	return mViewport;
}

D3D12_RECT Dx12ShadowMap::scissorRect()const
{
	return mScissorRect;
}

void Dx12ShadowMap::buildDescriptors()
{
	int32_t index = mDx12TextureHandleManager->allocStartIndex();
	mCpuSrvHandle = mDx12TextureHandleManager->getCpuHandleByIndex(index);
	mGpuSrvHandle = mDx12TextureHandleManager->getGpuHandleByIndex(index);

	auto device = DX12Helper::getSingleton().getDevice();
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(device->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));

	mCpuDsvHandle = mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	//  Create the descriptors
	// Create SRV to resource so we can sample the shadow map in a shader program.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	md3dDevice->CreateShaderResourceView(mShadowMap.Get(), &srvDesc, mCpuSrvHandle);

	// Create DSV to resource so we can render to the shadow map.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(mShadowMap.Get(), &dsvDesc, mCpuDsvHandle);

}

void Dx12ShadowMap::buildResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = mFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&mShadowMap)));
}