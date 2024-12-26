#include "OgreHeader.h"
#include "shaderManager.h"
#include "ScriptParserUtil.h"
#include "myutils.h"
#include "OgreString.h"
#include "OgreResourceManager.h"
#include "OgreMemoryStream.h"
#include "platform_file.h"
namespace Ogre {

	template<> ShaderManager* Ogre::Singleton<ShaderManager>::msSingleton = 0;

	ShaderManager::ShaderManager()
	{

	}

	ShaderManager::~ShaderManager()
	{

	}

	std::vector<String> ShaderManager::getSuffix()
	{
		return { ".shader" };
	}

	void ShaderManager::parseScript(ResourceInfo* res, const String& groupName)
	{
		std::shared_ptr<MemoryDataStream> stream = std::make_shared<MemoryDataStream>(res);
		std::string content = stream->getAsString();
		parseShaderImpl(content);
	}

	void ShaderManager::parseShaderImpl(const String& content)
	{
		std::stringstream ss;
		ss << content;

		std::string linePart;
		ss >> linePart;

		std::string name;


		

		while (!ss.eof()) {
			// Skip commented lines
			if (linePart == partComment) {
				NextAfterNewLine(ss, linePart);
				continue;
			}

			if (linePart == "shader")
			{
				ss >> name;

				ShaderFormat* shaderFormat = new ShaderFormat;
				addShader(name, shaderFormat);
				do
				{
					ss >> linePart;
				} while (linePart != partBlockStart);

				ss >> linePart;

				while (linePart != partBlockEnd) {
					// Skip commented lines
					if (linePart == partComment) {
						SkipLine(ss);
						continue;
					}

					if (linePart == "shader_unit") {
						readShaderUnit(ss, shaderFormat);
					}


					ss >> linePart;
				}
			}
			else if (linePart == "shaderMapping")
			{
				ss >> name;
				do
				{
					ss >> linePart;
				} while (linePart != partBlockStart);

				ss >> linePart;

				while (linePart != partBlockEnd) {
					// Skip commented lines
					if (linePart == partComment) {
						SkipLine(ss);
						continue;
					}

					if (linePart == "vertexShaderInput") 
					{
						readMappingInfo(ss, name, VertexInput);
					}
					else if (linePart == "vertexShaderOutput")
					{
						readMappingInfo(ss, name, VertexOutput);
					}
					else if (linePart == "PixelShaderInput")
					{
						readMappingInfo(ss, name, PixelInput);
					}


					ss >> linePart;
				}
			}
			else
			{
				ss >> linePart;
			}
			
		}
	}

	bool ShaderManager::readShaderUnit(
		std::stringstream& ss,
		ShaderFormat* shaderFormat)
	{
		static std::map<String, EngineType> enginetype_map =
		{
			{"directx", EngineType_Dx12},
			{"vulkan", EngineType_Vulkan}
		};

		std::string linePart;
		ss >> linePart;

		if (linePart != partBlockStart) {
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "shader unit error");
		}

		ss >> linePart;

		EngineType enginetype = EngineType_Dx12;
		std::map<String, String> keyvalueMap;
		while (linePart != partBlockEnd) {
			// Skip commented lines
			if (linePart == partComment) {
				SkipLine(ss);
				continue;
			}

			std::string val = SkipLine(ss);
			Ogre::StringUtil::trim(val);

			if (linePart == "shader_type")
			{
				enginetype = enginetype_map[val];
			}
			else
			{
				keyvalueMap[linePart] = val;
			}
			
			ss >> linePart;
		}

