#ifndef LUAENGINE_H
#define LUAENGINE_H

#include "RanakEngine/Asset/LuaScript.h"
#include "sol/sol.hpp"
#include <memory>
#include <stdexcept>


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

        /**
         * @brief Registers a C++ usertype with the Lua state under the given name.
         *
         * Thin forwarder to sol::state::new_usertype so callers can register
         * usertypes without the LuaEngine exposing its underlying sol::state.
         *
         * @tparam T    Type to register.
         * @tparam Args Forwarded arguments for sol::state::new_usertype.
         */
        template<typename T, typename... Args>
        void AddUserType(Args&&... _args)
        {
            m_state.new_usertype<T>(std::forward<Args>(_args)...);
        };

        sol::object GetGlobal(std::string _name);

        /**
        * @brief Takes in raw lua script and runs it in the engine.
        * @param _scriptText The string containing lua code that is to be executed.
        */
        void RunScript(std::string _scriptText);
        void RunScriptFile(std::weak_ptr<Asset::LuaScript> _file);
        
        
        /**
        * @brief Takes in raw lua script with an expected return type, executes the program, and returns the expected value.
        * @tparam T Expected return type
        * @param _scriptText String containing lua code that is to be executed
        */

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
                    
                    throw std::runtime_error(((sol::error)l_funcResult).what());
                }
                else
                {
                    throw std::runtime_error(((sol::error)l_loadResult).what());
                }
            }
            catch (sol::error _e)
            {
                printf("[LuaEngine::RunScript]: %s", _e.what());
            }

            return sol::nullopt;
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
