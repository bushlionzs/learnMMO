#include "OgreHeader.h"
#include "bin_loader.h"
#include "OgreMesh.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreVertexDeclaration.h"
#include "shader.h"
#include "OgreMaterial.h"

typedef enum ShaderSemantic
{
    SEMANTIC_UNDEFINED = 0,
    SEMANTIC_POSITION,
    SEMANTIC_NORMAL,
    SEMANTIC_COLOR,
    SEMANTIC_TANGENT,
    SEMANTIC_BITANGENT,
    SEMANTIC_JOINTS,
    SEMANTIC_WEIGHTS,
    SEMANTIC_CUSTOM,
    SEMANTIC_TEXCOORD0,
    SEMANTIC_TEXCOORD1,
    SEMANTIC_TEXCOORD2,
    SEMANTIC_TEXCOORD3,
    SEMANTIC_TEXCOORD4,
    SEMANTIC_TEXCOORD5,
    SEMANTIC_TEXCOORD6,
    SEMANTIC_TEXCOORD7,
    SEMANTIC_TEXCOORD8,
    SEMANTIC_TEXCOORD9,
    MAX_SEMANTICS
} ShaderSemantic;

typedef struct Meshlet
{
    /// Offsets within meshlet_vertices and meshlet_triangles arrays with meshlet data
    uint32_t vertexOffset;
    uint32_t triangleOffset;

    /// Number of vertices and triangles used in the meshlet; data is stored in consecutive range defined by offset and count
    uint32_t vertexCount;
    uint32_t triangleCount;
} Meshlet;

typedef struct MeshletData
{
    Ogre::Vector3 center;
    float  radius;

    /// Normal cone, useful for backface culling
    Ogre::Vector3 coneApex;
    Ogre::Vector3 coneAxis;
    float  coneCutoff; // = cos(angle/2)
} MeshletData;

typedef struct GeometryMeshlets
{
    uint64_t     mMeshletCount;
    Meshlet* mMeshlets;
    MeshletData* mMeshletsData;

    uint64_t  mVertexCount;
    uint32_t* mVertices;

    uint64_t mTriangleCount;
    uint8_t* mTriangles;
} GeometryMeshlets;

typedef struct IndirectDrawIndexArguments
{
    uint32_t mIndexCount;
    uint32_t mInstanceCount;
    uint32_t mStartIndex;
    uint32_t mVertexOffset;
    uint32_t mStartInstance;
} IndirectDrawIndexArguments;

typedef struct Geometry
{
    /// Index buffer to bind when drawing this geometry
    void* pIndexBuffer;
    /// The array of vertex buffers to bind when drawing this geometry
    void* pVertexBuffers[MAX_VERTEX_BINDINGS];
 

    /// The array of traditional draw arguments to draw each subset in this geometry
    IndirectDrawIndexArguments* pDrawArgs;

    /// The array of vertex buffer strides to bind when drawing this geometry
    uint32_t mVertexStrides[MAX_VERTEX_BINDINGS];

    /// Number of vertex buffers in this geometry
    uint32_t mVertexBufferCount : 8;
    /// Index type (32 or 16 bit)
    uint32_t mIndexType : 2;
    /// Number of draw args in the geometry
    uint32_t mDrawArgCount : 22;
    /// Number of indices in the geometry
    uint32_t mIndexCount;
    /// Number of vertices in the geometry
    uint32_t mVertexCount;

    // If present, data is stored in pGeometryBuffer
    void* pGeometryBuffer;

    GeometryMeshlets meshlets;

    uint32_t mPad[20];
} Geometry;

