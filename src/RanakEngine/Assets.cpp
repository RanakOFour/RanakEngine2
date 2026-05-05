#include "RanakEngine/Assets.h"

#include "RanakEngine/Core/LuaContext.h"
#include "sol/sol.hpp"

namespace RanakEngine::Asset
{
    namespace
    {
        static std::shared_ptr<Asset::Manager> AssetManager; ///< Module-private singleton handle (set by Init()).
        static std::shared_ptr<Asset::Shader> DefaultShader; ///< Default shader used for drawing entities without an explicit shader.
        static std::shared_ptr<Asset::Model> DefaultModel;   ///< Default model used for drawing entities without an explicit model.
        static sol::table AssetTable;
    }

    std::shared_ptr<Asset::Shader> GetDefaultShader()
    {
        assert(DefaultShader != nullptr && "DefaultShader was not initialised! Did you forget to call Asset::Init()?");
        return DefaultShader;
    };

    std::shared_ptr<Asset::Model> GetDefaultModel()
    {
        assert(DefaultModel != nullptr && "DefaultModel was not initialised! Did you forget to call Asset::Init()?");
        return DefaultModel;
    };

    void DefineLuaLib()
    {
        auto l_context = Core::LuaContext::Instance().lock();

        AssetTable = l_context->CreateTable();

        AssetTable.new_usertype<Texture>("Texture");
        AssetTable.set_function("Texture", [](const std::string _path)
                                             { 
                                                 auto l_tex = AssetManager->Load<Texture>(_path);
                                                return l_tex;
                                             });

        AssetTable.new_usertype<Model>("Model");
        AssetTable.set_function("Model", [](const std::string _path)
                                           { 
                                               auto l_model = AssetManager->Load<Model>(_path);
                                               return l_model;
                                           });

        AssetTable.new_usertype<Shader>("Shader");
        AssetTable.set_function("Shader", [](const std::string _path)
                                            { 
                                                auto l_shader = AssetManager->Load<Shader>(_path);
                                                return l_shader;
                                            });

        AssetTable.new_usertype<Audio>("Audio");
        AssetTable.set_function("Audio", [](const std::string _path)
                                           { 
                                               auto l_audio = AssetManager->Load<Audio>(_path);
                                               return l_audio;
                                           });

        l_context->SetGlobal("Asset", AssetTable);
    }

    std::filesystem::path GetTempDir()
    {
        return std::filesystem::temp_directory_path() / "GameDevIntro";
    };

    std::shared_ptr<Asset::Manager> Init()
    {
        AssetManager = Asset::Manager::Init();

        const std::string c_defaultVertShaderData = "#version 430 core\n"
            "in vec3 a_Position;\n"
            "in vec2 a_PixelColor;\n"
            "uniform mat4 u_Projection;\n"
            "uniform mat4 u_View;\n"
            "uniform mat4 u_Model;\n"
            "out vec2 v_texCoord;\n"

            "void main()\n"
            "{\n"
            "    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);\n"
            "    v_texCoord = a_PixelColor;\n"
            "}\n";

        const std::string c_defaultFragShaderData =
            "#version 430 core\n"
            "in vec2 v_texCoord;\n"
            "uniform sampler2D u_Texture;\n"
            "\n"
            "out vec4 o_fragColor;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    vec4 tex = texture(u_Texture, v_texCoord);\n"
            "    o_fragColor = tex;\n"
            "}\n";

        std::filesystem::path l_tempDir = GetTempDir();

        std::filesystem::path l_defaultVertShaderPath = l_tempDir / "Shaders" / "REDefaultVertShader.vs";
        std::filesystem::path l_defaultFragShaderPath = l_tempDir / "Shaders" / "REDefaultFragShader.fs";

        if(!std::filesystem::exists(l_defaultVertShaderPath))
        {
            std::filesystem::create_directories(l_defaultVertShaderPath.parent_path());

            std::ofstream l_fileWriter(l_defaultVertShaderPath);
            l_fileWriter << c_defaultVertShaderData;
            l_fileWriter.close();

            l_fileWriter.open(l_defaultFragShaderPath);
            l_fileWriter << c_defaultFragShaderData;
            l_fileWriter.close();
        }

        DefaultShader = std::make_shared<Asset::Shader>(l_defaultFragShaderPath.string() + ";" + l_defaultVertShaderPath.string());

        const std::string s_quadData =      "o Plane\n"
                                            "v -1.000000 -1.000000 -0.000000\n"
                                            "v 1.000000 -1.000000 -0.000000\n"
                                            "v -1.000000 1.000000 0.000000\n"
                                            "v 1.000000 1.000000 0.000000\n"
                                            "vn -0.0000 -0.0000 1.0000\n"
                                            "vt 0.000000 0.000000\n"
                                            "vt 1.000000 0.000000\n"
                                            "vt 1.000000 1.000000\n"
                                            "vt 0.000000 1.000000\n"
                                            "s 0\n"
                                            "usemtl \n"
                                            "f 1/1/1 2/2/1 4/3/1 3/4/1";

        std::filesystem::path l_defaultModelPath = l_tempDir / "Models" / "REDefaultModel.obj";

        if(!std::filesystem::exists(l_defaultModelPath))
        {
            std::filesystem::create_directories(l_defaultModelPath.parent_path());
            std::ofstream l_fileWriter(l_defaultModelPath);
            l_fileWriter << s_quadData;
            l_fileWriter.close();
        }

        DefaultModel = std::make_shared<Asset::Model>(l_defaultModelPath.string());

        return AssetManager;
    }

    void Stop()
    {
        std::filesystem::path l_tempDir = GetTempDir();

        std::filesystem::path l_defaultModelPath = l_tempDir / "REDefaultModel.obj";
        if(std::filesystem::exists(l_defaultModelPath))
        {
            std::filesystem::remove(l_defaultModelPath);
        }

        std::filesystem::path l_defaultVertShaderPath = l_tempDir / "REDefaultVertShader.vs";
        if(std::filesystem::exists(l_defaultVertShaderPath))
        {
            std::filesystem::remove(l_defaultVertShaderPath);
        }

        std::filesystem::path l_defaultFragShaderPath = l_tempDir / "REDefaultFragShader.fs";
        if(std::filesystem::exists(l_defaultFragShaderPath))
        {
            std::filesystem::remove(l_defaultFragShaderPath);
        }

        AssetTable.abandon();
        AssetManager.reset();
    }
}