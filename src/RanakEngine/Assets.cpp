#include "RanakEngine/Assets.h"
#include "RanakEngine/Asset/AssetCache.h"
#include "RanakEngine/Asset/AudioSample.h"
#include "RanakEngine/LuaEngine.h"

#include "sol/sol.hpp"
#include <memory>

#if _WIN32
    #include <windows.h>
#include <shlobj.h>
#endif

namespace RanakEngine::Asset
{
    std::shared_ptr<Asset::Shader> GetDefaultShader()
    {
        assert(g_DefaultShader != nullptr && "DefaultShader was not initialised! Did you forget to call Asset::Init()?");
        return g_DefaultShader;
    };

    std::shared_ptr<Asset::Model> GetDefaultModel()
    {
        assert(g_DefaultModel != nullptr && "DefaultModel was not initialised! Did you forget to call Asset::Init()?");
        return g_DefaultModel;
    };

    void CreateIfNotExists(const std::string &_path, const char *_data)
    {
        CreateIfNotExists(_path, _data, strlen(_data));
    }

    void CreateIfNotExists(const std::string& _path, const char* _data, const unsigned int _size)
    {
        Log::Message("Checking for file " + _path);
        std::filesystem::path l_path(_path);
        if (!std::filesystem::exists(l_path))
        {
            Log::Message("Creating file...");
            std::filesystem::create_directories(l_path.parent_path());
            std::ofstream l_file(l_path, std::ios::binary);
            l_file.write(_data, _size);
            l_file.close();
        }
    }

    void DefineLuaLib(LuaEngine& _engine)
    {
        g_AssetTable = _engine.AddTable();

        g_AssetTable.new_usertype<Texture>("Texture");
        g_AssetTable.set_function("Texture", [](const std::string _path)
                                             { 
                                                 auto l_tex = g_AssetCache->Load<Texture>(_path);
                                                return l_tex;
                                             });

        g_AssetTable.new_usertype<Model>("Model");
        g_AssetTable.set_function("Model", [](const std::string _path)
                                           { 
                                               auto l_model = g_AssetCache->Load<Model>(_path);
                                               return l_model;
                                           });

        g_AssetTable.new_usertype<Shader>("Shader");
        g_AssetTable.set_function("Shader", [](const std::string _path)
                                            { 
                                                auto l_shader = g_AssetCache->Load<Shader>(_path);
                                                return l_shader;
                                            });

        g_AssetTable.new_usertype<AudioSample>("Audio");
        g_AssetTable.set_function("Audio", [](const std::string _path)
                                           { 
                                               auto l_audio = g_AssetCache->Load<AudioSample>(_path);
                                               return l_audio;
                                           });

        _engine.SetGlobal<sol::table>("Asset", g_AssetTable);
    }

    std::filesystem::path GetTempDir()
    {
        return std::filesystem::temp_directory_path() / "GameDevIntro";
    };

    std::filesystem::path GetDataDir()
    {
        std::filesystem::path l_base;
    #if _WIN32
        PWSTR l_appdata;
        if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &l_appdata) == S_OK)
        {
            l_base = std::filesystem::path(l_appdata);
        }
        else
        {
            fprintf(stderr, "Could not find appdata path!\n");
        }
    #else
        const char* l_xdgRaw  = std::getenv("XDG_DATA_HOME");
        const char* l_homeRaw = std::getenv("HOME");
        const std::string l_xdg  = l_xdgRaw  ? l_xdgRaw  : "";
        const std::string l_home = l_homeRaw ? l_homeRaw : "";
        if (!l_xdg.empty())
            l_base = std::filesystem::path(l_xdg);
        else if (!l_home.empty())
            l_base = std::filesystem::path(l_home) / ".local" / "share";
        else
            l_base = std::filesystem::path(".");
    #endif
        return (l_base / "RanakEngine");
    }

    std::shared_ptr<AssetCache> Init(LuaEngine& _engine)
    {
        g_AssetCache = std::make_shared<AssetCache>();

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

        CreateIfNotExists( 
                          l_defaultVertShaderPath.string(), 
                          c_defaultVertShaderData.c_str()
                         );

        CreateIfNotExists(
                          l_defaultFragShaderPath.string(), 
                          c_defaultFragShaderData.c_str()
                         );

        g_DefaultShader = g_AssetCache->Load<Shader>(l_defaultFragShaderPath.string() + 
                                                     ";" + l_defaultVertShaderPath.string())
                                                     .lock();
        
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

        CreateIfNotExists(l_defaultModelPath.string(), s_quadData.c_str());

        g_DefaultModel = g_AssetCache->Load<Model>(l_defaultModelPath.string()).lock();

        DefineLuaLib(_engine);

        return g_AssetCache;
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

        g_AssetTable.abandon();
        g_AssetCache.reset();
    }
}