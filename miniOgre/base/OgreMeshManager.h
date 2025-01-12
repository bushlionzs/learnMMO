#pragma once

#include "OgreSingleton.h"
#include "OgreMesh.h"
#include "OgreSceneNode.h"

class MaterialInfo;
namespace Ogre {
    class MeshManager : public Ogre::Singleton<MeshManager>
    {
    public:
        MeshManager();
        ~MeshManager();
        std::shared_ptr<Mesh> load(const std::string& name);
        std::shared_ptr<Mesh> createManual(const std::string& name);
        void remove(const std::string& name);
        std::shared_ptr<Mesh> getByName(const std::string& name) const;
        bool addMesh(const std::string& name, std::shared_ptr<Mesh>& mesh);
        std::shared_ptr<Mesh> createBox(
            const std::string& name, 
            int32_t edge_size,
            const std::string& matName);
        std::shared_ptr<Mesh> createRect(
            filament::Engine* engine,
            const std::string& name,
            Ogre::Vector3& leftop,
            Ogre::Vector3& leftbottom,
            Ogre::Vector3& righttop,
            Ogre::Vector3& rightbottom,
            Ogre::Vector3& normal);
        std::shared_ptr<Mesh> createGround(
            const std::string& name,
            float width,
            float height,
            uint32_t m,
            uint32_t n,
            Ogre::Vector3& lefttop);

        std::shared_ptr<Mesh> createGrid(
            const std::string& name,
            float width,
            float height,
            uint32_t m,
            uint32_t n,
            bool change = true);
        std::shared_ptr<Mesh> CreateSphere(
            const std::string& name,
            float radius, uint32_t sliceCount, uint32_t stackCount);
        std::shared_ptr<Mesh> CreateSphereSky(
            const std::string& name,
            float radius, uint32_t sliceCount, uint32_t stackCount);
        std::shared_ptr<Mesh> CreateCylinder(const std::string& name, float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);
        std::shared_ptr<Mesh> CreateSkull(const std::string& name);
        std::shared_ptr<Mesh> createSimpleRoom(const std::string& name);
    private:
        void BuildCylinderTopCap(
            float bottomRadius,
            float topRadius,
            float height,
            uint32 sliceCount,
            uint32 stackCount,
            std::vector<SVertexElement>& vertices,
            std::vector<uint16_t>& indices);
        void BuildCylinderBottomCap(
            float bottomRadius,
            float topRadius,
            float height,
            uint32 sliceCount,
            uint32 stackCount,
            std::vector<SVertexElement>& vertices,
            std::vector<uint16_t>& indices);

        Mesh* BuildHardBuffer(
            std::vector<SVertexElement>& vertices,
            std::vector<uint16_t>& indices);

        void applyMesh(Mesh* pMesh, MaterialInfo& matInfo);

    private:
        std::unordered_map<std::string, std::shared_ptr<Mesh>> mMeshMap;
    };
}