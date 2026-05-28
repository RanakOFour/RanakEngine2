#include "RanakEngine/RanakEngine.h"

namespace RanakEngine
{
    EngineContents Initialise(bool _debug, Vector2 _screenSize, std::string _appName)
    {
        EngineContents l_toReturn{};

        Log::Init();

        // The Lua runtime is heap-allocated so its address stays stable when this
        // struct is returned by value; subsystems (e.g. Core::ScriptRegistry) hold
        // a long-lived reference to it.
        l_toReturn.luaEngine = std::make_shared<LuaEngine>();
        LuaEngine& l_engine = *l_toReturn.luaEngine;

        // IO first: it creates the window + GL context that Asset::Init needs to
        // build the default shader/model, and that the Camera queries on creation.
        l_toReturn.io = IO::Init(_screenSize);
        l_toReturn.assets = Asset::Init(l_engine);
        l_toReturn.core = Core::Init(_debug, _appName, l_engine);
        l_toReturn.physics = Physics::Init();

        UI::Init(l_toReturn.io);

        Math::DefineLuaLib(l_engine);
        Core::DefineLuaLib(l_engine);
        IO::DefineLuaLib(l_engine);
        Log::DefineLuaLib(l_engine);
        Physics::DefineLuaLib(l_engine);
        UI::DefineLuaLib(l_engine);

        return l_toReturn;
    };

    void Shutdown(EngineContents &_contents)
    {
        UI::Stop();
        Physics::Stop();

        Log::Message("Stopping IO");
        IO::Stop();

        Log::Message("Stopping Asset");
        Asset::Stop();

        Math::Stop();

        Log::Message("Stopping Log and Core");

        // Core shuts down Lua, which should be the last thing to stop since other subsystems
        // have Lua tables that need to be cleaned up.

        // Core also shuts down Log
        Core::Stop();
    }
}