typedef struct GeometryData
{
    struct Hair
    {
        uint32_t mVertexCountPerStrand;
        uint32_t mGuideCountPerStrand;
    };

    struct ShadowData
    {
        void* pIndices;
        void* pAttributes[MAX_SEMANTICS];

        // Strides for the data in pAttributes, this might not match Geometry::mVertexStrides since those are generated based on
        // GeometryLoadDesc::pVertexLayout, e.g. if the normals are packed on the GPU as half2 then:
        //         - Geometry::mVertexStrides will be sizeof(half2)
        //         - ShadowData::mVertexStrides might be sizeof(float3) = 12 (or maybe sizeof(float4) = 16)
        // If the data readed from the file in pAttributes is already packed then ShadowData::mVertexStrides[i] ==
        // Geometry::mVertexStrides[i]
        uint32_t mVertexStrides[MAX_SEMANTICS];

        // We might have a different number of attributes than mVertexCount.
        // This happens for example for Hair
        uint32_t mAttributeCount[MAX_SEMANTICS];

        // TODO: Consider if we want to store here mIndexStride to access ShadowData::pIndices,
        //       right now it depends on the number of vertexes in the mesh (uint16_t if mVertexCount < 64k otherwise uint32_t)
    };

    /// Shadow copy of the geometry vertex and index data if requested through the load flags
    ShadowData* pShadow;

    /// The array of joint inverse bind-pose matrices ( object-space )
    void* pInverseBindPoses;
    /// The array of data to remap skin batch local joint ids to global joint ids
    uint32_t* pJointRemaps;

    /// Number of joints in the skinned geometry
    uint32_t mJointCount;

    /// Hair data
    Hair mHair;

    uint32_t mPad0[1];

    MeshletData* meshlets;

    // Custom data imported by the user in custom AssetPipelines, this can be data that was exported from a custom tool/plugin
    // specific to the engine/game. See AssetPipeline: callbacks in ProcessGLTFParams for more information.
    void* pUserData;
    uint32_t mUserDataSize;

    uint32_t mPad1[5];
} GeometryData;



struct BinVertex
{
    Ogre::Vector3 Pos;
    Ogre::Vector3 Normal;
    Ogre::Vector2 TexC;
};

BinLoader::BinLoader()
{

}

BinLoader::~BinLoader()
{

}

const char GEOMETRY_FILE_MAGIC_STR[] = { 'G', 'e', 'o', 'm', 'e', 't', 'r', 'y', 'T', 'F' };
template<typename T, size_t N>
char(*array_counter(const T(&)[N]))[N];
#ifndef TF_ARRAY_COUNT
#define TF_ARRAY_COUNT(array) (sizeof(*array_counter(array)) + 0)
#endif

VertexElementType BinMappingElementType(ShaderSemantic semantic)
{
    switch (semantic)
    {
    case SEMANTIC_POSITION:
        return VET_FLOAT3;
    case SEMANTIC_NORMAL:
        return VET_FLOAT3;
    case SEMANTIC_COLOR:
        return VET_FLOAT4;
    case SEMANTIC_TANGENT:
        return VET_FLOAT4;
    case SEMANTIC_JOINTS:
        return VET_UINT4;
    case SEMANTIC_WEIGHTS:
        return VET_FLOAT4;
    case SEMANTIC_TEXCOORD0:
        return VET_FLOAT2;
    case SEMANTIC_TEXCOORD1:
        return VET_FLOAT2;
    default:
        assert(false);
        break;
    }
    return VET_FLOAT3;
}

VertexElementSemantic BinMappingElementSemantic(ShaderSemantic semantic)
{
    switch (semantic)
    {
    case SEMANTIC_POSITION:
        return VES_POSITION;
    case SEMANTIC_NORMAL:
        return VES_NORMAL;
    case SEMANTIC_COLOR:
        return VES_COLOUR;
    case SEMANTIC_TANGENT:
        return VES_TANGENT;
    case SEMANTIC_JOINTS:
        return VES_BLEND_INDICES;
    case SEMANTIC_WEIGHTS:
        return VES_BLEND_WEIGHTS;
    case SEMANTIC_TEXCOORD0:
        return VES_TEXTURE_COORDINATES;
    case SEMANTIC_TEXCOORD1:
        return VES_TEXTURE_COORDINATES;
    default:
        assert(false);
        break;
    }
    return VES_POSITION;
}

inline Ogre::Vector2 octWrap_float2(Ogre::Vector2 v)
{
    return (Ogre::Vector2(1.f, 1.f) - Ogre::Vector2(abs(v.y), abs(v.x))) * Ogre::Vector2(v.x >= 0.f ? 1.f : -1.f, v.y >= 0.f ? 1.f : -1.f);
}

