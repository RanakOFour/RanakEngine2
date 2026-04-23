#ifndef RANAKLOG_H
#define RANAKLOG_H

#include "RanakEngine/Log/LogManager.h"
#include "sol/sol.hpp"

namespace RanakEngine::Log
{
    namespace
    {
        std::shared_ptr<Log::Manager> LogManager; ///< Module-private singleton handle (set by Init()).
    };

    /**
     * @brief Logs a normal informational message.
     * @param _message Message text to log.
     */
    inline static void Message(std::string _message)
    {
        LogManager->LogMessage(Log::MessageContent::NORMAL, _message);
    }

    /**
     * @brief Logs a debug-level message (only shown when debug mode is enabled).
     * @param _message Message text to log.
     */
    inline static void Debug(std::string _message)
    {
        LogManager->LogMessage(Log::MessageContent::DEBUG, _message);
    }

    /**
     * @brief Logs a warning message.
     * @param _message Message text to log.
     */
    inline static void Warning(std::string _message)
    {
        LogManager->LogMessage(Log::MessageContent::WARNING, _message);
    }

    /**
     * @brief Logs an error message.
     * @param _message Message text to log.
     */
    inline static void Error(std::string _message)
    {
        LogManager->LogMessage(Log::MessageContent::ERRORLOG, _message);
    }

    /**
     * @brief Converts a Lua table to a human-readable string for logging.
     * @param _tableData The Lua table to serialise.
     * @param _index     Current nesting depth (0 for the outermost call).
     * @return String representation of the table.
     */
    std::string Table(sol::table _tableData, int _index);

    /** @brief Registers Log Lua bindings (log.message, log.warning, …) with the LuaContext. */
    void DefineLuaLib();

    /** @brief Creates the Log::Manager singleton. */
    void Init();
    /** @brief Flushes and closes the log, releasing all resources. */
    void Stop();
}

#endif