#ifndef RULE_H
#define RULE_H

#include <string>
#include <vector>
#include <memory>

#include "RanakEngine/Core/EntityRegistry.h"

#include "sol/sol.hpp"

namespace RanakEngine::Asset
{
    class LuaFile;
}

namespace RanakEngine::Core
{
    class LuaContext;
    class Scene;

    /**
     * @class Rule
     * @brief Represents an ECR system (logic script) that operates on matching entities each frame.
     *
     * A Rule is created by running a Lua script that returns a Rule table constructed
     * with the call-constructor syntax:
     * @code{.lua}
     *   return Rule {
     *       categories = { "Moveable", "Physics" },
     *       Init   = function(self, data) ... end,
     *       Update = function(self, data, dt) ... end,
     *       Draw   = function(self, data) ... end,
     *   }
     * @endcode
     *
     * At runtime Scene calls Init, Update, and Draw each frame.  For every entity
     * whose combined signature includes all the rule's required category bits, the
     * corresponding Lua function is invoked with that entity's data table.
     */
    class Rule
    {
        friend Scene;
        friend LuaContext;
        private:
        std::weak_ptr<LuaContext>    m_context;    ///< Scripting context used during Init/Update/Draw dispatch.
        std::weak_ptr<Asset::LuaFile> m_originFile; ///< The .lua file this rule was loaded from.

        bool        m_active;     ///< When false the rule is skipped each frame.
        std::string m_name;       ///< Human-readable rule name (set from the Lua filename).
        std::bitset<1024>          m_signature;  ///< Combined signature of all required Categories.
        std::vector<std::string>   m_categories; ///< Names of required Categories (used to build m_signature).

        sol::protected_function m_initFunction;   ///< Lua Init callback.
        sol::protected_function m_updateFunction; ///< Lua Update callback.
        sol::protected_function m_drawFunction;   ///< Lua Draw callback.
        sol::table              m_table;          ///< Shared rule-level data table visible to all callbacks.

        /**
         * @brief Registers the Rule usertype with the given Lua state so scripts can construct Rules.
         * @param _state Reference to the sol::state to register with.
         */
        static void DefineUsertype(sol::state& _state)
        {
            _state.new_usertype<Rule>("Rule",
                                              sol::call_constructor,
                                              sol::factories([](sol::table _opts) -> Rule {
                                                  Rule r;
                                                  sol::optional<std::vector<std::string>> cats = _opts.get<sol::optional<std::vector<std::string>>>("categories");
                                                  if (cats.has_value()) r.m_categories = *cats;
                                                  sol::optional<sol::table> fields = _opts.get<sol::optional<sol::table>>("fields");
                                                  if (fields.has_value()) r.m_table = *fields;
                                                  return r;
                                              }),
                                              "name", &Rule::m_name,
                                              "categories", &Rule::m_categories,
                                              "fields", &Rule::m_table,
                                              "Init", &Rule::m_initFunction,
                                              "Update", &Rule::m_updateFunction,
                                              "Draw", &Rule::m_drawFunction,
                                              "getCategories", sol::readonly(&Rule::GetCategories),
                                              "isActive", sol::readonly(&Rule::GetActive),
                                              "setActive", sol::readonly(&Rule::SetActive)
                                    );
        }

        /**
         * @brief Builds m_signature by OR-ing together the signatures of all required Categories.
         *
         * Called once by LuaContext::CreateRule() after the rule table has been
         * populated from Lua.
         */
        void CreateSignature();

        public:
        Rule();
        ~Rule();

        /**
         * @brief Calls the Lua Init function once for every matching entity.
         * @param _registry The scene's EntityRegistry used to enumerate entities.
         */
        void Init(EntityRegistry& _registry);
        /**
         * @brief Calls the Lua Update function once per frame for every matching entity.
         * @param _registry The scene's EntityRegistry.
         */
        void Update(EntityRegistry& _registry);
        /**
         * @brief Calls the Lua Draw function once per frame for every matching entity.
         * @param _registry The scene's EntityRegistry.
         */
        void Draw(EntityRegistry& _registry);

        /**
         * @brief Enables or disables this rule.
         * @param _a True to activate, false to deactivate.
         */
        void SetActive(bool _a);
        /** @brief Returns true when this rule is active. */
        bool GetActive();

        /** @brief Returns the rule's name. */
        std::string GetName();
        /** @brief Returns the combined Category signature this rule requires. */
        std::bitset<1024> GetCategories();
        /** @brief Returns the rule-level shared data table. */
        sol::table GetData();

        /**
         * @brief Associates this rule with the .lua file it was loaded from.
         * @param _file Weak pointer to the source LuaFile.
         */
        void SetOriginFile(std::weak_ptr<Asset::LuaFile> _file);
        /** @brief Returns a weak pointer to the source .lua file, if any. */
        std::weak_ptr<Asset::LuaFile> GetOriginFile();
    };
}

#endif