#pragma once
#include "dx12Common.h"
#include "dx12Commands.h"


class Dx12RenderTarget;
class Dx12Texture;

class DX12SwapChain
{
    struct DX12RenderTargetInfo
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> swapChainBuffer;
        ComPtr<ID3D12DescriptorHeap> rtvHeap;
    };

public:
    DX12SwapChain(
        DX12Commands* commands, 
        HWND hWnd,
        bool srgb);
    void present();
    void acquire(bool& reized);

    uint32 getWidth()
    {
        return mWidth;
    }
    uint32_t getHeight()
    {
        return mHeight;
    }

    Dx12Texture* getDepthTexture();
    Dx12Texture* getCurrentColor();

    DXGI_FORMAT getColorFormat()
    {
        return mColorFormat;
    }

    DXGI_FORMAT getDepthFormat()
    {
        return mDepthFormat;
    }
private:
    void createSwapChain2(bool srgb);
private:
    ComPtr<IDXGISwapChain> mSwapChain;
    IDXGISwapChain3* mSwapChain3;
    Dx12Texture* mDepth;
    std::vector<Dx12Texture*> mColors;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

    uint32_t mCurrentFrameIndex;

    DX12Commands* mCommands;
    HWND mHwnd;

    uint32_t mWidth;
    uint32_t mHeight;

    DXGI_FORMAT mColorFormat;
    DXGI_FORMAT mDepthFormat;
};