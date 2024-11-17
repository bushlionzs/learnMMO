#include "OgreHeader.h"
#include "dx12Texture.h"
#include "OgreImage.h"
#include "dx12RenderSystem.h"
#include "OgreResourceManager.h"
#include "dx12TextureHandleManager.h"
#include "myutils.h"
#include "dx12Helper.h"
#include "dx12HardwarePixelBuffer.h"
#include "D3D12Mappings.h"
#include "dx12HardwareBuffer.h"
#include "dx12Commands.h"


Dx12Texture::Dx12Texture(
    const std::string& name, 
    Ogre::TextureProperty* texProperty, 
    DX12Commands* commands,
    Dx12TextureHandleManager* mgr)
    :
    mCommands(commands),
    mDx12TextureHandleManager(mgr),
    OgreTexture(name, texProperty)
{
    
}

Dx12Texture::Dx12Texture(
    const std::string& name,
    Ogre::TextureProperty* texProperty,
    DX12Commands* commands,
    ID3D12Resource* resource,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle):
    OgreTexture(name, texProperty)
{
    mName = name;
    mCommands = commands;
    mDx12TextureHandleManager = nullptr;
    mTex = resource;
    mCpuHandle = cpuHandle;
    createInternalResourcesImpl();
}

Dx12Texture::~Dx12Texture()
{
}

void Dx12Texture::_createSurfaceList(void)
{
    if (mTextureProperty._tex_usage & Ogre::TextureUsage::COLOR_ATTACHMENT)
    {
        return;
    }
    if (mTextureProperty._tex_usage & Ogre::TextureUsage::DEPTH_ATTACHMENT)
    {
        return;
    }

    if (mTextureProperty._texType == TEX_TYPE_3D)
    {
        return;
    }
    // Create new list of surfaces
    mSurfaceList.clear();
    size_t depth = mTextureProperty._depth;

    for (size_t face = 0; face < mFace; ++face)
    {
        size_t width = mTextureProperty._width;
        size_t height = mTextureProperty._height;
        for (size_t mip = 0; mip <= mTextureProperty._numMipmaps; ++mip)
        {

            Dx12HardwarePixelBuffer* buffer;
            buffer = new Dx12HardwarePixelBuffer(
                this, // parentTexture
                mip,
                width,
                height,
                depth,
                face,
                mFormat,
                (HardwareBuffer::Usage)mUsage
            );

            mSurfaceList.push_back(HardwarePixelBufferPtr(buffer));

            if (width > 1) width /= 2;
            if (height > 1) height /= 2;
            if (depth > 1 && mTextureProperty._texType != TEX_TYPE_2D_ARRAY) depth /= 2;
        }
    }
}

void Dx12Texture::createInternalResourcesImpl(void)
{
    mFormat = D3D12Mappings::_getClosestSupportedPF(mFormat);

    mD3DFormat = D3D12Mappings::_getGammaFormat(D3D12Mappings::_getPF(mFormat), false);


    switch (mTextureProperty._texType)
    {
    case TEX_TYPE_2D:
    case TEX_TYPE_CUBE_MAP:
    case TEX_TYPE_2D_ARRAY:
        this->_create2DTex();
        break;
    default:
        this->freeInternalResources();
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D11Texture::createInternalResources");
    }

    _createSurfaceList();
    
}

void Dx12Texture::freeInternalResourcesImpl()
{
    mTex.Reset();
    mTexUpload.Reset();

}

void Dx12Texture::_create2DTex()
{
    if (mTex)
        return;
    auto device = DX12Helper::getSingleton().getDevice();
    UINT numMips = mTextureProperty._numMipmaps + 1;
    D3D12_RESOURCE_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = mTextureProperty._width;
    texDesc.Height = mTextureProperty._height;
    texDesc.DepthOrArraySize = mFace;
    texDesc.MipLevels = numMips;
    texDesc.Format = mD3DFormat;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
   
    D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_COMMON;

    D3D12_CLEAR_VALUE* pvalue = nullptr;
    if (mTextureProperty.isRenderTarget())
    {
        auto& backColor = mTextureProperty._backgroudColor;
        D3D12_CLEAR_VALUE ClearValue = {};
        ClearValue.Format = getDxFormat();
        memcpy(ClearValue.Color, &backColor, sizeof(float) * 4);
        pvalue = &ClearValue;
        texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        states = D3D12_RESOURCE_STATE_GENERIC_READ;
        texDesc.SampleDesc.Count = 1;
    }
    else
    {
        texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    }

        
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        states,
        pvalue,
        IID_PPV_ARGS(&mTex)
    );

    if (FAILED(hr))
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "CreateCommittedResource failed!");
    }

    

    if (mTextureProperty.isRenderTarget())
    {
        return;
    }

    const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mTex.Get(), 0, num2DSubresources);
    heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&mTexUpload));

    if (FAILED(hr))
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "CreateCommittedResource failed!");
    }
}

void Dx12Texture::postLoad()
{
    if (mTextureProperty.isRenderTarget())
    {
        return;
    }


    updateTextureData();

   // generateMipmaps();
}

void Dx12Texture::generateMipmaps()
{
   

}

void Dx12Texture::updateTextureData()
{
    if (mSurfaceList.empty())
    {
        return;
    }
    std::vector<D3D12_SUBRESOURCE_DATA> subResourceData;
    subResourceData.resize(mSurfaceList.size());

    for (auto i = 0; i < mSurfaceList.size(); i++)
    {
        HardwareBufferLockGuard lock(mSurfaceList[i].get());
        subResourceData[i].pData = lock.data();
        subResourceData[i].RowPitch = PixelUtil::getMemorySize(
            mSurfaceList[i]->getWidth(), 1, 1, mSurfaceList[i]->getFormat());
        subResourceData[i].SlicePitch =
            PixelUtil::getMemorySize(
                mSurfaceList[i]->getWidth(),
                mSurfaceList[i]->getHeight(), 
                1, 
                mSurfaceList[i]->getFormat());
    }
    ID3D12GraphicsCommandList* cmdList = mCommands->get();
    UpdateSubresources(
        cmdList,
        mTex.Get(),
        mTexUpload.Get(),
        0,
        0,
        mSurfaceList.size(),
        subResourceData.data());
}

void Dx12Texture::buildDescriptorHeaps(int32_t handleIndex)
{
    auto device = DX12Helper::getSingleton().getDevice();
    if (!mCreate)
    {
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = mTex->GetDesc().MipLevels;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        if (this->isCubeTexture())
        {
            srvDesc.Format = mTex->GetDesc().Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MostDetailedMip = 0;
            srvDesc.TextureCube.MipLevels = mTex->GetDesc().MipLevels;
            srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
        }
        else
        {
            srvDesc.Format = mTex->GetDesc().Format;
            if (mTextureProperty.isRenderTarget())
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            }
            else
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            }
            
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = mTex->GetDesc().MipLevels;
        }

        mCpuHandle = mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        device->CreateShaderResourceView(mTex.Get(), &srvDesc, mCpuHandle);

        mCreate = true;
    }

    auto dstHandle = mDx12TextureHandleManager->getCpuHandleByIndex(handleIndex);
    device->CopyDescriptorsSimple(1, dstHandle, mCpuHandle,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}