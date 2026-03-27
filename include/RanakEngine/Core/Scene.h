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
    class Scene
    {
        friend LuaContext;
        private:
        std::weak_ptr<LuaContext> m_luaContext;

        std::string m_name;
        sol::table m_sceneTable;

        std::weak_ptr<Camera> m_camera;
        
        EntityRegistry m_registry;

        std::vector<std::shared_ptr<Rule>> m_rules;

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
        Scene(sol::table _tableData);
        ~Scene();

        int AddEntity();
        void RemoveEntity(int _id);

        void AddToCategory(int _id, std::string _categoryName);
        void RemoveFromCategory(int _id, std::string _categoryName);

        sol::table GetAttributesOf(int _id);

        void RenameEntity(int _id, const std::string _newName);

        std::vector<int> GetEntitiesWith(const std::vector<std::string>& _categoryNames);

        void AddRule(Rule& _rule);
        void RemoveRule(Rule& _rule);

        void Init();
        void Update(float _deltaTime);
        void Draw();

        // Returns id of hit entity and RaycastHit info if more is needed.
        // Returns -1 is no entity was hit.
        int Raycast(Ray& _ray, RaycastHit& _out);

        EntityRegistry& GetRegistry();
        std::string GetName() const { return m_name; }
        sol::table GetEntityTable();
        sol::table GetSceneTable();
    };
};
};

#endif