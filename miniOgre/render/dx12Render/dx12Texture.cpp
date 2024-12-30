#include "OgreHeader.h"
#include "dx12Texture.h"
#include "OgreImage.h"
#include "dx12RenderSystem.h"
#include "OgreResourceManager.h"
#include "dx12TextureHandleManager.h"
#include "myutils.h"
#include "dx12Helper.h"
#include "D3D12Mappings.h"
#include "dx12Commands.h"
#include "memoryAllocator.h"


Dx12Texture::Dx12Texture(
    const std::string& name, 
    Ogre::TextureProperty* texProperty, 
    DX12Commands* commands,
    DxDescriptorID descriptorId,
    DxDescriptorID renderTargetId)
    :
    mCommands(commands),
    OgreTexture(name, texProperty),
    mDescriptors(descriptorId),
    mTargetDescriptorID(renderTargetId)
{
    
}

Dx12Texture::Dx12Texture(
    const std::string& name,
    Ogre::TextureProperty* texProperty,
    DX12Commands* commands,
    ID3D12Resource* resource,
    DxDescriptorID descriptorId):
    OgreTexture(name, texProperty)
{
    mName = name;
    mCommands = commands;
    mTex = resource;
    mDescriptors = -1;
    mTargetDescriptorID = descriptorId;
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
    
    buildDescriptorHeaps();
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
    mMipLevels = mTextureProperty._numMipmaps + 1;

    auto width = getWidth();
    auto height = getHeight();
    if (need_midmap())
    {
        if (mTextureProperty._numMipmaps == 0)
        {
            auto current = static_cast<uint32_t>(floor(log2(std::max(width, height))) + 1.0);

            if (current > mMipLevels)
            {
                mNeedMipmaps = true;
                mMipLevels = current;
            }
        }
    }
    D3D12_RESOURCE_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = mTextureProperty._width;
    texDesc.Height = mTextureProperty._height;
    texDesc.DepthOrArraySize = mFace;
    texDesc.MipLevels = mMipLevels;
    texDesc.Format = mD3DFormat;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
   
    D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_COMMON;

    D3D12_CLEAR_VALUE* pvalue = nullptr;
    D3D12_CLEAR_VALUE ClearValue = {};
    if (mTextureProperty.isRenderTarget())
    {
        auto& backColor = mTextureProperty._backgroudColor;
        
        ClearValue.Format = D3D12Mappings::_getPF(mTextureProperty._tex_format);
        
        pvalue = &ClearValue;

        if (mTextureProperty._tex_usage & Ogre::TextureUsage::COLOR_ATTACHMENT)
        {
            memcpy(ClearValue.Color, &backColor, sizeof(float) * 4);
            texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
        else if (mTextureProperty._tex_usage & Ogre::TextureUsage::DEPTH_ATTACHMENT)
        {
            ClearValue.DepthStencil.Depth = 1.0f;
            ClearValue.DepthStencil.Stencil = 0.0f;
            texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }

        states = D3D12_RESOURCE_STATE_GENERIC_READ;
        texDesc.SampleDesc.Count = 1;

        mNeedMipmaps = false;
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

    uint32_t mysize = 0;
    {
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts;
        if (width > 1)
        {
            
            {
                UINT64 RequiredSize = 0;
                UINT64 MemToAlloc = static_cast<UINT64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * num2DSubresources;
                if (MemToAlloc > SIZE_MAX)
                {
                    return;
                }
                void* pMem = HeapAlloc(GetProcessHeap(), 0, static_cast<SIZE_T>(MemToAlloc));
                if (pMem == NULL)
                {
                    return;
                }
                pLayouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(pMem);
                UINT64* pRowSizesInBytes = reinterpret_cast<UINT64*>(pLayouts + num2DSubresources);
                UINT* pNumRows = reinterpret_cast<UINT*>(pRowSizesInBytes + num2DSubresources);

                D3D12_RESOURCE_DESC Desc = mTex->GetDesc();
                ID3D12Device* pDevice;
                mTex->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
                pDevice->GetCopyableFootprints(&Desc, 0, num2DSubresources, 0, pLayouts, pNumRows, pRowSizesInBytes, &RequiredSize);
                pDevice->Release();
            }
            auto bytePerPixel = PixelUtil::getNumElemBytes(mFormat);
            for (uint32_t face = 0; face < mFace; face++)
            {
                auto width = mTextureProperty._width;
                auto height = mTextureProperty._height;
                for (uint32_t mip = 0; mip < texDesc.MipLevels; mip++)
                {
                    uint32_t rowPitch = (width * bytePerPixel + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
                    uint32_t aa = rowPitch * height;
                    uint32_t bb = PixelUtil::getMemorySize(width, height, 1, mFormat);
                    //mysize += rowPitch * height;
                    if (mip == 5)
                    {
                        int kk = 0;
                    }
                    mysize += bb;
                    if (width > 4)
                    {
                        width /= 2;
                    }
                    else
                    {
                        int kk = 0;
                    }

                    if (height > 4)
                    {
                        height /= 2;
                    }
                }
            }

            int kk = 0;
        }
        
    }
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

void Dx12Texture::updateTexture(const std::vector<const CImage*>& images)
{
    uint32 faces = mFace;
    int32_t depth = 1;
    uint32_t offset = 0;

    BYTE* pData;
    HRESULT hr = mTexUpload->Map(0, NULL, reinterpret_cast<void**>(&pData));

    assert_invariant(hr == S_OK);

    
    updateLayoutInfos();

    uint32_t mips = this->getNumMipmaps();
    uint32_t num2DSubresources = mFace * mips;

    for (uint32 face = 0; face < mFace; ++face)
    {
        for (uint32 mip = 0; mip < mips; ++mip)
        {
            uint32_t i = face * mips + mip;
            PixelBox src;
            src = images[0]->getPixelBox(face, mip);
            D3D12_SUBRESOURCE_DATA srcData;
            srcData.pData = src.data;
            srcData.RowPitch = PixelUtil::getMemorySize(
                src.getWidth(),
                1,
                1,
                mFormat);
            srcData.SlicePitch = PixelUtil::getMemorySize(
                src.getWidth(),
                src.getHeight(),
                1,
                mFormat);
            D3D12_MEMCPY_DEST DestData = { pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, pLayouts[i].Footprint.RowPitch * pNumRows[i] };
            MemcpySubresource(&DestData, &srcData, (SIZE_T)pRowSizesInBytes[i], pNumRows[i], pLayouts[i].Footprint.Depth);
        }
    }

    mTexUpload->Unmap(0, nullptr);
}

void Dx12Texture::postLoad()
{
    if (mTextureProperty.isRenderTarget())
    {
        return;
    }

    uploadData();

    if (mNeedMipmaps)
    {
        generateMipmaps();
    }
   

    Dx12RenderSystemBase* rs = DX12Helper::getSingleton().getDx12RenderSystem();
    struct DescriptorHeap** heaps = rs->getCPUDescriptorHeaps();
    
}

void Dx12Texture::buildDescriptorHeaps()
{
    if (!mCreate)
    {
        auto device = DX12Helper::getSingleton().getDevice();
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        if (this->isCubeTexture())
        {
            srvDesc.Format = D3D12Mappings::util_to_dx12_srv_format(mTex->GetDesc().Format);
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MostDetailedMip = 0;
            srvDesc.TextureCube.MipLevels = mTex->GetDesc().MipLevels;
            srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
        }
        else
        {
            srvDesc.Format = D3D12Mappings::util_to_dx12_srv_format(mTex->GetDesc().Format);
            
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            

            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = mTex->GetDesc().MipLevels;
        }

        Dx12RenderSystemBase* rs = DX12Helper::getSingleton().getDx12RenderSystem();
        struct DescriptorHeap** heaps = rs->getCPUDescriptorHeaps();

        if (mTextureProperty._tex_usage & (uint32_t)Ogre::TextureUsage::DEPTH_ATTACHMENT)
        {
            int kk = 0;
        }
        if (mDescriptors >= 0)
        {
            auto cpuHandle = descriptor_id_to_cpu_handle(heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], mDescriptors);

            device->CreateShaderResourceView(mTex.Get(), &srvDesc, cpuHandle);
        }
        

        if (mTargetDescriptorID >= 0)
        {
            if (mTextureProperty._tex_usage & (uint32_t)Ogre::TextureUsage::COLOR_ATTACHMENT)
            {
                auto cpuHandle = descriptor_id_to_cpu_handle(heaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV], mTargetDescriptorID);
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                
                rtvDesc.Format = D3D12Mappings::_getPF(mTextureProperty._tex_format);
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                device->CreateRenderTargetView(mTex.Get(), &rtvDesc, cpuHandle);
            }
            else if (mTextureProperty._tex_usage & (uint32_t)Ogre::TextureUsage::DEPTH_ATTACHMENT)
            {
                auto cpuHandle = descriptor_id_to_cpu_handle(heaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV], mTargetDescriptorID);
                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
                dsvDesc.Format = mTex->GetDesc().Format;
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                device->CreateDepthStencilView(mTex.Get(), &dsvDesc, cpuHandle);
            }
        }

        mSamplerDescriptorID = DX12Helper::getSingleton().getSampler(mTextureProperty._samplerParams, heaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]);
        mCreate = true;
    }
}

