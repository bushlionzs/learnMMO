#pragma once

#include "shader.h"
#include "engine_struct.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <D3Dcompiler.h>
#include <IThread.h>

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
#define D3D12MA_IMPLEMENTATION
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

typedef struct DescriptorHeap
{
    /// DX Heap
    ID3D12DescriptorHeap* pHeap;
    /// Lock for multi-threaded descriptor allocations
    Mutex                       mMutex;
    ID3D12Device* pDevice;
    /// Start position in the heap
    D3D12_CPU_DESCRIPTOR_HANDLE mStartCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE mStartGpuHandle;
    // Bitmask to track free regions (set bit means occupied)
    uint32_t* pFlags;
    /// Description
    D3D12_DESCRIPTOR_HEAP_TYPE  mType;
    uint32_t                    mNumDescriptors;
    /// Descriptor Increment Size
    uint32_t                    mDescriptorSize;
    // Usage
    uint32_t                    mUsedDescriptors;
} DescriptorHeap;

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

    uint32_t set_index;
    // what stages use this resource
    uint8_t used_stages;

    std::string name;

    // 1D / 2D / Array / MSAA / ...
    TextureDimension dim;
};

typedef int32_t DxDescriptorID;

typedef struct RootParameter
{
	ShaderResource  mShaderResource;
	//DescriptorInfo* pDescriptorInfo;
} RootParameter;

typedef struct DescriptorHeapProperties
{
    uint32_t                    mMaxDescriptors;
    D3D12_DESCRIPTOR_HEAP_FLAGS mFlags;
} DescriptorHeapProperties;


struct DescriptorHeapContext
{
    struct DescriptorHeap** mCPUDescriptorHeaps;
    struct DescriptorHeap** mCbvSrvUavHeaps;
};

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

#define CHECK_HRESULT(exp)                                                       \
    do                                                                           \
    {                                                                            \
        HRESULT hres = (exp);                                                    \
        if (!SUCCEEDED(hres))                                                    \
        {                                                                        \
            assert(false);                                                       \
        }                                                                        \
    } while (0)