#ifndef ENTITYREGISTRY_H
#define ENTITYREGISTRY_H

#include <vector>
#include <map>
#include <unordered_map>
#include <bitset>
#include <memory>

#include "sol/sol.hpp"

namespace RanakEngine::Core
{
    class LuaContext;
    class Category;

    /**
     * @class EntityRegistry
     * @brief Manages entity lifetimes and tracks which Categories each entity belongs to.
     *
     * Entities are represented as integer IDs.  The registry maintains a free-list
     * (m_freeIds) to recycle IDs after deletion, and defers actual deletion until
     * DeleteFlaggedEntities() is called at the end of each frame so that iterating
     * rules and removing entities during a pass is safe.
     *
     * Each entity has a combined signature bitset that is the OR of every Category
     * signature it has been added to.  Rules use this combined signature to quickly
     * find all entities that match their own required Categories.
     *
     * Also exposes Lua-visible tables (m_dataTable) used by the Lua rule callbacks.
     */
    class EntityRegistry
    {
        private:
        std::weak_ptr<LuaContext> m_luaContext; ///< Used to access the CategoryFactory during add/remove operations.

        int              m_nextFreeId = 0;  ///< Next ID to allocate if the free-list is empty.
        std::vector<int> m_idsToDelete;     ///< Entity IDs marked for removal at end-of-frame.
        std::vector<int> m_freeIds;         ///< Recycled IDs available for new entities.
        std::unordered_map<std::bitset<1024>, std::weak_ptr<Category>> m_categories; ///< Signature -> Category cache.
        std::map<int, std::bitset<1024>>    m_entityBitset; ///< Entity ID -> combined category signature.

        sol::table m_dataTable; ///< Lua-side table exposed so rules can query entity data.

        std::bitset<1024> m_activeCategories; ///< OR of all Category signatures currently in use.

        /** @brief Removes all entities flagged for deletion this frame. */
        void DeleteFlaggedEntities();
        /** @brief Recomputes m_activeCategories from the current entity bitsets. */
        void UpdateActiveCategories();

        public:
        EntityRegistry();
        ~EntityRegistry();

        /**
         * @brief Creates a new entity and returns its unique ID.
         * @return The newly allocated entity ID.
         */
        int AddEntity();
        /**
         * @brief Flags the entity for removal at the end of the current frame.
         * @param _id Entity ID to remove.
         */
        void RemoveEntity(int _id);

        /**
         * @brief Adds entity _id to the Category identified by _signature.
         * @param _id        Entity ID.
         * @param _signature Unique signature of the Category to add the entity to.
         */
        void AddToCategory(int _id, std::bitset<1024> _signature);
        /**
         * @brief Removes entity _id from the Category identified by _signature.
         * @param _id        Entity ID.
         * @param _signature Category signature to remove the entity from.
         */
        void RemoveFromCategory(int _id, std::bitset<1024> _signature);

        /**
         * @brief Returns all entity IDs whose combined signatures include every bit in _combinedSignature.
         * @param _combinedSignature Bitwise OR of one or more Category signatures.
         * @return Entities that belong to all specified categories.
         */
        std::vector<int> GetEntitiesWith(std::bitset<1024> _combinedSignature);

        /**
         * @brief Returns a human-readable name for the entity (e.g. "Entity 3").
         * @param _id Entity ID.
         */
        std::string GetEntityName(int _id);
        /**
         * @brief Returns a Lua table containing all category field tables for the entity.
         * @param _id Entity ID.
         */
        sol::table GetEntityAttributes(int _id);

        /** @brief Returns true when no entities exist. */
        bool Empty();
        /** @brief Returns the current number of live entities. */
        int GetEntityCount();

        /** @brief Returns the raw per-entity Lua data table. */
        sol::table GetEntityTable();
        /** @brief Returns the raw per-category Lua lookup table. */
        sol::table GetCategoryTable();
    };
}

#endif