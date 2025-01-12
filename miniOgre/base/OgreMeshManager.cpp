#include "OgreHeader.h"
#include "OgreMeshManager.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreHardwareBufferManager.h"
#include "OgreSceneNode.h"
#include "myutils.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreVertexDeclaration.h"
#include "OgreMaterialManager.h"
#include "OgreResourceManager.h"
#include "renderSystem.h"
#include "m3d_loader.h"




namespace Ogre {

template<> MeshManager* Ogre::Singleton<MeshManager>::msSingleton = 0;
struct MeshVertex
{
	Ogre::Vector3 Pos;
	Ogre::Vector3 Normal;
	Ogre::Vector3 Tex;
};
MeshManager::MeshManager()
{
}

MeshManager::~MeshManager()
{

}

std::shared_ptr<Mesh> MeshManager::load(const std::string& name)
{
	auto it = mMeshMap.find(name);
	if (it != mMeshMap.end())
	{
		return it->second;
	}
			
	std::shared_ptr<Mesh> mesh = ResourceManager::getSingletonPtr()->loadMeshFromFile(name);
	
	if (mesh)
	{
		mMeshMap[name] = mesh;
		return mesh;
	}

	return mesh;
}

std::shared_ptr<Mesh> MeshManager::createManual(const std::string& name)
{
	auto it = mMeshMap.find(name);
	if (it != mMeshMap.end())
	{
		return it->second;
	}

	std::shared_ptr<Mesh> m = std::make_shared<Mesh>(name);
	mMeshMap[name] = m;
	return m;
}

void MeshManager::remove(const std::string& name)
{

}

std::shared_ptr<Mesh> MeshManager::getByName(const std::string& name) const
{
    auto it = mMeshMap.find(name);
    if (it != mMeshMap.end())
    {
        return it->second;
    }

    return std::shared_ptr<Mesh>();
}

bool MeshManager::addMesh(const std::string& name, std::shared_ptr<Mesh>& mesh)
{
	auto it = mMeshMap.find(name);

	if (it != mMeshMap.end())
	{
		return false;
	}

	mMeshMap[name] = mesh;
	return true;
}

std::shared_ptr<Mesh> MeshManager::createBox(
	const std::string& name, 
	int32_t edge_size,
	const std::string& matName)
{
    auto it = mMeshMap.find(name);
    if (it != mMeshMap.end())
    {
        return it->second;
    }

	float dir = 1.0f;

	int width = edge_size;
	int height = edge_size;
	int depth = edge_size;
	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;
	std::vector<SVertexElement> vertices;
	// Fill in the front face vertex data
	vertices.push_back(SVertexElement(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f));
	vertices.push_back(SVertexElement( -w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f ));
	vertices.push_back(SVertexElement(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f,  0.0f, 0.0f));
	vertices.push_back(SVertexElement(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f,  1.0f, 0.0f));
	

	// Fill in the back face vertex data.
	vertices.push_back(SVertexElement(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f));
	vertices.push_back(SVertexElement(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f));
	vertices.push_back(SVertexElement(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
	vertices.push_back(SVertexElement(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f,  1.0f, 0.0f));

	// Fill in the top face vertex data.
	vertices.push_back(SVertexElement(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f));
	vertices.push_back(SVertexElement(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f));
	vertices.push_back(SVertexElement(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f,  1.0f, 0.0f));
	vertices.push_back(SVertexElement(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f,  1.0f, 1.0f));


	// Fill in the bottom face vertex data
	vertices.push_back(SVertexElement(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f,  1.0f, 1.0f));
	vertices.push_back(SVertexElement(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f,  0.0f, 1.0f));
	vertices.push_back(SVertexElement(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f,  0.0f, 0.0f));
	vertices.push_back(SVertexElement(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f,  1.0f, 0.0f));

	// Fill in the left face vertex data
	vertices.push_back(SVertexElement(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
	vertices.push_back(SVertexElement(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
	vertices.push_back(SVertexElement(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
	vertices.push_back(SVertexElement(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f ));
	
	// Fill in the right face vertex data
	vertices.push_back(SVertexElement(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f));
	vertices.push_back(SVertexElement(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
	vertices.push_back(SVertexElement(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
	vertices.push_back(SVertexElement(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f,  1.0f, 1.0f));


	std::vector<uint16_t> indices =
	{
		0, 1, 2, 0, 2, 3, 
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19, //left
		20, 21, 22, 20, 22, 23
	};

	AxisAlignedBox mAABB;

	mAABB.setNull();
	for (auto& vertex : vertices)
	{
		mAABB.merge(vertex.mPosition);
	}
	

	Mesh* pMesh = BuildHardBuffer(vertices, indices);

	pMesh->_setBounds(mAABB);

	auto mat = MaterialManager::getSingleton().getByName(matName);

	if (mat)
	{
		auto subMesh = pMesh->getSubMesh(0);

		subMesh->setMaterial(mat);
	}
	else
	{
		WARNING_LOG("fail to load material:%s", matName.c_str());
	}
	
	
	std::shared_ptr<Mesh> p(pMesh);
	mMeshMap[name] = p;
	return p;
}

std::shared_ptr<Mesh> MeshManager::createRect(
	filament::Engine* engine,
	const std::string& name, 
	Ogre::Vector3& leftop,
	Ogre::Vector3& leftbottom,
	Ogre::Vector3& righttop,
	Ogre::Vector3& rightbottom,
	Ogre::Vector3& normal)
{
	float tex = 1.0f;
	std::vector<SVertexElement> vertices;
	if (vertices.empty())
	{
		vertices.push_back(SVertexElement(leftbottom.x, leftbottom.y, leftbottom.z, normal.x, normal.y, normal.z,  0.0f, tex));
		vertices.push_back(SVertexElement(leftop.x, leftop.y, leftop.z, normal.x, normal.y, normal.z,  0.0f, 0.0f));
		vertices.push_back(SVertexElement(righttop.x, righttop.y, righttop.z, normal.x, normal.y, normal.z,  tex, 0.0f));
		vertices.push_back(SVertexElement(rightbottom.x, rightbottom.y, rightbottom.z, normal.x, normal.y, normal.z,  tex, tex));
	}
	

	std::vector<uint16_t> indices = {0, 2, 1, 0, 3, 2};

	Mesh* pMesh = BuildHardBuffer(vertices, indices);
	auto mat = MaterialManager::getSingleton().getByName("myrect");

	auto subMesh = pMesh->getSubMesh(0);

	subMesh->setMaterial(mat);

	std::shared_ptr<Mesh> p(pMesh);

	mMeshMap[name] = p;

	return p;
}


float GetHillsHeight(float x, float z)
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

Ogre::Vector3 GetHillsNormal(float x, float z)
{
	// n = (-df/dx, 1, -df/dz)
	DirectX::XMFLOAT3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	DirectX::XMVECTOR unitNormal = DirectX::XMVector3Normalize(XMLoadFloat3(&n));
	DirectX::XMStoreFloat3(&n, unitNormal);

	return Ogre::Vector3(n.x, n.y, n.z);
}

std::shared_ptr<Mesh> MeshManager::createGround(
	const std::string& name,
	float width,
	float height,
	uint32_t m,
	uint32_t n,
	Ogre::Vector3& lefttop)
{
	uint32_t vertexCount = m * n;
	uint32_t gridCount = (m - 1) * (n - 1);
	float dx = width / (n - 1);
	float dz = height / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	std::vector<SVertexElement> vertices;
	vertices.reserve(gridCount*4);
	SVertexElement element;
	element.mPosition = Ogre::Vector3(0.0f, 1.0f, 0.0f);
	element.mUV = Ogre::Vector2(0.0f, 0.0f);
	for (int32_t i = 0; i < gridCount; i++)
	{
		int32_t xindex = i % (n-1);
		int32_t zindex = i / (m-1);
		float x = - xindex * dx + lefttop.x;
		float z = zindex * dz + lefttop.z;
		element.mPosition = Ogre::Vector3(x, lefttop.y, z);
		vertices.push_back(element);
		element.mPosition = Ogre::Vector3(x - dx, lefttop.y, z);
		vertices.push_back(element);
		element.mPosition = Ogre::Vector3(x, lefttop.y, z + dz);
		vertices.push_back(element);
		element.mPosition = Ogre::Vector3(x - dx, lefttop.y, z + dz);
		vertices.push_back(element);
		
	}
	
	std::vector<uint16_t> indices;
	indices.resize(gridCount * 6);

	uint16_t* pIndex = indices.data();
	for (int32_t i = 0; i < gridCount; ++i)
	{
		// NB: We should take care with the clockwise ordered quad vertices
		/*
		0      1
		+------+
		|     /|
		|    / |
		|   /  |
		|  /   |
		| /    |
		|/     |
		+------+
		2      3
		*/

		* pIndex++ = i * 4 + 1;
		*pIndex++ = i * 4 + 0;
		*pIndex++ = i * 4 + 2;

		*pIndex++ = i * 4 + 1;
		*pIndex++ = i * 4 + 2;
		*pIndex++ = i * 4 + 3;
	}


	Mesh* pMesh = BuildHardBuffer(vertices, indices);

	auto mat = MaterialManager::getSingleton().getByName("myground");
	pMesh->getSubMesh(0)->setMaterial(mat);
	std::shared_ptr<Mesh> p(pMesh);
	mMeshMap[name] = p;
	return p;
}

std::shared_ptr<Mesh> MeshManager::createGrid(
	const std::string& name, 
	float width, 
	float height, 
	uint32_t m, 
	uint32_t n,
	bool change)
{
	uint32_t vertexCount = m * n;
	uint32_t faceCount = (m - 1) * (n - 1) * 2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f * width;
	float halfHeight = 0.5f * height;

	float dx = width / (n - 1);
	float dz = height / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);




	std::vector<SVertexElement> vertices;
	vertices.resize(vertexCount);
	for (uint32 i = 0; i < m; ++i)
	{
		float y = halfHeight - i * dz;
		for (uint32 j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			vertices[i * n + j].mPosition = Ogre::Vector3(x, 0.0f, y);
			vertices[i * n + j].mNormal = Ogre::Vector3(0.0f, 1.0f, 0.0f);

			// Stretch texture over grid.
			vertices[i * n + j].mUV.x = j * du;
			vertices[i * n + j].mUV.y = i * dv;
		}
	}

	for (auto& v : vertices)
	{
		if (change)
		{
			v.mPosition.y = GetHillsHeight(v.mPosition.x, v.mPosition.z) - 2.0f;
			v.mNormal = GetHillsNormal(v.mPosition.x, v.mPosition.z);
		}
		else
		{
			v.mPosition.y = -2.0f;
			v.mNormal = Ogre::Vector3(0.0f, 1.0f, 0.0f);
		}
	}
	//
	// Create the indices.
	//
	std::vector<uint16_t> indices;
	indices.resize(faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	uint32 k = 0;
	for (uint32 i = 0; i < m - 1; ++i)
	{
		for (uint32 j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}


	Mesh* pMesh = BuildHardBuffer(vertices, indices);

	MaterialInfo matInfo;
	matInfo.mTexname = "floor.dds";
	matInfo.mTexScale = 10.0f;
	applyMesh(pMesh, matInfo);

	std::shared_ptr<Mesh> p(pMesh);

	mMeshMap[name] = p;
	return p;
}

std::shared_ptr<Mesh> MeshManager::CreateSphere(const std::string& name, float radius, uint32_t sliceCount, uint32_t stackCount)
{
	//topvertex
	SVertexElement topvertex = SVertexElement(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f,  0.0f, 0.0f);
	SVertexElement bottomvertex = SVertexElement(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f );
	std::vector<SVertexElement> vertices;
	vertices.push_back(topvertex);

	float phiStep = DirectX::XM_PI / stackCount;
	float thetaStep = 2.0f * DirectX::XM_PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (uint32 i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (uint32 j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			DirectX::XMFLOAT3 Position;

			// spherical to cartesian
			Position.x = radius * sinf(phi) * cosf(theta);
			Position.y = radius * cosf(phi);
			Position.z = radius * sinf(phi) * sinf(theta);

			// Partial derivative of P with respect to theta
			DirectX::XMFLOAT3 TangentU;
			TangentU.x = -radius * sinf(phi) * sinf(theta);
			TangentU.y = 0.0f;
			TangentU.z = +radius * sinf(phi) * cosf(theta);

			DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&TangentU);
			DirectX::XMStoreFloat3(&TangentU, DirectX::XMVector3Normalize(T));

			DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&Position);

			DirectX::XMFLOAT3 Normal;
			DirectX::XMStoreFloat3(&Normal, DirectX::XMVector3Normalize(p));

			DirectX::XMFLOAT2 TexC;
			TexC.x = theta / DirectX::XM_2PI;
			TexC.y = phi / DirectX::XM_PI;
			vertices.push_back(SVertexElement(Position.x, Position.y, Position.z, Normal.x, Normal.y, Normal.z, TexC.x, TexC.y));
		}
	}

	vertices.push_back(bottomvertex);

	std::vector<uint16_t> indices;

	for (uint32_t i = 1; i <= sliceCount; ++i)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	uint32_t baseIndex = 1;
	uint32_t ringVertexCount = sliceCount + 1;
	for (uint32_t i = 0; i < stackCount - 2; ++i)
	{
		for (uint32_t j = 0; j < sliceCount; ++j)
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}
	uint32_t southPoleIndex = (uint32)indices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}

	Mesh* pMesh = BuildHardBuffer(vertices, indices);

	std::shared_ptr<Mesh> p(pMesh);

	std::shared_ptr<Material> mat =
		MaterialManager::getSingletonPtr()->getByName(std::string("mysphere"));
	pMesh->getSubMesh(0)->setMaterial(mat);

	mMeshMap[name] = p;
	return p;
}

std::shared_ptr<Mesh> MeshManager::CreateSphereSky(
	const std::string& name,
	float radius, uint32_t sliceCount, uint32_t stackCount)
{
	//topvertex
	std::vector<SVertexElement> vertices;
	SVertexElement topvertex = SVertexElement(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f,  0.0f, 0.0f);
	SVertexElement bottomvertex = SVertexElement(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);

	vertices.push_back(topvertex);

	float phiStep = DirectX::XM_PI / stackCount;
	float thetaStep = 2.0f * DirectX::XM_PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (uint32 i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (uint32 j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			DirectX::XMFLOAT3 Position;

			// spherical to cartesian
			Position.x = radius * sinf(phi) * cosf(theta);
			Position.y = radius * cosf(phi);
			Position.z = radius * sinf(phi) * sinf(theta);

			// Partial derivative of P with respect to theta
			DirectX::XMFLOAT3 TangentU;
			TangentU.x = -radius * sinf(phi) * sinf(theta);
			TangentU.y = 0.0f;
			TangentU.z = +radius * sinf(phi) * cosf(theta);

			DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&TangentU);
			DirectX::XMStoreFloat3(&TangentU, DirectX::XMVector3Normalize(T));

			DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&Position);

			DirectX::XMFLOAT3 Normal;
			DirectX::XMStoreFloat3(&Normal, DirectX::XMVector3Normalize(p));

			DirectX::XMFLOAT2 TexC;
			TexC.x = theta / DirectX::XM_2PI;
			TexC.y = phi / DirectX::XM_PI;
			vertices.push_back(SVertexElement(Position.x, Position.y, Position.z, Normal.x, Normal.y, Normal.z, TexC.x, TexC.y));
		}
	}

	vertices.push_back(bottomvertex);

	std::vector<uint16_t> indices;

	for (uint32_t i = 1; i <= sliceCount; ++i)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	uint32_t baseIndex = 1;
	uint32_t ringVertexCount = sliceCount + 1;
	for (uint32_t i = 0; i < stackCount - 2; ++i)
	{
		for (uint32_t j = 0; j < sliceCount; ++j)
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}
	uint32_t southPoleIndex = (uint32)indices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}

	Mesh* pMesh = BuildHardBuffer(vertices, indices);
	
	MaterialInfo matInfo;
	matInfo.mTexname = "grasscube1024.dds";
	matInfo.mCube = true;
	matInfo.mVSname = "sky.hlsl";
	matInfo.mPSname = "sky.hlsl";
	matInfo.mMatInfo.DiffuseAlbedo = Ogre::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	matInfo.mMatInfo.FresnelR0 = Ogre::Vector3(0.1f, 0.1f, 0.1f);
	matInfo.mMatInfo.Roughness = 1.0f;
	applyMesh(pMesh, matInfo);

	std::shared_ptr<Mesh> p(pMesh);

	mMeshMap[name] = p;
	return p;
}

std::shared_ptr<Mesh> MeshManager::CreateCylinder(const std::string& name, float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount)
{
	float stackHeight = height / stackCount;

	std::vector<SVertexElement> vertices;

	// Amount to increment radius as we move up each stack level from bottom to top.
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	uint32_t ringCount = stackCount + 1;

	for (uint32 i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		// vertices of ring
		float dTheta = 2.0f * DirectX::XM_PI / sliceCount;
		for (uint32 j = 0; j <= sliceCount; ++j)
		{


			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(r * c, y, r * s);
			DirectX::XMFLOAT2 TexC;
			TexC.x = (float)j / sliceCount;
			TexC.y = 1.0f - (float)i / stackCount;

			// This is unit length.
			Ogre::Vector3 TangentU = Ogre::Vector3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			Ogre::Vector3 bitangent(dr * c, -height, dr * s);

			Ogre::Vector3 T = TangentU;
			Ogre::Vector3 B = bitangent;
			Ogre::Vector3 N = T.crossProduct(B);

			Ogre::Vector3& Normal = N;

			vertices.push_back( SVertexElement(Position.x, Position.y, Position.z, Normal.x, Normal.y, Normal.z, TexC.x, TexC.y));
		}
	}

	// Add one because we duplicate the first and last vertex per ring
	// since the texture coordinates are different.
	uint32 ringVertexCount = sliceCount + 1;

	// Compute indices for each stack.

	std::vector<uint16_t> indices;
	for (uint32 i = 0; i < stackCount; ++i)
	{
		for (uint32 j = 0; j < sliceCount; ++j)
		{
			indices.push_back(i * ringVertexCount + j);
			indices.push_back((i + 1) * ringVertexCount + j);
			indices.push_back((i + 1) * ringVertexCount + j + 1);

			indices.push_back(i * ringVertexCount + j);
			indices.push_back((i + 1) * ringVertexCount + j + 1);
			indices.push_back(i * ringVertexCount + j + 1);
		}
	}

	BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, vertices, indices);
	BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, vertices, indices);


	Mesh* pMesh = BuildHardBuffer(vertices, indices);
	

	std::shared_ptr<Mesh> p(pMesh);

	std::shared_ptr<Material> mat =
		MaterialManager::getSingletonPtr()->getByName(std::string("myCylinder"));
	pMesh->getSubMesh(0)->setMaterial(mat);
	
	mMeshMap[name] = p;
	return p;
}

std::shared_ptr<Mesh> MeshManager::createSimpleRoom(const std::string& name)
{
	float d = 0.0f;
	std::array<SVertexElement, 20> vertices =
	{
		// Floor: Observe we tile texture coordinates.
		SVertexElement(-3.5f, d, -10.0f, 0.0f, 1.0f, 0.0f,0.0f, 4.0f), // 0 
		SVertexElement(-3.5f, d,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f),
		SVertexElement(7.5f, d,   0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f),
		SVertexElement(7.5f, d, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f),

		// Wall: Observe we tile texture coordinates, and that we
		// leave a gap in the middle for the mirror.
		SVertexElement(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 4
		SVertexElement(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		SVertexElement(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f),
		SVertexElement(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f),

		SVertexElement(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 8 
		SVertexElement(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		SVertexElement(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f),
		SVertexElement(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f),

		SVertexElement(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 12
		SVertexElement(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		SVertexElement(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f),
		SVertexElement(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f),

		// Mirror
		SVertexElement(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,  0.0f, 1.0f), // 16
		SVertexElement(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f,  0.0f, 0.0f),
		SVertexElement(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f,  1.0f, 0.0f),
		SVertexElement(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f)

	};

	std::vector<uint32_t> indices =
	{
		// Floor
		0, 1, 2,
		0, 2, 3,

		// Walls
		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		// Mirror
		16, 17, 18,
		16, 18, 19
	};

	Mesh* pMesh = new Mesh(name);

	VertexData* vd = pMesh->getVertexData();
	IndexData* id = pMesh->getIndexData();
	uint32_t size = vertices.size() * sizeof(SVertexElement);

	vd->setVertexCount(vertices.size());
	vd->addBindBuffer(0, sizeof(SVertexElement), vertices.size());
	vd->writeBindBufferData(0, (const char*)vertices.data(), size);
	id->createBuffer(4, indices.size());
	id->writeData((const char*)indices.data(), indices.size() * 4);

	VertexDeclaration* declaration;

	

	vd->addElement(0, 0, 0, VET_FLOAT3, VES_POSITION);
	vd->addElement(0, 0, 12, VET_FLOAT3, VES_NORMAL);
	vd->addElement(0, 0, 24, VET_FLOAT3, VES_TANGENT);
	vd->addElement(0, 0, 36, VET_FLOAT2, VES_TEXTURE_COORDINATES);

	SubMesh* floor = pMesh->addSubMesh(true, true);
	floor->addIndexs(6, 0, 0);
	auto mat_floor = MaterialManager::getSingletonPtr()->getByName("floor");
	floor->setMaterial(mat_floor);

	SubMesh* wall = pMesh->addSubMesh(true, true);
	wall->addIndexs(18, 6, 0);
	auto mat_wall = MaterialManager::getSingletonPtr()->getByName("wall");
	wall->setMaterial(mat_wall);

	SubMesh* mirror = pMesh->addSubMesh(true, true);
	mirror->addIndexs(6, 24, 0);
	auto mat_mirror = MaterialManager::getSingletonPtr()->getByName("mirror");
	mirror->setMaterial(mat_mirror);
	
	std::shared_ptr<Mesh> p(pMesh);

	p->prepare();
	mMeshMap[name] = p;
	return p;
	
}

void MeshManager::BuildCylinderTopCap(
	float bottomRadius, 
	float topRadius, 
	float height, 
	uint32 sliceCount, 
	uint32 stackCount, 
	std::vector<SVertexElement>& vertices,
	std::vector<uint16_t>& indices)
{
	uint32_t baseIndex = (uint32_t)vertices.size();

	float y = 0.5f * height;
	float dTheta = 2.0f * DirectX::XM_PI / sliceCount;

	// Duplicate cap ring vertices because the texture coordinates and normals differ.
	for (uint32_t i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius * cosf(i * dTheta);
		float z = topRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		vertices.push_back(SVertexElement(x, y, z, 0.0f, 1.0f, 0.0f,  u, v));
	}

	// Cap center vertex.
	vertices.push_back(SVertexElement(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f,  0.5f, 0.5f));

	// Index of center vertex.
	uint32_t centerIndex = (uint32_t)vertices.size() - 1;

	for (uint32_t i = 0; i < sliceCount; ++i)
	{
		indices.push_back(centerIndex);
		indices.push_back(baseIndex + i + 1);
		indices.push_back(baseIndex + i);
	}
}

void MeshManager::BuildCylinderBottomCap(
	float bottomRadius, 
	float topRadius, 
	float height, 
	uint32 sliceCount, 
	uint32 stackCount, 
	std::vector<SVertexElement>& vertices,
	std::vector<uint16_t>& indices)
{
	uint32 baseIndex = (uint32)vertices.size();
	float y = -0.5f * height;

	// vertices of ring
	float dTheta = 2.0f * DirectX::XM_PI / sliceCount;
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius * cosf(i * dTheta);
		float z = bottomRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		vertices.push_back(SVertexElement(x, y, z, 0.0f, -1.0f, 0.0f,  u, v));
	}

	// Cap center vertex.
	vertices.push_back(SVertexElement(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f,  0.5f, 0.5f));

	// Cache the index of center vertex.
	uint32_t centerIndex = (uint32_t)vertices.size() - 1;

	for (uint32_t i = 0; i < sliceCount; ++i)
	{
		indices.push_back(centerIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}
}

Mesh* MeshManager::BuildHardBuffer(
	std::vector<SVertexElement>& vertices,
	std::vector<uint16_t>& indices)
{
	Mesh* pMesh = new Mesh(std::string());

	VertexData* vd = pMesh->getVertexData();
	IndexData* id = pMesh->getIndexData();
	uint32_t size = vertices.size() * sizeof(SVertexElement);
	

	auto vertexCount = vertices.size();
	vd->setVertexCount(vertexCount);

	vd->addBindBuffer(0, sizeof(SVertexElement), vertexCount);
	vd->writeBindBufferData(0, (const char*)vertices.data(), size);

	uint32_t indexSize = sizeof(indices[0]);
	id->createBuffer(indexSize, indices.size());
	id->writeData((const char*)indices.data(), indexSize * indices.size());
	SubMesh* sub = pMesh->addSubMesh(true, true);

	sub->addIndexs(indices.size(), 0, 0);


	vd->addElement(0, 0, 0, VET_FLOAT3, VES_POSITION);
	vd->addElement(0, 0, 12, VET_FLOAT3, VES_NORMAL);
	vd->addElement(0, 0, 24, VET_FLOAT2, VES_TEXTURE_COORDINATES);

	pMesh->prepare();
	return pMesh;
}

void MeshManager::applyMesh(Mesh* pMesh, MaterialInfo& matInfo)
{
	static uint32_t mat_index = 0;
	++mat_index;
	std::shared_ptr<Material> mat = MaterialManager::getSingletonPtr()->create(std::to_string(mat_index));

	TextureProperty texProperty;
	if (matInfo.mCube)
	{
		texProperty._texType = TEX_TYPE_CUBE_MAP;
	}
	mat->addTexture(matInfo.mTexname, &texProperty);
}
}