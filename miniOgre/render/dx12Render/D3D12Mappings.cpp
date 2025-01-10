#include "OgreHeader.h"
#include "D3D12Mappings.h"

namespace Ogre {

    D3D12_BLEND D3D12Mappings::get(SceneBlendFactor sbf, bool forAlpha)
    {
        switch (sbf)
        {
        case SBF_ONE:                       return D3D12_BLEND_ONE;
        case SBF_ZERO:                      return D3D12_BLEND_ZERO;
        case SBF_DEST_COLOUR:               return forAlpha ? D3D12_BLEND_DEST_ALPHA : D3D12_BLEND_DEST_COLOR;
        case SBF_SOURCE_COLOUR:             return forAlpha ? D3D12_BLEND_SRC_ALPHA : D3D12_BLEND_SRC_COLOR;
        case SBF_ONE_MINUS_DEST_COLOUR:     return forAlpha ? D3D12_BLEND_INV_DEST_ALPHA : D3D12_BLEND_INV_DEST_COLOR;
        case SBF_ONE_MINUS_SOURCE_COLOUR:   return forAlpha ? D3D12_BLEND_INV_SRC_ALPHA : D3D12_BLEND_INV_SRC_COLOR;
        case SBF_DEST_ALPHA:                return D3D12_BLEND_DEST_ALPHA;
        case SBF_SOURCE_ALPHA:              return D3D12_BLEND_SRC_ALPHA;
        case SBF_ONE_MINUS_DEST_ALPHA:      return D3D12_BLEND_INV_DEST_ALPHA;
        case SBF_ONE_MINUS_SOURCE_ALPHA:    return D3D12_BLEND_INV_SRC_ALPHA;
        }
        return D3D12_BLEND_ZERO;
    }

    /// return a D3D11 equivalent for a Ogre SceneBlendOperation value
    D3D12_BLEND_OP D3D12Mappings::get(SceneBlendOperation sbo)
    {
        switch (sbo)
        {
        case SBO_ADD:
            return D3D12_BLEND_OP_ADD;
        case SBO_SUBTRACT:
            return D3D12_BLEND_OP_SUBTRACT;
        case SBO_REVERSE_SUBTRACT:
            return D3D12_BLEND_OP_REV_SUBTRACT;
        case SBO_MIN:
            return D3D12_BLEND_OP_MIN;
        case SBO_MAX:
            return D3D12_BLEND_OP_MAX;
        }
        return D3D12_BLEND_OP_ADD;
    }

    LPCSTR D3D12Mappings::getSemanticName(VertexElementSemantic sem)
    {
        // todo - add to ogre - POSITIONT and PSIZE ("Transformed vertex position" and "Point size")
        switch (sem)
        {
        case VES_BLEND_INDICES:
            return "BLENDINDICES";
        case VES_BLEND_WEIGHTS:
            return "BLENDWEIGHT";
        case VES_DIFFUSE:
            return "COLOR"; // NB index will differentiate
        case VES_SPECULAR:
            return "COLOR"; // NB index will differentiate
        case VES_NORMAL:
            return "NORMAL";
        case VES_POSITION:
            return "POSITION";
        case VES_TEXTURE_COORDINATES:
            return "TEXCOORD";
        case VES_BINORMAL:
            return "BINORMAL";
        case VES_TANGENT:
            return "TANGENT";
        }
        // to keep compiler happy
        return "";
    }

    PixelFormat D3D12Mappings::_getClosestSupportedPF(PixelFormat ogrePF)
    {
        if (_getPF(ogrePF) != DXGI_FORMAT_UNKNOWN)
        {
            return ogrePF;
        }
        switch (ogrePF)
        {
        case PF_R8G8B8:
            return PF_X8R8G8B8;
        case PF_FLOAT16_RGB:
            return PF_FLOAT16_RGBA;
        case PF_FLOAT32_RGB:
            return PF_FLOAT32_RGBA;
        case PF_DEPTH16:
            return PF_L16;
        case PF_DEPTH24_STENCIL8:
        case PF_DEPTH32:
        case PF_DEPTH32F:
            return PF_FLOAT32_R;
        case PF_UNKNOWN:
        default:
            return PF_A8B8G8R8;
        }
    }

