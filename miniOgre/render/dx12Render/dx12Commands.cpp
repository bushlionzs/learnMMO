#include <OgreHeader.h>
#include "dx12Commands.h"
#include "OgreRoot.h"

DX12Commands::DX12Commands(ID3D12Device* device)
{
    mDevice = device;

	EngineConfig& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();

	mNumFrame = ogreConfig.swapBufferCount;

	mCommandList.resize(mNumFrame);

	for (auto i = 0; i < mNumFrame; i++)
	{
		FrameCommand& current = mCommandList[i];
		

		ThrowIfFailed(mDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&current.commandAllocator)));

		ThrowIfFailed(mDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			current.commandAllocator,
			nullptr,
			IID_PPV_ARGS(&current.commandList)));

		current.commandList->Close();
	}
	

	ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mFence)));

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(mDevice->CreateCommandQueue(&queueDesc,
		IID_PPV_ARGS(&mCommandQueue)));
}

DX12Commands::~DX12Commands()
{

}

ID3D12GraphicsCommandList* DX12Commands::get()
{
	if (mCurrentCommandBufferIndex >= 0)
	{
		return mCommandList[mCurrentCommandBufferIndex].commandList;
	}

	int8_t nextIndex = (mLastCommandBufferIndex + 1) % mNumFrame;

	waitFence(nextIndex);

	mCurrentCommandBufferIndex = nextIndex;

	auto cmdList = mCommandList[nextIndex].commandList;
	auto cmdAllocator = mCommandList[nextIndex].commandAllocator;
	cmdAllocator->Reset();
	ThrowIfFailed(cmdList->Reset(cmdAllocator, nullptr));

    return cmdList;
}

ID3D12CommandQueue* DX12Commands::getCommandQueue()
{
	return mCommandQueue;
}

bool DX12Commands::flush(bool waitCmd)
{
	if (mCurrentCommandBufferIndex < 0) {
		return false;
	}

	auto* commandList = mCommandList[mCurrentCommandBufferIndex].commandList;
	commandList->Close();
	mCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**) &commandList);

	mLastCommandBufferIndex = mCurrentCommandBufferIndex;

	signalFence();

	if (waitCmd)
	{
		waitFence(mCurrentCommandBufferIndex);
	}

	mCurrentCommandBufferIndex = -1;

	return true;
}

void DX12Commands::signalFence()
{
	if (mCurrentCommandBufferIndex < 0)
	{
		assert(false);
		return;
	}

	auto fence = ++mCurrentFence;
	mCommandList[mCurrentCommandBufferIndex].fence = fence;

	mCommandQueue->Signal(mFence, fence);
}

void DX12Commands::waitFence(uint32_t index)
{
	auto completedValue = mFence->GetCompletedValue();
	auto frameFence = mCommandList[index].fence;
	if (frameFence > 0 && completedValue < frameFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(frameFence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}