#include "RanakEngine/RanakEngine.h"

#include <filesystem>

namespace RanakEngine
{
    EngineContents Initialise(bool _debug, Vector2 _screenSize)
    {
        EngineContents l_toReturn;

        Log::Init();

        l_toReturn.resources = Asset::Init();
        l_toReturn.io = IO::Init(_screenSize);
        l_toReturn.core = Core::Init(_debug);
        // l_toReturn.physics = Physics::Init();

        Math::DefineLuaLib();
        Asset::DefineLuaLib();
        Core::DefineLuaLib();
        IO::DefineLuaLib();
        Log::DefineLuaLib();

        // Load transform category (Required by registry to exist)
        auto l_transformFile = l_toReturn.resources->Load<Asset::LuaFile>("./resources/Categories/Transform.lua");
        if (l_transformFile.lock() == nullptr)
        {
            printf("Could not find transform file\n");
            std::string l_cwd = std::filesystem::current_path().generic_string();
            printf("CWD: %s", l_cwd.c_str());
        }
        l_toReturn.core->GetLuaContext()->CreateCategory(l_transformFile);

        return l_toReturn;
    };

    void Shutdown(EngineContents &_contents)
    {
        // Physics::Stop();

        Log::Message("Stopping IO");
        IO::Stop();

        Log::Message("Stopping Asset");
        Asset::Stop();

        Log::Message("Stopping Log");
        Log::Stop();

        Math::Stop();

        Log::Message("Stopping Core");
        Core::Stop();
    }
}