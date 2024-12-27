#include <OgreHeader.h>
#include <platform_file.h>
#include "glslUtil.h"
#include "hlslUtil.h"
#include "myutils.h"
#include <libshaderc_util/file_finder.h>
#include <VulkanTools.h>
#include <VulkanHelper.h>
#include <shaderManager.h>
#include <mutex>
#include "OgreResourceManager.h"
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_hlsl.hpp>



class MyIncluderInterface : public shaderc::CompileOptions::IncluderInterface
{
public:
    struct shaderc_include_result_private
    {
        shaderc_include_result result;
        std::string content;
    };
    MyIncluderInterface()
    {
    }
    virtual shaderc_include_result* GetInclude(const char* requested_source,
        shaderc_include_type type,
        const char* requesting_source,
        size_t include_depth)
    {
        ResourceInfo* resInfo = ResourceManager::getSingleton().getResource(requested_source);
        assert(resInfo);
        auto& name = resInfo->_fullname;

        auto context = new shaderc_include_result_private;

        auto* result = &context->result;
        result->source_name = name.c_str();
        result->source_name_length = name.size();

        get_file_content(name.c_str(), context->content);
        result->content = context->content.c_str();
        result->content_length = context->content.length();
        return result;
    }

    // Handles shaderc_include_result_release_fn callbacks.
    virtual void ReleaseInclude(shaderc_include_result* data)
    {
        shaderc_include_result_private* context = (shaderc_include_result_private*)data;
        delete context;
    }
};

std::string getGlslKey(
    const std::string& shaderName,
    const std::vector<std::pair<std::string, std::string>>& shaderMacros,
    Ogre::ShaderType shaderType)
{
    uint64_t macro_bit = 0;
    std::vector<std::string> keys;
    for (auto& pair : shaderMacros)
    {
        keys.push_back(pair.first);
        keys.push_back(pair.second);
    }

    keys.push_back(std::to_string(shaderType));

    std::sort(keys.begin(), keys.end());

    std::string result = shaderName;
    for (auto& key : keys)
    {
        result += key;
    }
    return result;
}

struct ShaderContent
{
    VkShaderModule shaderModule;
    std::string spv;
    std::vector<GlslInputDesc> inputDesc;
};
static std::unordered_map<std::string, ShaderContent> gShaderCacheMap;

static std::mutex gShaderMutex;

bool glslCompileShader(
    const std::string& shaderName,
    const std::string& shaderContent,
    const std::string& entryPoint,
	const std::vector<std::pair<std::string, std::string>>& shaderMacros,
    VkShaderModuleInfo& shaderModuleInfo,
    bool createModule
)
{
    std::string key = getGlslKey(shaderName, shaderMacros, shaderModuleInfo.shaderType);

    {
        std::unique_lock<std::mutex> lck(gShaderMutex);
        auto itor = gShaderCacheMap.find(key);

        if (itor != gShaderCacheMap.end())
        {
            shaderModuleInfo.shaderModule = itor->second.shaderModule;
            shaderModuleInfo.inputDesc = itor->second.inputDesc;
            shaderModuleInfo.spv = itor->second.spv;
            return true;
        }
    }
    
    std::string result;
    const char* suffix = getSuffix(shaderName);
    if (strcmp(suffix, ".glsl") == 0)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetTargetSpirv(shaderc_spirv_version_1_4);



        options.SetIncluder(std::make_unique<MyIncluderInterface>());
        // Like -DMY_DEFINE=1
        for (auto& pair : shaderMacros)
        {
            options.AddMacroDefinition(pair.first, pair.second);
        }

        shaderc_shader_kind kind;
        switch (shaderModuleInfo.shaderType)
        {
        case Ogre::ShaderType::VertexShader:
            options.AddMacroDefinition("VERTEX_SHADER", "1");
            kind = shaderc_glsl_vertex_shader;
            break;
        case Ogre::ShaderType::PixelShader:
            options.AddMacroDefinition("FRAGMENT_SHADER", "1");
            kind = shaderc_glsl_fragment_shader;
            break;
        case Ogre::ShaderType::GeometryShader:
            options.AddMacroDefinition("GEOMETRY_SHADER", "1");
            kind = shaderc_glsl_geometry_shader;
            break;
        case Ogre::ShaderType::ComputeShader:
            kind = shaderc_glsl_compute_shader;
            break;
        case Ogre::ShaderType::RayGenShader:
            kind = shaderc_glsl_raygen_shader;
            break;
        case Ogre::ShaderType::MissShader:
            kind = shaderc_glsl_miss_shader;
            break;
        case Ogre::ShaderType::AnyHitShader:
            kind = shaderc_glsl_anyhit_shader;
            break;
        case Ogre::ShaderType::ClosestHitShader:
            kind = shaderc_glsl_closesthit_shader;
            break;
        default:
            assert_invariant(false);
        }


        shaderc::SpvCompilationResult module =
            compiler.CompileGlslToSpv(
                shaderContent,
                kind,
                shaderName.c_str(),
                entryPoint.c_str(),
                options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::string aa = module.GetErrorMessage();
            assert(false);
            return false;
        }


        std::vector<uint32_t> aa = { module.cbegin(), module.cend() };

        
        result.resize(aa.size() * sizeof(uint32_t));
        memcpy((void*)result.data(), aa.data(), aa.size() * sizeof(uint32_t));
    }
    else
    {
        //hlsl
        
         hlslToSpirv(shaderName, shaderContent, 
             entryPoint, shaderMacros, shaderModuleInfo.shaderType, result);
         std::string hlslSource;

         spvToHlsl(shaderModuleInfo.shaderType, shaderName, result, hlslSource);
         std::string glslSource;
         //spvToGlsl(shaderModuleInfo.shaderType, shaderName, result, glslSource);
         int kk = 0;
    }

    VkShaderModule shader = VK_NULL_HANDLE;
    if (createModule)
    {
        auto device = VulkanHelper::getSingleton().getDevcie();
        shader = vks::tools::loadShaderMemory(result, device);
        if (VK_NULL_HANDLE == shader)
        {
            return false;
        }
    }
    
    {
        std::unique_lock<std::mutex> lck(gShaderMutex);
        auto itor = gShaderCacheMap.find(key);
        if (itor == gShaderCacheMap.end())
        {
            gShaderCacheMap[key].shaderModule = shader;
            parserGlslInputDesc(result, shaderModuleInfo.inputDesc);
            gShaderCacheMap[key].inputDesc = shaderModuleInfo.inputDesc;
            gShaderCacheMap[key].spv = result;
            shaderModuleInfo.shaderModule = shader;
            shaderModuleInfo.spv = result;
        }
    }

    
    return true;
}

