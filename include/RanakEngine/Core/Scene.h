#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include <memory>

#include "RanakEngine/Core/EntityRegistry.h"
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Core/LuaContext.h"
#include "sol/sol.hpp"

namespace RanakEngine
{
    class LuaContext;

namespace Core
{
    struct Ray;
    struct RaycastHit;
    class Camera;
    class LuaContext;

    /**
     * @class Scene
     * @brief Aggregates the entity registry, active rules, and camera for one game scene.
     *
     * A Scene is the top-level runtime container.  It owns an EntityRegistry that
     * tracks live entities and their category membership, a list of active Rules
     * that are dispatched each frame, and a weak reference to the main Camera.
     *
     * Scenes can be constructed from Lua:
     * @code{.lua}
     *   myScene = Scene()
     * @endcode
     * and then passed to CoreManager::SetScene().
     *
     * Rule lifecycle within a Scene:
     * - AddRule()  → Rule::Init() called once, then stored for future frames.
     * - Update()   → Rule::Update() called on all active rules.
     * - Draw()     → Rule::Draw() called on all active rules.
     * - RemoveRule() → Rule removed from the list.
     */
    class Scene
    {
        friend LuaContext;
        private:
        std::weak_ptr<LuaContext> m_luaContext; ///< Used to resolve category names during add/remove.

        std::string m_name;        ///< Scene name (optional identifier).
        sol::table  m_sceneTable;  ///< Lua-side representation of this scene.

        std::weak_ptr<Camera> m_camera; ///< The camera used when rendering this scene.

        EntityRegistry m_registry; ///< All entities and their category memberships.

        std::vector<std::shared_ptr<Rule>> m_rules; ///< Rules dispatched each frame.

        static void DefineUsertype(sol::state& _state)
        {
            _state.new_usertype<Scene>("Scene", sol::constructors<Scene(), Scene(sol::table)>(),
                                        "name", sol::property(&Scene::GetName),
                                        "addEntity", &Scene::AddEntity,
                                        "removeEntity", &Scene::RemoveEntity,
                                        "addToCategory", &Scene::AddToCategory,
                                        "removeFromCategory", &Scene::RemoveFromCategory,
                                        "getAttributesOf", &Scene::GetAttributesOf,
                                        "addRule", &Scene::AddRule,
                                        "removeRule", &Scene::RemoveRule,
                                        "getEntitiesWith", &Scene::GetEntitiesWith,
                                        "getEntityTable", &Scene::GetEntityTable
                                        );
        };

        public:
        Scene();

        /** @brief Constructs a Scene, initialising it with the data from a Lua table. */
        Scene(sol::table _tableData);
        ~Scene();

        /**
         * @brief Creates a new entity in the registry and returns its ID.
         * @return Newly allocated entity ID.
         */
        int AddEntity();
        /**
         * @brief Marks the entity for deferred removal from the registry.
         * @param _id Entity ID to remove.
         */
        void RemoveEntity(int _id);

        /**
         * @brief Adds the entity to the named Category, creating the per-entity field table.
         * @param _id           Entity ID.
         * @param _categoryName Name of the Category to join.
         */
        void AddToCategory(int _id, std::string _categoryName);
        /**
         * @brief Removes the entity from the named Category.
         * @param _id           Entity ID.
         * @param _categoryName Name of the Category to leave.
         */
        void RemoveFromCategory(int _id, std::string _categoryName);

        /**
         * @brief Returns all category field tables for the given entity as a single Lua table.
         * @param _id Entity ID.
         */
        sol::table GetAttributesOf(int _id);

        /**
         * @brief Renames an entity in the registry's name map.
         * @param _id      Entity ID.
         * @param _newName New human-readable name.
         */
        void RenameEntity(int _id, const std::string _newName);

        /**
         * @brief Returns all entity IDs that belong to every Category listed.
         * @param _categoryNames List of required Category names.
         * @return Entity IDs that are members of all specified categories.
         */
        std::vector<int> GetEntitiesWith(const std::vector<std::string>& _categoryNames);

        /**
         * @brief Adds a Rule to the scene; calls Rule::Init() immediately.
         * @param _rule Rule to add (copied into a shared_ptr).
         */
        void AddRule(Rule& _rule);
        /**
         * @brief Removes the rule with the same name as _rule from the scene.
         * @param _rule Rule to remove (matched by name).
         */
        void RemoveRule(Rule& _rule);

        /** @brief Calls Init on all rules — typically once after scene setup. */
        void Init();
        /**
         * @brief Calls Update on all active rules.
         * @param _deltaTime Time elapsed since the last frame in seconds.
         */
        void Update(float _deltaTime);
        /** @brief Calls Draw on all active rules. */
        void Draw();

        /**
         * @brief Tests _ray against every entity's AABB and returns the first hit.
         * @param _ray Ray to cast.
         * @param _out Populated with information about the intersection point.
         * @return ID of the hit entity, or -1 if nothing was hit.
         */
        int Raycast(Ray& _ray, RaycastHit& _out);

        /** @brief Returns a reference to the scene's EntityRegistry. */
        EntityRegistry& GetRegistry();
        /** @brief Returns the scene's name. */
        std::string GetName() const { return m_name; }
        /** @brief Returns the Lua-side entity data table. */
        sol::table GetEntityTable();
        /** @brief Returns the Lua-side scene table. */
        sol::table GetSceneTable();
    };
};
};

#endif