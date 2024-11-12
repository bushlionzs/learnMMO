#include "stdafx.h"
#include <string_util.h>
#include "PbrMaterial.h"
#include "engine_manager.h"
#include "OgreParticleSystem.h"
#include "myutils.h"
#include "OgreResourceManager.h"
#include "renderSystem.h"
#include "OgreViewport.h"
#include "OgreRenderWindow.h"
#include "OgreTextureManager.h"
#include "OgreMaterialManager.h"
#include "OgreRenderTexture.h"
#include "OgreCamera.h"
#include "OgreSubEntity.h"
#include "pbrWindow.h"
#include "OgreRoot.h"
#include "OgreEntity.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreMeshManager.h"
#include "OgreMaterial.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "game_camera.h"
#include <platform_file.h>
#include <OgreRoot.h>
#include <algorithm>


#define ENTRY_INSTANCE_COUNT 6

PbrMaterial::PbrMaterial()
{
   

}

PbrMaterial::~PbrMaterial()
{

}

std::vector<std::string> matNameList =
{
	"Aluminum",
	"ScratchedGold",
	"Copper",
	"TiledMetal",
	"OldIron",
	"Bronze",
	"WoodenPlank05",
	"WoodenPlank06",
	"Wood03",
	"Wood08",
	"Wood16",
	"Wood18",
	"ForgeGround",
	"ForgePlate",
};

void PbrMaterial::setup(
	RenderPipeline* renderPipeline,
	RenderSystem* rs,
	Ogre::RenderWindow* renderWindow,
	Ogre::SceneManager* sceneManager,
	GameCamera* gameCamera)
{
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	ogreConfig.reverseDepth = true;
	uiInit();
	if (1)
	{
		example1(renderPipeline, rs, renderWindow, sceneManager, gameCamera);
	}
	else
	{
		example2(renderPipeline, rs, renderWindow, sceneManager, gameCamera);
	}
	
}

