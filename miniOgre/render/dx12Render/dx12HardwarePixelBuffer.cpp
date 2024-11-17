#include "OgreHeader.h"
#include "dx12HardwarePixelBuffer.h"
#include "OgreTexture.h"
#include "OgreStringConverter.h"
#include "dx12Texture.h"
#include "dx12Helper.h"
#include "dx12RenderSystem.h"

Dx12HardwarePixelBuffer::Dx12HardwarePixelBuffer(
	Dx12Texture* parentTexture,
	UINT mipLevel,
	size_t width,
	size_t height,
	size_t depth,
	UINT face,
	PixelFormat format,
	HardwareBuffer::Usage usage)
	:HardwarePixelBuffer(width, height, depth, format, usage, false, false),
	mParentTexture(parentTexture),
	mFace(face),
	mMipLevel(mipLevel)
{
	mUsage = usage;
	
	uint32_t byteSize = PixelUtil::getMemorySize(width, height, depth, format);

	mBuffer.resize(byteSize);

}

Dx12HardwarePixelBuffer::~Dx12HardwarePixelBuffer()
{

}

void Dx12HardwarePixelBuffer::blitFromMemory(const PixelBox& src, const Box& dst)
{
	Vector3i srcSize(src.getWidth(), src.getHeight(), src.getDepth());
	Vector3i dstSize(dst.getWidth(), dst.getHeight(), dst.getDepth());
	if (srcSize != dstSize)
	{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
			"D3D12 device cannot copy a subresource - source and dest size are not the same and they have to be the same in DX12.",
			"D3D12HardwarePixelBuffer::blitFromMemory");
	}


	PixelBox dstBox = PixelBox(src.getWidth(), src.getHeight(),
		src.getDepth(), mFormat, mBuffer.data());
	PixelUtil::bulkPixelConversion(src, dstBox);

}

void Dx12HardwarePixelBuffer::blitToMemory(const Box& srcBox, const PixelBox& dst)
{
	assert(false);
}

void* Dx12HardwarePixelBuffer::lockimpl(size_t offset, size_t length, LockOptions options)
{
	return mBuffer.data();
}

void Dx12HardwarePixelBuffer::unlock()
{

}

void Dx12HardwarePixelBuffer::uploadData()
{
	mParentTexture->updateTextureData();
}