    /// utility method, convert Ogre pixel format to D3D11 pixel format
    DXGI_FORMAT D3D12Mappings::_getPF(PixelFormat ogrePF)
    {
        switch (ogrePF)
        {
        case PF_R8:             return DXGI_FORMAT_R8_UNORM;
        case PF_L16:            return DXGI_FORMAT_R16_UNORM;
        case PF_A8:             return DXGI_FORMAT_A8_UNORM;
        case PF_BYTE_LA:        return DXGI_FORMAT_UNKNOWN;
        case PF_R3G3B2:         return DXGI_FORMAT_UNKNOWN;
        case PF_A1R5G5B5:       return DXGI_FORMAT_UNKNOWN;
        case PF_R5G6B5:         return DXGI_FORMAT_UNKNOWN;
        case PF_A4R4G4B4:       return DXGI_FORMAT_UNKNOWN;
        case PF_R8G8B8:         return DXGI_FORMAT_UNKNOWN;
        case PF_A8R8G8B8:       return DXGI_FORMAT_B8G8R8A8_UNORM;
        case PF_A8R8G8B8_SRGB:       return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case PF_A8B8G8R8:       return DXGI_FORMAT_R8G8B8A8_UNORM;
        case PF_X8R8G8B8:       return DXGI_FORMAT_B8G8R8X8_UNORM;
        case PF_X8B8G8R8:       return DXGI_FORMAT_UNKNOWN;
        case PF_A2B10G10R10:    return DXGI_FORMAT_R10G10B10A2_TYPELESS;
        case PF_A2R10G10B10:    return DXGI_FORMAT_UNKNOWN;
        case PF_R11G11B10_FLOAT:return DXGI_FORMAT_R11G11B10_FLOAT;
        case PF_FLOAT16_R:      return DXGI_FORMAT_R16_FLOAT;
        case PF_FLOAT16_GR:     return DXGI_FORMAT_R16G16_FLOAT;
        case PF_FLOAT16_RGBA:   return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case PF_RGBA16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
        case PF_FLOAT32_R:      return DXGI_FORMAT_R32_FLOAT;
        case PF_FLOAT32_RGBA:   return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case PF_SHORT_RGBA:     return DXGI_FORMAT_R16G16B16A16_UNORM;
        case PF_DXT1:           return DXGI_FORMAT_BC1_UNORM;
        case PF_DXT2:           return DXGI_FORMAT_BC1_UNORM;
        case PF_DXT3:           return DXGI_FORMAT_BC2_UNORM;
        case PF_DXT4:           return DXGI_FORMAT_BC2_UNORM;
        case PF_DXT5:           return DXGI_FORMAT_BC3_UNORM;
        case PFG_BC3_UNORM:             return DXGI_FORMAT_BC3_UNORM;
        case PFG_BC3_UNORM_SRGB:        return DXGI_FORMAT_BC3_UNORM_SRGB;
        case PF_BC4_SNORM:      return DXGI_FORMAT_BC4_SNORM;
        case PF_BC4_UNORM:      return DXGI_FORMAT_BC4_UNORM;
        case PF_BC5_SNORM:      return DXGI_FORMAT_BC5_SNORM;
        case PF_BC5_UNORM:      return DXGI_FORMAT_BC5_UNORM;
        case PF_BC6H_UF16:      return DXGI_FORMAT_BC6H_UF16;
        case PF_BC6H_SF16:      return DXGI_FORMAT_BC6H_SF16;
        case PF_BC7_UNORM:      return DXGI_FORMAT_BC7_UNORM;
        case PF_R16G16_SINT:    return DXGI_FORMAT_R16G16_SINT;
        case PF_FLOAT32_GR:     return DXGI_FORMAT_R32G32_FLOAT;
        case PF_DEPTH16:        return DXGI_FORMAT_R32_TYPELESS;
        case PF_DEPTH32:        return DXGI_FORMAT_R32_TYPELESS;
        case PF_DEPTH32F:       return DXGI_FORMAT_D32_FLOAT;
        case PF_DEPTH24_STENCIL8:     return DXGI_FORMAT_D24_UNORM_S8_UINT;
        
        default:                return DXGI_FORMAT_UNKNOWN;
        }
    }
    /// utility method, optionally maps plain format to _SRGB counterparts
    DXGI_FORMAT D3D12Mappings::_getGammaFormat(DXGI_FORMAT format, bool appendSRGB)
    {
        if (appendSRGB)
        {
            switch (format)
            {
            case DXGI_FORMAT_R8G8B8A8_UNORM:       return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case DXGI_FORMAT_B8G8R8A8_UNORM:       return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            case DXGI_FORMAT_B8G8R8X8_UNORM:       return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
            case DXGI_FORMAT_BC1_UNORM:            return DXGI_FORMAT_BC1_UNORM_SRGB;
            case DXGI_FORMAT_BC2_UNORM:            return DXGI_FORMAT_BC2_UNORM_SRGB;
            case DXGI_FORMAT_BC3_UNORM:            return DXGI_FORMAT_BC3_UNORM_SRGB;
            case DXGI_FORMAT_BC7_UNORM:            return DXGI_FORMAT_BC7_UNORM_SRGB;
            }
        }
        return format;
    }