void parserGlslInputDesc(
    const std::string& code,
    std::vector<GlslInputDesc>& inputDesc)
{
    spirv_cross::CompilerGLSL  glsl((const uint32_t*)code.data(), code.size()/4);

    auto inputs = glsl.get_shader_resources().stage_inputs;

    int32_t size = inputs.size();
    inputDesc.resize(size);

    for (int32_t i = 0; i < size; i++) 
    {
        auto& input = inputs[i];
        inputDesc[i]._name = glsl.get_name(input.id);
        inputDesc[i]._index = 0;
        assert(!inputDesc[i]._name.empty());

        if (inputDesc[i]._name.back() >= '0' && inputDesc[i]._name.back() <= '9')
        {
            inputDesc[i]._index = inputDesc[i]._name.back() - '0';
            inputDesc[i]._name.pop_back();
        }

        inputDesc[i]._location = glsl.get_decoration(input.id, spv::DecorationLocation);

        inputDesc[i]._type = glsl.get_type(input.type_id);


        auto offset = glsl.get_decoration(input.id, spv::DecorationOffset);
        auto binding = glsl.get_decoration(input.id, spv::DecorationBinding);
        int kk = 0;
    }
}


void spvToHlsl(
    Ogre::ShaderType shaderType, 
    const std::string& name,
    const std::string& code, 
    std::string& hlslSource)
{
    std::vector<uint32_t> spirv;

    spirv.resize(code.size() / 4);

    memcpy(spirv.data(), code.c_str(), code.size());

    spirv_cross::CompilerHLSL compiler(spirv);

    // 获取着色器反射信息
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();
    std::string original_name;
    for (auto& ub : resources.uniform_buffers)
    {
        original_name = compiler.get_name(ub.base_type_id);
        original_name.erase(original_name.size()-6, 6);
        compiler.set_name(ub.base_type_id, original_name); // 确保使用原始名称
    }
    // 设置编译选项
    spirv_cross::CompilerHLSL::Options options;
    options.shader_model = 51; // 对应 HLSL Shader Model 5.1
    
    compiler.set_hlsl_options(options);
    SPIRV_CROSS_NAMESPACE::HLSLVertexAttributeRemap attr;

    std::string shortname = getShortFilename(name);
    if (shaderType == ShaderType::VertexShader)
    {
        {
            const auto& semanticList = ShaderManager::getSingleton().getVertexInputMapping(shortname);

            for (auto& obj : semanticList)
            {
                attr.location = obj.location;
                attr.semantic = obj.semantic;
                compiler.add_vertex_attribute_remap(attr);
            }
        }

        {
            const auto& semanticList = ShaderManager::getSingleton().getVertexOutputMapping(shortname);

            for (auto& obj : semanticList)
            {
                attr.location = obj.location;
                attr.semantic = obj.semantic;
                compiler.add_vertex_attribute_remap_output(attr);
            }
        }
       
    }
    else if (shaderType == ShaderType::PixelShader)
    {
        const auto& semanticList = ShaderManager::getSingleton().getPixelInputMapping(shortname);

        for (auto& obj : semanticList)
        {
            attr.location = obj.location;
            attr.semantic = obj.semantic;
            compiler.add_vertex_attribute_remap(attr);
        }
    }
    
    // 编译 HLSL 代码
    hlslSource = compiler.compile();
}


void spvToGlsl(
    Ogre::ShaderType shaderType,
    const std::string& name,
    const std::string& code,
    std::string& glslSource)
{
    std::vector<uint32_t> spirv;

    spirv.resize(code.size() / 4);

    memcpy(spirv.data(), code.c_str(), code.size());

    spirv_cross::CompilerGLSL compiler(spirv);

    // 获取着色器反射信息
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();
    std::string original_name;
    for (auto& ub : resources.uniform_buffers)
    {
        original_name = compiler.get_name(ub.base_type_id);
    }
    // 设置编译选项
    spirv_cross::CompilerGLSL::Options options;


    std::string shortname = getShortFilename(name);
    

    // 编译 HLSL 代码
    glslSource = compiler.compile();
}

bool translateToHlsl(
    const std::string& shaderName,
    const std::string& shaderContent,
    const std::string& entryPoint,
    const std::vector<std::pair<std::string, std::string>>& shaderMacros,
    Ogre::ShaderType shaderType,
    std::string& hlslSource
)
{
    VkShaderModuleInfo moduleInfo;
    moduleInfo.shaderType = shaderType;
    bool compile = glslCompileShader(shaderName, shaderContent, entryPoint, 
        shaderMacros, moduleInfo, false);
    if (!compile)
    {
        return false;
    }
    spvToHlsl(shaderType, shaderName, moduleInfo.spv, hlslSource);
    return true;
}