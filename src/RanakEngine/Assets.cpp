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
        AssetManager = Asset::Manager::Init();
        return AssetManager;
    }

    void Stop()
    {
        AssetTable.clear();
        AssetManager.reset();
    }
}