#ifndef RANAKLOG_H
#define RANAKLOG_H

#include "RanakEngine/Log/LogManager.h"
#include "sol/sol.hpp"

namespace RanakEngine::Log
{
    namespace
    {
        std::shared_ptr<Log::Manager> LogManager;
    };

    inline static void Message(std::string _message)
    {
        LogManager->LogMessage(Log::MessageContent::NORMAL, _message);
    }

    inline static void Debug(std::string _message)
    {
        LogManager->LogMessage(Log::MessageContent::DEBUG, _message);
    }

    inline static void Warning(std::string _message)
    {
        LogManager->LogMessage(Log::MessageContent::WARNING, _message);
    }

    inline static void Error(std::string _message)
    {
        LogManager->LogMessage(Log::MessageContent::ERRORLOG, _message);
    }

    std::string Table(sol::table _tableData, int _index);

    void DefineLuaLib();

    void Init();
    void Stop();
}

#endif