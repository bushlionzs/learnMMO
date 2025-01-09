#pragma once
#define OGRE_STREAM_TEMP_SIZE 128
#include <OgreHeader.h>
#include <Handle.h>
#include <DriverEnums.h>
#include <OgreCommon.h>


using namespace filament::backend;

enum RenderListType
{
    RenderListType_Opaque = 0,
    RenderListType_Transparent
};

enum ObjectType
{
    ObjectType_Static = 0,
    ObjectType_Dynamic
};

class EngineRenderList
{
public:
    std::vector<Ogre::Renderable*> mOpaqueList;
    std::vector<Ogre::Renderable*> mTransparentList;
};

class CubeMatrix
{
public:
    Ogre::Matrix4 view;
    Ogre::Matrix4 proj;
};

class TransformInfo
{
public:
    Ogre::Vector3 translate;
    Ogre::Quaternion rotate;
    Ogre::Vector3 scale;
};

class RawData
{
public:
    const char* mData;
    uint32_t mDataSize;
    uint32_t mDataStride;
};

class IndexDataView
{
public:
    uint32_t mIndexCount;
    uint32_t mIndexLocation;
    uint32_t mBaseVertexLocation;
};

class RawDataView
{
public:
    uint32_t mVertexStart;
    uint32_t mVertexCount;
};

typedef struct 
{
    unsigned int vertexIndex;
    unsigned short boneIndex;
    Real weight;

} VertexBoneAssignment;

typedef union ClearValue
{
    struct
    {
        float r;
        float g;
        float b;
        float a;
    };
    struct
    {
        float    depth;
        uint32_t stencil;
    };
} ClearValue;

typedef struct BindRenderTargetDesc
{
    Ogre::RenderTarget* renderTarget;
    ClearValue clearColour;
} BindRenderTargetDesc;

typedef struct BindDepthTargetDesc
{
    Ogre::RenderTarget* depthStencil;
    ClearValue      clearValue;
} BindDepthTargetDesc;

enum
{
    MAX_INSTANCE_EXTENSIONS = 64,
    MAX_DEVICE_EXTENSIONS = 64,
    /// Max number of GPUs in SLI or Cross-Fire
    MAX_LINKED_GPUS = 4,
    /// Max number of GPUs in unlinked mode
    MAX_UNLINKED_GPUS = 4,
    /// Max number of GPus for either linked or unlinked mode. must update WindowsBase::setupPlatformUI accordingly
    MAX_MULTIPLE_GPUS = 4,
    MAX_RENDER_TARGET_ATTACHMENTS = 8,
    MAX_VERTEX_BINDINGS = 15,
    MAX_VERTEX_ATTRIBS = 15,
    MAX_RESOURCE_NAME_LENGTH = 256,
    MAX_SEMANTIC_NAME_LENGTH = 128,
    MAX_DEBUG_NAME_LENGTH = 128,
    MAX_MIP_LEVELS = 0xFFFFFFFF,
    MAX_SWAPCHAIN_IMAGES = 3,
    MAX_GPU_VENDOR_STRING_LENGTH = 256, // max size for GPUVendorPreset strings
    MAX_SAMPLE_LOCATIONS = 16,
};

typedef enum LoadActionType
{
    LOAD_ACTION_DONTCARE,
    LOAD_ACTION_LOAD,
    LOAD_ACTION_CLEAR,
    MAX_LOAD_ACTION
} LoadActionType;

typedef enum StoreActionType
{
    // Store is the most common use case so keep that as default
    STORE_ACTION_STORE,
    STORE_ACTION_DONTCARE,
    STORE_ACTION_NONE,
#if defined(USE_MSAA_RESOLVE_ATTACHMENTS)
    // Resolve into pResolveAttachment and also store the MSAA attachment (rare - maybe used for debug)
    STORE_ACTION_RESOLVE_STORE,
    // Resolve into pResolveAttachment and discard MSAA attachment (most common use case for resolve)
    STORE_ACTION_RESOLVE_DONTCARE,
#endif
    MAX_STORE_ACTION
} StoreActionType;

struct RenderPassInfo
{
    uint32_t renderTargetCount = 1;
    BindRenderTargetDesc renderTargets[MAX_RENDER_TARGET_ATTACHMENTS];
    LoadActionType renderLoadAction = LOAD_ACTION_CLEAR;
    StoreActionType renderStoreAction = STORE_ACTION_STORE;
    BindDepthTargetDesc  depthTarget;
    LoadActionType depthLoadAction = LOAD_ACTION_CLEAR;
    StoreActionType depthStoreAction = STORE_ACTION_STORE;
    uint32_t            extent[2];
    bool viewport = true;
    bool flipY = true;
    const char* passName = nullptr;
};

