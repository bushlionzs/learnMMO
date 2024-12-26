#pragma once

#include "OgreScriptLoader.h"
class MPQArchive;
class WowResourceParser : public Ogre::ScriptLoader
{
public:
    WowResourceParser();

    virtual std::vector<String> getSuffix();
    virtual void parseScript(ResourceInfo* res, const String& groupName);
    virtual Real getLoadingOrder(void) const;
private:
    std::unordered_map<String, MPQArchive*> mMpqMap;
};