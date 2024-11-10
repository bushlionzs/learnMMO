#include "OgreHeader.h"
#include "VulkanMappings.h"

namespace Ogre {

    VkBlendFactor VulkanMappings::get(SceneBlendFactor sbf, bool forAlpha)
    {
        switch (sbf)
        {
        case SBF_ONE:                       
            return VK_BLEND_FACTOR_ONE;
        case SBF_ZERO:                      
            return VK_BLEND_FACTOR_ZERO;
        case SBF_DEST_COLOUR:               
            return forAlpha ? VK_BLEND_FACTOR_DST_ALPHA : VK_BLEND_FACTOR_DST_COLOR;
        case SBF_SOURCE_COLOUR:             
            return forAlpha ? VK_BLEND_FACTOR_SRC_ALPHA : VK_BLEND_FACTOR_SRC_COLOR;
        case SBF_ONE_MINUS_DEST_COLOUR:     
            return forAlpha ? VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA : VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case SBF_ONE_MINUS_SOURCE_COLOUR:   
            return forAlpha ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA : VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case SBF_DEST_ALPHA:                
            return VK_BLEND_FACTOR_DST_ALPHA;
        case SBF_SOURCE_ALPHA:              
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case SBF_ONE_MINUS_DEST_ALPHA:      
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case SBF_ONE_MINUS_SOURCE_ALPHA:    
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        }
        return VK_BLEND_FACTOR_ZERO;
    }

    /// return a D3D11 equivalent for a Ogre SceneBlendOperation value
    VkBlendOp VulkanMappings::get(SceneBlendOperation sbo)
    {
        switch (sbo)
        {
        case SBO_ADD:
            return VK_BLEND_OP_ADD;
        case SBO_SUBTRACT:
            return VK_BLEND_OP_SUBTRACT;
        case SBO_REVERSE_SUBTRACT:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case SBO_MIN:
            return VK_BLEND_OP_MIN;
        case SBO_MAX:
            return VK_BLEND_OP_MAX;
        }
        return VK_BLEND_OP_ADD;
    }

    VkPrimitiveTopology VulkanMappings::get(OperationType opType)
    {
        switch (opType)
        {
            // clang-format off
        case OT_POINT_LIST:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case OT_LINE_LIST:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case OT_LINE_STRIP:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case OT_TRIANGLE_LIST:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case OT_TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case OT_TRIANGLE_FAN:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        default:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            // clang-format on
        }
    }

    LPCSTR VulkanMappings::getSemanticName(VertexElementSemantic sem)
    {
        // todo - add to ogre - POSITIONT and PSIZE ("Transformed vertex position" and "Point size")
        switch (sem)
        {
        case VES_BLEND_INDICES:
            return "bone_indices";
        case VES_BLEND_WEIGHTS:
            return "bone_weights";
        case VES_DIFFUSE:
            return "color_diffuse"; // NB index will differentiate
        case VES_SPECULAR:
            return "color_specular"; // NB index will differentiate
        case VES_NORMAL:
            return "normal";
        case VES_POSITION:
            return "position";
        case VES_TEXTURE_COORDINATES:
            return "texcoord";
        case VES_BINORMAL:
            return "binormal";
        case VES_TANGENT:
            return "tangent";
        }
        // to keep compiler happy
        return "";
    }

    PixelFormat VulkanMappings::_getClosestSupportedPF(PixelFormat ogrePF)
    {
        if (_getPF(ogrePF) != VK_FORMAT_UNDEFINED)
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
    //TIF_VK_FORMAT_R32G32B32_SFLOAT
    VkFormat VulkanMappings::getVertexFormat(VertexElementType elementType)
    {
        switch (elementType)
        {
        case VET_FLOAT3:
            return VK_FORMAT_R32G32B32_SFLOAT;
            break;
        default:
            assert(false);
        }
        return VK_FORMAT_R32G32B32_SFLOAT;
    }

    VkFormat VulkanMappings::_getPF(PixelFormat ogrePF)
    {
        switch (ogrePF)
        {
        case PF_R8:             return VK_FORMAT_R8_UNORM;
        case PF_L16:            return VK_FORMAT_R16_UNORM;
        case PF_A8:             return VK_FORMAT_R8_UNORM;
        case PF_BYTE_LA:        return VK_FORMAT_UNDEFINED;
        case PF_R3G3B2:         return VK_FORMAT_UNDEFINED;
        case PF_A1R5G5B5:       return VK_FORMAT_UNDEFINED;
        case PF_R5G6B5:         return VK_FORMAT_UNDEFINED;
        case PF_A4R4G4B4:       return VK_FORMAT_UNDEFINED;
        case PF_R8G8B8:         return VK_FORMAT_UNDEFINED;
        case PF_A8R8G8B8:       return VK_FORMAT_B8G8R8A8_UNORM;
        case PF_A8B8G8R8:       return VK_FORMAT_R8G8B8A8_UNORM;
        case PF_X8R8G8B8:       return VK_FORMAT_B8G8R8A8_UNORM;
        case PF_A8R8G8B8_SRGB:  return VK_FORMAT_B8G8R8A8_SRGB;
        case PF_A8B8G8R8_SRGB:  return VK_FORMAT_R8G8B8A8_SRGB;
        case PF_X8B8G8R8:       return VK_FORMAT_UNDEFINED;
        case PF_A2B10G10R10:    return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case PF_A2R10G10B10:    return VK_FORMAT_UNDEFINED;
        case PF_R11G11B10_FLOAT:return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case PF_FLOAT16_R:      return VK_FORMAT_R16_SFLOAT;
        case PF_FLOAT16_GR:     return VK_FORMAT_R16G16_SFLOAT;
        case PF_FLOAT16_RGBA:   return VK_FORMAT_R16G16B16A16_SFLOAT;
        case PF_FLOAT32_R:      return VK_FORMAT_R32_SFLOAT;
        case PF_FLOAT32_RGBA:   return VK_FORMAT_R32G32B32A32_SFLOAT;
        case PF_SHORT_RGBA:     return VK_FORMAT_R16G16B16A16_UNORM;
        case PF_DXT1:           return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
        case PF_DXT2:           return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case PF_DXT3:           return VK_FORMAT_BC2_UNORM_BLOCK;
        case PF_DXT4:           return VK_FORMAT_BC2_UNORM_BLOCK;
        case PF_DXT5:           return VK_FORMAT_BC3_UNORM_BLOCK;
        case PFG_BC3_UNORM:             return VK_FORMAT_BC3_UNORM_BLOCK;
        case PFG_BC3_UNORM_SRGB:        return VK_FORMAT_BC3_SRGB_BLOCK;
        case PF_BC4_SNORM:      return VK_FORMAT_BC4_UNORM_BLOCK;
        case PF_BC4_UNORM:      return VK_FORMAT_BC4_UNORM_BLOCK;
        case PF_BC5_SNORM:      return VK_FORMAT_BC5_UNORM_BLOCK;
        case PF_BC5_UNORM:      return VK_FORMAT_BC5_UNORM_BLOCK;
        case PF_BC6H_UF16:      return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case PF_BC7_UNORM:      return VK_FORMAT_BC7_UNORM_BLOCK;
        case PF_R16G16_SINT:    return VK_FORMAT_R16G16_SINT;
        case PF_FLOAT32_GR:     return VK_FORMAT_R32G32_SFLOAT;
        case PF_DEPTH16:        return VK_FORMAT_R32_UINT;
        case PF_DEPTH32:        return VK_FORMAT_R32_UINT;
        case PF_DEPTH32F:       return VK_FORMAT_D32_SFLOAT;
        case PF_DEPTH24_STENCIL8:     return VK_FORMAT_D24_UNORM_S8_UINT;
        case PF_DEPTH32_STENCIL8:     return VK_FORMAT_D32_SFLOAT_S8_UINT;
        default:
            //assert(false);
            return VK_FORMAT_UNDEFINED;
        }
    }

    PixelFormat VulkanMappings::getPixelFormat(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_B8G8R8A8_UNORM:          return PF_A8R8G8B8;
        case VK_FORMAT_B8G8R8A8_SRGB:           return PF_A8R8G8B8_SRGB;
        case VK_FORMAT_D32_SFLOAT:              return PF_DEPTH32F;
        case VK_FORMAT_D32_SFLOAT_S8_UINT :     return PF_DEPTH32_STENCIL8;
        default:
            assert(false);
            return PF_UNKNOWN;
        }
    }

