#include "OgreHeader.h"
#include "packFileParser.h"
#include "OgreResourceManager.h"
#include "OgreMemoryStream.h"
struct PackHeader
{
	uint32_t _headerLength;
	uint32_t _version;
	uint32_t _nameOffset;
	uint32_t _dataOffset;
	uint32_t _fileCount;
};

#define MAX_ENTRY_NAME 100
struct NameEntry
{
	char _name[MAX_ENTRY_NAME];
	uint32_t _dataOffset;
	uint32_t _dataSize;
};

PackfileParser::PackfileParser()
{

}

PackfileParser::~PackfileParser()
{

}

std::vector<String> PackfileParser::getSuffix()
{
	return { ".pack" };
}

void PackfileParser::parseScript(
	ResourceInfo* res, const String& groupName)
{
	std::shared_ptr<MemoryDataStream> stream = std::make_shared<MemoryDataStream>(res);
	mPackList.push_back(stream);


	const char* data = mPackList.back()->getStreamData();

	uint32_t size = mPackList.back()->getStreamLength();

	//todo: Endian issues should be taken into consideration
	PackHeader* header = (PackHeader*)data;
	NameEntry* first = (NameEntry*)(data + header->_nameOffset);

	Ogre::StreamContent dummy;
	
	for (uint32_t i = 0; i < header->_fileCount; i++)
	{
		NameEntry* entry = first + i;

		dummy._data = data + entry->_dataOffset;
		dummy._size = entry->_dataSize;

		ResourceInfo* res = new ResourceInfo;
		res->_fullname = entry->_name;
		res->_base = this;

		mPackMap[res->_fullname] = dummy;

		if (res->_fullname == "char_race.tab")
		{
			int kk = 0;
		}
		
		ResourceManager::getSingleton()._addResource(res->_fullname, res);
	}
}

Real PackfileParser::getLoadingOrder(void) const
{
	return 1.0f;
}

Ogre::StreamContent* PackfileParser::getPackfile(
	const String& name, const String& group)
{
	auto itor = mPackMap.find(name);
	if (itor != mPackMap.end())
	{
		return &itor->second;
	}

	return nullptr;
}