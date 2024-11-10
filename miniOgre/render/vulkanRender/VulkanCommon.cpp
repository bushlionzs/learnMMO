#include "OgreHeader.h"
#include "VulkanCommon.h"


VkFormat getVKFormatFromType(VertexElementType type)
{
    switch (type)
    {
    case VET_FLOAT2:
        return VK_FORMAT_R32G32_SFLOAT;
        break;
    case VET_FLOAT3:
        return VK_FORMAT_R32G32B32_SFLOAT;
        break;
    case VET_FLOAT4:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
        break;
    case VET_UINT4:
        return VK_FORMAT_R32G32B32A32_UINT;
        break;
    case VET_UBYTE4_NORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
        break;
    default:
        assert(false);
        break;
    }

    return VK_FORMAT_R32G32_SFLOAT;
}

VkFormat getVKFormatFromType(spirv_cross::SPIRType type)
{
    switch (type.basetype)
    {
    case spirv_cross::SPIRType::Float:
        if (type.vecsize == 2)
        {
            return VK_FORMAT_R32G32_SFLOAT;
        }
        else if (type.vecsize == 3)
        {
            return VK_FORMAT_R32G32B32_SFLOAT;
        }
        else if (type.vecsize == 4)
        {
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        }
        break;
    }
    assert(false);
    return VK_FORMAT_R32G32_SFLOAT;
}

int32_t getTypeSize(spirv_cross::SPIRType type)
{
    switch (type.basetype)
    {
    case spirv_cross::SPIRType::Float:
        return sizeof(float) * type.vecsize;
        break;
    }
    assert(false);
    return 0;
}