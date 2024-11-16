#pragma once

#include "shader.h"
#include "engine_struct.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <D3Dcompiler.h>

using namespace Microsoft::WRL;
class Dx12Shader;
class Dx12RenderableData;
class Dx12Frame;

enum Dx12PassState
{
	PassState_Normal = 0,
	PassState_Shadow,
	PassState_CubeMap
};
#define FRAME_RESOURCE_COUNT 1

class Dx12Pass
{
public:
	Ogre::Material* mMaterial;
	Dx12Shader* mShader;
	Ogre::Renderable* mRenderable;
	uint32_t  mPassState;
	ID3D12RootSignature* mRootSignature;
	Dx12RenderableData* mDx12RenderableData;
	RenderListType mRenderListType;	
};

typedef enum TextureDimension
{
    TEXTURE_DIM_1D,
    TEXTURE_DIM_2D,
    TEXTURE_DIM_2DMS,
    TEXTURE_DIM_3D,
    TEXTURE_DIM_CUBE,
    TEXTURE_DIM_1D_ARRAY,
    TEXTURE_DIM_2D_ARRAY,
    TEXTURE_DIM_2DMS_ARRAY,
    TEXTURE_DIM_CUBE_ARRAY,
    TEXTURE_DIM_COUNT,
    TEXTURE_DIM_UNDEFINED,
} TextureDimension;

struct ShaderResource
{
    // resource Type
    D3D_SHADER_INPUT_TYPE type;

    // The resource set for binding frequency
    uint32_t set;

    // The resource binding location
    uint32_t reg;

    // The size of the resource. This will be the DescriptorInfo array size for textures
    uint32_t size;

    // what stages use this resource
    uint8_t used_stages;

    std::string name;

    // 1D / 2D / Array / MSAA / ...
    TextureDimension dim;
};

typedef struct RootParameter
{
	ShaderResource  mShaderResource;
	//DescriptorInfo* pDescriptorInfo;
} RootParameter;

std::wstring AnsiToWString(const std::string& str);
std::wstring AnsiToWString(const char* str);

class DxException
{
public:
    DxException() = default;
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif