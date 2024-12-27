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
		std::string name = dy::unicode_to_acsi(pFilename + 2);
		ResourceInfo* res = ResourceManager::getSingleton().getResource(name);

		get_file_content(res->_fullname.c_str(), content);

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

bool hlslToSpirv(
	const std::string& shaderName,
	const std::string& shaderContent,
	const std::string& entryPoint,
	const std::vector<std::pair<std::string, std::string>>& shaderMacros,
	Ogre::ShaderType shaderType,
	std::string& spv
)
{
	hlslInit();

	DxcBuffer dxcBuffer;

	dxcBuffer.Ptr = shaderContent.c_str();
	dxcBuffer.Size = shaderContent.size();
	dxcBuffer.Encoding = CP_UTF8;
	std::vector<LPCWSTR> arguments = {
	//L"-T", L"vs_5_1", // 目标着色器模型
	//L"-E", L"VS",   // 入口点名称
	L"-spirv"         // 编译为目标SPIR-V
	};
	

	if (shaderType == VertexShader)
	{
		arguments.push_back(L"-T");
		arguments.push_back(L"vs_5_1");
		arguments.push_back(L"-E");
		arguments.push_back(L"VS");
	}
	else if (shaderType == PixelShader)
	{
		arguments.push_back(L"-T");
		arguments.push_back(L"ps_5_1");
		arguments.push_back(L"-E");
		arguments.push_back(L"PS");
	}
	else
	{
		assert(false);
	}

	wchar_t buffer[256];

	
	for (auto& obj : shaderMacros)
	{
		std::wstring aa = dy::acsi_to_widebyte(obj.first);
		std::wstring bb = dy::acsi_to_widebyte(obj.second);
		swprintf_s(buffer, L"%s=%s", aa.c_str(), bb.c_str());
		arguments.push_back(L"-D");
		arguments.push_back(buffer);
	}

	
	CustomDxcInclude includer;
	IDxcResult* pResult = nullptr;
	IDxcCompilerArgs;
	HRESULT hr = pCompiler->Compile(
		&dxcBuffer,           // 源代码
		arguments.data(),      // 编译参数
		arguments.size(),
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
	const char* data = (const char*)pShaderBlob->GetBufferPointer();
	uint32_t size = pShaderBlob->GetBufferSize();
	spv.assign(data, size);
	pResult->Release();
	return true;
}