struct ComputePassInfo
{
    std::vector<Handle<HwDescriptorSet>> descSets;
    Handle<HwComputeProgram> programHandle;
    Ogre::Vector3i computeGroup;
};


#ifndef MAX_NUM_DIR_LIGHTS
#define MAX_NUM_DIR_LIGHTS 1
#endif


struct LightInfo
{
    Ogre::Vector3 Strength = { 0.5f, 0.5f, 0.5f };
    float FalloffStart = 1.0f;                          // point/spot light only
    Ogre::Vector3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
    float FalloffEnd = 10.0f;                           // point/spot light only
    Ogre::Vector3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
    float SpotPower = 64.0f;                            // spot light only
    Ogre::Vector3 lightColor = {1.0f, 1.0f, 1.0f};
};

struct FrameConstantBuffer
{
    Ogre::Matrix4 View = Ogre::Matrix4::IDENTITY;
    Ogre::Matrix4 InvView = Ogre::Matrix4::IDENTITY;
    Ogre::Matrix4 Proj = Ogre::Matrix4::IDENTITY;
    Ogre::Matrix4 InvProj = Ogre::Matrix4::IDENTITY;
    Ogre::Matrix4 ViewProj = Ogre::Matrix4::IDENTITY;
    Ogre::Matrix4 InvViewProj = Ogre::Matrix4::IDENTITY;
    Ogre::Matrix4 ShadowTransform = Ogre::Matrix4::IDENTITY;
    Ogre::Vector3 EyePosW = { 0.0f, 0.0f, 0.0f };
    uint32_t Shadow = 0;
    Ogre::Vector2 RenderTargetSize = { 0.0f, 0.0f };
    Ogre::Vector2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;

    LightInfo directionLights[MAX_NUM_DIR_LIGHTS];
    uint32_t numDirLights = 1;
};

struct GeneralMaterialConstantBuffer
{
    Ogre::Vector4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    Ogre::Vector3 FresnelR0 = { 0.98f, 0.97f, 0.95f };
    float Roughness = 0.1f;

    // Used in texture mapping.
    Ogre::Matrix4 TexScale = Ogre::Matrix4::IDENTITY;
    Ogre::Matrix4 TexTransform = Ogre::Matrix4::IDENTITY;
    uint32_t DiffuseMapIndex = 0;
    uint32_t MaterialPad0;
    uint32_t MaterialPad1;
    uint32_t MaterialPad2;
};


struct PbrMaterialConstanceBuffer
{
    //some constance value;
    Ogre::Vector2 metallicRoughnessValues = { 0.1f, 1.0f };
    Real occlusionStrength = 1.0f;
    uint32_t alphaMask = 0;
    Ogre::Vector3 emissiveFactor = { 1.0f, 1.0f, 1.0f };
    float alphaMaskCutoff = 0.0f;
    Ogre::Vector4 baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
    Ogre::Vector4 scaleIBLAmbient = { 1.0f, 1.0f, 1.0f, 1.0f };
    uint32_t debugRenderMode = 0;
    uint32_t hasEmissiveMap = 0;
    uint32_t hasNormalMap = 0;
    uint32_t hasMetalRoughNessMap = 0;
};

struct ObjectConstantBuffer
{
    Ogre::Matrix4 world = Ogre::Matrix4::IDENTITY;
    Ogre::Matrix4 projector = Ogre::Matrix4::IDENTITY;
    Ogre::Vector4 diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
};

class SkinnedConstantBuffer
{
public:
    Ogre::Matrix4 BoneTransforms[200];
};

struct ShaderInfo
{
    String shaderName;
    std::vector<std::pair<std::string, std::string>> shaderMacros;
};

struct RaytracingShaderInfo
{
    String rayTracingName;
    String rayGenEntryName;
    String rayMissEntryName;
    String rayClosethitEntryName;
    String rayAnyHitEntryName;
};

class SkinnedData
{
public:
    float boneWeight[4];
    uint32_t boneIndices[4];
};


enum EngineType
{
    EngineType_Dx12,
    EngineType_Vulkan,
    EngineType_Count
};
