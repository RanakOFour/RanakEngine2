#ifndef RANAKIO_H
#define RANAKIO_H

#include "RanakEngine/IO/MouseInfo.h"
#include "RanakEngine/IO/KBInfo.h"
#include "RanakEngine/IO/Audio.h"
#include "RanakEngine/IO/Window.h"
#include "RanakEngine/IO/IOManager.h"

namespace RanakEngine::IO
{
    /** @brief Registers IO Lua bindings (input queries, audio playback, …) with the LuaContext. */
    void DefineLuaLib();
    /**
     * @brief Creates the IO::Manager singleton, Window, and Audio subsystems.
     * @param _screenSize Initial window dimensions in pixels.
     * @return Shared pointer to the initialised IO::Manager.
     */
    std::shared_ptr<IO::Manager> Init(Vector2 _screenSize);
    /** @brief Closes the window and releases all IO resources. */
    void Stop();
}

#endif