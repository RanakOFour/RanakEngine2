#include "RanakEngine/RanakEngine.h"
#include "RanakEngine/Physics.h"

namespace RanakEngine
{
    EngineContents Initialise(bool _debug, Vector2 _screenSize)
    {
        EngineContents l_toReturn;

        Log::Init();

        l_toReturn.io = IO::Init(_screenSize);
        l_toReturn.core = Core::Init(_debug);
        l_toReturn.physics = Physics::Init();
        l_toReturn.resources = Asset::Init();

        UI::Init(*l_toReturn.io);

        Math::DefineLuaLib();
        Asset::DefineLuaLib();
        Core::DefineLuaLib();
        IO::DefineLuaLib();
        Log::DefineLuaLib();
        Physics::DefineLuaLib();
        UI::DefineLuaLib();

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