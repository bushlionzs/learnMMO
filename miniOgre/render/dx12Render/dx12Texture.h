#pragma once

#include "OgreTexture.h"
#include "d3dutil.h"
#include "engine_struct.h"
class DX12Commands;
class Dx12TextureHandleManager;
class Dx12HardwarePixelBuffer;
class Dx12Texture :public OgreTexture
{
public:
    Dx12Texture(
        const std::string& name, 
        Ogre::TextureProperty* texProperty, 
        DX12Commands* commands,
        DxDescriptorID descriptorId);

    Dx12Texture(
        const std::string& name,
        Ogre::TextureProperty* texProperty,
        DX12Commands* commands,
        ID3D12Resource* resource,
        DxDescriptorID descriptorId);

    ~Dx12Texture();

    DxDescriptorID getDescriptorId()
    {
        return mDescriptors;
    }

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
    void updateTextureData();
    void generateMipmaps();
private:
    virtual void createInternalResourcesImpl(void);
    virtual void freeInternalResourcesImpl(void);
    void _create2DTex();
    void _createSurfaceList(void);
    virtual void postLoad();
    void buildDescriptorHeaps();
private:

    Microsoft::WRL::ComPtr<ID3D12Resource> mTex;
    Microsoft::WRL::ComPtr<ID3D12Resource> mTexUpload;
    DxDescriptorID mDescriptors;

    bool mCreate = false;
    DXGI_FORMAT mD3DFormat;
    DXGI_SAMPLE_DESC mFSAAType;

    int32_t mTexStartIndex = -1;

    DX12Commands* mCommands;
};