void Dx12Texture::generateMipmaps()
{
    DX12Helper::getSingleton().generateMipmaps(this);
}

void Dx12Texture::blitFromMemory(
    const PixelBox& src, const Box& dst, uint32_t face, uint32_t mipmap)
{
    Vector3i srcSize(src.getWidth(), src.getHeight(), src.getDepth());
    Vector3i dstSize(dst.getWidth(), dst.getHeight(), dst.getDepth());
    if (srcSize != dstSize)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "invalid size");
    }

    BYTE* pData;
    HRESULT hr = mTexUpload->Map(0, NULL, reinterpret_cast<void**>(&pData));

    assert_invariant(hr == S_OK);
    PixelBox dstBox = PixelBox(src.getWidth(), src.getHeight(),
        src.getDepth(), mFormat, pData);
    PixelUtil::bulkPixelConversion(src, dstBox);
    mTexUpload->Unmap(0, nullptr);
}

void Dx12Texture::uploadData()
{
    ID3D12GraphicsCommandList* cl = mCommands->get();
    uint32_t mips = this->getNumMipmaps();
    uint32_t numSubresources = mFace * mips;

    uint32_t bytePerPixel = PixelUtil::getNumElemBytes(mFormat);
    for (uint32_t face = 0; face < mFace; face++)
    {

        for (uint32_t mip = 0; mip < mips; mip++)
        {
            uint32_t subResourceIndex = face * mips + mip;

            uint32_t width = pLayouts[subResourceIndex].Footprint.Width;
            uint32_t height = pLayouts[subResourceIndex].Footprint.Height;
            D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
            srcLocation.pResource = mTexUpload.Get();
            srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            srcLocation.PlacedFootprint.Offset = pLayouts[subResourceIndex].Offset;
            srcLocation.PlacedFootprint.Footprint.Format = mD3DFormat; // 确保与纹理格式匹配
            srcLocation.PlacedFootprint.Footprint.Width = width;
            srcLocation.PlacedFootprint.Footprint.Height = height;
            srcLocation.PlacedFootprint.Footprint.Depth = 1;
            srcLocation.PlacedFootprint.Footprint.RowPitch = pLayouts[subResourceIndex].Footprint.RowPitch;

            D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
            dstLocation.pResource = mTex.Get();
            dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

            UINT dstSubresource = D3D12CalcSubresource(
                mip, face, 0, mips, mFace);
            dstLocation.SubresourceIndex = dstSubresource;
            cl->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

            if (width > 4)
            {
                width /= 2;
            }

            if (height > 4)
            {
                height /= 2;
            }
        }
    }
}