void PbrMaterial::example1(RenderPipeline* renderPipeline,
	RenderSystem* rs,
	Ogre::RenderWindow* renderWindow,
	Ogre::SceneManager* sceneManager,
	GameCamera* gameCamera)
{
	float baseX = 22.0f;
	float baseY = -1.8f;
	float baseZ = 12.0f;
	float offsetX = 8.0f;
	float scaleVal = 4.0f;
	float roughDelta = 1.0f;
	float materialPlateOffset = 4.0f;
	SceneNode* root = sceneManager->getRoot()->createChildSceneNode("root");
	std::string cubeName = "cube.bin";
	auto cube = MeshManager::getSingletonPtr()->load(cubeName);

	std::string matBallName = "matBall.bin";
	auto matBall = MeshManager::getSingletonPtr()->load(matBallName);
	matBallList.resize(ENTRY_INSTANCE_COUNT);
	for (uint32_t i = 0; i < ENTRY_INSTANCE_COUNT; ++i)
	{
		std::string entryName = "matBall_" + std::to_string(i);
		Entity* entity = sceneManager->createEntity(entryName, matBallName);

		entity->setMaterialName(matNameList[i], false);
		auto* r = entity->getSubEntity(0);
		Ogre::Vector3 pos = Ogre::Vector3(baseX - i - offsetX * i, baseY, baseZ);
		r->setLocalMatrix(pos,
			Ogre::Vector3(4.0),
			Ogre::Quaternion(Ogre::Radian(Ogre::Math::PI), Ogre::Vector3::UNIT_Y));

		SceneNode* entityNode = root->createChildSceneNode(entryName);
		entityNode->attachObject(entity);

		matBallList[i] = entity;
	}

	std::vector<Ogre::Entity*> cubeList;
	cubeList.resize(ENTRY_INSTANCE_COUNT);
	for (uint32_t i = 0; i < ENTRY_INSTANCE_COUNT; ++i)
	{
		std::string entryName = "cube_" + std::to_string(i);
		Entity* entity = sceneManager->createEntity(entryName, cubeName);

		entity->setMaterialName("ForgePlate", false);
		auto* r = entity->getSubEntity(0);
		r->setLocalMatrix(Ogre::Vector3(baseX - i - offsetX * i, -5.8f, baseZ + materialPlateOffset),
			Ogre::Vector3(3.0f, 0.1f, 1.0f),
			Ogre::Quaternion(Ogre::Radian(Ogre::Math::PI / 5.0f), Ogre::Vector3::UNIT_X));

		SceneNode* entityNode = root->createChildSceneNode(entryName);
		entityNode->attachObject(entity);

		cubeList[i] = entity;
	}

	{
		std::string entryName = "cube_ground";
		Entity* entity = sceneManager->createEntity(entryName, cubeName);

		entity->setMaterialName("ForgeGround", false);
		auto* r = entity->getSubEntity(0);
		r->setLocalMatrix(Ogre::Vector3(0.0f, -6.0f, 5.0f),
			Ogre::Vector3(30.0f, 0.2f, 30.0f),
			Ogre::Quaternion::IDENTITY);

		SceneNode* entityNode = root->createChildSceneNode(entryName);
		entityNode->attachObject(entity);
	}



	std::vector<Ogre::MaterialPtr> matList;
	for (auto& name : matNameList)
	{
		auto mat = MaterialManager::getSingleton().getByName(name);
		matList.push_back(mat);
	}

	TextureProperty tp;
	tp._need_mipmap = false;
	tp._texType = TEX_TYPE_CUBE_MAP;
	tp._tex_addr_mod = TAM_CLAMP;
	tp._samplerParams.filterMag = filament::backend::SamplerFilterType::LINEAR;
	tp._samplerParams.filterMin = filament::backend::SamplerFilterType::LINEAR;
	tp._samplerParams.mipMapMode = filament::backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR;
	tp._samplerParams.wrapS = filament::backend::SamplerWrapMode::REPEAT;
	tp._samplerParams.wrapT = filament::backend::SamplerWrapMode::REPEAT;
	tp._samplerParams.wrapR = filament::backend::SamplerWrapMode::REPEAT;
	tp._samplerParams.anisotropyLog2 = 0;
	auto environmentCube = TextureManager::getSingletonPtr()->load("LA_Helipad3D.dds", &tp, true).get();

	{
		std::string prefilteredenvName = "prefilteredMap";
		prefilteredMap = rs->generateCubeMap(prefilteredenvName, environmentCube, PF_FLOAT32_RGBA, 128, CubeType_Prefiltered);
		TextureManager::getSingleton().addTexture(prefilteredenvName, prefilteredMap);
		tp._pbrType = TextureTypePbr_IBL_Specular;

		std::for_each(matList.begin(), matList.end(),
			[&prefilteredenvName, &tp](Ogre::MaterialPtr& mat)
			{
				mat->addTexture(prefilteredenvName, &tp);
			});
	}

	{
		std::string irradianceName = "IrradianceMap";
		irradianceMap = rs->generateCubeMap(irradianceName, environmentCube, PF_FLOAT32_RGBA, 32, CubeType_Irradiance);
		TextureManager::getSingleton().addTexture(irradianceName, irradianceMap);
		tp._pbrType = TextureTypePbr_IBL_Diffuse;
		std::for_each(matList.begin(), matList.end(),
			[&irradianceName, &tp](Ogre::MaterialPtr mat)
			{
				mat->addTexture(irradianceName, &tp);
			});
	}

	{
		std::string brdfLutName = "brdflut";
		brdf = rs->generateBRDFLUT(brdfLutName);
		TextureManager::getSingleton().addTexture(brdfLutName, brdf);
		tp._pbrType = TextureTypePbr_BRDF_LUT;

		std::for_each(matList.begin(), matList.end(),
			[&brdfLutName, &tp](Ogre::MaterialPtr mat)
			{
				mat->addTexture(brdfLutName, &tp);
			});
	}

	//sceneManager->setSkyBox(true, "SkyMap", 5000);

	Ogre::Vector3 camPos = Ogre::Vector3(0, 10.0f, 60.0f);
	Ogre::Vector3 lookAt = Ogre::Vector3::ZERO;
	gameCamera->lookAt(camPos, lookAt);
	Ogre::Matrix4 m;

	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	if (ogreConfig.reverseDepth)
	{
		float aspectInverse = ogreConfig.height / (float)ogreConfig.width;
		m = Ogre::Math::makePerspectiveMatrixReverseZ(
			Ogre::Math::PI / 3.0f, aspectInverse, 0.1, 6000);
	}
	else
	{
		float aspect = ogreConfig.width / (float)ogreConfig.height;
		m = Ogre::Math::makePerspectiveMatrix(
			Ogre::Math::PI / 3.0f, aspect, 0.1, 6000);
	}

	gameCamera->getCamera()->updateProjectMatrix(m);
	gameCamera->setCameraType(CameraMoveType_FirstPerson);
	gameCamera->setMoveSpeed(20);

	RenderPassInput input;
	input.color = renderWindow->getColorTarget();
	input.depth = renderWindow->getDepthTarget();
	input.cam = gameCamera->getCamera();
	input.sceneMgr = sceneManager;
	auto mainPass = createStandardRenderPass(input);
	renderPipeline->addRenderPass(mainPass);
}

