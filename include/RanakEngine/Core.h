#ifndef RANAKCORE_H
#define RANAKCORE_H

#include "RanakEngine/Core/Raycast.h"
#include "RanakEngine/Core/Camera.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/CoreManager.h"
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Core/Scene.h"
#include "RanakEngine/Core/Rule.h"

#include "sol/sol.hpp"

namespace RanakEngine::Core
{
    /** @brief Returns the application name string set at Core::Manager initialisation. */
    std::string GetAppName();

    /** @brief Registers all Core Lua bindings (Scene, Rule, Camera, …) with the LuaContext. */
    void DefineLuaLib();

    /**
     * @brief Creates the Core::Manager singleton, LuaContext, and main Camera.
     * @param _isDebug When true enables verbose engine logging.
     * @return Shared pointer to the initialised Core::Manager.
     */
    std::shared_ptr<Core::Manager> Init(bool _isDebug, std::string _appName);
    /** @brief Stops the core update loop and releases engine resources. */
    void Stop();
}

#endif