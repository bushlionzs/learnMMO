#include "OgreHeader.h"
#include "dds_load.h"
#include "OgrePixelFormat.h"
#include "OgreColourValue.h"
#include "OgreDataStream.h"
#include "OgreBitwise.h"
using namespace Ogre;


#define FOURCC(c0, c1, c2, c3) (c0 | (c1 << 8) | (c2 << 16) | (c3 << 24))
static void flipEndian(void* pData, size_t size)
{
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    Bitwise::bswapBuffer(pData, size);
#endif
}

static void flipEndian(void* pData, size_t size, size_t count)
{
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    Bitwise::bswapChunks(pData, size, count);
#endif
}

struct DDSPixelFormat
{
    uint32_t size;
    uint32_t flags;
    uint32_t fourCC;
    uint32_t rgbBits;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t alphaMask;
};

// Nested structure
struct DDSCaps
{
    uint32_t caps1;
    uint32_t caps2;
    uint32_t caps3;
    uint32_t caps4;
};

struct DDSHeader
{
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t sizeOrPitch;
    uint32_t depth;
    uint32_t mipMapCount;
    uint32_t reserved1[11];
    DDSPixelFormat pixelFormat;
    DDSCaps caps;
    uint32_t reserved2;
};

// Extended header
struct DDSExtendedHeader
{
    uint32_t dxgiFormat;
    uint32_t resourceDimension;
    uint32_t miscFlag; // see D3D11_RESOURCE_MISC_FLAG
    uint32_t arraySize;
    uint32_t reserved;
};


// An 8-byte DXT colour block, represents a 4x4 texel area. Used by all DXT formats
struct DXTColourBlock
{
    // 2 colour ranges
    uint16_t colour_0;
    uint16_t colour_1;
    // 16 2-bit indexes, each byte here is one row
    uint8_t indexRow[4];
};
// An 8-byte DXT explicit alpha block, represents a 4x4 texel area. Used by DXT2/3
struct DXTExplicitAlphaBlock
{
    // 16 4-bit values, each 16-bit value is one row
    uint16_t alphaRow[4];
};
// An 8-byte DXT interpolated alpha block, represents a 4x4 texel area. Used by DXT4/5
struct DXTInterpolatedAlphaBlock
{
    // 2 alpha ranges
    uint8_t alpha_0;
    uint8_t alpha_1;
    // 16 3-bit indexes. Unfortunately 3 bits doesn't map too well to row bytes
    // so just stored raw
    uint8_t indexes[6];
};


const uint32_t DDS_MAGIC = FOURCC('D', 'D', 'S', ' ');
const uint32_t DDS_PIXELFORMAT_SIZE = 8 * sizeof(uint32_t);
const uint32_t DDS_CAPS_SIZE = 4 * sizeof(uint32_t);
const uint32_t DDS_HEADER_SIZE = 19 * sizeof(uint32_t) + DDS_PIXELFORMAT_SIZE + DDS_CAPS_SIZE;

const uint32_t DDSD_CAPS = 0x00000001;
const uint32_t DDSD_HEIGHT = 0x00000002;
const uint32_t DDSD_WIDTH = 0x00000004;
const uint32_t DDSD_PIXELFORMAT = 0x00001000;
const uint32_t DDSD_DEPTH = 0x00800000;
const uint32_t DDPF_ALPHAPIXELS = 0x00000001;
const uint32_t DDPF_FOURCC = 0x00000004;
const uint32_t DDPF_RGB = 0x00000040;
const uint32_t DDSCAPS_COMPLEX = 0x00000008;
const uint32_t DDSCAPS_TEXTURE = 0x00001000;
const uint32_t DDSCAPS_MIPMAP = 0x00400000;
const uint32_t DDSCAPS2_CUBEMAP = 0x00000200;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000;
const uint32_t DDSCAPS2_VOLUME = 0x00200000;

