#pragma once
#include "OgreHardwareBuffer.h"
#include "OgrePixelFormat.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreImage.h"
#include <utils/JobSystem.h>

namespace Ogre {
    enum TextureTypePbr
    {
        TextureTypePbr_general = 0,
        TextureTypePbr_Albedo = 1,
        TextureTypePbr_AmbientOcclusion = 2,
        TextureTypePbr_NormalMap = 3,
        TextureTypePbr_Emissive = 4,
        TextureTypePbr_MetalRoughness = 5,
        TextureTypePbr_Roughness = 6,
        TextureTypePbr_BRDF_LUT = 7,
        TextureTypePbr_IBL_Diffuse = 8,
        TextureTypePbr_IBL_Specular = 9
    };
    

    enum TextureType : uint8_t
    {
        /// 1D texture, used in combination with 1D texture coordinates
        TEX_TYPE_1D = 1,
        /// 2D texture, used in combination with 2D texture coordinates (default)
        TEX_TYPE_2D = 2,
        /// 3D volume texture, used in combination with 3D texture coordinates
        TEX_TYPE_3D = 3,
        /// cube map (six two dimensional textures, one for each cube face), used in combination with 3D
        /// texture coordinates
        TEX_TYPE_CUBE_MAP = 4,
        /// 2D texture array
        TEX_TYPE_2D_ARRAY = 5,
        /// GLES2 only OES texture type
        TEX_TYPE_EXTERNAL_OES = 6
    };

    enum TextureUsage
    {
        NONE = 0x0000,
        COLOR_ATTACHMENT = 0x0001,            //!< Texture can be used as a color attachment
        DEPTH_ATTACHMENT = 0x0002,            //!< Texture can be used as a depth attachment
        STENCIL_ATTACHMENT = 0x0004,          //!< Texture can be used as a stencil attachment
        UPLOADABLE = 0x0008,                  //!< Data can be uploaded into this texture (default)
        SAMPLEABLE = 0x0010,                  //!< Texture can be sampled (default)
        WRITEABLE  = 0x0020,
        TU_DEFAULT = UPLOADABLE | SAMPLEABLE,
    };

    class TextureProperty
    {
    public:
        TextureType _texType;
        TextureTypePbr _pbrType;
        uint32_t _width;
        uint32_t _height;
        uint32_t _depth;
        uint32_t _numMipmaps;
        Ogre::PixelFormat _tex_format;
        bool _need_mipmap;
        uint32_t _maxMipLevel;
        float _gamma;
        uint32_t _fsaa;
        uint32_t _tex_usage; //TextureUsage
        ColourValue _backgroudColor;
        TextureAddressingMode  _tex_addr_mod;
        filament::backend::SamplerParams _samplerParams;
        TextureProperty();
        bool isRenderTarget();
        bool haveImageFile();
        void gltfSampler();
    };


    class OgreTexture
    {
    public:
        OgreTexture(const String& name, TextureProperty* texProperty);
        ~OgreTexture();
        const String& getName()
        {
            return mName;
        }

        void setWidth(int32_t width)
        {
            mTextureProperty._width = width;
        }

        int32_t getWidth()
        {
            return mTextureProperty._width;
        }

        void setHeight(int32_t height)
        {
            mTextureProperty._height = height;
        }

        int32_t getHeight()
        {
            return mTextureProperty._height;
        }

        int32_t getDepth()
        {
            return mTextureProperty._depth;
        }

        uint32_t getSize()
        {
            return PixelUtil::getMemorySize(mTextureProperty._width, mTextureProperty._height,
                mTextureProperty._depth, mTextureProperty._tex_format);
        }

        Ogre::PixelFormat getFormat()
        {
            return mTextureProperty._tex_format;
        }

        uint32_t getNumMipmaps()
        {
            return mTextureProperty._numMipmaps + 1;
        }

        uint32_t getFace()
        {
            return mFace;
        }

        bool isCubeTexture()
        {
            return mTextureProperty._texType == TEX_TYPE_CUBE_MAP;
        }
        bool isLoaded()
        {
            return mLoad;
        }

        TextureProperty* getTextureProperty()
        {
            return &mTextureProperty;
        }

        TextureType getTextureType();

        void createInternalResources();
        void freeInternalResources(void);
        virtual void createInternalResourcesImpl(void) = 0;
        virtual void freeInternalResourcesImpl(void) = 0;
        virtual void updateTexture(const std::vector<const CImage*>& images) = 0;
        virtual void preLoad();
        bool load(utils::JobSystem::Job* job);
        void loadImpl();
        virtual void postLoad();
        uint32 getMaxMipmaps() const;
        
        virtual void unload();
        virtual void uploadData() {}
        virtual void blitFromMemory(
            const PixelBox& src, const Box& dstBox, uint32_t face = 0, uint32_t mipmap = 0) {}
        void loadImage(const CImage& img);
        void loadRawData(DataStreamPtr& stream, ushort uWidth, ushort uHeight,
            PixelFormat format);

        uint32_t getDataOffset(uint32_t face, uint32_t mip);
    private:
        
        void _loadImages(const std::vector<const CImage*>& images);
    protected:
        String mName;
        TextureProperty mTextureProperty;
        uint32_t mFace = 1;
        PixelFormat mFormat;
        int mUsage;

        PixelFormat mSrcFormat;
        uint32 mSrcWidth, mSrcHeight, mSrcDepth;

        std::vector<String> mLayerNames;

        bool mLoad = false;

        bool mInternalResourcesCreated = false;

        std::vector<uint32_t> mOffsetList;
    };
}