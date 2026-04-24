#include "RanakEngine/Assets.h"

#include "RanakEngine/Core/LuaContext.h"
#include "sol/sol.hpp"

namespace RanakEngine::Asset
{
    namespace
    {
        static sol::table AssetTable;
    }

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

    std::shared_ptr<Asset::Manager> Init()
    {
        auto l_manager = Asset::Manager::Init();
        AssetManager = Asset::Manager::Instance().lock();

        DefaultShader = std::make_shared<Asset::Shader>();

        DefaultShader->LoadFromString(
            "#version 430 core\n"
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
            "}\n",
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
            "}\n");

        DefaultModel = std::make_shared<Asset::Model>();

        // Unit quad (1×1, centred at origin), two triangles, 6 vertices.
        // Each vertex: pos.xyz, uv.xy, normal.xyz (8 floats)
        static constexpr float s_quadVerts[] = {
            // pos              uv       normal
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, 0.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
             0.5f,  0.5f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
             0.5f,  0.5f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
            -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        };
        DefaultModel->LoadFromArray(s_quadVerts, sizeof(s_quadVerts) / sizeof(float));

        return l_manager;
    }

    void Stop()
    {
        AssetTable.abandon();
        AssetManager.reset();
    }
}