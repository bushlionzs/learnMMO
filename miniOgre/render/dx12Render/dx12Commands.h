#pragma once
#include "dx12Common.h"
class DX12Commands
{
    struct FrameCommand
    {
        ID3D12GraphicsCommandList* commandList;
        ID3D12CommandAllocator* commandAllocator;
        uint32_t fence = 0;
    };
public:
    DX12Commands(ID3D12Device* device);
    ~DX12Commands();
    ID3D12GraphicsCommandList* get();
    bool flush(bool waitCmd);
private:
    void signalFence();
    void waitFence(uint32_t index);
private:
    ID3D12Device* mDevice;
    ID3D12Fence* mFence;
    ID3D12CommandQueue* mCommandQueue;
    std::vector<FrameCommand> mCommandList;
    uint64_t mCurrentFence = 0;
    int8_t mCurrentCommandBufferIndex = -1;

    int8_t mNumFrame;
    int8_t mLastCommandBufferIndex = -1;
};