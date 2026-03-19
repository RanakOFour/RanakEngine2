#include "RanakEngine/Core/EntityRegistry.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/Category.h"

namespace RanakEngine::Core
{
    EntityRegistry::EntityRegistry()
    : m_nextFreeId(-1)
    , m_freeIds()
    , m_categories()
    , m_entityBitset()
    {
        m_luaContext = LuaContext::Instance().lock();
        m_dataTable = m_luaContext->CreateTable();
        m_dataTable.create_named("Entities");
        m_dataTable.create_named("Categories");
    }

    EntityRegistry::~EntityRegistry()
    {
    }

    int EntityRegistry::AddEntity()
    {
        int l_newID{-1};

        if (!m_freeIds.empty())
        {
            l_newID = m_freeIds.back();
            m_freeIds.pop_back();
        }
        else
        {
            l_newID = ++m_nextFreeId;
        }

        sol::table l_newEntityTable = m_dataTable.raw_get<sol::table>("Entities").create_named(l_newID);
        l_newEntityTable.create_named("attributes");

        // Add to transform
        auto l_signature = m_luaContext->GetCategory("Transform")
                                .lock()->GetSignature();
        AddToCategory(l_newID, l_signature);

        return l_newID;
    }

    void EntityRegistry::RemoveEntity(int _id)
    {
        m_idsToDelete.push_back(_id);

        m_dataTable.raw_get<sol::table>("Entities").raw_set(_id, sol::nil);
        
        DeleteFlaggedEntities();
    }

    void EntityRegistry::DeleteFlaggedEntities()
    {
        for (int l_id : m_idsToDelete)
        {
            auto l_idSignature = m_entityBitset[l_id];

            for (auto l_pair : m_categories)
            {
                // If there is a match
                if ((l_idSignature & l_pair.first).any())
                {
                    l_pair.second->RemoveMember(l_id);
                }
            }

            m_entityBitset.erase(l_id);

            m_freeIds.push_back(l_id);
        }

        m_freeIds.erase(m_freeIds.begin(), m_freeIds.end());
    }

    void EntityRegistry::AddToCategory(int _id, std::bitset<1024> _signature)
    {
        // Add to registry if not logged already
        if (m_categories.find(_signature) == m_categories.end())
        {
            // Ask context for registered category
            std::shared_ptr<Category> l_categoryPtr = m_luaContext->GetCategory(_signature).lock();

            if (l_categoryPtr.get() != nullptr)
            {
                // Add valid ptr to category map
                m_categories[_signature] = l_categoryPtr;
                m_dataTable.raw_get<sol::table>("Categories")
                    .set(l_categoryPtr->GetName(), l_categoryPtr.get());
            }
            else
            {
                return;
            }
        }

        // Category is registered
        // Edit entityData in table
        m_dataTable.traverse_raw_get<sol::table>(
                       "Entities",
                       _id,
                       "attributes")
            .set(m_categories[_signature]->GetName(), m_categories[_signature]->AddMember(_id));

        // Update entity signature
        m_entityBitset[_id] |= _signature;
    }

    void EntityRegistry::RemoveFromCategory(int _id, std::bitset<1024> _signature)
    {
        // Category is registered
        if (m_categories.find(_signature) != m_categories.end())
        {
            m_categories[_signature]->RemoveMember(_id);
            // There's probably a faster way to do this, but I haven't found it yet
            m_dataTable.traverse_raw_get<sol::table>(
                           "Entities",
                           _id,
                           "attributes",
                           m_categories[_signature]->GetName())
                .abandon();

            // The registry does not have to manage empty categories
            if (m_categories[_signature]->GetSize() == 0)
            {
                m_categories.erase(_signature);
            }
        }

        // Update entity signature
        m_entityBitset[_id] ^= _signature;
    }

    sol::table EntityRegistry::GetEntityTable()
    {
        return m_dataTable.raw_get<sol::table>("Entities");
    }

    sol::table EntityRegistry::GetCategoryTable()
    {
        return m_dataTable.raw_get<sol::table>("Categories");
    }

    std::weak_ptr<Category> EntityRegistry::GetCategory(std::bitset<1024> _signature)
    {
        if (m_categories.find(_signature) != m_categories.end())
        {
            return m_categories[_signature];
        }

        return std::weak_ptr<Category>();
    }

    std::weak_ptr<Category> EntityRegistry::GetCategory(std::string _name)
    {
        for(auto& l_pair : m_categories)
        {
            if(l_pair.second->GetName() == _name)
            {
                return l_pair.second;
            }
        }

        return std::weak_ptr<Category>();
    }

    std::vector<int> EntityRegistry::GetEntitiesWith(std::bitset<1024> _combinedSignature)
    {
        std::vector<int> l_entities;
        for (auto l_pair : m_entityBitset)
        {
            // entity has signature if the signature is a subset of the entity's
            // Therefore, it is also correct if the signature and not(entity sig) is empty
            if ((_combinedSignature & ~l_pair.second).none())
            {
                l_entities.push_back(l_pair.first);
            }
        }

        return l_entities;
    }

    sol::table EntityRegistry::GetEntityAttributes(int _id)
    {
        return m_dataTable.traverse_raw_get<sol::table>("Entities", _id, "attributes");
    }

    bool EntityRegistry::Empty()
    {
        return m_entityBitset.empty();
    }

    int EntityRegistry::GetEntityCount()
    {
        return m_entityBitset.size();
    }
}