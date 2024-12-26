#include "OgreHeader.h"
#include "ParticleScriptParser.h"
#include "ScriptParserUtil.h"
#include "OgreParticleSystemManager.h"
#include "OgreParticleSystem.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleAffector.h"
#include "myutils.h"
#include "OgreString.h"
#include "shaderManager.h"
#include "OgreMemoryStream.h"
std::vector<String> ParticleScriptParser::getSuffix()
{
    return { ".particle" };
}
void ParticleScriptParser::parseScript(ResourceInfo* res, const String& groupName)
{
    std::shared_ptr<MemoryDataStream> stream = std::make_shared<MemoryDataStream>(res);
    std::string content = stream->getAsString();
    parseParticleImpl(content);
}

Real ParticleScriptParser::getLoadingOrder(void) const
{
	return 1.0f;
}

void ParticleScriptParser::parseParticleImpl(const std::string& content)
{
    std::stringstream ss;
    ss << content;

    std::string linePart;
    ss >> linePart;


    const std::string partParticle = "particle_system";

    std::string particlename;
    while (!ss.eof()) {
        // Skip commented lines
        if (linePart == partComment) {
            NextAfterNewLine(ss, linePart);
            continue;
        }
        if (linePart != partParticle) {
            ss >> linePart;
            continue;
        }

        ss >> particlename;

        particlename = string_trim(particlename);
        particlename = UTF8ToGBK(particlename.c_str());
        SkipLine(ss);

        ParticleSystem* particle =
            ParticleSystemManager::getSingletonPtr()->createTemplate(particlename, BLANKSTRING);

        readParticle(ss, particle);

        ss >> linePart;
    }
}

bool ParticleScriptParser::readParticle(std::stringstream& ss, ParticleSystem* particle)
{
    std::string linePart;
    ss >> linePart;

    if (linePart != partBlockStart) {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "particle error");
    }

    ss >> linePart;

    std::string right;
    while (linePart != partBlockEnd)
    {
        if (linePart == partComment) {
            NextAfterNewLine(ss, linePart);
            continue;
        }

        if (linePart.empty())
        {
            NextAfterNewLine(ss, linePart);
            continue;
        }



        if (linePart == "emitter")
        {
            std::string emitterType;
            ss >> emitterType;

            auto emitter =
                particle->addEmitter(emitterType);
            SkipLine(ss);

            readEmitter(ss, emitter);

            ss >> linePart;
        }
        else if (linePart == "affector")
        {
            std::string affectorType;
            ss >> affectorType;

            auto affector =
                particle->addAffector(affectorType);
            SkipLine(ss);

            readAffector(ss, affector);

            ss >> linePart;
        }
        else
        {
            right = SkipLine(ss);

            Ogre::StringUtil::trim(right);

            bool bset = particle->setParameter(linePart, right);

            if (!bset)
            {
                auto render = particle->getRenderer();
                bset = render->setParameter(linePart, right);
            }

            assert(bset);

            ss >> linePart;
        }
    }

    SkipLine(ss);
    return true;
}

bool ParticleScriptParser::readEmitter(std::stringstream& ss, ParticleEmitter* emitter)
{
    std::string linePart;
    ss >> linePart;

    if (linePart != partBlockStart) {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "emitter error");
    }

    ss >> linePart;

    std::string right;
    while (linePart != partBlockEnd)
    {
        if (linePart == partComment) {
            NextAfterNewLine(ss, linePart);
            continue;
        }

        if (linePart.empty())
        {
            NextAfterNewLine(ss, linePart);
            continue;
        }


        right = SkipLine(ss);
        Ogre::StringUtil::trim(right);
        bool bset = emitter->setParameter(linePart, right);

        assert(bset);

        ss >> linePart;
    }

    SkipLine(ss);
    return true;
}

bool ParticleScriptParser::readAffector(std::stringstream& ss, ParticleAffector* affector)
{
    std::string linePart;
    ss >> linePart;

    if (linePart != partBlockStart) {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "affector error");
    }

    ss >> linePart;

    std::string right;
    while (linePart != partBlockEnd)
    {
        if (linePart == partComment) {
            NextAfterNewLine(ss, linePart);
            continue;
        }

        if (linePart.empty())
        {
            NextAfterNewLine(ss, linePart);
            continue;
        }


        right = SkipLine(ss);
        Ogre::StringUtil::trim(right);
        bool bset = affector->setParameter(linePart, right);
        assert(bset);
        ss >> linePart;
    }
    SkipLine(ss);
    return true;
}