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
    bool needSrv)
    :
    mCommands(commands),
    OgreTexture(name, texProperty),
    mNeedSrv(needSrv)
{
    
}

Dx12Texture::Dx12Texture(
    const std::string& name,
    Ogre::TextureProperty* texProperty,
    DX12Commands* commands,
    ID3D12Resource* resource):
    mNeedSrv(false),
    OgreTexture(name, texProperty)
{
    mName = name;
    mCommands = commands;
    mTex = resource;
    mDescriptors = -1;
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
    case TEX_TYPE_3D:
    case TEX_TYPE_CUBE_MAP:
    case TEX_TYPE_2D_ARRAY:
        _createTex();
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

void Dx12Texture::_createTex()
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

            current = std::min(current, mTextureProperty._maxMipLevel);
            if (current > mMipLevels)
            {
                if (mTextureProperty._texType != TEX_TYPE_3D)
                {
                    mNeedMipmaps = true;
                }
                mMipLevels = current;
            }
        }
    }
    D3D12_RESOURCE_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    if (mTextureProperty._texType == TEX_TYPE_3D)
    {
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        texDesc.DepthOrArraySize = mTextureProperty._depth;
    }
    else if (mTextureProperty._texType == TEX_TYPE_CUBE_MAP)
    {
        texDesc.DepthOrArraySize = mFace;
    }
    else
    {
        texDesc.DepthOrArraySize = 1;
    }
    texDesc.Alignment = 0;
    texDesc.Width = mTextureProperty._width;
    texDesc.Height = mTextureProperty._height;    
    texDesc.MipLevels = mMipLevels;
    texDesc.Format = mD3DFormat;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
   
    D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_COMMON;

    D3D12_CLEAR_VALUE* pvalue = nullptr;
    D3D12_CLEAR_VALUE ClearValue{};

    if (mTextureProperty.isRenderTarget())
    {
        auto& backColor = mTextureProperty._backgroudColor;
        
        ClearValue.Format = D3D12Mappings::_getPF(mTextureProperty._tex_format);
        
        if (mTextureProperty._tex_usage & Ogre::TextureUsage::COLOR_ATTACHMENT)
        {
            memcpy(ClearValue.Color, &backColor, sizeof(float) * 4);
            texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            pvalue = &ClearValue;
        }
        else if (mTextureProperty._tex_usage & Ogre::TextureUsage::DEPTH_ATTACHMENT)
        {
            ClearValue.DepthStencil.Depth = 1.0f;
            ClearValue.DepthStencil.Stencil = 0.0f;
            texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            pvalue = &ClearValue;
        }

        states = D3D12_RESOURCE_STATE_GENERIC_READ;
        texDesc.SampleDesc.Count = 1;

        mNeedMipmaps = false;
    }
    else
    {
        texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        if (mTextureProperty._tex_usage & Ogre::TextureUsage::WRITEABLE)
        {
            texDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }
        
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

    CD3DX12_RESOURCE_DESC bufferDesc;
    if (mTextureProperty._texType != TEX_TYPE_3D)
    {
        const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mTex.Get(), 0, num2DSubresources);
        heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

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
    
    PixelFormat imageFormat = images[0]->getFormat();
    if (mFormat == imageFormat)
    {
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
    }
    else
    {
        uint32_t memSize = 0;
        for (uint32 face = 0; face < mFace; ++face)
        {
            uint32_t width = mTextureProperty._width;
            uint32_t height = mTextureProperty._height;
            for (uint32 mip = 0; mip < mips; ++mip)
            {
                memSize += PixelUtil::getMemorySize(width, height, 1, mFormat);

                if (width > 1)
                    width /= 2;
                if (height > 1)
                    height /= 2;
            }
        }

        std::string mem;
        mem.resize(memSize);

        for (uint32 face = 0; face < mFace; ++face)
        {
            uint32_t width = mTextureProperty._width;
            uint32_t height = mTextureProperty._height;
            for (uint32 mip = 0; mip < mips; ++mip)
            {
                Vector3i srcSize(width, height, 1);
                Vector3i dstSize(width, height, 1);

                PixelBox dstBox = PixelBox(width, height, 1, mFormat, mem.data());
                PixelBox src = images[0]->getPixelBox(face, mip);
                PixelUtil::bulkPixelConversion(src, dstBox);

                if (width > 1)
                    width /= 2;
                if (height > 1)
                    height /= 2;
            }
        }
        
        for (uint32 face = 0; face < mFace; ++face)
        {
            uint32_t width = mTextureProperty._width;
            uint32_t height = mTextureProperty._height;
            for (uint32 mip = 0; mip < mips; ++mip)
            {
                uint32_t i = face * mips + mip;

                D3D12_SUBRESOURCE_DATA srcData;
                srcData.pData = mem.data() + offset;
                srcData.RowPitch = PixelUtil::getMemorySize(
                    width,
                    1,
                    1,
                    mFormat);
                srcData.SlicePitch = PixelUtil::getMemorySize(
                    width,
                    height,
                    1,
                    mFormat);
                D3D12_MEMCPY_DEST DestData = { pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, pLayouts[i].Footprint.RowPitch * pNumRows[i] };
                MemcpySubresource(&DestData, &srcData, (SIZE_T)pRowSizesInBytes[i], pNumRows[i], pLayouts[i].Footprint.Depth);
                offset += PixelUtil::getMemorySize(width, height, 1, mFormat);
                if (width > 1)
                    width /= 2;
                if (height > 1)
                    height /= 2;
            }
        }
    }
    

    mTexUpload->Unmap(0, nullptr);
}

