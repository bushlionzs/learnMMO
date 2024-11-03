#pragma once

#include "model_loader.h"

namespace tinygltf
{
	class Model;
	class Node;
	struct Skin;
	struct AnimationChannel;
	struct AnimationSampler;
}

class MeshSerializerListener;

class  GltfLoader : public ModelLoader
{
public:
	GltfLoader(bool binary = false);
	~GltfLoader();

	virtual std::shared_ptr<Ogre::Mesh> loadMeshFromFile(std::shared_ptr<Ogre::DataStream>& stream);

private:
	struct GltfNode
	{
		GltfNode* parent = nullptr;
		Ogre::Vector3 translation = Ogre::Vector3::ZERO;
		Ogre::Vector3 scale{ 1.0f };
		Ogre::Quaternion rotation;
	};
	void TraverseNode(
		Ogre::Mesh* mesh, 
		GltfNode* parentNode, 
		const tinygltf::Node& node,
		const tinygltf::Model& model);
	bool loadSkeleton(tinygltf::Model* pModel, std::vector<std::shared_ptr<Ogre::Skeleton>>& skeletonlist);
	bool loadBone(tinygltf::Model* pModel, tinygltf::Skin* skin, Ogre::Skeleton* skeleton);
	bool loadAnimation(
		tinygltf::Model* pModel, 
		tinygltf::Skin* skin,
		std::map<uint32_t, std::vector<tinygltf::AnimationChannel*>>& nodeToChannelMap,
		Ogre::Animation* ani);

	void parseTimeposFromSampler(
		tinygltf::Model* pModel,
		tinygltf::AnimationSampler& sampler,
		int32_t frameId, float& fValue);
	void parseVector3dFromSampler(
		tinygltf::Model* pModel,
		tinygltf::AnimationSampler& sampler,
		int32_t frameId, Ogre::Vector3& v);
	void parseQuaternionFromSampler(
		tinygltf::Model* pModel,
		tinygltf::AnimationSampler& sampler,
		int32_t frameId, Ogre::Quaternion& v);
private:
	std::unordered_map<uint32_t, Ogre::Bone*> mBoneMap;
	bool mBinary;
};