Ogre::Vector3 decodeDir(Ogre::Vector2& encN)
{
    encN = encN * 2.0f - Ogre::Vector2(1.0f, 1.f);

    Ogre::Vector3 n;
    n.z = 1.0f - abs(encN.x) - abs(encN.y);

    if (n.z >= 0.0f)
    {
        n.x = encN.x;
        n.y = encN.y;
    }
    else
    {
        Ogre::Vector2 wrap = octWrap_float2(Ogre::Vector2(encN.x, encN.y));
        n.x = wrap.x;
        n.y = wrap.y;
    }
    n /= sqrt(n.x * n.x + n.y * n.y + n.z * n.z);

    return n;
}

void unpackNormal(uint32_t packedValue, Ogre::Vector3& normal)
{
    uint32_t mask = 0xFFFF;
    uint32_t value1 = packedValue & mask;
    uint32_t value2 = (packedValue >> 16) & mask;

    Ogre::Vector2 tmp;
    tmp.x = static_cast<float>(value1) / static_cast<float>(mask);
    tmp.y = static_cast<float>(value2) / static_cast<float>(mask);

    normal = decodeDir(tmp);
}

float unpackHalf1x16(uint16_t value) {
    const uint16_t signMask = 0x8000;
    const uint16_t exponentMask = 0x7C00;
    const uint16_t mantissaMask = 0x03FF;

    int16_t sign = (value & signMask) >> 15;
    int16_t exponent = (value & exponentMask) >> 10;
    int16_t mantissa = value & mantissaMask;

    float result;

    if (exponent == 0) {
        if (mantissa == 0) {
            result = (sign == 0) ? 0.0f : -0.0f;
        }
        else {
            result = std::pow(2, -14) * (sign == 0 ? 1 : -1) * (mantissa / 1024.0f);
        }
    }
    else if (exponent == 31) {
        result = (mantissa == 0) ? (sign == 0 ? std::numeric_limits<float>::infinity() : -std::numeric_limits<float>::infinity()) : std::numeric_limits<float>::quiet_NaN();
    }
    else {
        result = std::pow(2, exponent - 15) * (1.0f + (mantissa / 1024.0f)) * (sign == 0 ? 1 : -1);
    }

    return result;
}

void unpackHalf2x16(uint32_t packed, Ogre::Vector2& f2) {
    const uint32_t mask = 0x0000FFFF;

    uint16_t unpacked1 = packed & mask;
    uint16_t unpacked2 = (packed >> 16) & mask;

    f2.x = unpackHalf1x16(unpacked1);
    f2.y = unpackHalf1x16(unpacked2);
}