    DXGI_FORMAT D3D12Mappings::dx12GetType(VertexElementType vType)
    {
        switch (vType)
        {
            // Float32
        case VET_FLOAT1:
            return DXGI_FORMAT_R32_FLOAT;
        case VET_FLOAT2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case VET_FLOAT3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case VET_FLOAT4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;

            // Signed short
        case VET_SHORT1:
            return DXGI_FORMAT_R16_SINT;
        case VET_SHORT2:
            return DXGI_FORMAT_R16G16_SINT;
        case VET_SHORT4:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case VET_SHORT2_NORM:
            return DXGI_FORMAT_R16G16_SNORM;
        case VET_SHORT4_NORM:
            return DXGI_FORMAT_R16G16B16A16_SNORM;

            // Unsigned short
        case VET_USHORT1:
            return DXGI_FORMAT_R16_UINT;
        case VET_USHORT2:
            return DXGI_FORMAT_R16G16_UINT;
        case VET_USHORT4:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case VET_USHORT2_NORM:
            return DXGI_FORMAT_R16G16_UNORM;
        case VET_USHORT4_NORM:
            return DXGI_FORMAT_R16G16B16A16_UNORM;

            // Signed int
        case VET_INT1:
            return DXGI_FORMAT_R32_SINT;
        case VET_INT2:
            return DXGI_FORMAT_R32G32_SINT;
        case VET_INT3:
            return DXGI_FORMAT_R32G32B32_SINT;
        case VET_INT4:
            return DXGI_FORMAT_R32G32B32A32_SINT;

            // Unsigned int
        case VET_UINT1:
            return DXGI_FORMAT_R32_UINT;
        case VET_UINT2:
            return DXGI_FORMAT_R32G32_UINT;
        case VET_UINT3:
            return DXGI_FORMAT_R32G32B32_UINT;
        case VET_UINT4:
            return DXGI_FORMAT_R32G32B32A32_UINT;

        case VET_BYTE4:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case VET_BYTE4_NORM:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case VET_UBYTE4:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case VET_UBYTE4_NORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        }
        // to keep compiler happy
        return DXGI_FORMAT_R32G32B32_FLOAT;
    }

    D3D12_CULL_MODE D3D12Mappings::getCullMode(backend::CullingMode mode)
    {
        switch (mode) {
        case backend::CullingMode::NONE:           return D3D12_CULL_MODE_NONE;
        case backend::CullingMode::FRONT:          return D3D12_CULL_MODE_FRONT;
        case backend::CullingMode::BACK:           return D3D12_CULL_MODE_BACK;
        case backend::CullingMode::FRONT_AND_BACK: return D3D12_CULL_MODE_NONE;
        }
    }

    D3D12_BLEND D3D12Mappings::getBlendFactor(BlendFunction mode) {
        switch (mode) {
        case BlendFunction::ZERO:                  return D3D12_BLEND_ZERO;
        case BlendFunction::ONE:                   return D3D12_BLEND_ONE;
        case BlendFunction::SRC_COLOR:             return D3D12_BLEND_SRC_COLOR;
        case BlendFunction::ONE_MINUS_SRC_COLOR:   return D3D12_BLEND_INV_SRC_COLOR;
        case BlendFunction::DST_COLOR:             return D3D12_BLEND_DEST_COLOR;
        case BlendFunction::ONE_MINUS_DST_COLOR:   return D3D12_BLEND_INV_DEST_COLOR;
        case BlendFunction::SRC_ALPHA:             return D3D12_BLEND_SRC_ALPHA;
        case BlendFunction::ONE_MINUS_SRC_ALPHA:   return D3D12_BLEND_INV_SRC_ALPHA;
        case BlendFunction::DST_ALPHA:             return D3D12_BLEND_DEST_ALPHA;
        case BlendFunction::ONE_MINUS_DST_ALPHA:   return D3D12_BLEND_INV_DEST_ALPHA;
        case BlendFunction::SRC_ALPHA_SATURATE:    return D3D12_BLEND_SRC_ALPHA_SAT;
        default:
            assert(false);
            break;
        }
    }

    D3D12_BLEND_OP D3D12Mappings::getBlendOp(BlendEquation equation)
    {
        
        switch (equation)
        {
        case BlendEquation::ADD:
            return D3D12_BLEND_OP_ADD;
        case BlendEquation::SUBTRACT:
            return D3D12_BLEND_OP_SUBTRACT;
        case BlendEquation::REVERSE_SUBTRACT:
            return D3D12_BLEND_OP_REV_SUBTRACT;
        default:
            assert(false);
            break;
        }
    }

