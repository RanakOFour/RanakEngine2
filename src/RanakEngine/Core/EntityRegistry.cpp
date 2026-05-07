#include "RanakEngine/Core/EntityRegistry.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Log.h"

namespace RanakEngine::Core
{
    EntityRegistry::EntityRegistry()
    : m_nextFreeId(-1)
    , m_freeIds()
    , m_categories()
    , m_entityBitset()
    {
        m_luaContext = LuaContext::Instance();
        m_dataTable = m_luaContext.lock()->CreateTable();
        m_dataTable.create_named("Entities");
        m_dataTable.create_named("Categories");
    }

    EntityRegistry::~EntityRegistry()
    {
        // Remove all entities from their category data tables so that global
        // Category objects don't accumulate orphaned entries across scene changes.
        auto l_context = m_luaContext.lock();
        if (!l_context) return;

        for (const auto& l_pair : m_entityBitset)
        {
            int l_id = l_pair.first;
            const std::bitset<1024>& l_sig = l_pair.second;
            for (int i = 0; i < 1024; ++i)
            {
                if (l_sig.test(i))
                {
                    std::bitset<1024> l_catSig;
                    l_catSig.set(i);
                    auto l_cat = l_context->GetCategory(l_catSig).lock();
                    if (l_cat)
                        l_cat->RemoveMember(l_id);
                }
            }
        }
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
        l_newEntityTable["name"] = "Entity" + std::to_string(l_newID);
        l_newEntityTable.create_named("attributes");

        // Add to transform
        auto l_transformCategory = m_luaContext.lock()->GetCategory("Transform").lock();
        
        if(l_transformCategory)
        {
            AddToCategory(l_newID, l_transformCategory->GetSignature());
        }

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
            auto l_context = m_luaContext.lock();
            
            for (int i = 0; i < 1024; ++i)
            {
                if (l_idSignature.test(i))
                {
                    std::bitset<1024> l_sig;
                    l_sig.set(i);
                    auto l_cat = l_context->GetCategory(l_sig).lock();
                    if (l_cat)
                    {
                        l_cat->RemoveMember(l_id);
                    }
                }
            }

            m_entityBitset.erase(l_id);
            m_freeIds.push_back(l_id);
        }

        m_idsToDelete.clear();

        // Rebuild active categories set (expensive but only after deletions)
        UpdateActiveCategories();
    }

    void EntityRegistry::UpdateActiveCategories()
    {
        m_activeCategories.reset();
        for (const auto& pair : m_entityBitset)
        {
            m_activeCategories |= pair.second;
        }
    }

    void EntityRegistry::AddToCategory(int _id, std::bitset<1024> _signature)
    {
        auto l_context = m_luaContext.lock();
        auto l_category = l_context->GetCategory(_signature).lock();

        if (!l_category)
        {
            Log::Error("Cannot add entity to category with unknown signature.");
            return;
        }

        // Guard against double-adds: if the entity is already in this category, do nothing.
        if (m_entityBitset.count(_id) && (m_entityBitset[_id] & _signature).any())
        {
            return;
        }

        // Add component data to category storage
        sol::table l_componentTable = l_category->AddMember(_id);

        // Store reference in the entity's combined attributes table
        m_dataTable.traverse_raw_get<sol::table>(
                       "Entities",
                       _id,
                       "attributes")
            .set(l_category->GetName(), l_componentTable);

        // Update entity signature
        m_entityBitset[_id] |= _signature;
        m_activeCategories |= _signature;
    }

    void EntityRegistry::RemoveFromCategory(int _id, std::bitset<1024> _signature)
    {
        auto l_context = m_luaContext.lock();
        auto l_category = l_context->GetCategory(_signature).lock();

        if (!l_category)
        {
            return;
        }

        l_category->RemoveMember(_id);

        // Remove from entity's combined table
        m_dataTable.traverse_raw_get<sol::table>("Entities", _id, "attributes")
            .set(l_category->GetName(), sol::nil);

        // Update entity signature
        m_entityBitset[_id] ^= _signature;

        // If after removal, no entity uses this category, remove from active set
        bool l_stillInUse = false;
        for (const auto& pair : m_entityBitset)
        {
            if ((pair.second & _signature).any())
            {
                l_stillInUse = true;
                break;
            }
        }
        if (!l_stillInUse)
        {
            m_activeCategories &= ~_signature;
        }
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

    std::vector<int> EntityRegistry::GetAllRegisteredIds()
    {
        std::vector<int> l_ids;
        for (const auto& pair : m_entityBitset)
        {
            l_ids.push_back(pair.first);
        }
        return l_ids;
    }

    std::string EntityRegistry::GetEntityName(int _id)
    {
        return  m_dataTable.traverse_raw_get<std::string>("Entities", _id, "name");
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

    sol::table EntityRegistry::GetEntityTable()
    {
        return m_dataTable.raw_get<sol::table>("Entities");
    }

    sol::table EntityRegistry::GetCategoryTable()
    {
        auto l_context = m_luaContext.lock();
        sol::table l_categoryTable = l_context->CreateTable();

        // Iterate over active categories and add them to the table
        for (int i = 0; i < 1024; ++i)
        {
            if (m_activeCategories.test(i))
            {
                std::bitset<1024> l_sig;
                l_sig.set(i);
                auto l_cat = l_context->GetCategory(l_sig).lock();
                if (l_cat)
                {
                    l_categoryTable.set(l_cat->GetName(), l_cat.get());
                }
            }
        }

        return l_categoryTable;
    }
}