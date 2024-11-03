#pragma once
#include <stdint.h>
#include <DriverBase.h>
#include <OgreCommon.h>
#include <OgrePixelFormat.h>

struct AccelerationStructure;


enum AccelerationStructureType
{
    ACCELERATION_STRUCTURE_TYPE_BOTTOM = 0,
    ACCELERATION_STRUCTURE_TYPE_TOP,
};

enum AccelerationStructureBuildFlags
{
    ACCELERATION_STRUCTURE_BUILD_FLAG_NONE = 0,
    ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE = 0x1,
    ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION = 0x2,
    ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE = 0x4,
    ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD = 0x8,
    ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY = 0x10,
    ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE = 0x20,
};

enum AccelerationStructureGeometryFlags
{
    ACCELERATION_STRUCTURE_GEOMETRY_FLAG_NONE = 0,
    ACCELERATION_STRUCTURE_GEOMETRY_FLAG_OPAQUE = 0x1,
    ACCELERATION_STRUCTURE_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION = 0x2
} ;

enum AccelerationStructureInstanceFlags
{
    ACCELERATION_STRUCTURE_INSTANCE_FLAG_NONE = 0,
    ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE = 0x1,
    ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE = 0x2,
    ACCELERATION_STRUCTURE_INSTANCE_FLAG_FORCE_OPAQUE = 0x4,
    ACCELERATION_STRUCTURE_INSTANCE_FLAG_FORCE_NON_OPAQUE = 0x8
} ;

typedef struct AccelerationStructureInstanceDesc
{
    AccelerationStructure* pBottomAS;
    /// Row major affine transform for transforming the vertices in the geometry stored in pAccelerationStructure
    float                              mTransform[12];
    /// User defined instanced ID which can be queried in the shader
    uint32_t                           mInstanceID;
    uint32_t                           mInstanceMask;
    uint32_t                           mInstanceContributionToHitGroupIndex;
    AccelerationStructureInstanceFlags mFlags;
} AccelerationStructureInstanceDesc;

typedef struct AccelerationStructureGeometryDesc
{
    Handle<HwBufferObject> vertexBufferHandle;
    Handle<HwBufferObject> indexBufferHandle;
    Handle< HwBufferObject> transformBufferHandle;
    uint32_t                           mVertexOffset;
    uint32_t                           mVertexCount;
    uint32_t                           mVertexStride;
    VertexElementType         mVertexElementType;
    uint32_t                           mIndexOffset;
    uint32_t                           mIndexCount;
    Ogre::IndexType              mIndexType;
    AccelerationStructureGeometryFlags mFlags;
} AccelerationStructureGeometryDesc;

typedef struct AccelerationStructureDescBottom
{
    /// Number of geometries / instances in thie acceleration structure
    uint32_t                           mDescCount;
    /// Array of geometries in the bottom level acceleration structure
    AccelerationStructureGeometryDesc* pGeometryDescs;
} AccelerationStructureDescBottom;

typedef struct AccelerationStructureDescTop
{
    uint32_t                           mDescCount;
    AccelerationStructureInstanceDesc* pInstanceDescs;
} AccelerationStructureDescTop;

typedef struct AccelerationStructureDesc
{
    AccelerationStructureType       mType;
    AccelerationStructureBuildFlags mFlags;
    union
    {
        AccelerationStructureDescBottom mBottom;
        AccelerationStructureDescTop    mTop;
    };
} AccelerationStructureDesc;

struct RaytracingBuildASDesc
{
    AccelerationStructure* pAccelerationStructure;
    bool                   mIssueRWBarrier;
};