bool Dx12Texture::need_midmap()
{
    if (!mTextureProperty._need_mipmap)
        return false;
    if (mTextureProperty._tex_usage & Ogre::TextureUsage::COLOR_ATTACHMENT)
    {
        return false;
    }
    if (mTextureProperty._tex_usage & Ogre::TextureUsage::DEPTH_ATTACHMENT)
    {
        return false;
    }
    return true;
}

void Dx12Texture::updateLayoutInfos()
{
    uint32_t num2DSubresources = mFace * getNumMipmaps();

    {
        UINT64 RequiredSize = 0;

        static void* mem = nullptr;
        if (mem == nullptr)
        {
            UINT64 MemToAlloc = static_cast<UINT64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * 512;
            mem = malloc(MemToAlloc);
        }

        pLayouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(mem);
        pRowSizesInBytes = reinterpret_cast<UINT64*>(pLayouts + num2DSubresources);
        pNumRows = reinterpret_cast<UINT*>(pRowSizesInBytes + num2DSubresources);

        D3D12_RESOURCE_DESC Desc = mTex->GetDesc();

        auto device = DX12Helper::getSingleton().getDevice();
        device->GetCopyableFootprints(&Desc, 0, num2DSubresources, 0, pLayouts, pNumRows, pRowSizesInBytes, &RequiredSize);

    }
}