void PbrMaterial::example2(RenderPipeline* renderPipeline,
	RenderSystem* rs,
	Ogre::RenderWindow* renderWindow,
	Ogre::SceneManager* sceneManager,
	GameCamera* gameCamera)
{
	example_type = 2;
	std::string name = "FlightHelmet.gltf";
	auto mesh = MeshManager::getSingletonPtr()->load(name);

	SceneNode* root = sceneManager->getRoot()->createChildSceneNode("root");

    gltfEntity = sceneManager->createEntity("FlightHelmet", name);
	SceneNode* gltfNode = root->createChildSceneNode("FlightHelmet");

	gltfNode->attachObject(gltfEntity);


	//sceneManager->setSkyBox(true, "SkyLan", 2000);

	TextureProperty tp;
	tp._need_mipmap = false;
	tp._texType = TEX_TYPE_CUBE_MAP;
	tp._tex_addr_mod = TAM_CLAMP;
	tp._samplerParams.filterMag = filament::backend::SamplerFilterType::LINEAR;
	tp._samplerParams.filterMin = filament::backend::SamplerFilterType::LINEAR;
	tp._samplerParams.mipMapMode = filament::backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR;
	tp._samplerParams.wrapS = filament::backend::SamplerWrapMode::REPEAT;
	tp._samplerParams.wrapT = filament::backend::SamplerWrapMode::REPEAT;
	tp._samplerParams.wrapR = filament::backend::SamplerWrapMode::REPEAT;
	tp._samplerParams.anisotropyLog2 = 0;
	auto environmentCube = TextureManager::getSingletonPtr()->load("papermill.ktx", &tp, true).get();
	auto count = gltfEntity->getNumSubEntities();
	std::vector<Ogre::Material*> matList;
	for (auto i = 0; i < count; i++)
	{
		SubEntity* subEntity = gltfEntity->getSubEntity(i);
		auto& mat = subEntity->getMaterial();
		matList.push_back(mat.get());
		ShaderInfo& shaderInfo = mat->getShaderInfo();
		shaderInfo.shaderMacros.push_back(std::pair<std::string, std::string>("USE_IBL", "1"));
	}
	{
		std::string prefilteredenvName = "prefilteredMap";
		prefilteredMap = rs->generateCubeMap(prefilteredenvName, environmentCube, PF_FLOAT32_RGBA, 512, CubeType_Prefiltered);
		TextureManager::getSingleton().addTexture(prefilteredenvName, prefilteredMap);
		tp._pbrType = TextureTypePbr_IBL_Specular;
		std::for_each(matList.begin(), matList.end(),
			[&prefilteredenvName, &tp](Ogre::Material* mat)
			{
				mat->addTexture(prefilteredenvName, &tp);
			});
	}

	{
		std::string irradianceName = "IrradianceMap";
		irradianceMap = rs->generateCubeMap(irradianceName, environmentCube, PF_FLOAT32_RGBA, 64, CubeType_Irradiance);
		TextureManager::getSingleton().addTexture(irradianceName, irradianceMap);
		tp._pbrType = TextureTypePbr_IBL_Diffuse;
		std::for_each(matList.begin(), matList.end(),
			[&irradianceName, &tp](Ogre::Material* mat)
			{
				mat->addTexture(irradianceName, &tp);
			});
	}

	{
		std::string brdfLutName = "brdflut";
		brdf = rs->generateBRDFLUT(brdfLutName);
		TextureManager::getSingleton().addTexture(brdfLutName, brdf);
		tp._pbrType = TextureTypePbr_BRDF_LUT;

		std::for_each(matList.begin(), matList.end(),
			[&brdfLutName, &tp](Ogre::Material* mat)
			{
				mat->addTexture(brdfLutName, &tp);
			});
	}
	Ogre::Vector3 camPos = Ogre::Vector3(0, 0, 1);
	Ogre::Vector3 lookAt = Ogre::Vector3::ZERO;

	
	gameCamera->lookAt(camPos, lookAt);
	gameCamera->setMoveSpeed(1);
	gameCamera->setRotateSpeed(1.5);

	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	Ogre::Matrix4 projectMatrix;
	if (ogreConfig.reverseDepth)
	{
		float aspectInverse = ogreConfig.height / (float)ogreConfig.width;
		projectMatrix = Ogre::Math::makePerspectiveMatrixReverseZ(
			Ogre::Math::PI / 2.0f, aspectInverse, 0.1, 5000);
	}
	else
	{
		float aspect = ogreConfig.width / (float)ogreConfig.height;
		projectMatrix = Ogre::Math::makePerspectiveMatrix(
			Ogre::Math::PI / 4.0f, aspect, 0.01, 5000);
	}

	gameCamera->getCamera()->updateProjectMatrix(projectMatrix);
	gameCamera->setCameraType(CameraMoveType_FirstPerson);
	gameCamera->setCameraType(CameraMoveType_LookAt);
	RenderPassInput input;
	input.color = renderWindow->getColorTarget();
	input.depth = renderWindow->getDepthTarget();
	input.cam = gameCamera->getCamera();
	input.sceneMgr = sceneManager;
	auto mainPass = createStandardRenderPass(input);
	renderPipeline->addRenderPass(mainPass);
}