		for (auto& pair : keyvalueMap)
		{
			std::vector<String> aa = Ogre::StringUtil::split(pair.second);
			if (pair.first == "vertex_shader")
			{
				shaderFormat->shaderInfo[enginetype].privateInfo.vertexShaderName = aa[0];
				shaderFormat->shaderInfo[enginetype].privateInfo.vertexShaderEntryPoint = aa[1];

				if (enginetype == EngineType_Dx12)
				{
					shaderFormat->shaderInfo[EngineType_Dx12].privateInfo.vertexShaderName = aa[0];
					shaderFormat->shaderInfo[EngineType_Dx12].privateInfo.vertexShaderEntryPoint = aa[1];
				}
			}
			else if (pair.first == "frag_shader")
			{
				shaderFormat->shaderInfo[enginetype].privateInfo.fragShaderName = aa[0];
				shaderFormat->shaderInfo[enginetype].privateInfo.fragShaderEntryPoint = aa[1];
				if (enginetype == EngineType_Dx12)
				{
					shaderFormat->shaderInfo[EngineType_Dx12].privateInfo.fragShaderName = aa[0];
					shaderFormat->shaderInfo[EngineType_Dx12].privateInfo.fragShaderEntryPoint = aa[1];
				}
			}
			else if (pair.first == "compute_shader")
			{
				shaderFormat->shaderInfo[enginetype].privateInfo.computeShaderName = aa[0];
				shaderFormat->shaderInfo[enginetype].privateInfo.computeShaderEntryPoint = aa[1];
				if (enginetype == EngineType_Dx12)
				{
					shaderFormat->shaderInfo[EngineType_Dx12].privateInfo.computeShaderName = aa[0];
					shaderFormat->shaderInfo[EngineType_Dx12].privateInfo.computeShaderEntryPoint = aa[1];
				}
			}
			else if (pair.first == "geometry_shader")
			{
				shaderFormat->shaderInfo[enginetype].privateInfo.geometryShaderName = aa[0];
				shaderFormat->shaderInfo[enginetype].privateInfo.geometryShaderEntryPoint = aa[1];
				if (enginetype == EngineType_Dx12)
				{
					shaderFormat->shaderInfo[EngineType_Dx12].privateInfo.geometryShaderName = aa[0];
					shaderFormat->shaderInfo[EngineType_Dx12].privateInfo.geometryShaderEntryPoint = aa[1];
				}
			}
			
		}
		return true;
	}

	Real ShaderManager::getLoadingOrder(void) const
	{
		return 2.0f;
	}

	ShaderPrivateInfo* ShaderManager::getShader(
		const String& name,
		EngineType renderSystem)
	{
		auto itor = mShaderMap.find(name);

		if (itor != mShaderMap.end())
		{
			return &itor->second->shaderInfo[renderSystem].privateInfo;
		}

		return nullptr;
	}

	String* ShaderManager::getShaderContent(const String& name)
	{
		auto itor = mShaderContentMap.find(name);

		if (itor != mShaderContentMap.end())
		{
			return &itor->second;
		}

		auto res = ResourceManager::getSingleton().getResource(name);

		if (res == nullptr)
		{
			return nullptr;
		}

		get_file_content(res->_fullname.c_str(), mShaderContentMap[name]);

		itor = mShaderContentMap.find(name);

		return &itor->second;
	}

	bool ShaderManager::readMappingInfo(
		std::stringstream& ss, std::string& name, MappingType mappingType)
	{
		std::string first;
		std::string second;

		std::unordered_map<String, std::vector<ShaderMappingInfo>>* pMap = nullptr;
		if (mappingType == VertexInput)
		{
			pMap = &mVertexInputMap;
		}
		else if (mappingType == VertexOutput)
		{
			pMap = &mVertexOutputMap;
		}
		else if (mappingType == PixelInput)
		{
			pMap = &mPixelOutputMap;
		}


		std::unordered_map<String, std::vector<ShaderMappingInfo>>& shaderMap = *pMap;
		do
		{
			ss >> first;
		} while (first != partBlockStart);

		ss >> first ;
		ShaderMappingInfo info;
		while (first != partBlockEnd) {
			// Skip commented lines
			if (first == partComment) {
				SkipLine(ss);
				continue;
			}

			ss >> second;
			info.location = atoi(first.c_str());
			info.semantic = second;
			shaderMap[name].push_back(info);
			ss >> first;
		}
		return true;
	}

	void ShaderManager::addShader(const String& name, ShaderFormat* sf)
	{
		auto itor = mShaderMap.find(name);
		if (itor != mShaderMap.end())
		{
			assert(false);
		}

		mShaderMap[name] = sf;
	}

	void ShaderManager::addMacro(const String& name)
	{
		auto itor = mMacroMap.find(name);

		if (itor == mMacroMap.end())
		{
			mMacroMap[name] = mMacroValue;

			mMacroValue <<= 1;
		}
	}

	int32_t ShaderManager::getMacroIndex(const String& name)
	{
		auto itor = mMacroMap.find(name);

		if (itor == mMacroMap.end())
		{
			assert(false);
		}

		return itor->second;
	}

	const std::vector<ShaderMappingInfo>& ShaderManager::getVertexInputMapping(
		const std::string& shaderFileName)
	{
		auto itor = mVertexInputMap.find(shaderFileName);
		if (itor != mVertexInputMap.end())
		{
			return itor->second;
		}

		return mDummy;
	}

	const std::vector<ShaderMappingInfo>& ShaderManager::getVertexOutputMapping(
		const std::string& shaderFileName)
	{
		auto itor = mVertexOutputMap.find(shaderFileName);
		if (itor != mVertexOutputMap.end())
		{
			return itor->second;
		}

		return mDummy;
	}

	const std::vector<ShaderMappingInfo>& ShaderManager::getPixelInputMapping(
		const std::string& shaderFileName)
	{
		auto itor = mPixelOutputMap.find(shaderFileName);
		if (itor != mPixelOutputMap.end())
		{
			return itor->second;
		}

		return mDummy;
	}
}

