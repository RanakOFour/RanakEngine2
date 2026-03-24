#include "RanakEngine/Core/Scene.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Core/Raycast.h"

#include "RanakEngine/Log.h"

namespace RanakEngine::Core
{
    Scene::Scene()
    : m_name("Scene")
    , m_registry()
    , m_rules()
    {
        auto l_luaContext = LuaContext::Instance().lock();
        m_sceneTable = l_luaContext->CreateTable();
        m_sceneTable.set("Entities", m_registry.GetEntityTable());
        m_sceneTable.set("Categories", m_registry.GetCategoryTable());
        m_sceneTable.set("Rules", l_luaContext->CreateTable());
    }

    Scene::Scene(sol::table _tableData)
    : m_name()
    , m_registry()
    , m_rules()
    , m_sceneTable(_tableData)
    {
        m_name = _tableData.raw_get<std::string>("name");
        auto l_luaContext = LuaContext::Instance().lock();
        l_luaContext->AddVariable<sol::table>("Scene", m_sceneTable);
    }

    Scene::~Scene()
    {
    }

    int Scene::AddEntity()
    {
        return m_registry.AddEntity();
    }

    int Scene::AddEntity(std::bitset<1024> _signature)
    {
        int l_newEntityID = m_registry.AddEntity();
        m_registry.AddToCategory(l_newEntityID, _signature);

        return l_newEntityID;
    }

    void Scene::AddEntityToCategory(int _id, std::bitset<1024> _signature)
    {
        m_registry.AddToCategory(_id, _signature);
    }

    void Scene::RemoveEntity(int _id)
    {
        m_registry.RemoveEntity(_id);
    }

    void Scene::RenameEntity(int _id, const std::string _newName)
    {
        sol::table l_entity = m_registry.GetEntityTable().raw_get<sol::table>(_id);
        l_entity["name"] = _newName;
    }

    void Scene::RemoveCategory(std::bitset<1024> _signature)
    {
        std::vector<int> l_entities = m_registry.GetEntitiesWith(_signature);

        for (int l_entity : l_entities)
        {
            m_registry.RemoveFromCategory(l_entity, _signature);
        }
    }

    void Scene::AddRule(Rule &_rule)
    {
        _rule.CreateSignature();
        m_rules.push_back(_rule);
        m_sceneTable.raw_get<sol::table>("Rules").raw_set(_rule.GetName(), _rule);
    }

    void Scene::RemoveRule(Rule &_rule)
    {
        std::string l_name = _rule.GetName();
        for (int i = 0; i < m_rules.size(); i++)
        {
            if (m_rules[i].GetName() == l_name)
            {
                m_rules.erase(m_rules.begin() + i);
                break;
            }
        }

        m_sceneTable.raw_get<sol::table>("Rules").raw_set(_rule.GetName(), sol::lua_nil);
    }

    void Scene::Init()
    {
        auto l_contextPtr = m_luaContext.lock();

        for (Rule& l_rule : m_rules)
        {
            l_rule.CreateSignature();
            l_rule.Init(m_registry);
        }
    }

    void Scene::Update(float _dt)
    {
        auto l_contextPtr = m_luaContext.lock();

        for (Rule& l_rule : m_rules)
        {
            l_rule.Update(m_registry);
        }
    }

    void Scene::Draw()
    {
        auto l_contextPtr = m_luaContext.lock();

        for(Rule& l_rule : m_rules)
        {
            l_rule.Draw(m_registry);
        }
    }

    int Scene::Raycast(Ray& _ray, RaycastHit& _out)
    {
        std::map<float, float> l_hitEntities;
        sol::table l_entities = m_registry.GetEntityTable();

        auto l_pairs = l_entities.pairs();

        for(auto& l_entity : l_pairs)
        {
            // Just AABB it

            sol::table l_transform = l_entity.second.as<sol::table>().traverse_raw_get<sol::table>("attributes", "Transform");
            Vector3 l_position(l_transform.raw_get<Vector2>("Position"), l_transform.raw_get<float>("Layer"));
            Vector3 l_scale(l_transform.raw_get<Vector2>("Scale"), 0.5f);

            Vector3 l_min = l_position - (l_scale);
            Vector3 l_max = l_position + (l_scale);

            bool l_hit = AABBIntersection(_ray, l_min, l_max, _out.hitInfo);

            if(l_hit)
            {
                l_hitEntities[l_entity.first.as<float>()] = _out.hitInfo.distance;
            }
        }

        if(l_hitEntities.size() == 0)
        {
            return -1;
        }

        float l_closestEntity = (*(l_hitEntities.begin())).first;

        for(auto& l_entity : l_hitEntities)
        {
            if(l_entity.second < l_hitEntities[l_closestEntity])
            {
                l_closestEntity = l_entity.first;
            }
        }

        return (int)l_closestEntity;
    }

    EntityRegistry* Scene::GetRegistry()
    {
        return &m_registry;
    }

    sol::table Scene::GetSceneTable()
    {
        return m_sceneTable;
    }
}