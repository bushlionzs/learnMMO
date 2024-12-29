#pragma once
#include <OgreCommon.h>

bool hlslToBin(
	const std::string& shaderName,
	const std::string& shaderContent,
	const std::string& entryPoint,
	const std::vector<std::pair<std::string, std::string>>& shaderMacros,
	Ogre::ShaderType shaderType,
	std::string& spv,
	bool vulkan = true
);