void Dx12Texture::postLoad()
{
    if (!mTextureProperty.haveImageFile())
    {
        return;
    }

    uploadData();

    if (mNeedMipmaps)
    {
        generateMipmaps();
    }
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
            if (mTextureProperty._texType == TEX_TYPE_3D)
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            }

            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = mTex->GetDesc().MipLevels;
        }


        DescriptorHeapContext* context = DX12Helper::getSingleton().getHeapContext();

        if (mNeedSrv)
        {
            if (mName == "voxelTextureNegX")
            {

            }
            mDescriptors = consume_descriptor_handles(
                context->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1);
            auto cpuHandle = descriptor_id_to_cpu_handle(
                context->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], mDescriptors);

            device->CreateShaderResourceView(mTex.Get(), &srvDesc, cpuHandle);
        }
        else
        {
            mDescriptors = -1;
        }


        if (mTextureProperty._tex_usage & (uint32_t)Ogre::TextureUsage::COLOR_ATTACHMENT)
        {
            mTargetDescriptorID = consume_descriptor_handles(
                context->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV], 1);
            auto cpuHandle = descriptor_id_to_cpu_handle(
                context->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV], mTargetDescriptorID);
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

            rtvDesc.Format = D3D12Mappings::_getPF(mTextureProperty._tex_format);
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            device->CreateRenderTargetView(mTex.Get(), &rtvDesc, cpuHandle);
        }
        else if (mTextureProperty._tex_usage & (uint32_t)Ogre::TextureUsage::DEPTH_ATTACHMENT)
        {
            mTargetDescriptorID = consume_descriptor_handles(
                context->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV], 1);
            auto cpuHandle = descriptor_id_to_cpu_handle(
                context->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV], mTargetDescriptorID);
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = mTex->GetDesc().Format;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            device->CreateDepthStencilView(mTex.Get(), &dsvDesc, cpuHandle);
        }
        else if (mTextureProperty._tex_usage & (uint32_t)Ogre::TextureUsage::WRITEABLE)
        {
            mTargetDescriptorID = consume_descriptor_handles(
                context->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], mMipLevels);

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = mTex->GetDesc().Format;
            if (mTextureProperty._texType == TEX_TYPE_3D)
            {
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                uavDesc.Texture3D.WSize = -1;
            }
            else
            {
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            }
            if (mMipLevels == 1)
            {
                int kk = 0;
            }
            assert(mMipLevels <= D3D12_MAX_MIPMAP_COUNT);

            for (uint32_t i = 0; i < mMipLevels; i++)
            {
                auto cpuHandle = descriptor_id_to_cpu_handle(
                    context->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], mTargetDescriptorID + i);

                if (mTextureProperty._texType == TEX_TYPE_3D)
                {
                    uavDesc.Texture3D.MipSlice = i;
                }
                else
                {
                    uavDesc.Texture2D.MipSlice = i;
                }
                
                device->CreateUnorderedAccessView(mTex.Get(), nullptr, &uavDesc, cpuHandle);
            }

        }
        mSamplerDescriptorID = DX12Helper::getSingleton().getSampler(mTextureProperty._samplerParams);
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

    if (src.format == mFormat)
    {
        BYTE* pData;
        HRESULT hr = mTexUpload->Map(0, NULL, reinterpret_cast<void**>(&pData));

        assert_invariant(hr == S_OK);
        PixelBox dstBox = PixelBox(src.getWidth(), src.getHeight(),
            src.getDepth(), mFormat, pData);
        PixelUtil::bulkPixelConversion(src, dstBox);
        mTexUpload->Unmap(0, nullptr);
    }
    else
    {
        assert(false);
    }
}

void Dx12Texture::uploadData()
{
    ID3D12GraphicsCommandList* cl = mCommands->get();
    uint32_t mips = this->getNumMipmaps();
    uint32_t numSubresources = mFace * mips;

    uint32_t bytePerPixel = PixelUtil::getNumElemBytes(mFormat);

    updateLayoutInfos();

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


    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(mTex.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    cl->ResourceBarrier(1, &barrier);
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
    if (pLayouts)
    {
        return;
    }
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
