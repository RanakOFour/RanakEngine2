#ifndef RANAKLOG_H
#define RANAKLOG_H

#include "RanakEngine/Log/LogManager.h"
#include "sol/sol.hpp"

namespace RanakEngine
{
    class LuaEngine;
}

namespace RanakEngine::Log
{
    /** @brief Logs a normal informational message. */
    void Message(std::string _message);

    /** @brief Logs a debug-level message (only shown when debug mode is enabled). */
    void Debug(std::string _message);

    /** @brief Logs a warning message. */
    void Warning(std::string _message);

    /** @brief Logs an error message. */
    void Error(std::string _message);

    /**
     * @brief Converts a Lua table to a human-readable string for logging.
     * @param _tableData The Lua table to serialise.
     * @param _index     Current nesting depth (0 for the outermost call).
     * @return String representation of the table.
     */
    std::string Table(sol::table _tableData, int _index);

    /** @brief Registers Log Lua bindings (log.message, log.warning, …) with the given LuaEngine. */
    void DefineLuaLib(LuaEngine& _engine);

    /** @brief Creates the Log::Manager singleton. */
    void Init();
    /** @brief Flushes and closes the log, releasing all resources. */
    void Stop();
}

#endif