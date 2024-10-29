#pragma once
#include <OgreCommon.h>
#define STRUCT(NAME) struct NAME
#define DATA(TYPE, NAME, SEM) TYPE NAME
STRUCT(VBConstants)
{
	DATA(uint32_t, indexOffset, None);
	DATA(uint32_t, pad_0, None);
	DATA(uint32_t, pad_1, None);
	DATA(uint32_t, pad_2, None);
};

STRUCT(Transform)
{
	DATA(Ogre::Matrix4, vp, None);
	DATA(Ogre::Matrix4, mvp, None);
	DATA(Ogre::Matrix4, projection, None);
	DATA(Ogre::Matrix4, invVP, None);
};

STRUCT(CullingViewPort)
{
	DATA(Ogre::Vector2, windowSize, None);
	DATA(uint32_t, sampleCount, None);
	DATA(uint32_t, _pad0, None);
};

STRUCT(PerFrameVBConstants)
{
	DATA(Transform, transform[5], None);
	DATA(CullingViewPort, cullingViewports[5], None);
	DATA(uint32_t, numViewports, None);
};

STRUCT(MeshInfoUniformBlock)
{
	DATA(Ogre::Matrix4, worldViewProjMat, None);
	DATA(uint32_t, viewID, None);
};

STRUCT(FilterDispatchGroupData)
{
	DATA(uint32_t, meshIndex, None);
	DATA(uint32_t, indexOffset, None);
	DATA(uint32_t, instanceDataIndex, None);
	DATA(uint32_t, geometrySet_faceCount, None);
};

STRUCT(MeshConstants)
{
	DATA(uint32_t, indexOffset, None);
	DATA(uint32_t, vertexOffset, None);
	DATA(uint32_t, materialID, None);
	DATA(uint32_t, twoSided, None);
};

STRUCT(VertexDataBuffer)
{
	DATA(Ogre::Vector3, vertexPosition, None);
	DATA(Ogre::Vector2, vertexTextureUV, None);
};

enum MaterialFlagBits
{
	MATERIAL_FLAG_NONE = 0,
	MATERIAL_FLAG_TWO_SIDED = (1 << 0),
	MATERIAL_FLAG_ALPHA_TESTED = (1 << 1),
	MATERIAL_FLAG_TRANSPARENT = (1 << 2),
	MATERIAL_FLAG_DOUBLE_VOXEL_SIZE = (1 << 3),
	MATERIAL_FLAG_ALL = MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE
};

typedef struct LightUniformBlock
{
	Ogre::Matrix4 mLightViewProj;
	Ogre::Vector4 mLightPosition;
	Ogre::Vector4 mLightColor = { 1, 0, 0, 1 };
	Ogre::Vector4 mLightUpVec;
	Ogre::Vector4 mTanLightAngleAndThresholdValue;
	Ogre::Vector3 mLightDir;
} LightUniformBlock;

typedef struct CameraUniform
{
	Ogre::Matrix4         mView;
	Ogre::Matrix4         mProject;
	Ogre::Matrix4         mViewProject;
	Ogre::Matrix4         mInvView;
	Ogre::Matrix4         mInvProj;
	Ogre::Matrix4         mInvViewProject;
	Ogre::Vector4         mCameraPos;
	float                 mNear;
	float                 mFar;
	float                 mFarNearDiff;
	float                 mFarNear;
	Ogre::Vector2         mTwoOverRes;
	Ogre::Vector2         mWindowSize;
	Ogre::Vector4         mDeviceZToWorldZ;
} CameraUniform;

enum ShadowType
{
	SHADOW_TYPE_ESM,            // Exponential Shadow Map
	SHADOW_TYPE_ASM,            // Adaptive Shadow Map, has Parallax Corrected Cache algorithm that approximate moving sun's shadow
	SHADOW_TYPE_MESH_BAKED_SDF, // Signed Distance field shadow for mesh using generated baked data
	SHADOW_TYPE_VSM,            // Variance Shadow Map
	SHADOW_TYPE_MSM,            // Moments Shadow Map
	SHADOW_TYPE_COUNT
};
typedef struct RenderSettingsUniformData
{
	Ogre::Vector4     mWindowDimension = { 1, 1, 0, 0 }; // only first two are used to represents window width and height, z and w are paddings
	uint32_t mShadowType = SHADOW_TYPE_ESM;
} RenderSettingsUniformData;

typedef struct ESMInputConstants
{
	float mEsmControl = 80.f;
} ESMInputConstants;


typedef struct MeshInfoStruct
{
	Ogre::Vector4   mColor;
	Ogre::Vector3   mTranslation;
	Ogre::Vector3   mOffsetTranslation;
	Ogre::Vector3   mScale;
	Ogre::Matrix4   mTranslationMat;
	Ogre::Matrix4   mScaleMat;
	Ogre::Matrix4   mWorldMat;
} MeshInfoStruct;

struct LightCpuSettings
{
	float  mSourceAngle = 1.0f;
	// only used for ESM shadow
	Ogre::Vector2 mSunControl = { -2.1f, -0.961f };
	float  mSunSpeedY = 0.025f;
	// only for SDF shadow now
	bool   mAutomaticSunMovement = false;
} ;

#define VIEW_CAMERA          0
#define VIEW_SHADOW          1
std::shared_ptr<Mesh> loadSanMiguel(const std::string& name);

#define INDIRECT_DRAW_ARGUMENTS_ELEMENTS 8
#define SAN_MIGUEL_OFFSETX          150.f
#define MESH_SCALE                  10.f
#define GET_INDIRECT_DRAW_ELEM_INDEX(VIEW, GEOM, ELEM) (((((VIEW) * NUM_GEOMETRY_SETS) + (GEOM)) * INDIRECT_DRAW_ARGUMENTS_ELEMENTS) + (ELEM))