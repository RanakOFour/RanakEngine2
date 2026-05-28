#ifndef SCRIPTREGISTRY_H
#define SCRIPTREGISTRY_H

#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/LuaEngine.h"
#include "RanakEngine/Log.h"

#include "sol/sol.hpp"

#include <memory>
#include <bitset>
#include <string>
#include <type_traits>

namespace RanakEngine
{
    class LuaEngine;

    namespace Asset
    {
        class LuaScript;
    }
}

namespace RanakEngine::Core
{
    class Category;
    class CategoryFactory;

    /**
     * @class ScriptRegistry
     * @brief Core-side scripting facade that owns the CategoryFactory and the
     *        Category/Rule creation logic.
     *
     * Successor to the former Core::LuaContext.  It no longer owns the sol::state;
     * instead it wraps a reference to the engine-wide LuaEngine (owned by
     * EngineContents) and routes all script execution and table creation through
     * it.  All C++ types that need to be usable from Lua (Category, Rule, Camera,
     * Scene) are registered with the LuaEngine at construction time.
     *
     * Access the instance with ScriptRegistry::Instance(); create it once per
     * session with ScriptRegistry::Init(luaEngine).
     */
    class ScriptRegistry
    {
    private:
        inline static std::weak_ptr<ScriptRegistry> m_self; ///< Weak self-reference for the singleton pattern.
        LuaEngine* m_engine = nullptr;                       ///< Shared, app-owned Lua runtime (not owned here).
        std::shared_ptr<Core::CategoryFactory> m_categoryFactory; ///< Owned category registry.

        explicit ScriptRegistry(LuaEngine& _engine);

    public:
        ~ScriptRegistry();

        /** @brief Creates the singleton ScriptRegistry and registers all Core usertypes. */
        static std::shared_ptr<ScriptRegistry> Init(LuaEngine& _engine);
        /** @brief Returns a weak pointer to the singleton instance. */
        static std::weak_ptr<ScriptRegistry> Instance();

        /** @brief Returns a fresh empty Lua table from the underlying engine. */
        sol::table CreateTable();

        /** @brief Returns the underlying LuaEngine. */
        LuaEngine& GetEngine();

        /**
         * @brief Exposes a C++ variable to Lua under the given global name.
         * @tparam T    Variable type (must be usable by sol2).
         * @param _name Global name in Lua.
         * @param _var  Reference to the C++ variable.
         */
        template <typename T>
        void AddVariable(std::string _name, T& _var)
        {
            m_engine->SetGlobal<T>(_name, _var);
        };

        /**
         * @brief Executes the Lua script file and returns the last expression as type T.
         * @tparam T Return type expected from the script (use void for side-effect scripts).
         * @param _file Weak pointer to the LuaScript to run.
         */
        template <typename T>
        T RunScript(std::weak_ptr<Asset::LuaScript> _file)
        {
            auto l_file = _file.lock();
            if (!l_file)
            {
                Log::Warning("RunScript: received expired file pointer.");
                if constexpr (std::is_void_v<T>) return; else return T{};
            }

            if constexpr (std::is_void_v<T>)
            {
                m_engine->RunScriptFile(_file);
                return;
            }
            else
            {
                sol::optional<T> l_opt = m_engine->RunScriptFile<T>(_file);
                if (!l_opt.has_value())
                {
                    Log::Error("RunScript: script did not return the expected type.");
                    return T{};
                }
                return l_opt.value();
            }
        };

        /**
         * @brief Compiles and executes _code as Lua, registers the resulting Category.
         * @param _code Raw Lua source string defining the category.
         * @return Weak pointer to the newly registered Category.
         */
        std::weak_ptr<Core::Category> CreateCategory(const std::string _code);
        /**
         * @brief Runs _file, registers the returned Category, and associates it with the file.
         * @param _file Weak pointer to the LuaScript containing the category definition.
         * @return Weak pointer to the newly registered Category.
         */
        std::weak_ptr<Core::Category> CreateCategory(std::weak_ptr<Asset::LuaScript> _file);
        /**
         * @brief Looks up a Category by its signature.
         * @param _signature Unique 1024-bit signature of the target Category.
         */
        std::weak_ptr<Core::Category> GetCategory(std::bitset<1024> _signature);
        /**
         * @brief Looks up a Category by name.
         * @param _name Category name string.
         */
        std::weak_ptr<Core::Category> GetCategory(std::string _name);

        /**
         * @brief Runs _file as a Lua script and returns the resulting Rule object.
         * @param _file Weak pointer to the LuaScript containing the rule definition.
         */
        Rule CreateRule(std::weak_ptr<Asset::LuaScript> _file);

        /**
         * @brief Hot-reloads a Category by re-running its source file.
         * @param _category Weak pointer to the Category to reload.
         */
        void ReloadCategory(std::weak_ptr<Core::Category> _category);

        /** @brief Returns a semi-colon separated list of all registered Category names. */
        std::string GetCategoryNames();
    };
}

#endif
