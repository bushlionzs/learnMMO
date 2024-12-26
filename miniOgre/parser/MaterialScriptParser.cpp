#include "OgreHeader.h"
#include "MaterialScriptParser.h"
#include "ScriptParserUtil.h"
#include "OgreMaterialManager.h"
#include "OgreScriptParam.h"
#include "OgreString.h"
#include "myutils.h"
#include "OgreMemoryStream.h"

std::vector<String> MaterialScriptParser::getSuffix()
{
    return { ".material" };
}
void MaterialScriptParser::parseScript(ResourceInfo* res, const String& groupName)
{
    std::shared_ptr<DataStream> stream = std::make_shared<MemoryDataStream>(res);
	std::string content = stream->getAsString();
	parseMaterialImpl(content);
}

void MaterialScriptParser::parseMaterialImpl(const std::string& content)
{
    std::stringstream ss;
    ss << content;

    const std::string partMaterial = "material";
    const std::string partPbrMaterial = "pbrmaterial";
    const std::string partTechnique = "technique";

    std::string linePart;
    ss >> linePart;

    std::string materialname;

    while (!ss.eof()) {
        // Skip commented lines
        if (linePart == partComment) {
            NextAfterNewLine(ss, linePart);
            continue;
        }
        if (linePart != partMaterial &&
            linePart != partPbrMaterial) {
            ss >> linePart;
            continue;
        }

        bool pbr = false;

        if (linePart == partPbrMaterial)
        {
            pbr = true;
        }

        ss >> materialname;

        if (materialname == "SkyLan")
        {
            int kk = 0;
        }

        materialname = string_trim(materialname);
        materialname = UTF8ToGBK(materialname.c_str());

        auto mat = MaterialManager::getSingletonPtr()->create(materialname, pbr);


        OgreMaterialParam* param = new OgreMaterialParam(mat.get(), this);

        addMaterialParam(materialname, param);
        NextAfterNewLine(ss, linePart);
        if (linePart != partBlockStart) {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "material error");
            return;
        }
        ss >> linePart;
        if (pbr)
        {
            while (linePart != partBlockEnd)
            {
                if (linePart == partComment) {
                    NextAfterNewLine(ss, linePart);
                    continue;
                }

                std::string val = SkipLine(ss);
                Ogre::StringUtil::trim(val);
                param->addParam(linePart, val);
                ss >> linePart;
            }
            int kk = 0;
        }
        else
        {
            while (linePart != partBlockEnd) {
                // Proceed to the first technique
                if (linePart == partTechnique) {
                    std::string techniqueName = SkipLine(ss);
                    readTechnique(techniqueName, ss, mat.get(), param);
                    while (linePart != partBlockEnd)
                    {
                        ss >> linePart;
                    }

                    break;
                }
                else
                {
                    std::string val = SkipLine(ss);
                    Ogre::StringUtil::trim(val);
                    param->addParam(linePart, val);
                }
                ss >> linePart;
            }
        }

        

        param->compile();
        ss >> linePart;
    }
}

bool MaterialScriptParser::readTechnique(
    const std::string& techniqueName, 
    std::stringstream& ss, 
    Material* mat,
    OgreMaterialParam* param)
{
    std::string linePart;
    ss >> linePart;

    if (linePart != partBlockStart) {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "technique error");
    }


    const std::string partPass = "pass";

    while (linePart != partBlockEnd) {
        ss >> linePart;

        // Skip commented lines
        if (linePart == partComment) {
            SkipLine(ss);
            continue;
        }

        /// @todo Techniques have other attributes than just passes.
        if (linePart == partPass) {
            std::string passName = SkipLine(ss);
            readPass(passName, ss, mat, param);
            while (linePart != partBlockEnd)
            {
                ss >> linePart;
            }
            break;
        }
    }
    return true;
}

bool MaterialScriptParser::readPass(
    const std::string& passName, 
    std::stringstream& ss, 
    Material* mat,
    OgreMaterialParam* param)
{
    std::string linePart;
    ss >> linePart;

    if (linePart != partBlockStart) {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "pass error");
    }


    const std::string partAmbient = "ambient";
    const std::string partDiffuse = "diffuse";
    const std::string partSpecular = "specular";
    const std::string partEmissive = "emissive";
    const std::string partTextureUnit = "texture_unit";


    if (mat->getName() == "chuansongmen_01_03")
    {
        int kk = 0;
    }

    ss >> linePart;

    
    while (linePart != partBlockEnd) {
        // Skip commented lines
        if (linePart == partComment) {
            SkipLine(ss);
            continue;
        }

        if (linePart == partTextureUnit) {
            std::string textureUnitName = SkipLine(ss);
            readTextureUnit(textureUnitName, ss, mat, param);

        }
        else
        {
            
            std::string val = SkipLine(ss);
            Ogre::StringUtil::trim(val);

            param->addParam(linePart, val);
            //param.setParameter(linePart, val);
        }

        ss >> linePart;
    }



    return true;
}

bool MaterialScriptParser::readTextureUnit(
    const std::string& textureUnitName, 
    std::stringstream& ss, 
    Material* mat,
    OgreMaterialParam* param)
{
    std::string linePart;
    ss >> linePart;

    if (linePart != partBlockStart) {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "texture unit error");
    }


    const std::string partTexture = "texture";
    const std::string partTextCoordSet = "tex_coord_set";
    const std::string partColorOp = "colour_op";


    std::string textureRef;
    int uvCoord = 0;

    ss >> linePart;
    TextureProperty tp;

    int32_t tex_index = param->mUnitParamPairs.size();
    while (linePart != partBlockEnd) {
        // Skip commented lines
        if (linePart == partComment) {
            SkipLine(ss);
            continue;
        }

        
        {
            std::string val = SkipLine(ss);
            Ogre::StringUtil::trim(val);

            param->addUnitParam(linePart, val, tex_index);
        }

        ss >> linePart;
    }

    return true;
}

Real MaterialScriptParser::getLoadingOrder(void) const
{
	return 1.0f;
}

OgreMaterialParam* MaterialScriptParser::getMaterialParam(const std::string& name)
{
    auto itor = mMaterialParamMap.find(name);
    if (itor != mMaterialParamMap.end())
    {
        return itor->second;
    }

    return nullptr;
}

void MaterialScriptParser::addMaterialParam(const std::string& name, OgreMaterialParam* param)
{
    mMaterialParamMap[name] = param;
}