// Currently unused
//    const uint32 DDSD_PITCH = 0x00000008;
//    const uint32 DDSD_MIPMAPCOUNT = 0x00020000;
//    const uint32 DDSD_LINEARSIZE = 0x00080000;

    // Special FourCC codes
const uint32_t D3DFMT_R16F = 111;
const uint32_t D3DFMT_G16R16F = 112;
const uint32_t D3DFMT_A16B16G16R16F = 113;
const uint32_t D3DFMT_R32F = 114;
const uint32_t D3DFMT_G32R32F = 115;
const uint32_t D3DFMT_A32B32G32R32F = 116;




enum ImageFlags
{
    IF_COMPRESSED = 0x00000001,
    IF_CUBEMAP = 0x00000002,
    IF_3D_TEXTURE = 0x00000004
};


PixelFormat convertDXToOgreFormat(uint32_t dxfmt)
{
    switch (dxfmt) {
    case 2: // DXGI_FORMAT_R32G32B32A32_FLOAT
        return PF_FLOAT32_RGBA;
    case 3: // DXGI_FORMAT_R32G32B32A32_UINT
        return PF_R32G32B32A32_UINT;
    case 4: //DXGI_FORMAT_R32G32B32A32_SINT
        return PF_R32G32B32A32_SINT;
    case 6: // DXGI_FORMAT_R32G32B32_FLOAT
        return PF_FLOAT32_RGB;
    case 7: // DXGI_FORMAT_R32G32B32_UINT
        return PF_R32G32B32_UINT;
    case 8: // DXGI_FORMAT_R32G32B32_SINT
        return PF_R32G32B32_SINT;
    case 10: // DXGI_FORMAT_R16G16B16A16_FLOAT
        return PF_FLOAT16_RGBA;
    case 12: // DXGI_FORMAT_R16G16B16A16_UINT
        return PF_R16G16B16A16_UINT;
    case 13: // DXGI_FORMAT_R16G16B16A16_SNORM
        return PF_R16G16B16A16_SNORM;
    case 14: // DXGI_FORMAT_R16G16B16A16_SINT
        return PF_R16G16B16A16_SINT;
    case 16: // DXGI_FORMAT_R32G32_FLOAT
        return PF_FLOAT32_GR;
    case 17: // DXGI_FORMAT_R32G32_UINT
        return PF_R32G32_UINT;
    case 18: // DXGI_FORMAT_R32G32_SINT
        return PF_R32G32_SINT;
    case 24: // DXGI_FORMAT_R10G10B10A2_UNORM
    case 25: // DXGI_FORMAT_R10G10B10A2_UINT
        return PF_A2B10G10R10;
    case 26: // DXGI_FORMAT_R11G11B10_FLOAT
        return PF_R11G11B10_FLOAT;
    case 28: // DXGI_FORMAT_R8G8B8A8_UNORM
    case 29: // DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
        return PF_A8B8G8R8;
    case 30: // DXGI_FORMAT_R8G8B8A8_UINT
        return PF_R8G8B8A8_UINT;
    case 31: // DXGI_FORMAT_R8G8B8A8_SNORM
        return PF_R8G8B8A8_SNORM;
    case 32: // DXGI_FORMAT_R8G8B8A8_SINT
        return PF_R8G8B8A8_SINT;
    case 34: // DXGI_FORMAT_R16G16_FLOAT
        return PF_FLOAT16_GR;
    case 35: // DXGI_FORMAT_R16G16_UNORM
        return PF_SHORT_GR;
    case 36: // DXGI_FORMAT_R16G16_UINT
        return PF_R16G16_UINT;
    case 37: // DXGI_FORMAT_R16G16_SNORM
        return PF_R16G16_SNORM;
    case 38: // DXGI_FORMAT_R16G16_SINT
        return PF_R16G16_SINT;
    case 41: // DXGI_FORMAT_R32_FLOAT
        return PF_FLOAT32_R;
    case 42: // DXGI_FORMAT_R32_UINT
        return PF_R32_UINT;
    case 43: // DXGI_FORMAT_R32_SINT
        return PF_R32_SINT;
    case 49: // DXGI_FORMAT_R8G8_UNORM
    case 50: // DXGI_FORMAT_R8G8_UINT
        return PF_R8G8_UINT;
    case 52: // DXGI_FORMAT_R8G8_SINT
        return PF_R8G8_SINT;
    case 54: // DXGI_FORMAT_R16_FLOAT
        return PF_FLOAT16_R;
    case 56: // DXGI_FORMAT_R16_UNORM
        return PF_L16;
    case 57: // DXGI_FORMAT_R16_UINT
        return PF_R16_UINT;
    case 58: // DXGI_FORMAT_R16_SNORM
        return PF_R16_SNORM;
    case 59: // DXGI_FORMAT_R16_SINT
        return PF_R16_SINT;
    case 61: // DXGI_FORMAT_R8_UNORM
        return PF_R8;
    case 62: // DXGI_FORMAT_R8_UINT
        return PF_R8_UINT;
    case 63: // DXGI_FORMAT_R8_SNORM
        return PF_R8_SNORM;
    case 64: // DXGI_FORMAT_R8_SINT
        return PF_R8_SINT;
    case 65: // DXGI_FORMAT_A8_UNORM
        return PF_A8;
    case 72:  // DXGI_FORMAT_BC1_UNORM_SRGB
        return PFG_BC1_UNORM_SRGB;
    case 75:  // DXGI_FORMAT_BC2_UNORM_SRGB
        return PFG_BC2_UNORM_SRGB;
    case 78:  // DXGI_FORMAT_BC3_UNORM_SRGB
        return PFG_BC3_UNORM_SRGB;
    case 80: // DXGI_FORMAT_BC4_UNORM
        return PF_BC4_UNORM;
    case 81: // DXGI_FORMAT_BC4_SNORM
        return PF_BC4_SNORM;
    case 83: // DXGI_FORMAT_BC5_UNORM
        return PF_BC5_UNORM;
    case 84: // DXGI_FORMAT_BC5_SNORM
        return PF_BC5_SNORM;
    case 85: // DXGI_FORMAT_B5G6R5_UNORM
        return PF_R5G6B5;
    case 86: // DXGI_FORMAT_B5G5R5A1_UNORM
        return PF_A1R5G5B5;
    case 87: // DXGI_FORMAT_B8G8R8A8_UNORM
        return PF_A8R8G8B8;
    case 88: // DXGI_FORMAT_B8G8R8X8_UNORM
        return PF_X8R8G8B8;
    case 95: // DXGI_FORMAT_BC6H_UF16
        return PF_BC6H_UF16;
    case 96: // DXGI_FORMAT_BC6H_SF16
        return PF_BC6H_SF16;
    case 98: // DXGI_FORMAT_BC7_UNORM
    case 99: // DXGI_FORMAT_BC7_UNORM_SRGB
        return PF_BC7_UNORM;
    case 20: // DXGI_FORMAT_D32_FLOAT_S8X24_UINT
    case 22: // DXGI_FORMAT_X32_TYPELESS_G8X24_UINT
    case 40: // DXGI_FORMAT_D32_FLOAT
    case 45: // DXGI_FORMAT_D24_UNORM_S8_UINT
    case 47: // DXGI_FORMAT_X24_TYPELESS_G8_UINT
    case 55: // DXGI_FORMAT_D16_UNORM
    default:
        assert(false);
        return PF_UNKNOWN;
    }
}