    VkFormat VulkanMappings::_getGammaFormat(VkFormat format, bool appendSRGB)
    {
        return format;
    }

    VkAttachmentLoadOp VulkanMappings::getVkAttachmentLoadOp(LoadActionType loadAction)
    {
        switch (loadAction)
        {
        case LOAD_ACTION_DONTCARE:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case LOAD_ACTION_LOAD:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case LOAD_ACTION_CLEAR:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        default:
            assert(false);
        }
    }

    VkAttachmentStoreOp VulkanMappings::getVkAttachmentStoreOp(StoreActionType storeAction)
    {
        switch (storeAction)
        {
        case STORE_ACTION_STORE:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case STORE_ACTION_DONTCARE:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case STORE_ACTION_NONE:
            return VK_ATTACHMENT_STORE_OP_NONE;
        default:
            assert(false);
        }
    }

    VkSamplerAddressMode VulkanMappings::getWrapMode(filament::backend::SamplerWrapMode mode)
    {
        switch (mode) {
        case filament::backend::SamplerWrapMode::REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case filament::backend::SamplerWrapMode::CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case filament::backend::SamplerWrapMode::MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        default:
            assert(false);
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    VkFilter VulkanMappings::getFilter(filament::backend::SamplerMinFilter filter)
    {
        switch (filter) {
        case filament::backend::SamplerMinFilter::NEAREST:
            return VK_FILTER_NEAREST;
        case filament::backend::SamplerMinFilter::LINEAR:
            return VK_FILTER_LINEAR;
        case filament::backend::SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
            return VK_FILTER_NEAREST;
        case filament::backend::SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
            return VK_FILTER_LINEAR;
        case filament::backend::SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
            return VK_FILTER_NEAREST;
        case filament::backend::SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
            return VK_FILTER_LINEAR;
        default:
            assert(false);
            return VK_FILTER_NEAREST;
        }
    }
    VkFilter VulkanMappings::getFilter(filament::backend::SamplerFilterType filter)
    {
        switch (filter) {
        case filament::backend::SamplerFilterType::NEAREST:
            return VK_FILTER_NEAREST;
        case filament::backend::SamplerFilterType::LINEAR:
            return VK_FILTER_LINEAR;
        default:
            assert(false);
            return VK_FILTER_NEAREST;
        }
    }

    VkSamplerMipmapMode VulkanMappings::getMipmapMode(filament::backend::SamplerMipMapMode filter)
    {
        switch (filter) {
        case filament::backend::SamplerMipMapMode::MIPMAP_MODE_NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case filament::backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        default:
            assert(false);
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        }
    }

    float VulkanMappings::getMaxLod(filament::backend::SamplerMipMapMode mipMapMode)
    {
        switch (mipMapMode) {
        case filament::backend::SamplerMipMapMode::MIPMAP_MODE_NEAREST:
            return 0;
        case filament::backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR:
            // The Vulkan spec recommends a max LOD of 0.25 to "disable" mipmapping.
            // See "Mapping of OpenGL to Vulkan filter modes" in the VK Spec.
            return VK_LOD_CLAMP_NONE;
        default:
            assert(false);
            return VK_LOD_CLAMP_NONE;
        }
    }

    VkBool32 VulkanMappings::getCompareEnable(filament::backend::SamplerCompareMode mode)
    {
        return mode == filament::backend::SamplerCompareMode::NONE ? VK_FALSE : VK_TRUE;
    }

    VkCompareOp VulkanMappings::getCompareOp(filament::backend::SamplerCompareFunc func)
    {
        using Compare = filament::backend::SamplerCompareFunc;
        switch (func) {
        case Compare::LE: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case Compare::GE: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case Compare::L:  return VK_COMPARE_OP_LESS;
        case Compare::G:  return VK_COMPARE_OP_GREATER;
        case Compare::E:  return VK_COMPARE_OP_EQUAL;
        case Compare::NE: return VK_COMPARE_OP_NOT_EQUAL;
        case Compare::A:  return VK_COMPARE_OP_ALWAYS;
        case Compare::N:  return VK_COMPARE_OP_NEVER;
        default:
            assert(false);
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        }
    }

    VkAccessFlags VulkanMappings::util_to_vk_access_flags(uint32_t state)
    {
        VkAccessFlags ret = 0;
        if (state & RESOURCE_STATE_COPY_SOURCE)
        {
            ret |= VK_ACCESS_TRANSFER_READ_BIT;
        }
        if (state & RESOURCE_STATE_COPY_DEST)
        {
            ret |= VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        if (state & RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
        {
            ret |= VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        }
        if (state & RESOURCE_STATE_INDEX_BUFFER)
        {
            ret |= VK_ACCESS_INDEX_READ_BIT;
        }
        if (state & RESOURCE_STATE_UNORDERED_ACCESS)
        {
            ret |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        }
        if (state & RESOURCE_STATE_INDIRECT_ARGUMENT)
        {
            ret |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
        }
        if (state & RESOURCE_STATE_RENDER_TARGET)
        {
            ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        }
        if (state & RESOURCE_STATE_DEPTH_WRITE)
        {
            ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }
        if (state & RESOURCE_STATE_DEPTH_READ)
        {
            ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        }
        if (state & RESOURCE_STATE_SHADER_RESOURCE)
        {
            ret |= VK_ACCESS_SHADER_READ_BIT;
        }
        if (state & RESOURCE_STATE_PRESENT)
        {
            ret |= VK_ACCESS_MEMORY_READ_BIT;
        }

        if (state & RESOURCE_STATE_ACCELERATION_STRUCTURE_READ)
        {
            ret |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        }
        if (state & RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE)
        {
            ret |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        }

        return ret;
    }

    VkImageLayout VulkanMappings::util_to_vk_image_layout(uint32_t usage)
    {
        if (usage & RESOURCE_STATE_COPY_SOURCE)
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        if (usage & RESOURCE_STATE_COPY_DEST)
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        if (usage & RESOURCE_STATE_RENDER_TARGET)
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (usage & RESOURCE_STATE_DEPTH_WRITE)
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        else if (usage & RESOURCE_STATE_DEPTH_READ)
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        if (usage & RESOURCE_STATE_UNORDERED_ACCESS)
            return VK_IMAGE_LAYOUT_GENERAL;

        if (usage & RESOURCE_STATE_SHADER_RESOURCE)
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        if (usage & RESOURCE_STATE_PRESENT)
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        if (usage == RESOURCE_STATE_COMMON)
            return VK_IMAGE_LAYOUT_GENERAL;

        return VK_IMAGE_LAYOUT_UNDEFINED;
    }


    VkGeometryFlagsKHR VulkanMappings::util_to_vk_geometry_flags(
        AccelerationStructureGeometryFlags flags)
    {
        VkGeometryFlagsKHR ret = 0;
        if (flags & ACCELERATION_STRUCTURE_GEOMETRY_FLAG_OPAQUE)
            ret |= VK_GEOMETRY_OPAQUE_BIT_KHR;
        if (flags & ACCELERATION_STRUCTURE_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION)
            ret |= VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;

        return ret;
    }

    VkGeometryInstanceFlagsKHR VulkanMappings::util_to_vk_instance_flags(
        AccelerationStructureInstanceFlags flags)
    {
        VkGeometryInstanceFlagsKHR ret = 0;
        if (flags & ACCELERATION_STRUCTURE_INSTANCE_FLAG_FORCE_OPAQUE)
            ret |= VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR;
        if (flags & ACCELERATION_STRUCTURE_INSTANCE_FLAG_FORCE_NON_OPAQUE)
            ret |= VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR;
        if (flags & ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE)
            ret |= VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        if (flags & ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE)
            ret |= VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR;

        return ret;
    }

    VkBuildAccelerationStructureFlagsKHR VulkanMappings::ToVkBuildASFlags(
        AccelerationStructureBuildFlags flags)
    {
        VkBuildAccelerationStructureFlagsKHR ret = 0;
        if (flags & ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION)
            ret |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
        if (flags & ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE)
            ret |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
        if (flags & ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY)
            ret |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR;
        if (flags & ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE)
            ret |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
        if (flags & ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD)
            ret |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
        if (flags & ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE)
            ret |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

        return ret;
    }

    VkAccelerationStructureTypeKHR VulkanMappings::ToVkASType(AccelerationStructureType type)
    {
        return ACCELERATION_STRUCTURE_TYPE_BOTTOM == 
            type ? VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
            : VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    }
}
