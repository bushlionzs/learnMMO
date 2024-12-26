#include "OgreHeader.h"
#include "WowResourceParser.h"
#include <StormLib.h>
#include "MPQFile.h"
WowResourceParser::WowResourceParser()
{

}

std::vector<String> WowResourceParser::getSuffix()
{
    return { ".MPQ" };
}

void WowResourceParser::parseScript(ResourceInfo* res, const String& groupName)
{
    auto itor = mMpqMap.find(res->_fullname);

    if (itor != mMpqMap.end())
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "duplicated mpq!");
        return;
    }
    
    MPQArchive* archive = new MPQArchive(res->_fullname);

    archive->load();

    mMpqMap.emplace(res->_fullname, archive);
}

Real WowResourceParser::getLoadingOrder(void) const
{
    return 20.0f;
}