std::shared_ptr<Ogre::Mesh> BinLoader::loadMeshFromFile(std::shared_ptr<Ogre::DataStream>& stream)
{
    uint32_t size = stream->getStreamLength();

    const char* data = stream->getStreamData();

    char magic[TF_ARRAY_COUNT(GEOMETRY_FILE_MAGIC_STR)] = { 0 };

    stream->read(magic, sizeof(magic));

    if (strncmp(magic, GEOMETRY_FILE_MAGIC_STR, TF_ARRAY_COUNT(magic)) != 0)
    {
        return std::shared_ptr<Mesh>();
    }

    uint32_t geomSize = 0;
    stream->read(&geomSize, sizeof(uint32_t));

    if (geomSize < 352)
    {
        return std::shared_ptr<Mesh>();
    }
    std::shared_ptr<Ogre::Mesh> mesh = std::make_shared<Ogre::Mesh>(stream->getName());

    Geometry* geom = (Geometry*)malloc(geomSize);

    stream->read(geom, geomSize);

    uint32_t geomDataSize = 0;
    stream->read(&geomDataSize, sizeof(uint32_t));

    GeometryData* geomData = (GeometryData*)malloc(geomDataSize);
    stream->read(geomData, geomDataSize);
    uint32_t shadowSize = 0;
    stream->read(&shadowSize, sizeof(uint32_t));

    geomData->pShadow = (GeometryData::ShadowData*)malloc(shadowSize);
    stream->read(geomData->pShadow, shadowSize);
    const uint32_t indexStride = geom->mVertexCount > UINT16_MAX ? sizeof(uint32_t) : sizeof(uint16_t);

    geomData->pShadow->pIndices = geomData->pShadow + 1;

    geomData->pShadow->pAttributes[SEMANTIC_POSITION] = (uint8_t*)geomData->pShadow->pIndices + (geom->mIndexCount * indexStride);

    for (uint32_t s = SEMANTIC_POSITION + 1; s < MAX_SEMANTICS; ++s)
        geomData->pShadow->pAttributes[s] = (uint8_t*)geomData->pShadow->pAttributes[s - 1] +
        geomData->pShadow->mVertexStrides[s - 1] * geomData->pShadow->mAttributeCount[s - 1];

    for (uint32_t i = 0; i < TF_ARRAY_COUNT(geomData->pShadow->mVertexStrides); ++i)
    {
        if (geomData->pShadow->mVertexStrides[i] == 0)
            geomData->pShadow->pAttributes[i] = nullptr;
    }

    uint32_t vertexAttribCount[MAX_SEMANTICS] = {};
    uint32_t vertexOffsets[MAX_SEMANTICS] = {}; // offset in the GPU layout
    uint32_t vertexBindings[MAX_SEMANTICS] = {};
    for (uint32_t i = 0; i < TF_ARRAY_COUNT(vertexOffsets); ++i)
        vertexOffsets[i] = UINT_MAX;

    uint32_t defaultTexcoordSemantic = SEMANTIC_UNDEFINED;
    geom->pDrawArgs = (IndirectDrawIndexArguments*)(geom + 1);
    for (uint32_t i = 0; i < geom->mDrawArgCount; i++)
    {
        auto subMesh = mesh->addSubMesh(true, true);

        IndirectDrawIndexArguments& arg = geom->pDrawArgs[i];
        subMesh->addIndexs(arg.mIndexCount, arg.mStartIndex, arg.mVertexOffset);
        std::string matName = "Material_" + stream->getName() + std::to_string(i);
        std::shared_ptr<Material> mat = std::make_shared<Material>(matName, false);

        ShaderInfo sinfo;
        sinfo.shaderName = "basic";
        mat->addShader(sinfo);
        subMesh->setMaterial(mat);
    }
    

    
    
    IndexData* indexData = mesh->getIndexData();
    VertexData* vertexData = mesh->getVertexData();
    vertexData->setVertexCount(geom->mVertexCount);
    indexData->createBuffer(indexStride, geom->mIndexCount);
    indexData->writeData((const char*)geomData->pShadow->pIndices, indexStride * geom->mIndexCount);
    uint32_t source = 0;

    vertexData->addElement(0, 0, 0, VET_FLOAT3, VES_POSITION);
    vertexData->addElement(0, 0, 12, VET_FLOAT3, VES_NORMAL);
    vertexData->addElement(0, 0, 24, VET_FLOAT2, VES_TEXTURE_COORDINATES);
   

    uint32_t binding = 0;
    vertexData->addBindBuffer(binding, sizeof(BinVertex), geom->mVertexCount);

    std::vector<BinVertex> vertexList(geom->mVertexCount);
    BinVertex* vdata = (BinVertex*)vertexList.data();

    float* position = (float*)geomData->pShadow->pAttributes[SEMANTIC_POSITION];
    uint32_t* normal = (uint32_t*)geomData->pShadow->pAttributes[SEMANTIC_NORMAL];
    uint32_t* tex = (uint32_t*)geomData->pShadow->pAttributes[SEMANTIC_TEXCOORD0];
    for (int32_t i = 0; i < geom->mVertexCount; i++)
    {
        vdata[i].Pos.x = position[0];
        vdata[i].Pos.y = position[1];
        vdata[i].Pos.z = position[2];
        position += 3;
        unpackNormal(normal[i], vdata[i].Normal);
        unpackHalf2x16(tex[i], vdata[i].TexC);
    }

    vertexData->writeBindBufferData(binding, (const  char*)vdata, sizeof(BinVertex) * geom->mVertexCount);


    AxisAlignedBox box;
    box.setInfinite();
    mesh->_setBounds(box);
    return mesh;
}