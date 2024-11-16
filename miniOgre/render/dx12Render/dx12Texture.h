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
    ~Dx12Texture();

    
    void buildDescriptorHeaps(int32_t handleIndex);
    CD3DX12_GPU_DESCRIPTOR_HANDLE getTextureHandle();

    ID3D12Resource* getTextureResource()
    {
        return mTex.Get();
    }

    DXGI_FORMAT getDxFormat()
    {
        return mD3DFormat;
    }

    void updateTextureData();
    void generateMipmaps();
private:
    virtual void createInternalResourcesImpl(void);
    virtual void freeInternalResourcesImpl(void);
    void _create2DTex();
    void _createSurfaceList(void);
    virtual void postLoad();
    void createRenderTarget();
private:

    Microsoft::WRL::ComPtr<ID3D12Resource> mTex;
    Microsoft::WRL::ComPtr<ID3D12Resource> mTexUpload;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mDescriptorHandle;
    bool mCreate = false;
    DXGI_FORMAT mD3DFormat;
    DXGI_SAMPLE_DESC mFSAAType;

    int32_t mTexStartIndex = -1;

    DX12Commands* mDX12Commands;
    Dx12TextureHandleManager* mDx12TextureHandleManager;

};