void PbrMaterial::update(float delta)
{

}

void PbrMaterial::uiInit()
{
	PBRWindow* window = new PBRWindow(this);
}

void PbrMaterial::updateRenderMode(uint32_t mode) 
{
	if (example_type == 1)
	{
		for (auto& name : matNameList)
		{
			auto mat = MaterialManager::getSingleton().getByName(name);
			PbrMaterialConstanceBuffer& matInfo = mat->getPbrMatInfo();
			matInfo.debugRenderMode = mode;
		}

		for (auto entity : matBallList)
		{
			SubEntity* subEntity = entity->getSubEntity(0);
			subEntity->updateMaterialInfo(false);
		}
	}
	else
	{
		auto count = gltfEntity->getNumSubEntities();
		for (auto i = 0; i < count; i++)
		{
			SubEntity* subEntity = gltfEntity->getSubEntity(i);
			auto& mat = subEntity->getMaterial();
			PbrMaterialConstanceBuffer& matInfo = mat->getPbrMatInfo();
			matInfo.debugRenderMode = mode;
			subEntity->updateMaterialInfo(false);
		}
	}
}

void PbrMaterial::updateMaterialType(uint32_t type)
{
	if (example_type != 1)
		return;
	for (auto i = 0; i < matBallList.size(); i++)
	{
		std::string& name = matNameList[i + type * ENTRY_INSTANCE_COUNT];
		matBallList[i]->setMaterialName(name, false);
	}

	for (auto entity : matBallList)
	{
		SubEntity* subEntity = entity->getSubEntity(0);
		subEntity->updateMaterialInfo(true);
	}
}

void PbrMaterial::updateLightDirection(const Ogre::Vector3& dir)
{
	//assert(false);
}

