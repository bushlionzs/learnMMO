#pragma once

#include "OgreTexture.h"
#include "d3dutil.h"
#include "engine_struct.h"
class DX12Commands;
class Dx12TextureHandleManager;
class Dx12HardwarePixelBuffer;
struct DX12Sampler;
class Dx12Texture :public OgreTexture
{
public:
    Dx12Texture(
        const std::string& name, 
        Ogre::TextureProperty* texProperty, 
        DX12Commands* commands,
        bool needSrv);

    Dx12Texture(
        const std::string& name,
        Ogre::TextureProperty* texProperty,
        DX12Commands* commands,
        ID3D12Resource* resource);

    ~Dx12Texture();

    

    ID3D12Resource* getResource()
    {
        return mTex.Get();
    }

    DXGI_FORMAT getDxFormat()
    {
        return mD3DFormat;
    }

    uint32_t getMipLevel()
    {
        return mTextureProperty._numMipmaps + 1;
    }


    DxDescriptorID getDescriptorId() const
    {
        return mDescriptors;
    }

    DxDescriptorID getTargetDescriptorId() const
    {
        return mTargetDescriptorID;
    }
    DxDescriptorID getSamplerDescriptorID()
    {
        return mSamplerDescriptorID;
    }

    virtual void blitFromMemory(
        const PixelBox& src, const Box& dstBox, uint32_t face, uint32_t mipmap)override;
    virtual void uploadData()override;
    void generateMipmaps();
private:
    virtual void createInternalResourcesImpl(void);
    virtual void freeInternalResourcesImpl(void);
    void _createTex();
    void _createSurfaceList(void);
    virtual void updateTexture(const std::vector<const CImage*>& images);
    virtual void postLoad();
    void buildDescriptorHeaps();
    bool need_midmap();
    void updateLayoutInfos();
private:

    Microsoft::WRL::ComPtr<ID3D12Resource> mTex;
    Microsoft::WRL::ComPtr<ID3D12Resource> mTexUpload;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts = nullptr;
    UINT64* pRowSizesInBytes = nullptr;
    UINT* pNumRows = nullptr;
    bool mCreate = false;
    DXGI_FORMAT mD3DFormat;
    DXGI_SAMPLE_DESC mFSAAType;

    int32_t mTexStartIndex = -1;

    DX12Commands* mCommands;


    bool mNeedSrv;
    DxDescriptorID mDescriptors;
    DxDescriptorID mTargetDescriptorID;


    DxDescriptorID mSamplerDescriptorID;
    uint32_t mMipLevels = 1;
    bool mNeedMipmaps = false;
};