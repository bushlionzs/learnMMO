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
        Dx12TextureHandleManager* mgr);

    Dx12Texture(
        const std::string& name,
        Ogre::TextureProperty* texProperty,
        DX12Commands* commands,
        ID3D12Resource* resource,
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);

    ~Dx12Texture();

    
    void buildDescriptorHeaps(int32_t handleIndex);
    D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle()
    {
        return mCpuHandle;
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
private:

    Microsoft::WRL::ComPtr<ID3D12Resource> mTex;
    Microsoft::WRL::ComPtr<ID3D12Resource> mTexUpload;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE mCpuHandle;

    bool mCreate = false;
    DXGI_FORMAT mD3DFormat;
    DXGI_SAMPLE_DESC mFSAAType;

    int32_t mTexStartIndex = -1;

    DX12Commands* mCommands;
    Dx12TextureHandleManager* mDx12TextureHandleManager;

};