PixelFormat convertFourCCFormat(uint32_t fourcc)
{
    // convert dxt pixel format
    switch (fourcc)
    {
    case FOURCC('D', 'X', 'T', '1'):
        return PF_DXT1;
    case FOURCC('D', 'X', 'T', '2'):
        return PF_DXT2;
    case FOURCC('D', 'X', 'T', '3'):
        return PF_DXT3;
    case FOURCC('D', 'X', 'T', '4'):
        return PF_DXT4;
    case FOURCC('D', 'X', 'T', '5'):
        return PF_DXT5;
    case FOURCC('A', 'T', 'I', '1'):
    case FOURCC('B', 'C', '4', 'U'):
        return PF_BC4_UNORM;
    case FOURCC('B', 'C', '4', 'S'):
        return PF_BC4_SNORM;
    case FOURCC('A', 'T', 'I', '2'):
    case FOURCC('B', 'C', '5', 'U'):
        return PF_BC5_UNORM;
    case FOURCC('B', 'C', '5', 'S'):
        return PF_BC5_SNORM;
    case D3DFMT_R16F:
        return PF_FLOAT16_R;
    case D3DFMT_G16R16F:
        return PF_FLOAT16_GR;
    case D3DFMT_A16B16G16R16F:
        return PF_FLOAT16_RGBA;
    case D3DFMT_R32F:
        return PF_FLOAT32_R;
    case D3DFMT_G32R32F:
        return PF_FLOAT32_GR;
    case D3DFMT_A32B32G32R32F:
        return PF_FLOAT32_RGBA;
        // We could support 3Dc here, but only ATI cards support it, not nVidia
    default:
        assert(false);
        return PF_UNKNOWN;
    };

}
//---------------------------------------------------------------------
PixelFormat convertPixelFormat(uint32_t rgbBits, uint32_t rMask,
    uint32_t gMask, uint32_t bMask, uint32_t aMask)
{
    // General search through pixel formats
    for (int i = PF_UNKNOWN + 1; i < PF_COUNT; ++i)
    {
        PixelFormat pf = static_cast<PixelFormat>(i);
        if (PixelUtil::getNumElemBits(pf) == rgbBits)
        {
            uint64_t testMasks[4];
            PixelUtil::getBitMasks(pf, testMasks);
            int testBits[4];
            PixelUtil::getBitDepths(pf, testBits);
            if (testMasks[0] == rMask && testMasks[1] == gMask &&
                testMasks[2] == bMask &&
                // for alpha, deal with 'X8' formats by checking bit counts
                (testMasks[3] == aMask || (aMask == 0 && testBits[3] == 0)))
            {
                return pf;
            }
        }
    }

    assert(false);
    return PF_UNKNOWN;
}
//---------------------------------------------------------------------
void unpackDXTColour(PixelFormat pf, const DXTColourBlock& block,
    ColourValue* pCol)
{
    // Note - we assume all values have already been endian swapped

    // Colour lookup table
    ColourValue derivedColours[4];

    if (pf == PF_DXT1 && block.colour_0 <= block.colour_1)
    {
        // 1-bit alpha
        PixelUtil::unpackColour(&(derivedColours[0]), PF_R5G6B5, &(block.colour_0));
        PixelUtil::unpackColour(&(derivedColours[1]), PF_R5G6B5, &(block.colour_1));
        // one intermediate colour, half way between the other two
        derivedColours[2] = (derivedColours[0] + derivedColours[1]) / 2;
        // transparent colour
        derivedColours[3] = ColourValue::ZERO;
    }
    else
    {
        PixelUtil::unpackColour(&(derivedColours[0]), PF_R5G6B5, &(block.colour_0));
        PixelUtil::unpackColour(&(derivedColours[1]), PF_R5G6B5, &(block.colour_1));
        // first interpolated colour, 1/3 of the way along
        derivedColours[2] = (2 * derivedColours[0] + derivedColours[1]) / 3;
        // second interpolated colour, 2/3 of the way along
        derivedColours[3] = (derivedColours[0] + 2 * derivedColours[1]) / 3;
    }

    // Process 4x4 block of texels
    for (size_t row = 0; row < 4; ++row)
    {
        for (size_t x = 0; x < 4; ++x)
        {
            // LSB come first
            uint8_t colIdx = static_cast<uint8_t>(block.indexRow[row] >> (x * 2) & 0x3);
            if (pf == PF_DXT1)
            {
                // Overwrite entire colour
                pCol[(row * 4) + x] = derivedColours[colIdx];
            }
            else
            {
                // alpha has already been read (alpha precedes colour)
                ColourValue& col = pCol[(row * 4) + x];
                col.r = derivedColours[colIdx].r;
                col.g = derivedColours[colIdx].g;
                col.b = derivedColours[colIdx].b;
            }
        }

    }


}
//---------------------------------------------------------------------
void unpackDXTAlpha(
    const DXTExplicitAlphaBlock& block, ColourValue* pCol)
{
    // Note - we assume all values have already been endian swapped

    // This is an explicit alpha block, 4 bits per pixel, LSB first
    for (size_t row = 0; row < 4; ++row)
    {
        for (size_t x = 0; x < 4; ++x)
        {
            // Shift and mask off to 4 bits
            uint8_t val = static_cast<uint8_t>(block.alphaRow[row] >> (x * 4) & 0xF);
            // Convert to [0,1]
            pCol->a = (float)val / (float)0xF;
            pCol++;

        }

    }

}
//---------------------------------------------------------------------
void unpackDXTAlpha(
    const DXTInterpolatedAlphaBlock& block, ColourValue* pCol)
{
    // Adaptive 3-bit alpha part
    float derivedAlphas[8];

    // Explicit extremes
    derivedAlphas[0] = ((float)block.alpha_0) * (1.0f / 255.0f);
    derivedAlphas[1] = ((float)block.alpha_1) * (1.0f / 255.0f);

    if (block.alpha_0 > block.alpha_1)
    {
        // 6 interpolated alpha values.
        // full range including extremes at [0] and [7]
        // we want to fill in [1] through [6] at weights ranging
        // from 1/7 to 6/7
        for (size_t i = 1; i < 7; ++i)
            derivedAlphas[i + 1] = (derivedAlphas[0] * (7 - i) + derivedAlphas[1] * i) * (1.0f / 7.0f);
    }
    else
    {
        // 4 interpolated alpha values.
        // full range including extremes at [0] and [5]
        // we want to fill in [1] through [4] at weights ranging
        // from 1/5 to 4/5
        for (size_t i = 1; i < 5; ++i)
            derivedAlphas[i + 1] = (derivedAlphas[0] * (5 - i) + derivedAlphas[1] * i) * (1.0f / 5.0f);

        derivedAlphas[6] = 0.0f;
        derivedAlphas[7] = 1.0f;
    }

    // Ok, now we've built the reference values, process the indexes
    uint32_t dw = block.indexes[0] | (block.indexes[1] << 8) | (block.indexes[2] << 16);

    for (size_t i = 0; i < 8; ++i, dw >>= 3)
        pCol[i].a = derivedAlphas[dw & 0x7];

    dw = block.indexes[3] | (block.indexes[4] << 8) | (block.indexes[5] << 16);

    for (size_t i = 8; i < 16; ++i, dw >>= 3)
        pCol[i].a = derivedAlphas[dw & 0x7];
}

