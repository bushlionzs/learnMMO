#pragma once

#include "OgreScriptLoader.h"

class PackfileParser : public ScriptLoader, public PackFileBase
{
public:
	PackfileParser();
	~PackfileParser();
	virtual std::vector<String> getSuffix();
	virtual void parseScript(ResourceInfo* res, const String& groupName);
	virtual Real getLoadingOrder(void) const;

	virtual Ogre::StreamContent* getPackfile(
		const String& name, const String& group);

private:
	std::vector<std::shared_ptr<DataStream>> mPackList;

	std::unordered_map<String, Ogre::StreamContent> mPackMap;
};