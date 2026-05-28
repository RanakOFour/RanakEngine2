#include "RanakEngine/LuaEngine.h"
#include "sol/sol.hpp"

namespace RanakEngine
{
    LuaEngine::LuaEngine()
    : m_state()
    {
        // Opens all basic libraries
        m_state.open_libraries(
                               sol::lib::base,
                               sol::lib::table,
                               sol::lib::string
                              );

        // Field(default, opts) — wraps a default value with optional editor metadata.
        // CloneTable extracts only the default when creating per-entity data.
        m_state.set_function("Field", [this](sol::object _default, sol::optional<sol::table> _opts) {
            sol::table l_field = m_state.create_table();
            l_field["__isField"] = true;
            l_field["default"]   = _default;
            if (_opts.has_value())
            {
                l_field["opts"] = *_opts;
            }
            return l_field;
        });
    }

    sol::table LuaEngine::AddTable()
    {
        return m_state.create_table();
    }

    sol::table LuaEngine::AddTable(std::string _name)
    {
        return m_state.create_named_table(_name);
    }

    sol::object LuaEngine::GetGlobal(std::string _name)
    {
        return m_state[_name];
    }

    void LuaEngine::RunScript(std::string _code)
    {
        try
        {
            sol::load_result l_loadResult = m_state.load(_code);
            if(l_loadResult.valid())
            {
                sol::protected_function_result l_funcResult = ((sol::protected_function)l_loadResult)();
                if(l_funcResult.valid())
                {
                    return;
                }
                
                throw ((sol::error)l_funcResult).what();
            }
            else
            {
                throw ((sol::error)l_loadResult).what();
            }
        }
        catch (sol::error _e)
        {
            printf("[LuaEngine::RunScript]: %s", _e.what());
        }
    }

    void LuaEngine::RunScriptFile(std::weak_ptr<Asset::LuaScript> _file)
    {
        auto l_filePtr = _file.lock();
        return RunScript(l_filePtr->GetContents());
    }
}