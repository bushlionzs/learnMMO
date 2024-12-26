#pragma once


#include "OgreScriptLoader.h"
class OgreMaterialParam;
class MaterialScriptParser : public ScriptLoader
{
public:
	virtual std::vector<String> getSuffix();
	virtual void parseScript(ResourceInfo* res, const String& groupName);
	virtual Real getLoadingOrder(void) const;

	OgreMaterialParam* getMaterialParam(const std::string& name);
private:
	void parseMaterialImpl(const std::string& content);
	bool readTechnique(
		const std::string& techniqueName, 
		std::stringstream& ss, 
		Material* mat,
		OgreMaterialParam* param);
	bool readPass(
		const std::string& passName, 
		std::stringstream& ss, 
		Material* mat,
		OgreMaterialParam* param);
	bool readTextureUnit(
		const std::string& textureUnitName, 
		std::stringstream& ss, 
		Material* mat,
		OgreMaterialParam* param);
	
	void addMaterialParam(const std::string& name, OgreMaterialParam* param);
private:
	std::unordered_map<std::string, OgreMaterialParam*> mMaterialParamMap;
};