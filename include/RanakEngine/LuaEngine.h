#ifndef LUAENGINE_H
#define LUAENGINE_H

#include "RanakEngine/Asset/LuaScript.h"
#include "sol/sol.hpp"
#include <memory>


namespace RanakEngine
{
    class LuaEngine
    {
        private:
        sol::state m_state;

        public:
        LuaEngine();
        ~LuaEngine();

        sol::table AddTable();
        sol::table AddTable(std::string _name);

        template<typename T>
        void SetGlobal(std::string _name, T _data)
        {
            m_state[_name] = _data;
        };

        sol::object GetGlobal(std::string _name);

        void RunScript(std::string _scriptText);
        void RunScriptFile(std::weak_ptr<Asset::LuaScript> _file);

        template<typename T>
        inline sol::optional<T> RunScript(std::string _scriptText)
        {
            try
            {
                sol::load_result l_loadResult = m_state.load(_scriptText);
                if(l_loadResult.valid())
                {
                    sol::protected_function_result l_funcResult = ((sol::protected_function)l_loadResult)();
                    if(l_funcResult.valid())
                    {
                        return l_funcResult.get<sol::optional<T>>();
                    }
                    
                    throw l_funcResult;
                }
                else
                {
                    throw l_loadResult;
                }
            }
            catch (sol::error _e)
            {
                printf("[LuaEngine::RunScript]: %s", _e.what());
            }
        }

        template <typename T>
        inline sol::optional<T> RunScriptFile(std::weak_ptr<Asset::LuaScript> _file)
        {
            auto l_filePtr = _file.lock();
            return RunScript<T>(l_filePtr->GetContents());
        }
    };
}

#endif