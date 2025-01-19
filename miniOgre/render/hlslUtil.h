#pragma once
#include <OgreCommon.h>

bool hlslToBin(
	const std::string& shaderName,
	const std::string& shaderContent,
	const std::string& entryPoint,
	const std::vector<std::pair<std::string, std::string>>& shaderMacros,
	const std::vector<std::wstring>* args,
	Ogre::ShaderType shaderType,
	std::string& spv,
	bool vulkan = true
);