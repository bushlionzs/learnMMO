#include <OgreHeader.h>
#include "hlslUtil.h"
#include "string_util.h"
#include <platform_file.h>
#include <dxcapi.h>
#include <wrl.h>
#include <OgreResourceManager.h>

static IDxcUtils* pUtils = nullptr;
static IDxcCompiler3* pCompiler = nullptr;
static IDxcLibrary* pLibrary = nullptr;
using namespace Microsoft::WRL;
void hlslInit()
{
	if (pUtils == nullptr)
	{
	    DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&pLibrary));

		HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
		if (SUCCEEDED(hr))
		{
			hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
		}
	}
}


class CustomDxcInclude : public IDxcIncludeHandler {
public:
	// 构造函数和析构函数
	CustomDxcInclude() {}
	virtual ~CustomDxcInclude() {}

	// IUnknown 方法
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override
	{
		if (riid == __uuidof(IUnknown) || riid == __uuidof(IDxcIncludeHandler))
		{
			*ppvObject = static_cast<IDxcIncludeHandler*>(this);
			AddRef();
			return S_OK;
		}
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef() override
	{
		return InterlockedIncrement(&m_refCount);
	}

	STDMETHODIMP_(ULONG) Release() override
	{
		ULONG refCount = InterlockedDecrement(&m_refCount);
		if (refCount == 0)
		{
			delete this;
		}
		return refCount;
	}

	virtual HRESULT STDMETHODCALLTYPE LoadSource(
		_In_z_ LPCWSTR pFilename,
		_COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource
	)
	{
		std::string name = dy::unicode_to_acsi(pFilename);
		name = dy::get_short_name(name);
		ResourceInfo* res = ResourceManager::getSingleton().getResource(name);

		get_file_content(res->_fullname.c_str(), content);
		const char* kk = content.c_str();
		ComPtr<IDxcBlobEncoding> pBlob;

		HRESULT hr = pLibrary->CreateBlobWithEncodingFromPinned(
			reinterpret_cast<const uint8_t*>(content.c_str()),
			content.size(),
			CP_ACP,
			&pBlob);
		if (SUCCEEDED(hr))
		{
			*ppIncludeSource = pBlob.Detach();
			return S_OK;
		}
		*ppIncludeSource = nullptr;
		return E_FAIL;
	}

private:
	LONG m_refCount = 1;
	std::string content;
};

bool hlslToBin(
	const std::string& shaderName,
	const std::string& shaderContent,
	const std::string& entryPoint,
	const std::vector<std::pair<std::string, std::string>>& shaderMacros,
	Ogre::ShaderType shaderType,
	std::string& spv,
	bool vulkan
)
{
	hlslInit();

	DxcBuffer dxcBuffer;

	dxcBuffer.Ptr = shaderContent.c_str();
	dxcBuffer.Size = shaderContent.size();
	dxcBuffer.Encoding = CP_ACP;
	std::vector<LPCWSTR> arguments = {
	L"-Zi",
	L"-Od",
	L"-Qembed_debug"
	};
	
	std::wstring targetProfile;
	std::wstring wEntryPoint = dy::acsi_to_widebyte(entryPoint);
	if (shaderType == VertexShader)
	{
		targetProfile = L"vs_6_5";
	}
	else if (shaderType == PixelShader)
	{
		targetProfile = L"ps_6_5";
	}
	else if (shaderType == GeometryShader)
	{
		targetProfile = L"gs_6_5";
	}
	else if(shaderType == ComputeShader)
	{
		targetProfile = L"cs_6_5";
	}
	else if (shaderType == RayGenShader ||
		       shaderType == MissShader ||
		       shaderType == AnyHitShader ||
		      shaderType == ClosestHitShader)
	{
		targetProfile = L"lib_6_5";
	}
	else
	{
		assert(false);
	}
	
	std::wstring wShaderName = dy::acsi_to_widebyte(shaderName);
	wchar_t buffer[256];
	std::vector<std::wstring> pool;
	pool.reserve(shaderMacros.size());
	for (auto& obj : shaderMacros)
	{
		std::wstring aa = dy::acsi_to_widebyte(obj.first);
		std::wstring bb = dy::acsi_to_widebyte(obj.second);
		swprintf_s(buffer, L"%s=%s", aa.c_str(), bb.c_str());
		pool.push_back(buffer);
		arguments.push_back(L"-D");
		arguments.push_back(pool.back().c_str());
	}

	if (vulkan)
	{
		arguments.push_back(L"-D VULKAN");
		arguments.push_back(L"-spirv");
		arguments.push_back(L"-fspv-target-env=vulkan1.3");
	}
	

	arguments.push_back(L"-D DIRECT3D12");

	IDxcCompilerArgs* args = nullptr;
	pUtils->BuildArguments(
		wShaderName.c_str(),
		wEntryPoint.c_str(),
		targetProfile.c_str(),
		arguments.data(),
		static_cast<UINT>(arguments.size()),
		NULL,
		0,
		&args);
	{
		uint32_t size = args->GetCount();
		LPCWSTR* str = args->GetArguments();
		std::vector<std::wstring> bb;
		for (uint32_t i = 0; i < size; i++)
		{
			LPCWSTR aa = str[i];
			bb.push_back(aa);
		}
		int kk = 0;
	}
	CustomDxcInclude includer;
	IDxcResult* pResult = nullptr;

	HRESULT hr = pCompiler->Compile(
		&dxcBuffer,           // 源代码
		args->GetArguments(),      // 编译参数
		args->GetCount(),
		&includer,
		IID_PPV_ARGS(&pResult)
		);
	if (FAILED(hr))
	{
		assert(false);
		pResult->Release();
		return false;
	}
	ComPtr<IDxcBlob> pShaderBlob;
	pResult->GetResult(&pShaderBlob);

	IDxcBlobUtf8* errors = nullptr;
	if (FAILED(pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr))) return false;
	if (errors != nullptr && errors->GetStringLength() != 0)
	{
		const char* str = (char*)errors->GetStringPointer();
		 OutputDebugStringA(str);
		 assert(false);
		 return false;
	}
	const char* data = (const char*)pShaderBlob->GetBufferPointer();
	uint32_t size = pShaderBlob->GetBufferSize();
	
	spv.assign(data, size);
	pResult->Release();
	args->Release();
	return true;
}