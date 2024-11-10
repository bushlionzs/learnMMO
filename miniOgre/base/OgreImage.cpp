// Image.cpp: implementation of the CImage class.
//
//////////////////////////////////////////////////////////////////////

#include "OgreHeader.h"
#include "OgreImage.h"
#include "myutils.h"
#include "stb_image.h"
#include "dds_load.h"
#include "OgreResourceManager.h"
#include "OgreMemoryStream.h"
#include "OgreBlp.h"
#include <gli/gli.hpp>
#include <platform_file.h>

namespace Ogre {

    CImage::CImage()
    {
        mImageData = nullptr;
        mPixelSize = 0;
    }

    CImage::~CImage()
    {
        freeMemory();
    }

    backend::ImageType CImage::getImageType(const std::string& name)
    {
        std::string suffix = getSuffix(name);
        if (suffix == ".dds")
        {
            return backend::ImageType::ImageType_DDS;
        }
        else if (suffix == ".ktx")
        {
            return backend::ImageType::ImageType_KTX;
        }
        else if (suffix == ".blp")
        {
            return backend::ImageType::ImageType_BLP;
        }
        else if (suffix == ".png")
        {
            return backend::ImageType::ImageType_PNG;
        }
        else if (suffix == ".jpg")
        {
            return backend::ImageType::ImageType_JPG;
        }
        else
        {
            return backend::ImageType::ImageType_UnSupported;
        }
    }

    bool CImage::loadImageInfo(
        const uint8_t* data, 
        uint32_t byteCount, 
        ImageInfo& imageInfo,
        backend::ImageType type)
    {
        switch (type)
        {
        case backend::ImageType::ImageType_PNG:
        case backend::ImageType::ImageType_JPG:
        {
            int width;
            int height;
            int numComponents;
            stbi_info_from_memory(data, byteCount, &width, &height, &numComponents);
            imageInfo.width = width;
            imageInfo.height = height;
            if (numComponents == 3)
            {
                imageInfo.format = Ogre::PF_BYTE_RGB;
            }
            else if (numComponents == 4)
            {
                imageInfo.format = Ogre::PF_BYTE_RGBA;
            }
            else if (numComponents == 2)
            {
                imageInfo.format = Ogre::PF_BYTE_LA;
            }
            else if (numComponents == 1)
            {
                imageInfo.format = Ogre::PF_L8;
            }
            
            return true;
        }
            
        case backend::ImageType::ImageType_DDS:
            return DDSImage::load_simple_info((const char*)data, byteCount, imageInfo);
        default:
            assert(false);
            return false;
            break;
        }
        
    }

    Ogre::PixelFormat translateKtxFormat(gli::texture::format_type format)
    {
        switch (format)
        {
        case gli::FORMAT_RGBA16_SFLOAT_PACK16:
            return Ogre::PF_FLOAT16_RGBA;
        case gli::FORMAT_RGBA8_UNORM_PACK8:
            return Ogre::PF_A8B8G8R8;
        default:
            assert(false);
        }
        return Ogre::PF_FLOAT16_RGBA;
    }
    bool CImage::loadImageInfo(
        const std::string& name,
        ImageInfo& imageInfo,
        bool cube)
    {
        ResourceInfo* res = nullptr;
        backend::ImageType type = CImage::getImageType(name);
        if (cube)
        {
            if (type == backend::ImageType::ImageType_DDS ||
                type == backend::ImageType::ImageType_KTX)
            {
                res = ResourceManager::getSingleton().getResource(name);
            }
            else
            {
                std::string suffix = getSuffix(name);
                std::string basename = removeSuffix(name);
                std::string current = basename + "_rt" + suffix;
                res = ResourceManager::getSingleton().getResource(current);
            }
            
        }
        else
        {
            res = ResourceManager::getSingleton().getResource(name);
        }
        

        std::string content;
        get_file_content(res->_fullname.c_str(), content, 2048);

        const char* data = content.c_str();
        uint32_t byteCount = content.size();
        
        return CImage::loadImageInfo((const uint8_t*)data, byteCount, imageInfo, type);
    }

    void CImage::freeImageData(void* data)
    {
        stbi_image_free(data);
    }

    bool CImage::loadImage(const std::string& name, bool cube)
    {
        auto type = getImageType(name);
        uint32_t nrComponents = 0;

        unsigned char* data = nullptr;
        if (type == backend::ImageType::ImageType_DDS)
        {
            if (name == "copper_normal.dds")
            {
                int kk = 0;
            }
            std::shared_ptr<DataStream> stream
                = ResourceManager::getSingleton().openResource(name);
            DDSImage ddsload;
            if (ddsload.load(stream.get()))
            {
                data = ddsload.data();
                nrComponents = 4;

                ImageInfo* imageData = ddsload.getImageInfo();
                mImageInfo = *imageData;
            }
        }
        else if (type == backend::ImageType::ImageType_KTX)
        {
            auto resInfo = ResourceManager::getSingleton().getResource(name);
            gli::texture tmp = gli::load(resInfo->_fullname.c_str());

            mImageInfo.width = static_cast<uint32_t>(tmp.extent().x);
            mImageInfo.height = static_cast<uint32_t>(tmp.extent().y);
            mImageInfo.num_mipmaps = static_cast<uint32_t>(tmp.levels()) - 1;
            mImageInfo.face = tmp.faces();
            mImageInfo.size = tmp.size();
            auto ktxFormat = tmp.format();
            mImageInfo.format = translateKtxFormat(ktxFormat);
            data = new unsigned char[mImageInfo.size];
            memcpy(data, tmp.data(), mImageInfo.size);
            int kk = 0;
        }
        else if (type == backend::ImageType::ImageType_BLP)
        {
            std::shared_ptr<DataStream> stream
                = ResourceManager::getSingleton().openResource(name);
            OgreBlpImage blpImage;
            blpImage.load(stream);
            data = blpImage.data();
            ImageInfo* imageData = blpImage.getImageInfo();

            mImageInfo = *imageData;
        }
        else
        {
            mImageInfo.face = 1;
            PixelFormat format[4] = { Ogre::PF_L8, Ogre::PF_BYTE_LA, Ogre::PF_BYTE_RGB, Ogre::PF_BYTE_RGBA };
            {
                std::shared_ptr<DataStream> stream
                    = ResourceManager::getSingleton().openResource(name);

                const stbi_uc* stream_data = (const stbi_uc*)stream->getStreamData();
                uint32_t size = stream->getStreamLength();
                data = stbi_load_from_memory(stream_data, size,
                    (int*)&mImageInfo.width, (int*)&mImageInfo.height, (int*)&nrComponents, 0);
                mImageInfo.format = format[nrComponents - 1];

                
                if (nrComponents == 2)
                {
                   /* std::unordered_set<uint32_t> aa;
                    for (uint32_t i = 0; i < mImageInfo.width * mImageInfo.height; i++)
                    {
                        auto color = data[i * 2];
                        auto alpha = data[i * 2 + 1];
                        if (alpha != 0)
                        {
                            aa.insert(alpha);
                        }
                    }*/
                    int kk = 0;
                }
            }
            

            mPixelSize = nrComponents;
            
            
        }


       // mNumMipmaps = 0;
        mImageData = data;

        mImageDataSize = calculateSize(mImageInfo);

        if (mImageInfo.format == PF_L8)
        {
            int kk = 0;
        }
        

        return true;
    }

    bool CImage::loadImage(DataStreamPtr& stream)
    {
        const uint8_t* data = (const uint8_t*)stream->getStreamData();
        uint32_t size = stream->getStreamLength();
        return loadImage(data, size, backend::ImageType::ImageType_PNG);
    }

    bool CImage::loadImage(const uint8_t* data, uint32_t byteCount, backend::ImageType type)
    {
        switch (type)
        {
        case backend::ImageType::ImageType_PNG:
        case backend::ImageType::ImageType_JPG:
        {
            mImageInfo.face = 1;
            uint32_t nrComponents = 0;
            auto imagedata = stbi_load_from_memory(data, byteCount,
                (int*)&mImageInfo.width, (int*)&mImageInfo.height, (int*)&nrComponents, 0);

            if (nullptr == imagedata)
            {
                return false;
            }

            mPixelSize = nrComponents;
            if (nrComponents == 3)
            {
                mImageInfo.format = Ogre::PF_BYTE_RGB;
            }
            else if (nrComponents == 4)
            {
                mImageInfo.format = Ogre::PF_BYTE_RGBA;
            }
            else if (nrComponents == 2)
            {
                mImageInfo.format = Ogre::PF_BYTE_LA;
            }
            else if (nrComponents == 1)
            {
                mImageInfo.format = Ogre::PF_L8;
            }

            // mNumMipmaps = 0;
            mImageData = (unsigned char*)imagedata;

            mImageDataSize = calculateSize(mImageInfo);

            return true;
        }
        case backend::ImageType::ImageType_DDS:
        {
            DDSImage ddsload;
            MemoryDataStream sm((const char*)data, byteCount);
            if (ddsload.load(&sm))
            {
                data = ddsload.data();

                ImageInfo* imageData = ddsload.getImageInfo();

                mImageInfo = *imageData;

                mImageData = (unsigned char*)data;

                mImageDataSize = calculateSize(mImageInfo);
            }
            return true;
        }
        default:
            assert(false);
            return false;
        }
        
    }

    bool CImage::loadRawData(DataStreamPtr& stream, ushort uWidth, ushort uHeight, PixelFormat format)
    {
        const char* data = stream->getStreamData();
        uint32_t size = stream->getStreamLength();

        mImageData = (unsigned char*)malloc(size);
        memcpy(mImageData, data, size);
        mImageInfo.format = format;

        mImageInfo.width = uWidth;
        mImageInfo.height = uHeight;
        mImageInfo.depth = 1;
        mImageInfo.num_mipmaps = 1;

        return true;
    }

