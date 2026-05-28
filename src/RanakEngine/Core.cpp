#include "RanakEngine/Core.h"

#include "RanakEngine/Core/ScriptRegistry.h"
#include "RanakEngine/LuaEngine.h"
#include "sol/sol.hpp"
#include <string>

namespace RanakEngine::Core
{
    namespace
    {
        static std::shared_ptr<Core::Manager> CoreManager;
        static sol::table CoreTable;
    }

    std::string GetAppName()
    {
        assert(CoreManager != nullptr && "CoreManager is not initialised! Did you forget to call Core::Init()?");
        return CoreManager->GetAppName();
    }

    void DefineLuaLib(LuaEngine& _engine)
    {
        CoreTable = _engine.AddTable();

        CoreTable.set_function("DeltaTime", [](){return CoreManager->DeltaTime();});

        std::weak_ptr<Camera> l_camera = CoreManager->GetCamera();
        CoreTable.set("Camera", l_camera.lock());

        std::shared_ptr<Scene> l_scene = CoreManager->GetScene().lock();
        CoreTable.set("CurrentScene", l_scene);

        CoreTable.set("AppName", CoreManager->GetAppName());

        _engine.SetGlobal<sol::table>("Core", CoreTable);
    }

    std::shared_ptr<Core::Manager> Init(bool _isDebug, std::string _appName, LuaEngine& _engine)
    {
        CoreManager = Core::Manager::Init(_isDebug, _appName, _engine);
        return CoreManager;
    }

    void Stop()
    {
        CoreTable.abandon();
        CoreManager.reset();
    }
}