    D3D12_COMPARISON_FUNC D3D12Mappings::getComparisonFunc(
        RasterState::DepthFunc depthFunc)
    {
        switch (depthFunc)
        {
        case RasterState::DepthFunc::LE:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case RasterState::DepthFunc::GE:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case RasterState::DepthFunc::L:
            return D3D12_COMPARISON_FUNC_LESS;
        case RasterState::DepthFunc::G:
            return D3D12_COMPARISON_FUNC_GREATER;
        case RasterState::DepthFunc::E:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case RasterState::DepthFunc::NE:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case RasterState::DepthFunc::A:
            return D3D12_COMPARISON_FUNC_ALWAYS;
        case RasterState::DepthFunc::N:
            return D3D12_COMPARISON_FUNC_NEVER;
        };
        return D3D12_COMPARISON_FUNC_ALWAYS;
    }

    D3D12_DESCRIPTOR_RANGE_TYPE D3D12Mappings::getRangeType(D3D_SHADER_INPUT_TYPE inputType)
    {
        switch (inputType)
        {
        case D3D_SIT_CBUFFER:
            return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        case D3D_SIT_TEXTURE:
        case D3D_SIT_BYTEADDRESS:
        case D3D_SIT_STRUCTURED:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        case D3D_SIT_SAMPLER:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        case D3D_SIT_UAV_RWSTRUCTURED:
        case D3D_SIT_UAV_RWBYTEADDRESS:
        case D3D_SIT_UAV_RWTYPED:
            return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        default:
            assert(false);
            break;
        }
    }

    D3D12_SHADER_VISIBILITY D3D12Mappings::getShaderVisibility(uint8_t stages)
    {
        D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        
        return shaderVisibility;
    }