size_t calculateSize(uint32 mipmaps, uint32 faces, uint32 width, uint32 height, uint32 depth,
    PixelFormat format)
{
    size_t size = 0;
    for (uint32 mip = 0; mip <= mipmaps; ++mip)
    {
        size += PixelUtil::getMemorySize(width, height, depth, format) * faces;
        if (width != 1) width /= 2;
        if (height != 1) height /= 2;
        if (depth != 1) depth /= 2;
    }
    return size;
}

DDSImage::DDSImage()
{
    mImageData = nullptr;
    mDDSData = nullptr;
}

DDSImage::~DDSImage()
{
    if (mImageData)
    {
        delete mImageData;
    }
}

bool DDSImage::load_simple_info(const char* data, uint32_t size, ImageInfo& info)
{
    MemoryDataStream stream(data, size);

    uint32_t fileType;
    stream.read(&fileType, sizeof(uint32_t));
    flipEndian(&fileType, sizeof(uint32_t));

    if (FOURCC('D', 'D', 'S', ' ') != fileType)
    {
        return false;
    }

    DDSHeader header;
    stream.read(&header, sizeof(DDSHeader));
    flipEndian(&header, 4, sizeof(header) / 4);

    info.width = header.width;
    info.height = header.height;
    // Pixel format
    PixelFormat sourceFormat = PF_UNKNOWN;

    if (header.pixelFormat.flags & DDPF_FOURCC)
    {
        // Check if we have an DX10 style extended header and read it. This is necessary for B6H and B7 formats
        if (header.pixelFormat.fourCC == FOURCC('D', 'X', '1', '0'))
        {
            DDSExtendedHeader extHeader;
            stream.read(&extHeader, sizeof(DDSExtendedHeader));

            // Endian flip if required, all 32-bit values
            flipEndian(&header, sizeof(DDSExtendedHeader));
            sourceFormat = convertDXToOgreFormat(extHeader.dxgiFormat);
        }
        else
        {
            sourceFormat = convertFourCCFormat(header.pixelFormat.fourCC);
        }
    }
    else
    {
        sourceFormat = convertPixelFormat(header.pixelFormat.rgbBits,
            header.pixelFormat.redMask, header.pixelFormat.greenMask,
            header.pixelFormat.blueMask,
            header.pixelFormat.flags & DDPF_ALPHAPIXELS ?
            header.pixelFormat.alphaMask : 0);
    }
    info.format = sourceFormat;
    return true;
}

