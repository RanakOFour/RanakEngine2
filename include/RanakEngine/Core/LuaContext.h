#ifndef LUACONTEXT_H
#define LUACONTEXT_H

#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Rule.h"

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

#include <memory>
#include <map>

namespace RanakEngine::Core
{
    class CategoryFactory;

    /**
     * @class LuaContext
     * @brief Singleton wrapper around a sol2 Lua state that serves as the scripting runtime.
     *
     * LuaContext is the sole owner of the sol::state.  All C++ types that need to
     * be usable from Lua (Category, Rule, Vector2, etc.) are registered here via
     * DefineUsertype() calls at initialisation.
     *
     * It provides type-safe script execution through the RunScript<T>() template,
     * which compiles scripts on first call and caches the load_result for subsequent
     * frames.  Category creation/reloading is also routed here so that the
     * CategoryFactory remains an implementation detail.
     *
     * Access the instance with LuaContext::Instance(); create it once per session
     * with LuaContext::Init().
     */
    class LuaContext
    {
    private:
        inline static std::weak_ptr<LuaContext> m_self; ///< Weak self-reference for the singleton pattern.
        sol::state                                       m_state;        ///< The underlying Lua VM.
        std::map<std::string, sol::protected_function>  m_loadedScripts; ///< Path -> compiled chunk cache (registry-backed, stable across stack changes).
        std::shared_ptr<Core::CategoryFactory>   m_categoryFactory; ///< Owned category registry.

        LuaContext();

    public:
        ~LuaContext();

        /** @brief Creates the singleton LuaContext and registers all engine usertypes. */
        static std::shared_ptr<LuaContext> Init();
        /** @brief Returns a weak pointer to the singleton instance. */
        static std::weak_ptr<LuaContext> Instance();

        /**
         * @brief Compiles and caches the Lua script without executing it.
         * @param _file Weak pointer to the LuaFile to compile.
         */
        void LoadScript(std::weak_ptr<Asset::LuaFile> _file);

        /**
         * @brief Executes the Lua script and returns the last expression as type T.
         *
         * The script is compiled on first call and cached.  On subsequent calls only
         * the cached chunk is re-executed.  When T is void nothing is returned.
         *
         * @tparam T Return type expected from the script (use void for side-effect scripts).
         * @param _file Weak pointer to the LuaFile to run.
         * @return Script result cast to T.
         */
        template <typename T>
        T RunScript(std::weak_ptr<Asset::LuaFile> _file)
        {
            auto l_file = _file.lock();
            std::string l_path = l_file->GetPath();

            if (m_loadedScripts.find(l_path) == m_loadedScripts.end())
            {
                LoadScript(_file);
            }

            // LoadScript logs an error and returns without inserting on failure.
            if (m_loadedScripts.find(l_path) == m_loadedScripts.end())
            {
                if constexpr (std::is_void<T>::value)
                    return;
                else
                    return T{};
            }

            sol::protected_function_result l_result = m_loadedScripts[l_path]();

            if (!l_result.valid())
            {
                sol::error l_err = l_result;
                printf("Error running script \"%s\":\n%s\n", l_path.c_str(), l_err.what());
                // Return a default value; do NOT call l_result.get<T>() on an
                // invalid result — doing so throws a sol2 exception that escapes
                // through the Lua/C++ boundary and breaks the entire call chain.
                if constexpr (std::is_void<T>::value)
                    return;
                else
                    return T{};
            }

            // Return on void
            if constexpr (std::is_void<T>::value)
            {
                return;
            }

            // Get result type of there is a result
            T l_toReturn = l_result.get<T>();
            return l_toReturn;
        };

        sol::table CreateTable();

        /**
         * @brief Compiles and executes _code as Lua, registers the resulting Category.
         * @param _code Raw Lua source string defining the category.
         * @return Weak pointer to the newly registered Category.
         */
        std::weak_ptr<Core::Category> CreateCategory(const std::string _code);
        /**
         * @brief Runs _file, registers the returned Category, and associates it with the file.
         * @param _file Weak pointer to the LuaFile containing the category definition.
         * @return Weak pointer to the newly registered Category.
         */
        std::weak_ptr<Core::Category> CreateCategory(std::weak_ptr<Asset::LuaFile> _file);
        /**
         * @brief Looks up a Category by its signature.
         * @param _signature Unique 1024-bit signature of the target Category.
         * @return Weak pointer to the Category, or expired if not found.
         */
        std::weak_ptr<Core::Category> GetCategory(std::bitset<1024> _signature);
        /**
         * @brief Looks up a Category by name.
         * @param _name Category name string.
         * @return Weak pointer to the Category, or expired if not found.
         */
        std::weak_ptr<Core::Category> GetCategory(std::string _name);

        /**
         * @brief Runs _file as a Lua script and returns the resulting Rule object.
         * @param _file Weak pointer to the LuaFile containing the rule definition.
         * @return Constructed Rule ready to be added to a Scene.
         */
        Rule CreateRule(std::weak_ptr<Asset::LuaFile> _file);

        /**
         * @brief Hot-reloads a Category by re-running its source file.
         * @param _category Weak pointer to the Category to reload.
         */
        void ReloadCategory(std::weak_ptr<Core::Category> _category);

        /**
         * @brief Exposes a C++ variable to Lua under the given global name.
         * @tparam T Variable type (must be usable by sol2).
         * @param _name Global name in Lua.
         * @param _var  Reference to the C++ variable.
         */
        template <typename T>
        void AddVariable(std::string _name, T &_var)
        {
            m_state[_name] = &_var;
        };

        /**
         * @brief Registers a C++ usertype with the Lua state.
         * @tparam T      Type to register.
         * @tparam Args   Forwarded arguments for sol::state::new_usertype.
         */
        template<typename T, typename... Args>
        void AddUserType(Args&&... _args)
        {
            m_state.new_usertype<T>(std::forward<Args>(_args)...);
        }

        /**
         * @brief Sets a named global in the Lua state to the given sol::object.
         * @param _name Global variable name.
         * @param _obj  The sol::object value.
         */
        void SetGlobal(std::string _name, sol::object &_obj);

        /** @brief Returns the global Lua table (_G) as a sol::table. */
        sol::table GetGlobalTable();

        /** @brief Returns a raw pointer to the underlying sol::state (for advanced use). */
        sol::state *GetState() { return &m_state; };

        /**
         * @brief Returns a semi-colon separated list of all registered Category names.
         * @return Names separated by ';'.
         */
        std::string GetCategoryNames();
    };
}

#endif