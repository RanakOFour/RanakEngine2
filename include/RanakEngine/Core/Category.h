#ifndef CATEGORY_H
#define CATEGORY_H

#include "sol/sol.hpp"

#include <string>
#include <bitset>
#include <vector>
#include <map>

namespace RanakEngine::Asset
{
    class LuaFile;
}

namespace RanakEngine::Core
{
    class LuaContext;
    class CategoryFactory;

    /**
     * @class Category
     * @brief Defines a named set of typed fields that can be attached to entities.
     *
     * Equivalent to a "component" in traditional ECS.  A Category owns the
     * canonical set of field names and default values (m_fields), and stores
     * an independent copy of those fields for every entity member
     * (m_entityDataTables).  Fast entity<->index lookup is provided by the
     * bidirectional maps m_entityToIndex / m_indexToEntity.
     *
     * Each Category is assigned a unique bit in a 1024-bit signature by the
     * CategoryFactory; entity queries combine signatures with bitwise AND.
     *
     * Categories are constructed from Lua via the call-constructor syntax:
     * @code{.lua}
     *   return Category { Speed = 5.0, Jumping = false }
     * @endcode
     * The filename is injected as the name afterwards by LuaContext::CreateCategory().
     */
    class Category
    {
        friend LuaContext;
        friend CategoryFactory;
        private:
        std::weak_ptr<Asset::LuaFile> m_originFile; ///< The .lua file this category was loaded from.

        std::string           m_name;      ///< The category's name (derived from filename).
        std::bitset<1024>     m_signature; ///< Unique per-bit identifier assigned by CategoryFactory.
        sol::table            m_fields;    ///< Base (default) field table shared by all members.

        int                        m_size;             ///< Number of entities in this category.
        std::vector<sol::table>    m_entityDataTables; ///< Per-entity field tables (indexed by m_entityToIndex).
        std::map<int, int>         m_entityToIndex;    ///< Entity ID -> data-table index.
        std::map<int, int>         m_indexToEntity;    ///< Data-table index -> entity ID.

        static void DefineUsertype(sol::state& _state)
        {
            _state.new_usertype<Category>("Category",
                                            sol::call_constructor,
                                            sol::factories([](sol::table _fields) {
                                                if(!_fields.valid())
                                                {
                                                    throw std::runtime_error("Category constructor requires a table argument.");
                                                }

                                                std::string l_name = _fields.get_or<std::string>("name", "");
                                                return Category(l_name, _fields);
                                            }),
                                            "name", sol::readonly(&Category::m_name),
                                            "baseFields", sol::readonly(&Category::m_fields),
                                            "getMembers", sol::readonly(&Category::GetMembers),
                                            "getFieldsFor", sol::readonly(&Category::GetFieldsFor),
                                            "getSize", sol::readonly(&Category::GetSize)
                                         );
        };

        public:
        Category();
        /**
         * @brief Constructs a category with a name and a base-field table.
         * @param _name   Category name (usually injected from the filename).
         * @param _baseFields Lua table of field-name -> default-value pairs.
         */
        Category(std::string _name, sol::table _baseFields);
        ~Category();

        /**
         * @brief Adds the entity with the given ID as a member and returns its field table.
         *
         * The table is a deep copy of the base fields so the entity's values
         * are independent of every other member.
         *
         * @param _id Entity ID to add.
         * @return Reference to the newly created per-entity field table.
         */
        sol::table& AddMember(int _id);
        /**
         * @brief Removes the entity with the given ID from this category.
         * @param id Entity ID to remove.
         */
        void RemoveMember(int id);

        /**
         * @brief Returns the field table for the given entity.
         * @param _id Entity ID.
         * @return Copy of the entity's field table.
         */
        sol::table GetFieldsFor(int _id);
        /** @brief Returns a list of all entity IDs that are members of this category. */
        std::vector<int> GetMembers();

        /** @brief Returns a reference to the base (default) field table. */
        sol::table& GetBaseData();
        /** @brief Returns the category's name. */
        std::string GetName();
        /** @brief Returns the number of entities currently in this category. */
        int GetSize();
        /** @brief Returns the category's unique 1024-bit signature. */
        std::bitset<1024> GetSignature();

        /**
         * @brief Associates this category with the .lua file it was loaded from.
         * @param _file Weak pointer to the source LuaFile.
         */
        void SetOriginFile(std::weak_ptr<Asset::LuaFile> _file);
        /** @brief Returns a weak pointer to the source .lua file, if any. */
        std::weak_ptr<Asset::LuaFile> GetOriginFile();
    };
}

#endif