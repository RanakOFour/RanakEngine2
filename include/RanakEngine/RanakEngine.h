#pragma once
#ifndef RANAKENGINE_H
#define RANAKENGINE_H

#include <memory>

#include "RanakEngine/Math.h"
#include "RanakEngine/Log.h"
#include "RanakEngine/Assets.h"
#include "RanakEngine/Core.h"
#include "RanakEngine/IO.h"
#include "RanakEngine/Physics.h"

namespace RanakEngine
{
    /**
     * @struct EngineContents
     * @brief Aggregates the shared pointers for all engine subsystem managers.
     *
     * Returned by Initialise() and passed to Shutdown().  Callers store this
     * struct for the lifetime of the application and forward relevant managers
     * to the editor or game layer as needed.
     */
    struct EngineContents
    {
        std::shared_ptr<Asset::Manager>    resources; ///< Asset management subsystem.
        std::shared_ptr<Core::Manager>     core;      ///< Core ECR + scripting subsystem.
        std::shared_ptr<IO::Manager>       io;        ///< Window, input, and audio subsystem.
        std::shared_ptr<Physics::Manager>  physics;   ///< Physics simulation subsystem.
    };

    /**
     * @brief Initialises all engine subsystems in dependency order and returns their handles.
     *
     * Equivalent to calling Log::Init(), Math::DefineLuaLib(), Asset::Init(),
     * Core::Init(), and IO::Init() in the correct sequence.
     *
     * @param _debug      When true enables verbose engine logging and debug overlays.
     * @param _screenSize Initial window size in pixels.
     * @return EngineContents struct populated with all live manager shared pointers.
     */
    EngineContents Initialise(bool _debug, Vector2 _screenSize);

    /**
     * @brief Shuts down all engine subsystems in reverse dependency order.
     *
     * Calls Stop() on each subsystem and clears the shared pointers in _contents
     * so that reference counts reach zero and destructors run cleanly.
     *
     * @param _contents Reference to the EngineContents returned by Initialise().
     */
    void Shutdown(EngineContents &_contents);
}

namespace RE = RanakEngine;

#endif