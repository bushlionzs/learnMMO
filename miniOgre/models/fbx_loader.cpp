#include "OgreHeader.h"
#include "fbx_loader.h"
#include "OgreMesh.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreVertexDeclaration.h"
#include "shader.h"
#include "OgreMaterial.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

FbxLoader::FbxLoader()
{

}

FbxLoader::~FbxLoader()
{

}

struct FbxVertex
{
    Ogre::Vector3 Pos;
    Ogre::Vector3 Normal;
    Ogre::Vector2 TexC;
};

std::shared_ptr<Ogre::Mesh> FbxLoader::loadMeshFromFile(
    std::shared_ptr<Ogre::DataStream>& stream)
{
    Assimp::Importer importer;
    UINT flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices 
        | aiProcess_SortByPType | aiProcess_FlipWindingOrder;
    bool flipUVs = true;
    if (flipUVs)
    {
        flags |= aiProcess_FlipUVs;
    }

    const std::string& name = stream->getName();
    const aiScene* scene = importer.ReadFile(name, flags);

    assert_invariant(scene != nullptr);

    if (scene->HasMaterials())
    {
        for (UINT i = 0; i < scene->mNumMaterials; i++)
        {
            
        }
    }

    uint32_t numMaterials = scene->mNumMaterials;
    aiMaterial** sourceMaterials = scene->mMaterials;
    Ogre::Mesh* mesh = new Ogre::Mesh(name);
    if (scene->HasMeshes())
    {
        for (uint32_t i = 0; i < scene->mNumMeshes; i++)
        {
            
            aiMesh* sourceMesh = scene->mMeshes[i];
            SubMesh* subMesh = mesh->addSubMesh(false, false);

            uint32_t numVertices = sourceMesh->mNumVertices;

            std::vector<FbxVertex> vertexList;
            vertexList.resize(numVertices);

            aiVector3D* sourceVertices = sourceMesh->mVertices;
            aiVector3D* sourceNormals = sourceMesh->mNormals;
            aiVector3D* sourceTexcoords = sourceMesh->mTextureCoords[0];
            aiColor4D* sourceColors = sourceMesh->mColors[0];
            for (uint32_t index = 0; index < numVertices; index++)
            {
                FbxVertex& vertex = vertexList.at(index);
             
                vertex.Pos.x = sourceVertices[index].x;
                vertex.Pos.y = sourceVertices[index].y;
                vertex.Pos.z = sourceVertices[index].z;

                vertex.Normal.x = sourceNormals[index].x;
                vertex.Normal.y = sourceNormals[index].y;
                vertex.Normal.z = sourceNormals[index].z;

                vertex.TexC.x = sourceTexcoords[index].x;
                vertex.TexC.y = sourceTexcoords[index].y;
            }

            //update verteices
            VertexData* vertexData = subMesh->getVertexData();
            
            vertexData->addElement(0, 0, 0, VET_FLOAT3, VES_POSITION);
            vertexData->addElement(0, 0, 12, VET_FLOAT3, VES_NORMAL);
            vertexData->addElement(0, 0, 24, VET_FLOAT2, VES_TEXTURE_COORDINATES);

            vertexData->addBindBuffer(0, sizeof(FbxVertex), numVertices);

            vertexData->writeBindBufferData(0, (const  char*)vertexList.data(), sizeof(FbxVertex) * numVertices);
            //update index
            std::vector<uint32_t> indexs;

            uint32_t faces = sourceMesh->mNumFaces;
            indexs.reserve(faces * 3);
            for (uint32_t face = 0; face < faces; face++)
            {
                aiFace& sourceFace = sourceMesh->mFaces[face];

                for (uint32_t j = 0; j < sourceFace.mNumIndices; j++)
                {
                    indexs.push_back(sourceFace.mIndices[j]);
                }

                if (sourceFace.mNumIndices != 3)
                {
                    assert(false);
                }
            }

            IndexData* indexData = subMesh->getIndexData();
            indexData->createBuffer(4, indexs.size());
            indexData->writeData((const char*)indexs.data(), 4 * indexs.size());

            subMesh->addIndexs(indexs.size(), 0, 0);
            //update material
            aiMaterial* sourceMat = sourceMaterials[i];

            aiString name;
            sourceMat->Get(AI_MATKEY_NAME, name);

            std::string matName = name.C_Str();
            std::shared_ptr<Ogre::Material> mat = std::make_shared<Ogre::Material>(matName, true);

            for (aiTextureType aiType = aiTextureType_DIFFUSE; aiType < aiTextureType_UNKNOWN;
                aiType = aiTextureType((uint32_t)aiType + 1))
            {
                uint32_t textureCount = sourceMat->GetTextureCount(aiTextureType_DIFFUSE);
                if (textureCount > 0)
                {
                    int kk = 0;
                }
            }

            subMesh->setMaterial(mat);
            ShaderInfo info;
            info.shaderName = "fbx";
            mat->addShader(info);
        }
    }

    return std::shared_ptr<Ogre::Mesh>(mesh);
}