    Ogre::PixelFormat D3D12Mappings::getPixelFormat(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_B8G8R8A8_UNORM:          return PF_A8R8G8B8;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:           return PF_A8R8G8B8_SRGB;
        case DXGI_FORMAT_D32_FLOAT:              return PF_DEPTH32F;
        case DXGI_FORMAT_D24_UNORM_S8_UINT: return PF_DEPTH24_STENCIL8;
        default:
            assert(false);
            return PF_UNKNOWN;
        }
    }

    D3D12_RESOURCE_STATES D3D12Mappings::util_to_dx12_resource_state(uint32_t state)
    {
        D3D12_RESOURCE_STATES ret = D3D12_RESOURCE_STATE_COMMON;

        // These states cannot be combined with other states so we just do an == check
        if (state == RESOURCE_STATE_GENERIC_READ)
            return D3D12_RESOURCE_STATE_GENERIC_READ;
        if (state == RESOURCE_STATE_COMMON)
            return D3D12_RESOURCE_STATE_COMMON;
        if (state == RESOURCE_STATE_PRESENT)
            return D3D12_RESOURCE_STATE_PRESENT;

        if (state & RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            ret |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        if (state & RESOURCE_STATE_INDEX_BUFFER)
            ret |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        if (state & RESOURCE_STATE_RENDER_TARGET)
            ret |= D3D12_RESOURCE_STATE_RENDER_TARGET;
        if (state & RESOURCE_STATE_UNORDERED_ACCESS)
            ret |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        if (state & RESOURCE_STATE_DEPTH_WRITE)
            ret |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
        else if (state & RESOURCE_STATE_DEPTH_READ)
            ret |= D3D12_RESOURCE_STATE_DEPTH_READ;
        if (state & RESOURCE_STATE_STREAM_OUT)
            ret |= D3D12_RESOURCE_STATE_STREAM_OUT;
        if (state & RESOURCE_STATE_INDIRECT_ARGUMENT)
            ret |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        if (state & RESOURCE_STATE_COPY_DEST)
            ret |= D3D12_RESOURCE_STATE_COPY_DEST;
        if (state & RESOURCE_STATE_COPY_SOURCE)
            ret |= D3D12_RESOURCE_STATE_COPY_SOURCE;
        if (state & RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
            ret |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (state & RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
            ret |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
#ifdef D3D12_RAYTRACING_AVAILABLE
        if (state & (RESOURCE_STATE_ACCELERATION_STRUCTURE_READ | RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE))
            ret |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
#endif

        return ret;
    }

    DXGI_FORMAT D3D12Mappings::util_to_dx12_srv_format(DXGI_FORMAT defaultFormat)
    {
        switch (defaultFormat)
        {
        case DXGI_FORMAT_D32_FLOAT:
            return DXGI_FORMAT_R32_FLOAT;
        default:
            return defaultFormat;
        }
    }

    D3D12_TEXTURE_ADDRESS_MODE D3D12Mappings::getWrapMode(
        filament::backend::SamplerWrapMode mode)
    {
        switch (mode)
        {
        case SamplerWrapMode::CLAMP_TO_EDGE:
            return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case SamplerWrapMode::CLAMP_TO_BODY:
            return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case SamplerWrapMode::REPEAT:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case SamplerWrapMode::MIRRORED_REPEAT:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
    }

    D3D12_FILTER D3D12Mappings::getFilter(const filament::backend::SamplerParams& params)
    {
        if (params.filterMin == SamplerFilterType::NEAREST)
        {
            if (params.filterMag == SamplerFilterType::NEAREST)
            {
                switch (params.mipMapMode) {
                case backend::SamplerMipMapMode::MIPMAP_MODE_NEAREST:
                    return D3D12_FILTER_MIN_MAG_MIP_POINT;
                case backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR:
                    return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
                }
            }
            else
            {
                switch (params.mipMapMode) {
                case backend::SamplerMipMapMode::MIPMAP_MODE_NEAREST:
                    return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
                case backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR:
                    return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
                }
            }
        }
        else if (params.filterMin == SamplerFilterType::LINEAR)
        {
            if (params.filterMag == SamplerFilterType::NEAREST)
            {
                switch (params.mipMapMode) {
                case backend::SamplerMipMapMode::MIPMAP_MODE_NEAREST:
                    return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
                case backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR:
                    return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                }
            }
            else
            {
                switch (params.mipMapMode) {
                case backend::SamplerMipMapMode::MIPMAP_MODE_NEAREST:
                    return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                case backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR:
                    return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                }
            }
        }
        return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    }

    float D3D12Mappings::getMaxLod(filament::backend::SamplerMipMapMode mipMapMode)
    {
        switch (mipMapMode) {
        case filament::backend::SamplerMipMapMode::MIPMAP_MODE_NEAREST:
            return FLT_MAX;
        case filament::backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR:
            return FLT_MAX;
        default:
            assert(false);
            return 1000.0f;
        }
    }
    D3D12_COMPARISON_FUNC D3D12Mappings::getCompareOp(
        filament::backend::SamplerCompareFunc func)
    {
        using Compare = filament::backend::SamplerCompareFunc;
        switch (func) {
        case Compare::LE: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case Compare::GE: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case Compare::L:  return D3D12_COMPARISON_FUNC_LESS;
        case Compare::G:  return D3D12_COMPARISON_FUNC_GREATER;
        case Compare::E:  return D3D12_COMPARISON_FUNC_EQUAL;
        case Compare::NE: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case Compare::A:  return D3D12_COMPARISON_FUNC_ALWAYS;
        case Compare::N:  return D3D12_COMPARISON_FUNC_NEVER;
        default:
            assert(false);
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        }
    }

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS
        D3D12Mappings::util_to_dx_acceleration_structure_build_flags(AccelerationStructureBuildFlags flags)
    {
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS ret = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
        if (flags & ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION)
            ret |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
        if (flags & ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE)
            ret |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
        if (flags & ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY)
            ret |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
        if (flags & ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE)
            ret |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
        if (flags & ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD)
            ret |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
        if (flags & ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE)
            ret |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

        return ret;
    }

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE D3D12Mappings::ToDXRASType(AccelerationStructureType type)
    {
        return ACCELERATION_STRUCTURE_TYPE_BOTTOM == 
            type ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL
            : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    }

    D3D12_RAYTRACING_GEOMETRY_FLAGS D3D12Mappings::util_to_dx_geometry_flags(AccelerationStructureGeometryFlags flags)
    {
        D3D12_RAYTRACING_GEOMETRY_FLAGS ret = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
        if (flags & ACCELERATION_STRUCTURE_GEOMETRY_FLAG_OPAQUE)
            ret |= D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        if (flags & ACCELERATION_STRUCTURE_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION)
            ret |= D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;

        return ret;
    }

    D3D12_RAYTRACING_INSTANCE_FLAGS D3D12Mappings::util_to_dx_instance_flags(AccelerationStructureInstanceFlags flags)
    {
        D3D12_RAYTRACING_INSTANCE_FLAGS ret = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
        if (flags & ACCELERATION_STRUCTURE_INSTANCE_FLAG_FORCE_OPAQUE)
            ret |= D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE;
        if (flags & ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE)
            ret |= D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE;
        if (flags & ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE)
            ret |= D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE;

        return ret;
    }
}