    int CImage::getWidth() const
    {
        return mImageInfo.width;
    }

    int CImage::getHeight() const
    {
        return mImageInfo.height;
    }

    Ogre::PixelFormat CImage::getFormat() const
    {
        return mImageInfo.format;
    }

    int CImage::getDepth() const
    {
        return mImageInfo.depth;
    }

    int CImage::getNumFaces() const
    {
        return mImageInfo.face;
    }

    int CImage::getNumMipmaps() const
    {
        return mImageInfo.num_mipmaps;
    }

    unsigned char* CImage::getImageData()
    {
        return mImageData;
    }

    uint32_t CImage::getSize()
    {
        return mImageDataSize;
    }

    Ogre::PixelBox CImage::getPixelBox(uint32 face, uint32 mipmap) const
    {
        uint8* offset = mImageData;
        uint32 width = mImageInfo.width, height = mImageInfo.height, depth = mImageInfo.depth;
        uint32 numMips = mImageInfo.num_mipmaps;

        size_t fullFaceSize = 0;
        size_t finalFaceSize = 0;
        uint32 finalWidth = 0, finalHeight = 0, finalDepth = 0;
        for (uint32 mip = 0; mip <= numMips; ++mip)
        {
            if (mip == mipmap)
            {
                finalFaceSize = fullFaceSize;
                finalWidth = width;
                finalHeight = height;
                finalDepth = depth;
            }
            fullFaceSize += PixelUtil::getMemorySize(width, height, depth, getFormat());

            /// Half size in each dimension
            if (width != 1) width /= 2;
            if (height != 1) height /= 2;
            if (depth != 1) depth /= 2;
        }
        // Advance pointer by number of full faces, plus mip offset into
        offset += face * fullFaceSize;
        offset += finalFaceSize;
        // Return subface as pixelbox
        PixelBox src(finalWidth, finalHeight, finalDepth, getFormat(), offset);
        return src;
    }


    const uchar* CImage::getData(uint32 x, uint32 y, uint32 z) const
    {
        assert(mImageData);
        assert(x < mImageInfo.width&& y < mImageInfo.height&& z < mImageInfo.depth);
        return mImageData + mPixelSize * (z * mImageInfo.width * mImageInfo.height + mImageInfo.width * y + x);
    }

    void CImage::convertRawData(void* from, void* to, size_t _size, int _format)
    {

    }

    void CImage::CImage::freeMemory()
    {
    }

    Ogre::ColourValue CImage::getColourAt(uint32 x, uint32 y, uint32 z) const
    {
        Ogre::ColourValue rval;
        Ogre::PixelUtil::unpackColour(&rval, mImageInfo.format, getData(x, y, z));
        return rval;
    }

    CImage& CImage::loadDynamicImage(
        uchar* data, 
        uint32 width, 
        uint32 height, 
        uint32 depth, 
        PixelFormat format,
        bool autoDelete, 
        uint32 numFaces, 
        uint32 numMipMaps)
    {
        freeMemory();
        // Set image metadata
        mImageInfo.width = width;
        mImageInfo.height = height;
        mImageInfo.depth = depth;
        mImageInfo.format = format;
        mPixelSize = static_cast<uchar>(PixelUtil::getNumElemBytes(format));
        mImageInfo.num_mipmaps = numMipMaps;
        mFlags = 0;
        // Set flags
        if (PixelUtil::isCompressed(format))
            mFlags |= IF_COMPRESSED;
        if (mImageInfo.depth != 1)
            mFlags |= IF_3D_TEXTURE;
        if (mImageInfo.face == 6)
            mFlags |= IF_CUBEMAP;
        OgreAssert(numFaces == 6 || numFaces == 1, "Invalid number of faces");

        mImageDataSize = calculateSize(mImageInfo);
        mImageData = data;
        mAutoDelete = autoDelete;

        return *this;
    }

    //-----------------------------------------------------------------------------
    size_t CImage::calculateSize(
        Ogre::ImageInfo& info)
    {
        size_t size = 0;
        auto width = info.width;
        auto height = info.height;
        auto depth = info.depth;
        for (uint32 mip = 0; mip <= info.num_mipmaps; ++mip)
        {
            size += PixelUtil::getMemorySize(width, height, info.depth, info.format) * info.face;
            if (width != 1) width /= 2;
            if (height != 1) height /= 2;
            if (depth != 1) depth /= 2;
        }
        return size;
    }

    size_t CImage::calculateFaceSize(Ogre::ImageInfo& info)
    {
        return PixelUtil::getMemorySize(info.width, info.height, info.depth, info.format);
    }

    void CImage::save(const String& filename)
    {

    }
}