bool DDSImage::load(DataStream* stream)
{
    uint32_t fileType;
    stream->read(&fileType, sizeof(uint32_t));
    flipEndian(&fileType, sizeof(uint32_t));

    if (FOURCC('D', 'D', 'S', ' ') != fileType)
    {
        return false;
    }

    DDSHeader header;
    stream->read(&header, sizeof(DDSHeader));
    flipEndian(&header, 4, sizeof(header) / 4);

    if (header.size != DDS_HEADER_SIZE)
    {
        return false;
    }
    if (header.pixelFormat.size != DDS_PIXELFORMAT_SIZE)
    {
        return false;
    }


    ImageInfo* imgData = new ImageInfo();

    mImageData = imgData;

    imgData->depth = 1; // (deal with volume later)
    imgData->width = header.width;
    imgData->height = header.height;
    imgData->face = 1; // assume one face until we know otherwise

    if (header.caps.caps1 & DDSCAPS_MIPMAP)
    {
        imgData->num_mipmaps = static_cast<uint8_t>(header.mipMapCount - 1);
    }
    else
    {
        imgData->num_mipmaps = 0;
    }
    imgData->flags = 0;

    bool decompressDXT = false;

    if (header.caps.caps2 & DDSCAPS2_CUBEMAP)
    {
        imgData->flags |= IF_CUBEMAP;
        imgData->face = 6;
    }
    else if (header.caps.caps2 & DDSCAPS2_VOLUME)
    {
        imgData->flags |= IF_3D_TEXTURE;
        imgData->depth = header.depth;
    }
    // Pixel format
    PixelFormat sourceFormat = PF_UNKNOWN;

    if (header.pixelFormat.flags & DDPF_FOURCC)
    {
        // Check if we have an DX10 style extended header and read it. This is necessary for B6H and B7 formats
        if (header.pixelFormat.fourCC == FOURCC('D', 'X', '1', '0'))
        {
            DDSExtendedHeader extHeader;
            stream->read(&extHeader, sizeof(DDSExtendedHeader));

            // Endian flip if required, all 32-bit values
            flipEndian(&header, sizeof(DDSExtendedHeader));
            sourceFormat = convertDXToOgreFormat(extHeader.dxgiFormat);
        }
        else
        {
            sourceFormat = convertFourCCFormat(header.pixelFormat.fourCC);
        }
    }
    else
    {
        sourceFormat = convertPixelFormat(header.pixelFormat.rgbBits,
            header.pixelFormat.redMask, header.pixelFormat.greenMask,
            header.pixelFormat.blueMask,
            header.pixelFormat.flags & DDPF_ALPHAPIXELS ?
            header.pixelFormat.alphaMask : 0);
    }
    imgData->format = sourceFormat;
    if (PixelUtil::isCompressed(sourceFormat))
    {
        if (false)
        {
            // We'll need to decompress
            decompressDXT = true;
            // Convert format
            switch (sourceFormat)
            {
            case PF_DXT1:
                // source can be either 565 or 5551 depending on whether alpha present
                // unfortunately you have to read a block to figure out which
                // Note that we upgrade to 32-bit pixel formats here, even 
                // though the source is 16-bit; this is because the interpolated
                // values will benefit from the 32-bit results, and the source
                // from which the 16-bit samples are calculated may have been
                // 32-bit so can benefit from this.
                DXTColourBlock block;
                stream->read(&block, sizeof(DXTColourBlock));
                flipEndian(&(block.colour_0), sizeof(uint16_t));
                flipEndian(&(block.colour_1), sizeof(uint16_t));
                // skip back since we'll need to read this again
                stream->skip(0 - (long)sizeof(DXTColourBlock));
                // colour_0 <= colour_1 means transparency in DXT1
                if (block.colour_0 <= block.colour_1)
                {
                    imgData->format = PF_BYTE_RGBA;
                }
                else
                {
                    imgData->format = PF_BYTE_RGB;
                }
                break;
            case PF_DXT2:
            case PF_DXT3:
            case PF_DXT4:
            case PF_DXT5:
                // full alpha present, formats vary only in encoding 
                imgData->format = PF_BYTE_RGBA;
                break;
            case PF_BC4_UNORM:
            case PF_BC5_UNORM:
                // full alpha present, formats vary only in encoding
                imgData->format = PF_BYTE_RGBA;
                break;
            default:
                // all other cases need no special format handling
                break;
            }
        }
        else
        {
            // Use original format
            imgData->format = sourceFormat;
            // Keep DXT data compressed
            imgData->flags |= IF_COMPRESSED;
        }
    }
    else // not compressed
    {
        // Don't test against DDPF_RGB since greyscale DDS doesn't set this
        // just derive any other kind of format
        imgData->format = sourceFormat;
    }

    // Calculate total size from number of mipmaps, faces and size
    imgData->size = calculateSize(imgData->num_mipmaps, imgData->face,
        imgData->width, imgData->height, imgData->depth, imgData->format);

    // Bind output buffer
    //output.reset(OGRE_NEW MemoryDataStream(imgData->size));


    // Now deal with the data

    mDDSData = new char[imgData->size];
    void* destPtr = mDDSData;

    // all mips for a face, then each face
    for (size_t i = 0; i < imgData->face; ++i)
    {
        uint32_t width = imgData->width;
        uint32_t height = imgData->height;
        uint32_t depth = imgData->depth;

        for (size_t mip = 0; mip <= imgData->num_mipmaps; ++mip)
        {
            size_t dstPitch = width * PixelUtil::getNumElemBytes(imgData->format);

            if (PixelUtil::isCompressed(sourceFormat))
            {
                // Compressed data
                if (decompressDXT)
                {
                    DXTColourBlock col;
                    DXTInterpolatedAlphaBlock iAlpha;
                    DXTExplicitAlphaBlock eAlpha;
                    // 4x4 block of decompressed colour
                    ColourValue tempColours[16];
                    size_t destBpp = PixelUtil::getNumElemBytes(imgData->format);

                    // slices are done individually
                    for (size_t z = 0; z < depth; ++z)
                    {
                        size_t remainingHeight = height;

                        // 4x4 blocks in x/y
                        for (size_t y = 0; y < height; y += 4)
                        {
                            size_t sy = std::min<size_t>(remainingHeight, 4u);
                            remainingHeight -= sy;

                            size_t remainingWidth = width;

                            for (size_t x = 0; x < width; x += 4)
                            {
                                size_t sx = std::min<size_t>(remainingWidth, 4u);
                                size_t destPitchMinus4 = dstPitch - destBpp * sx;

                                remainingWidth -= sx;

                                if (sourceFormat == PF_DXT2 ||
                                    sourceFormat == PF_DXT3)
                                {
                                    // explicit alpha
                                    stream->read(&eAlpha, sizeof(DXTExplicitAlphaBlock));
                                    flipEndian(eAlpha.alphaRow, sizeof(uint16_t), 4);
                                    unpackDXTAlpha(eAlpha, tempColours);
                                }
                                else if (sourceFormat == PF_DXT4 ||
                                    sourceFormat == PF_DXT5)
                                {
                                    // interpolated alpha
                                    stream->read(&iAlpha, sizeof(DXTInterpolatedAlphaBlock));
                                    flipEndian(&(iAlpha.alpha_0), sizeof(uint16_t));
                                    flipEndian(&(iAlpha.alpha_1), sizeof(uint16_t));
                                    unpackDXTAlpha(iAlpha, tempColours);
                                }
                                // always read colour
                                stream->read(&col, sizeof(DXTColourBlock));
                                flipEndian(&(col.colour_0), sizeof(uint16_t));
                                flipEndian(&(col.colour_1), sizeof(uint16_t));
                                unpackDXTColour(sourceFormat, col, tempColours);

                                // write 4x4 block to uncompressed version
                                for (size_t by = 0; by < sy; ++by)
                                {
                                    for (size_t bx = 0; bx < sx; ++bx)
                                    {
                                        PixelUtil::packColour(tempColours[by * 4 + bx],
                                            imgData->format, destPtr);
                                        destPtr = static_cast<void*>(
                                            static_cast<uchar*>(destPtr) + destBpp);
                                    }
                                    // advance to next row
                                    destPtr = static_cast<void*>(
                                        static_cast<uchar*>(destPtr) + destPitchMinus4);
                                }
                                // next block. Our dest pointer is 4 lines down
                                // from where it started
                                if (x + 4 >= width)
                                {
                                    // Jump back to the start of the line
                                    destPtr = static_cast<void*>(
                                        static_cast<uchar*>(destPtr) - destPitchMinus4);
                                }
                                else
                                {
                                    // Jump back up 4 rows and 4 pixels to the
                                    // right to be at the next block to the right
                                    destPtr = static_cast<void*>(
                                        static_cast<uchar*>(destPtr) - dstPitch * sy + destBpp * sx);

                                }

                            }

                        }
                    }

                }
                else
                {
                    // load directly
                    // DDS format lies! sizeOrPitch is not always set for DXT!!
                    size_t dxtSize = PixelUtil::getMemorySize(width, height, depth, imgData->format);
                    stream->read(destPtr, dxtSize);
                    destPtr = static_cast<void*>(static_cast<uchar*>(destPtr) + dxtSize);
                }

            }
            else
            {
                // Note: We assume the source and destination have the same pitch
                for (size_t z = 0; z < depth; ++z)
                {
                    for (size_t y = 0; y < height; ++y)
                    {
                        stream->read(destPtr, dstPitch);
                        destPtr = static_cast<void*>(static_cast<uchar*>(destPtr) + dstPitch);
                    }
                }
            }

            /// Next mip
            if (width != 1) width /= 2;
            if (height != 1) height /= 2;
            if (depth != 1) depth /= 2;
        }

    }


    return true;
}
